#ifndef ARENA_CONSTRAINTS_H
#define ARENA_CONSTRAINTS_H

#define ARENA_LEFT_WALL 20
#define ARENA_RIGHT_WALL 300
#define ARENA_FLOOR 190

// fixed point versions
#define ARENA_LEFT_WALLF fixedpt_fromint(ARENA_LEFT_WALL)
#define ARENA_RIGHT_WALLF fixedpt_fromint(ARENA_RIGHT_WALL)
#define ARENA_FLOORF fixedpt_fromint(ARENA_FLOOR)

#endif // ARENA_CONSTRAINTS_H
