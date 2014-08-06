#ifndef _COL_COORD_H
#define _COL_COORD_H

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t null; 
    uint8_t frame_id;
} sd_coord;

#ifdef __cplusplus
}
#endif

#endif // _COL_COORD_H