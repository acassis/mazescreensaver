#include "walker.h"

#include <stdlib.h>

/* ── Direction helpers ───────────────────────────────────────────────── */

Direction dir_opposite(Direction d) {
    switch (d) {
        case DIR_NORTH: return DIR_SOUTH;
        case DIR_EAST:  return DIR_WEST;
        case DIR_SOUTH: return DIR_NORTH;
        case DIR_WEST:  return DIR_EAST;
        default:        return DIR_NORTH;
    }
}

Vec3 dir_to_vec(Direction d) {
    switch (d) {
        case DIR_NORTH: return vec3( 0.0f, 0.0f,-1.0f);
        case DIR_EAST:  return vec3( 1.0f, 0.0f, 0.0f);
        case DIR_SOUTH: return vec3( 0.0f, 0.0f, 1.0f);
        case DIR_WEST:  return vec3(-1.0f, 0.0f, 0.0f);
        default:        return vec3( 0.0f, 0.0f, 0.0f);
    }
}

/* ── Walker ──────────────────────────────────────────────────────────── */

static int walker_open(const Walker *w, Direction d) {
    switch (d) {
        case DIR_NORTH: return !maze_north(w->maze, w->i, w->j);
        case DIR_EAST:  return !maze_east (w->maze, w->i, w->j);
        case DIR_SOUTH: return !maze_south(w->maze, w->i, w->j);
        case DIR_WEST:  return !maze_west (w->maze, w->i, w->j);
        default:        return 0;
    }
}

void walker_init(Walker *w, const Maze *maze, int i, int j) {
    w->maze = maze;
    w->i    = i;
    w->j    = j;
    /* Pick an initial open direction: prefer South, else East */
    w->direction = (!maze_south(maze, i, j)) ? DIR_SOUTH : DIR_EAST;
}

Vec3 walker_to_point(const Walker *w) {
    return vec3(w->j + 0.5f, 0.0f, w->i + 0.5f);
}

void walker_next(Walker *w) {
    /* Collect open directions */
    Direction dirs[4] = { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };
    Direction open[4];
    int n_open = 0;

    for (int k = 0; k < 4; k++)
        if (walker_open(w, dirs[k]))
            open[n_open++] = dirs[k];

    if (n_open == 0) return;

    /* Shuffle open directions (Fisher-Yates) */
    for (int k = n_open - 1; k > 0; k--) {
        int r = rand() % (k + 1);
        Direction tmp = open[k]; open[k] = open[r]; open[r] = tmp;
    }

    /* Choose: avoid going backwards unless it's the only option */
    Direction opp = dir_opposite(w->direction);
    for (int k = 0; k < n_open; k++) {
        if (open[k] != opp || n_open == 1) {
            Direction chosen = open[k];
            switch (chosen) {
                case DIR_NORTH: w->i--; break;
                case DIR_EAST:  w->j++; break;
                case DIR_SOUTH: w->i++; break;
                case DIR_WEST:  w->j--; break;
            }
            w->direction = chosen;
            return;
        }
    }
}
