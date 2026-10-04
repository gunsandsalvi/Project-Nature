# Research 10: minds

**Question:** how do games and researchers make thousands of people choose believably, feel, remember, know and mis-know, get along and fall out, and find their way?
How do they do it while staying explainable and fast enough for your phone (`MND`, `PRN-01`, `PRN-13`, `MND-14`, `MND-15`, `TIM-07`)?

## How others do it

### Choosing

- **Utility AI, from The Sims:**
  - each possible action is scored by how well it meets the person's needs now, and the choice is drawn among the best;
  - objects advertise what they offer (research 09);
  - The Sims 3 added preferences, so different people react differently.

  Dave Mark's Infinite Axis Utility System (GDC 2015) makes the scoring data: each consideration is a response curve, and new behaviour needs little code ([Wikipedia: utility system](https://en.wikipedia.org/wiki/Utility_system), [GameAI.com](https://gameai.com/iaus.php)).
- **Planning:**
  - GOAP (Jeff Orkin, F.E.A.R.) chains actions toward a goal.
  - HTN planners break tasks into fixed sub-tasks, more predictable.
    They were used in Horizon Zero Dawn, Killzone 2 and 3 and Transformers: Fall of Cybertron, after Troy Humphreys' "Exploring HTN Planners through Example" ([Fluid HTN](https://awesome.ecosyste.ms/projects/github.com%2Fptrefall%2Ffluid-hierarchical-task-network), [Unity forum](https://discussions.unity.com/t/released-fluid-hierarchical-task-network-planner-ai-free/740036)).
  - Behaviour trees react well but plan badly, and planners the reverse, so games combine them ([Davide Aversa](https://davideaversa.it/blog/choosing-behavior-tree-goap-planning/)).
- **At scale:**
  - AI budgets are typically 1 to 4 ms of a 16.6 ms frame;
  - decisions are made less often than frames (every 0.25 s by default in one utility toolkit, about human reaction time);
  - deciding is spread over frames in batches, so 500 agents in batches of 20 take 25 frames;
  - batched, vectorised scoring is meant for 1,000 to 10,000 characters ([Utility Intelligence docs](https://uintel-go.utilityworlds.com/Documentation/TipsAndTricks/OptimizationTricks/), [GameAI.Net](https://www.nuget.org/packages/GameAI.Net/0.1.7)).

### Feeling

- **The OCC model** (Ortony, Clore and Collins) distinguishes 22 emotions as reactions to events, to others' actions, and to objects:
  - is it desirable;
  - did it happen to me or to another;
  - did it happen, or is it expected?

  FAtiMA built this into autonomous game characters, first for a serious game about bullying ([arXiv: emotion engines for NPCs](https://arxiv.org/pdf/2307.10031), [FAtiMA](https://www.doi.org/10.1007/978-3-319-12973-0_3), [AAAI](https://cdn.aaai.org/AAAI/2007/AAAI07-021.pdf)).
- **RimWorld's mood:**
  - thoughts ("Hungry" −6, "Ate fine meal" +5) add up from a base;
  - mental breaks risk starting below 35%, 20% and 5%, the lines moved by traits such as Steadfast or Nervous;
  - thoughts stack with limits ([RimWorld wiki: mood](https://rimworldwiki.com/wiki/Mood), [mental break](https://rimworldwiki.com/wiki/Break), [thoughts](https://rimworldwiki.com/wiki/Thoughts)).
- **Dwarf Fortress** ([DF wiki: thoughts](https://dwarffortresswiki.org/Thoughts_and_preferences), [memory](https://dwarffortresswiki.org/Memory_(thought))):
  - turns experiences into thoughts with emotions of some strength;
  - turns those into short-term, long-term and core memories;
  - lets core memories change personality, with a date.

### Knowing, telling and mis-knowing

- **Talk of the Town** (James Ryan and others, AIIDE) simulates a farming town from 1839 to 1979, paying "particular attention to what characters know".
  Characters "observe, tell, misremember, and lie" ([AIIDE paper](https://ojs.aaai.org/index.php/AIIDE/article/view/12825), [PDF](https://cdn.aaai.org/ojs/12825/12825-52-16341-1-2-20201228.pdf), [Emily Short on Ryan's thesis](https://emshort.blog/2019/05/28/curating-simulated-storyworlds-james-ryan-ch-6f/)):
  - a belief forms from evidence seen, from a report (true or false), or from confabulation out of related beliefs;
  - salient details are noticed and remembered more;
  - repeating a belief aloud commits the speaker to it.

  It is the closest precedent for `MND-23` (who knows what) and `CUL-24` (talk).
- **Lyra** (Azad and Martens, AIIDE 2019) simulates characters debating, their opinions and biases spreading, and groups of like minds forming ([AIIDE](https://ojs.aaai.org/index.php/AIIDE/article/view/5232)).
  It is a model for moving opinions (`MND-33`).

### Getting along

- **Comme il Faut**, behind Prom Week, writes social norms and acts as reusable rules: a shy person is less likely to be outgoing, and someone you were mean to is less likely to be nice ([AIIDE](https://ojs.aaai.org/index.php/AIIDE/article/view/12454)).
- **Versu** (Richard Evans, The Sims 3's AI lead) lets relationships nobody wrote arise from a social model ([Versu](https://modemworld.me/2013/07/10/versu-making-npcs-human/)).
- **Bad News** (Ryan and others, CHI 2016) runs a whole town's simulated lives as the ground for play ([PDF](https://eis.ucsc.edu/papers/ryanEtAl_BadNewsCHI2016.pdf)).

### Finding the way

- **HPA\*** splits the map into linked clusters: about 10 times faster for a 1% longer path ([Botea et al.](https://webdocs.cs.ualberta.ca/%7emmueller/ps/hpastar.pdf)).
- **Flow fields** (Supreme Commander 2, Elijah Emerson in Game AI Pro): paths between sectors first, then flow-field tiles inside them.
  They move "hundreds to thousands" of agents without rebuilding a path for each ([Game AI Pro 360](https://www.taylorfrancis.com/books/9780429055096/chapters/10.1201/9780429055096-8), [GameDev.net](https://gamedev.net/forums/topic/701267-flow-field-architecture-advice)).
- **Songs of Syx** caches paths between clusters.
  **Dwarf Fortress** tracks connected regions to reject impossible trips at once, and its "FPS death" comes mostly from pathfinding (research 03).
- **Godot's own navigation** is costly with many agents (research 01).

## A first estimate for your phone

- `TIM-07` asks 1,000 people to run at least 1 game year a real minute.
  That is 60 game days a minute, so one game day a second.
- If a person decides on average every 30 game minutes, when an activity ends (`TIM-17`), that is about 48,000 decisions a second.
- On four cores, that is about 12,000 per core: some 80 µs each, before bodies, talk and paths.
- A data-oriented utility scoring of some 50 known actions with a few considerations each fits in a few microseconds.
- So the target is plausible.
  But it must be measured on the phone at held speed, with memory, talk and paths included, before production (`MND-15`).

## What we take

1. **Choosing by utility, as The Sims and IAUS do:**
   - every known action is scored by data-driven response curves against needs, personality, mood, plans and beliefs;
   - the choice is drawn by keyed chance among the best;
   - the top reasons are kept for the card (`MND-09`, `PRN-13`).
2. **A small HTN planner on top** for multi-step jobs, each step re-checked by utility, so people still react to a wolf.
3. **Decisions only when an activity ends or something interrupts it** (`TIM-17`), never every tick.
   Thinking is spread over threads in fixed chunks (research 03).
4. **Feelings by appraisal, OCC-style;** mood as summed thoughts with trait-moved thresholds, RimWorld-style.
   Memories at three depths that can change personality with a date, Dwarf Fortress-style (`MND-19`, `MND-29`, `MND-30`, `MND-18`).
5. **Knowledge per person with its source:** seen, told by whom, worked out.
   Beliefs can be wrong, misremembered or spread by talk, Talk of the Town-style.
   Choices read only this, never the world's truth (`PRN-01`, `MND-02`, `MND-23`).
6. **Opinions move in talk,** Lyra-style.
   Social acts and norms are data rules over relationships and personality, Comme il Faut-style (`MND-33`, `CUL-24`).
7. **Pathfinding in levels:** connected regions, then cluster paths cached, then A\* or flow fields inside clusters.
   It is recomputed only where the land changes.
8. **A prototype before production:** a thousand simple minds with needs, utility choice, talk and paths, on your phone at held speed.
   It checks the estimate above.

## Sources

- Choosing:
  - [Wikipedia: utility system](https://en.wikipedia.org/wiki/Utility_system)
  - [GameAI.com: IAUS](https://gameai.com/iaus.php)
  - [Fluid HTN](https://awesome.ecosyste.ms/projects/github.com%2Fptrefall%2Ffluid-hierarchical-task-network)
  - [Unity forum: Fluid HTN](https://discussions.unity.com/t/released-fluid-hierarchical-task-network-planner-ai-free/740036)
  - [Davide Aversa](https://davideaversa.it/blog/choosing-behavior-tree-goap-planning/)
  - [Utility Intelligence: optimisation](https://uintel-go.utilityworlds.com/Documentation/TipsAndTricks/OptimizationTricks/)
  - [GameAI.Net](https://www.nuget.org/packages/GameAI.Net/0.1.7)
- Feeling:
  - [arXiv: emotion engines](https://arxiv.org/pdf/2307.10031)
  - [FAtiMA](https://www.doi.org/10.1007/978-3-319-12973-0_3)
  - [AAAI 2007](https://cdn.aaai.org/AAAI/2007/AAAI07-021.pdf)
  - [RimWorld wiki: mood](https://rimworldwiki.com/wiki/Mood)
  - [mental break](https://rimworldwiki.com/wiki/Break)
  - [thoughts](https://rimworldwiki.com/wiki/Thoughts)
  - [DF wiki: thoughts](https://dwarffortresswiki.org/Thoughts_and_preferences)
  - [DF wiki: memory](https://dwarffortresswiki.org/Memory_(thought))
- Knowing:
  - [Talk of the Town (AIIDE)](https://ojs.aaai.org/index.php/AIIDE/article/view/12825)
  - [PDF](https://cdn.aaai.org/ojs/12825/12825-52-16341-1-2-20201228.pdf)
  - [Emily Short on Ryan](https://emshort.blog/2019/05/28/curating-simulated-storyworlds-james-ryan-ch-6f/)
  - [Lyra](https://ojs.aaai.org/index.php/AIIDE/article/view/5232)
- Getting along:
  - [Comme il Faut](https://ojs.aaai.org/index.php/AIIDE/article/view/12454)
  - [Versu](https://modemworld.me/2013/07/10/versu-making-npcs-human/)
  - [Bad News](https://eis.ucsc.edu/papers/ryanEtAl_BadNewsCHI2016.pdf)
- Paths:
  - [HPA*](https://webdocs.cs.ualberta.ca/%7emmueller/ps/hpastar.pdf)
  - [Emerson: flow field tiles](https://www.taylorfrancis.com/books/9780429055096/chapters/10.1201/9780429055096-8)
  - [GameDev.net: flow fields](https://gamedev.net/forums/topic/701267-flow-field-architecture-advice)
