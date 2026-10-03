# Research 01: the engine

**Question:** do we keep building our own engine, or build on an existing one?
Studios decide this first in pre-production, often after a short prototype in each candidate, because everything else rests on it.

## What Kindling needs from an engine

- **Your reference look:** a real 3D scene drawn at low resolution, with outlines from depth and normals, stepped light, shadows, instanced grass, water with reflections, and a camera locked to the pixel grid (see research 03).
- **Your phone:** a Pixel 11 Pro XL with a Tensor G6 and a PowerVR graphics chip ([Notebookcheck](https://notebookcheck.net/Pixel-11-Pro-XL-Geekbench-Compute-result-reveals-Tensor-G6-GPU-speed.1364963.0.html)).
  On the Pixel 10's PowerVR chip, Vulkan improved a lot after driver updates while OpenGL ES stayed poor ([Android Authority](https://www.androidauthority.com/pixel-10-gaming-android-17-update-pixel-11-worried-3693728/)), so the renderer should use **Vulkan**.
  Our current renderer uses OpenGL ES.
- **A web build:** for the automated screenshot tests in the cloud and for previews in the private page, which takes files up to 15 MB.
- **A big simulation:** thousands of people with needs, minds and memories, the same result every run, using the phone's cores.
- **AI builders:** everything in text files, built and tested from the command line, with no editor needed.
- **Models and animation:** loading models (glTF), skeletons and poses, instancing thousands of copies.

## How others do it

- **Similar games mostly use a general engine, with the simulation in their own code:** RimWorld is built on Unity ([Wikipedia](https://en.wikipedia.org/wiki/RimWorld)).
  Dwarf Fortress is its own C and C++ engine over OpenGL and SDL, built by one developer over 20 years ([Stack Overflow blog](https://stackoverflow.blog/2021/07/28/700000-lines-of-code-20-years-and-one-developer-how-dwarf-fortress-is-built/)).
  Songs of Syx is its own Java engine ([GamingOnLinux](https://gamingonlinux.com/2020/09/songs-of-syx-appears-to-be-an-early-success-brewing-on-steam)).
- **Tiny Glade**, a small stylised 3D game in Rust, uses Bevy for its data (the ECS) and its own Vulkan renderer ([Lemmy thread](https://lemmy.kde.social/post/2875159)).
- **The 3D pixel-art references:** David Holland built his in Godot 4.3, but had to change Godot's own source several times: shadows readable in shaders, depth and normals in the transparent pass, custom camera projections.
  He is now moving to his own framework (Odin and raylib) "for increased rendering pipeline control" ([David Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).
  t3ssel8r's work is in Unity ([JoonyoungSeo's pipeline credits](https://github.com/JoonyoungSeo/unity-isometric-pixel-pipeline)), and PixelageGames' demos are Godot ([itch.io](https://pixelagegames.itch.io/3d-pixelart-vegetation)).

## The candidates

| | Godot 4 | Bevy | Our own engine, rebuilt |
|---|---|---|---|
| What it is | A full engine with an editor; scenes and scripts are text files | A Rust engine, all code, built on data (ECS) | Our Rust code, on a modern graphics library (wgpu) instead of OpenGL ES |
| Reference look | Closest ready-made examples; some needed engine changes (Holland) | Full control of the render passes; fewer ready-made examples | Full control; everything written by us |
| Phone | Mature Android export; its mobile Vulkan renderer was tuned with Google ([Android Developers](https://developer.android.com/stories/games/godot-vulkan)) | Vulkan through wgpu; Android "usable" but less polished ([Bevy Cheat Book](https://bevy-cheatbook.github.io/platforms.html)) | Vulkan through wgpu; we write the Android shell |
| Web | Different renderer (WebGL2) from the phone's; an empty project's web build is about 35 MB ([Godot forum](https://forum.godotengine.org/t/export-optimising-a-build-for-size/94584)) | Same shaders on phone and web (WebGL2 or WebGPU); builds around 15 MB unless trimmed ([Bevy discussion](https://gh.nn.ci/bevyengine/bevy/discussions/2476)) | Smallest |
| Simulation | Script too slow for thousands of people; needs C++ or Rust plug-ins, and Rust on Android is "experimental" ([godot-rust](https://github.com/godot-rust/gdext)) | Rust ECS built for big simulations, runs systems across cores ([StraySpark](https://www.strayspark.studio/blog/bevy-rust-game-engine-2026-indie-guide)) | Rust; we write the scheduling |
| AI builders | Text files, but the engine assumes an editor; assets need an import step | Pure code, command-line builds | Pure code |
| Models and animation | Excellent glTF import and animation tools | glTF, skinning and animation built in | We write them |
| Stability | Stable within 4.x | Breaking changes every 3 to 4 months, "usually mechanical" ([StraySpark](https://www.strayspark.studio/blog/bevy-rust-game-engine-2026-indie-guide)) | Ours to keep |

## Recommendation

A short **bake-off**, as studios do: the same small reference scene in Godot and in Bevy, both on your phone, judged by you by eye, with frame times and a note on how hard each was to build.
If you'd rather choose now: **Bevy**.
- It is all code, which suits AI builders.
- It draws the same way on your phone and on the web.
- Its data model is built for big simulations.
- It leaves us full control of the pixel-art passes, which Holland's Godot work shows we will need.

Its risks are a less polished Android path and an upgrade every few months, pinned to one version between upgrades.
