#ifndef MATH3D_H
#define MATH3D_H

#include <math.h>

#define DEG2RAD(d) ((d) * (3.14159265358979323846f / 180.0f))

/* ── Vec3 ─────────────────────────────────────────────────────────────── */
typedef struct { float x, y, z; } Vec3;

static inline Vec3 vec3(float x, float y, float z) { Vec3 v = {x,y,z}; return v; }
static inline Vec3 vec3_add(Vec3 a, Vec3 b) { return vec3(a.x+b.x, a.y+b.y, a.z+b.z); }
static inline Vec3 vec3_sub(Vec3 a, Vec3 b) { return vec3(a.x-b.x, a.y-b.y, a.z-b.z); }
static inline Vec3 vec3_scale(float s, Vec3 v) { return vec3(s*v.x, s*v.y, s*v.z); }
static inline float vec3_dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline float vec3_len(Vec3 v) { return sqrtf(vec3_dot(v,v)); }
static inline Vec3 vec3_norm(Vec3 v) {
    float l = vec3_len(v);
    return (l > 1e-8f) ? vec3_scale(1.0f/l, v) : vec3(0,0,0);
}
static inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return vec3(a.y*b.z - a.z*b.y,
                a.z*b.x - a.x*b.z,
                a.x*b.y - a.y*b.x);
}
static inline float vec3_dist(Vec3 a, Vec3 b) { return vec3_len(vec3_sub(a,b)); }
static inline float vec3_angle(Vec3 a, Vec3 b) {
    float d = vec3_dot(vec3_norm(a), vec3_norm(b));
    if (d >  1.0f) d =  1.0f;
    if (d < -1.0f) d = -1.0f;
    return acosf(d);
}

/* ── Mat4 (column-major, matches OpenGL) ─────────────────────────────── */
typedef struct { float m[16]; } Mat4;

static inline Mat4 mat4_identity(void) {
    Mat4 r = {{0}};
    r.m[0]=r.m[5]=r.m[10]=r.m[15]=1.0f;
    return r;
}

static inline Mat4 mat4_mul(Mat4 a, Mat4 b) {
    Mat4 r = {{0}};
    for (int c = 0; c < 4; c++)
        for (int row = 0; row < 4; row++)
            for (int k = 0; k < 4; k++)
                r.m[c*4+row] += a.m[k*4+row] * b.m[c*4+k];
    return r;
}

static inline Mat4 mat4_translate(Vec3 t) {
    Mat4 r = mat4_identity();
    r.m[12]=t.x; r.m[13]=t.y; r.m[14]=t.z;
    return r;
}

static inline Mat4 mat4_scale_f(float s) {
    Mat4 r = mat4_identity();
    r.m[0]=r.m[5]=r.m[10]=s;
    return r;
}

static inline Mat4 mat4_rotate_y(float rad) {
    Mat4 r = mat4_identity();
    float c=cosf(rad), s=sinf(rad);
    r.m[0]=c; r.m[8]=s; r.m[2]=-s; r.m[10]=c;
    return r;
}

static inline Mat4 mat4_rotate_x(float rad) {
    Mat4 r = mat4_identity();
    float c=cosf(rad), s=sinf(rad);
    r.m[5]=c; r.m[9]=-s; r.m[6]=s; r.m[10]=c;
    return r;
}

/* Rodrigues rotation around arbitrary axis */
static inline Mat4 mat4_rotate_axis(Vec3 axis, float rad) {
    axis = vec3_norm(axis);
    float c=cosf(rad), s=sinf(rad), t=1.0f-c;
    float x=axis.x, y=axis.y, z=axis.z;
    Mat4 r = mat4_identity();
    r.m[0]  = t*x*x+c;   r.m[4]  = t*x*y-s*z; r.m[8]  = t*x*z+s*y;
    r.m[1]  = t*x*y+s*z; r.m[5]  = t*y*y+c;   r.m[9]  = t*y*z-s*x;
    r.m[2]  = t*x*z-s*y; r.m[6]  = t*y*z+s*x; r.m[10] = t*z*z+c;
    return r;
}

/* View matrix: look_at */
static inline Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = vec3_norm(vec3_sub(center, eye));
    Vec3 r = vec3_norm(vec3_cross(f, up));
    Vec3 u = vec3_cross(r, f);
    Mat4 m = mat4_identity();
    m.m[0]=r.x; m.m[4]=r.y; m.m[8] =r.z;
    m.m[1]=u.x; m.m[5]=u.y; m.m[9] =u.z;
    m.m[2]=-f.x;m.m[6]=-f.y;m.m[10]=-f.z;
    m.m[12]=-vec3_dot(r,eye);
    m.m[13]=-vec3_dot(u,eye);
    m.m[14]= vec3_dot(f,eye);
    return m;
}

/* Perspective matrix (fov in radians) */
static inline Mat4 mat4_perspective(float fov_rad, float aspect, float near, float far) {
    float t = tanf(fov_rad * 0.5f);
    Mat4 r = {{0}};
    r.m[0]  = 1.0f/(aspect*t);
    r.m[5]  = 1.0f/t;
    r.m[10] = -(far+near)/(far-near);
    r.m[11] = -1.0f;
    r.m[14] = -(2.0f*far*near)/(far-near);
    return r;
}

/* Apply 3×3 part of mat4_rotate_axis to a vector (for camera rotation) */
static inline Vec3 mat3_rotate_axis_vec(Vec3 axis, float rad, Vec3 v) {
    Mat4 m = mat4_rotate_axis(axis, rad);
    return vec3(m.m[0]*v.x + m.m[4]*v.y + m.m[8]*v.z,
                m.m[1]*v.x + m.m[5]*v.y + m.m[9]*v.z,
                m.m[2]*v.x + m.m[6]*v.y + m.m[10]*v.z);
}

#endif /* MATH3D_H */
