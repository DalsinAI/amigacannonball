#!/bin/sh
# Host tests (Linux, gcc/g++, SDL 2 development files; the real Boost
# headers too, if the host has them, for step 3's comparison):
#  1. upstream at the locked commit takes amiga/patches;
#  2. the whole game and amiga_start.c build with the host's compiler
#     through amiga/Makefile.amigaos, on amiga/compat/boost (our stand-in)
#     and without Boost;
#  3. test_shim reads every setting config.cpp reads from upstream's
#     config.xml and from amiga/config.xml, writes them back and reads them
#     again, writes and reads a score file, reads comments, attributes,
#     entities and several top-level elements, and checks CRC-32 and the
#     string predicates. With Boost on the host it is built a second time on
#     Boost, and both runs must print exactly the same;
#  4. test_start: the first-start copy of config.xml into User and the
#     no-ROMs message;
#  5. the game itself starts with SDL's dummy drivers in a drawer laid out as
#     on the Amiga (PROGDIR:Res, PROGDIR:ROMs, PROGDIR:User are plain
#     directory names here): it copies Res/config.xml into User, reads it,
#     looks for the ROMs in PROGDIR:ROMs/ and, as there are none (they are
#     the player's own), says so and stops.
# Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/../.." && pwd)
. "$ROOT/package/cannonball.lock"
T=${WORK:-"${TMPDIR:-/tmp}/amigacannonball-test"}
mkdir -p "$T"
cd "$ROOT"
ok() { echo "ok: $*"; }

# 1. upstream + patches
CB="$T/cannonball"
if [ "$(git -C "$CB" rev-parse HEAD 2>/dev/null)" != "$CB_COMMIT" ]; then
    rm -rf "$CB"; mkdir -p "$CB"
    git -C "$CB" init -q
    git -C "$CB" fetch -q --depth 1 "$CB_URL" "$CB_COMMIT"
    git -C "$CB" checkout -q FETCH_HEAD
fi
git -C "$CB" checkout -q -- . && git -C "$CB" clean -qfdx
for p in amiga/patches/*.patch; do git -C "$CB" apply "$ROOT/$p"; done
ok "patches apply to $CB_COMMIT"

# 2. the build, without Boost: only amiga/compat/boost answers <boost/...>
B="$T/build"
make -s -f amiga/Makefile.amigaos -j8 CB="$CB" BUILD="$B" CC=gcc CXX=g++ CPU= NOIX= \
    SDL2KIT=/usr OPT=-O1 DEFS=-D__amigaos__ LIBS="$(sdl2-config --libs) -lm" \
    "$B/Cannonball.debug" > "$T/make.log" 2>&1 || { tail -30 "$T/make.log"; exit 1; }
for f in frontend/config frontend/menu romloader main; do
    if g++ -std=gnu++14 -M -D__amigaos__ -Iamiga/compat -I"$CB/src/main" $(sdl2-config --cflags) \
        "$CB/src/main/$f.cpp" | tr ' ' '\n' | grep -v amiga/compat | grep -q "/boost/"; then
        echo "FAIL: $f.cpp reaches a Boost header that is not ours"; exit 1
    fi
done
ok "game builds on the host, on our boost stand-in only"

# 3. the shim, and the same test on the real Boost when there is one
g++ -std=gnu++14 -O1 -Wall -Iamiga/compat -o "$T/test_shim" tests/host/test_shim.cpp
mkdir -p "$T/shim" "$T/real"
"$T/test_shim" "$CB/res/config.xml" amiga/config.xml "$T/shim" > "$T/shim.out"
grep -q "^fails=0$" "$T/shim.out" || { cat "$T/shim.out"; exit 1; }
ok "test_shim: $(grep -c = "$T/shim.out") values read and checked"
if [ -f /usr/include/boost/property_tree/ptree.hpp ]; then
    g++ -std=gnu++14 -O1 -o "$T/test_boost" tests/host/test_shim.cpp
    "$T/test_boost" "$CB/res/config.xml" amiga/config.xml "$T/real" > "$T/real.out"
    diff "$T/real.out" "$T/shim.out" || { echo "FAIL: our stand-in and Boost differ"; exit 1; }
    ok "test_shim prints the same on Boost $(sed -n 's/#define BOOST_LIB_VERSION "\(.*\)"/\1/p' /usr/include/boost/version.hpp)"
else
    echo "skip: no Boost headers on this host for the comparison"
fi

# 4. amiga_start.c
gcc -O1 -Wall -Iamiga -o "$T/test_start" tests/host/test_start.c amiga/amiga_start.c
"$T/test_start" "$T"

# 5. the game starts, finds no ROMs and says so
R="$T/run"
rm -rf "$R"; mkdir -p "$R/PROGDIR:Res" "$R/PROGDIR:ROMs"
cp "$CB/res/tilemap.bin" "$CB/res/tilepatch.bin" "$CB/res/gamecontrollerdb.txt" amiga/config.xml "$R/PROGDIR:Res/"
set +e
(cd "$R" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout -s KILL 20 "$B/Cannonball.debug") > "$T/run.out" 2>&1
rc=$?
set -e
[ "$rc" != 137 ] || { echo "FAIL: the game did not stop without ROMs"; exit 1; }
cmp -s amiga/config.xml "$R/PROGDIR:User/config.xml" || { echo "FAIL: User/config.xml is not Res/config.xml"; exit 1; }
grep -q "Unable to locate rom in path: PROGDIR:ROMs/" "$T/run.out" || { echo "FAIL: ROMs not looked for in PROGDIR:ROMs/"; cat "$T/run.out"; exit 1; }
grep -q "needs your own OutRun ROMs" "$T/run.out" || { echo "FAIL: no no-ROMs message"; cat "$T/run.out"; exit 1; }
ok "the game reads User/config.xml, looks in PROGDIR:ROMs/ and says the ROMs are missing"
echo "all host tests passed"
