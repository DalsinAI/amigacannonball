/* The Amiga port's start: the stack, the version string and PROGDIR:User
 * (the settings and the scores), made before main(). The game reads its
 * settings from User/config.xml: on the first start that is a copy of
 * Res/config.xml (the port's Amiga settings), and the player's own changes
 * stay there (Install-Cannonball and GFOpenUp keep User when they update
 * the game).
 * Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt). */
#include <stdio.h>
#include <sys/stat.h>
#include "amiga_start.h"
#if defined(__amigaos__) && defined(__m68k__)
#include <intuition/intuition.h>
#include <proto/intuition.h>
#endif

/* A Shell gives 4 KB of stack; SDL 2 and the engine want more. libnix's
 * startup sets this much up. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) =
    "$VER: Cannonball 0.35 (16.11.2026)";

int cb_copy_if_missing(const char *from, const char *to)
{
	char buf[4096];
	size_t n;
	FILE *in, *out;

	if ((out = fopen(to, "rb")) != NULL) {
		fclose(out);
		return 0;                       /* the player's own: keep it */
	}
	if ((in = fopen(from, "rb")) == NULL)
		return -1;
	if ((out = fopen(to, "wb")) == NULL) {
		fclose(in);
		return -1;
	}
	while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
		if (fwrite(buf, 1, n, out) != n) {
			fclose(in);
			fclose(out);
			remove(to);
			return -1;
		}
	}
	fclose(in);
	if (fclose(out) != 0) {
		remove(to);
		return -1;
	}
	return 1;
}

const char *cb_no_roms_text(void)
{
	return "Cannonball needs your own OutRun ROMs (revision B,\n"
	       "the arcade board's 31 files) in its ROMs drawer:\n"
	       "%s\n\n"
	       "Run Install-Cannonball again to copy them from your\n"
	       "drawer or disk; ROMs/roms.txt lists them.";
}

/* The game could not find the ROMs: tell the player, on Workbench too
 * (upstream only writes to the Shell). */
void amiga_no_roms(const char *rom_path)
{
#if defined(__amigaos__) && defined(__m68k__)
	struct EasyStruct es;
	APTR args[1];

	es.es_StructSize = sizeof es;
	es.es_Flags = 0;
	es.es_Title = (UBYTE *)"Cannonball";
	es.es_TextFormat = (UBYTE *)cb_no_roms_text();
	es.es_GadgetFormat = (UBYTE *)"OK";
	args[0] = (APTR)rom_path;
	EasyRequestArgs(NULL, &es, NULL, args);
#else
	fprintf(stderr, cb_no_roms_text(), rom_path);
	fputc('\n', stderr);
#endif
}

#ifdef __amigaos__
static void __attribute__((constructor)) amiga_user_drawer(void)
{
	mkdir("PROGDIR:User", 0755);
	cb_copy_if_missing("PROGDIR:Res/config.xml", "PROGDIR:User/config.xml");
}
#endif
