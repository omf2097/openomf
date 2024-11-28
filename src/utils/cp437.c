#include "cp437.h"
#include "utils/c_array_util.h"
#include <assert.h>

char const *cp437_result_to_string(cp437_result result) {
    switch(result) {
        case CP437_SUCCESS:
            return "CP437_SUCCESS";
        case CP437_ERROR_UNKNOWN_CODEPOINT:
            return "CP437_ERROR_UNKNOWN_CODEPOINT";
        case CP437_ERROR_INVALID_UTF8:
            return "CP437_ERROR_INVALID_UTF8";
        case CP437_ERROR_OUTPUTBUFFER_TOOSMALL:
            return "CP437_ERROR_OUTPUTBUFFER_TOOSMALL";
        default:
            assert(0);
            return "! invalid cp437_result !";
    }
}

// lookup table for cp437->UTF-32
char32_t const cp437_toutf32_lookup[] = {
    // clang-format off
    // 0     1     2     3     4     5     6     7      8     9     A     B     C      D     E     F
    U'\0', U'☺', U'☻', U'♥', U'♦', U'♣', U'♠', U'•',  U'◘', U'○', U'◙', U'♂', U'♀',  U'♪', U'♫', U'☼',      // 0
    U'►',  U'◄', U'↕', U'‼', U'¶', U'§', U'▬', U'↨',  U'↑', U'↓', U'→', U'←', U'∟',  U'↔', U'▲', U'▼',      // 1
    U' ',  U'!', U'"', U'#', U'$', U'%', U'&', U'\'', U'(', U')', U'*', U'+', U',',  U'-', U'.', U'/',      // 2
    U'0',  U'1', U'2', U'3', U'4', U'5', U'6', U'7',  U'8', U'9', U':', U';', U'<',  U'=', U'>', U'?',      // 3
    U'@',  U'A', U'B', U'C', U'D', U'E', U'F', U'G',  U'H', U'I', U'J', U'K', U'L',  U'M', U'N', U'O',      // 4
    U'P',  U'Q', U'R', U'S', U'T', U'U', U'V', U'W',  U'X', U'Y', U'Z', U'[', U'\\', U']', U'^', U'_',      // 5
    U'`',  U'a', U'b', U'c', U'd', U'e', U'f', U'g',  U'h', U'i', U'j', U'k', U'l',  U'm', U'n', U'o',      // 6
    U'p',  U'q', U'r', U's', U't', U'u', U'v', U'w',  U'x', U'y', U'z', U'{', U'|',  U'}', U'~', U'⌂',      // 7
    U'Ç',  U'ü', U'é', U'â', U'ä', U'à', U'å', U'ç',  U'ê', U'ë', U'è', U'ï', U'î',  U'ì', U'Ä', U'Å',      // 8
    U'É',  U'æ', U'Æ', U'ô', U'ö', U'ò', U'û', U'ù',  U'ÿ', U'Ö', U'Ü', U'¢', U'£',  U'¥', U'₧', U'ƒ',      // 9
    U'á',  U'í', U'ó', U'ú', U'ñ', U'Ñ', U'ª', U'º',  U'¿', U'⌐', U'¬', U'½', U'¼',  U'¡', U'«', U'»',      // A
    U'░',  U'▒', U'▓', U'│', U'┤', U'╡', U'╢', U'╖',  U'╕', U'╣', U'║', U'╗', U'╝',  U'╜', U'╛', U'┐',      // B
    U'└',  U'┴', U'┬', U'├', U'─', U'┼', U'╞', U'╟',  U'╚', U'╔', U'╩', U'╦', U'╠',  U'═', U'╬', U'╧',      // C
    U'╨',  U'╤', U'╥', U'╙', U'╘', U'╒', U'╓', U'╫',  U'╪', U'┘', U'┌', U'█', U'▄',  U'▌', U'▐', U'▀',      // D
    U'α',  U'ß', U'Γ', U'π', U'Σ', U'σ', U'µ', U'τ',  U'Φ', U'Θ', U'Ω', U'δ', U'∞',  U'φ', U'ε', U'∩',      // E
    U'≡',  U'±', U'≥', U'≤', U'⌠', U'⌡', U'÷', U'≈',  U'°', U'∙', U'·', U'√', U'ⁿ',  U'²', U'■', U'\u00A0', // F
    // clang-format on
};

