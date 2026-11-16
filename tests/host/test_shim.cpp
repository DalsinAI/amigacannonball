// Host test of amiga/compat/boost (our stand-in for the Boost calls
// Cannonball makes). Built twice by tests/host/test_all.sh: once on our
// shim and, when the host has the real Boost headers, once on Boost, and
// the two runs' outputs must be the same. It reads every setting
// Cannonball's config.cpp reads from upstream's res/config.xml and from
// amiga/config.xml, writes the tree back and reads it again, writes and
// reads a score file the way config.cpp does, and checks CRC-32 and the
// string predicates.
//   test_shim <upstream res/config.xml> <amiga/config.xml> <scratch dir>
// Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/crc.hpp>
#include <boost/version.hpp>
#include <boost/static_assert.hpp>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#if (BOOST_VERSION >= 105600)
typedef boost::property_tree::xml_writer_settings<std::string> xml_writer_settings;
#else
typedef boost::property_tree::xml_writer_settings<char> xml_writer_settings;
#endif
BOOST_STATIC_ASSERT_MSG(sizeof(int) == 4, "int is not 4 bytes");

using boost::property_tree::ptree;
static int fails;
#define CHECK(c) do { if (!(c)) { std::cout << "FAIL " << __LINE__ << " " #c "\n"; fails++; } } while (0)

// Every setting config.cpp's load() reads, with its default, printed.
static void dump(const ptree &pt)
{
    const char *ints[] = {
        "data.crc32", "menu.enabled", "menu.roadspeed", "video.mode", "video.window.scale",
        "video.scanlines", "video.fps", "video.fps_counter", "video.widescreen", "video.hires",
        "video.filtering", "video.vsync", "video.shadow", "sound.enable", "sound.rate",
        "sound.advertise", "sound.preview", "sound.fix_samples", "sound.music_timer",
        "smartypi.<xmlattr>.enabled", "smartypi.outputs", "smartypi.cabinet",
        "controls.gear", "controls.steerspeed", "controls.pedalspeed",
        "controls.keyconfig.up", "controls.keyconfig.down", "controls.keyconfig.left",
        "controls.keyconfig.right", "controls.keyconfig.acc", "controls.keyconfig.brake",
        "controls.keyconfig.gear1", "controls.keyconfig.gear2", "controls.keyconfig.start",
        "controls.keyconfig.coin", "controls.keyconfig.menu", "controls.keyconfig.view",
        "controls.padconfig.acc", "controls.padconfig.right", "controls.padconfig.limit_c",
        "controls.analog.<xmlattr>.enabled", "controls.pad_id", "controls.analog.axis.wheel",
        "controls.analog.axis.accel.<xmlattr>.invert", "controls.analog.wheel.zone",
        "controls.analog.haptic.<xmlattr>.enabled", "controls.analog.haptic.max_force",
        "engine.freeplay", "engine.time", "engine.traffic", "engine.japanese_tracks",
        "engine.prototype", "engine.levelobjects", "engine.fix_bugs", "engine.offroad",
        "engine.car_color", "time_trial.laps", "time_trial.traffic", "continuous.traffic",
        "no.such.setting" };
    for (unsigned i = 0; i < sizeof ints / sizeof ints[0]; i++)
        std::cout << ints[i] << "=" << pt.get(ints[i], -7) << "\n";
    std::cout << "rompath=" << pt.get("data.rompath", "roms/") << "\n";
    std::cout << "respath=" << pt.get("data.respath", "res/") << "\n";
    std::cout << "savepath=" << pt.get("data.savepath", "./") << "\n";
    std::cout << "rumble=" << pt.get("controls.rumble", 1.0f) << "\n";
    for (int i = 0;; i++) {
        std::ostringstream t;
        t << "sound.custom_music.track" << (i + 1);
        boost::optional<int> tag = pt.get_optional<int>(t.str() + ".<xmlattr>.enabled");
        if (!tag.is_initialized())
            break;
        std::cout << t.str() << " enabled=" << tag.value()
                  << " title=" << pt.get(t.str() + ".title", "TRACK " + std::string("?"))
                  << " file=" << pt.get(t.str() + ".filename", std::string("track.wav")) << "\n";
    }
}

