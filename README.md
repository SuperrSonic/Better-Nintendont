# Better-Nintendont

After years of working on this on and off, I think it's finally in a state that deserves to be shared.

# New stuff

* Path-based argument support
* Sideways Wii Remote
* Blacks out when exiting
* Power button exits game
* MEM1 patching system via a patch.txt file
* Handles game crashes by rebooting
* Doesn't initialize the Wii Remote on the loader
* Sets the horizontal position from SYSCONF
* Wii Message Board playlog
* Writes savefiles from Nintendo's GBA emulator
* Saves pictures taken in Pokémon Box
* GameCube menu boot jingle forcing from Swiss
* GameCube menu 480p patch from Swiss

# New Game Compatibility

* Star Wars Rogue Squadron III Rebel Strike - no longer shows the glitched HP meter.
* Pokémon Box Ruby & Sapphire - Adventure Mode finally boots the GBA games.
* Tales of Symphonia - Consistent hangs no longer occur.

# GBA Emulator Saving

* The emulator is featured in Interactive Multi-Game Demo Disc v16, other versions may work.
* Supports games that use SRAM saves.
* Games that use 128 KB savefiles have an internal list.
* Other games need to be SRAM patched.
* The savefile needs to be provided in the same directory as the game.
* Savefiles are intended to be used with the FST format as data.sav, this means that for game.iso, the savefile is called game.isodata.sav
* The FST format allows swapping ROMs quickly for testing the emulator.
* Patches to zoom, oversample, remove dither, change screen filters, and brightness will be available as a patch.txt file.

# Video
[![Video backstory](https://img.youtube.com/vi/BbS-w6YMeiI/maxresdefault.jpg)](https://youtu.be/BbS-w6YMeiI)

# Why is this a hard fork?

For convenience, and the main project seems to be considered inactive anyway.
