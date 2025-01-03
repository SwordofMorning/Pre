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

/* ----- V4L2 CSI ---- */

int v4l2_vis_csi_fd;
int v4l2_vis_csi_nplanes;
int v4l2_vis_csi_buffer_global_index;
int v4l2_vis_csi_buffer_global_length;
struct V4L2_VIS_CSI_Buffer* v4l2_vis_csi_buffer_global;

int v4l2_vis_csi_mode;
int v4l2_vis_csi_width;
int v4l2_vis_csi_height;

/* ----- SHM ---- */

struct FrameSync16 frame_sync_dvp;
struct FrameSync8 frame_sync_csi;

// Buffer
uint16_t* algo_in = NULL;
uint8_t* shm_out_yuv = NULL;
float* shm_out_float = NULL;
uint8_t* shm_out_algo = NULL;

// ID
int shmid_yuv = -1;
int shmid_float = -1;
int shmid_csi = -1;
int shmid_algo = -1;
int semid_vo = -1;
int semid_ab = -1;

// Pointer
uint8_t* shm_yuv = NULL;
float* shm_float = NULL;
uint8_t* shm_vis = NULL;
uint8_t* shm_algo = NULL;

/* ----- User Config ---- */

struct UserConfig usr;

/* ========================================================================================== */
/* ======================================== Function ======================================== */
/* ========================================================================================== */

static void Init_Frame_Sync_DVP()
{
    pthread_mutex_init(&frame_sync_dvp.mutex, NULL);
    pthread_cond_init(&frame_sync_dvp.producer_cond, NULL);
    pthread_cond_init(&frame_sync_dvp.consumer_cond, NULL);
    frame_sync_dvp.write_pos = 0;
    frame_sync_dvp.read_pos = 0;
    frame_sync_dvp.frame_count = 0;
    frame_sync_dvp.buffer_full = false;
    gettimeofday(&frame_sync_dvp.last_frame_time, NULL);

    for (int i = 0; i < FRAME_SYNC_BUFFER_SIZE; i++)
    {
        frame_sync_dvp.frame_buffer[i] = (uint16_t*)malloc(v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(uint16_t));
    }
}

static void Init_DVP()
{
    /* Without any action  */
    v4l2_ir_dvp_fd = 0;
    v4l2_ir_dvp_nplanes = 0;
    v4l2_ir_dvp_buffer_global_index = 0;
    v4l2_ir_dvp_buffer_global_length = 0;

    /* Default in 640 mode */
    v4l2_ir_dvp_mode = V4L2_IR_DVP_MODE_640;
    v4l2_ir_dvp_capture_width = V4L2_IR_DVP_CAPTURE_WIDTH_640;
    v4l2_ir_dvp_capture_height = V4L2_IR_DVP_CAPTURE_HEIGHT_640;
    v4l2_ir_dvp_valid_width = V4L2_IR_DVP_VALID_WIDTH_640;
    v4l2_ir_dvp_valid_height = V4L2_IR_DVP_VALID_HEIGHT_640;

    /* DVP shm */
    v4l2_ir_dvp_share_buffer_updated = 0;

    Init_Frame_Sync_DVP();
}

static void Init_Frame_Sync_CSI()
{
    pthread_mutex_init(&frame_sync_csi.mutex, NULL);
    pthread_cond_init(&frame_sync_csi.producer_cond, NULL);
    pthread_cond_init(&frame_sync_csi.consumer_cond, NULL);
    frame_sync_csi.write_pos = 0;
    frame_sync_csi.read_pos = 0;
    frame_sync_csi.frame_count = 0;
    frame_sync_csi.buffer_full = false;
    gettimeofday(&frame_sync_csi.last_frame_time, NULL);

    for (int i = 0; i < FRAME_SYNC_BUFFER_SIZE; i++)
    {
        frame_sync_csi.frame_buffer[i] = (uint8_t*)malloc(v4l2_vis_csi_width * v4l2_vis_csi_height * sizeof(uint8_t) * V4L2_VIS_CSI_PIX_FMT_SCALE);
    }
}

