#include "resources/palette.h"

void fixup_palette(palette *palette) {
    // XXX just cram some known-good values in the player part of the palette for now
    
    /*palette->data[0][0] = 147;*/
    /*palette->data[0][1] = 0;*/
    /*palette->data[0][2] = 0;*/

    palette->data[1][0] = 0;
    palette->data[1][1] = 7;
    palette->data[1][2] = 15;

    palette->data[2][0] = 0;
    palette->data[2][1] = 11;
    palette->data[2][1] = 31;

    palette->data[3][0] = 0;
    palette->data[3][1] = 15;
    palette->data[3][2] = 51;

    palette->data[4][0] = 0;
    palette->data[4][1] = 23;
    palette->data[4][2] = 67;

    palette->data[4][0] = 0;
    palette->data[4][1] = 23;
    palette->data[4][2] = 67;

    palette->data[5][0] = 0;
    palette->data[5][1] = 23;
    palette->data[5][2] = 67;

    palette->data[6][0] = 0;
    palette->data[6][1] = 31;
    palette->data[6][2] = 99;

    palette->data[7][0] = 0;
    palette->data[7][1] = 39;
    palette->data[7][2] = 119;

    palette->data[8][0] = 0;
    palette->data[8][1] = 43;
    palette->data[8][2] = 135;

    palette->data[9][0] = 0;
    palette->data[9][1] = 51;
    palette->data[9][2] = 151;

    palette->data[10][0] = 0;
    palette->data[10][1] = 55;
    palette->data[10][2] = 167;

    palette->data[11][0] = 0;
    palette->data[11][1] = 63;
    palette->data[11][2] = 187;

    palette->data[12][0] = 0;
    palette->data[12][1] = 67;
    palette->data[12][2] = 203;

    palette->data[13][0] = 0;
    palette->data[13][1] = 75;
    palette->data[13][2] = 219;

    palette->data[14][0] = 0;
    palette->data[14][1] = 79;
    palette->data[14][2] = 235;

    palette->data[15][0] = 0;
    palette->data[15][1] = 87;
    palette->data[15][2] = 255;

    palette->data[16][0] = 21;
    palette->data[16][1] = 10;
    palette->data[16][2] = 5;

    palette->data[17][0] = 15;
    palette->data[17][1] = 15;
    palette->data[17][2] = 7;

    palette->data[18][0] = 27;
    palette->data[18][1] = 31;
    palette->data[18][2] = 19;

    palette->data[19][0] = 43;
    palette->data[19][1] = 51;
    palette->data[19][2] = 27;

    palette->data[20][0] = 59;
    palette->data[20][1] = 67;
    palette->data[20][2] = 35;

    palette->data[21][0] = 71;
    palette->data[21][1] = 83;
    palette->data[21][2] = 47;

    palette->data[22][0] = 89;
    palette->data[22][1] = 99;
    palette->data[22][2] = 55;

    palette->data[23][0] = 102;
    palette->data[23][1] = 119;
    palette->data[23][2] = 67;

    palette->data[24][0] = 119;
    palette->data[24][1] = 135;
    palette->data[24][2] = 75;

    palette->data[25][0] = 131;
    palette->data[25][1] = 151;
    palette->data[25][2] = 83;

    palette->data[26][0] = 147;
    palette->data[26][1] = 167;
    palette->data[26][2] = 95;

    palette->data[27][0] = 163;
    palette->data[27][1] = 187;
    palette->data[27][2] = 103;

    palette->data[28][0] = 175;
    palette->data[28][1] = 203;
    palette->data[28][2] = 111;

    palette->data[29][0] = 191;
    palette->data[29][1] = 219;
    palette->data[29][2] = 123;

    palette->data[30][0] = 207;
    palette->data[30][1] = 235;
    palette->data[30][2] = 131;

    palette->data[31][0] = 223;
    palette->data[31][1] = 255;
    palette->data[31][2] = 143;

    palette->data[32][0] = 15;
    palette->data[32][1] = 16;
    palette->data[32][2] = 53;

    palette->data[33][0] = 15;
    palette->data[33][1] = 23;
    palette->data[33][2] = 59;

    palette->data[34][0] = 19;
    palette->data[34][1] = 27;
    palette->data[34][2] = 63;

    palette->data[35][0] = 23;
    palette->data[35][1] = 31;
    palette->data[35][2] = 71;

    palette->data[36][0] = 23;
    palette->data[36][1] = 35;
    palette->data[36][2] = 75;

    palette->data[37][0] = 27;
    palette->data[37][1] = 43;
    palette->data[37][2] = 87;

    palette->data[38][0] = 35;
    palette->data[38][1] = 55;
    palette->data[38][2] = 95;

    palette->data[39][0] = 43;
    palette->data[39][1] = 67;
    palette->data[39][2] = 107;

    palette->data[40][0] = 51;
    palette->data[40][1] = 83;
    palette->data[40][2] = 110;

    palette->data[41][0] = 63;
    palette->data[41][1] = 95;
    palette->data[41][2] = 131;

    palette->data[42][0] = 71;
    palette->data[42][1] = 111;
    palette->data[42][2] = 143;

    palette->data[43][0] = 83;
    palette->data[43][1] = 127;
    palette->data[43][2] = 155;

    palette->data[44][0] = 95;
    palette->data[44][1] = 143;
    palette->data[44][2] = 167;

    palette->data[45][0] = 107;
    palette->data[45][1] = 159;
    palette->data[45][2] = 179;

    palette->data[46][0] = 143;
    palette->data[46][1] = 203;
    palette->data[46][2] = 211;

    palette->data[47][0] = 186;
    palette->data[47][1] = 240;
    palette->data[47][2] = 239;
}