static_assert(256 == N_ELEMENTS(cp437_toutf32_lookup), "cp437 lookup table must be 256 entries long");

inline static size_t code_utf8len(char32_t utf32) {
    if(utf32 <= 0x7F) {
        return 1;
    } else if(utf32 <= 0x7FF) {
        return 2;
    } else if(utf32 <= 0xFFFF) {
        return 3;
    }
// CP437 doesn't contain any Unicode codepoints above U+FFFF
// see also: CP437_MAX_UTF8_PER_CP437
#if 1
    assert(utf32 <= 0x10FFFF);
#else
    else if(utf32 <= 0x10FFFF) {
        return 4;
    }
#endif
    return 0;
}

static size_t code_to_utf8(unsigned char *buffer, char32_t utf32) {
    switch(code_utf8len(utf32)) {
        default:
            assert(0);
        case 0:
            return 0;
        case 1:
            buffer[0] = utf32;
            return 1;
        case 2:
            buffer[0] = 0xC0 | (utf32 >> 6);   /* 110xxxxx */
            buffer[1] = 0x80 | (utf32 & 0x3F); /* 10xxxxxx */
            return 2;
        case 3:
            buffer[0] = 0xE0 | (utf32 >> 12);         /* 1110xxxx */
            buffer[1] = 0x80 | ((utf32 >> 6) & 0x3F); /* 10xxxxxx */
            buffer[2] = 0x80 | (utf32 & 0x3F);        /* 10xxxxxx */
            return 3;
        case 4:
            buffer[0] = 0xF0 | (utf32 >> 18);          /* 11110xxx */
            buffer[1] = 0x80 | ((utf32 >> 12) & 0x3F); /* 10xxxxxx */
            buffer[2] = 0x80 | ((utf32 >> 6) & 0x3F);  /* 10xxxxxx */
            buffer[3] = 0x80 | (utf32 & 0x3F);         /* 10xxxxxx */
            return 4;
    }
}

static cp437_result next_utf32(char32_t *out_utf32, unsigned char const **utf8, size_t *utf8_len) {
    assert(out_utf32 && utf8 && *utf8 && utf8_len && *utf8_len);
    unsigned char first_byte = **utf8;
    size_t advance;
    char32_t utf32;
    switch(first_byte >> 4) {
        // 0b1111
        case 0xF:
            // first_byte & 0b1111'1000 != 0b1111'0xxx
            if((first_byte & 0xF8) != 0xF0) {
                // 0b1111'1xxx is not a valid first_byte of UTF-8
                return CP437_ERROR_INVALID_UTF8;
            }
            // 0b1111'0xxx (and then 3 continuation bytes)
            utf32 = first_byte & 0x07;
            advance = 4;
            break;
        // 0b1110
        case 0xE:
            // 0b1110'xxxx (and then 2 continuation bytes)
            utf32 = first_byte & 0x0F;
            advance = 3;
            break;
        // 0b110x
        case 0xD:
        case 0xC:
            // 0b110x'xxxx (and then 1 continuation byte)
            utf32 = first_byte & 0x1F;
            advance = 2;
            break;
        // 0b10xx
        case 0xB:
        case 0xA:
        case 0x9:
        case 0x8:
            // unexpected continuation byte
            return CP437_ERROR_INVALID_UTF8;
        // 0b0xxx
        default:
            // 0b0xxx'xxxx (no continuation bytes)
            utf32 = first_byte;
            advance = 1;
            break;
    }
    if(advance > *utf8_len) {
        // truncated UTF-8
        return CP437_ERROR_INVALID_UTF8;
    }

    // read continuation bytes
    for(size_t cont = 1; cont < advance; cont++) {
        unsigned char cont_byte = (*utf8)[cont];
        // cont_byte & 0b1100'0000 != 0b10xx'xxxx
        if((cont_byte & 0xC0) != 0x80) {
            // expected continuation byte
            return CP437_ERROR_INVALID_UTF8;
        }
        cont_byte &= 0x7F;
        utf32 <<= 6;
        utf32 |= cont_byte;
    }

    *out_utf32 = utf32;

    *utf8 += advance;
    *utf8_len -= advance;
    return CP437_SUCCESS;
}

