#include "utils/fixedpt.h"

size_t fixedpt_str(fixedpt A, char *str, size_t bufsize, int max_dec) {
    // boy this function sure is safe with pointers :(
    size_t nint = 0;  // number of integer digits
    size_t nfrac = 0; // number of fractional digits
    size_t slen = 0;  // string length used

    // accumulator for integer digits, can hold the max number of digits for FIXEDPT_BITS
    char acc[FIXEDPT_WCHARS] = {0};
    fixedptud fr, ip;
    fixedptud magnitude = (fixedptud)(fixedptd)A;
    const fixedptud one = (fixedptud)1 << FIXEDPT_BITS;
    const fixedptud mask = one - 1;
    size_t num_places = 0;

    // return if the destination is null or too small
    if(str == NULL || bufsize <= 0) {
        return 0;
    } else if(bufsize < 2) {
        str[0] = '\0';
        return 0;
    }

    if(max_dec == -1) {
#if FIXEDPT_BITS == 32
#if FIXEDPT_FBITS < 16
        num_places = 2;
#else
        num_places = 4;
#endif
#elif FIXEDPT_BITS == 64
        num_places = 10;
#else
#error Invalid width
#endif
    } else if(max_dec == -2) {
        num_places = 15;
    } else if(max_dec == -3) {
        num_places = 15;
    } else if(max_dec < 0) {
        // other negative values of max_dec not allowed
        return 0;
    } else {
        num_places = max_dec;
    }

    if(A < 0) {
        str[slen++] = '-';
        magnitude *= -1;
    }

    // Accumulate the integer digits in reverse order
    // by dividing the integer component by 10 each time.
    // Use do/while so at least a 0 is written
    ip = fixedpt_toint(magnitude);
    do {
        acc[nint++] = '0' + ip % 10;
        ip /= 10;
    } while(ip != 0 && nint < sizeof(acc) - 1);

    // reverse the accumulated digits into the output string
    while(nint > 0 && slen < bufsize - 1) {
        str[slen++] = acc[--nint];
    }

    // check if we want, and have space for, decimal digits
    if(num_places > 0 && slen < bufsize - 1) {
        str[slen++] = '.';

        fr = (fixedpt_fracpart(magnitude) << FIXEDPT_WBITS) & mask;
        while(nfrac < num_places && slen < bufsize - 1) {
            fr = (fr & mask) * 10;

            str[slen++] = '0' + (fr >> FIXEDPT_BITS) % 10;
            nfrac++;
        }

        // trim any trailing 0s, if desired
        while(max_dec == -3 && nfrac > 0 && str[slen - 1] == '0') {
            slen--;
            nfrac--;
        }

        // if we didn't add any decimal values, omit the decimal place
        if(nfrac == 0) {
            slen--;
        }
    }

    size_t bytes_used = slen < bufsize ? slen : bufsize - 1;
    str[bytes_used] = '\0';
    return bytes_used;
}
