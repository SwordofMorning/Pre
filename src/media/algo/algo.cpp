#include "algo.h"

int Process_One_Frame()
{
    // 指针设置
    uint8_t* y = algo_out_yuv;
    uint8_t* u = y + v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height;
    uint8_t* v = u + (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height / 4);

    // 寻找数据范围（用于float输出）
    uint16_t min_val = 65535;
    uint16_t max_val = 0;
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

    // float
    float scale = 1.0f / (max_val - min_val);
    for (int i = 0; i < v4l2_ir_dvp_valid_height; i++)
    {
        for (int j = 0; j < v4l2_ir_dvp_valid_width; j++)
        {
            uint16_t val = algo_in[i * v4l2_ir_dvp_valid_width + j];
            algo_out_float[i * v4l2_ir_dvp_valid_width + j] = (float)(val - min_val) * scale;
        }
    }

    // YUV420P
    Pseudo(algo_in, y, u, v, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height);

    GST_Push_Frame(y, u, v, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height);

    return 0;
}

void Pseudo(uint16_t* input, uint8_t* y_out, uint8_t* u_out, uint8_t* v_out, int width, int height)
{
    static PseudoAdaptiveMapper mapper;

    // Update value mapping
    mapper.UpdateRange(input, width, height);
    float scale = mapper.GetScale();
    float min_val = mapper.GetMin();

    // clang-format off
    switch(usr.pseudo)
    {
        case PSEUDO_BLACK_HOT:
        {
            // Y
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    uint16_t val = input[i * width + j];
                    float mapped = (val - min_val) * scale;
                    // 限制在0-255范围内
                    mapped = std::clamp(mapped, 0.0f, 255.0f);
                    y_out[i * width + j] = 255 - (uint8_t)mapped;
                }
            }

            // UV
            size_t uv_size = (width * height) / 4;
            std::memset(u_out, 128, uv_size);
            std::memset(v_out, 128, uv_size);
            break;
        }

        case PSEUDO_WHITE_HOT:
        {
            // Y
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    uint16_t val = input[i * width + j];
                    float mapped = (val - min_val) * scale;
                    mapped = std::clamp(mapped, 0.0f, 255.0f);
                    y_out[i * width + j] = (uint8_t)mapped;
                }
            }

            // UV
            size_t uv_size = (width * height) / 4;
            std::memset(u_out, 128, uv_size);
            std::memset(v_out, 128, uv_size);
            break;
        }

        default:
        {
            const struct YUV420P_LUT* lut = Get_LUT(usr.pseudo);
            if (!lut)
                break;

            // Y
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    uint16_t val = input[i * width + j];
                    float mapped = (val - min_val) * scale;
                    mapped = std::clamp(mapped, 0.0f, 255.0f);
                    int color_idx = (int)(mapped * (lut->size - 1) / 255);
                    y_out[i * width + j] = lut->y[color_idx];
                }
            }

            // UV, 420P
            for (int i = 0; i < height/2; i++)
            {
                for (int j = 0; j < width/2; j++)
                {
                    uint32_t sum = 0;
                    for(int di = 0; di < 2; di++)
                    {
                        for(int dj = 0; dj < 2; dj++)
                        {
                            sum += input[(i*2+di) * width + (j*2+dj)];
                        }
                    }
                    uint32_t avg_val = sum / 4;
                    
                    float mapped = (avg_val - min_val) * scale;
                    mapped = std::clamp(mapped, 0.0f, 255.0f);
                    int color_idx = (int)(mapped * (lut->size - 1) / 255);
                    u_out[i * (width/2) + j] = lut->u[color_idx];
                    v_out[i * (width/2) + j] = lut->v[color_idx];
                }
            }
            break;
        }
    }
    // clang-format on
}