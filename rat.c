#include "rat.h"
#include "shader.h"

#define RAT_MOVE_SPEED 3.0f

static const float RAT_VERTS[20] = {
     0.5f,-0.5f, 0.0f,  0.0f, 1.0f,
    -0.5f,-0.5f, 0.0f,  1.0f, 1.0f,
    -0.5f, 0.5f, 0.0f,  1.0f, 0.0f,
     0.5f, 0.5f, 0.0f,  0.0f, 0.0f,
};
static const unsigned int RAT_IDX[6] = { 0,1,3, 1,2,3 };

void rat_init(Rat *rat, const Maze *maze, int i, int j) {
    walker_init(&rat->walker, maze, i, j);
    walker_next(&rat->walker);
    rat->pos = vec3(j + 0.5f, 0.0f, i + 0.5f);
}

void rat_update(Rat *rat, float dt) {
    Vec3 p_to   = walker_to_point(&rat->walker);
    Vec3 old_dir = vec3_norm(vec3_sub(p_to, rat->pos));

    rat->pos = vec3_add(rat->pos,
                        vec3_scale(RAT_MOVE_SPEED * dt, old_dir));

    Vec3 new_dir = vec3_norm(vec3_sub(p_to, rat->pos));
    if (vec3_dist(old_dir, new_dir) >= 0.5f) {
        rat->pos = p_to;
        walker_next(&rat->walker);
    }
}

void rat_renderer_init(RatRenderer *rr) {
    GLuint vbo, ebo;
    glGenVertexArrays(1, &rr->vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(rr->vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RAT_VERTS),
                 RAT_VERTS, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(RAT_IDX),
                 RAT_IDX, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void rat_renderer_setup(const RatRenderer *rr, const Shader *s,
                        const Texture textures[TEX_COUNT]) {
    glBindVertexArray(rr->vao);
    shader_set_bool(s, "rat",    1);
    shader_set_bool(s, "shaded", 0);
    shader_set_int (s, "tex",    (int)textures[TEX_RAT].number);
    shader_set_int (s, "tiling", textype_tiling(TEX_RAT));
}

void rat_renderer_draw(const RatRenderer *rr, const Shader *s,
                       const Rat *rat) {
    Mat4 model = mat4_translate(rat->pos);
    shader_set_mat4(s, "model", &model);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    (void)rr;
}
