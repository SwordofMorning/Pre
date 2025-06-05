#include "pseudo.h"

void Pseudo::Pseudo_NV12_CL(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height, float cb_min, float cb_max)
{
    PAM_mapper.UpdateRange(input, width, height);
    float scale = PAM_mapper.GetScale();
    float min_val = PAM_mapper.GetMin();

    const struct YUV420P_LUT* lut = NULL;

    if (usr.pseudo != PSEUDO_BLACK_HOT && usr.pseudo != PSEUDO_WHITE_HOT)
        lut = Get_LUT(usr.pseudo);

    // clang-format off
    if (PseudoCL_ProcessNV12(&pseudo_cl, input, y_out, uv_out, 
                            width, height, usr.pseudo, lut, 
                            scale, min_val, cb_min, cb_max) != 0)
        litelog.log.warning("PseudoCL_ProcessNV12 processing failed\n");
    // clang-format on
}

void Pseudo::Pseudo_Isotherms_CL(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height, float* temps, float threshold_min, float threshold_max, uint8_t* uv_maps)
{
    PAM_mapper.UpdateRange(input, width, height);
    float scale = PAM_mapper.GetScale();
    float min_val = PAM_mapper.GetMin();

    const struct YUV420P_LUT* lut = NULL;

    if (usr.pseudo != PSEUDO_BLACK_HOT && usr.pseudo != PSEUDO_WHITE_HOT)
        lut = Get_LUT(usr.pseudo);

    // clang-format off
    if (PseudoCL_ProcessIsotherms(&pseudo_cl, input, y_out, uv_out, 
                            width, height, lut, scale, min_val, temps, threshold_min, threshold_max, uv_maps) != 0)
        litelog.log.warning("PseudoCL_ProcessNV12 processing failed\n");
    // clang-format on
}

void Pseudo::operator()(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height, float cb_min, float cb_max, float* temps)
{
    uint8_t UV_MAP[6] = {0, 0, 128, 0, 255, 255};  // 映射表

    // isotherms mode
    if (true)
        return this->Pseudo_Isotherms_CL(input, y_out, uv_out, width, height, temps, 30.f, 40.f, UV_MAP);
    else
        return this->Pseudo_NV12_CL(input, y_out, uv_out, width, height, cb_min, cb_max);
}