#ifndef SHADER_H
#define SHADER_H

#include <GL/gl.h>
#include "math3d.h"

typedef struct {
    GLuint id;
} Shader;

Shader  shader_new(const char *vert_path, const char *frag_path);
void    shader_use(const Shader *s);
void    shader_set_int (const Shader *s, const char *name, int value);
void    shader_set_bool(const Shader *s, const char *name, int value); /* 0/1 */
void    shader_set_vec3(const Shader *s, const char *name, Vec3 v);
void    shader_set_mat4(const Shader *s, const char *name, const Mat4 *m);

#endif /* SHADER_H */
