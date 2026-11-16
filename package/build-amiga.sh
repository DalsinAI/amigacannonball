#!/bin/sh
# The release archive and the GFOpenUp part, for Cradle's stove_build
# (target package):
#  1. upstream Cannonball, fetched at the commit package/cannonball.lock
#     names (it is never kept in this repository), with amiga/patches
#     applied;
#  2. the game built by amiga/Makefile.amigaos;
#  3. the drawer, its icon and the Installer script beside it packed as
#     NAME-VERSION-68040.lha (tools/pack_dist.py), with its .sha256 and a
#     short .readme;
#  4. OUT/Parts/Cannonball, a GFOpenUp part: its Part file and the
#     drawer's files for SYS:Games/Cannonball, installed with
#       GFOpenUpTool INSTALL Parts/Cannonball
#     It needs the GFOpenGPU part, whose SDL2.module draws the game.
# The OutRun ROMs are the player's own: neither the archive nor the part
# carries them. Nothing is published from here.
# Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/.." && pwd)
. "$HERE/package.env"
. "$HERE/cannonball.lock"
OUT=${OUT:-"$ROOT/build-amiga-package"}
mkdir -p "$OUT"
OUT=$(CDPATH= cd -- "$OUT" && pwd)
STOVE=${STOVE:-"$HOME/AmiCradle/kitchen/stoves/os32-gcc16-open6"}
# outside the checkout: Cradle cleans it (git clean -ffdx) before every job
WORK=${WORK:-"$HOME/.cache/amigacannonball"}
mkdir -p "$WORK"
cd "$ROOT"

# 1. upstream at the locked commit, patched afresh every time
CB="$WORK/cannonball"
if [ "$(git -C "$CB" rev-parse HEAD 2>/dev/null)" != "$CB_COMMIT" ]; then
    rm -rf "$CB"; mkdir -p "$CB"
    git -C "$CB" init -q
    git -C "$CB" fetch -q --depth 1 "$CB_URL" "$CB_COMMIT"
    git -C "$CB" checkout -q FETCH_HEAD
fi
git -C "$CB" checkout -q -- . && git -C "$CB" clean -qfdx
for p in "$ROOT"/amiga/patches/*.patch; do
    git -C "$CB" apply "$p"
done

# 2. the game (a stove's compiler is in its prefix/, or STOVE is the prefix)
if [ -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ]; then PREFIX="$STOVE/prefix"; else PREFIX="$STOVE"; fi
rm -rf "$DIST"
make -f amiga/Makefile.amigaos CB="$CB" STOVE="$STOVE" PREFIX="$PREFIX" -j4 $TARGET > "$OUT/build-output.txt" 2>&1 || { tail -n 60 "$OUT/build-output.txt"; exit 1; }

# 3. the archive
python3 "$ROOT/tools/pack_dist.py" --dist "$DIST" --name "$NAME" --out "$OUT"

# 4. the GFOpenUp part
sh "$ROOT/package/write-part.sh" "$DIST/$NAME" "$OUT/Parts/$NAME" "$NAME" "$PORT_VERSION" \
    "Cannonball: the enhanced OutRun engine (needs your own OutRun ROMs)"
echo "part: $(grep -c '^copy' "$OUT/Parts/$NAME/Part") files, $(du -sk "$OUT/Parts/$NAME" | cut -f1) KB"
ls -l "$DIST/$NAME/$NAME"
