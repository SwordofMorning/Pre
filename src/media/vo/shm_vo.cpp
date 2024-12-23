#include "shm_vo.h"

/**
 * @brief Copy data from frame_sync_dvp to algo_in;
 * 
 * @return success or not.
 */
static int SHM_VO_Copy()
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    pthread_mutex_lock(&frame_sync_dvp.mutex);

    // Wait new frame
    while (frame_sync_dvp.frame_count == 0)
    {
        pthread_cond_wait(&frame_sync_dvp.consumer_cond, &frame_sync_dvp.mutex);
    }

    // Calculate time interval between frames
    double frame_interval = (current_time.tv_sec - frame_sync_dvp.last_frame_time.tv_sec) + (current_time.tv_usec - frame_sync_dvp.last_frame_time.tv_usec) / 1000000.0;

    // If processing is too slow, skip some frames
    if (frame_interval > 1.0 / IR_TARGET_FPS)
    {
        // Skip the intermediate frames and process only the latest ones
        while (frame_sync_dvp.frame_count > 1)
        {
            frame_sync_dvp.read_pos = (frame_sync_dvp.read_pos + 1) % FRAME_SYNC_BUFFER_SIZE;
            frame_sync_dvp.frame_count--;
            litelog.log.warning("Dropping frame due to processing delay.");
        }
    }

    // Copy Data
    memcpy(algo_in, frame_sync_dvp.frame_buffer[frame_sync_dvp.read_pos], v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(uint16_t));

    // Update frame index
    frame_sync_dvp.read_pos = (frame_sync_dvp.read_pos + 1) % FRAME_SYNC_BUFFER_SIZE;
    frame_sync_dvp.frame_count--;
    frame_sync_dvp.buffer_full = false;

    // Update timestamp
    frame_sync_dvp.last_frame_time = current_time;

    pthread_cond_signal(&frame_sync_dvp.producer_cond);
    pthread_mutex_unlock(&frame_sync_dvp.mutex);

    return 0;
}

/**
 * @brief Send data from algo_out to SHM.
 * 
 * @return success or not.
 */
static int SHM_VO_Send()
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
    if (semop(semid_vo, &sem_op, 1) < 0)
    {
        litelog.log.fatal("Wait signal error.");
        perror("semop");
        return -1;
    }

    // Copy data to shm with verification
    memcpy(shm_yuv, shm_out_yuv, SHM_OUT_YUV_SIZE);
    memcpy(shm_float, shm_out_float, SHM_OUT_FLOAT_SIZE);

    // Copy CSI data
    pthread_mutex_lock(&frame_sync_csi.mutex);
    if (frame_sync_csi.frame_count > 0)
    {
        memcpy(shm_vis, frame_sync_csi.frame_buffer[frame_sync_csi.read_pos], SHM_OUT_CSI_SIZE);

        frame_sync_csi.read_pos = (frame_sync_csi.read_pos + 1) % FRAME_SYNC_BUFFER_SIZE;
        frame_sync_csi.frame_count--;
        frame_sync_csi.buffer_full = false;

        pthread_cond_signal(&frame_sync_csi.producer_cond);
    }
    pthread_mutex_unlock(&frame_sync_csi.mutex);

    // Release signal
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    if (semop(semid_vo, &sem_op, 1) < 0)
    {
        litelog.log.fatal("Release signal error.");
        perror("semop");
        return -1;
    }

    return 0;
}

int SHM_VO_Init()
{
    // Allocate internal buffer
    algo_in = (uint16_t*)malloc(v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(uint16_t));
    shm_out_yuv = (uint8_t*)malloc(SHM_OUT_YUV_SIZE);
    shm_out_float = (float*)malloc(SHM_OUT_FLOAT_SIZE);

    if (!algo_in || !shm_out_yuv || !shm_out_float)
    {
        litelog.log.fatal("Init SHM error.");
        perror("malloc");
        return -1;
    }

    // Create SHM buffer
    shmid_yuv = shmget(VO_YUV_KEY, SHM_OUT_YUV_SIZE, IPC_CREAT | 0666);
    shmid_float = shmget(VO_FLOAT_KEY, SHM_OUT_FLOAT_SIZE, IPC_CREAT | 0666);
    shmid_csi = shmget(VO_CSI_KEY, SHM_OUT_CSI_SIZE, IPC_CREAT | 0666);
    if (shmid_yuv < 0 || shmid_float < 0 || shmid_csi < 0)
    {
        litelog.log.fatal("VO Create SHM error.");
        perror("shmget");
        return -1;
    }

    // Mapping shared memory
    shm_yuv = (uint8_t*)shmat(shmid_yuv, NULL, 0);
    shm_float = (float*)shmat(shmid_float, NULL, 0);
    shm_vis = (uint8_t*)shmat(shmid_csi, NULL, 0);
    if (shm_yuv == (void*)-1 || shm_float == (void*)-1 || shm_vis == (void*)-1)
    {
        litelog.log.fatal("shmat error.");
        perror("shmat");
        return -1;
    }

    // Create signal
    semid_vo = semget(VO_SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid_vo < 0)
    {
        litelog.log.fatal("semget error.");
        perror("semget");
        return -1;
    }

    // Init signal
    if (semctl(semid_vo, 0, SETVAL, 1) < 0)
    {
        litelog.log.fatal("semctl error.");
        perror("semctl");
        return -1;
    }

    printf("shm init\n");
    return 0;
}

int SHM_VO_Process()
{
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    if (SHM_VO_Copy() < 0)
    {
        return -1;
    }

    if (Process_One_Frame() < 0)
    {
        return -1;
    }

    if (SHM_VO_Send() < 0)
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

void SHM_VO_Exit()
{
    // Release internal buffer
    if (algo_in)
        free(algo_in);
    if (shm_out_yuv)
        free(shm_out_yuv);
    if (shm_out_float)
        free(shm_out_float);

    // Disconnect SHM
    if (shm_yuv != (void*)-1)
        shmdt(shm_yuv);
    if (shm_float != (void*)-1)
        shmdt(shm_float);
    if (shm_vis != (void*)-1)
        shmdt(shm_vis);

    // Delete SHM
    if (shmid_yuv >= 0)
        shmctl(shmid_yuv, IPC_RMID, NULL);
    if (shmid_float >= 0)
        shmctl(shmid_float, IPC_RMID, NULL);
    if (shmid_csi >= 0)
        shmctl(shmid_csi, IPC_RMID, NULL);

    // Delete signal
    if (semid_vo >= 0)
        semctl(semid_vo, 0, IPC_RMID);
}