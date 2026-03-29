#include "camera.h"
#include <math.h>

void camera_init(Camera *cam, int i, int j, Vec3 dir) {
    cam->pos         = vec3(i + 0.5f, 0.0f, j + 0.5f);
    cam->dir         = dir;
    cam->up          = vec3(0.0f, 1.0f, 0.0f);
    cam->upside_down = 0;
}

int camera_looking_at(const Camera *cam, Vec3 v_dir) {
    return vec3_angle(cam->dir, v_dir) < 0.01f;
}

/* Sign of the rotation needed to turn from v1 towards v2 around the Y axis */
static float rotation_sign(const Camera *cam, Vec3 v1, Vec3 v2) {
    float sign = (vec3_cross(v1, v2).y > 0.0f) ? 1.0f : -1.0f;
    return cam->upside_down ? -sign : sign;
}

int camera_rotate_to(Camera *cam, Vec3 v_dir, float dt) {
    float sign = rotation_sign(cam, cam->dir, v_dir);
    cam->dir = mat3_rotate_axis_vec(cam->up, dt * sign * TURN_SPEED, cam->dir);

    /* If the rotation overshot, snap to target */
    float new_sign = rotation_sign(cam, cam->dir, v_dir);
    if (sign * new_sign < 0.0f)
        cam->dir = v_dir;

    return camera_looking_at(cam, v_dir);
}

int camera_move_to(Camera *cam, Vec3 p_to, float dt) {
    Vec3 old_dir = vec3_norm(vec3_sub(p_to, cam->pos));
    cam->pos = vec3_add(cam->pos, vec3_scale(MOVE_SPEED * dt, cam->dir));

    Vec3 new_dir = vec3_norm(vec3_sub(p_to, cam->pos));
    /* Overshot: the direction reversed */
    if (vec3_dist(old_dir, new_dir) >= 0.5f) {
        cam->pos = p_to;
        return 1;
    }
    return 0;
}

int camera_roll_to(Camera *cam, Vec3 v_dir, float dt) {
    cam->up = mat3_rotate_axis_vec(cam->dir, dt * TURN_SPEED, cam->up);
    if (vec3_angle(cam->up, v_dir) < 0.1f) {
        cam->up = v_dir;
        return 1;
    }
    return 0;
}
