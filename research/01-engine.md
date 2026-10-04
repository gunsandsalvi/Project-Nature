# Research 01: the engine, and whether Godot can do what Kindling needs

**Question:** can Godot 4.7 do each demanding thing `PROJECT.md` asks, on your Pixel 11 Pro XL?
If it can, at what cost?
What must be built by us, and what must be proven by a prototype before anything rests on it (`PRC-03`, `PRE-02`, `VIS-14`, `PLT-01`)?

**How it was checked:**
- Godot's own documentation for 4.7 and its release notes;
- the godot-cpp project;
- Google's and Arm's case studies of Godot on Android;
- forum threads with measurements;
- the one artist who built our reference look in Godot;
- Godot games of the same kind, shipped or in development;
- our own bake-off scene.

The phone itself is research 02.

## The decision so far

- **The bake-off (3 October 2026):** the same reference scene was built in Godot and in Bevy.
  - The Godot scene reached the look of the art guide in a couple of hours.
  - The Bevy side stopped early, because the same effects needed much more hand-written plumbing.
    For example, a leaf's sway had to be written again for its shadow in Bevy, while Godot applies it everywhere at once.
- **You chose Godot:** it is stable, has much help online, is easy to build in, has open source, and is the most liked.
- **Confirmed with you:**
  - the simulation is in C++ as a Godot plug-in (GDExtension);
  - Godot's own source is changed only as a last resort;
  - there is no browser preview (Godot's web build is about 40 MB, beyond the private page's 15 MB).
- **Not yet known:** the bake-off app's frame times on your phone.

## What Godot gives, and its limits

### 1. The renderer on a phone

