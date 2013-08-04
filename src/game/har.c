#include "game/har.h"
#include "utils/log.h"
#include "video/texture.h"
#include "video/video.h"
#include "audio/sound.h"
#include "utils/array.h"
#include "utils/list.h"
#include "game/physics/space.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "controller/controller.h"

typedef struct hook_function_t {
    void(*fp)(char* buf, void *userdata);
    void *userdata;
} hook_function;

// Internal functions
void har_add_ani_player(void *userdata, int id, int mx, int my, int mg);
void har_set_ani_finished(void *userdata, int id);

void fire_hooks(har *har, char *buf);

void har_add_ani_player(void *userdata, int id, int mx, int my, int mg) {
    int px,py,successor;
    har *har = userdata;
    animation *ani = array_get(&har->animations, id);
    if(ani != NULL) {
        // Position for new object
        object_get_pos(&har->pobj, &px, &py);
        DEBUG("spawning %d at (%d + %d + %d, %d + %d + %d)", id, ani->sdani->start_x, mx, px, ani->sdani->start_y, my, py);
        px += ani->sdani->start_x;
        px += mx;
        py += ani->sdani->start_y;
        py += my;

        // Particle object
        particle *p = malloc(sizeof(particle));
        particle_create(p, id, ani, 0, px, py, 0, 0, mg/100.0f);
        
        // Find successor
        successor = har->af->moves[id]->unknown[16];
        if(successor) {
            p->successor = array_get(&har->animations, successor);
            DEBUG("%d has a successor: %d", id, successor);
        }
        
        // Add to particle list
        list_append(&har->particles, &p, sizeof(particle*));
        DEBUG("Create animation %d", id);
        return;
    } 
}

void har_set_ani_finished(void *userdata, int id) {
    har *har = userdata;
    iterator it;
    particle **p = 0;
    
    list_iter_begin(&har->particles, &it);
    while((p = iter_next(&it)) != NULL) {
        if((*p)->id == id) {
            (*p)->finished = 1;
            return;
        }
    }
}

void har_switch_animation(har *har, int id) {
    animationplayer_free(&har->player);
    animationplayer_create(&har->player, id, array_get(&har->animations, id), &har->pobj);
    animationplayer_set_direction(&har->player, har->direction);
    har->player.userdata = har;
    har->player.add_player = har_add_ani_player;
    har->player.del_player = har_set_ani_finished;
    har->hit_this_time = 0;
    har->player.reverse = 0;
    animationplayer_run(&har->player);
}

/*
static void ground_check(cpBody *body, cpArbiter *arb, cpVect *groundNormal){
    cpVect n = cpvneg(cpArbiterGetNormal(arb, 0));

    // TODO we should really check if we're colliding with the ground plane
    if(n.y < groundNormal->y){
        (*groundNormal) = n;
    }
}
*/
int har_on_ground(har *har) {
/*
    cpVect groundNormal = cpvzero;

    cpBodyEachArbiter(har->pobj.body, (cpBodyArbiterIteratorFunc)ground_check, &groundNormal);

    cpBool grounded = (groundNormal.y < 0.0);

    if (grounded && har->state == STATE_JUMPING) {
        DEBUG("landed");
        har->state = STATE_STANDING;
        har->player.reverse = 0;
        har->player.finished = 1;
        object_set_vel(&har->pobj, 0, 0);
        return 0;
    } else if (har->state == STATE_JUMPING) {
         float vx, vy;
         object_get_vel(&har->pobj, &vx, &vy);
         if (vy > 0.0) {
             DEBUG("falling");
         } else if (vy < 0.0) {
             DEBUG("rising");
         }
    } else if (!grounded && har->state == STATE_RECOIL) {
        har->state = STATE_AIRBORNE_RECOIL;
    } else if (grounded && har->state == STATE_AIRBORNE_RECOIL) {
         float vx, vy;
         object_get_vel(&har->pobj, &vx, &vy);
         if (vy >= 0.0) {
             // falling, not rising
             har_switch_animation(har, ANIM_STANDUP);
             har->state = STATE_STANDING;
             object_set_vel(&har->pobj, 0, 0);
         }
    }
*/
    return 1;
}