cp437_result cp437_from_utf8(uint8_t *out_cp437, size_t sizeof_out_cp437, size_t *out_cp437_len,
                             unsigned char const *utf8, size_t utf8_len) {
    assert(utf8);
    size_t cp437_len = 0;
    uint8_t *out_cp437_end = NULL;
    if(out_cp437 != NULL) {
        out_cp437_end = out_cp437 + sizeof_out_cp437;
    }
    while(utf8_len > 0) {
        if(out_cp437 && out_cp437 >= out_cp437_end) {
            return CP437_ERROR_OUTPUTBUFFER_TOOSMALL;
        }
        char32_t utf32;
        cp437_result result = next_utf32(&utf32, &utf8, &utf8_len);
        if(result != CP437_SUCCESS) {
            if(out_cp437_len)
                *out_cp437_len = 0;
            return result;
        }
        uint8_t cp437;
        result = cp437_from_utf32(&cp437, utf32);
        if(result != CP437_SUCCESS) {
            if(out_cp437_len)
                *out_cp437_len = 0;
            return result;
        }
        if(out_cp437) {
            *out_cp437++ = cp437;
        }
        cp437_len++;
    }
    if(out_cp437_len)
        *out_cp437_len = cp437_len;
    return CP437_SUCCESS;
}

cp437_result cp437_to_utf8(unsigned char *out_utf8, size_t sizeof_out_utf8, size_t *out_utf8_len, uint8_t const *cp437,
                           size_t cp437_len) {
    assert(cp437);
    assert(out_utf8 || out_utf8_len);
    if(out_utf8_len) {
        *out_utf8_len = 0;
    }
    unsigned char *out_utf8_end = NULL;
    if(out_utf8 != NULL) {
        out_utf8_end = out_utf8 + sizeof_out_utf8;
    }
    while(cp437_len > 0) {
        char32_t utf32;
        cp437_to_utf32(&utf32, *cp437);
        size_t utf8_advance = code_utf8len(utf32);
        if(out_utf8) {
            if(out_utf8 + utf8_advance > out_utf8_end) {
                return CP437_ERROR_OUTPUTBUFFER_TOOSMALL;
            }
            code_to_utf8(out_utf8, utf32);
            out_utf8 += utf8_advance;
        }
        if(out_utf8_len) {
            *out_utf8_len += utf8_advance;
        }
        cp437++;
        cp437_len--;
    }
    return CP437_SUCCESS;
}

void cp437_to_utf32(char32_t *out_utf32, uint8_t cp437) {
    assert(out_utf32);
    if(cp437 < 0x20) {
        // control character
        *out_utf32 = (char32_t)cp437;
        return;
    }
    *out_utf32 = cp437_toutf32_lookup[cp437];
}

