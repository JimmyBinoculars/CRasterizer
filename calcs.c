#include "calcs.h"

// ===== Vec2 =====

Vec2 vec2_add(Vec2 a, Vec2 b) {
    return (Vec2){ a.x + b.x, a.y + b.y };
}

Vec2 vec2_sub(Vec2 a, Vec2 b) {
    return (Vec2){ a.x - b.x, a.y - b.y };
}

float vec2_dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

// ===== Vec3 =====

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 vec3_scale(Vec3 v, float s) {
    return (Vec3){ v.x * s, v.y * s, v.z * s };
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float vec3_length(Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    return len > 0 ? vec3_scale(v, 1.0f / len) : v;
}

// ===== Vec4 =====

Vec4 vec4_from_vec3(Vec3 v, float w) {
    return (Vec4){ v.x, v.y, v.z, w };
}

Vec3 vec3_from_vec4(Vec4 v) {
    return (Vec3){ v.x, v.y, v.z };
}

Vec4 vec4_scale(Vec4 v, float s) {
    return (Vec4){ v.x * s, v.y * s, v.z * s, v.w * s };
}

// ===== Mat4 =====

Mat4 mat4_identity(void) {
    Mat4 m = {0};
    m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
    return m;
}

Mat4 mat4_mul(Mat4 a, Mat4 b) {
    Mat4 result = {0};
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            for (int k = 0; k < 4; k++) {
                result.m[row][col] += a.m[row][k] * b.m[k][col];
            }
        }
    }
    return result;
}

Vec4 mat4_mul_vec4(Mat4 m, Vec4 v) {
    return (Vec4){
        m.m[0][0]*v.x + m.m[0][1]*v.y + m.m[0][2]*v.z + m.m[0][3]*v.w,
        m.m[1][0]*v.x + m.m[1][1]*v.y + m.m[1][2]*v.z + m.m[1][3]*v.w,
        m.m[2][0]*v.x + m.m[2][1]*v.y + m.m[2][2]*v.z + m.m[2][3]*v.w,
        m.m[3][0]*v.x + m.m[3][1]*v.y + m.m[3][2]*v.z + m.m[3][3]*v.w
    };
}

Mat4 mat4_translation(Vec3 t) {
    Mat4 m = mat4_identity();
    m.m[0][3] = t.x;
    m.m[1][3] = t.y;
    m.m[2][3] = t.z;
    return m;
}

Mat4 mat4_scale(Vec3 s) {
    Mat4 m = {0};
    m.m[0][0] = s.x;
    m.m[1][1] = s.y;
    m.m[2][2] = s.z;
    m.m[3][3] = 1.0f;
    return m;
}

Mat4 mat4_rotation_y(float angle_rad) {
    Mat4 m = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    m.m[0][0] =  c;
    m.m[0][2] =  s;
    m.m[2][0] = -s;
    m.m[2][2] =  c;
    return m;
}

Mat4 mat4_perspective(float fov_y_rad, float aspect, float near_z, float far_z) {
    float f = 1.0f / tanf(fov_y_rad / 2.0f);
    Mat4 m = {0};
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (far_z + near_z) / (near_z - far_z);
    m.m[2][3] = (2 * far_z * near_z) / (near_z - far_z);
    m.m[3][2] = -1.0f;
    return m;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = vec3_normalize(vec3_sub(center, eye));
    Vec3 s = vec3_normalize(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);

    Mat4 m = mat4_identity();
    m.m[0][0] = s.x; m.m[0][1] = s.y; m.m[0][2] = s.z;
    m.m[1][0] = u.x; m.m[1][1] = u.y; m.m[1][2] = u.z;
    m.m[2][0] = -f.x; m.m[2][1] = -f.y; m.m[2][2] = -f.z;
    m.m[0][3] = -vec3_dot(s, eye);
    m.m[1][3] = -vec3_dot(u, eye);
    m.m[2][3] =  vec3_dot(f, eye);
    return m;
}

Vec3 mat4_mul_vec3(const Mat4 mat, Vec3 v) {
    float x = v.x, y = v.y, z = v.z;

    float tx = mat.m[0][0] * x + mat.m[0][1] * y + mat.m[0][2] * z + mat.m[0][3];
    float ty = mat.m[1][0] * x + mat.m[1][1] * y + mat.m[1][2] * z + mat.m[1][3];
    float tz = mat.m[2][0] * x + mat.m[2][1] * y + mat.m[2][2] * z + mat.m[2][3];
    float tw = mat.m[3][0] * x + mat.m[3][1] * y + mat.m[3][2] * z + mat.m[3][3];

    // Perspective divide if w is not 0 or 1
    if (tw != 0.0f && tw != 1.0f) {
        tx /= tw;
        ty /= tw;
        tz /= tw;
    }

    Vec3 result = { tx, ty, tz };
    return result;
}

Vec3 get_camera_forward(Camera cam) {
    return (Vec3){
        cosf(cam.pitch) * sinf(cam.yaw),
        sinf(cam.pitch),
        cosf(cam.pitch) * cosf(cam.yaw)
    };
}

Vec3 get_camera_right(Camera cam) {
    Vec3 forward = get_camera_forward(cam);
    Vec3 up = {0, 1, 0};
    return vec3_normalize(vec3_cross(up, forward));
}
