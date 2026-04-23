# Better-Nintendont

After years of working on this on and off, I think it's finally in a state that deserves to be shared.

# New stuff

* Path-based argument support
* Sideways Wii Remote
* Blacks out when exiting
* Power button exits game
* MEM1 patching system via a patch.txt file
* Handles game crashes by rebooting
* Sets the horizontal position from SYSCONF
* Wii Message Board playlog
* Writes savefiles from Nintendo's GBA emulator
* Saves pictures taken in Pokémon Box
* GameCube menu boot jingle forcing from Swiss
* GameCube menu 480p patch from Swiss

# Game Compatibility Tweaks

* Star Wars Rogue Squadron III Rebel Strike - no longer shows the glitched HP meter.
* Pokémon Box Ruby & Sapphire - Adventure Mode finally boots the GBA games.
* Tales of Symphonia - Consistent hangs no longer occur.
* Namco Museum 50th Anniversary - Loading into games no longer has a pop sound.
* Sonic Riders - The delay patch is adjusted to the absolute limit.

# GBA Emulator Saving

* The emulator required is from Interactive Multi-Game Demo Disc v16, other versions may work.
* Supports games that use SRAM saves.
* Games that use 128 KB savefiles have an internal list.
* Other games need to be SRAM patched.
* The savefile needs to be provided in the same directory as the game.
* Savefiles are intended to be used with the FST format as data.sav, this means that for game.iso, the savefile is called game.isodata.sav
* The FST format allows swapping ROMs quickly for testing the emulator.
* Patches to zoom, oversample, remove dither, change screen filters, and brightness will be available as a patch.txt file.

# Video
[![Video backstory](https://img.youtube.com/vi/BbS-w6YMeiI/maxresdefault.jpg)](https://youtu.be/BbS-w6YMeiI)


# Howto

It's intended to be interchangeable with Devolution, so memory cards now go in the /apps/gc_devo/ folder with the memcard.bin filename.

For the GameCube main menu /apps/gc_devo/ipl.bin, as is the case with segaboot.bin.

You can't load games from the internal interface itself, but you can specify an argument in the meta.xml, like: "usb:/games/Sonic Riders [GXEE8P]/game.iso" to autoboot a game, see the release pack for another example.

For regular loading, use USB Loader GX, WiiFlow, etc. The internal interface isn't used because the loader would initialize the wiimote, making the boot process longer, the interface is also visually offensive.

To exit a game with the GameCube controller, you must either press the power button, or connect a wiimote and press A+HOME.


# Using patch.txt

The following functions are possible:

* "poke" = Writes a 32-bit value to the provided MEM1 address.
* "pokeifequal" = Same as above but only if the first two arguments are true.
* "rand" = Writes a randomized value, takes 5 arguments, addr, start, end, pos, initial value.
* "triArcade" = If 1, it will stop parsing, useful for turning off big patches with the Triforce setting.
* "wiiChan" = Override a controller port with the sideways Wii Remote.
* "noVidpatch" = Clears all video patches, useful in some cases.
* "noProgAsk" = Skips the "Progressive Scan" screen if the game supports it.
* "iplJingle" = Force a specific boot jingle when using the GameCube menu, 1=baby, 2=kabuki, higher is random.
* "noTrapFilter" = Tends to blur the screen when using composite cables.
* "volume" = Change the volume of the AVE-RVL, 255 is the maximum, games that have a low volume may benefit.
* "ccDirect" = Rotates the ABXY on the Classic Controller (untested!)
* "mcdDelay" = Adjust the emulated memory card's access delay.
* "noLoadStub" = Launches the System Menu on exit, needed for the playlog.
* "writePlaylog" = If the arguments are ( 1, 0 , 0 ) it will use the opening.bnr title.
* "hexPlaylog" = You can supply the title via hex codes, allowing more control of characters.

Look at the included patch.txt for more examples, if you put a patch.txt file in the apps/gc_devo/ folder it will affect every game unless they already have their own specific patch.txt file.


# Why is this a hard fork?

For convenience, and the main project seems to be considered inactive anyway.
