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
    SD_ACT_NONE = 0x00, ///< No action

    SD_ACT_PUNCH = 0x01, ///< Punch/select
    SD_ACT_KICK = 0x02,  ///< Kick/select

    SD_ACT_UPUP = 0x10,
    SD_ACT_UPRIGHT = 0x20,
    SD_ACT_RIGHTRIGHT = 0x30,
    SD_ACT_DOWNRIGHT = 0x40,
    SD_ACT_DOWNDOWN = 0x50,
    SD_ACT_DOWNLEFT = 0x60,
    SD_ACT_LEFTLEFT = 0x70,
    SD_ACT_UPLEFT = 0x80,
} sd_action;

#endif // SD_ACTIONS_H
