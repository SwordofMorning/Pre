#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <stdarg.h>

#include "device.h"
#include "enum.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IR */

// Share memory of dvp captured data.
extern uint16_t v4l2_ir_dvp_share_buffer[V4L2_IR_DVP_VALID_WIDTH_640 * V4L2_IR_DVP_VALID_HEIGHT_640];
// Mutex of dvp shm.
extern pthread_mutex_t v4l2_ir_dvp_share_buffer_mutex;
// Cond of dvp shm.
extern pthread_cond_t v4l2_ir_dvp_share_buffer_cond;
// Count of dvp shm.
extern int v4l2_ir_dvp_share_buffer_updated;

/* Config */

// Gas Enhancement Param.
extern int gas_enhancement;

// To specific which pseudo color mode we use.
extern int pseudo_color;

#ifdef __cplusplus
}
#endif