/*-
 * Copyright (c) 2023 Chris Spiegel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <algorithm>
#include <codecvt>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <libopenmpt/libopenmpt.hpp>

// If true, only display the lines of the message which differ;
// otherwise, the entire message is shown.
static bool diff_only = true;

static std::vector<std::uint32_t> utf8_to_codepoints(const std::string &s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string utf32 = converter.from_bytes(s);

    return {utf32.begin(), utf32.end()};
}

static std::string codepoints_to_utf8(const std::vector<std::uint32_t> &codepoints)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string utf32(codepoints.begin(), codepoints.end());

    return converter.to_bytes(utf32);
}

static std::uint32_t cp437_to_unicode(std::uint32_t c)
{
    switch (c) {
        case 0x00: return 0x2400; case 0x01: return 0x263A; case 0x02: return 0x263B; case 0x03: return 0x2665;
        case 0x04: return 0x2666; case 0x05: return 0x2663; case 0x06: return 0x2660; case 0x07: return 0x2022;
        case 0x08: return 0x25D8; case 0x09: return 0x25CB; /*case 0x0A: return 0x25D9;*/ case 0x0B: return 0x2642;
        case 0x0C: return 0x2640; case 0x0D: return 0x266A; case 0x0E: return 0x266B; case 0x0F: return 0x263C;
        case 0x10: return 0x25BA; case 0x11: return 0x25C4; case 0x12: return 0x2195; case 0x13: return 0x203C;
        case 0x14: return 0x00B6; case 0x15: return 0x00A7; case 0x16: return 0x25AC; case 0x17: return 0x21A8;
        case 0x18: return 0x2191; case 0x19: return 0x2193; case 0x1A: return 0x2192; case 0x1B: return 0x2190;
        case 0x1C: return 0x221F; case 0x1D: return 0x2194; case 0x1E: return 0x25B2; case 0x1F: return 0x25BC;
        case 0x7F: return 0x2302;
        case 0x80: return 0x00C7; case 0x81: return 0x00FC; case 0x82: return 0x00E9; case 0x83: return 0x00E2;
        case 0x84: return 0x00E4; case 0x85: return 0x00E0; case 0x86: return 0x00E5; case 0x87: return 0x00E7;
        case 0x88: return 0x00EA; case 0x89: return 0x00EB; case 0x8A: return 0x00E8; case 0x8B: return 0x00EF;
        case 0x8C: return 0x00EE; case 0x8D: return 0x00EC; case 0x8E: return 0x00C4; case 0x8F: return 0x00C5;
        case 0x90: return 0x00C9; case 0x91: return 0x00E6; case 0x92: return 0x00C6; case 0x93: return 0x00F4;
        case 0x94: return 0x00F6; case 0x95: return 0x00F2; case 0x96: return 0x00FB; case 0x97: return 0x00F9;
        case 0x98: return 0x00FF; case 0x99: return 0x00D6; case 0x9A: return 0x00DC; case 0x9B: return 0x00A2;
        case 0x9C: return 0x00A3; case 0x9D: return 0x00A5; case 0x9E: return 0x20A7; case 0x9F: return 0x0192;
        case 0xA0: return 0x00E1; case 0xA1: return 0x00ED; case 0xA2: return 0x00F3; case 0xA3: return 0x00FA;
        case 0xA4: return 0x00F1; case 0xA5: return 0x00D1; case 0xA6: return 0x00AA; case 0xA7: return 0x00BA;
        case 0xA8: return 0x00BF; case 0xA9: return 0x2310; case 0xAA: return 0x00AC; case 0xAB: return 0x00BD;
        case 0xAC: return 0x00BC; case 0xAD: return 0x00A1; case 0xAE: return 0x00AB; case 0xAF: return 0x00BB;
        case 0xB0: return 0x2591; case 0xB1: return 0x2592; case 0xB2: return 0x2593; case 0xB3: return 0x2502;
        case 0xB4: return 0x2524; case 0xB5: return 0x2561; case 0xB6: return 0x2562; case 0xB7: return 0x2556;
        case 0xB8: return 0x2555; case 0xB9: return 0x2563; case 0xBA: return 0x2551; case 0xBB: return 0x2557;
        case 0xBC: return 0x255D; case 0xBD: return 0x255C; case 0xBE: return 0x255B; case 0xBF: return 0x2510;
        case 0xC0: return 0x2514; case 0xC1: return 0x2534; case 0xC2: return 0x252C; case 0xC3: return 0x251C;
        case 0xC4: return 0x2500; case 0xC5: return 0x253C; case 0xC6: return 0x255E; case 0xC7: return 0x255F;
        case 0xC8: return 0x255A; case 0xC9: return 0x2554; case 0xCA: return 0x2569; case 0xCB: return 0x2566;
        case 0xCC: return 0x2560; case 0xCD: return 0x2550; case 0xCE: return 0x256C; case 0xCF: return 0x2567;
        case 0xD0: return 0x2568; case 0xD1: return 0x2564; case 0xD2: return 0x2565; case 0xD3: return 0x2559;
        case 0xD4: return 0x2558; case 0xD5: return 0x2552; case 0xD6: return 0x2553; case 0xD7: return 0x256B;
        case 0xD8: return 0x256A; case 0xD9: return 0x2518; case 0xDA: return 0x250C; case 0xDB: return 0x2588;
        case 0xDC: return 0x2584; case 0xDD: return 0x258C; case 0xDE: return 0x2590; case 0xDF: return 0x2580;
        case 0xE0: return 0x03B1; case 0xE1: return 0x00DF; case 0xE2: return 0x0393; case 0xE3: return 0x03C0;
        case 0xE4: return 0x03A3; case 0xE5: return 0x03C3; case 0xE6: return 0x00B5; case 0xE7: return 0x03C4;
        case 0xE8: return 0x03A6; case 0xE9: return 0x0398; case 0xEA: return 0x03A9; case 0xEB: return 0x03B4;
        case 0xEC: return 0x221E; case 0xED: return 0x03C6; case 0xEE: return 0x03B5; case 0xEF: return 0x2229;
        case 0xF0: return 0x2261; case 0xF1: return 0x00B1; case 0xF2: return 0x2265; case 0xF3: return 0x2264;
        case 0xF4: return 0x2320; case 0xF5: return 0x2321; case 0xF6: return 0x00F7; case 0xF7: return 0x2248;
        case 0xF8: return 0x00B0; case 0xF9: return 0x2219; case 0xFA: return 0x00B7; case 0xFB: return 0x221A;
        case 0xFC: return 0x207F; case 0xFD: return 0x00B2; case 0xFE: return 0x25A0; case 0xFF: return 0x00A0;
    default:
        return c;
    }
}

