#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

#define SAVE_FILE 1

// 共享内存和信号量的key
#define ALGO_SHM_YUV_KEY 0x0010
#define ALGO_SHM_FLOAT_KEY 0x0011
#define ALGO_SHM_CSI_KEY 0x0012
#define ALGO_SHM_ALGO_KEY 0x0013
#define ALGO_SEM_KEY 0x0020

// 图像尺寸和缓冲区大小定义
#define ALGO_WIDTH 640
#define ALGO_HEIGHT 512
#define ALGO_YUV_SIZE (ALGO_WIDTH * ALGO_HEIGHT * 3 / 2)
#define ALGO_FLOAT_SIZE (ALGO_WIDTH * ALGO_HEIGHT * sizeof(float))
#define ALGO_CSI_SIZE (2592 * 1944 * 2)
#define ALGO_ALGO_SIZE ALGO_YUV_SIZE

// FPS计算相关设置
#define FPS_UPDATE_INTERVAL 1.0

static FILE* fp_yuv = NULL;
static FILE* fp_float = NULL;
static FILE* fp_csi = NULL;
static FILE* fp_algo = NULL;

// 全局变量
static int running = 1;
static int shmid_yuv = -1;
static int shmid_float = -1;
static int semid = -1;
static uint8_t* shm_yuv = NULL;
static float* shm_float = NULL;
static int shmid_csi = -1;
static int shmid_algo = -1;
static uint8_t* shm_csi = NULL;
static uint8_t* shm_algo = NULL;

// FPS统计相关变量
static struct timeval last_fps_time;
static int fps_frame_count = 0;
static double current_fps = 0.0;

// 信号处理函数
void signal_handler(int signo)
{
    if (signo == SIGINT)
    {
        printf("\nReceived SIGINT, preparing to exit...\n");
        running = 0;
    }
}

// 生成带时间戳的文件名
void generate_filename(char* yuv_filename, char* float_filename, 
                      char* csi_filename, char* algo_filename)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    sprintf(yuv_filename, "capture_%04d%02d%02d_%02d%02d.yuv",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min);

    sprintf(float_filename, "capture_%04d%02d%02d_%02d%02d.float",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min);

    sprintf(csi_filename, "capture_%04d%02d%02d_%02d%02d.csi",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min);

    sprintf(algo_filename, "capture_%04d%02d%02d_%02d%02d.algo",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min);
}

// FPS统计函数
void update_fps()
{
    fps_frame_count++;

    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    double time_diff = (current_time.tv_sec - last_fps_time.tv_sec) + 
                      (current_time.tv_usec - last_fps_time.tv_usec) / 1000000.0;

    if (time_diff >= FPS_UPDATE_INTERVAL)
    {
        current_fps = fps_frame_count / time_diff;
        fps_frame_count = 0;
        last_fps_time = current_time;
    }
}

// 初始化函数
int init_resources()
{
    // 获取共享内存
    shmid_yuv = shmget(ALGO_SHM_YUV_KEY, ALGO_YUV_SIZE, 0666);
    shmid_float = shmget(ALGO_SHM_FLOAT_KEY, ALGO_FLOAT_SIZE, 0666);
    shmid_csi = shmget(ALGO_SHM_CSI_KEY, ALGO_CSI_SIZE, 0666);
    shmid_algo = shmget(ALGO_SHM_ALGO_KEY, ALGO_ALGO_SIZE, 0666);
    
    if (shmid_yuv < 0 || shmid_float < 0 || shmid_csi < 0 || shmid_algo < 0)
    {
        perror("shmget failed");
        return -1;
    }

    // 附加到共享内存
    shm_yuv = (uint8_t*)shmat(shmid_yuv, NULL, 0);
    shm_float = (float*)shmat(shmid_float, NULL, 0);
    shm_csi = (uint8_t*)shmat(shmid_csi, NULL, 0);
    shm_algo = (uint8_t*)shmat(shmid_algo, NULL, 0);
    
    if (shm_yuv == (void*)-1 || shm_float == (void*)-1 || 
        shm_csi == (void*)-1 || shm_algo == (void*)-1)
    {
        perror("shmat failed");
        return -1;
    }

    // 获取信号量
    semid = semget(ALGO_SEM_KEY, 1, 0666);
    if (semid < 0)
    {
        perror("semget failed");
        return -1;
    }

#if SAVE_FILE
    // 生成文件名并打开文件
    char yuv_filename[100];
    char float_filename[100];
    char csi_filename[100];
    char algo_filename[100];
    
    generate_filename(yuv_filename, float_filename, 
                     csi_filename, algo_filename);

    fp_yuv = fopen(yuv_filename, "wb");
    fp_float = fopen(float_filename, "wb");
    fp_csi = fopen(csi_filename, "wb");
    fp_algo = fopen(algo_filename, "wb");
    
    if (!fp_yuv || !fp_float || !fp_csi || !fp_algo)
    {
        perror("Failed to open files");
        return -1;
    }

    printf("Recording to files:\n");
    printf("YUV: %s\n", yuv_filename);
    printf("Float: %s\n", float_filename);
    printf("CSI: %s\n", csi_filename);
    printf("Algo: %s\n", algo_filename);
#endif

    return 0;
}