Godot 4.7 has three renderers ([Godot docs: renderers](https://docs.godotengine.org/en/stable/tutorials/rendering/renderers.html)).
- **Forward+** is for desktops.
  On phones it is "supported, but poorly optimized.
  Use Mobile or Compatibility instead".
- **Mobile** is the one Godot means for phones: Vulkan, forward lighting, post effects in the fragment shader, and subpasses for tile-based chips like your PowerVR ([Arm](https://developer.arm.com/community/arm-community-blogs/b/mobile-graphics-and-gaming-blog/posts/optimizing-3d-scenes-in-godot-on-arm-gpus)).
- **Compatibility** is OpenGL ES 3, for old devices.
  Godot has had to disable a shader cache on every PowerVR device in this renderer ([Godot 4.5.2](https://godotengine.org/article/maintenance-release-godot-4-5-2/)).

What the Mobile renderer lacks, from Godot's own comparison table:

| Feature | Mobile | Forward+ | Matters to us |
|---|---|---|---|
| Normal and roughness buffer | no | yes | the outline pass read it in the bake-off |
| Depth texture, screen texture | yes | yes | outlines and water can read them |
| Ambient occlusion (SSAO), screen-space reflections, global illumination, volumetric fog | no | yes | none is in our look |
| Soft directional shadows (PCSS) | no | yes | we use hard shadows |
| Glow, decals, MSAA | yes | yes | fire glow; marks on the ground |
| Compute shaders | yes, slower on older devices | yes | optional |
| Lights | 8 per mesh, 256 in view | 512 per cluster | enough for campfires |

**What this means:** the bake-off ran Forward+ on your phone, which Godot advises against.
The look must be rebuilt on the Mobile renderer, where the outline pass has no normal buffer.
- **The standard fix:** rebuild each pixel's normal from the depth texture, which Mobile does have.
  A refined method samples the depth several times in each direction to get the normal of the true surface ([atyuwen](https://atyuwen.github.io/posts/normal-reconstruction/), [Wicked Engine](https://wickedengine.net/2019/09/improved-normal-reconstruction-from-depth/)).
  Blender's EEVEE uses it, and so does a Godot forum shader ([Godot forums](https://godotforums.org/d/30111-improved-normal-from-depth-shader)).
- **Or:** draw outlines from depth only, as the simplest pixel-art outliners do.
- **Either way:** this must be prototyped on your phone before anything is built on it.

### 2. Drawing at low resolution

There are two ways in Godot.
- **A SubViewport at a quarter of the screen size, scaled up with nearest sampling**, as in the bake-off.
  It allows the trick that keeps pans smooth: the camera snaps to the pixel grid, and the leftover fraction shifts the scaled image by part of a pixel.
- **Godot 4.7's built-in nearest scaling of the 3D buffer** ([Godot docs: Viewport](https://docs.godotengine.org/en/4.7/classes/class_viewport.html), [Godot 4.7](https://godotengine.org/releases/4.7/)).
  It has "no additional rendering cost", and the interface stays at full resolution.
  It cannot shift the image by part of a pixel, so pans would step whole art pixels.

Both suit your phone's tile-based chip, whose limit is memory bandwidth.
Arm measured one grey cube at full resolution with 4× MSAA running at 50 frames a second on a phone until Godot fixed its memory traffic in 4.4 ([Arm](https://developer.arm.com/community/arm-community-blogs/b/mobile-graphics-and-gaming-blog/posts/optimizing-3d-scenes-in-godot-on-arm-gpus)).
Drawing at a quarter of the width and height means 16 times fewer pixels.

### 3. The artist who built our reference look in Godot

David Holland built the 3D pixel art we use as a reference in a modified Godot 4.3 ([David Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).

He had to change Godot's source four times:

| His change | Why | In Godot 4.7? | Do we need it? |
|---|---|---|---|
| Depth and normal textures readable in the transparent pass | outlines under water | no | no: water is drawn after the outlines |
| A function to read the sun's shadow map in shaders | grass and light shafts | no official function | no: grass uses `LIGHT_VERTEX` |
| `LIGHT_VERTEX`, lighting at a chosen point | each grass card shaded as the ground at its foot | yes, merged ([PR 91136](https://github.com/godotengine/godot/pull/91136)); the bake-off uses it | yes |
| Custom camera projections | mirror-image water reflections | no: 4.7's RenderingServer has no custom projection ([Godot docs](https://docs.godotengine.org/en/4.7/classes/class_renderingserver.html)) | only for true reflections |

He later left Godot for his own framework (Odin with raylib), wanting "more control over the rendering pipeline and engine setup in general".
His target was a 2016 laptop graphics card (GTX 1060), at 640 × 360.
- **For us:** the look is reachable in Godot without changing its source, if the water fakes its reflections from the sky's colours rather than mirroring the scene.
  Real mirror reflections need an engine change, or a second camera with its own clipping, at the cost of drawing the scene twice.

### 4. Many things on screen

- **MultiMesh** draws thousands of copies in one call.
  But it is culled as one box, so copies spread far apart are always drawn: they must be grouped by area ([Godot docs: MultiMesh](https://docs.godotengine.org/en/stable/classes/class_multimesh.html)).
- **Below the scene tree,** Godot's servers can be driven directly by ID, for the case where "dealing with tens of thousands of instances for something that needs to be processed every frame can be a bottleneck" ([Godot docs: servers](https://docs.godotengine.org/en/4.7/tutorials/performance/using_servers.html)).
- **Proof from others:**
  - THRUM, a Godot 4 colony and civilisation game, aims at 10,000 persistent individuals with personalities, drawn by MultiMesh, on a deterministic fixed-step tick engine with speeds up to 100,000× ([Godot forum](https://forum.godotengine.org/t/thrum-colony-civilisation-simulation/138371));
  - a developer whose released Godot RTS handles a few thousand units says: no node per unit, use the servers directly, and write the logic in multi-threaded C++ ([r/godot](https://redlib.hbubli.cc/r/godot/comments/1jafwl4/rts_em_godot));
  - a desktop demo ran a million GPU-driven soldiers at about 126 frames a second.

### 5. Animated people and animals

- **Skinned characters with skeletons are expensive in Godot.**
  Developers report 50 to 100 on screen "safely", and 300 at 36 frames a second on an integrated desktop chip.
  Skeleton updates cost mostly processor time, the animation tree is heavy, and skeletons are not culled off screen ([Godot forum, January 2026](https://forum.godotengine.org/t/performance-of-crowds-of-animated-rigged-characters-in-3d/130409)).
- **Our figures are blocks.**
  Minecraft draws its creatures as rigid cuboid parts, each turned about its own pivot every frame, with no skinning at all ([Minecraft model renderer](https://www.mintlify.com/Minecraft-Community-Edition/client/api/model-renderer), [Blockbench](https://blockbench.net/wiki/guides/bedrock-modeling)).
  Each part of every figure can be one MultiMesh copy, posed by C++.
  A camp of 300 people with 6 parts each is 1,800 copies, which is cheap.

### 6. Threads

Godot's rules ([Godot docs: thread-safe APIs](https://docs.godotengine.org/en/4.7/tutorials/performance/thread_safe_apis.html)):
- the active scene tree is not thread-safe;
- the servers may be called from threads, but the RenderingServer only in the "Separate" thread model, which "has several known bugs".

So the simulation's own threads never touch Godot.
Once a frame, Godot's main thread copies the simulation's finished snapshot into its buffers.

### 7. The C++ plug-in

- **godot-cpp** builds with SCons or CMake.
  An extension built for an older 4.x works on later ones ([godot-cpp](https://github.com/godotengine/godot-cpp)).
- **Android builds** use the NDK, and `.gdextension` files can bundle native libraries inside Android plug-ins ([Godot docs](https://docs.godotengine.org/en/4.5/tutorials/scripting/gdextension/gdextension_file.html)).
- **Large C++ extensions already run on Android:** Terrain3D ships arm64 binaries, though its mobile support is marked experimental ([Terrain3D](https://store.godotengine.org/asset/tokisangames/terrain3d/)).

### 8. A world 2,000 km wide

- **Godot's double-precision build** needs a rebuilt editor and export templates.
  It costs speed and memory, and is "tailored towards mid-range/high-end desktop platforms" ([Godot docs: large world coordinates](https://docs.godotengine.org/en/4.7/tutorials/physics/large_world_coordinates.html)).
- **So:** single precision in Godot, with an origin that moves with the camera, while the simulation keeps exact coordinates of its own (research 07).
- **Terrain:** Terrain3D's clipmaps cover one height map.
  None of Godot's terrain tools zoom out to a globe, so that is ours to build (research 07).

### 9. Android's own features

Godot reaches Android without changing its source:
- **Android plug-ins** in Kotlin or Java;
- **JavaClassWrapper** (4.4 and later) calls Java classes directly;
- **implementing Java interfaces from GDScript** (4.7) ([Godot docs: Android APIs](https://docs.godotengine.org/uk/4.5/tutorials/platform/android/javaclasswrapper_and_androidruntimeplugin.html), [Godot 4.7](https://godotengine.org/releases/4.7/)).

What we would reach that way:
- **The phone's writer, Gemini Nano,** through ML Kit's Prompt API.
  - It has been in beta since 28 January 2026, and the Pixel 11 runs its newest model, nano-v4 ([ML Kit GenAI](https://developers.google.com/ml-kit/genai)).
  - Its limits matter:
    - it works "only when the app is the top foreground application";
    - each app has quotas, and a battery quota over long periods;
    - "different versions may return different output from the same prompt".
  - The alternative is NobodyWho: llama.cpp inside Godot, with downloaded models (for example Gemma 2 2B), Vulkan acceleration and Android support ([NobodyWho](https://docs.nobodywho.ooo)).
    It has no quotas, but costs about 1.5 GB of model and much battery.
- **The phone's temperature forecast** (ADPF thermal headroom).
  Google ships ready-made plug-ins for Unreal and Defold, not for Godot, so ours is a few lines of Kotlin ([Android Developers](https://developer.android.com/games/engines/unreal/unreal-adpf)).
- **The display's refresh rate.**
  Godot's frame cap renders 60 frames a second, but leaves the screen refreshing at 120.
  Games that save battery switch the screen itself to 60 through Android ([Godot forum](https://forum.godotengine.org/t/android-godot-why-fps-limit-display-refresh-rate-on-120-hz-devices-terraria-mobile-legends-example/132320)).

### 10. Smoothness and stability on Android

- **Frame pacing:** Google's Swappy is built in and on by default, with the screen's rotation done by Godot ([Godot docs: jitter and stutter](https://docs.godotengine.org/en/4.4/tutorials/rendering/jitter_stutter.html)).
- **Speed:** Google and The Forge optimised Godot's Vulkan code for Android, cutting graphics frame times by 10 to 20% ([Android Developers](https://developer.android.com/stories/games/godot-vulkan)).
  Examples:
  - memory allocated lazily on tile-based chips, saving about 50 MB;
  - shared buffers on phones' unified memory;
  - pre-rotation of the screen.
- **Crashes:** Godot 4.5.2 (19 March 2026) fixed "a lot of crash reports" from Vulkan Mobile games on Google Play ([Godot 4.5.2](https://godotengine.org/article/maintenance-release-godot-4-5-2/)).
  Users of older PowerVR chips have reported 3D not drawing at all under Vulkan ([Godot Q&A](https://ask.godotengine.org/152628/godot-exporting-android-doesnt-show-the-objects-only-the-ui)).
  - Your phone's chip family has a history of driver trouble (research 02).
  - So the first phone builds must exercise every rendering feature we plan, not only the simple ones.

### 11. When the app is closed

- **Godot's main loop stops when the app goes to the background,** and Android may kill it there at any time.
  So the world must be saved on pausing ([Godot forum](https://forum.godotengine.org/t/overview-on-android-application-lifecycle/57756)).
  This matches `TIM-05` and `PLT-07`.
- **The world runs only while the app is open,** which is what `TIM-05` asks.
  Running it in the background is not an option anyway:
  - Godot does not run in the background;
  - since Android 15, long-running services of the fitting kinds are limited to 6 hours a day ([Android Developers](https://developer.android.com/about/versions/15/changes/foreground-service-types)).

### 12. Sound, touch and screens

- **Sound:** Godot's 3D audio players, buses and effects cover the design.
  Several developers report delayed sound on some Android phones, up to half a second, and Godot may move to Google's Oboe library ([Godot forum](https://forum.godotengine.org/t/audio-latency-on-android-devices/134704)).
  Ambience does not mind; a tap's click might.
  It is measured on your phone (research 15).
- **Touch:** Godot gives each finger's touches and drags.
  Pinch and twist are worked out by our own code (research 14).
- **Screens:** containers, stretch modes and the screen's safe area cover portrait and landscape ([Godot docs: multiple resolutions](https://docs.godotengine.org/en/4.7/tutorials/rendering/multiple_resolutions.html)).

### 13. Games like ours made in Godot

- **Of Life and Land** (Godot, early access April 2024) has a deep simulation of nature ([GamingOnLinux](https://www.gamingonlinux.com/2024/04/of-life-and-land-is-a-promising-settlement-building-game-with-rich-simulation-out-on-steam/page=1/)):
  - animals that hunger, thirst, sleep and form groups;
  - plants that grow by climate and season;
  - temperature affecting everyone;
  - time up to 300 times faster.

  It is the closest precedent for our living world, on PC.
- **THRUM** (above): thousands of individuals with personalities, also on PC.
- **No precedent found:** a Godot game simulating thousands of minds on a phone.
  That part is ours to prove.

## Can Godot do it? Requirement by requirement

| What `PROJECT.md` asks | Godot? | How | Risk | Prototype first? |
|---|---|---|---|---|
| The 3D pixel look (`PRE-01` to `PRE-30`) | Yes, on the Mobile renderer, with our own normals for outlines | low resolution, nearest scaling, our light function, outlines from depth | medium | yes, on the phone |
| Smooth at 60 frames a second (`VIS-14`, `PLT-04`) | Likely: low resolution suits the chip | MultiMesh by area; frame pacing built in | high: weak, throttling GPU and driver history (research 02) | yes, with a full scene |
| One zoom from a person to the globe (`PRE-03`, `PRE-29`, `WLD-02`) | Not built in; possible with our own level of detail | chunks of ground at several detail levels, moving origin, map look, globe | high | yes |
| Thousands of full minds (`MND-14`, `MND-15`, `TIM-07`) | Not Godot's job: our C++ library on the processor's cores | entities, events, fixed threads | high: the processor budget | yes: a benchmark world on the phone |
| People and animals drawn and moving (`PRE-27`, `PRE-44`) | Yes, without skeletons | rigid block parts as MultiMesh copies, posed in C++ | low | in the look prototype |
| Plants and grass everywhere (`WLD-31`, `PRE-46`) | Yes | MultiMesh per area, visibility ranges | medium: overdraw of cards | in the look prototype |
| The same history everywhere (`RES-05`, `TIM-16`) | Outside Godot | our C++ rules, never Godot's maths | medium | yes: the same bits on the phone |
| A world 2,000 km across (`WLD-03`) | Yes, with a moving origin | single precision in Godot, exact in the simulation | medium | in the zoom prototype |
| Saving, export and import (`PLT-07`, `PLT-08`) | Yes | files in the app's storage; 4.7's file picker | low | no |
| The phone's writer (`PRE-37`, `PRE-41`) | Yes, through a small Android plug-in | ML Kit Prompt API, or llama.cpp | medium: beta, quotas, front only, output varies by model | yes |
| Sound and the murmur (`SND`) | Yes | 3D audio players and buses | low to medium: Android latency | later |
| Both orientations, gestures (`PRE-34`, `PRE-33`) | Yes | containers, safe area, our gesture reader | low | no |
| Offline (`PLT-03`) | Yes | no network permission; the system manages the writer's model | low | no |
| Tests and pictures in the cloud (`PLT-05`) | Yes | headless Godot; software Vulkan pictures (proven in the bake-off) | low | proven |

## What we take

1. **Godot stays.**
   Everything `PROJECT.md` asks is within its reach without changing its source.
   The one exception is mirror reflections on water, which we fake from the sky instead.
2. **The Mobile renderer is the default,** with outlines from normals rebuilt from depth.
   Forward+ stays only if your phone's numbers show it is both fast and needed for the look.
3. **The simulation is a separate C++ library on its own threads.**
   It hands Godot a snapshot once a frame, and never calls Godot itself.
4. **Built by us on top of Godot:**
   - block figures as MultiMesh parts;
   - level of detail from a person to the globe;
   - pathfinding, since Godot's navigation is costly with many agents (research 10);
   - the gesture reader;
   - a small Android plug-in for the writer, temperature and refresh rate.
5. **Prototypes before production** (the guide, research 00):
   - the look on the Mobile renderer on your phone;
   - the frame time of a full scene;
   - the zoom to the globe;
   - the simulation's speed with thousands of minds on your phone;
   - the same bits on phone and cloud;
   - the writer.

## Sources

- Godot 4.7 documentation:
  - [renderers](https://docs.godotengine.org/en/stable/tutorials/rendering/renderers.html)
  - [Viewport](https://docs.godotengine.org/en/4.7/classes/class_viewport.html)
  - [servers](https://docs.godotengine.org/en/4.7/tutorials/performance/using_servers.html)
  - [thread-safe APIs](https://docs.godotengine.org/en/4.7/tutorials/performance/thread_safe_apis.html)
  - [large world coordinates](https://docs.godotengine.org/en/4.7/tutorials/physics/large_world_coordinates.html)
  - [RenderingServer](https://docs.godotengine.org/en/4.7/classes/class_renderingserver.html)
  - [multiple resolutions](https://docs.godotengine.org/en/4.7/tutorials/rendering/multiple_resolutions.html)
  - [MultiMesh](https://docs.godotengine.org/en/stable/classes/class_multimesh.html)
  - [jitter and stutter](https://docs.godotengine.org/en/4.4/tutorials/rendering/jitter_stutter.html)
  - [Android APIs](https://docs.godotengine.org/uk/4.5/tutorials/platform/android/javaclasswrapper_and_androidruntimeplugin.html)
  - [the .gdextension file](https://docs.godotengine.org/en/4.5/tutorials/scripting/gdextension/gdextension_file.html)
- Godot releases: [4.7](https://godotengine.org/releases/4.7/), [4.5.2](https://godotengine.org/article/maintenance-release-godot-4-5-2/), [PR 91136, LIGHT_VERTEX](https://github.com/godotengine/godot/pull/91136), [godot-cpp](https://github.com/godotengine/godot-cpp)
- Case studies:
  - [Google: Godot Vulkan optimisation for Android](https://developer.android.com/stories/games/godot-vulkan)
  - [Arm: optimising 3D scenes in Godot on Arm GPUs](https://developer.arm.com/community/arm-community-blogs/b/mobile-graphics-and-gaming-blog/posts/optimizing-3d-scenes-in-godot-on-arm-gpus)
  - [Google: ADPF for Unreal](https://developer.android.com/games/engines/unreal/unreal-adpf)
- [David Holland: 3D pixel art rendering](https://www.davidhol.land/articles/3d-pixel-art-rendering/)
- Normals from depth: [atyuwen](https://atyuwen.github.io/posts/normal-reconstruction/), [Wicked Engine](https://wickedengine.net/2019/09/improved-normal-reconstruction-from-depth/), [Godot forums](https://godotforums.org/d/30111-improved-normal-from-depth-shader)
- Godot forum threads:
  - [crowds of animated characters](https://forum.godotengine.org/t/performance-of-crowds-of-animated-rigged-characters-in-3d/130409)
  - [THRUM](https://forum.godotengine.org/t/thrum-colony-civilisation-simulation/138371)
  - [Android audio latency](https://forum.godotengine.org/t/audio-latency-on-android-devices/134704)
  - [Android lifecycle](https://forum.godotengine.org/t/overview-on-android-application-lifecycle/57756)
  - [120 Hz displays](https://forum.godotengine.org/t/android-godot-why-fps-limit-display-refresh-rate-on-120-hz-devices-terraria-mobile-legends-example/132320)
  - [3D not drawn on an older PowerVR chip](https://ask.godotengine.org/152628/godot-exporting-android-doesnt-show-the-objects-only-the-ui)
- [r/godot: RTS with thousands of units](https://redlib.hbubli.cc/r/godot/comments/1jafwl4/rts_em_godot)
- Writers:
  - [ML Kit GenAI APIs](https://developers.google.com/ml-kit/genai)
  - [Prompt API announcement](https://android-developers.googleblog.com/2025/10/ml-kit-genai-prompt-api-alpha-release.html)
  - [NobodyWho](https://docs.nobodywho.ooo)
- Android: [Android 15 service types](https://developer.android.com/about/versions/15/changes/foreground-service-types)
- Precedents:
  - [Of Life and Land](https://www.gamingonlinux.com/2024/04/of-life-and-land-is-a-promising-settlement-building-game-with-rich-simulation-out-on-steam/page=1/)
  - [Terrain3D](https://store.godotengine.org/asset/tokisangames/terrain3d/)
  - Minecraft's block models: [model renderer](https://www.mintlify.com/Minecraft-Community-Edition/client/api/model-renderer), [Blockbench](https://blockbench.net/wiki/guides/bedrock-modeling)