static void config(const std::string &file, const std::string &dir)
{
    ptree pt;
    read_xml(file, pt, boost::property_tree::xml_parser::trim_whitespace);
    std::cout << "== " << file.substr(file.find_last_of('/') + 1) << "\n";
    dump(pt);
    // save() puts settings, then the file is written and read again
    pt.put("video.mode", 1);
    pt.put("controls.rumble", 0.5f);
    pt.put("controls.analog.<xmlattr>.enabled", 2);
    pt.put("engine.new_setting", 3);
    std::string out = dir + "/written.xml";
    write_xml(out, pt, std::locale(), xml_writer_settings('\t', 1));
    ptree back;
    read_xml(out, back, boost::property_tree::xml_parser::trim_whitespace);
    std::cout << "-- written and read again\n";
    dump(back);
    std::cout << "engine.new_setting=" << back.get("engine.new_setting", 0) << "\n";
}

static void scores(const std::string &dir)
{
    ptree pt;
    for (int i = 0; i < 3; i++) {
        std::ostringstream t;
        t << "score" << i;
        pt.put(t.str() + ".score", "1A2B3C");
        pt.put(t.str() + ".initial1", i == 1 ? "." : "A");
        pt.put(t.str() + ".initial2", std::string(1, (char)('X' + i)));
        pt.put(t.str() + ".maptiles", "20202020");
    }
    std::string f = dir + "/scores.xml";
    write_xml(f, pt, std::locale(), xml_writer_settings('\t', 1));
    ptree back;
    read_xml(f, back, boost::property_tree::xml_parser::trim_whitespace);
    std::cout << "== scores\n";
    for (int i = 0; i < 4; i++) {
        std::ostringstream t;
        t << "score" << i;
        std::cout << back.get<std::string>(t.str() + ".score", "0") << " "
                  << back.get(t.str() + ".initial1", ".")[0]
                  << back.get(t.str() + ".initial2", ".")[0] << " "
                  << back.get<std::string>(t.str() + ".maptiles", "20202020") << "\n";
    }
}

static void edge(const std::string &dir)
{
    std::string f = dir + "/edge.xml";
    FILE *o = fopen(f.c_str(), "wb");
    fputs("<?xml version=\"1.0\"?>\n<!-- a comment -->\n<a x=\"1 &amp; 2\">\n"
          "  <b>  3 <!-- inside --> </b>\n  <c/>\n  <d>&lt;tag&gt; &#65;&#x42;</d>\n"
          "  <b>second</b>\n</a>\n<e>5</e>\n", o);
    fclose(o);
    ptree pt;
    read_xml(f, pt, boost::property_tree::xml_parser::trim_whitespace);
    std::cout << "== edge\n";
    std::cout << "a.x=" << pt.get("a.<xmlattr>.x", "") << "\n";
    std::cout << "a.b=" << pt.get("a.b", 0) << "\n";
    std::cout << "a.c=[" << pt.get("a.c", "none") << "]\n";
    std::cout << "a.d=" << pt.get("a.d", "") << "\n";
    std::cout << "e=" << pt.get("e", 0) << "\n";
    std::cout << "a.b as text=" << pt.get("a.b", "") << "\n";
    std::cout << "bad int=" << pt.get("a.d", 9) << "\n";
    bool threw = false;
    try {
        ptree none;
        read_xml(dir + "/no-such-file.xml", none, boost::property_tree::xml_parser::trim_whitespace);
    } catch (std::exception &) {
        threw = true;
    }
    std::cout << "missing file throws=" << threw << "\n";
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        std::cerr << "test_shim <res/config.xml> <amiga/config.xml> <dir>\n";
        return 2;
    }
    config(argv[1], argv[3]);
    config(argv[2], argv[3]);
    scores(argv[3]);
    edge(argv[3]);

    boost::crc_32_type c;
    c.process_bytes("123456789", 9);
    CHECK(c.checksum() == 0xCBF43926u);           // the standard check value
    boost::crc_32_type z;
    CHECK(z.checksum() == 0);
    std::cout << "crc=" << std::hex << c.checksum() << std::dec << "\n";

    CHECK(boost::starts_with(std::string("SOUND"), "SOU"));
    CHECK(!boost::starts_with(std::string("SO"), "SOUND"));
    CHECK(boost::ends_with(std::string("track1.wav"), ".wav"));
    CHECK(!boost::ends_with(std::string("track1.bin"), ".wav"));
    std::cout << "fails=" << fails << "\n";
    return fails != 0;
}