static std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::istringstream ss(s);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

static std::size_t get_grapheme_count(const std::string &s)
{
    auto it = s.begin();
    std::size_t n = 0;

    while (it != s.end()) {
        std::size_t bytes = 1;

        if ((*it & 0b10000000) != 0) {
            while (((*it << bytes) & 0b11000000) == 0b10000000) {
                bytes++;
            }
        }

        it += bytes;
        n++;
    }

    return n;
}

static void check_messages(const std::string &filename, const openmpt::module &mod)
{
    auto message = mod.get_metadata("message_raw");
    if (message.empty()) {
        return;
    }

    auto codepoints = utf8_to_codepoints(message);
    std::vector<std::uint32_t> converted;

    std::transform(codepoints.begin(), codepoints.end(), std::back_inserter(converted), cp437_to_unicode);

    auto new_message = codepoints_to_utf8(converted);

    auto message_lines = split(message, '\n');
    auto new_message_lines = split(new_message, '\n');

    if (message_lines.size() != new_message_lines.size()) {
        throw std::runtime_error("internal error: size mismatch");
    }

    if (message == new_message) {
        return;
    }

    std::cout << "Difference in " << filename << ":\n\n";

    for (std::size_t i = 0; i < message_lines.size(); i++) {
        if (diff_only && message_lines[i] == new_message_lines[i]) {
            continue;
        }

        auto graphemes = get_grapheme_count(message_lines[i]);
        std::cout << message_lines[i];
        if (graphemes < 80) {
            std::string padding(80 - graphemes, ' ');
            std::cout << padding;
        }

        std::cout << " | " << new_message_lines[i] << std::endl;
    }

    std::cout << std::endl;
}

static void process_file(const std::string &filename)
{
    std::ifstream file(filename);

    try {
        openmpt::module mod(file);
        check_messages(filename, mod);
    } catch (const std::exception &e) {
        std::cerr << "can't open " << filename << ": " << e.what() << std::endl;
    }
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        process_file(argv[i]);
    }

    return 0;
}
