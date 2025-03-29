#include "algorithm.h"

Vec2 unit_x_vec2 =
{
    .x = 1,
};

Vec2 unit_y_vec2 =
{
    .y = 1,
};

Vec2 unit_45_vec2 =
{
    .x = 0.707106,
    .y = 0.707106,
};

Vec2 unit_135_vec2 =
{
    .x = -0.707106,
    .y = 0.707106,
};

Vec2 unit_225_vec2 =
{
    .x = -0.707106,
    .y = -0.707106,
};

Vec2 unit_315_vec2 =
{
    .x = 0.707106,
    .y = -0.707106,
};

float algo_steerangles_preserve[4];

void algo_get_steerBetter_vec(int velo_now, float angleDeg_now, int* velo_targ, float* angleDeg_targ)
{
    // 先将极坐标系下的velo和angle映射到，直角v_theta坐标系下，并生成一组点列
    // 对于v, theta，有(v, theta)和(-v, theta -180)，(-v, theta +180)三种实现方法（点列是周期的）,我们找到这三种中舵向最近的那个点，返回回去

    // 限制在 0 ~ 360度
    while (*angleDeg_targ < 0)
    {
        *angleDeg_targ += 360;
    }
    while (*angleDeg_targ > 360)
    {
        *angleDeg_targ -= 360;
    }
    
    
    int velo_mid = *velo_targ;
    float angleDeg_mid = *angleDeg_targ;

    int velo_left = -*velo_targ;
    float angleDeg_left = *angleDeg_targ - 180.0;
    int velo_right = -*velo_targ;
    float angleDeg_right = *angleDeg_targ + 180.0;

    float steer_distan_mid = fabs(angleDeg_mid - angleDeg_now);
    float steer_distan_left = fabs(angleDeg_left - angleDeg_now);
    float steer_distan_right = fabs(angleDeg_right - angleDeg_now);

    // 选择舵向最小的方法
    if (steer_distan_left < steer_distan_mid && steer_distan_left < steer_distan_right)
    {
        *velo_targ = velo_left;
        *angleDeg_targ = angleDeg_left;
    }
    if (steer_distan_right < steer_distan_mid && steer_distan_right < steer_distan_left)
    {
        *velo_targ = velo_right;
        *angleDeg_targ = angleDeg_right;
    }
}

void algo_vec2_add(Vec2* vec_1, Vec2* vec_2)
{
    vec_1->x += vec_2->x;
    vec_1->y += vec_2->y;
}

Vec2 algo_vec2_add_tonew(Vec2 vec_1, Vec2 vec_2)
{
    Vec2 new_vec;
    new_vec.x = vec_1.x + vec_2.x;
    new_vec.y = vec_1.y + vec_2.y;
    return new_vec;
}

Vec3 algo_vec3_add_tonew(Vec3 vec_1, Vec3 vec_2)
{
    Vec3 new_vec;
    new_vec.x = vec_1.x + vec_2.x;
    new_vec.y = vec_1.y + vec_2.y;
    new_vec.z = vec_1.z + vec_2.z;
    return new_vec;
}

void algo_vec2_add_xy(Vec2* vec_1, float x, float y)
{
    vec_1->x += x;
    vec_1->y += y;
}

void algo_vec2_multiply(Vec2* vec, float k)
{
    vec->x *= k;
    vec->y *= k;
}

void algo_vec2_to_polesys(Vec2* vec)
{
    float x_temp = vec->x;
    float y_temp = vec->y;

    // 计算极坐标系下的R
    vec->x = sqrt(x_temp * x_temp + y_temp * y_temp);

    // 计算极坐标系下的theta
    if (fabs(x_temp) < 0.003)
    {
        if (y_temp < -0.003)
        {
            vec->y = 270.0;
            return;
        }
        else if (y_temp > 0.003)
        {
            vec->y = 90.0;
            return;
        }
        else
        {
            vec->y = 0.0;
            return;
        }
    }
    
    if (x_temp >= 0 && y_temp >= 0)       // 第一象限
    {
        vec->y = (atan(y_temp / x_temp) / MATH_PI * 180);
    }
    else if (x_temp <= 0 && y_temp >= 0)     // 第二象限
    {
        vec->y = (atan(y_temp / x_temp) / MATH_PI * 180) + 180.0;
    }
    else if (x_temp <= 0 && y_temp <= 0)     // 第三象限
    {
        vec->y = (atan(y_temp / x_temp) / MATH_PI * 180) + 180.0;
    }
    else if (x_temp >= 0 && y_temp <= 0)     // 第四象限
    {
        vec->y = (atan(y_temp / x_temp) / MATH_PI * 180) + 360.0;
    }
    else
    {
        vec->y = (atan(y_temp / x_temp) / MATH_PI * 180);
    }
}


