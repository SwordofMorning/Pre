#include "shm_algo.h"

/**
 * @brief Send data from algo_out to SHM.
 * 
 * @return success or not.
 */
static int SHM_ALGO_Send()
{
    static uint8_t last_frame_sum = 0;
    uint8_t current_frame_sum = 0;

    // 计算当前帧的校验和
    for (int i = 0; i < 100; i++)
    {
        current_frame_sum += shm_out_yuv[i];
    }

    // 验证数据是否有更新
    if (current_frame_sum == last_frame_sum)
    {
        // printf("Duplicate frame detected in SHM_Send\n");
        return 0;
    }
    // printf("current sum: %d\n", current_frame_sum);
    last_frame_sum = current_frame_sum;

    struct sembuf sem_op;

    // Wait signal
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    if (semop(semid_ab, &sem_op, 1) < 0)
    {
        litelog.log.fatal("Wait signal error.");
        perror("semop");
        return -1;
    }

    {
        size_t y_size = v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height;
        memcpy(shm_algo, shm_out_yuv, y_size);

        uint8_t* uv_plane = shm_algo + y_size;
        memset(uv_plane, 128, y_size / 2);
    }

    // Release signal
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    if (semop(semid_ab, &sem_op, 1) < 0)
    {
        litelog.log.fatal("Release signal error.");
        perror("semop");
        return -1;
    }

    return 0;
}

int SHM_ALGO_Init()
{
    // Allocate internal buffer
    shm_out_algo = (uint8_t*)malloc(SHM_OUT_ALGO_SIZE);

    if (shm_out_algo)
    {
        litelog.log.fatal("Init SHM error.");
        perror("malloc");
        return -1;
    }

    // Create SHM buffer
    shmid_algo = shmget(ALGO_YUV_KEY, SHM_OUT_ALGO_SIZE, IPC_CREAT | 0666);
    if (shmid_algo < 0)
    {
        litelog.log.fatal("Create SHM error.");
        perror("shmget");
        return -1;
    }

    // Mapping shared memory
    shm_algo = (uint8_t*)shmat(shmid_algo, NULL, 0);
    if (shm_algo == (void*)-1)
    {
        litelog.log.fatal("shmat error.");
        perror("shmat");
        return -1;
    }

    // Create signal
    semid_ab = semget(ALGO_SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid_ab < 0)
    {
        litelog.log.fatal("semget error.");
        perror("semget");
        return -1;
    }

    // Init signal
    if (semctl(semid_ab, 0, SETVAL, 1) < 0)
    {
        litelog.log.fatal("semctl error.");
        perror("semctl");
        return -1;
    }

    printf("shm init\n");
    return 0;
}

int SHM_ALGO_Process()
{
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    if (SHM_ALGO_Send() < 0)
    {
        return -1;
    }

    gettimeofday(&end_time, NULL);

    // Calculate process time
    double process_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    // If the processing time is less than the frame interval, wait (control FPS)
    double frame_interval = 1.0 / IR_TARGET_FPS;
    if (process_time < frame_interval)
    {
        usleep((frame_interval - process_time) * 1000000);
    }

    return 0;
}

void SHM_ALGO_Exit()
{
    // Release internal buffer
    if (shm_out_algo)
        free(shm_out_algo);

    // Disconnect SHM
    if (shm_algo != (void*)-1)
        shmdt(shm_algo);

    // Delete SHM
    if (shmid_algo >= 0)
        shmctl(shmid_algo, IPC_RMID, NULL);

    // Delete signal
    if (semid_ab >= 0)
        semctl(semid_ab, 0, IPC_RMID);
}