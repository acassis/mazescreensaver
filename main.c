#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "math3d.h"
#include "shader.h"
#include "texture.h"
#include "maze.h"
#include "walker.h"
#include "camera.h"
#include "wall.h"
#include "ico.h"
#include "rat.h"

/* ── tunables ────────────────────────────────────────────────────────── */
#define MAZE_W   20
#define MAZE_H   20
#define MAX_WALLS (MAZE_W * MAZE_H * 4 + MAZE_W + MAZE_H + 4)
#define MAX_ICOS  (MAZE_W * MAZE_H)
#define MAX_RATS  (MAZE_W * MAZE_H)

/* ── global state ────────────────────────────────────────────────────── */
typedef enum { STATE_WALKING, STATE_TURNING, STATE_ROLLING } State;

static Maze        g_maze;
static Shader      g_shader;
static Texture     g_textures[TEX_COUNT];

static WallRenderer g_wall_renderer;
static Wall         g_walls[MAX_WALLS];
static int          g_n_walls;

static IcoRenderer  g_ico_renderer;
static Ico          g_icos[MAX_ICOS];
/* parallel array: tile index (i*MAZE_W+j) or -1 if slot free */
static int          g_ico_tile[MAX_ICOS];
static int          g_n_icos;

static RatRenderer  g_rat_renderer;
static Rat          g_rats[MAX_RATS];
static int          g_n_rats;

static Walker  g_walker;
static Camera  g_camera;
static State   g_state;

static double  g_last_frame;
static double  g_last_second;
static int     g_frame_count;

static int g_width, g_height;
static int g_fullscreen;

/* ── utility: find ico by tile ───────────────────────────────────────── */
static int ico_find(int i, int j) {
    int tile = i * MAZE_W + j;
    for (int k = 0; k < g_n_icos; k++)
        if (g_ico_tile[k] == tile) return k;
    return -1;
}
static void ico_remove(int idx) {
    g_n_icos--;
    g_icos[idx]     = g_icos[g_n_icos];
    g_ico_tile[idx] = g_ico_tile[g_n_icos];
}

/* ── scene setup ─────────────────────────────────────────────────────── */
static TexType random_wall_tex(void) {
    return ((rand() % 100) < 90) ? TEX_BRICK : TEX_THING;
}

static void gen_walls(void) {
    g_n_walls = 0;
    Wall *w = g_walls;

    /* north border */
    for (int j = 0; j < g_maze.width; j++, w++) {
        w->pos     = vec3(j + 0.5f, 0.0f, 0.0f);
        w->angle_y = 0.0f; w->angle_x = 0.0f;
        w->textype = random_wall_tex();
    }
    /* west border */
    for (int i = 0; i < g_maze.height; i++, w++) {
        w->pos     = vec3(0.0f, 0.0f, i + 0.5f);
        w->angle_y = 90.0f; w->angle_x = 0.0f;
        w->textype = random_wall_tex();
    }
    /* inner + floor/ceiling */
    for (int i = 0; i < g_maze.height; i++) {
        for (int j = 0; j < g_maze.width; j++) {
            if (maze_south(&g_maze, i, j)) {
                w->pos     = vec3(j + 0.5f, 0.0f, i + 1.0f);
                w->angle_y = 0.0f; w->angle_x = 0.0f;
                w->textype = random_wall_tex(); w++;
            }
            if (maze_east(&g_maze, i, j)) {
                w->pos     = vec3(j + 1.0f, 0.0f, i + 0.5f);
                w->angle_y = 90.0f; w->angle_x = 0.0f;
                w->textype = random_wall_tex(); w++;
            }
            /* ceiling */
            w->pos     = vec3(j + 0.5f,  0.5f, i + 0.5f);
            w->angle_y = 0.0f; w->angle_x = 90.0f;
            w->textype = TEX_CEILING; w++;
            /* floor */
            w->pos     = vec3(j + 0.5f, -0.5f, i + 0.5f);
            w->angle_y = 0.0f; w->angle_x = 90.0f;
            w->textype = TEX_FLOOR; w++;
        }
    }
    g_n_walls = (int)(w - g_walls);

    /* sort by textype to minimise uniform changes */
    /* simple insertion sort (small arrays are fine on embedded) */
    for (int a = 1; a < g_n_walls; a++) {
        Wall tmp = g_walls[a];
        int b = a - 1;
        while (b >= 0 && g_walls[b].textype > tmp.textype) {
            g_walls[b+1] = g_walls[b]; b--;
        }
        g_walls[b+1] = tmp;
    }
}

