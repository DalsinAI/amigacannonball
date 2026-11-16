Cannonball for GFOS
===================

A port of Cannonball, Chris White's enhanced OutRun engine, from
https://github.com/djyt/cannonball at commit
27493ebf62be3498dff93ed6a45e8e2db819bae1 (Cannonball 0.35), for GFOS:
AmigaOS 3.2 with GFOpenUp, on a 68040 or 68060 with an FPU, on GFOpenUp's
SDL 2.

Cannonball is the arcade OutRun, rebuilt from the original game's code:
the Ferrari, the blonde, the five-stage branching coast road, the radio,
with extras the arcade never had (time trial, continuous mode, the
Japanese tracks, a widescreen and a high-resolution road).

It runs the original arcade game's data: you need your own OutRun ROMs
(revision B). They are not in this archive and never will be.

Status: host tests pass; built in Cradle's os32-gcc16-open6 stove; not yet
run in a lab.


What you need
-------------

- GFOS: AmigaOS 3.2 with GFOpenUp 0.8.26 or later, which installs
  LIBS:GFOpenGPU/SDL2.module.
- A 68040 or 68060 with an FPU (not an LC040 or EC040), an RTG screen
  with room for a 796 x 448 window (the widescreen road at twice
  the arcade size; the menu sets a smaller one), and about 16 MB of free memory.
- Your own OutRun ROMs, revision B: the arcade board's 31 files,
  epr-10187.88 to opr-10268.99. ROMs/roms.txt lists them. The eight
  files of the first Japanese release are optional and add its tracks.


How to play
-----------

Install it one of three ways:

- Run Install-Cannonball from the archive (it offers Work:Games). It asks
  for the drawer or disk holding your ROMs and copies them into ROMs.
- GFOpenUp: `GFOpenUpTool INSTALL Parts/Cannonball` puts it in
  SYS:Games/Cannonball; then copy your ROMs into SYS:Games/Cannonball/ROMs.
- Or copy the Cannonball drawer where you want it and your ROMs into its
  ROMs drawer.

Then double-click Cannonball. Without the ROMs it says which drawer it
looked in and stops.

Keys: cursor left and right steer, Z accelerates, X brakes, Space changes
gear, 5 puts in a coin, 1 starts, Left Shift changes the view, F5 opens
the menu (the settings, the controls, time trial and continuous mode).
A joystick or pad works too. Esc leaves.

The settings (config.xml) and the high scores are kept in the drawer's
User drawer. On the first start the port's settings are copied there from
Res; change that copy, or use the game's menu. Install-Cannonball and
GFOpenUp keep User and ROMs when they update the game.


Building
--------

    sh package/build-amiga.sh        (STOVE=/path/to/os32-gcc16-open6)

or, in Cradle, stove_build with the os32-gcc16-open6 stove and target
package. It fetches upstream at the commit package/cannonball.lock names,
applies amiga/patches, builds with amiga/Makefile.amigaos and packs
Cannonball-VERSION-68040.lha and the GFOpenUp part Parts/Cannonball.

Host tests: `sh tests/host/test_all.sh` (Linux, gcc, SDL 2 development
files; Boost headers too, if the host has them) checks that the patch
applies; builds the whole game with the host's compiler through the same
Makefile, on our Boost stand-in and nothing else; reads every setting the
game reads from upstream's config.xml and from ours, writes them back and
reads them again, writes and reads a score file, and checks CRC-32 and
the string calls, and, on a host with Boost, builds the same test on the
real Boost and requires exactly the same output; tests the first-start
copy and the no-ROMs message; and starts the game with SDL's dummy
drivers in a drawer laid out as on the Amiga, where it must copy its
settings into User, look for the ROMs in PROGDIR:ROMs/ and, finding none,
say so and stop.


What was changed
----------------

- amiga/compat/boost: Cannonball reads its settings and scores with Boost
  (property_tree and its XML reader and writer, optional, crc_32_type,
  starts_with/ends_with). Boost is not on the Amiga, so this is our own
  small stand-in for exactly those calls; upstream's `#include
  <boost/...>` finds it first.
- amiga/patches/0001-amigaos-drawers.patch:
  - the settings file is PROGDIR:User/config.xml, the ROMs are read from
    PROGDIR:ROMs/, the resources from PROGDIR:Res/ and the scores kept in
    PROGDIR:User/;
  - when the ROMs are missing the game tells the player, on Workbench
    too, instead of only writing to the Shell.
- amiga/amiga_start.c: 256 KB of stack, the $VER string, PROGDIR:User
  made at start and the port's settings copied into it, and the no-ROMs
  requester.
- amiga/config.xml: upstream's settings with the Amiga drawer's paths,
  the original game's 30/60 frame rate, 22050 Hz sound, and the menu and
  view keys as SDL 2 key codes.
- The game draws with its SDL 2 software renderer (no OpenGL) and has no
  force feedback.
- amiga/: the Makefile, the GlowIcons (our own art), the Installer script
  and its spec. package/: the build, the release archive and the GFOpenUp
  part.

The AmiCradle Team.
