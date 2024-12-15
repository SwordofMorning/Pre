#include "v4l2_csi.h"

static void CSI_Error(const char* error_str)
{
    litelog.log.fatal("%s", error_str);
    exit(EXIT_FAILURE);
}

static int CSI_Open_Device()
{
    litelog.log.trace("CSI_Open_Device() in");

    v4l2_vis_csi_fd = open(V4L2_VIS_CSI_DEVICE_NAME, O_RDWR /*| O_NONBLOCK*/, 0);
    if (v4l2_vis_csi_fd == 1)
        CSI_Error("CSI_Open_Device() open camera fail.");
    return v4l2_vis_csi_fd;
}

static int CSI_MMAP()
{
    litelog.log.trace("CSI_MMAP() Begin.");
    /* Step 1 : Request Buffer */
    struct v4l2_requestbuffers reqbuf;
    CLEAR(reqbuf);

    reqbuf.count = V4L2_VIS_CSI_REQ_COUNT;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_REQBUFS, &reqbuf))
    {
        if (errno == EINVAL)
            CSI_Error("CSI_MMAP() not support mmap.");
        else
            CSI_Error("CSI_MMAP() mmap other error.");
    }

    /* Step 2 : mmap Buffer */
    v4l2_vis_csi_buffer_global = calloc(reqbuf.count, sizeof(*v4l2_vis_csi_buffer_global));
    if (!v4l2_vis_csi_buffer_global)
        CSI_Error("CSI_MMAP() calloc error.");

    for (int i = 0; i < reqbuf.count; ++i)
    {
        struct v4l2_buffer buff;
        CLEAR(buff);

        buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buff.memory = V4L2_MEMORY_MMAP;
        buff.index = i;

        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        memset(planes, 0, VIDEO_MAX_PLANES * sizeof(struct v4l2_plane));
        /**
         * @note when type=V4L2_BUF_TYPE_VIDEO_CAPTURE, length means frame size
         * @note when type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, length means how many planes 
         */
        buff.length = v4l2_vis_csi_nplanes;
        buff.m.planes = planes;

        if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_QUERYBUF, &buff))
            CSI_Error("CSI_MMAP() VIDIOC_QUERYBUF fail.");

        /**
         * @note usr_buffer[0].start = frame addr, but not planes[0].m.mem_offset
         * 
         * @note user_buffer[0].start = 
         * 		planes[0].m.mem_offset + 
         * 		planes[1].m.mem_offset + ... 
         * 		planes[nplanes-1].m.mem_offset
         * 
         * @note in this case, nplanes == 1, so frame buffer have only one plane
         */
        // clang-format off
        v4l2_vis_csi_buffer_global[i].start = 
            (uint8_t*)mmap(NULL, buff.m.planes[0].length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            v4l2_vis_csi_fd,
            buff.m.planes[0].m.mem_offset);
        v4l2_vis_csi_buffer_global[i].length = buff.m.planes[0].length;
        v4l2_vis_csi_buffer_global_length = v4l2_vis_csi_buffer_global[i].length;
        
        litelog.log.trace("mmap buffer index[%d], length = %d, phyaddr = %p, viraddr = %p",
            buff.index, v4l2_vis_csi_buffer_global[i].length,
            buff.m.planes[0].m.mem_offset, v4l2_vis_csi_buffer_global[i].start);
        // clang-format on

        if (MAP_FAILED == v4l2_vis_csi_buffer_global[i].start)
            CSI_Error("CSI_MMAP() mmap fail.");
        if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_QBUF, &buff))
            CSI_Error("CSI_MMAP() VIDIOC_QBUF fail.");
    }
    return 0;
}

static int CSI_Init_Device()
{
    litelog.log.trace("CSI_Init_Device() Begin.");

    CSI_Open_Device();

    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    /* Step 1 : Check Capability */
    if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_QUERYCAP, &cap))
    {
        if (errno = EINVAL)
            CSI_Error("CSI_Init_Device() non V4l2 device.");
        else
            CSI_Error("CSI_Init_Device() VIDIOC_QUERYCAP.");
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
        CSI_Error("CSI_Init_Device() non capture device.");
    if (!(cap.capabilities & V4L2_CAP_STREAMING))
        CSI_Error("CSI_Init_Device() not support streaming.");

    /* Step 2 : Set FMT */
    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt.fmt.pix_mp.width = v4l2_vis_csi_width;
    fmt.fmt.pix_mp.height = v4l2_vis_csi_height;
    fmt.fmt.pix_mp.pixelformat = V4L2_VIS_CSI_PIX_FMT;

    if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_S_FMT, &fmt))
        CSI_Error("CSI_Init_Device() set fmt fail.");

    v4l2_vis_csi_nplanes = fmt.fmt.pix_mp.num_planes;
    litelog.log.trace("v4l2_vis_csi_nplanes = %d", v4l2_vis_csi_nplanes);

    /* Step 4 : Init MMAP */
    return CSI_MMAP();
}

