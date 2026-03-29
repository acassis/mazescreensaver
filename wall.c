#include "wall.h"
#include "shader.h"
#include "texture.h"

#include <stddef.h>

/* Vertices: pos(3) + texcoord(2) */
static const float WALL_VERTS[20] = {
     0.5f,-0.5f, 0.0f,  0.0f, 1.0f,   /* bottom right */
    -0.5f,-0.5f, 0.0f,  1.0f, 1.0f,   /* bottom left  */
    -0.5f, 0.5f, 0.0f,  1.0f, 0.0f,   /* top left     */
     0.5f, 0.5f, 0.0f,  0.0f, 0.0f,   /* top right    */
};
static const unsigned int WALL_IDX[6] = { 0,1,3, 1,2,3 };

void wall_renderer_init(WallRenderer *wr) {
    GLuint vbo, ebo;
    glGenVertexArrays(1, &wr->vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(wr->vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WALL_VERTS),
                 WALL_VERTS, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(WALL_IDX),
                 WALL_IDX, GL_STATIC_DRAW);

    /* aPos = 0 */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    /* aTex = 1 */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    wr->last_textype = TEX_COUNT; /* "none" sentinel */
}

void wall_renderer_setup(WallRenderer *wr, const Shader *s) {
    glBindVertexArray(wr->vao);
    shader_set_bool(s, "rat",    0);
    shader_set_bool(s, "shaded", 0);
}

void wall_renderer_draw(WallRenderer *wr, const Shader *s,
                        const Texture textures[TEX_COUNT],
                        const Wall *w) {
    Mat4 model = mat4_mul(
        mat4_mul(mat4_translate(w->pos),
                 mat4_rotate_y(DEG2RAD(w->angle_y))),
        mat4_rotate_x(DEG2RAD(w->angle_x)));

    shader_set_mat4(s, "model", &model);

    /* Only update texture uniforms when the type changes */
    if (wr->last_textype != w->textype) {
        shader_set_int(s, "tex",    (int)textures[w->textype].number);
        shader_set_int(s, "tiling", textype_tiling(w->textype));
        wr->last_textype = w->textype;
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