/*
void phycb_fall(physics_state *state, void *userdata) {
    har *h = userdata;
    if (h->state == STATE_JUMPING) {
        animationplayer_next_frame(&h->player);
        if(h->player.id == ANIM_JUMPING && h->phy.spd.x*h->direction < 0) {
            animationplayer_goto_frame(&h->player, sd_stringparser_num_frames(h->player.parser)-1);
            h->player.reverse = 1;
        }
        DEBUG("switching to falling");
    }
}

void phycb_floor_hit(physics_state *state, void *userdata, int flight_mode) {
    DEBUG("Hit the floor");

    har *h = userdata;
    h->player.reverse = 0;
    h->player.finished = 1;
    if (h->state == STATE_JUMPING) {
        h->state = STATE_STANDING;
        // XXX-Hunter adding this line seem to have fixed the bunnyhop bug
        physics_move(&h->phy, 0.0f);
        DEBUG("stopped jumping");
    } else if (h->state == STATE_RECOIL) {
        DEBUG("crashed into ground");
        har_switch_animation(h, ANIM_STANDUP);
        physics_move(&h->phy, 0.0f);
        //h->state = STATE_STANDING;
    }
}

void phycb_stop(physics_state *state, void *userdata) {
    har *h = userdata;
    if(h->state != STATE_STANDING && h->state != STATE_SCRAP && h->state != STATE_DESTRUCTION && h->state != STATE_RECOIL) {
        h->state = STATE_STANDING;
        har_switch_animation(h, ANIM_IDLE);
        animationplayer_set_repeat(&h->player, 1);
        DEBUG("switching to idle");
    }
}

void phycb_jump(physics_state *state, void *userdata) {
    har *h = userdata;
    h->state = STATE_JUMPING;
    har_switch_animation(h, ANIM_JUMPING);
    DEBUG("switching to jumping");
}

void phycb_move(physics_state *state, void *userdata) {
    har *h = userdata;
    if (h->state != STATE_RECOIL) {
        h->state = STATE_WALKING;
        har_switch_animation(h, ANIM_WALKING);
        animationplayer_set_repeat(&h->player, 1);
        DEBUG("switching to walking");
        // TODO: Handle reverse animation if necessary
    }
}

void phycb_crouch(physics_state *state, void *userdata) {
    har *h = userdata;
    if(h->state != STATE_CROUCHING) {
        h->state = STATE_CROUCHING;
        har_switch_animation(h, ANIM_CROUCHING);
        //animationplayer_set_repeat(&h->player, 1);
        DEBUG("crouching");
    }
}
*/

int har_load(har *h, sd_palette *pal, int id, int direction) {
    list_create(&h->hooks);

    h->cd_debug_enabled = 0;
    image_create(&h->cd_debug, 420, 350);
    h->cd_debug_tex.data = NULL;
    
    // Initial bot stuff
    h->state = STATE_STANDING;
    h->direction = direction;
    h->tick = 0; // TEMPORARY
    h->af = sd_af_create();
    
    // fill the input buffer with 'pauses'
    memset(h->inputs, '5', 10);
    h->inputs[10] = '\0';
    
    // Load AF
    int ret;
    switch (id) {
        case HAR_JAGUAR:
            ret = sd_af_load(h->af, "resources/FIGHTR0.AF");
            break;
        case HAR_SHADOW:
            ret = sd_af_load(h->af, "resources/FIGHTR1.AF");
            break;
        case HAR_THORN:
            ret = sd_af_load(h->af, "resources/FIGHTR2.AF");
            break;
        case HAR_PYROS:
            ret = sd_af_load(h->af, "resources/FIGHTR3.AF");
            break;
        case HAR_ELECTRA:
            ret = sd_af_load(h->af, "resources/FIGHTR4.AF");
            break;
        case HAR_KATANA:
            ret = sd_af_load(h->af, "resources/FIGHTR5.AF");
            break;
        case HAR_SHREDDER:
            ret = sd_af_load(h->af, "resources/FIGHTR6.AF");
            break;
        case HAR_FLAIL:
            ret = sd_af_load(h->af, "resources/FIGHTR7.AF");
            break;
        case HAR_GARGOYLE:
            ret = sd_af_load(h->af, "resources/FIGHTR8.AF");
            break;
        case HAR_CHRONOS:
            ret = sd_af_load(h->af, "resources/FIGHTR9.AF");
            break;
        case HAR_NOVA:
            ret = sd_af_load(h->af, "resources/FIGHTR10.AF");
            break;
        default:
            return 1;
            break;
    }

    if (ret) {
        PERROR("Failed to load HAR %d", id);
        return 1;
    }
    
    // Handle animations
    animation *ani;
    sd_move *move;
    array_create(&h->animations);
    for(unsigned int i = 0; i < 70; i++) {
        move = h->af->moves[i];
        if(move != NULL) {
            // Create animation + textures, etc.
            ani = malloc(sizeof(animation));
            animation_create(ani, move->animation, pal, -1, h->af->soundtable);
            array_set(&h->animations, i, ani);
            DEBUG("Loading animation %d", i);
        }
    }
    
    list_create(&h->particles);

    // Har properties
    h->health = h->health_max = 500;
    /*h->endurance = h->endurance_max = h->af->endurance;*/
    h->endurance = h->endurance_max = 200;
    DEBUG("Har %d loaded!", id);
    return 0;
}

