/* amiga_start.c: a file copied only where none is yet.
 * Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt). */
#ifndef AMIGA_START_H
#define AMIGA_START_H
/* 1 copied, 0 "to" was there already (left alone), -1 failed */
int cb_copy_if_missing(const char *from, const char *to);
/* the words the no-ROMs requester shows (one %s: the ROMs drawer) */
const char *cb_no_roms_text(void);
/* tells the player the ROMs are missing (a requester on the Amiga) */
void amiga_no_roms(const char *rom_path);
#endif