static float rnd_f(void) {
    return (float)rand() / RAND_MAX * 2.0f - 1.0f;
}

static void gen_icos(void) {
    int total = g_maze.width * g_maze.height;
    int count = total * 6 / 100;
    if (count < 2) count = 2;
    g_n_icos = 0;

    /* Reservoir sampling – pick `count` distinct tiles */
    int *selected = (int *)malloc(count * sizeof(int));
    if (!selected) { perror("malloc"); exit(1); }
    for (int k = 0; k < count; k++) selected[k] = k;
    for (int k = count; k < total; k++) {
        int r = rand() % (k + 1);
        if (r < count) selected[r] = k;
    }
    for (int k = 0; k < count; k++) {
        int ti = selected[k] / g_maze.width;
        int tj = selected[k] % g_maze.width;
        g_ico_tile[k] = selected[k];
        g_icos[k].pos    = vec3(tj + 0.5f, 0.0f, ti + 0.5f);
        g_icos[k].axis   = vec3_norm(vec3(rnd_f(), rnd_f(), rnd_f()));
        g_icos[k].active = 1;
    }
    g_n_icos = count;
    free(selected);
}

static void gen_rats(void) {
    int total = g_maze.width * g_maze.height;
    int count = total * 5 / 100;
    if (count < 2) count = 2;
    g_n_rats = 0;

    int *selected = (int *)malloc(count * sizeof(int));
    if (!selected) { perror("malloc"); exit(1); }
    for (int k = 0; k < count; k++) selected[k] = k;
    for (int k = count; k < total; k++) {
        int r = rand() % (k + 1);
        if (r < count) selected[r] = k;
    }
    for (int k = 0; k < count; k++) {
        int ri = selected[k] / g_maze.width;
        int rj = selected[k] % g_maze.width;
        rat_init(&g_rats[k], &g_maze, ri, rj);
    }
    g_n_rats = count;
    free(selected);
}

/* ── GLUT callbacks ──────────────────────────────────────────────────── */

static void on_keyboard(unsigned char key, int x, int y) {
    (void)x; (void)y;
    if (key == 27) { /* ESC */
        maze_free(&g_maze);
        exit(0);
    }
}

static void on_reshape(int w, int h) {
    g_width = w; g_height = h;
    glViewport(0, 0, w, h);
    float ratio = (float)w / (float)h;
    Mat4 proj = mat4_perspective(DEG2RAD(60.0f), ratio, 0.1f, 100.0f);
    shader_use(&g_shader);
    shader_set_mat4(&g_shader, "proj", &proj);
}