int har_init(har *har, int x, int y) {
    object_create(&har->pobj, x, y, 0, 0);
    object_set_gravity(&har->pobj, 1.0f);
    object_set_layers(&har->pobj, LAYER_HAR);
    // Start player with animation 11
    animationplayer_create(&har->player, ANIM_IDLE, array_get(&har->animations, ANIM_IDLE), &har->pobj);
    animationplayer_set_direction(&har->player, har->direction);
    animationplayer_set_repeat(&har->player, 1);
    har->player.userdata = har;
    har->player.add_player = har_add_ani_player;
    har->player.del_player = har_set_ani_finished;
    har->hit_this_time = 0;
    animationplayer_run(&har->player);
    return 0;
}

void har_free(har *h) {
    iterator it;
    animation *ani;
    particle **p;

    // Free AF
    sd_af_delete(h->af);

    image_free(&h->cd_debug);
    
    // Free animations
    array_iter_begin(&h->animations, &it);
    while((ani = iter_next(&it)) != 0) {
        animation_free(ani);
        free(ani);
    }
    array_free(&h->animations);
    
    // Free particles
    list_iter_begin(&h->particles, &it);
    while((p = iter_next(&it)) != NULL) {
        particle_free(*p);
        free(*p);
    }
    list_free(&h->particles);
    
    // Unload player
    animationplayer_free(&h->player);
    
    // Free object
    object_free(&h->pobj);
}

void har_take_damage(har *har, int amount, const char *string) {
    har->health -= amount / 2.0f;
    har->endurance -= amount;
    if(har->health <= 0) {
        har->health = 0;
    }
    if(har->endurance <= 0) {
        har->endurance = 0;
    }
    DEBUG("HAR took %f damage, and its health is now %d -- %d", amount / 2.0f, har->health, har->endurance);
    if (har->health == 0 && har->endurance == 0) {
        har->state = STATE_DEFEAT;
        har_switch_animation(har, ANIM_DEFEAT);
    } else if (har->endurance == 0 && har->state != STATE_STUNNED) {
        har->state = STATE_STUNNED;
        har_switch_animation(har, ANIM_STUNNED);
        // TODO time out after some time...
        animationplayer_set_repeat(&har->player, 1);
    } else {
        animationplayer_free(&har->player);
        animationplayer_create(&har->player, ANIM_DAMAGE, array_get(&har->animations, ANIM_DAMAGE), &har->pobj);
        animationplayer_set_direction(&har->player, har->direction);
        if (string) {
            animationplayer_set_string(&har->player, string);
        }
        har->hit_this_time = 0;
        har->player.userdata = har;
        har->player.add_player = har_add_ani_player;
        har->player.del_player = har_set_ani_finished;
        if (har->state == STATE_STUNNED) {
            // hit while stunned, refill the endurance meter
            har->endurance = har->endurance_max;
        }
        har->state = STATE_RECOIL;
        animationplayer_run(&har->player);
    }
}

void har_spawn_scrap(har *h, int x, int y, int direction) {
    // Spawn scrap!
    animation *scrap_ani;
    for(int i = 1; i < 16; i++) {
        particle *p = malloc(sizeof(particle));
        scrap_ani = array_get(&h->animations, ANIM_SCRAP_METAL+(i%3));
        int vy = (-(4.0 / 16 * i + 2.0));
        int vx = direction * (30.0 / 16 * i + 2.0);
        particle_create(p, ANIM_SCRAP_METAL+(i%3), scrap_ani, direction, x, y, vx, vy, 10.0f);
        particle_set_lifetime(p, 40);
        object_set_group(&p->pobj, 2); // Set group for scrap
        object_set_layers(&p->pobj, LAYER_SCRAP); // Set layer for scrap
        list_append(&h->particles, &p, sizeof(particle*));
    }
}

