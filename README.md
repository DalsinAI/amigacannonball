# Cannonball for GFOS

The AmiCradle Team's port of Cannonball (Chris White's enhanced OutRun
engine) to GFOS: AmigaOS 3.2 with GFOpenUp, 68040/68060 + FPU, on
GFOpenUp's SDL 2. It ships as a release archive with an Installer and as
the GFOpenUp part Parts/Cannonball.

Cannonball needs the player's own OutRun arcade ROMs (revision B). They
are never in this repository, the archive or the part; Install-Cannonball
copies them from the player's own drawer or disk.

Upstream is fetched at build time (package/cannonball.lock); this
repository holds only the Amiga port. See [AMIGA_PORT.md](AMIGA_PORT.md).

Cannonball's licence (non-commercial; a changed build carries its
complete source, which the archive does in its Source drawer).
Copyright (c) 2026 AmiCradle Team.
