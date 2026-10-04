# Research 00: how game teams work, and the guide Kindling follows

**Question:** how do real teams take a game from an idea to a finished build?
How should Kindling follow them, given one owner, AI builders, and a game this ambitious?

## What real teams do

### The phases

Studios split a game into **concept, pre-production, production, and the finishing stages** (alpha, beta, release) ([Game-Ace](https://game-ace.com/blog/game-development-stages/), [RocketBrush](https://rocketbrush.com/blog/game-development-process-guide)).

- **Pre-production and production are different kinds of work.**
  - Mark Cerny, who shaped how Sony's studios work, calls them "as different as night and day".
    Pre-production is where a team tries to "capture lightning", and it cannot be put on a timeline ([VGC](https://www.videogameschronicle.com/features/who-is-mark-cerny/)).
    Production builds what pre-production proved.
  - In his words, "pre-production is about concentrating all that work that gets tossed out into the very first few months of the game" ([Game Developer](https://www.gamedeveloper.com/marketing/conversations-from-gdc-europe-mark-cerny-jonty-barnes-jason-kingsley)).
  - Pre-production ends with a "publishable first playable": a polished piece of the game that decides whether the project lives or dies ([Wikipedia: Mark Cerny](https://en.wikipedia.org/wiki/Mark_Cerny), [VGC](https://www.videogameschronicle.com/features/who-is-mark-cerny/)).
- **Clinton Keith's version**, from his book's chapter on the project life cycle, as quoted in [course notes](https://cs.ccsu.edu/~stan/classes/CS415/notes/06-AgileProjects.html):
  - pre-production teams "explore what is fun and how they are going to build assets to support it during production", making levels and assets of production quality;
  - production builds the full game from what was discovered, and "focuses on efficiency and incremental improvements";
  - both run in short iterations.
- **The finishing stages** ([Wayline](https://www.wayline.io/blog/setting-game-development-milestones-concept-launch), [Bugnet](https://bugnet.io/blog/milestones-for-indie-game-projects)):
  - a first playable;
  - alpha: every feature in, the game playable start to finish;
  - beta: every piece of content in, bugs, balance and speed fixed;
  - release.

### Prototypes and the vertical slice

- **Prototypes answer whether you should make the game; the vertical slice proves you can.**
  This is Rami Ismail's distinction ([Rami Ismail](https://ltpf.ramiismail.com/prototypes-and-vertical-slice/)):
  - prototypes are "fast, cheap, and precise", one per question, thrown away;
  - "don't allow things that aren't needed to answer questions in".
- **Prototype the riskiest system first.**
  A technical spike is a short, time-boxed test of the greatest uncertainty, hard-coded and deleted afterwards ([Youngju](https://www.youngju.dev/blog/2026-06-30-fde-throwaway-prototypes.en), [Bugnet](https://bugnet.io/blog/what-is-a-game-prototype)).
  A working prototype proves a game can be fun, not that it can be built and finished ([Alexitsios](https://alexitsios.substack.com/p/why-a-working-prototype-doesnt-mean)).
  This is older than games: Boehm's spiral model of software puts "a series of prototypes aimed at risk reduction" first, before requirements, design and code ([Osterweil 2011](https://web.cs.umass.edu/publication/docs/2011/UM-CS-2011-023.pdf)).
- **The vertical slice is a production prototype:**
  - one of each thing, at shipping quality, so the whole creation cycle runs once;
  - it finds the pipeline's problems;
  - then make a second of each thing: "the time it takes you to make the second thing is the time you can divide your development timeline by" ([Rami Ismail](https://ltpf.ramiismail.com/prototypes-and-vertical-slice/)).
- **Its cost is real:** months of work, and much of the game's effort hidden in a small piece of content ([Unity discussions](https://discussions.unity.com/t/mvp-vs-vertical-slice/632748)).
  Clinton Keith would rather call it a "game increment", each adding real value.
  He likens it to portrait painters who finished the head first, the main risk, before the rest ([Game Developer](https://www.gamedeveloper.com/design/why-we-should-stop-saying-vertical-slices-)).

### The documents

- **The design document** says what the game is ([WPI course slides](https://web.cs.wpi.edu/~imgd1001/a08/slides/imgd1001_05_GameDesignDocs.pdf)).
- **The technical specification** says how it is built: "the architectural vision; technology to be used".
  It must include ([WPI course slides](https://web.cs.wpi.edu/~imgd1001/a08/slides/imgd1001_05_GameDesignDocs.pdf), [Game Design Skills](https://gamedesignskills.com/game-design/technical-design-document/)):
  - tooling;
  - the art, music, sound and production pipelines;
  - platform issues;
  - the key areas of technical risk.
- **The art bible fixes the look.**
  Four public art bibles share five things ([Makko](https://blog.makko.ai/what-public-game-art-bibles-say/)), detailed in research 05:
  - a tie-breaker rule when rules conflict ("gameplay trumps realism");
  - a light direction stated without ambiguity;
  - the shadow colour as an actual value;
  - a smallest detail worth making, tied to a real object ("anything smaller than a human hand should not be modeled");
  - a named list of common mistakes.
- **Pipelines are defined before production,** with acceptance criteria at each stage, so nothing is reworked late ([Game Developer](https://www.gamedeveloper.com/blogs/what-you-should-take-out-of-pre-production)).
- **Documents stay short and current;** keeping them so is part of finishing a task.

### What goes wrong

A study of 155 published postmortems found the same few kinds of mistake in game after game ([Washburn et al., ICSE 2016](https://thomas-zimmermann.com/publications/files/washburn-icse-2016.pdf)).

| What went wrong | Share of postmortems |
|---|---|
| Obstacles, often in newly formed teams | 37% |
| Schedules: optimistic estimates, overlooked work | 25% |
| The development process, mostly too little planning up front ("everybody was carrying a slightly different picture in his head of what the final game would be") | 24% |
| Game design, mostly overambition: "most of the individual features seemed doable, but added up they represented a tremendous amount of work" | 22% |

What went right most often:
- game design with a clear vision (50%);
- the team (40%);
- art (39%).

### How games like ours were made

- **RimWorld:** Tynan Sylvester made a series of quick prototypes of different games.
  He kept the colony game after friends played it until they were falling asleep and would not leave ([Wikipedia: RimWorld](https://en.wikipedia.org/wiki/RimWorld), [DualShockers](https://www.dualshockers.com/rimworld-developer-game-different/)).
  It then grew in public, in early access, from 2013 to 2018.
- **Factorio:** begun by one programmer in 2012, crowdfunded in 2013, in early access from 2016, released in 2020.
  The team wrote a public development note every Friday ([Factorio](https://factorio.com/presskit), [Wikipedia: Factorio](https://www.wikipedia.org/wiki/Factorio)).
- **Dwarf Fortress:** grown since 2002 by "arcs", each a set of features around one theme, against a design written out in full on paper.
  "When we finish the paper, that's 1.0", says Tarn Adams ([Dwarf Fortress wiki](https://dwarffortresswiki.org/index.php/Arc), [Game Developer](https://www.gamedeveloper.com/business/dwarf-fortress-in-2013)).
- **Valve's Team Fortress 2:** its art style was chosen for a purpose, readability, so players can "read" the scene in any light.
  This was "stylization with a purpose", written down before production ([Valve, NPAR 2007](https://www.cs.princeton.edu/courses/archive/fall07/cos597B/papers/mitchell-team-fortress.pdf), [GDC 2008](https://cdn.steamstatic.com/apps/valve/2008/GDC2008_StylizationWithAPurpose_TF2.pdf)).

## What went wrong in our earlier attempts

- **The first plan** wrote 8,600 lines of tasks for two years before anything was proven.
  It had no prototypes, no vertical slice and no art bible, so nothing measured the pictures against your references.
- **The second plan** (3 October) went bottom up, as you asked.
  But it still skipped what this guide says comes first: a prototype of every risky system, a full art bible, and a slice that proves the whole pipeline at the final quality.
  It checked only one risk (the renderer), and checked Godot's ability to do the game hardly at all (now research 01).

## The guide Kindling follows

1. **Pre-production ends with proof, not paper.**
   Its deliverables:
   - the design document (`PROJECT.md`);
   - this research;
   - the art bible;
   - the technical design (`ARCHITECTURE.md`), with its pipelines and risks;
   - a prototype of every risky system;
   - a vertical slice.
2. **A prototype for each risk, riskiest first,** each answering one question on your phone or in the cloud, then thrown away.
   The risks found by this research:
   - the look on Godot's Mobile renderer;
   - the frame time of a full scene;
   - the zoom from a person to the globe;
   - thousands of minds at speed on the phone;
   - the same bits on phone and cloud;
   - world generation time;
   - the phone's writer;
   - whether discovery and teaching produce a believable pace at all (the sharp-stone question, `RSK-01`).

   The last is a design prototype: it asks whether the game should be made the way `PROJECT.md` describes.
3. **A vertical slice before production:** one small piece of the real game, at the final look, on your phone, built in the real architecture.
   An example: a band at a cliff camp through a day, drawn and lit as the art bible says.
   It proves the whole pipeline once, sets the quality bar you judge, and its second-of-everything measures the true pace.
4. **Production goes bottom up, as you asked:** foundations, the graphics engine, the world, living nature, people, minds, crafts, culture, and the game.
   Each layer is built properly, using what the prototypes proved, and grows the slice into the game.
5. **Plan in detail only the next milestone.**
   Later ones stay outlines until their turn, re-estimated from the measured pace.
6. **Short steps, each ending in a build on your phone,** reviewed by the builder.
   An independent review closes each numbered alpha.
7. **A short report at each milestone's end, which includes what went right and wrong:** the postmortem habit, so mistakes are not repeated.
8. **Scope is guarded.**
   Each milestone lists its must-haves.
   Anything that grows beyond them is cut or moved, with your OK, so features that each "seemed doable" do not add up to an unfinishable game.
9. **Documents stay lean and current:**
   - the design document says what;
   - the art bible says how it looks;
   - the architecture says how it is built and why;
   - the plan says in what order;
   - the code says where.

## Sources

- Phases:
  - [Wikipedia: Mark Cerny](https://en.wikipedia.org/wiki/Mark_Cerny)
  - [VGC: who is Mark Cerny](https://www.videogameschronicle.com/features/who-is-mark-cerny/)
  - [Game Developer: Mark Cerny at GDC Europe](https://www.gamedeveloper.com/marketing/conversations-from-gdc-europe-mark-cerny-jonty-barnes-jason-kingsley)
  - [Course notes quoting Clinton Keith's Agile Game Development](https://cs.ccsu.edu/~stan/classes/CS415/notes/06-AgileProjects.html)
  - [Game-Ace](https://game-ace.com/blog/game-development-stages/)
  - [RocketBrush](https://rocketbrush.com/blog/game-development-process-guide)
  - [Wayline](https://www.wayline.io/blog/setting-game-development-milestones-concept-launch)
  - [Bugnet](https://bugnet.io/blog/milestones-for-indie-game-projects)
- Prototypes and slices:
  - [Rami Ismail](https://ltpf.ramiismail.com/prototypes-and-vertical-slice/)
  - [Clinton Keith: stop saying vertical slices](https://www.gamedeveloper.com/design/why-we-should-stop-saying-vertical-slices-)
  - [Unity discussions](https://discussions.unity.com/t/mvp-vs-vertical-slice/632748)
  - [Youngju: throwaway prototypes](https://www.youngju.dev/blog/2026-06-30-fde-throwaway-prototypes.en)
  - [Bugnet: what is a prototype](https://bugnet.io/blog/what-is-a-game-prototype)
  - [Alexitsios](https://alexitsios.substack.com/p/why-a-working-prototype-doesnt-mean)
  - [Osterweil: a process programmer looks at the spiral model (2011)](https://web.cs.umass.edu/publication/docs/2011/UM-CS-2011-023.pdf)
- Documents:
  - [WPI design document slides](https://web.cs.wpi.edu/~imgd1001/a08/slides/imgd1001_05_GameDesignDocs.pdf)
  - [Game Design Skills: technical design document](https://gamedesignskills.com/game-design/technical-design-document/)
  - [Makko: four public art bibles](https://blog.makko.ai/what-public-game-art-bibles-say/)
  - [Game Developer: what to take out of pre-production](https://www.gamedeveloper.com/blogs/what-you-should-take-out-of-pre-production)
- [Washburn et al.: 155 postmortems (ICSE 2016)](https://thomas-zimmermann.com/publications/files/washburn-icse-2016.pdf)
- Games:
  - [Wikipedia: RimWorld](https://en.wikipedia.org/wiki/RimWorld)
  - [DualShockers](https://www.dualshockers.com/rimworld-developer-game-different/)
  - [Factorio press kit](https://factorio.com/presskit)
  - [Wikipedia: Factorio](https://www.wikipedia.org/wiki/Factorio)
  - [Dwarf Fortress wiki: arcs](https://dwarffortresswiki.org/index.php/Arc)
  - [Game Developer: Dwarf Fortress in 2013](https://www.gamedeveloper.com/business/dwarf-fortress-in-2013)
  - [Valve: illustrative rendering in TF2](https://www.cs.princeton.edu/courses/archive/fall07/cos597B/papers/mitchell-team-fortress.pdf)
  - [Valve: stylization with a purpose](https://cdn.steamstatic.com/apps/valve/2008/GDC2008_StylizationWithAPurpose_TF2.pdf)