int check_collision(har *har_a, har *har_b, sd_animation *ani, int frame_id, sd_sprite *sprite, sd_vga_image *vga) {
    int x,y;
    object_get_pos(&har_b->pobj, &x, &y);
    x += sprite->pos_x;
    y += sprite->pos_y;
    int w = sprite->img->w;
    int h = sprite->img->h;
    int hit = 0;

    if (har_b->direction == -1) {
        x = object_get_px(&har_b->pobj) + ((sprite->pos_x * har_b->direction) - sprite->img->w);
    }

    if(har_a->cd_debug_enabled) {
        image_clear(&har_a->cd_debug, color_create(0, 0, 0, 0));
        // draw the bounding box
        image_rect(&har_a->cd_debug, x+50, y+50, w, h, color_create(0, 0, 0, 255));

        // draw the 'ghost'
        for (int i = 0; i < vga->w*vga->h; i++) {
            if (vga->data[i] > 0 && vga->data[i] < 48) {
                if (har_b->direction == -1) {
                    image_set_pixel(&har_a->cd_debug, x + 50 + (vga->w - (i % vga->w)), 50 + y + (i / vga->w), color_create(255, 255, 255, 100));
                } else {
                    image_set_pixel(&har_a->cd_debug, x + 50 + (i % vga->w), 50 + y + (i / vga->w), color_create(255, 255, 255, 100));
                }
            }
        }
        image_set_pixel(&har_a->cd_debug, object_get_px(&har_a->pobj)  + 50, object_get_py(&har_a->pobj) + 50, color_create(255, 255, 0, 255));
        image_set_pixel(&har_a->cd_debug, object_get_px(&har_b->pobj) + 50 , object_get_py(&har_b->pobj) + 50, color_create(255, 255, 0, 255));
    }

    // Find collision points, if any
    for(int i = 0; i < ani->col_coord_count; i++) {
        if(ani->col_coord_table[i].y_ext == frame_id) {
            if(har_a->cd_debug_enabled) {
                image_set_pixel(
                    &har_a->cd_debug, 
                    50 + (ani->col_coord_table[i].x * har_a->direction) + object_get_px(&har_a->pobj), 
                    50 + ani->col_coord_table[i].y + object_get_py(&har_a->pobj), 
                    color_create(0, 0, 255, 255)
                );
            }
            // coarse check vs sprite dimensions
            if ((ani->col_coord_table[i].x * har_a->direction) + object_get_px(&har_a->pobj) > x && (ani->col_coord_table[i].x * har_a->direction) + object_get_px(&har_a->pobj) < x + w) {
                if (ani->col_coord_table[i].y + object_get_py(&har_a->pobj) > y && ani->col_coord_table[i].y + object_get_py(&har_a->pobj) < y + h) {
                    if(har_a->cd_debug_enabled) {
                        image_set_pixel(&har_a->cd_debug, 50 + (ani->col_coord_table[i].x * har_a->direction) + object_get_px(&har_a->pobj), 50 + ani->col_coord_table[i].y + object_get_py(&har_a->pobj), color_create(0, 255, 0, 255));
                    }
                    // Do a fine grained per-pixel check for a hit
                    //boxhit = 1;

                    int xoff = x - object_get_px(&har_a->pobj);
                    int yoff = object_get_py(&har_b->pobj) - object_get_py(&har_a->pobj);
                    int xcoord = (ani->col_coord_table[i].x * har_a->direction) - xoff;
                    int ycoord = (h + (ani->col_coord_table[i].y - yoff)) - (h+sprite->pos_y);

                    if (har_b->direction == -1) {
                        xcoord = vga->w - xcoord;
                    }

                    unsigned char hitpixel = vga->data[ycoord*vga->w+xcoord];
                    if (hitpixel > 0 && hitpixel < 48) {
                        // this is a HAR pixel

                        // TODO: Move this elsewhere, possibly to har_take_damage
                        har_spawn_scrap(
                                har_b, 
                                object_get_px(&har_b->pobj) + (sprite->img->w - xcoord), 
                                object_get_py(&har_b->pobj) - (sprite->img->h - ycoord), 
                                har_a->direction
                        );

                        DEBUG("hit point was %d, %d -- %d", xcoord, ycoord, h+sprite->pos_y);
                        hit = 1;
                        if (!har_a->cd_debug_enabled) {
                            // not debugging, we can break out of the loop and not draw any more pixels
                            break;
                        } else {
                            image_set_pixel(
                                &har_a->cd_debug, 
                                50 + (ani->col_coord_table[i].x * har_a->direction) + object_get_px(&har_a->pobj), 
                                50 + ani->col_coord_table[i].y + object_get_py(&har_a->pobj), 
                                color_create(255, 0, 0, 255)
                            );
                        }
                    }
                }
            }
        }
    }
    return hit;
}