cp437_result cp437_from_utf32(uint8_t *out_cp437, char32_t utf32) {
    assert(out_cp437);
    if(utf32 < 0x80) {
        // Map ASCII found in UTF strings 1:1
        *out_cp437 = (uint8_t)utf32;
        return CP437_SUCCESS;
    }

    // XXX Adding an optimized version of this for the N64 could be a fun exercise-- memory bandwidth is at a premium.

    // XXX Probably better to use a perfect hash table rather than leaving the implementation to the compiler

    // giant switch statement to let the compiler optimize it as it pleases
    switch(utf32) {
        case U'⌂':
            *out_cp437 = 0x7F;
            return CP437_SUCCESS;
        case U'Ç':
            *out_cp437 = 0x80;
            return CP437_SUCCESS;
        case U'ü':
            *out_cp437 = 0x81;
            return CP437_SUCCESS;
        case U'é':
            *out_cp437 = 0x82;
            return CP437_SUCCESS;
        case U'â':
            *out_cp437 = 0x83;
            return CP437_SUCCESS;
        case U'ä':
            *out_cp437 = 0x84;
            return CP437_SUCCESS;
        case U'à':
            *out_cp437 = 0x85;
            return CP437_SUCCESS;
        case U'å':
            *out_cp437 = 0x86;
            return CP437_SUCCESS;
        case U'ç':
            *out_cp437 = 0x87;
            return CP437_SUCCESS;
        case U'ê':
            *out_cp437 = 0x88;
            return CP437_SUCCESS;
        case U'ë':
            *out_cp437 = 0x89;
            return CP437_SUCCESS;
        case U'è':
            *out_cp437 = 0x8A;
            return CP437_SUCCESS;
        case U'ï':
            *out_cp437 = 0x8B;
            return CP437_SUCCESS;
        case U'î':
            *out_cp437 = 0x8C;
            return CP437_SUCCESS;
        case U'ì':
            *out_cp437 = 0x8D;
            return CP437_SUCCESS;
        case U'Ä':
            *out_cp437 = 0x8E;
            return CP437_SUCCESS;
        case U'Å':
            *out_cp437 = 0x8F;
            return CP437_SUCCESS;
        case U'É':
            *out_cp437 = 0x90;
            return CP437_SUCCESS;
        case U'æ':
            *out_cp437 = 0x91;
            return CP437_SUCCESS;
        case U'Æ':
            *out_cp437 = 0x92;
            return CP437_SUCCESS;
        case U'ô':
            *out_cp437 = 0x93;
            return CP437_SUCCESS;
        case U'ö':
            *out_cp437 = 0x94;
            return CP437_SUCCESS;
        case U'ò':
            *out_cp437 = 0x95;
            return CP437_SUCCESS;
        case U'û':
            *out_cp437 = 0x96;
            return CP437_SUCCESS;
        case U'ù':
            *out_cp437 = 0x97;
            return CP437_SUCCESS;
        case U'ÿ':
            *out_cp437 = 0x98;
            return CP437_SUCCESS;
        case U'Ö':
            *out_cp437 = 0x99;
            return CP437_SUCCESS;
        case U'Ü':
            *out_cp437 = 0x9A;
            return CP437_SUCCESS;
        case U'¢':
            *out_cp437 = 0x9B;
            return CP437_SUCCESS;
        case U'£':
            *out_cp437 = 0x9C;
            return CP437_SUCCESS;
        case U'¥':
            *out_cp437 = 0x9D;
            return CP437_SUCCESS;
        case U'₧':
            *out_cp437 = 0x9E;
            return CP437_SUCCESS;
        case U'ƒ':
            *out_cp437 = 0x9F;
            return CP437_SUCCESS;
        case U'á':
            *out_cp437 = 0xA0;
            return CP437_SUCCESS;
        case U'í':
            *out_cp437 = 0xA1;
            return CP437_SUCCESS;
        case U'ó':
            *out_cp437 = 0xA2;
            return CP437_SUCCESS;
        case U'ú':
            *out_cp437 = 0xA3;
            return CP437_SUCCESS;
        case U'ñ':
            *out_cp437 = 0xA4;
            return CP437_SUCCESS;
        case U'Ñ':
            *out_cp437 = 0xA5;
            return CP437_SUCCESS;
        case U'ª':
            *out_cp437 = 0xA6;
            return CP437_SUCCESS;
        case U'º':
            *out_cp437 = 0xA7;
            return CP437_SUCCESS;
        case U'¿':
            *out_cp437 = 0xA8;
            return CP437_SUCCESS;
// OMF glyphs 0xA9..=0xDF aren't worth mapping
#if 0
        case U'⌐':
            *out_cp437 = 0xA9;
            return CP437_SUCCESS;
        case U'¬':
            *out_cp437 = 0xAA;
            return CP437_SUCCESS;
        case U'½':
            *out_cp437 = 0xAB;
            return CP437_SUCCESS;
        case U'¼':
            *out_cp437 = 0xAC;
            return CP437_SUCCESS;
        case U'¡':
            *out_cp437 = 0xAD;
            return CP437_SUCCESS;
        case U'«':
            *out_cp437 = 0xAE;
            return CP437_SUCCESS;
        case U'»':
            *out_cp437 = 0xAF;
            return CP437_SUCCESS;
        case U'░':
            *out_cp437 = 0xB0;
            return CP437_SUCCESS;
        case U'▒':
            *out_cp437 = 0xB1;
            return CP437_SUCCESS;
        case U'▓':
            *out_cp437 = 0xB2;
            return CP437_SUCCESS;
        case U'│':
            *out_cp437 = 0xB3;
            return CP437_SUCCESS;
        case U'┤':
            *out_cp437 = 0xB4;
            return CP437_SUCCESS;
        case U'╡':
            *out_cp437 = 0xB5;
            return CP437_SUCCESS;
        case U'╢':
            *out_cp437 = 0xB6;
            return CP437_SUCCESS;
        case U'╖':
            *out_cp437 = 0xB7;
            return CP437_SUCCESS;
        case U'╕':
            *out_cp437 = 0xB8;
            return CP437_SUCCESS;
        case U'╣':
            *out_cp437 = 0xB9;
            return CP437_SUCCESS;
        case U'║':
            *out_cp437 = 0xBA;
            return CP437_SUCCESS;
        case U'╗':
            *out_cp437 = 0xBB;
            return CP437_SUCCESS;
        case U'╝':
            *out_cp437 = 0xBC;
            return CP437_SUCCESS;
        case U'╜':
            *out_cp437 = 0xBD;
            return CP437_SUCCESS;
        case U'╛':
            *out_cp437 = 0xBE;
            return CP437_SUCCESS;
        case U'┐':
            *out_cp437 = 0xBF;
            return CP437_SUCCESS;
        case U'└':
            *out_cp437 = 0xC0;
            return CP437_SUCCESS;
        case U'┴':
            *out_cp437 = 0xC1;
            return CP437_SUCCESS;
        case U'┬':
            *out_cp437 = 0xC2;
            return CP437_SUCCESS;
        case U'├':
            *out_cp437 = 0xC3;
            return CP437_SUCCESS;
        case U'─':
            *out_cp437 = 0xC4;
            return CP437_SUCCESS;
        case U'┼':
            *out_cp437 = 0xC5;
            return CP437_SUCCESS;
        case U'╞':
            *out_cp437 = 0xC6;
            return CP437_SUCCESS;
        case U'╟':
            *out_cp437 = 0xC7;
            return CP437_SUCCESS;
        case U'╚':
            *out_cp437 = 0xC8;
            return CP437_SUCCESS;
        case U'╔':
            *out_cp437 = 0xC9;
            return CP437_SUCCESS;
        case U'╩':
            *out_cp437 = 0xCA;
            return CP437_SUCCESS;
        case U'╦':
            *out_cp437 = 0xCB;
            return CP437_SUCCESS;
        case U'╠':
            *out_cp437 = 0xCC;
            return CP437_SUCCESS;
        case U'═':
            *out_cp437 = 0xCD;
            return CP437_SUCCESS;
        case U'╬':
            *out_cp437 = 0xCE;
            return CP437_SUCCESS;
        case U'╧':
            *out_cp437 = 0xCF;
            return CP437_SUCCESS;
        case U'╨':
            *out_cp437 = 0xD0;
            return CP437_SUCCESS;
        case U'╤':
            *out_cp437 = 0xD1;
            return CP437_SUCCESS;
        case U'╥':
            *out_cp437 = 0xD2;
            return CP437_SUCCESS;
        case U'╙':
            *out_cp437 = 0xD3;
            return CP437_SUCCESS;
        case U'╘':
            *out_cp437 = 0xD4;
            return CP437_SUCCESS;
        case U'╒':
            *out_cp437 = 0xD5;
            return CP437_SUCCESS;
        case U'╓':
            *out_cp437 = 0xD6;
            return CP437_SUCCESS;
        case U'╫':
            *out_cp437 = 0xD7;
            return CP437_SUCCESS;
        case U'╪':
            *out_cp437 = 0xD8;
            return CP437_SUCCESS;
        case U'┘':
            *out_cp437 = 0xD9;
            return CP437_SUCCESS;
        case U'┌':
            *out_cp437 = 0xDA;
            return CP437_SUCCESS;
        case U'█':
            *out_cp437 = 0xDB;
            return CP437_SUCCESS;
        case U'▄':
            *out_cp437 = 0xDC;
            return CP437_SUCCESS;
        case U'▌':
            *out_cp437 = 0xDD;
            return CP437_SUCCESS;
        case U'▐':
            *out_cp437 = 0xDE;
            return CP437_SUCCESS;
        case U'▀':
            *out_cp437 = 0xDF;
            return CP437_SUCCESS;
#endif // 0
        case U'α':
            *out_cp437 = 0xE0;
            return CP437_SUCCESS;
        case U'ß':
            *out_cp437 = 0xE1;
            return CP437_SUCCESS;
        case U'Γ':
            *out_cp437 = 0xE2;
            return CP437_SUCCESS;
        case U'π':
            *out_cp437 = 0xE3;
            return CP437_SUCCESS;
        case U'Σ':
            *out_cp437 = 0xE4;
            return CP437_SUCCESS;
        case U'σ':
            *out_cp437 = 0xE5;
            return CP437_SUCCESS;
        case U'µ':
            *out_cp437 = 0xE6;
            return CP437_SUCCESS;
        case U'τ':
            *out_cp437 = 0xE7;
            return CP437_SUCCESS;
        case U'Φ':
            *out_cp437 = 0xE8;
            return CP437_SUCCESS;
        case U'Θ':
            *out_cp437 = 0xE9;
            return CP437_SUCCESS;
        case U'Ω':
            *out_cp437 = 0xEA;
            return CP437_SUCCESS;
        case U'δ':
            *out_cp437 = 0xEB;
            return CP437_SUCCESS;
        case U'∞':
            *out_cp437 = 0xEC;
            return CP437_SUCCESS;
        case U'φ':
            *out_cp437 = 0xED;
            return CP437_SUCCESS;
        case U'ε':
            *out_cp437 = 0xEE;
            return CP437_SUCCESS;
        case U'∩':
            *out_cp437 = 0xEF;
            return CP437_SUCCESS;
        case U'≡':
            *out_cp437 = 0xF0;
            return CP437_SUCCESS;
        case U'±':
            *out_cp437 = 0xF1;
            return CP437_SUCCESS;
        case U'≥':
            *out_cp437 = 0xF2;
            return CP437_SUCCESS;
        case U'≤':
            *out_cp437 = 0xF3;
            return CP437_SUCCESS;
        case U'⌠':
            *out_cp437 = 0xF4;
            return CP437_SUCCESS;
        case U'⌡':
            *out_cp437 = 0xF5;
            return CP437_SUCCESS;
        case U'÷':
            *out_cp437 = 0xF6;
            return CP437_SUCCESS;
        case U'≈':
            *out_cp437 = 0xF7;
            return CP437_SUCCESS;
        case U'°':
            *out_cp437 = 0xF8;
            return CP437_SUCCESS;
        case U'∙':
            *out_cp437 = 0xF9;
            return CP437_SUCCESS;
        case U'·':
            *out_cp437 = 0xFA;
            return CP437_SUCCESS;
        case U'√':
            *out_cp437 = 0xFB;
            return CP437_SUCCESS;
        case U'ⁿ':
            *out_cp437 = 0xFC;
            return CP437_SUCCESS;
        case U'²':
            *out_cp437 = 0xFD;
            return CP437_SUCCESS;
        case U'■':
            *out_cp437 = 0xFE;
            return CP437_SUCCESS;
        case U'\u00A0':
            *out_cp437 = 0xFF;
            return CP437_SUCCESS;
        default:
            *out_cp437 = 0x21; // '!'
            // printf("Unknown codepoint U+%04X\n", utf32);
            return CP437_ERROR_UNKNOWN_CODEPOINT;
    }
}
