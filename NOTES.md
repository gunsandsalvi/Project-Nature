# Kindling: notes for production

What pre-production has shown that production must take: your observations and mine, written down as the prototypes go.
At α0.8a each note goes into the architecture or the vertical slice's plan (`IMPLEMENTATION.md`), or comes to you first if it would change `PROJECT.md`.

## The look

### The ground shows what covers it (you, 5 October 2026, on P8's camp)

- The ground's colours must not just appear, green here and brown there.
  Green is there because grass grows there, yellow because it is sand, brown because the earth is bare or trodden.
- Everything crisp, clear and detailed, as in your picture of a pixel-art river glade: every plant, stone, stump and animal reads on its own.
  Each has a clean shape, a few colours of its own and a dark edge; nothing is a smear or a soft blob.
- All the textures will be made again from scratch for production: P8's colours, ramps and patterns are stand-ins.

What I take from it, for your OK at α0.8a:
- **Each place is drawn as what covers it:** grass as blades, tufts and flowers; sand as sand; mud, gravel, stones and fallen leaves as themselves.
  Its colour follows from its cover, and a change of colour is always the cover changing, such as grass thinning onto a path, or sand drifting into grass.
- **Each cover at every zoom:** up close as things (tufts, pebbles); further out as marks in the ground's texture (dots, strokes); from far as its colour.
  One rule decides the cover, and everything that draws the ground reads it, as P8 already does for trees (A8.3).
- **A crispness check for every asset** at its smallest size on screen: a readable shape, its own colours and its outline.

### Your pictures for inspiration (5 October 2026)

- **A planet game:** vibrant, with details you can see, 3D clouds, biomes that differ, forests, deserts and water that moves.
  Not to be copied.
- **A pixel-art river glade:** dense, layered greens of many kinds of plant, such as reeds, ferns, bamboo and shrubs with coloured tips.
  Clear water over dark depths, with stepping stones, ripples, fish and a small waterfall.
  A bare-earth path with cracks and darker patches, and animals everywhere: deer, herons, a crab, a frog, a squirrel, ducks.

### P8's camp against the art book (me, 5 October 2026)

- **Grass at the person stop:** the art book's tufts are about 0.4 m across and up to 0.45 m tall, one every 2 or 3 m², with darker ones among them.
  P8's are about a quarter of that size, and sparser.
- **Grass at the close camp:** the art book shows grass as specks of a pixel or two, flowers as coloured dots, and reeds 6 to 12 pixels tall along the banks.
  P8 draws small plants only in the nearest ground, so its close camp shows none, and its river no reeds.
  Production draws them wherever they would stand at least a pixel tall, and as marks in the ground's texture below that.
- **The meadow at the camp stop:** the art book's is finely mottled, with small dark bushes and dots of tufts; P8's patches are too big and flat.
- **The camp's trodden floor:** P8's darker patches on it read as shadows.
- **The hearth's stones** read as blue spikes.
- **The band** crowds the hearth; the art book spreads people over the camp at their work.
- **Not yet in the zoom:** outlines and lit edges (`PRE-21`), and smoke over the hearth.
  Dusk and night at the close stops are not yet compared with the art book.

## Living things

### What P9's ecology taught (me, 5 October 2026)

- **Numbers that hold, but move too little in mild country:** in 20 worlds left alone for 100 years every species stayed within 0.66 and 1.10 of its settled total, while round the start regions numbers crashed in droughts and hard winters to a fifth and boomed to twice.
  In forests with mild winters they barely moved.
  Production adds what moves them there within `WLD-18`'s rules: hunters that can starve when their prey thins, and cycles like the hare's and the lynx's.
- **A game winter must cost an animal what a real one does,** though it lasts 20 game days, or the weather moves nothing.
- **Dry country's plants live on less water:** measured against a meadow's needs, they starved the animals there in every normal year.
- **Young animals leave crowded land for emptier land next door,** from every cell, or land emptied by a hard year stays empty.
- **A world made from a rough start drifts for decades,** so production makes it directly near its balance (`WLD-08`).
- **Not yet tried:** fish, birds and bears, which need rivers, seas and seasons of their own; and the 1 km cells with herds that `WLD-32` describes, since P9 ran on cells of about 4 km.

## Culture

### What P10 taught (me, 5 October 2026)

- **Superstitions keep themselves with `MND-05`'s numbers:** a hit adds 15 and a miss takes 5, so an act done before hunts that succeed three times in ten sustains itself once believed.
  P10 held it in bounds by letting only those an outcome befalls link it, so a band credits an act only through talk.
  Production should measure how often a credited act becomes a rite, beside `CUL-34`'s done-when for the hunting song.
- **Talk lends conviction, it doesn't pile it up:** a belief told again and again must not grow past what the teller lends, or every told belief lives forever.
- **"A rite a band keeps" needs a test of keeping;** P10 used a year of being credited, or done as the band's way.
- **Customs come within months** from the commonest cases, big kills, and the way with the dead within a year or two at forager death rates; `CUL-33` now gives them a year, with your OK.
- **Realistic rates matter:** hunting deaths at ten times the real rate flooded the bands with sudden deaths, spirits of the aurochs and burial cases.

## The director

### What P11 taught (me, 5 October 2026)

- **Signs must earn their slowdowns:** in these worlds a hunch tried again ended in its discovery 3 times in a thousand, a storm over a camp in lightning there 13 times in a thousand, and one you follow hurt or ill in their death never.
  Scored at what their end would be worth, they took four slowdowns in five and almost none came true.
  Production scores each sign by how often it comes true, measured in the pace tests, so time slows before an outcome only for strong signs, such as a predator stalking or hostile groups meeting.
- **Accidents give no warning:** most first flakes come by accident, so they are caught as they happen, never before.
- **The list fills fast:** about 200 moments an hour at the globe, two thirds in a world's first 20 years, when every custom, rite, spirit and cause of death is a first, and later mostly a band losing its last fire and births to those you follow.
  Production gathers repeats into one line (the same band's lost fires), ranks the list by score, and may score a kind lower each time it recurs.
- **A watch opens rested:** without it, a minor first took the first slowdown seconds before the world's first sharp flake; with the bar at its rest height at the start, world-first discoveries slowed for rose from 20 to 31 of 60.
- **A first needs its kind spelled out:** a custom by its answer, a spirit by its being (not each of the dead), a death by its cause, a thing by what it is; otherwise every death is a first.
- **The log is the director's only window:** each event with its day and hour, kind, who and what, an activity logged at its start and its result at its end, so a sign can come before what it foretells; and what the director needs to know, such as who can make what, it keeps from what it has seen.
- **One stream of chance can hide a path:** in a short test world one stray draw by a careless host left no trace, so comparing worlds at the end alone can miss it.
  Production's chance keyed by being and moment (`TIM-16`) has no shared stream to disturb, and its repeat check compares the logs as they go.
- **Ages and the pace:** with the arc's pace halved twice, fire comes within 8 years of the first flakes, so 20 years between ages never gives the age of fire: an age now lasts until the next turning point, however short (`PRE-39`, your OK).