void har_collision_har(har *har_a, har *har_b) {
    // Make stuff easier to get to :)
    int ani_id = har_a->player.id;
    int o_ani_id = ani_id; // original ani_id

    sd_animation *ani = har_a->af->moves[ani_id]->animation;

    int other_ani_id = har_b->player.id;
    if (har_b->state == STATE_RECOIL || har_a->hit_this_time) {
        // can't kick them while they're down
        // also can't hit other har twice with one move?
        return;
    }

    if (har_b->state == STATE_DEFEAT && (har_a->state == STATE_SCRAP || har_a->state == STATE_DESTRUCTION) && har_a->hit_this_time == 0) {
        DEBUG("SCRAP/DESTRUCTION!");
        har_a->hit_this_time = 1;
        char *string = har_a->af->moves[ani_id]->footer_string;
        animationplayer_free(&har_b->player);
        animationplayer_create(&har_b->player, ANIM_DAMAGE, array_get(&har_b->animations, ANIM_DAMAGE), &har_b->pobj);
        animationplayer_set_direction(&har_b->player, har_b->direction);
        if (string) {
            animationplayer_set_string(&har_b->player, string);
        }
        har_b->player.userdata = har_b;
        har_b->player.add_player = har_add_ani_player;
        har_b->player.del_player = har_set_ani_finished;
        return;
    }

    // har_b frame must be ready here or else it will crash
    if(har_b->player.parser->is_frame_ready) {
        int frame_id = animationplayer_get_frame(&har_a->player);
        char other_frame_letter = animationplayer_get_frame_letter(&har_b->player);
        sd_sprite *sprite = har_b->af->moves[other_ani_id]->animation->sprites[(int)other_frame_letter - 65];
        int hit = 0;

        sd_vga_image *vga = sd_sprite_vga_decode(sprite->img);

        hit = check_collision(har_a, har_b, ani, frame_id, sprite, vga);

        // TODO: FIX Particle to HAR collisions
        
        // check any projectiles this har has in=flight
        iterator it;
        particle **p;

        list_iter_begin(&har_a->particles, &it);
        while((p = iter_next(&it)) != NULL && hit == 0) {
            ani = (*p)->player.ani->sdani;
            ani_id = (*p)->player.id;
            frame_id = animationplayer_get_frame_letter(&(*p)->player) - 65;
            hit = check_collision(har_a, har_b, ani, frame_id, sprite, vga);
            if (hit && (*p)->successor) {
                int direction = (*p)->player.direction;
                animationplayer_free(&(*p)->player);
                animationplayer_create(&(*p)->player, 0, (*p)->successor, &(*p)->pobj);
                animationplayer_set_direction(&(*p)->player, direction);
                (*p)->player.userdata = *p;
                animationplayer_run(&(*p)->player);
                (*p)->successor = NULL;
                (*p)->finished = 1;
            }
        }

        sd_vga_image_delete(vga);

        if (!hit) {
            // particles did not hit, restore original ani id
            ani_id = o_ani_id;
        }

        // TODO during SCRAP/DESTRUCTION, apply the correct move string to the other HAR
        if (hit || har_a->af->moves[ani_id]->unknown[13] == CAT_CLOSE) {
            // close moves always hit, for now
            har_a->hit_this_time = 1;
            object_get_pos(&har_a->pobj, &har_b->player.enemy_x, &har_b->player.enemy_y);
            har_take_damage(har_b, har_a->af->moves[ani_id]->unknown[17], har_a->af->moves[ani_id]->footer_string);
            // check if there's a subsequent animation to change to, eg spike charge
            if (har_a->af->moves[ani_id]->unknown[12] != 0) {
                DEBUG("chained animation detected");
                har_switch_animation(har_a, har_a->af->moves[ani_id]->unknown[12]);
            }
            if (har_b->health == 0 && har_b->endurance == 0) {
                har_a->state = STATE_VICTORY;
                har_switch_animation(har_a, ANIM_VICTORY);
            }
        }

        if (hit && har_a->cd_debug_enabled) {
            if (har_a->cd_debug_tex.data) {
                texture_free(&har_a->cd_debug_tex);
            }

            texture_create_from_img(&har_a->cd_debug_tex, &har_a->cd_debug);
        }
    }

    har_a->close = 0;

    if (har_a->state == STATE_WALKING && har_a->direction == -1) {
        // walking towards the enemy
        // 35 is a made up number that 'looks right'
        if (object_get_px(&har_a->pobj) < object_get_px(&har_b->pobj)+35 && object_get_px(&har_a->pobj) > object_get_px(&har_b->pobj)) {
            object_set_px(&har_a->pobj, object_get_px(&har_b->pobj) + 35);
        }
        if (object_get_px(&har_a->pobj) < object_get_px(&har_b->pobj)+45 && object_get_px(&har_a->pobj) > object_get_px(&har_b->pobj)) {
            har_a->close = 1;
        }
    }
    if (har_a->state == STATE_WALKING && har_a->direction == 1) {
        // walking towards the enemy
        if (object_get_px(&har_a->pobj)+35 > object_get_px(&har_b->pobj) && object_get_px(&har_a->pobj) < object_get_px(&har_b->pobj)) {
            object_set_px(&har_a->pobj, object_get_px(&har_b->pobj) - 35);
        }
        if (object_get_px(&har_a->pobj)+45 > object_get_px(&har_b->pobj) && object_get_px(&har_a->pobj) < object_get_px(&har_b->pobj)) {
            har_a->close = 1;
        }
    }
}

