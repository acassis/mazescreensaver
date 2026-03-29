#ifndef CAMERA_H
#define CAMERA_H

#include "math3d.h"

#define MOVE_SPEED 2.0f
#define TURN_SPEED 2.5f

typedef struct {
    Vec3 pos;
    Vec3 dir;
    Vec3 up;
    int  upside_down;   /* 0 = normal, 1 = flipped */
} Camera;

void camera_init(Camera *cam, int i, int j, Vec3 dir);

/* Returns 1 if v_dir is essentially where we're looking */
int  camera_looking_at(const Camera *cam, Vec3 v_dir);

/* Rotate camera towards v_dir; returns 1 when done */
int  camera_rotate_to(Camera *cam, Vec3 v_dir, float dt);

/* Move camera towards p_to; returns 1 when arrived */
int  camera_move_to(Camera *cam, Vec3 p_to, float dt);

/* Roll 'up' vector towards v_dir; returns 1 when done */
int  camera_roll_to(Camera *cam, Vec3 v_dir, float dt);

#endif /* CAMERA_H */
