#ifndef WALL_H
#define WALL_H

#include <GL/gl.h>
#include "math3d.h"
#include "shader.h"
#include "texture.h"

typedef struct {
    Vec3    pos;
    float   angle_y;   /* degrees */
    float   angle_x;   /* degrees */
    TexType textype;
} Wall;

typedef struct {
    GLuint  vao;
    TexType last_textype;
} WallRenderer;

void wall_renderer_init(WallRenderer *wr);
void wall_renderer_setup(WallRenderer *wr, const Shader *s);
void wall_renderer_draw (WallRenderer *wr, const Shader *s,
                         const Texture textures[TEX_COUNT],
                         const Wall *w);

#endif /* WALL_H */
