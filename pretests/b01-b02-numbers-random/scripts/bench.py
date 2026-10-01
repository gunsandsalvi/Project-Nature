#!/usr/bin/env python3
"""B01/B02 pre-test: makes the run lists and summarises the results (median of runs).

  bench.py gen OUTDIR        write sweep.jsonl (timed, 3 interleaved rounds), threads.jsonl
                             (2 and 3 threads, short), qemu.jsonl (checksums only), java.jsonl
  bench.py summarize RAWDIR  read *.out.jsonl, write ../results/*.csv, print the tables
"""
import collections, json, math, os, statistics, sys

HERE = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.join(HERE, '..', 'results')
FORMATS = {'heat': ['f32', 'f64', 'fx32', 'fx64'], 'walk': ['f32', 'f64', 'fx32', 'fx64'],
           'sum': ['f32', 'f64', 'fx32', 'fx64'], 'learn': ['f32', 'f64', 'fx32']}
GENS = ['splitmix', 'philox', 'squares', 'pcg', 'wy', 'chacha8']


def native_configs(langs, threads, seconds, min_reps=3):
    out = []
    for lang in langs:
        for t in threads:
            for k, fmts in FORMATS.items():
                for f in fmts:
                    out.append(dict(kernel=f'{lang}:{k}', format=f, rng='splitmix', threads=t, seconds=seconds, min_reps=min_reps))
            for g in GENS:
                if g != 'splitmix':
                    out.append(dict(kernel=f'{lang}:walk', format='f32', rng=g, threads=t, seconds=seconds, min_reps=min_reps))
                out.append(dict(kernel=f'{lang}:rng', rng=g, threads=t, seconds=seconds, min_reps=min_reps))
    return out


def java_configs(threads, seconds):
    out = []
    for t in threads:
        out += [dict(kernel='java:heat', format='f32', threads=t, seconds=seconds),
                dict(kernel='java:heat', format='fx32', threads=t, seconds=seconds),
                dict(kernel='java:rng', rng='splitmix', threads=t, seconds=seconds)]
    return out


def write(path, rows):
    with open(path, 'w') as f:
        for r in rows:
            f.write(json.dumps(r) + '\n')


def gen(outdir):
    os.makedirs(outdir, exist_ok=True)
    base = native_configs(['rust', 'cpp'], [1, 4], 1.0)
    write(os.path.join(outdir, 'sweep.jsonl'), base * 3)  # 3 interleaved rounds
    write(os.path.join(outdir, 'threads.jsonl'), native_configs(['rust', 'cpp', 'cppnc'], [2, 3], 0.1, 2)
          + native_configs(['cppnc'], [1, 4], 0.1, 2))
    write(os.path.join(outdir, 'qemu.jsonl'), native_configs(['rust', 'cpp', 'cppnc'], [1], 0.0, 1)
          + [dict(c, threads=2) for c in native_configs(['rust'], [1], 0.0, 1)[:4]])
    write(os.path.join(outdir, 'java.jsonl'), java_configs([1, 4], 1.0) * 3 + java_configs([2, 3], 0.2))


def key(r):
    v = r['format'] if r['name'] != 'rng' else '-'
    g = r['rng'] if r['name'] in ('walk', 'rng') else '-'
    return (r['name'], v, g, r['lang'])


def load(rawdir, names):
    rows = []
    for n in names:
        p = os.path.join(rawdir, n)
        if os.path.exists(p):
            for line in open(p):
                r = json.loads(line)
                if 'error' in r:
                    print('ERROR', r)
                    continue
                r['_src'] = n
                rows.append(r)
    return rows