static void Init_CIS()
{
    /* Without any action  */
    v4l2_vis_csi_fd = 0;
    v4l2_vis_csi_nplanes = 0;
    v4l2_vis_csi_buffer_global_index = 0;
    v4l2_vis_csi_buffer_global_length = 0;

    /* Set default mode */
#if (VISIBLE_CAMERA_MODE == VISIBLE_CAMERA_MODE_HGD_IMX335)
    v4l2_vis_csi_mode = V4L2_VIS_CSI_MODE_HGD_2592x1944at30;
#elif (VISIBLE_CAMERA_MODE == VISIBLE_CAMERA_MODE_NGD)
    v4l2_vis_csi_mode = V4L2_VIS_CSI_MODE_NGD_2688x1520at25;
#endif

    if (v4l2_vis_csi_mode == V4L2_VIS_CSI_MODE_HGD_2592x1944at30)
    {
        v4l2_vis_csi_width = V4L2_VIS_CSI_CAPTURE_WIDTH_2592;
        v4l2_vis_csi_height = V4L2_VIS_CSI_CAPTURE_HEIGHT_1944;
    }
    else if (v4l2_vis_csi_mode == V4L2_VIS_CSI_MODE_NGD_2688x1520at25)
    {
        v4l2_vis_csi_width = V4L2_VIS_CSI_CAPTURE_WIDTH_2688;
        v4l2_vis_csi_height = V4L2_VIS_CSI_CAPTURE_HEIGHT_1520;
    }

    Init_Frame_Sync_CSI();
}

static void Init_User_Config()
{
    usr.pseudo = PSEUDO_IRONBOW_FORWARD;
    usr.gas_enhancement = GAS_ENHANCEMENT_NONE;
    usr.in_focus = false;
}

static int Init_LUTs()
{
    if (Init_LUT(LUT_IRONBOW_FORWARD, "/root/app/pseudo/ironbow_forward.bin") < 0)
    {
        return -1;
    }
    if (Init_LUT(LUT_IRONBOW_REVERSE, "/root/app/pseudo/ironbow_reverse.bin") < 0)
    {
        return -1;
    }
    if (Init_LUT(LUT_LAVA_FORWARD, "/root/app/pseudo/lava_forward.bin") < 0)
    {
        return -1;
    }
    if (Init_LUT(LUT_LAVA_REVERSE, "/root/app/pseudo/lava_reverse.bin") < 0)
    {
        return -1;
    }
    if (Init_LUT(LUT_RAINBOW_FORWARD, "/root/app/pseudo/rainbow_forward.bin") < 0)
    {
        return -1;
    }
    if (Init_LUT(LUT_RAINBOW_REVERSE, "/root/app/pseudo/rainbow_reverse.bin") < 0)
    {
        return -1;
    }
    if (Init_LUT(LUT_RAINBOWHC_FORWARD, "/root/app/pseudo/rainbowhc_forward.bin") < 0)
    {
        return -1;
    }
    if (Init_LUT(LUT_RAINBOWHC_REVERSE, "/root/app/pseudo/rainbowhc_reverse.bin") < 0)
    {
        return -1;
    }

    if (!PseudoCL_Init(&cl_processor, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height))
    {
        printf("Failed to initialize OpenCL\n");
        return -1;
    }

    return 0;
}

static void Init_Log()
{
    litelog.init("Pre");
    litelog.log.notice("========== Program Version ==========");
    litelog.log.notice("Git User: %s", __GIT_USER__);
    litelog.log.notice("Git Branch: %s", __GIT_BRANCH__);
    litelog.log.notice("Git Commit: %s", __GIT_COMMIT_ID__);
    litelog.log.notice("Git Worktree: %s", __GIT_CLEAN__);
    litelog.log.notice("Compile Host: %s", __COMPILE_HOST__);
    litelog.log.notice("Compile User: %s", __COMPILE_USER__);
    litelog.log.notice("Compile Time: %s", __COMPILE_TIME__);
    litelog.log.notice("=====================================");
}

/* ===================================================================================== */
/* ======================================== API ======================================== */
/* ===================================================================================== */

void Config_Init()
{
    Init_Log();
    Init_User_Config();
    Init_DVP();
    Init_CIS();
    Init_LUTs();
}

void Config_Exit()
{
    PseudoCL_Cleanup(&cl_processor);
    Free_All_LUTs();
}