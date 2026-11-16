/* Host test of amiga/amiga_start.c: the first-start copy (config.xml into
 * User, the player's own copy kept) and the no-ROMs message's words.
 *   test_start <scratch dir>
 * Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt). */
#include <stdio.h>
#include <string.h>
#include "amiga_start.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

int main(int argc, char **argv)
{
	const char *dir = argc > 1 ? argv[1] : ".";
	char from[512], to[512], buf[64];
	const char *t;
	FILE *f;

	snprintf(from, sizeof from, "%s/from.xml", dir);
	snprintf(to, sizeof to, "%s/to.xml", dir);
	remove(to);
	f = fopen(from, "wb");
	fputs("<data/>", f);
	fclose(f);
	CHECK(cb_copy_if_missing(from, to) == 1);
	f = fopen(to, "rb");
	CHECK(f && fread(buf, 1, sizeof buf, f) == 7 && memcmp(buf, "<data/>", 7) == 0);
	if (f)
		fclose(f);
	f = fopen(to, "wb");
	fputs("mine", f);
	fclose(f);
	CHECK(cb_copy_if_missing(from, to) == 0);
	f = fopen(to, "rb");
	CHECK(f && fread(buf, 1, sizeof buf, f) == 4);
	if (f)
		fclose(f);
	snprintf(from, sizeof from, "%s/none.xml", dir);
	remove(to);
	CHECK(cb_copy_if_missing(from, to) == -1);

	/* the requester's text takes exactly one %s, the ROMs drawer */
	t = cb_no_roms_text();
	CHECK(strstr(t, "%s") != NULL);
	CHECK(strchr(strstr(t, "%s") + 1, '%') == NULL);
	CHECK(strstr(t, "31 files") != NULL);
	CHECK(strstr(t, "Install-Cannonball") != NULL);

	printf("%s: %d failed\n", argv[0], fails);
	return fails != 0;
}
