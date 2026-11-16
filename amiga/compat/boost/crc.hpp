// boost::crc_32_type, as Cannonball's romloader.cpp uses it to know the
// OutRun ROMs by their contents: the standard CRC-32 (the one zip and MAME
// use: reflected, polynomial 0xEDB88320, starting and ending with all ones).
// Our own; not Boost.
// Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
#ifndef CB_COMPAT_CRC_HPP
#define CB_COMPAT_CRC_HPP

#include <cstddef>
#include <stdint.h>

namespace boost {

class crc_32_type {
public:
    crc_32_type() : crc_(0xFFFFFFFFu) {}
    void process_bytes(const void *data, std::size_t n)
    {
        const unsigned char *p = static_cast<const unsigned char *>(data);
        const uint32_t *t = table();
        uint32_t c = crc_;
        while (n--)
            c = t[(c ^ *p++) & 0xFF] ^ (c >> 8);
        crc_ = c;
    }
    void process_byte(unsigned char b) { process_bytes(&b, 1); }
    uint32_t checksum() const { return crc_ ^ 0xFFFFFFFFu; }
    void reset() { crc_ = 0xFFFFFFFFu; }
private:
    static const uint32_t *table()
    {
        static uint32_t t[256];
        static bool made = false;
        if (!made) {
            for (uint32_t i = 0; i < 256; i++) {
                uint32_t c = i;
                for (int k = 0; k < 8; k++)
                    c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
                t[i] = c;
            }
            made = true;
        }
        return t;
    }
    uint32_t crc_;
};

} // namespace boost

#endif
