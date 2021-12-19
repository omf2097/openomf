#ifndef MENU_VIDEO_CONFIRM_H
#define MENU_VIDEO_CONFIRM_H

#include "game/gui/component.h"
#include "game/protos/scene.h"
#include "game/utils/settings.h"

component* menu_video_confirm_create(scene *s, settings_video *old_settings);

#endif // MENU_VIDEO_CONFIRM_H
