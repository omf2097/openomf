/*! \file
 * \brief HAR Actions list
 * \details HAR action types list
 * \copyright MIT license.
 * \date 2013-2014
 * \author Tuomas Virtanen
 */

#ifndef SD_ACTIONS_H
#define SD_ACTIONS_H

/*! \brief Contains all actions a HAR can do during a match
 */
typedef enum
{
    SD_ACT_NONE = 0x0,   ///< No action
    SD_ACT_PUNCH = 0x1,  ///< Punch/select
    SD_ACT_KICK = 0x2,   ///< Kick/select
    SD_ACT_UP = 0x4,     ///< Move up (jump)
    SD_ACT_DOWN = 0x8,   ///< Move down (crouch)
    SD_ACT_LEFT = 0x10,  ///< Move left (walk)
    SD_ACT_RIGHT = 0x20, ///< Move right (walk)
} sd_action;

#define SD_MOVE_MASK (SD_ACT_UP | SD_ACT_DOWN | SD_ACT_LEFT | SD_ACT_RIGHT) ///< Mask of all movement keys

#endif // SD_ACTIONS_H