static int CSI_Start_Capture()
{
    litelog.log.trace("CSI_Start_Capture() Begin.");

    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_STREAMON, &type))
        CSI_Error("CSI_Start_Capture() VIDIOC_STREAMON.");

    return 0;
}

static void CSI_Send()
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    pthread_mutex_lock(&frame_sync_csi.mutex);

    // 检查缓冲区是否已满
    while (frame_sync_csi.buffer_full)
    {
        pthread_cond_wait(&frame_sync_csi.producer_cond, &frame_sync_csi.mutex);
    }

    memcpy(frame_sync_csi.frame_buffer[frame_sync_csi.write_pos], (uint8_t*)v4l2_vis_csi_buffer_global[v4l2_vis_csi_buffer_global_index].start,
           v4l2_vis_csi_width * v4l2_vis_csi_height * sizeof(uint8_t) * 2);

    // Update write index
    frame_sync_csi.write_pos = (frame_sync_csi.write_pos + 1) % FRAME_SYNC_BUFFER_SIZE;
    frame_sync_csi.frame_count++;

    if (frame_sync_csi.write_pos == frame_sync_csi.read_pos)
    {
        frame_sync_csi.buffer_full = true;
    }

    pthread_cond_signal(&frame_sync_csi.consumer_cond);
    pthread_mutex_unlock(&frame_sync_csi.mutex);
}

static int CSI_Capture()
{
    litelog.log.trace("CSI_Capture() Begin.");

    struct v4l2_buffer buff;
    CLEAR(buff);
    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    memset(planes, 0, VIDEO_MAX_PLANES * sizeof(struct v4l2_plane));

    buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buff.memory = V4L2_MEMORY_MMAP;
    buff.length = v4l2_vis_csi_nplanes;
    buff.m.planes = planes;

#if (CSI_SAVE)
    FILE* fp = fopen("out.yuv", "a");
    litelog.log.trace("CSI Save Begin.");
#endif

    uint64_t index = 0;

    while (1)
    // for (int i = 0; i < 10; ++i)
    {
        if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_DQBUF, &buff))
        {
            if (errno == EAGAIN)
                CSI_Error("CSI_Capture() VIDIOC_DQBUF EAGAIN.");
            else if (errno == EINVAL)
                CSI_Error("CSI_Capture() VIDIOC_DQBUF EINVAL.");
            else if (errno == EIO)
                CSI_Error("CSI_Capture() VIDIOC_DQBUF EIO.");
            else if (errno == EPIPE)
                CSI_Error("CSI_Capture() VIDIOC_DQBUF EPIPE.");
        }

        CSI_Send();

#if (CSI_SAVE)
        CSI_Save(fp);
        printf("CSI save! %d\n", i);
#endif

        if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_QBUF, &buff))
            CSI_Error("CSI_Capture() fail to VIDIOC_QBUF.");
    }

#if (CSI_SAVE)
    fclose(fp);
    litelog.log.trace("CSI Save End.");
#endif

    return 0;
}

static int CSI_Stop_Capture()
{
    litelog.log.trace("CSI_Stop_Capture() Begin.");

    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    if (-1 == ioctl(v4l2_vis_csi_fd, VIDIOC_STREAMOFF, &type))
        CSI_Error("CSI_Stop_Capture() VIDIOC_STREAMOFF.");

    return 0;
}

static int CSI_Close_Device()
{
    litelog.log.trace("CSI_Close_Device() Begin.");

    if (-1 == close(v4l2_vis_csi_fd))
        CSI_Error("CSI_Close_Device() fail.");
    return 0;
}

static int CSI_MUNMAP()
{
    litelog.log.trace("CSI_MUNMAP() Begin.");

    for (int i = 0; i < V4L2_VIS_CSI_REQ_COUNT; ++i)
    {
        if (-1 == munmap(v4l2_vis_csi_buffer_global[i].start, v4l2_vis_csi_buffer_global->length))
            CSI_Error("CSI_MUNMAP() munmap fail.");
    }
}

static int CSI_Exit_Device()
{
    litelog.log.trace("CSI_Exit_Device() Begin.");

    CSI_MUNMAP();
    CSI_Close_Device();
}

int CSI_Streaming()
{
    litelog.log.trace("CSI_Streaming() Begin in mode: %d.", v4l2_vis_csi_mode);

    CSI_Init_Device();
    CSI_Start_Capture();

    CSI_Capture();

    CSI_Stop_Capture();
    CSI_Exit_Device();

    litelog.log.trace("CSI_Streaming() End.");

    return 0;
}