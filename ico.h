#ifndef ICO_H
#define ICO_H

#include <GL/gl.h>
#include "math3d.h"
#include "shader.h"

typedef struct {
    Vec3  pos;
    Vec3  axis;
    int   active;
} Ico;

typedef struct {
    GLuint vao;
} IcoRenderer;

void ico_renderer_init (IcoRenderer *ir);
void ico_renderer_setup(const IcoRenderer *ir, const Shader *s);
void ico_renderer_draw (const IcoRenderer *ir, const Shader *s,
                        const Ico *ico, float t);

#endif /* ICO_H */