static void on_display(void) {
    double now  = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    float  dt   = (float)(now - g_last_frame);
    g_last_frame = now;

    /* ── state machine ── */
    int completed = 0;
    switch (g_state) {
        case STATE_WALKING:
            completed = camera_move_to(&g_camera,
                            walker_to_point(&g_walker), dt);
            break;
        case STATE_TURNING:
            completed = camera_rotate_to(&g_camera,
                            dir_to_vec(g_walker.direction), dt);
            break;
        case STATE_ROLLING: {
            float y = g_camera.upside_down ? 1.0f : -1.0f;
            completed = camera_roll_to(&g_camera, vec3(0.0f, y, 0.0f), dt);
            break;
        }
    }

    if (completed) {
        switch (g_state) {
            case STATE_WALKING: {
                walker_next(&g_walker);
                Vec3 vdir = dir_to_vec(g_walker.direction);
                if (camera_looking_at(&g_camera, vdir)) {
                    int ki = ico_find(g_walker.i, g_walker.j);
                    g_state = (ki >= 0) ? STATE_ROLLING : STATE_WALKING;
                } else {
                    g_state = STATE_TURNING;
                }
                break;
            }
            case STATE_TURNING: {
                int ki = ico_find(g_walker.i, g_walker.j);
                g_state = (ki >= 0) ? STATE_ROLLING : STATE_WALKING;
                break;
            }
            case STATE_ROLLING: {
                g_camera.upside_down = !g_camera.upside_down;
                int ki = ico_find(g_walker.i, g_walker.j);
                if (ki >= 0) ico_remove(ki);
                g_state = STATE_WALKING;
                break;
            }
        }
    }

    /* update rats */
    for (int k = 0; k < g_n_rats; k++)
        rat_update(&g_rats[k], dt);

    /* view matrix */
    Vec3 eye    = g_camera.pos;
    Vec3 center = vec3_add(g_camera.pos, g_camera.dir);
    Mat4 view   = mat4_look_at(eye, center, g_camera.up);
    shader_set_mat4(&g_shader, "view", &view);

    /* FPS */
#ifndef NDEBUG
    if (now - g_last_second > 1.0) {
        g_last_second = now;
        printf("FPS: %d\n", g_frame_count);
        g_frame_count = 0;
    } else {
        g_frame_count++;
    }
#endif

    /* ── render ── */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    wall_renderer_setup(&g_wall_renderer, &g_shader);
    for (int k = 0; k < g_n_walls; k++)
        wall_renderer_draw(&g_wall_renderer, &g_shader,
                           g_textures, &g_walls[k]);

    rat_renderer_setup(&g_rat_renderer, &g_shader, g_textures);
    for (int k = 0; k < g_n_rats; k++)
        rat_renderer_draw(&g_rat_renderer, &g_shader, &g_rats[k]);

    ico_renderer_setup(&g_ico_renderer, &g_shader);
    for (int k = 0; k < g_n_icos; k++)
        ico_renderer_draw(&g_ico_renderer, &g_shader,
                          &g_icos[k], (float)now);

    glutSwapBuffers();
    glutPostRedisplay();
}

/* ── main ────────────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    srand((unsigned)time(NULL));

    g_fullscreen = 0;
    for (int a = 1; a < argc; a++)
        if (strcmp(argv[a], "--fullscreen") == 0) g_fullscreen = 1;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    if (g_fullscreen) {
        glutGameModeString("800x600:32@60");
        glutEnterGameMode();
    } else {
        g_width = 800; g_height = 600;
        glutInitWindowSize(g_width, g_height);
        glutCreateWindow("Win95 Maze");
    }

    /* Initialise maze and scene data */
    maze_init(&g_maze, MAZE_W, MAZE_H);
    maze_print(&g_maze);

    gen_walls();
    gen_icos();
    gen_rats();

    /* Shader + textures */
    g_shader = shader_new("shaders/vertex.glsl", "shaders/fragment.glsl");
    shader_use(&g_shader);
    shader_set_vec3(&g_shader, "color", vec3(0.8f, 0.1f, 0.5f));

    /* Initial projection (may be overridden by reshape) */
    {
        float ratio = (float)g_width / (float)g_height;
        Mat4 proj = mat4_perspective(DEG2RAD(60.0f), ratio, 0.1f, 100.0f);
        shader_set_mat4(&g_shader, "proj", &proj);
    }

    texture_load(&g_textures[TEX_BRICK],   "resources/brick.bmp",   0);
    texture_load(&g_textures[TEX_THING],   "resources/thing.bmp",   1);
    texture_load(&g_textures[TEX_CEILING], "resources/ceiling.bmp", 2);
    texture_load(&g_textures[TEX_FLOOR],   "resources/floor.bmp",   3);
    texture_load(&g_textures[TEX_RAT],     "resources/rat.bmp",     4);
    for (int k = 0; k < TEX_COUNT; k++)
        texture_bind(&g_textures[k]);

    /* GL state */
    glEnable(GL_DEPTH_TEST);

    /* Renderers */
    wall_renderer_init(&g_wall_renderer);
    ico_renderer_init (&g_ico_renderer);
    rat_renderer_init (&g_rat_renderer);

    /* Camera + walker */
    walker_init(&g_walker, &g_maze, 0, 0);
    camera_init(&g_camera, 0, 0, dir_to_vec(g_walker.direction));
    walker_next(&g_walker);

    g_state       = STATE_WALKING;
    g_last_frame  = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    g_last_second = g_last_frame;
    g_frame_count = 0;

    glutDisplayFunc (on_display);
    glutReshapeFunc (on_reshape);
    glutKeyboardFunc(on_keyboard);

    glutMainLoop();

    maze_free(&g_maze);
    return 0;
}