def summarize(rawdir):
    os.makedirs(RESULTS, exist_ok=True)
    timed = load(rawdir, ['sweep.out.jsonl', 'sweep2.out.jsonl', 'java.out.jsonl', 'java2.out.jsonl'])
    extra = load(rawdir, ['threads.out.jsonl'])
    qemu = load(rawdir, ['qemu.out.jsonl'])
    # --- speed table: median of the timed runs (1 s each), spread = (max-min)/median
    speed = collections.defaultdict(list)
    for r in timed:
        if r['elapsed_s'] >= 0.9:
            speed[key(r) + (r['threads'],)].append(r['ops_per_sec'])
    with open(os.path.join(RESULTS, 'speed.csv'), 'w') as f:
        f.write('kernel,format,rng,lang,threads,runs,median_Mops,min_Mops,max_Mops,spread_pct\n')
        for k in sorted(speed):
            v = speed[k]
            med = statistics.median(v)
            f.write(f'{k[0]},{k[1]},{k[2]},{k[3]},{k[4]},{len(v)},{med/1e6:.1f},{min(v)/1e6:.1f},{max(v)/1e6:.1f},'
                    f'{100*(max(v)-min(v))/med:.1f}\n')
    # --- determinism: one checksum per kernel/format/rng/lang across runs and thread counts
    cks = collections.defaultdict(lambda: collections.defaultdict(set))
    reps_bad = []
    for r in timed + extra:
        cks[key(r)][r['threads']].add(r['checksum'])
        if not r['reps_consistent']:
            reps_bad.append(key(r))
    q = {}
    for r in qemu:
        if r['threads'] == 1:
            q[key(r)] = r['checksum']
        else:
            cks[key(r) + ('qemu',)][r['threads']].add(r['checksum'])
    with open(os.path.join(RESULTS, 'determinism.csv'), 'w') as f:
        f.write('kernel,format,rng,lang,threads_seen,runs,one_checksum_all_runs_and_threads,x86_checksum,'
                'arm_qemu_checksum,arm_matches_x86\n')
        for k in sorted(cks):
            if len(k) == 5:
                continue
            per_t = cks[k]
            allc = set().union(*per_t.values())
            n = sum(1 for r in timed + extra if key(r) == k)
            x86 = sorted(allc)[0] if len(allc) == 1 else 'MULTIPLE'
            arm = q.get(k, '')
            f.write(f'{k[0]},{k[1]},{k[2]},{k[3]},{"/".join(map(str, sorted(per_t)))},{n},{len(allc) == 1},{x86},{arm},'
                    f'{"" if not arm else arm == x86}\n')
    print('reps inconsistent:', reps_bad or 'none')
    # cross-language agreement on x86
    by_kf = collections.defaultdict(set)
    for k in cks:
        if len(k) == 4:
            by_kf[k[:3]] |= set().union(*cks[k].values())
    print('kernel/format/rng with more than one checksum across ALL languages on x86:',
          [k for k, v in by_kf.items() if len(v) > 1] or 'none')


def practrand(path):
    """Returns (final_log2_len, [(log2_len, test, evaluation)]) from one RNG_test output."""
    import re
    if not os.path.exists(path):
        return None, []
    flags, length = [], None
    for line in open(path):
        m = re.search(r'\(2\^(\d+) bytes\)', line)
        if m and line.startswith('length='):
            length = int(m.group(1))
            continue
        m = re.match(r'\s+(\S+)\s+R=\s*\S+\s+p\s*=\s*\S+\s+(.+?)\s*$', line)
        if m and length is not None:
            flags.append((length, m.group(1), m.group(2)))
    return length, flags


def decide(rawdir):
    """Applies rules R1 to R5 from NOTES.md to results/*.csv and results/practrand/."""
    rows = list(csv_rows(os.path.join(RESULTS, 'speed.csv')))
    det = list(csv_rows(os.path.join(RESULTS, 'determinism.csv')))
    sp = {(r['kernel'], r['format'], r['rng'], r['lang'], int(r['threads'])): float(r['median_Mops']) for r in rows}
    det_ok = {(r['kernel'], r['format'], r['rng'], r['lang']): r['one_checksum_all_runs_and_threads'] == 'True' for r in det}
    print('\nR1 generator: PractRand + speed (rng kernel, 1 thread, best of Rust and C++)')
    for g in GENS:
        verdicts = []
        for mode in ('moments', 'beings'):
            final, flags = practrand(os.path.join(RESULTS, 'practrand', f'{g}-{mode}.txt'))
            fails = [f for f in flags if f[2].startswith('FAIL')]
            very = [f for f in flags if f[1:] and f[0] == final and 'very suspicious' in f[2].lower()]
            last2 = sorted(set(l for l, _, _ in flags))[-2:] if flags else []
            lens = sorted({f[0] for f in flags})
            repeat = []
            if final is not None:
                at = lambda L: {t for l, t, _ in flags if l == L}
                repeat = sorted(at(final) & at(final - 1))
            ok = final is not None and final >= 34 and not fails and not very and not repeat
            verdicts.append((mode, final, ok, len(flags), fails[:1], very[:1], repeat[:2]))
        speed = max(sp.get(('rng', '-', g, l, 1), 0) for l in ('rust', 'cpp'))
        walk = max(sp.get(('walk', 'f32', g, l, 1), 0) for l in ('rust', 'cpp'))
        q = all(v[2] for v in verdicts)
        print(f'  {g:9} qualifies={q}  rng {speed:7.1f} M/s  walk {walk:6.1f} M/s  ' +
              '; '.join(f'{m}: 2^{fl} flags={n} fail={fa} very={ve} repeat={re}' for m, fl, _, n, fa, ve, re in verdicts))
    print('\nR2 format: best language per format; qualifies if determinism ok and within 15% of fastest at 1 and 4 threads')
    for k, fmts in FORMATS.items():
        best = {}
        for t in (1, 4):
            for f in fmts:
                best[(f, t)] = max(sp.get((k, f, 'splitmix' if k == 'walk' else '-', l, t), 0) for l in ('rust', 'cpp'))
        quals = []
        for f in fmts:
            within = all(best[(f, t)] >= 0.85 * max(best[(g, t)] for g in fmts) for t in (1, 4))
            dok = all(det_ok.get((k, f, 'splitmix' if k == 'walk' else '-', l), False) for l in ('rust', 'cpp'))
            if within and dok:
                quals.append(f)
        pick = next((f for f in ['f32', 'fx32', 'f64', 'fx64'] if f in quals), None)
        print(f'  {k:6} ' + '  '.join(f'{f}: {best[(f, 1)]:7.1f}/{best[(f, 4)]:7.1f}' for f in fmts) +
              f'   qualifying={quals} -> pick {pick}')
    print('\nR3 language: Rust/C++ speed ratios')
    ratios = []
    for (k, f, g, l, t), v in sp.items():
        if l == 'rust' and (k, f, g, 'cpp', t) in sp:
            ratios.append(((k, f, g, t), v / sp[(k, f, g, 'cpp', t)]))
    gm = math.exp(sum(math.log(r) for _, r in ratios) / len(ratios))
    print(f'  {len(ratios)} pairs, geometric mean Rust/C++ = {gm:.3f}; range {min(r for _, r in ratios):.2f}..{max(r for _, r in ratios):.2f}')
    for kk, r in sorted(ratios, key=lambda x: x[1]):
        if abs(r - 1) > 0.25:
            print(f'    {kk}: {r:.2f}')
    print('\nR5 Java heat vs best native heat (same format and threads)')
    for f in ('f32', 'fx32'):
        for t in (1, 4):
            j = sp.get(('heat', f, '-', 'java', t), 0)
            n = max(sp.get(('heat', f, '-', l, t), 0) for l in ('rust', 'cpp'))
            print(f'  heat {f} {t}t: java {j:7.1f}  native {n:7.1f}  ratio {j / n if n else 0:.2f}')
    for t in (1, 4):
        j = sp.get(('rng', '-', 'splitmix', 'java', t), 0)
        n = max(sp.get(('rng', '-', 'splitmix', l, t), 0) for l in ('rust', 'cpp'))
        print(f'  rng splitmix {t}t: java {j:7.1f}  native {n:7.1f}  ratio {j / n if n else 0:.2f}')


