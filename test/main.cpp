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

// 使用与algo模块相同的定义
#define ALGO_SHM_YUV_KEY 0x0010
#define ALGO_SHM_FLOAT_KEY 0x0011
#define ALGO_SEM_KEY 0x0012

#define ALGO_WIDTH 640
#define ALGO_HEIGHT 512
#define ALGO_YUV_SIZE (ALGO_WIDTH * ALGO_HEIGHT * 3 / 2)
#define ALGO_FLOAT_SIZE (ALGO_WIDTH * ALGO_HEIGHT * sizeof(float))

// 全局变量
static int running = 1;
static int shmid_yuv = -1;
static int shmid_float = -1;
static int semid = -1;
static uint8_t* shm_yuv = NULL;
static float* shm_float = NULL;

// 信号处理函数
void signal_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nReceived SIGINT, preparing to exit...\n");
        running = 0;
    }
}

// 生成带时间戳的文件名
void generate_filename(char* yuv_filename, char* float_filename) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    sprintf(yuv_filename, "capture_%04d%02d%02d_%02d%02d%02d.yuv",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
            
    sprintf(float_filename, "capture_%04d%02d%02d_%02d%02d%02d.float",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
}

int main() {
    // 设置信号处理
    signal(SIGINT, signal_handler);

    // 获取共享内存
    shmid_yuv = shmget(ALGO_SHM_YUV_KEY, ALGO_YUV_SIZE, 0666);
    shmid_float = shmget(ALGO_SHM_FLOAT_KEY, ALGO_FLOAT_SIZE, 0666);
    if (shmid_yuv < 0 || shmid_float < 0) {
        perror("shmget");
        return -1;
    }

    // 附加到共享内存
    shm_yuv = (uint8_t*)shmat(shmid_yuv, NULL, 0);
    shm_float = (float*)shmat(shmid_float, NULL, 0);
    if (shm_yuv == (void*)-1 || shm_float == (void*)-1) {
        perror("shmat");
        return -1;
    }

    // 获取信号量
    semid = semget(ALGO_SEM_KEY, 1, 0666);
    if (semid < 0) {
        perror("semget");
        return -1;
    }

    printf("Start capturing data...\n");
    printf("Press Ctrl+C to stop.\n");

    struct sembuf sem_op;
    int frame_count = 0;
    
    while (running) {
        // 等待信号量
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        if (semop(semid, &sem_op, 1) < 0) {
            perror("semop");
            break;
        }

        // 生成文件名
        char yuv_filename[100];
        char float_filename[100];
        generate_filename(yuv_filename, float_filename);

        // 保存YUV数据
        FILE* fp_yuv = fopen(yuv_filename, "wb");
        if (fp_yuv) {
            fwrite(shm_yuv, 1, ALGO_YUV_SIZE, fp_yuv);
            fclose(fp_yuv);
        }

        // 保存float数据
        FILE* fp_float = fopen(float_filename, "wb");
        if (fp_float) {
            fwrite(shm_float, 1, ALGO_FLOAT_SIZE, fp_float);
            fclose(fp_float);
        }

        // 释放信号量
        sem_op.sem_op = 1;
        if (semop(semid, &sem_op, 1) < 0) {
            perror("semop");
            break;
        }

        frame_count++;
        printf("\rCaptured frames: %d", frame_count);
        fflush(stdout);

        // 添加适当的延时，避免产生过多文件
        usleep(100000);  // 100ms delay
    }

    printf("\nTotal captured frames: %d\n", frame_count);

    // 清理工作
    if (shm_yuv != (void*)-1) shmdt(shm_yuv);
    if (shm_float != (void*)-1) shmdt(shm_float);

    printf("Capture program terminated.\n");
    return 0;
}