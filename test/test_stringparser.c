#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>



void test_state_variables(const char *anim_str) {
    sd_stringparser *parser = sd_stringparser_create();
    char err_msg[256];

    int err = sd_stringparser_set_string(parser, anim_str);
    if(err == 0) 
    {
        const sd_stringparser_tag_value *blend_additive, *blend_start, *blend_end, *jump;

        int jump_count = 0;
        for(unsigned int tick=0;tick<2000;++tick) {
            if(sd_stringparser_run(parser, tick) == 0) {
                sd_stringparser_frame *frame = &parser->current_frame;
                if(frame->is_first_frame) printf("#%d First frame\n", frame->id);
                else if(frame->is_final_frame) printf("#%d Final frame\n", frame->id);
                else if(frame->is_animation_end) printf("#%d %d.Animation  finished\n", frame->id, tick);
                else printf("#%d Frame changed\n", frame->id);
                
                if(sd_stringparser_get_tag(parser, frame->id, SD_BLEND_ADDITIVE, &blend_additive)) {
                    printf("Error getting blend additive tag\n");
                    return;
                }
                if(sd_stringparser_get_tag(parser, frame->id, SD_BLEND_START, &blend_start)) {
                    printf("Error getting blend start tag\n");
                    return;
                }
                if(sd_stringparser_get_tag(parser, frame->id, SD_BLEND_FINISH, &blend_end)) {
                    printf("Error getting blend end tag\n");
                    return;
                }
                if(sd_stringparser_get_tag(parser, frame->id, SD_JUMP_TICK, &jump)) {
                    printf("Error getting jump tag\n");
                    return;
                }
                
                if(blend_additive->is_set) {
                    printf("Tick %d: blend additive %d\n", tick, blend_additive->value);
                }

                if(blend_start->is_set) {
                    printf("Tick %d: blend start %d\n", tick, blend_start->value);
                }

                if(blend_end->is_set) {
                    printf("Tick %d: blend finish %d\n", tick, blend_end->value);
                }

                if(jump->is_set) {
                    printf("Tick %d: jump to tick %d\n", tick, jump->value);
                    tick = jump->value-1;
                    jump_count++;
                }

                // limit jump count to avoid infinite loop
                if(jump_count > 3) break;
            }
        }
    } else {
        sd_get_error(err_msg, err);
        printf("Animation string parser error: %s (%s)\n", err_msg, anim_str);
    }

    sd_stringparser_delete(parser);
}

void test_anim_string(sd_stringparser *parser, const char *str) {
    // execute anim and print result
    unsigned int ticks = 0;
    char err_msg[100];
    int err = sd_stringparser_set_string(parser, str);
    if(err) {
        sd_get_error(err_msg, err);
        printf("Animation string parser error: %s (%s)\n", err_msg, str);
    } else {
        sd_stringparser_run(parser, ticks);
        sd_stringparser_prettyprint(parser);
        printf("%s passed\n", str);
    }
}

void test_broken_string(sd_stringparser *parser) {
    unsigned int ticks = 0;
    char err_msg[100];
    const char *broken_string = "x+4zcubs21l50zp";
    int err = sd_stringparser_set_string(parser, broken_string);
    if(err) {
        sd_get_error(err_msg, err);
        printf("Animation string parser error: %s (%s)\n", err_msg, broken_string);
    } else {
        sd_stringparser_run(parser, ticks);
        printf("broken string test passed\n\n");
    }
}

int main(int argc, char **argv) {
    char buf[256];
    char *ext;

    if (argc <= 1) {
        printf("Usage %s <AF filename>  <BK filename>\n", argv[0]);
        printf("Usage %s --testvar\n", argv[0]);
        return 1;
    }

    if(strcmp(argv[1], "--testvar") == 0) {
        test_state_variables("brA20-bs200B200-bf200C200");
        printf("\n\n");
        test_state_variables("brA20-bs200B200-bf200C200-d1B10");
        return 0;
    }

    sd_stringparser *parser = sd_stringparser_create();
    test_broken_string(parser);

    sd_af_file *af;
    sd_bk_file *bk;
    while(--argc) {
        strncpy(buf, argv[argc], 255);
        buf[255] = '0';

        ext = strrchr(buf, '.');

        if (ext == NULL) {
            printf("cannot determine file extension for %s\n", buf);
            return 1;
        }

        printf("******* Testing file %s\n", buf);
        if (strncmp(ext, ".AF", 3) == 0) {
            af = sd_af_create();
            if(sd_af_load(af, argv[argc]) == SD_FILE_OPEN_ERROR) {
                printf("failed to open %s\n", buf);
            } else {
                for(int i=0;i<70;++i) {
                    sd_move *move = af->moves[i];
                    if(move) {
                        test_anim_string(parser, move->animation->anim_string);
                        for(int j=0;j<move->animation->extra_string_count;++j) {
                            if(move->animation->extra_strings[j]) {
                                test_anim_string(parser, move->animation->extra_strings[j]);
                            }
                        }
                        if(move->footer_string) test_anim_string(parser, move->footer_string);
                    }
                }
                sd_af_delete(af);
            }
            
        }
        else if (strncmp(ext, ".BK", 3) == 0) {
            bk = sd_bk_create();
            if(sd_bk_load(bk, argv[argc]) == SD_FILE_OPEN_ERROR) {
                printf("failed to open %s\n", buf);
            } else {
                for(int i=0;i<50;++i) {
                    sd_bk_anim *anim = bk->anims[i];
                    if(anim) {
                        test_anim_string(parser, anim->animation->anim_string);
                        for(int j=0;j<anim->animation->extra_string_count;++j) {
                            if(anim->animation->extra_strings[j]) {
                                test_anim_string(parser, anim->animation->extra_strings[j]);
                            }
                        }
                    }
                }
                sd_bk_delete(bk);
            }
            
        } else {
            printf("Invalid extension for %s\n", buf);
        }
        printf("******* File passed  %s\n\n\n", buf);
    }

    sd_stringparser_delete(parser);

    return 0;
}
