#ifndef CALCS_H
#define CALCS_H

#include <math.h>

// ==== Vector types ====

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y, z, w;
} Vec4;

// ==== Matrix type ====

typedef struct {
    float m[4][4]; // row-major
} Mat4;

// ==== Vector2 Functions ====

Vec2 vec2_add(Vec2 a, Vec2 b);
Vec2 vec2_sub(Vec2 a, Vec2 b);
float vec2_dot(Vec2 a, Vec2 b);

// ==== Vector3 Functions ====

Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 v, float s);
Vec3 vec3_cross(Vec3 a, Vec3 b);
float vec3_dot(Vec3 a, Vec3 b);
float vec3_length(Vec3 v);
Vec3 vec3_normalize(Vec3 v);

// ==== Vector4 Functions ====

Vec4 vec4_from_vec3(Vec3 v, float w);
Vec3 vec3_from_vec4(Vec4 v);
Vec4 vec4_scale(Vec4 v, float s);

// ==== Matrix Functions ====

Mat4 mat4_identity(void);
Mat4 mat4_mul(Mat4 a, Mat4 b);
Vec4 mat4_mul_vec4(Mat4 m, Vec4 v);

Mat4 mat4_translation(Vec3 t);
Mat4 mat4_scale(Vec3 s);
Mat4 mat4_rotation_y(float angle_rad);
Mat4 mat4_perspective(float fov_y_rad, float aspect, float near_z, float far_z);
Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up);
Vec3 mat4_mul_vec3(const Mat4 mat, Vec3 v);

// ==== Geometry types ====
typedef struct {
    Vec3 pos;
} Vertex;

typedef struct {
    Vertex v0, v1, v2;
} Triangle;

// ==== Camera & Rendering ====
typedef struct {
    Vec3 position;
    float yaw;
    float pitch;
} Camera;

Vec3 get_camera_forward(Camera cam);

Vec3 get_camera_right(Camera cam); 

#endif // CALCS_H
