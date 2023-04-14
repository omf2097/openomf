#ifndef SCORES_H
#define SCORES_H

typedef struct score_entry_t {
    unsigned int score;
    unsigned int pilot_id;
    unsigned int har_id;
    char name[16];
} score_entry;

typedef struct scoreboard_t {
    score_entry entries[4][20];
} scoreboard;

int scores_read(scoreboard *sb);
int scores_write(scoreboard *sb);
void scores_clear(scoreboard *sb);

#endif // SCORES_H
