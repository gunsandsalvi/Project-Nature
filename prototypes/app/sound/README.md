# P14 Sound on the phone

The app's screen for P14 (IMPLEMENTATION α0.7c, research 15): do 32 sounds at once with distance filters, a cave's
echo and the murmur play without breaks on your phone? Its sound library is `prototypes/sound`, whose README names the
items it is about.

- **The camp** (`camp.gd`): the art book's close camp as a map, everything that sounds in it placed on its picture:
  the fire and people at work and talking under the rock shelter, more at the tents, two arguing above the cliff and a
  group in the meadow; children; walkers on the path; two dogs, birds in the wood and a wolf far off; the stream,
  wind, rain and thunder; and, with everyone at once, more workers and eight stand-in drummers for the music share.
  Each sounds in its hours (day, dusk, night, storm) at its own pace.
- **Where you stand** (`sound.gd`, `SND-08`): touch the picture to stand there, drag to walk. Every sound plays from a
  Godot 3D player at its place, quieter and duller with distance (their own low-pass); the cliff muffles what is on
  its other side (a stand-in for the simulation's line through the land); what sounds under the shelter echoes,
  through an area with reverb.
- **The voice manager** (`voices.gd`, `SND-01`, `SND-07`): 32 sounds at most, shared: up to 6 for place and weather,
  4 single voices, 8 music, 2 kept free for thunder or a scream, and the rest, at least 12, for work, fire, steps and
  animals. When a share is full, its quietest sounds join its hum (talk and work have one), each until it would have
  ended; the other shares let the quietest go.
- **Talk is made beside the game** on worker threads, from the bank's syllables (`SND-03`). **Voices** plays a child,
  a woman, a man and an old man, then anger and grief, one by one.
- **Measure** (`report.gd`, `SND-01`, `PLT-04`): everyone at once in a storm for a minute, asking for more than the cap
  so all 32 places stay full, while the probe times the audio thread; the line for the chat gives the sounds at most
  and on average, the audio thread's share of each block's time (mean, 99 in 100, worst), any block over its time
  (a break), the block size and rate, and the output's delay. It passes if the mix takes at most a quarter of each
  block's time on average and never more than half.
- **The reel** (`SND-12`): the screen run with "sound reel" tours the camp by day, dusk, night and storm, with the
  voices one by one and everyone at once; `tools/reel.sh` records it in the cloud for the note.

Its tests are `test/sound_test.gd`.
