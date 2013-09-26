#include "game/protos/player.h"
#include "game/protos/object.h"
#include "utils/string.h"
#include <shadowdive/stringparser.h>
#include <inttypes.h>
#include <stdlib.h>

// ---------------- Private functions ---------------- 

int isset(sd_stringparser_frame *frame, const char *tag) {
    const sd_stringparser_tag_value *v;
    sd_stringparser_get_tag(frame->parser, frame->id, tag, &v);
    return v->is_set;
}

int get(sd_stringparser_frame *frame, const char *tag) {
    const sd_stringparser_tag_value *v;
    sd_stringparser_get_tag(frame->parser, frame->id, tag, &v);
    return v->value;
}

int dist(int a, int b) {
    return abs((a < b ? a : b) - (a > b ? a : b)) * (a < b ? 1 : -1);
}

// ---------------- Public functions ---------------- 

void player_create(object *obj) {
    obj->animation_state.reverse = 0;
    obj->animation_state.end_frame = UINT32_MAX;
    obj->animation_state.ticks = 1;
    obj->animation_state.finished = 0;
    obj->animation_state.repeat = 0;
    obj->animation_state.enemy_x = 0;
    obj->animation_state.enemy_y = 0;
    obj->animation_state.add_player = NULL;
    obj->animation_state.parser = NULL;
}

void player_free(object *obj) {

}

void player_reload(object *obj) {
    // Unload old parser, if any
    if(obj->animation_state.parser != NULL) {
        sd_stringparser_delete(obj->animation_state.parser);
        obj->animation_state.parser = NULL;
    }

    // Load new parser
    obj->animation_state.parser = sd_stringparser_create();
    sd_stringparser_set_string(
        obj->animation_state.parser, 
        str_c(&obj->cur_animation->animation_string));

    // Peek parameters
    sd_stringparser_frame param;
    sd_stringparser_peek(obj->animation_state.parser, 0, &param);
    
    // Set initial position for sprite
    if(isset(&param, "x=")) {
        object_set_px(obj, get(&param, "y="));
    }
    if(isset(&param, "y=")) {
        object_set_py(obj, get(&param, "y="));
    }
}

void player_reset(object *obj) {
    obj->animation_state.ticks = 1;
    obj->animation_state.finished = 0;
    sd_stringparser_reset(obj->animation_state.parser);
}

void player_run(object *obj) {

}

void player_set_repeat(object *obj, int repeat) {
	obj->animation_state.repeat = repeat;
}

int player_get_repeat(object *obj) {
    return obj->animation_state.repeat;
}

void player_set_end_frame(object *obj, int end_frame) {
	obj->animation_state.end_frame = end_frame;
}

void player_next_frame(object *obj) {
    // right now, this can only skip the first frame...
    if(sd_stringparser_run(obj->animation_state.parser, 0) == 0) {
        obj->animation_state.ticks = obj->animation_state.parser->current_frame.duration + 1;
    }
}

void player_goto_frame(object *obj, int frame_id) {
    sd_stringparser_goto_frame(obj->animation_state.parser, frame_id, &obj->animation_state.ticks);
    obj->animation_state.ticks++;
}

int player_get_frame(object *obj) {
	return sd_stringparser_get_current_frame_id(obj->animation_state.parser);
}

char player_get_frame_letter(object *obj) {
	return sd_stringparser_get_current_frame_letter(obj->animation_state.parser);
}