// 清理资源函数
void cleanup_resources()
{
    if (shm_yuv != (void*)-1) shmdt(shm_yuv);
    if (shm_float != (void*)-1) shmdt(shm_float);
    if (shm_csi != (void*)-1) shmdt(shm_csi);
    if (shm_algo != (void*)-1) shmdt(shm_algo);

#if SAVE_FILE
    if (fp_yuv) fclose(fp_yuv);
    if (fp_float) fclose(fp_float);
    if (fp_csi) fclose(fp_csi);
    if (fp_algo) fclose(fp_algo);
#endif
}

// 保存一帧数据
int save_frame()
{
    static uint8_t last_frame_sum = 0;
    uint8_t current_frame_sum = 0;
    
    // 计算当前帧的校验和
    for(int i = 0; i < 100; i++)
    {
        current_frame_sum += shm_yuv[i];
    }
    
    if(current_frame_sum == last_frame_sum)
    {
        printf("Duplicate frame detected\n");
    }
    last_frame_sum = current_frame_sum;

    // 数据验证
    bool has_yuv_data = false;
    bool has_float_data = false;
    bool has_csi_data = false;
    bool has_algo_data = false;
    
    // 验证数据
    for(int i = 0; i < ALGO_YUV_SIZE; i++) 
        if(shm_yuv[i] > 0) { has_yuv_data = true; break; }
    
    for(int i = 0; i < ALGO_WIDTH * ALGO_HEIGHT; i++)
        if(shm_float[i] > 0) { has_float_data = true; break; }
    
    for(int i = 0; i < ALGO_CSI_SIZE; i++)
        if(shm_csi[i] > 0) { has_csi_data = true; break; }
    
    for(int i = 0; i < ALGO_ALGO_SIZE; i++)
        if(shm_algo[i] > 0) { has_algo_data = true; break; }
    
    printf("Frame received: yuv=%d, float=%d, csi=%d, algo=%d, checksum=%d\n",
           has_yuv_data, has_float_data, has_csi_data, has_algo_data, 
           current_frame_sum);

    if (!fp_yuv || !fp_float || !fp_csi || !fp_algo)
        return -1;

    // 写入数据
    if (fwrite(shm_yuv, 1, ALGO_YUV_SIZE, fp_yuv) != ALGO_YUV_SIZE ||
        fwrite(shm_float, 1, ALGO_FLOAT_SIZE, fp_float) != ALGO_FLOAT_SIZE ||
        fwrite(shm_csi, 1, ALGO_CSI_SIZE, fp_csi) != ALGO_CSI_SIZE ||
        fwrite(shm_algo, 1, ALGO_ALGO_SIZE, fp_algo) != ALGO_ALGO_SIZE)
    {
        perror("Failed to write data");
        return -1;
    }

    // 刷新文件缓冲
    fflush(fp_yuv);
    fflush(fp_float);
    fflush(fp_csi);
    fflush(fp_algo);

    return 0;
}

int main()
{
    // 设置信号处理
    signal(SIGINT, signal_handler);

    // 初始化资源
    if (init_resources() < 0)
    {
        printf("Failed to initialize resources\n");
        return -1;
    }

    // 初始化FPS统计
    gettimeofday(&last_fps_time, NULL);
    fps_frame_count = 0;

    printf("Start capturing data...\n");
    printf("Press Ctrl+C to stop.\n");

    struct sembuf sem_op;
    int frame_count = 0;
    
    // 记录开始时间
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    
    // 主循环-采集图像并保存
    while (running)
    {
        struct timeval frame_start_time;
        gettimeofday(&frame_start_time, NULL);

        // 等待信号量
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) < 0)
        {
            break;
        }

#if SAVE_FILE
        // 保存数据
        if (save_frame() < 0)
        {
            printf("Failed to save frame %d\n", frame_count);
        }
#endif

        // 释放信号量
        sem_op.sem_num = 0;
        sem_op.sem_op = 1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) < 0)
        {
            break;
        }

        frame_count++;
        update_fps();

        // 显示录制状态
        printf("\rRecorded frames: %d, Current FPS: %.2f", frame_count, current_fps);
        fflush(stdout);

        // 帧率控制-25fps
        struct timeval frame_end_time;
        gettimeofday(&frame_end_time, NULL);
        double frame_time = (frame_end_time.tv_sec - frame_start_time.tv_sec) +
                        (frame_end_time.tv_usec - frame_start_time.tv_usec) / 1000000.0;

        double target_frame_time = 1.0/25.0;
        if(frame_time < target_frame_time)
        {
            usleep((target_frame_time - frame_time) * 1000000);
        }
    }

    // 计算总体统计信息
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    
    double total_time = (end_time.tv_sec - start_time.tv_sec) + 
                       (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    
    // 打印统计信息
    printf("\n\nPerformance Statistics:\n");
    printf("Total frames captured: %d\n", frame_count);
    printf("Total running time: %.2f seconds\n", total_time);
    printf("Average FPS: %.2f\n", frame_count / total_time);

    // 清理资源
    cleanup_resources();
    printf("Capture program terminated.\n");

    return 0;
}