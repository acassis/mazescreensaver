#include "maze.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── carving ─────────────────────────────────────────────────────────── */

static void shuffle4(unsigned char *dirs) {
    /* Fisher-Yates on 4 elements */
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        unsigned char tmp = dirs[i]; dirs[i] = dirs[j]; dirs[j] = tmp;
    }
}

static void carve_from(Maze *m, int cx, int cy) {
    unsigned char dirs[4] = { MAZE_N, MAZE_E, MAZE_S, MAZE_W };
    shuffle4(dirs);

    for (int d = 0; d < 4; d++) {
        int nx = cx, ny = cy;
        unsigned char opp;

        switch (dirs[d]) {
            case MAZE_N: ny = cy - 1; opp = MAZE_S; break;
            case MAZE_E: nx = cx + 1; opp = MAZE_W; break;
            case MAZE_S: ny = cy + 1; opp = MAZE_N; break;
            case MAZE_W: nx = cx - 1; opp = MAZE_E; break;
            default: continue;
        }

        if (nx < 0 || nx >= m->width || ny < 0 || ny >= m->height) continue;
        if (m->grid[ny * m->width + nx] != 0) continue;

        m->grid[cy * m->width + cx] |= dirs[d];
        m->grid[ny * m->width + nx] |= opp;
        carve_from(m, nx, ny);
    }
}

/* ── public ──────────────────────────────────────────────────────────── */

void maze_init(Maze *m, int width, int height) {
    m->width  = width;
    m->height = height;
    m->grid   = (unsigned char *)calloc(width * height, 1);
    if (!m->grid) { perror("calloc"); exit(1); }
    srand((unsigned)time(NULL));
    carve_from(m, 0, 0);
}

void maze_free(Maze *m) {
    free(m->grid);
    m->grid = NULL;
}

void maze_print(const Maze *m) {
    putchar(' ');
    for (int c = 0; c < m->width * 2 - 1; c++) putchar('_');
    putchar('\n');

    for (int i = 0; i < m->height; i++) {
        putchar('|');
        for (int j = 0; j < m->width; j++) {
            unsigned char cell = m->grid[i * m->width + j];
            putchar((cell & MAZE_S) ? ' ' : '_');
            if (cell & MAZE_E) {
                unsigned char next = (j + 1 < m->width)
                                     ? m->grid[i * m->width + j + 1] : 0;
                putchar(((cell | next) & MAZE_S) ? ' ' : '_');
            } else {
                putchar('|');
            }
        }
        putchar('\n');
    }
}

int maze_north(const Maze *m, int i, int j) {
    return !(m->grid[i * m->width + j] & MAZE_N);
}
int maze_east(const Maze *m, int i, int j) {
    return !(m->grid[i * m->width + j] & MAZE_E);
}
int maze_south(const Maze *m, int i, int j) {
    return !(m->grid[i * m->width + j] & MAZE_S);
}
int maze_west(const Maze *m, int i, int j) {
    return !(m->grid[i * m->width + j] & MAZE_W);
}
