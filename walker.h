#ifndef WALKER_H
#define WALKER_H

#include "maze.h"
#include "math3d.h"

typedef enum { DIR_NORTH=0, DIR_EAST, DIR_SOUTH, DIR_WEST } Direction;

typedef struct {
    const Maze *maze;
    Direction   direction;
    int         i, j;
} Walker;

Direction  dir_opposite(Direction d);
Vec3       dir_to_vec(Direction d);

void  walker_init(Walker *w, const Maze *maze, int i, int j);
void  walker_next(Walker *w);
Vec3  walker_to_point(const Walker *w);  /* centre of current tile */

#endif /* WALKER_H */
