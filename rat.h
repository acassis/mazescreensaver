#ifndef RAT_H
#define RAT_H

#include <GL/gl.h>
#include "math3d.h"
#include "shader.h"
#include "texture.h"
#include "walker.h"

typedef struct {
    Vec3   pos;
    Walker walker;
} Rat;

typedef struct {
    GLuint vao;
} RatRenderer;

void rat_init    (Rat *rat, const Maze *maze, int i, int j);
void rat_update  (Rat *rat, float dt);

void rat_renderer_init (RatRenderer *rr);
void rat_renderer_setup(const RatRenderer *rr, const Shader *s,
                        const Texture textures[TEX_COUNT]);
void rat_renderer_draw (const RatRenderer *rr, const Shader *s,
                        const Rat *rat);

#endif /* RAT_H */
