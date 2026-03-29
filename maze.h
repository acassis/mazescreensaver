#ifndef MAZE_H
#define MAZE_H

#include <stddef.h>

#define MAZE_N 0x01
#define MAZE_E 0x02
#define MAZE_S 0x04
#define MAZE_W 0x08

typedef struct {
    unsigned char *grid;  /* row-major: grid[i * width + j] */
    int width;
    int height;
} Maze;

/* Allocate and carve a random perfect maze */
void maze_init(Maze *m, int width, int height);
void maze_free(Maze *m);
void maze_print(const Maze *m);

/* Wall queries: returns 1 if there IS a wall in that direction */
int maze_north(const Maze *m, int i, int j);
int maze_east (const Maze *m, int i, int j);
int maze_south(const Maze *m, int i, int j);
int maze_west (const Maze *m, int i, int j);

#endif /* MAZE_H */
