#include "config.h"

/* ======================================================================================== */
/* ======================================== Define ======================================== */
/* ======================================================================================== */

/* ----- V4L2 DVP ---- */

int v4l2_ir_dvp_mode;
int v4l2_ir_dvp_capture_width;
int v4l2_ir_dvp_capture_height;
int v4l2_ir_dvp_valid_width;
int v4l2_ir_dvp_valid_height;

int v4l2_ir_dvp_fd;
int v4l2_ir_dvp_nplanes;
int v4l2_ir_dvp_buffer_global_index;
int v4l2_ir_dvp_buffer_global_length;

struct V4L2_IR_DVP_Buffer* v4l2_ir_dvp_buffer_global;

uint16_t v4l2_ir_dvp_share_buffer[V4L2_IR_DVP_VALID_WIDTH_640 * V4L2_IR_DVP_VALID_HEIGHT_640];
pthread_mutex_t v4l2_ir_dvp_share_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t v4l2_ir_dvp_share_buffer_cond = PTHREAD_COND_INITIALIZER;
int v4l2_ir_dvp_share_buffer_updated;

/* ----- Pseudo Color ----- */

int pseudo_color;

/* ----- Gas Enhancement ----- */
int gas_enhancement;

/* ----- Socket ---- */

int sockfd;
int rws;
uint8_t socket_buffer[128];

/* ----- Script ---- */

pthread_t guardian_thread;

/* ========================================================================================== */
/* ======================================== Function ======================================== */
/* ========================================================================================== */

void Config_Init()
{
    /* Init: Without any action  */
    v4l2_ir_dvp_fd = 0;
    v4l2_ir_dvp_nplanes = 0;
    v4l2_ir_dvp_buffer_global_index = 0;
    v4l2_ir_dvp_buffer_global_length = 0;

    /* Init: Default in 640 mode */
    v4l2_ir_dvp_mode = V4L2_IR_DVP_MODE_640;
    v4l2_ir_dvp_capture_width = V4L2_IR_DVP_CAPTURE_WIDTH_640;
    v4l2_ir_dvp_capture_height = V4L2_IR_DVP_CAPTURE_HEIGHT_640;
    v4l2_ir_dvp_valid_width = V4L2_IR_DVP_VALID_WIDTH_640;
    v4l2_ir_dvp_valid_height = V4L2_IR_DVP_VALID_HEIGHT_640;

    /* Init: DVP shm */
    v4l2_ir_dvp_share_buffer_updated = 0;

    /* Init: pseudo color */
    pseudo_color = PSEUDO_COLOR_IRON_BOW;

    /* Init: gas enhancement */
    gas_enhancement = GAS_ENHANCEMENT_NONE;
}

void Config_Exit()
{
    // do nothing
}