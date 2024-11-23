#include "vo_shm.h"

/**
 * @brief Copy data from frame_sync to algo_in;
 * 
 * @return success or not.
 */
static int SHM_Copy()
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    pthread_mutex_lock(&frame_sync.mutex);

    // Wait new frame
    while (frame_sync.frame_count == 0)
    {
        pthread_cond_wait(&frame_sync.consumer_cond, &frame_sync.mutex);
    }

    // Calculate time interval between frames
    double frame_interval = (current_time.tv_sec - frame_sync.last_frame_time.tv_sec) + (current_time.tv_usec - frame_sync.last_frame_time.tv_usec) / 1000000.0;

    // If processing is too slow, skip some frames
    if (frame_interval > 1.0 / IR_TARGET_FPS)
    {
        // Skip the intermediate frames and process only the latest ones
        while (frame_sync.frame_count > 1)
        {
            frame_sync.read_pos = (frame_sync.read_pos + 1) % SHM_FRAME_BUFFER_SIZE;
            frame_sync.frame_count--;
            litelog.log.warning("Dropping frame due to processing delay.");
        }
    }

    // Copy Data
    memcpy(algo_in, frame_sync.frame_buffer[frame_sync.read_pos], v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(uint16_t));

    // Update frame index
    frame_sync.read_pos = (frame_sync.read_pos + 1) % SHM_FRAME_BUFFER_SIZE;
    frame_sync.frame_count--;
    frame_sync.buffer_full = false;

    // Update timestamp
    frame_sync.last_frame_time = current_time;

    pthread_cond_signal(&frame_sync.producer_cond);
    pthread_mutex_unlock(&frame_sync.mutex);

    return 0;
}

/**
 * @brief Send data from algo_out to SHM.
 * 
 * @return success or not.
 */
static int SHM_Send()
{
    static uint8_t last_frame_sum = 0;
    uint8_t current_frame_sum = 0;
    
    // 计算当前帧的校验和
    for(int i = 0; i < 100; i++) {
        current_frame_sum += algo_out_yuv[i];
    }
    
    // 验证数据是否有更新
    if(current_frame_sum == last_frame_sum) {
        printf("Duplicate frame detected in SHM_Send\n");
        return 0;
    }
    printf("current sum: %d\n", current_frame_sum);
    last_frame_sum = current_frame_sum;

    struct sembuf sem_op;

    // Wait signal
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    if (semop(semid, &sem_op, 1) < 0)
    {
        litelog.log.fatal("Wait signal error.");
        perror("semop");
        return -1;
    }

    // Copy data to shm with verification
    memcpy(shm_yuv, algo_out_yuv, SHM_OUT_YUV_SIZE);
    memcpy(shm_float, algo_out_float, SHM_OUT_FLOAT_SIZE);

    // Release signal
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    if (semop(semid, &sem_op, 1) < 0)
    {
        litelog.log.fatal("Release signal error.");
        perror("semop");
        return -1;
    }

    return 0;
}

int SHM_Init()
{
    // Allocate internal buffer
    algo_in = (uint16_t*)malloc(v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(uint16_t));
    algo_out_yuv = (uint8_t*)malloc(SHM_OUT_YUV_SIZE);
    algo_out_float = (float*)malloc(SHM_OUT_FLOAT_SIZE);

    if (!algo_in || !algo_out_yuv || !algo_out_float)
    {
        litelog.log.fatal("Init SHM error.");
        perror("malloc");
        return -1;
    }

    // Create SHM buffer
    shmid_yuv = shmget(ALGO_SHM_YUV_KEY, SHM_OUT_YUV_SIZE, IPC_CREAT | 0666);
    shmid_float = shmget(ALGO_SHM_FLOAT_KEY, SHM_OUT_FLOAT_SIZE, IPC_CREAT | 0666);
    if (shmid_yuv < 0 || shmid_float < 0)
    {
        litelog.log.fatal("Create SHM error.");
        perror("shmget");
        return -1;
    }

    // Mapping shared memory
    shm_yuv = (uint8_t*)shmat(shmid_yuv, NULL, 0);
    shm_float = (float*)shmat(shmid_float, NULL, 0);
    if (shm_yuv == (void*)-1 || shm_float == (void*)-1)
    {
        litelog.log.fatal("shmat error.");
        perror("shmat");
        return -1;
    }

    // Create signal
    semid = semget(ALGO_SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0)
    {
        litelog.log.fatal("semget error.");
        perror("semget");
        return -1;
    }

    // Init signal
    if (semctl(semid, 0, SETVAL, 1) < 0)
    {
        litelog.log.fatal("semctl error.");
        perror("semctl");
        return -1;
    }

    printf("shm init\n");
    return 0;
}

int SHM_Process()
{
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    if (SHM_Copy() < 0)
    {
        return -1;
    }

    if (Process_One_Frame() < 0)
    {
        return -1;
    }

    if (SHM_Send() < 0)
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

void SHM_Exit()
{
    // Release internal buffer
    if (algo_in)
        free(algo_in);
    if (algo_out_yuv)
        free(algo_out_yuv);
    if (algo_out_float)
        free(algo_out_float);

    // Disconnect SHM
    if (shm_yuv != (void*)-1)
        shmdt(shm_yuv);
    if (shm_float != (void*)-1)
        shmdt(shm_float);

    // Delete SHM
    if (shmid_yuv >= 0)
        shmctl(shmid_yuv, IPC_RMID, NULL);
    if (shmid_float >= 0)
        shmctl(shmid_float, IPC_RMID, NULL);

    // Delete signal
    if (semid >= 0)
        semctl(semid, 0, IPC_RMID);
}