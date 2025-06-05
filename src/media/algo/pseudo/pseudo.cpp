#include "pseudo.h"

void Pseudo::Pseudo_NV12_CL(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height)
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
                            scale, min_val, usr.color_bar_min, usr.color_bar_max) != 0)
        litelog.log.warning("PseudoCL_ProcessNV12 processing failed\n");
    // clang-format on
}

void Pseudo::Pseudo_Isotherms_CL(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height, float* temps)
{
    PAM_mapper.UpdateRange(input, width, height);
    float scale = PAM_mapper.GetScale();
    float min_val = PAM_mapper.GetMin();

    const struct YUV420P_LUT* lut = NULL;

    if (usr.pseudo != PSEUDO_BLACK_HOT && usr.pseudo != PSEUDO_WHITE_HOT)
        lut = Get_LUT(usr.pseudo);

    // clang-format off
    if (PseudoCL_ProcessIsotherms(&pseudo_cl, input, y_out, uv_out, 
                            width, height, lut, scale, min_val,
                            temps, usr.isothermal_threshold_min, usr.isothermal_threshold_max, usr.isothermal_uv_map) != 0)
        litelog.log.warning("PseudoCL_ProcessNV12 processing failed\n");
    // clang-format on
}

void Pseudo::operator()(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height, float* temps)
{
    // isotherms mode
    if (usr.isothermal)
        return this->Pseudo_Isotherms_CL(input, y_out, uv_out, width, height, temps);
    else
        return this->Pseudo_NV12_CL(input, y_out, uv_out, width, height);
}