def tables():
    """Markdown tables for NOTES.md, from results/*.csv and results/practrand/."""
    rows = list(csv_rows(os.path.join(RESULTS, 'speed.csv')))
    sp = {(r['kernel'], r['format'], r['rng'], r['lang'], int(r['threads'])): r for r in rows}
    def cell(k):
        r = sp.get(k)
        return f"{float(r['median_Mops']):,.0f} ({float(r['spread_pct']):.0f}%)" if r else '-'
    n = max(int(r['runs']) for r in rows)
    print(f'Million ops per second, median of {n} runs of 1 s (spread in brackets).\n')
    print('| Kernel | Rust 1 | C++ 1 | Rust 4 | C++ 4 |')
    print('|---|---|---|---|---|')
    for k, fmts in FORMATS.items():
        for f in fmts:
            g = 'splitmix' if k == 'walk' else '-'
            print(f'| {k} {f} | ' + ' | '.join(cell((k, f, g, l, t)) for t in (1, 4) for l in ('rust', 'cpp')) + ' |')
    print('\nJava (desktop JVM), same units:\n')
    print('| Kernel | Java 1 | Java 4 |')
    print('|---|---|---|')
    for k, f, g in (('heat', 'f32', '-'), ('heat', 'fx32', '-'), ('rng', '-', 'splitmix')):
        print(f'| {k} {f if f != "-" else g} | {cell((k, f, g, "java", 1))} | {cell((k, f, g, "java", 4))} |')
    print('\nGenerators: million draws per second (rng kernel) and agent-steps per second (walk f32), 1 thread, best of Rust and C++; PractRand result per stream.\n')
    print('| Generator | Draws | Walk | One key | Neighbours | Retry |')
    print('|---|---|---|---|---|---|')
    for g in GENS:
        d = max(float(sp[('rng', '-', g, l, 1)]['median_Mops']) for l in ('rust', 'cpp'))
        w = max(float(sp[('walk', 'f32', g, l, 1)]['median_Mops']) for l in ('rust', 'cpp'))
        res = []
        for mode in ('moments', 'beings', 'retry'):
            final, flags = practrand(os.path.join(RESULTS, 'practrand', f'{g}-{mode}.txt'))
            if final is None:
                res.append('not run')
                continue
            fails = [f for f in flags if f[2].startswith('FAIL')]
            if fails:
                res.append(f'FAIL at 2^{fails[0][0]}')
            else:
                worst = [f[2] for f in flags if f[0] == final]
                res.append(f'pass 2^{final}' + (f' ({", ".join(sorted(set(worst)))})' if worst else ''))
        print(f'| {g} | {d:,.0f} | {w:,.0f} | ' + ' | '.join(res) + ' |')


def csv_rows(path):
    import csv
    with open(path) as f:
        yield from csv.DictReader(f)


if __name__ == '__main__':
    if sys.argv[1] == 'gen':
        gen(sys.argv[2])
    elif sys.argv[1] == 'decide':
        decide(sys.argv[2] if len(sys.argv) > 2 else '')
    elif sys.argv[1] == 'tables':
        tables()
    else:
        summarize(sys.argv[2])