void har_tick(har *har) {
    iterator it;
    particle **p;

    har->tick++;

    // Handle ticks
    if(har->tick > 3) {
        // Particles
        list_iter_begin(&har->particles, &it);
        while((p = iter_next(&it)) != NULL) {
            if((*p)->finished) {
                particle_free(*p);
                free(*p);
                list_delete(&har->particles, &it);
            } else {
                particle_tick(*p);
            }
        }
        har_on_ground(har);
    
        // I don't even know what to do about this right now ...
        /*
        har->player.x = object_get_px(&har->pobj);
        if(physics_is_in_air(&har->phy)) {
            har->player.y = har->phy.pos.y - 20;
        } else {
            har->player.y = har->phy.pos.y;
        }

        if(har->player.id == ANIM_JUMPING &&
               physics_is_in_air(&har->phy) &&
               (physics_is_moving_left(&har->phy) || physics_is_moving_right(&har->phy)) &&
               physics_is_moving_down(&har->phy)) {

            if(har->phy.spd.x*har->direction > 0) {
                animationplayer_goto_frame(&har->player, animationplayer_get_frame(&har->player)+1);
            } else {
                animationplayer_goto_frame(&har->player, animationplayer_get_frame(&har->player)-1);
            }
        }*/
        
        // Animationplayer ticks
        animationplayer_run(&har->player);
        har->tick = 0;
        //regenerate endurance if not attacking, and not dead
        if ((har->health > 0 || har->endurance > 0) && har->endurance < har->endurance_max &&
                (har->player.id == ANIM_IDLE || har->player.id == ANIM_CROUCHING ||
                 har->player.id == ANIM_WALKING || har->player.id == ANIM_JUMPING)) {
            har->endurance++;
        }
    }

    if(har->player.finished) {
        if (har->state == STATE_RECOIL) {
            har->state = STATE_STANDING;
        }
        // 11 will never be finished, if it is set to repeat
        har->tick = 0;
        int animation = ANIM_IDLE;
        switch(har->state) {
            case STATE_STANDING:
                animation = ANIM_IDLE;
                break;
            case STATE_CROUCHING:
                animation = ANIM_CROUCHING;
                break;
            case STATE_SCRAP:
                animation = ANIM_VICTORY;
                break;
            case STATE_DESTRUCTION:
                animation = ANIM_VICTORY;
                break;
            case STATE_DEFEAT:
                animation = ANIM_DEFEAT;
                break;
        }
        DEBUG("next animation is %d", animation);
        har_switch_animation(har, animation);
        animationplayer_set_repeat(&har->player, 1);
    }
}

void har_render(har *har) {
    iterator it;
    particle **p;

    // Render particles
    list_iter_begin(&har->particles, &it);
    while((p = iter_next(&it)) != NULL) {
        particle_render(*p);
    }

    // Render HAR
    animationplayer_render(&har->player);

}

void har_set_direction(har *har, int direction) {
    har->direction = direction;
    if (har->player.id != 11 && har->player.id != 4 && har->player.id != 10 && har->player.id != 1) {
        // don't change the direction yet, non-idle move in progress
        return;
    }
    animationplayer_set_direction(&har->player, har->direction);
}

void add_input(har *har, char c) {
    // only add it if it is not the current head of the array
    if (har->inputs[0] == c) {
        return;
    }

    // use memmove to move everything over one spot in the array, leaving the first slot free
    memmove((har->inputs)+1, har->inputs, 9);
    // write the new first element
    har->inputs[0] = c;
}

int move_allowed(har *har, sd_move *move) {
    int cat = move->unknown[13];
    switch(har->state) {
        case STATE_JUMPING:
            if(cat == CAT_JUMPING) {
                return 1;
            }
            break;
        /*case STATE_CROUCHING:*/
            /*if (cat == CAT_CROUCH) {*/
                /*return 1;*/
            /*}*/
            /*break;*/
        case  STATE_VICTORY:
            if (cat == CAT_SCRAP) {
                har->state = STATE_SCRAP;
                return 1;
            }
            break;
        case STATE_SCRAP:
            if (cat == CAT_DESTRUCTION) {
                har->state = STATE_DESTRUCTION;
                return 1;
            }
            break;
        default:
            if (cat == CAT_CLOSE) {
                DEBUG("trying a CLOSE move %d", har->close);
                return har->close;
            } else if (cat == CAT_JUMPING) {
                return 0;
            }
            // allow anything else, for now
            return 1;
            break;
    }
    return 0;
}

