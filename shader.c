#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

/* ── helpers ─────────────────────────────────────────────────────────── */

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open shader: %s\n", path); exit(1); }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *buf = (char *)malloc(sz + 1);
    if (!buf) { fclose(f); exit(1); }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

static void check_errors(GLuint obj, const char *type) {
    GLint ok;
    char log[1024];
    GLsizei len;
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(obj, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            glGetShaderInfoLog(obj, sizeof(log), &len, log);
            fprintf(stderr, "SHADER COMPILE ERROR (%s):\n%.*s\n", type, len, log);
        }
    } else {
        glGetProgramiv(obj, GL_LINK_STATUS, &ok);
        if (!ok) {
            glGetProgramInfoLog(obj, sizeof(log), &len, log);
            fprintf(stderr, "PROGRAM LINK ERROR:\n%.*s\n", len, log);
        }
    }
}

/* ── public API ──────────────────────────────────────────────────────── */

Shader shader_new(const char *vert_path, const char *frag_path) {
    char *vsrc = read_file(vert_path);
    char *fsrc = read_file(frag_path);
    const char *cvs = vsrc, *cfs = fsrc;

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &cvs, NULL);
    glCompileShader(vert);
    check_errors(vert, "VERTEX");

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &cfs, NULL);
    glCompileShader(frag);
    check_errors(frag, "FRAGMENT");

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    check_errors(prog, "PROGRAM");

    glDeleteShader(vert);
    glDeleteShader(frag);
    free(vsrc);
    free(fsrc);

    Shader s; s.id = prog;
    return s;
}

void shader_use(const Shader *s) { glUseProgram(s->id); }

void shader_set_int(const Shader *s, const char *name, int v) {
    glUniform1i(glGetUniformLocation(s->id, name), v);
}
void shader_set_bool(const Shader *s, const char *name, int v) {
    glUniform1i(glGetUniformLocation(s->id, name), v);
}
void shader_set_vec3(const Shader *s, const char *name, Vec3 v) {
    glUniform3f(glGetUniformLocation(s->id, name), v.x, v.y, v.z);
}
void shader_set_mat4(const Shader *s, const char *name, const Mat4 *m) {
    glUniformMatrix4fv(glGetUniformLocation(s->id, name), 1, GL_FALSE, m->m);
}
