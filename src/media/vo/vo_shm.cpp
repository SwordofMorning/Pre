#include "vo_shm.h"

// 内部缓冲区
static uint16_t* algo_in = NULL;
static uint8_t* algo_out_yuv = NULL;
static float* algo_out_float = NULL;

// 共享内存ID
static int shmid_yuv = -1;
static int shmid_float = -1;
static int semid = -1;

// 共享内存指针
static uint8_t* shm_yuv = NULL;
static float* shm_float = NULL;

// 复制输入数据
static int algo_copy()
{
    pthread_mutex_lock(&v4l2_ir_dvp_share_buffer_mutex);
    memcpy(algo_in, v4l2_ir_dvp_share_buffer, ALGO_WIDTH * ALGO_HEIGHT * sizeof(uint16_t));
    pthread_mutex_unlock(&v4l2_ir_dvp_share_buffer_mutex);
    return 0;
}

// 算法处理
static int algo_convert()
{
    // 转换为YUV420P
    uint8_t* y = algo_out_yuv;
    uint8_t* u = y + ALGO_WIDTH * ALGO_HEIGHT;
    uint8_t* v = u + (ALGO_WIDTH * ALGO_HEIGHT / 4);

    // 添加数据统计
    uint16_t min_val = 65535;
    uint16_t max_val = 0;

    // 第一遍扫描：找出数据范围
    for (int i = 0; i < ALGO_HEIGHT; i++)
    {
        for (int j = 0; j < ALGO_WIDTH; j++)
        {
            uint16_t val = algo_in[i * ALGO_WIDTH + j];
            if (val > max_val)
                max_val = val;
            if (val < min_val)
                min_val = val;
        }
    }

    // 计算映射系数
    float scale = 255.0f / (max_val - min_val);

    // 转换数据
    for (int i = 0; i < ALGO_HEIGHT; i++)
    {
        for (int j = 0; j < ALGO_WIDTH; j++)
        {
            uint16_t val = algo_in[i * ALGO_WIDTH + j];

            // 归一化到0-1范围（用于float输出）
            algo_out_float[i * ALGO_WIDTH + j] = (float)(val - min_val) / (max_val - min_val);

            // 线性映射到0-255范围（用于YUV输出）
            y[i * ALGO_WIDTH + j] = (uint8_t)((val - min_val) * scale);
        }
    }

    // 生成UV分量
    for (int i = 0; i < ALGO_HEIGHT / 2; i++)
    {
        for (int j = 0; j < ALGO_WIDTH / 2; j++)
        {
            u[i * (ALGO_WIDTH / 2) + j] = 128;
            v[i * (ALGO_WIDTH / 2) + j] = 128;
        }
    }

    return 0;
}

// 发送处理结果
static int algo_send()
{
    struct sembuf sem_op;

    // 等待信号量
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    if (semop(semid, &sem_op, 1) < 0)
    {
        perror("semop");
        return -1;
    }

    // 复制数据到共享内存
    memcpy(shm_yuv, algo_out_yuv, ALGO_YUV_SIZE);
    memcpy(shm_float, algo_out_float, ALGO_FLOAT_SIZE);

    // 释放信号量
    sem_op.sem_op = 1;
    if (semop(semid, &sem_op, 1) < 0)
    {
        perror("semop");
        return -1;
    }

    return 0;
}

int algo_init()
{
    // 分配内部缓冲区
    algo_in = (uint16_t*)malloc(ALGO_WIDTH * ALGO_HEIGHT * sizeof(uint16_t));
    algo_out_yuv = (uint8_t*)malloc(ALGO_YUV_SIZE);
    algo_out_float = (float*)malloc(ALGO_FLOAT_SIZE);

    if (!algo_in || !algo_out_yuv || !algo_out_float)
    {
        perror("malloc");
        return -1;
    }

    // 创建共享内存
    shmid_yuv = shmget(ALGO_SHM_YUV_KEY, ALGO_YUV_SIZE, IPC_CREAT | 0666);
    shmid_float = shmget(ALGO_SHM_FLOAT_KEY, ALGO_FLOAT_SIZE, IPC_CREAT | 0666);
    if (shmid_yuv < 0 || shmid_float < 0)
    {
        perror("shmget");
        return -1;
    }

    // 映射共享内存
    shm_yuv = (uint8_t*)shmat(shmid_yuv, NULL, 0);
    shm_float = (float*)shmat(shmid_float, NULL, 0);
    if (shm_yuv == (void*)-1 || shm_float == (void*)-1)
    {
        perror("shmat");
        return -1;
    }

    // 创建信号量
    semid = semget(ALGO_SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0)
    {
        perror("semget");
        return -1;
    }

    // 初始化信号量
    if (semctl(semid, 0, SETVAL, 1) < 0)
    {
        perror("semctl");
        return -1;
    }

    return 0;
}

int algo_process()
{
    if (algo_copy() < 0)
    {
        std::cout << "algo_copy fail" << std::endl;
        return -1;
    }
    if (algo_convert() < 0)
    {
        std::cout << "algo_convert fail" << std::endl;
        return -1;
    }
    if (algo_send() < 0)
    {
        std::cout << "algo_send fail" << std::endl;
        return -1;
    }
    return 0;
}

void algo_exit()
{
    // 释放内部缓冲区
    if (algo_in)
        free(algo_in);
    if (algo_out_yuv)
        free(algo_out_yuv);
    if (algo_out_float)
        free(algo_out_float);

    // 断开共享内存
    if (shm_yuv != (void*)-1)
        shmdt(shm_yuv);
    if (shm_float != (void*)-1)
        shmdt(shm_float);

    // 删除共享内存
    if (shmid_yuv >= 0)
        shmctl(shmid_yuv, IPC_RMID, NULL);
    if (shmid_float >= 0)
        shmctl(shmid_float, IPC_RMID, NULL);

    // 删除信号量
    if (semid >= 0)
        semctl(semid, 0, IPC_RMID);
}