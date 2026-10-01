// B01/B02 pre-test, managed-code side (throwaway). Plain Java, no dependencies, so the
// phone test app can include this file as-is (Android runs it on its own runtime).
// Same initial state, same order of operations and same checksum as the Rust kernels
// in ../kbench, so "java:heat" f32/fx32 and "java:rng" splitmix must give the same
// checksums as "rust:heat" and "rust:rng" (X11, X1).
//
// Desktop use:  java Kernels.java '{"kernel":"java:heat","format":"f32","threads":1,"seconds":2}'
//               java Kernels.java --batch FILE      (one config JSON per line)
package dev.kindling.pretests;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.Locale;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public final class Kernels {
    private Kernels() {}

    static final long GOLDEN = 0x9e3779b97f4a7c15L;
    static final long FNV_OFF = 0xcbf29ce484222325L;
    static final long FNV_PRIME = 0x00000100000001b3L;
    static final long HEAT_SALT = 0x4845415400000001L;
    static final int N = 512;

    static long mix64(long z) {
        z = (z ^ (z >>> 30)) * 0xbf58476d1ce4e5b9L;
        z = (z ^ (z >>> 27)) * 0x94d049bb133111ebL;
        return z ^ (z >>> 31);
    }

    static long fnv(long h, long v) {
        return (h ^ v) * FNV_PRIME;
    }

    // ------------------------------------------------------------ B02: splitmix64 keyed draw
    /** Stream key k0 from (world, system, purpose); system and purpose are unsigned 32-bit. */
    static long streamKey(long world, int system, int purpose) {
        long h = mix64(world + GOLDEN);
        h = mix64(h ^ ((system & 0xffffffffL) + GOLDEN * 2));
        h = mix64(h ^ ((purpose & 0xffffffffL) + GOLDEN * 3));
        return h;
    }

    /** One draw for (being, moment): a pure function of the key, no shared state. */
    static long splitmixDraw(long k0, long being, long moment) {
        long seed = mix64(k0 ^ being);
        return mix64(seed + (moment + 1) * GOLDEN);
    }

    // ------------------------------------------------------------ kernels
    interface Kern {
        long opsPerRep();
        int steps();
        void reset(int t, int n);
        void step(int s, int t, int n);
        long checksum();
    }

    static int lo(int len, int t, int n) {
        return (int) ((long) len * t / n);
    }

    static float updF(float c, float n, float s, float e, float w, float a) {
        return c + a * (((n + s) + (e + w)) - 4.0f * c);
    }

    static int updX(int c, int n, int s, int e, int w, int a) {
        int lap = ((n + s) + (e + w)) - 4 * c; // Java int arithmetic wraps, like Rust's wrapping ops
        return c + (int) (((long) a * (long) lap) >> 16);
    }

    static final class HeatF32 implements Kern {
        final float[] init = new float[N * N];
        final float[][] buf = {new float[N * N], new float[N * N]};
        final int steps;

        HeatF32(int steps) {
            this.steps = steps;
            for (int i = 0; i < N * N; i++) {
                long u = mix64(i ^ HEAT_SALT) >>> 40;
                init[i] = (float) u * (100.0f / 16777216.0f);
            }
        }

        public long opsPerRep() { return (long) N * N * steps; }
        public int steps() { return steps; }

        public void reset(int t, int n) {
            int r0 = lo(N, t, n), r1 = lo(N, t + 1, n);
            System.arraycopy(init, r0 * N, buf[0], r0 * N, (r1 - r0) * N);
        }

        public void step(int s, int t, int n) {
            float[] src = buf[s & 1], dst = buf[(s + 1) & 1];
            final float a = 0.2f;
            int r0 = lo(N, t, n), r1 = lo(N, t + 1, n);
            for (int y = r0; y < r1; y++) {
                int ro = y * N, uo = ((y + N - 1) % N) * N, dn = ((y + 1) % N) * N;
                dst[ro] = updF(src[ro], src[uo], src[dn], src[ro + 1], src[ro + N - 1], a);
                for (int x = 1; x < N - 1; x++) {
                    dst[ro + x] = updF(src[ro + x], src[uo + x], src[dn + x], src[ro + x + 1], src[ro + x - 1], a);
                }
                dst[ro + N - 1] = updF(src[ro + N - 1], src[uo + N - 1], src[dn + N - 1], src[ro], src[ro + N - 2], a);
            }
        }

        public long checksum() {
            long h = FNV_OFF;
            for (float v : buf[steps & 1]) h = fnv(h, Float.floatToRawIntBits(v) & 0xffffffffL);
            return mix64(h);
        }
    }

    static final class HeatFx32 implements Kern {
        final int[] init = new int[N * N];
        final int[][] buf = {new int[N * N], new int[N * N]};
        final int steps;

        HeatFx32(int steps) {
            this.steps = steps;
            for (int i = 0; i < N * N; i++) {
                long u = mix64(i ^ HEAT_SALT) >>> 40;
                init[i] = (int) ((u * 100) >> 8);
            }
        }

        public long opsPerRep() { return (long) N * N * steps; }
        public int steps() { return steps; }

        public void reset(int t, int n) {
            int r0 = lo(N, t, n), r1 = lo(N, t + 1, n);
            System.arraycopy(init, r0 * N, buf[0], r0 * N, (r1 - r0) * N);
        }

        public void step(int s, int t, int n) {
            int[] src = buf[s & 1], dst = buf[(s + 1) & 1];
            final int a = 13107; // round(0.2 * 2^16)
            int r0 = lo(N, t, n), r1 = lo(N, t + 1, n);
            for (int y = r0; y < r1; y++) {
                int ro = y * N, uo = ((y + N - 1) % N) * N, dn = ((y + 1) % N) * N;
                dst[ro] = updX(src[ro], src[uo], src[dn], src[ro + 1], src[ro + N - 1], a);
                for (int x = 1; x < N - 1; x++) {
                    dst[ro + x] = updX(src[ro + x], src[uo + x], src[dn + x], src[ro + x + 1], src[ro + x - 1], a);
                }
                dst[ro + N - 1] = updX(src[ro + N - 1], src[uo + N - 1], src[dn + N - 1], src[ro], src[ro + N - 2], a);
            }
        }

        public long checksum() {
            long h = FNV_OFF;
            for (int v : buf[steps & 1]) h = fnv(h, v & 0xffffffffL);
            return mix64(h);
        }
    }

    /** One keyed draw per op: beings 0..65535 at moments 0..steps-1 (same as rust:rng). */
    static final class RngSplitmix implements Kern {
        static final int BEINGS = 65536;
        final long k0 = streamKey(1, 3, 9);
        final long[] slots = new long[64 * 8];
        final int steps;

        RngSplitmix(int steps) { this.steps = steps; }

        public long opsPerRep() { return (long) BEINGS * steps; }
        public int steps() { return steps; }
        public void reset(int t, int n) { slots[t * 8] = 0; }

        public void step(int s, int t, int n) {
            int b0 = lo(BEINGS, t, n), b1 = lo(BEINGS, t + 1, n);
            long acc = 0;
            for (long b = b0; b < b1; b++) acc += splitmixDraw(k0, b, s);
            slots[t * 8] += acc;
        }

        public long checksum() {
            long total = 0;
            for (long v : slots) total += v;
            return mix64(fnv(FNV_OFF, total));
        }
    }

    // ------------------------------------------------------------ harness
    static final class SpinBarrier {
        final int n;
        final AtomicInteger count = new AtomicInteger();
        volatile int gen;

        SpinBarrier(int n) { this.n = n; }

        void await() {
            int g = gen;
            if (count.incrementAndGet() == n) {
                count.set(0);
                gen = g + 1;
            } else {
                int spins = 0;
                while (gen == g) {
                    if (++spins > 2000) Thread.yield();
                }
            }
        }
    }

    /** Rep 0 is an untimed warm-up; then reps run until `seconds` pass (and at least minReps). */
    static long[] measure(final Kern k, final int threads, final double seconds, final int minReps) throws InterruptedException {
        final SpinBarrier barrier = new SpinBarrier(threads);
        final AtomicBoolean stop = new AtomicBoolean(false);
        final long[] res = new long[4]; // reps, elapsed ns, checksum of rep 0, checksum of last rep
        Thread[] ts = new Thread[threads];
        for (int t = 0; t < threads; t++) {
            final int tt = t;
            ts[t] = new Thread(() -> {
                int rep = 0;
                long ckFirst = 0, t0 = 0;
                while (true) {
                    k.reset(tt, threads);
                    barrier.await();
                    for (int s = 0; s < k.steps(); s++) {
                        k.step(s, tt, threads);
                        barrier.await();
                    }
                    if (tt == 0) {
                        if (rep == 0) {
                            ckFirst = k.checksum();
                            t0 = System.nanoTime();
                        } else {
                            long el = System.nanoTime() - t0;
                            if (el >= (long) (seconds * 1e9) && rep >= minReps) {
                                res[0] = rep;
                                res[1] = el;
                                res[2] = ckFirst;
                                res[3] = k.checksum();
                                stop.set(true);
                            }
                        }
                    }
                    barrier.await();
                    if (stop.get()) break;
                    rep++;
                }
            });
            ts[t].start();
        }
        for (Thread th : ts) th.join();
        return res;
    }

    // ------------------------------------------------------------ JSON in, JSON out
    static final Pattern KV = Pattern.compile("\"(\\w+)\"\\s*:\\s*(\"([^\"]*)\"|[-+0-9.eE]+|true|false)");

    static String get(String json, String key, String dflt) {
        Matcher m = KV.matcher(json);
        while (m.find()) {
            if (m.group(1).equals(key)) return m.group(3) != null ? m.group(3) : m.group(2);
        }
        return dflt;
    }

    static String hex(long v) {
        return String.format(Locale.ROOT, "%016x", v);
    }

    public static String run(String configJson) {
        try {
            if ("true".equals(get(configJson, "list", "false"))) {
                return "{\"kernels\":[{\"kernel\":\"java:heat\",\"formats\":[\"f32\",\"fx32\"],\"suggested\":{\"threads\":1,\"seconds\":1.5}},"
                        + "{\"kernel\":\"java:rng\",\"rngs\":[\"splitmix\"],\"suggested\":{\"threads\":1,\"seconds\":1.5}}]}";
            }
            String kernel = get(configJson, "kernel", "");
            String format = get(configJson, "format", "f32");
            int threads = Integer.parseInt(get(configJson, "threads", "1"));
            double seconds = Double.parseDouble(get(configJson, "seconds", "2"));
            int minReps = Integer.parseInt(get(configJson, "min_reps", "3"));
            Kern k;
            String op;
            if (kernel.equals("java:heat")) {
                int steps = Integer.parseInt(get(configJson, "steps", "64"));
                if (format.equals("f32")) k = new HeatF32(steps);
                else if (format.equals("fx32")) k = new HeatFx32(steps);
                else return "{\"error\":\"java:heat supports f32 and fx32\"}";
                op = "one cell update";
            } else if (kernel.equals("java:rng")) {
                String rng = get(configJson, "rng", "splitmix");
                if (!rng.equals("splitmix")) return "{\"error\":\"java:rng supports splitmix only\"}";
                k = new RngSplitmix(Integer.parseInt(get(configJson, "steps", "16")));
                op = "one keyed 64-bit draw";
                format = null;
            } else {
                return "{\"error\":\"unknown kernel " + kernel + "\"}";
            }
            if (threads < 1 || threads > 64) return "{\"error\":\"threads must be 1..64\"}";
            long[] r = measure(k, threads, seconds, Math.max(1, minReps));
            long ops = r[0] * k.opsPerRep();
            double el = r[1] / 1e9;
            return String.format(Locale.ROOT,
                    "{\"kernel\":\"%s\",\"lang\":\"java\",\"name\":\"%s\",\"format\":%s,\"rng\":%s,\"threads\":%d,"
                            + "\"ops_per_sec\":%.6e,\"ops\":%d,\"elapsed_s\":%.6f,\"reps\":%d,\"ops_per_rep\":%d,\"op\":\"%s\","
                            + "\"checksum\":\"%s\",\"checksum_first_rep\":\"%s\",\"reps_consistent\":%b,"
                            + "\"vm\":\"%s %s\"}",
                    kernel, kernel.substring(5), format == null ? "null" : "\"" + format + "\"",
                    format == null ? "\"splitmix\"" : "null", threads, ops / el, ops, el, r[0], k.opsPerRep(), op,
                    hex(r[3]), hex(r[2]), r[2] == r[3],
                    System.getProperty("java.vm.name", "?"), System.getProperty("java.vm.version", "?"));
        } catch (Throwable e) {
            return "{\"error\":\"" + e.toString().replace('"', '\'') + "\"}";
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length >= 2 && args[0].equals("--batch")) {
            try (BufferedReader br = new BufferedReader(new FileReader(args[1]))) {
                String line;
                while ((line = br.readLine()) != null) {
                    line = line.trim();
                    if (line.isEmpty() || line.startsWith("#")) continue;
                    System.out.println(run(line));
                    System.out.flush();
                }
            }
        } else if (args.length == 1) {
            System.out.println(run(args[0]));
        } else {
            System.err.println("usage: java Kernels.java '<json>' | --batch FILE");
            System.exit(2);
        }
    }
}
