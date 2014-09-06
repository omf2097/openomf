#ifndef _SD_ACTIONS_H
#define _SD_ACTIONS_H

typedef enum {
    SD_ACT_NONE =  0x0,
    SD_ACT_PUNCH = 0x1,
    SD_ACT_KICK  = 0x2,
    SD_ACT_UP    = 0x4,
    SD_ACT_DOWN  = 0x8,
    SD_ACT_LEFT  = 0x10,
    SD_ACT_RIGHT = 0x20,
} sd_action;

#endif // _SD_ACTIONS_H