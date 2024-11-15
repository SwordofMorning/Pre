#include "vo_shm.h"

// 内部缓冲区
uint16_t* algo_in = NULL;
uint8_t* algo_out_yuv = NULL;
float* algo_out_float = NULL;

// 共享内存ID
int shmid_yuv = -1;
int shmid_float = -1;
int semid = -1;

// 共享内存指针
uint8_t* shm_yuv = NULL;
float* shm_float = NULL;

// 复制输入数据
static int SHM_Copy()
{
    pthread_mutex_lock(&v4l2_ir_dvp_share_buffer_mutex);
    memcpy(algo_in, v4l2_ir_dvp_share_buffer, v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(uint16_t));
    pthread_mutex_unlock(&v4l2_ir_dvp_share_buffer_mutex);
    return 0;
}

// 发送处理结果
static int SHM_Send()
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
    memcpy(shm_yuv, algo_out_yuv, SHM_OUT_YUV_SIZE);
    memcpy(shm_float, algo_out_float, SHM_OUT_FLOAT_SIZE);

    // 释放信号量
    sem_op.sem_op = 1;
    if (semop(semid, &sem_op, 1) < 0)
    {
        perror("semop");
        return -1;
    }

    return 0;
}

int SHM_Init()
{
    // 分配内部缓冲区
    algo_in = (uint16_t*)malloc(v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(uint16_t));
    algo_out_yuv = (uint8_t*)malloc(SHM_OUT_YUV_SIZE);
    algo_out_float = (float*)malloc(SHM_OUT_FLOAT_SIZE);

    if (!algo_in || !algo_out_yuv || !algo_out_float)
    {
        perror("malloc");
        return -1;
    }

    // 创建共享内存
    shmid_yuv = shmget(ALGO_SHM_YUV_KEY, SHM_OUT_YUV_SIZE, IPC_CREAT | 0666);
    shmid_float = shmget(ALGO_SHM_FLOAT_KEY, SHM_OUT_FLOAT_SIZE, IPC_CREAT | 0666);
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

int SHM_Process()
{
    if (SHM_Copy() < 0)
    {
        std::cout << "SHM_Copy fail" << std::endl;
        return -1;
    }
    if (algo_convert() < 0)
    {
        std::cout << "algo_convert fail" << std::endl;
        return -1;
    }
    if (SHM_Send() < 0)
    {
        std::cout << "SHM_Send fail" << std::endl;
        return -1;
    }
    return 0;
}

void SHM_Exit()
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