/*! \file
 * \brief Text conversion routines between CP 437 and UTF-8.
 * \details OMF 2097 uses DOS Code Page 437 for its text with two exceptions:
    characters below 0x20 are treated as control characters,
    and 0xA9..=0xDF aren't used.

    TODO: check NETARENA.PCX, NETFONT1.PCX, and NETFONT2.PCX graphics

    This header uses `unsigned char` to store UTF-8 and `uint8_t` to store CP437 because:
    - C23 defines char8_t for storing UTF-8, and requires it be the same type as unsigned char.
    - I felt like storing CP437 in uint8_t.

    This header does not require your strings to contain NUL terminators, instead
    all functions expect you to supply a length.

 * \copyright MIT license.
 * \date 2024
 * \author Magnus Larsen
 */

#ifndef CP437_H
#define CP437_H

#include <stdint.h> // uint8_t
#include <uchar.h>  // char32_t

// TODO: Use warn_unused_result on these methods.
// TODO: Maybe expand cp437_result to signal what type of invalid UTF-8 was encountered, or where the error occured?

typedef enum cp437_result
{
    CP437_SUCCESS,
    CP437_ERROR_UNKNOWN_CODEPOINT,
    CP437_ERROR_INVALID_UTF8,
} cp437_result;

#define CP437_MAX_UTF8_PER_CP437 3

/*! \brief Convert a UTF-8 string to CP437
 *
 * Input range 0x00..=0x1F are written as-is, as Open OMF treats these as control characters.
 * Characters that translate to DOS 0xA9.=0xDF aren't supported, as the OMF fonts don't have useful glyphs there.
 *
 * At least one of out_cp437 or out_cp437_len should be non-NULL, or this function will assert (and do nothing).
 *
 * \retval CP437_ERROR_UNKNOWN_CODEPOINT The input contained a codepoint we cannot represent in CP437.
 * \retval CP437_ERROR_INVALID_UTF8 The input was not valid UTF-8.
 * \retval CP437_SUCCESS Encoding was successful.
 *
 * \param out_cp437 If non-null, the CP437 bytes that correspond to the input string will be written here
 * \param out_cp437_len If non-null, the number of CP437 bytes that correspond to the input string will be written here
 * \param utf8 The input UTF-8 string, such as u8"Muß".
 */
cp437_result cp437_from_utf8(uint8_t *out_cp437, size_t *out_cp437_len, unsigned char const *utf8, size_t utf8_len);

/*! \brief Convert a CP437 string to UTF-8
 *
 * Input range 0x00..=0x1F are written as-is, as Open OMF treats these as control characters.
 *
 * At least one of out_utf8 or out_utf8_len should be non-NULL, or this function will assert (and do nothing).
 *
 * \param out_utf8 If non-null, a location to write the utf-8 bytes.
 * NOTE: CP437 reencoded as UTF-8 might use as much as CP437_MAX_UTF8_PER_CP437 times larger! Beware pointer overwrite.
 * \param out_utf8_len A pointer to store the number of UTF-8 bytes.
 * \param cp437 A non-null pointer to an array of bytes to interpret in code page 437.
 * \param cp437_len The length of the cp437 array.
 */
void cp437_to_utf8(unsigned char *out_utf8, size_t *out_utf8_len, uint8_t const *cp437, size_t cp437_len);

/*! \brief Convert a single character from UTF-32 to CP437
 *
 * Input range 0x0000..=0x001F are written as-is, as Open OMF treats these as control characters.
 * Characters that translate to DOS 0xA9.=0xDF aren't supported, as the OMF fonts don't have useful glyphs there.
 *
 * \retval CP437_ERROR_UNKNOWN_CODEPOINT There was a code point
 * \retval CP437_SUCCESS Encoding was successful.
 *
 * \param out_cp437 On success, one CP437 character will be written here.
 * \param utf32 The input UTF-32 codepoint, such as U'ß'
 */
cp437_result cp437_from_utf32(uint8_t *out_cp437, char32_t utf32);

/*! \brief Convert a single character from CP437 to UTF-32
 *
 * Input range 0x00..=0x1F are written as-is, as Open OMF treats these as control characters.
 *
 * \param out_utf32 On success, one UTF-32 character will be written here.
 * \param cp437 The input CP 437 character, such as 0xE1 (aka ß)
 */
void cp437_to_utf32(char32_t *out_utf32, uint8_t cp437);

#endif // CP437_H
