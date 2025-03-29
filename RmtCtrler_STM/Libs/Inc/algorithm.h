#ifndef ALGORITHM_H
#define ALGORITHM_H
#include "rmt_stick.h"
#include "std_msg.h"

#define STEER_WHEEL_RADIO 0.0871
#define STEER_WHEEL_C 0.54636
#define STEER_WHEEL_60RPM_ERPM 1520
#define STEER_WHEEL_POLE_PAIRS 21
#define MATH_PI 3.1415926

#define MeterPerSec_2_ERPM_ (1 / STEER_WHEEL_C) * 60 * STEER_WHEEL_POLE_PAIRS
#define MeterPerSec_2_ERPM 2306.1718


void algo_get_steerBetter_vec(int velo_now, float angleDeg_now, int* velo_targ, float* angleDeg_targ);
void algo_calc_steer_vecs_4(float V_x, float V_y, float V_w, Vec2 Str_Ms[]);

void algo_vec2_add(Vec2* vec_1, Vec2* vec_2);
void algo_vec2_add_xy(Vec2* vec_1, float x, float y);
Vec2 algo_vec2_add_tonew(Vec2 vec_1, Vec2 vec_2);
Vec3 algo_vec3_add_tonew(Vec3 vec_1, Vec3 vec_2);
void algo_vec2_multiply(Vec2* vec, float k);
void algo_js_to_chassis_vec(Vec3 *chassis_vec, RmtJoystickInfo js_info);
void algo_polesys_to_vec2(Vec2* vec);
Vec3 algo_OdomVec_to_LocalVec(Vec3 Odom_Vec);
void algo_chassis_vec_constrain(Vec3 *chassis_vec, float limit);


extern Vec2 unit_x_vec2;
extern Vec2 unit_y_vec2;
extern Vec2 unit_45_vec2;
extern Vec2 unit_135_vec2;
extern Vec2 unit_225_vec2;
extern Vec2 unit_315_vec2;
extern float algo_steerangles_preserve[4];

#endif