void har_act(har *har, int act_type) {
    if (!(
            har->player.id == ANIM_IDLE ||
            har->player.id == ANIM_CROUCHING ||
            har->player.id == ANIM_WALKING ||
            har->player.id == ANIM_JUMPING ||
            har->state == STATE_VICTORY ||
            har->state == STATE_SCRAP ||
            har->state == STATE_RECOIL)) {
        // if we're not in the idle loop, bail for now
        return;
    }
    char buf[50];

    // for the reason behind the numbers, look at a numpad sometime
    switch(act_type) {
        case ACT_UP:
            add_input(har, '8');
            break;
        case ACT_DOWN:
            add_input(har, '2');
            break;
        case ACT_LEFT:
            if (har->direction == -1) {
                add_input(har, '6');
            } else {
                add_input(har, '4');
            }
            break;
        case ACT_RIGHT:
            if (har->direction == -1) {
                add_input(har, '4');
            } else {
                add_input(har, '6');
            }
            break;
        case ACT_UPRIGHT:
            if (har->direction == -1) {
                add_input(har, '7');
            } else {
                add_input(har, '9');
            }
            break;
        case ACT_UPLEFT:
            if (har->direction == -1) {
                add_input(har, '9');
            } else {
                add_input(har, '7');
            }
            break;
        case ACT_DOWNRIGHT:
            if (har->direction == -1) {
                add_input(har, '1');
            } else {
                add_input(har, '3');
            }
            break;
        case ACT_DOWNLEFT:
            if (har->direction == -1) {
                add_input(har, '3');
            } else {
                add_input(har, '1');
            }
            break;
        case ACT_KICK:
            add_input(har, 'K');
            break;
        case ACT_PUNCH:
            add_input(har, 'P');
            break; 
/*
        case ACT_STOP:
            if (har->state != STATE_VICTORY && har->state != STATE_SCRAP && har->state != STATE_DESTRUCTION) {
                if (har->player.id != ANIM_IDLE && har->player.id != ANIM_JUMPING) {
                    sprintf(buf, "%d|%d|%d|%f", har->phy.pos.x, har->phy.pos.y, HOOK_MOVE, 0.0f);
                    fire_hooks(har, buf);
                }
                physics_move(&har->phy, 0);
            }
            add_input(har, '5');
            break;
        case ACT_WALKLEFT:
            if (har->state != STATE_VICTORY && har->state != STATE_SCRAP && har->state != STATE_DESTRUCTION) {
                if ((har->player.id != ANIM_WALKING && har->player.id != ANIM_JUMPING) || har->phy.spd.x != -3.0f) {
                    sprintf(buf, "%d|%d|%d|%f", har->phy.pos.x, har->phy.pos.y, HOOK_MOVE, -3.0f);
                    fire_hooks(har, buf);
                }
                physics_move(&har->phy, -3.0f);
                return;
            }
            break;
        case ACT_WALKRIGHT:
            if (har->state != STATE_VICTORY && har->state != STATE_SCRAP && har->state != STATE_DESTRUCTION) {
                if ((har->player.id != ANIM_WALKING && har->player.id != ANIM_JUMPING) || har->phy.spd.x != 3.0f) {
                    sprintf(buf, "%d|%d|%d|%f", har->phy.pos.x, har->phy.pos.y, HOOK_MOVE, 3.0f);
                    fire_hooks(har, buf);
                }
                physics_move(&har->phy, 3.0f);
                return;
            }
            break;
        case ACT_CROUCH:
            if (har->state != STATE_VICTORY && har->state != STATE_SCRAP && har->state != STATE_DESTRUCTION) {
                if (har->player.id != ANIM_CROUCHING) {
                    sprintf(buf, "%d|%d|%d", har->phy.pos.x, har->phy.pos.y, HOOK_CROUCH);
                    fire_hooks(har, buf);
                }
                physics_crouch(&har->phy);
                return;
            }
            break;
        case ACT_JUMP:
            if (har->state != STATE_VICTORY && har->state != STATE_SCRAP && har->state != STATE_DESTRUCTION) {
                if (har->phy.vertical_state != PHY_VSTATE_JUMP) {
                    sprintf(buf, "%d|%d|%d|%f", har->phy.pos.x, har->phy.pos.y, HOOK_JUMP, -15.0f);
                    fire_hooks(har, buf);
                }
                physics_jump(&har->phy, -15.0f);
                return;
            }
            break;
*/
        case ACT_WALKRIGHT:
            if (har->state != STATE_VICTORY && har->state != STATE_SCRAP && har->state != STATE_DESTRUCTION && har->state != STATE_JUMPING) {
                if (har->state != STATE_WALKING) {
                    har->state = STATE_WALKING;
                    har_switch_animation(har, ANIM_WALKING);
                    animationplayer_set_repeat(&har->player, 1);
                }
                object_set_vel(&har->pobj, 5, 0);
            }
            return;
        case ACT_WALKLEFT:
            if (har->state != STATE_VICTORY && har->state != STATE_SCRAP && har->state != STATE_DESTRUCTION && har->state != STATE_JUMPING) {
                if (har->state != STATE_WALKING) {
                    har->state = STATE_WALKING;
                    har_switch_animation(har, ANIM_WALKING);
                    animationplayer_set_repeat(&har->player, 1);
                }
                object_set_vel(&har->pobj, -5, 0);
            }
            return;
        case ACT_STOP:
            if (har->state == STATE_CROUCHING || har->state == STATE_WALKING) {
                har->state = STATE_STANDING;
                har_switch_animation(har, ANIM_IDLE);
                animationplayer_set_repeat(&har->player, 1);
                object_set_vel(&har->pobj, 0, 0);
            }
            return;
        case ACT_CROUCH:
            if(har->state != STATE_CROUCHING) {
                har->state = STATE_CROUCHING;
                har_switch_animation(har, ANIM_CROUCHING);
                object_set_vel(&har->pobj, 0, 0);
            }
            return;
        case ACT_JUMP:
            if (har->state != STATE_VICTORY && har->state != STATE_SCRAP && har->state != STATE_DESTRUCTION) {
                if (har->state != STATE_JUMPING) {
                    har->state = STATE_JUMPING;
                    har_switch_animation(har, ANIM_JUMPING);
                    object_add_vel(&har->pobj, 0, -15);
                }
            }
            return;
    }
    /*DEBUG("input buffer is now %s", har->inputs);*/

    // try to find a matching move
    sd_move *move;
    size_t len;
    for(unsigned int i = 0; i < 70; i++) {
        move = har->af->moves[i];
        if(move != NULL) {
            len = strlen(move->move_string);
            if (!strncmp(move->move_string, har->inputs, len)) {
                if (move_allowed(har, move)) {
                    DEBUG("matched move %d with string %s", i, move->move_string);
                    DEBUG("input was %s", har->inputs);

                    sprintf(buf, "%d|%d|%d|%d", object_get_px(&har->pobj), object_get_py(&har->pobj), HOOK_ATTACK, i);
                    fire_hooks(har, buf);

                    //physics_move(&har->phy, 0);
                    har_switch_animation(har, i);
                    har->inputs[0]='\0';
                    break;
                }
            }
        }
    }
}

