# Flute synth, RMI workshop NIME 2026
The flute sound synthesis pipeline presented at https://piaoziyue.github.io/RMIs_workshop

- [./main/wave_cube_synth.h](./main/wave_cube_synth.h) and [./main/wave_cube_synth.c](./main/wave_cube_synth.c) are for WaveCube Synthesis.  
- [./main/timbre.c](./main/timbre.c) is the measured timbre of my flute as well as various vowels.
- [./main/electric_flute.h](./main/electric_flute.h) and [./main/electric_flute.c](./main/electric_flute.c) are for the mapping from breath pressure to octave, pitchbend, and amplitude.
- See [./main/bootup_jingle.c](./main/bootup_jingle.c) for a demo usage.

This repo is not ready-to-run because it is an expunged version from a main private repo. The recommended consumption of this repo is to understand wave_cube_synth.c and electric_flute.c and write your own version. If you really want to run this repo, it should be possible as long as you delete all usage of irrelevant/expunged features in main.c until it compiles.
