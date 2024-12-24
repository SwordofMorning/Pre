// #pragma once

// #include <gst/gst.h>
// #include <gst/app/app.h>
// #include <string.h>
// #include "../../config/config.h"
// #include "../../utils/log/litelog.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

// // 初始化GStreamer
// int GST_Init(void);

// // 释放GStreamer资源
// void GST_Deinit(void);

// // 创建GStreamer Pipeline
// int GST_Create_Pipeline(int width, int height);

// // 推送数据到GStreamer
// int GST_Push_Frame(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, int width, int height);

// // 主循环函数
// void* VO_GST_Streaming();

// #ifdef __cplusplus
// }
// #endif