void fire_hooks(har *har, char* buf) {
    iterator it;
    hook_function **p = 0;
    
    list_iter_begin(&har->hooks, &it);
    while((p = iter_next(&it)) != NULL) {
        ((*p)->fp)(buf, (*p)->userdata);
    }
}

void har_add_hook(har *har, void(*fp)(char* msg, void *user), void *userdata) {
    hook_function *h = malloc(sizeof(hook_function));
    h->fp = fp;
    h->userdata = userdata;
    list_append(&har->hooks, &h, sizeof(hook_function*));
}

void har_parse_command(har *har, char *buf) {
    int x, y, action, length, ret, move;
    float p;
    if ((ret =sscanf(buf, "%d|%d|%d%n", &x, &y, &action, &length)) == 3) {
        DEBUG("HAR at %d,%d did %d", x, y, action);
        switch (action) {
            case HOOK_MOVE:
                if(sscanf(buf+length, "|%f", &p) == 1) {
                    object_set_pos(&har->pobj, x, y);
                    //physics_move(&har->phy, p);
                } else {
                    DEBUG("bad packet %s", buf+length);
                }
                break;
            case HOOK_JUMP:
                if(sscanf(buf+length, "|%f", &p) == 1) {
                    object_set_pos(&har->pobj, x, y);
                    //physics_jump(&har->phy, p);
                } else {
                    DEBUG("bad packet %s", buf+length);
                }
                break;
            case HOOK_CROUCH:
                object_set_pos(&har->pobj, x, y);
                //physics_crouch(&har->phy);
                break;
            case HOOK_ATTACK:
                if(sscanf(buf+length, "|%d", &move) == 1) {
                    object_set_pos(&har->pobj, x, y);
                    //physics_move(&har->phy, 0);
                    har_switch_animation(har, move);
                } else {
                    DEBUG("bad packet %s", buf+length);
                }
                break;
        }
    } else {
        DEBUG("invalid command packet %s -- %d", buf, ret);
    }
}

