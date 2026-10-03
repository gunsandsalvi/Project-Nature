# Research 07: people's minds

**Question:** how do other games and researchers make thousands of people choose believably, remember, get along and fall out, find their way, and discover and lose crafts (`MND`, `PRN-01`, `PRN-13`), while staying explainable and fast?

## How others do it

- **Utility AI, from The Sims.**
  Each possible action scores how well it would satisfy the person's needs right now, and the choice is drawn among the best scores.
  - The Sims 3 extended needs with preferences, so different people react differently.
  - Dave Mark's Infinite Axis Utility System (GDC 2015) makes the scoring data-driven, needing little programming once wired in ([Wikipedia: utility system](https://en.wikipedia.org/wiki/Utility_system), [GameAI.com](https://gameai.com/iaus.php)).
- **Planning.**
  - GOAP (F.E.A.R., Jeff Orkin) chains actions toward a goal, flexible for agents with many actions.
  - HTN breaks tasks into fixed sub-tasks, simpler and more predictable.
  - Behaviour trees react well but plan badly; planners plan well but react slowly, so games combine them ([Davide Aversa](https://davideaversa.it/blog/choosing-behavior-tree-goap-planning/)).
- **Dwarf Fortress minds:**
  - experiences become thoughts with emotions of some strength;
  - thoughts become short-term, long-term and core memories, and core memories can change personality ("can easily fall in love … after gaining a sibling in 351");
  - personality is a set of facets, values, preferences and needs, all shown on a tab ([DF wiki: thoughts](https://dwarffortresswiki.org/Thoughts_and_preferences), [memory](https://dwarffortresswiki.org/Memory_(thought))).
- **Social AI.**
  - Comme il Faut, behind Prom Week, writes social norms and interactions as reusable rules: a shy person is less likely to be outgoing, and someone you were mean to is less likely to be nice ([AIIDE paper](https://ojs.aaai.org/index.php/AIIDE/article/view/12454)).
  - Versu, by The Sims 3's AI lead Richard Evans, lets relationships nobody wrote arise from a social model ([Versu](https://modemworld.me/2013/07/10/versu-making-npcs-human/)).
- **Pathfinding at scale.**
  - HPA* splits the map into linked clusters, about 10 times faster for a 1% longer path ([Botea et al.](https://webdocs.cs.ualberta.ca/%7emmueller/ps/hpastar.pdf)).
  - Songs of Syx caches paths between clusters.
  - Dwarf Fortress keeps track of connected regions to reject impossible trips at once (research 02).
- **Cultural evolution research.**
  - Agent-based models show that small populations lose skills by chance, as Henrich's model of Tasmania's losses shows.
  - Innovation spreads faster when its payoff is uncertain, and small groups can act as "cultural incubators" ([JASSS](https://www.jasss.org/18/4/8.html), [CoMSES](https://catalog.comses.net/publications/24759), [PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC3404092)).
  - This is exactly the dynamic the project wants for discovery, teaching and lost crafts (`MOM-02`, `RES-03`).

## What we take

1. **Choosing by utility, as The Sims and IAUS do.**
   - Every action a person knows how to do is scored against their needs, personality, mood, plans and beliefs.
   - The choice is drawn by keyed chance among the best (`PRN-12`).
   - The top reasons are kept with the choice, so the card can say why (`PRN-13`).
   - The scoring curves are data, tunable without code.
2. **A small planner on top, HTN style**, for multi-step jobs (fetch flint, knap, haft), re-checked by utility at each step so people still react to a wolf.
3. **Dwarf Fortress-style feelings and memories:**
   - experiences become feelings;
   - feelings become memories at three depths;
   - the deepest memories can change personality, and each change is dated, so a life story can say why someone changed.
4. **Knowledge is per person (`PRN-01`, `MND-02`):**
   - each person holds what they have seen, been told or worked out, with where it came from;
   - choices only ever read that knowledge, never the world's truth;
   - a test can trace every discovery's route.
5. **Social rules in the Comme il Faut way:** norms and social acts as data rules over relationships and personality, so new customs are data, and relationships nobody wrote can arise (`CUL`).
6. **Pathfinding in levels:**
   - connected regions reject impossible trips at once;
   - paths between clusters are cached, and A* runs only within a cluster;
   - everything is recomputed only where the land changed.
7. **Discovery and teaching follow the research.**
   Skills spread by imitation and teaching, and can be lost in small bands by chance.
   The scene tests check these patterns against the pass rules (`RES-03`).
