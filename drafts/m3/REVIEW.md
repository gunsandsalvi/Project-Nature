# M3 research: independent second pass

8 October 2026. This reviews the research proposal, not an implemented milestone. The requested independent critic read all four draft documents, checked original requirements and selected current code/engine contracts, and wrote `critic-findings.md`. The lead owns the final corrections. No repository files were changed, no build was needed and no phone performance is claimed.

## Result

The critic found three substantive gaps in acceptance coverage, plus four issues that the lead's concurrent cross-check also identified. All have been corrected in the consolidated drafts. A follow-up verification of the corrections is recorded below. Q1–Q10 were subsequently answered on 8 October 2026 and incorporated as recorded below; the research task does not authorize repository adoption.

## Findings and corrections

| Finding | What changed | Final location |
|---|---|---|
| Natural-event tests covered quake/eruption rates but could miss wrong storm, lightning, drought, flood or harsh-winter rates. Mean rain alone cannot catch persistence errors. | Added G4b, per-climate natural rates, independently defined event/exposure units, storm-day and dry-spell distribution checks, explicit reference preparation and run budget. Forced events never count. Wildfire remains M4 under Q1. | Architecture A7.10; plan T3.6b.3/T3.7a.3 and G4b; Q3/Q9; research section 7 |
| One hundred sampled eclipse locations could pass while another place failed WLD-07. | Kept sampling as an early diagnostic only. Final acceptance covers all canonical world-cell locations plus conservative intra-cell visibility bounds/refinement and reports spatial minimum/maximum. Uncertified regions fail. Q6 names the geometric horizon and separate obscuration reporting. | Architecture A7.10; plan T3.6a.3/G5; Q6 |
| Hammer/grinding stone patches and downstream distance-dependent rounding were missing. | Added quartzite/basalt/sandstone host/exposure rules and routed-distance rounding metadata, with independent occurrence and matched-stone monotonic rounding checks. | Architecture A7.6/A7.7; plan T3.3b.2/T3.4b.2–3/G2; research section 4 |
| Early cave certificates preceded erosion and could certify a cave that the finished land exposed, removed or flooded. | Rock-stage data is a provisional host envelope. Final existence/entrances follow erosion; dryness follows final climate/water and is rechecked at the retained start after settling. Added cut-through-envelope and flooded-start checks. | Architecture A7.6; plan α3.2b/α3.3b/α3.9 |
| Sea ice had incorrectly inherited the five-day sea cadence. | All ice updates daily. Sea warmth/ecological state keeps its five-day cadence. Added an intervening-day ice test and save/reopen across the cadence boundary. | Architecture A7.7/A7.11; plan T3.7b.1 and its tests |
| WLD-06 tests checked input bounds without final land-fraction/spread coverage. | Recheck the final eroded result across 100 seeds. Proposed minimum spread is one sample in each of ten equal tilt/land-share bins, frozen before tuning. | Plan α3.2a/G1; acceptance definitions subject to owner review |
| Plan wording implied separately versioned system chunks already existed. | Named local versions inside current SYST payloads; static base remains an explicit new essential chunk. Snapshot/storage extensions still require actual implementation and recovery proofs. | Architecture A7.14; plan T3.1b.2 |

The lead also accepted the critic's optional tightening:

- Derive the river audit set independently from the final drainage graph, using fixed headwater/confluence/outlet segmentation; omitted geometry cannot improve the denominator.
- Keep each field's fertility in its authoritative area delta over the cell baseline, and test two fields changing independently.
- Include full-moon legibility, moving cloud shadows and currents in the generated-world visual route, while retaining M2's PRE-31 route.

Additional lead checks corrected the settling task reference to α3.9b, clarified the deterministic fallback order, made the candidate qualification share explicitly aggregate over all 2,000 audited candidates, corrected the ledger total to 1.46 GiB, and narrowed the natural-valley flood interval to 21.6–26.4 hours to honor the existing 10% meaning of “about a day.”

## What was verified for this delivery

`budget-check.py` was run locally. It confirms the exact cell/area counts, 250 m alignment, 7.33 TiB cost of eagerly storing all area height grids, 5.77 MiB for 24 height grids alone, 1.463 GiB initial ledger, 512 revision-manifest entries and the long-run workload arithmetic. These are dimensions and estimates, not measured production allocations or phone timings.

The drafts were checked for unique task IDs, matching alpha/delivery rows, complete tests/phone/risk lines and requirement references. No implementation tests were run because this assignment changes only research documents in the research folder. The future plan explicitly requires implementation proofs, held-phone measurements, recovery tests and another independent milestone review.

The critic spot-checked the procedural-terrain evaluation paper, CrossCode developer article and Around the World climate account; the limited claims were supported. This is a targeted citation review, not a claim that every source's full text was read. The research marks the tectonic paper's abstract-only limitation and does not borrow published timing claims as Kindling phone evidence.

## Decision status after the owner response

The owner approved Q1's phased M3/M4/M5 acceptance on 8 October 2026. Living settling, actual food/species, survival/bands and real archaeology remain mapped obligations until their assigned milestones prove them. Q2–Q10 are also answered; Q6 is 3–8 visible eclipses at every place in 70 game years, replacing the recommended 2–5.

Q11 visual acceptance and Q12 milestone/M4-plan acceptance remain for the later implementation review. Unspecified detailed test/reference values still must be frozen before tuning.

## Follow-up verification

The same independent critic re-read the corrected drafts and saved a verification appendix in `critic-findings.md`. It confirmed all three required findings, all four associated corrections and the optional tightenings were present, with no new substantive contradiction found.

The critic's last read preceded the final flood-interval edit; the lead then checked both passages directly and confirmed 21.6–26.4 hours, with no 18–30-hour passage remaining in the consolidated drafts. The original critic report is retained as an audit record.

The final arithmetic pass also corrected the 25-search suite duration to 100 minutes at nominal limits or 110 at outer limits, plus setup; made the 60/30 fps deadline arithmetic explicit; and added direct two-core world-alone throughput reporting alongside the core-time budget. These checks do not change the original performance requirements.

The proposal is ready for the owner's review. Implementation, phone results and Q11/Q12 remain future work under the stated plan.


## Owner-answer update — 8 October 2026

Read OWNER-ANSWERS.md and folded all ten answers into M3-ARCHITECTURE.md, M3-PLAN.md and QUESTIONS.md; each question has its answer date. Approved values now cover phased acceptance, 250 m areas, climate/dry-spell definitions, one woolly mammoth species, start margins, 180/60-second targets with 198/66 outer limits, the 2 GiB working line, short-year units and flint/chert naming. The earlier research report and critic report remain historical evidence; this update and the answered questions supersede their pending-decision wording.

The changed eclipse requirement is 3–8 visible solar or lunar eclipses at every place over 70 game years regardless of cloud, without promising total solar eclipses everywhere. The orbit-family check found no structural conflict: its 15-day phase is fixed while inclination, node precession and apparent sizes remain adjustable. This is not a demonstrated fit; no concrete parameter set has yet passed the spatial proof. The architecture and α3.6a/G5 retain that obligation and require any failure to be reported without changing the owner's range.

All prior critic findings remain corrected. Checked the updated decision wording, eclipse bounds and unchanged task IDs. No new research, builds, subagent work or repository edits were needed.
