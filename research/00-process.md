# Research 00: how game teams work

**Question:** how should Kindling be planned and built, the way a real studio would?

## What real teams do

- **Pre-production comes first, and ends with proof, not paper.**
  Its deliverables are the design document, an art bible, a technical specification, prototypes and a vertical slice, and it usually takes 2 to 6 months for a team led by a designer, an art director, a tech lead and a producer ([Game-Ace](https://game-ace.com/blog/game-development-stages/), [RocketBrush](https://rocketbrush.com/blog/game-development-process-guide)).
- **Prototype each risky system separately before building the game.**
  "You should NOT start by simply creating the actual game from the get-go": each core system gets its own prototype, to find what works before full production ([Game Developer](https://www.gamedeveloper.com/blogs/what-you-should-take-out-of-pre-production)).
- **The vertical slice proves the quality bar.**
  It is one small part of the game at near-final quality: art, play and systems together, proving the team can reach the intended look, feel, technical quality and frame rate ([Game Developer](https://www.gamedeveloper.com/blogs/what-you-should-take-out-of-pre-production), [HacknPlan](https://hacknplan.com/blog/understanding-vertical-slicing)).
- **Pipelines are defined before production.**
  How each asset moves from concept to model to animation to the game, with acceptance criteria at each stage, so nothing is reworked late ([Game Developer](https://www.gamedeveloper.com/blogs/what-you-should-take-out-of-pre-production)).
- **An art bible fixes the look.**
  Reference images, a colour palette with each colour's role, light direction and shadow colour, proportions, level of detail, technical limits, and a list of the mistakes to avoid ([Makko](https://blog.makko.ai/what-public-game-art-bibles-say/), [GameDev.net](https://gamedev.net/forums/topic/644435-what-is-an-art-bible-and-any-tips-to-make-one-better/)).
- **Production runs in short iterations toward milestones.**
  Most studios use agile sprints with feedback loops rather than a fixed waterfall plan; milestones such as first playable, alpha (feature-complete) and beta mark the big steps ([Unity Learn](https://learn.unity.com/course/game-design-curricular-framework-resources/tutorial/why-agile), [UCSC CMPM 171](https://cmpm171-winter18-01.courses.soe.ucsc.edu/node/2.html)).
- **Documentation lives with the work.**
  Keeping the documents up to date is part of finishing a task, and they stay short enough to be read ([Game Developer](https://www.gamedeveloper.com/blogs/what-you-should-take-out-of-pre-production)).

## What went wrong in our old plan

- It wrote 8,600 lines of detailed tasks for two years of work before anything had been proven, so a change of direction makes most of it waste.
- It never had a vertical slice or an art bible, so nothing measured the pictures against the look you wanted.
- It built thin horizontal layers in a fixed order: ground first, objects much later.
  So the first 4 alphas could only ever show ground.

## What we take

1. **Pre-production first.**
   Research (these notes), the art bible, the technical design and a few short prototypes on your phone, before any production code.
2. **Bottom-up production, as you asked**: the foundations and data structures, then the graphics engine, then world generation, then living things, people, minds, culture, and last the game on top.
   Each layer ends with something you can open on your phone.
3. **A quality gate at the graphics layer.**
   Its milestone is a small scene that matches your reference pictures, judged by you, before anything is built on top of it.
   It plays the role of the vertical slice for the look.
4. **Plan in detail only the next milestone.**
   Later milestones stay an outline: their goal, the project items they deliver, and what you will see.
   Each gets its detailed tasks when it starts, from what the earlier ones taught us.
5. **Short sprints inside a milestone**, each ending in a build on your phone, with the builder's review and, at the milestone's end, an independent review.
6. **Lean documents.**
   The project file says what (it is the design document), the art bible how it looks, the architecture how it is built and why, with its sources, and the plan in what order.
   The code says where.
