Patch usage:

"PB_NTSC-U_BoxModePatched_mainDOL.bps"
This is meant to be applied to the main.dol of the NTSC-U version of Pokémon Box.
It enables the 'Box mode' to work without a link cable, for debugging the 'go-to-list' crash.

"Sonic Advance 2_HQ_11khz.bps"
Implements Ipatix's audio mixer, but keeps the samplerate at the original setting.
It fixes the pops in Techno Base's music.

"Sonic Advance 2_HQ_32khz.bps"
Implements Ipatix's audio mixer, but sets the samplerate higher to get better sound.
It's slower to emulate though.

"Sonic Advance 2_AGBE_compatible.bps"
Implements Ipatix's audio mixer, but keeps the original samplerate.
Implements an additional patch that skips a null dereference that resets the emulation during the end scenes.
Also applied is the custom Flash patch by Lesserkuma, this makes the emulator support saving.

All the Sonic Advance 2 patches are made and tested for the US version.

While the HQ patches can be used anywhere, the AGBE patch is ONLY intended for the GameCube's emulator.
