# Kindling α01d: Steady detail

## What is new

- Up close the ground has texture: grass grows in clumps, dirt in clods, rock in facets and scree in small bumps, each look with its own fine relief, fixed to the land. As you zoom away it fades out smoothly, before it could flicker.
- Stones and grass tufts: small stones lie on the scree below the cliff, and a few in the grass and on the dirt; grass and dirt grow little tufts of three to five blades. Each is a real small 3D shape, lit by the sun and the sky like the ground, with a darker foot. As you zoom out they drop away one by one, the smallest and least important first, until none are left at the camp view.
- The ground round a stone or a tuft is a little darker, where it hides part of the sky.
- The edges between grass, dirt, rock and scree are read from each surface's share of the ground under the pixel, so they stay smooth and never shimmer at any zoom.
- Behind the scenes, α01 ends with its steadiness counts: the tests record how many pixels each step of a pinch changes at the camp and close camp views and fail a later version that changes more without saying so, and a slow drag must change nothing but whole-pixel moves.

## What to try

1. Tap **Download and install** at the top of this page. It installs over α01c. Open Kindling.
2. Pinch in all the way near the foot of the cliff and look at the grass and the scree: small V-shaped tufts with dark feet in the grass, small stones on the scree, and clumps in the grass's light.
3. Pinch slowly out: the tufts and stones thin out one by one and the clumps soften, until the camp view shows plain ground again. Nothing should flicker or change all at once.
4. Drag slowly: the stones and tufts move with the land, whole pixels at a time, with no shimmer.
5. Tap the strip for other hours: the tufts and stones are lit with the land, and in the cliff's shadow they show as dark sprigs and stones.
6. Open the web link: the same in the browser, moved with the mouse.
7. If a box with a code appears, tap Copy and paste the code into your reply.

## What is rough

- The texture costs some steadiness close up: while you turn or pinch slowly at the close camp view, about 3 in 100 pixels crawl each frame, up from 2 in 100 in α01c; at the camp view it stays about 3 in 100, and a drag still moves whole pixels and crawls not at all. The fix chosen at the first visual review is meant to cut this.
- Each 0.01 step of a pinch now changes up to about half the pixels close up, up from two in five, because the ground there has texture; nearly all of that is the land really moving, and at most 3 in 10,000 pixels change where the land stays put.
- In full sun the stones on the bright scree are hard to tell from its bumps; they stand out in shade and when the sun is low.
- The stones and tufts are placed by the renderer for now; from α21 the land's own patches will place them.
- Loading the land takes a little longer in the browser, about 0.28 seconds instead of 0.24: the stones and tufts add a few hundredths of a second to the light's fields, which still take longer than planned, as in α01c (about 0.2 seconds once as the land loads, and about 15 ms at each tap of the strip).
- The app is a little larger: the download is 825 KB, up from 763 KB, and the web page's code 625 KB, up from 558 KB.
- Still as before: the cliff is a steep slope of 1 m squares, so close up its top edge shows steps (α02c); the haze is too faint to see within reach, and beyond the piece of land is dark (α02a).

## IDs delivered

`PRE-20` (part: smooth surface edges; each look's fine relief), `PRE-22` (part: detail that fades before it can flicker; the steadiness counts), `PRE-46` (part: stones and tufts), `PRE-02` (part), `WLD-12` (part: stones and ground cover on the areas' ground).

## Links

- APK: https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk
- Web: https://claude.ai/artifact/NmypTQyKQUAFZs18TNJELH
- Note: https://claude.ai/artifact/GBackmSHJPak61yAd6we4d
