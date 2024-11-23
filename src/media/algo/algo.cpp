#include "algo.h"

// int Process_One_Frame()
// {
//     // 指针设置
//     uint8_t* y = algo_out_yuv;
//     uint8_t* u = y + v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height;
//     uint8_t* v = u + (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height / 4);

//     // 寻找数据范围（用于float输出）
//     uint16_t min_val = 65535;
//     uint16_t max_val = 0;
//     for(int i = 0; i < v4l2_ir_dvp_valid_height; i++)
//     {
//         for(int j = 0; j < v4l2_ir_dvp_valid_width; j++)
//         {
//             uint16_t val = algo_in[i * v4l2_ir_dvp_valid_width + j];
//             if(val > max_val) max_val = val;
//             if(val < min_val) min_val = val;
//         }
//     }

//     // float输出
//     float scale = 1.0f / (max_val - min_val);
//     for(int i = 0; i < v4l2_ir_dvp_valid_height; i++)
//     {
//         for(int j = 0; j < v4l2_ir_dvp_valid_width; j++)
//         {
//             uint16_t val = algo_in[i * v4l2_ir_dvp_valid_width + j];
//             algo_out_float[i * v4l2_ir_dvp_valid_width + j] = 
//                 (float)(val - min_val) * scale;
//         }
//     }

//     // YUV420P输出（使用伪彩色）
//     Pseudo(algo_in, y, u, v, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height);

//     return 0;
// }

int Process_One_Frame()
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

void Pseudo(uint16_t* input, uint8_t* y_out, uint8_t* u_out, uint8_t* v_out, int width, int height)
{
    // Y分量直接映射
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            uint16_t val = input[i * width + j];
            y_out[i * width + j] = lava_lut.y[val];
        }
    }

    // UV分量需要4:2:0采样
    for(int i = 0; i < height/2; i++)
    {
        for(int j = 0; j < width/2; j++)
        {
            // 取2x2块的平均值
            uint16_t val00 = input[(i*2) * width + (j*2)];
            uint16_t val01 = input[(i*2) * width + (j*2+1)];
            uint16_t val10 = input[(i*2+1) * width + (j*2)];
            uint16_t val11 = input[(i*2+1) * width + (j*2+1)];
            
            uint32_t avg_val = (val00 + val01 + val10 + val11) / 4;
            
            u_out[i * (width/2) + j] = lava_lut.u[avg_val];
            v_out[i * (width/2) + j] = lava_lut.v[avg_val];
        }
    }
}