void algo_polesys_to_vec2(Vec2* vec)
{
    float R_temp = vec->x;
    float theta_temp = vec->y;

    // 将角度转换为弧度
    float theta_rad = theta_temp * MATH_PI / 180.0;

    // 处理特殊角度以优化计算
    if (fabs(theta_temp - 90.0f) < 0.003f) {
        vec->x = 0.0f;
        vec->y = R_temp;
        return;
    }
    else if (fabs(theta_temp - 270.0f) < 0.003f) {
        vec->x = 0.0f;
        vec->y = -R_temp;
        return;
    }

    // 计算直角坐标系下的X分量
    float cos_theta = cos(theta_rad);
    
    // 计算直角坐标系下的Y分量
    float sin_theta = sin(theta_rad);

    vec->x = R_temp * cos_theta;
    vec->y = R_temp * sin_theta;
}



void algo_calc_steer_vecs_4(float V_x, float V_y, float V_w, Vec2 Str_Ms[])
{
    // 速度向量叠加
    Vec2 StrM_1 = 
    {
        .x = 0,
        .y = 0,
    };
    algo_vec2_add_xy(&StrM_1, V_x, V_y);
    algo_vec2_add_xy(&StrM_1, unit_135_vec2.x * V_w, unit_135_vec2.y * V_w);

    Vec2 StrM_2 = 
    {
        .x = 0,
        .y = 0,
    };
    algo_vec2_add_xy(&StrM_2, V_x, V_y);
    algo_vec2_add_xy(&StrM_2, unit_225_vec2.x * V_w, unit_225_vec2.y * V_w);

    Vec2 StrM_3 = 
    {
        .x = 0,
        .y = 0,
    };
    algo_vec2_add_xy(&StrM_3, V_x, V_y);
    algo_vec2_add_xy(&StrM_3, unit_315_vec2.x * V_w, unit_315_vec2.y * V_w);

    Vec2 StrM_4 = 
    {
        .x = 0,
        .y = 0,
    };
    algo_vec2_add_xy(&StrM_4, V_x, V_y);
    algo_vec2_add_xy(&StrM_4, unit_45_vec2.x * V_w, unit_45_vec2.y * V_w);
    // 目前为止，各轮向量均为m/s单位

    // 将各向量转换到极坐标系下。
    algo_vec2_to_polesys(&StrM_1);
    algo_vec2_to_polesys(&StrM_2);
    algo_vec2_to_polesys(&StrM_3);
    algo_vec2_to_polesys(&StrM_4);

    // 换算成 ERPM
    StrM_1.x *= MeterPerSec_2_ERPM;
    StrM_2.x *= MeterPerSec_2_ERPM;
    StrM_3.x *= MeterPerSec_2_ERPM;
    StrM_4.x *= MeterPerSec_2_ERPM;

    // 填回舵轮解算
    Str_Ms[0] = StrM_1;
    Str_Ms[1] = StrM_2;
    Str_Ms[2] = StrM_3;
    Str_Ms[3] = StrM_4;
}

void algo_js_to_chassis_vec(Vec3 *chassis_vec, RmtJoystickInfo js_info)
{
    chassis_vec->x = js_info.right_stick_x / 10000.0;
    chassis_vec->y = js_info.right_stick_y / 10000.0;
    chassis_vec->z = js_info.left_stick_x / 10000.0;
}

Vec3 algo_OdomVec_to_LocalVec(Vec3 Odom_Vec)
{
    Vec3 Local_Vec;
    Local_Vec.x = Odom_Vec.y;
    Local_Vec.y = Odom_Vec.x;
    Local_Vec.z = Odom_Vec.z;
    return Local_Vec;
}


/**
 * @note 能够缩短Vec的长度
 */
void algo_chassis_vec_constrain(Vec3 *chassis_vec, float limit)
{
    Vec2 chassis_vec2 = {chassis_vec->x, chassis_vec->y};
    algo_vec2_to_polesys(&chassis_vec2);
    if (chassis_vec2.x <= limit)       // 长度小于limit的，直接不予理会
    {
        chassis_vec->x = 0;
        chassis_vec->y = 0;
    }
    else       // 长度大于limit的，缩短 limit 的长度
    {
        chassis_vec2.x -= limit;
        algo_polesys_to_vec2(&chassis_vec2);
        chassis_vec->x = chassis_vec2.x;
        chassis_vec->y = chassis_vec2.y;
    }
}