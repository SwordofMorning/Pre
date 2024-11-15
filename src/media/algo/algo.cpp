#include "algo.h"

int algo_convert()
{
    // 转换为YUV420P
    uint8_t* y = algo_out_yuv;
    uint8_t* u = y + v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height;
    uint8_t* v = u + (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height / 4);

    // 添加数据统计
    uint16_t min_val = 65535;
    uint16_t max_val = 0;

    // 第一遍扫描：找出数据范围
    for (int i = 0; i < v4l2_ir_dvp_valid_height; i++)
    {
        for (int j = 0; j < v4l2_ir_dvp_valid_width; j++)
        {
            uint16_t val = algo_in[i * v4l2_ir_dvp_valid_width + j];
            if (val > max_val)
                max_val = val;
            if (val < min_val)
                min_val = val;
        }
    }

    // 计算映射系数
    float scale = 255.0f / (max_val - min_val);

    // 转换数据
    for (int i = 0; i < v4l2_ir_dvp_valid_height; i++)
    {
        for (int j = 0; j < v4l2_ir_dvp_valid_width; j++)
        {
            uint16_t val = algo_in[i * v4l2_ir_dvp_valid_width + j];

            // 归一化到0-1范围（用于float输出）
            algo_out_float[i * v4l2_ir_dvp_valid_width + j] = (float)(val - min_val) / (max_val - min_val);

            // 线性映射到0-255范围（用于YUV输出）
            y[i * v4l2_ir_dvp_valid_width + j] = (uint8_t)((val - min_val) * scale);
        }
    }

    // 生成UV分量
    for (int i = 0; i < v4l2_ir_dvp_valid_height / 2; i++)
    {
        for (int j = 0; j < v4l2_ir_dvp_valid_width / 2; j++)
        {
            u[i * (v4l2_ir_dvp_valid_width / 2) + j] = 128;
            v[i * (v4l2_ir_dvp_valid_width / 2) + j] = 128;
        }
    }

    return 0;
}