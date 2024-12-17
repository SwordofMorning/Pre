#include "vo_gst.h"

// GStreamer全局变量
static GstElement* pipeline = NULL;
static GstElement* appsrc = NULL;
static GstElement* videoconvert = NULL;
static GstElement* waylandsink = NULL;
static GMainLoop* main_loop = NULL;

// 初始化GStreamer
int GST_Init(void)
{
    gst_init(NULL, NULL);
    main_loop = g_main_loop_new(NULL, FALSE);
    if (!main_loop)
    {
        g_print("Failed to create main loop\n");
        return -1;
    }
    return 0;
}

// 释放GStreamer资源
void GST_Deinit(void)
{
    if (pipeline)
    {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = NULL;
    }
    if (main_loop)
    {
        g_main_loop_quit(main_loop);
        g_main_loop_unref(main_loop);
        main_loop = NULL;
    }
}

// 创建GStreamer Pipeline
int GST_Create_Pipeline(int in_width, int in_height, int out_width, int out_height)
{
    // 创建pipeline
    pipeline = gst_pipeline_new("pipeline");
    if (!pipeline)
    {
        g_print("Failed to create pipeline\n");
        return -1;
    }

    // 创建元素
    appsrc = gst_element_factory_make("appsrc", "source");
    GstElement* videoflip = gst_element_factory_make("videoflip", "flip");
    GstElement* videoscale = gst_element_factory_make("videoscale", "scale");
    GstElement* capsfilter = gst_element_factory_make("capsfilter", "filter");
    videoconvert = gst_element_factory_make("videoconvert", "convert");
    waylandsink = gst_element_factory_make("waylandsink", "sink");

    if (!appsrc || !videoflip || !videoscale || !capsfilter || !videoconvert || !waylandsink)
    {
        g_print("Failed to create elements\n");
        return -1;
    }

    // 设置videoflip属性 (-90度旋转) 3 = counterclockwise
    g_object_set(G_OBJECT(videoflip), "method", 3, NULL);

    // 设置appsrc输入格式
    GstCaps* src_caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, in_width, "height", G_TYPE_INT, in_height, "framerate",
                                            GST_TYPE_FRACTION, 30, 1, NULL);
    g_object_set(G_OBJECT(appsrc), "caps", src_caps, "format", GST_FORMAT_TIME, "stream-type", 0, "is-live", TRUE, NULL);
    gst_caps_unref(src_caps);

    // 设置输出分辨率，注意交换宽高
    GstCaps* scale_caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, out_width, "height", G_TYPE_INT, out_height,
                                              "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, "colorimetry", G_TYPE_STRING, "bt709", NULL);
    g_object_set(G_OBJECT(capsfilter), "caps", scale_caps, NULL);
    gst_caps_unref(scale_caps);

    // 设置videoscale属性
    g_object_set(G_OBJECT(videoscale), "method", 1, "add-borders", FALSE, NULL);

    // 设置waylandsink属性
    g_object_set(G_OBJECT(waylandsink), "fullscreen", TRUE, "sync", FALSE, NULL);

    // 添加元素到pipeline
    gst_bin_add_many(GST_BIN(pipeline), appsrc, videoflip, videoscale, capsfilter, videoconvert, waylandsink, NULL);

    // 连接元素
    if (!gst_element_link_many(appsrc, videoflip, videoscale, capsfilter, videoconvert, waylandsink, NULL))
    {
        g_print("Failed to link elements\n");
        return -1;
    }

    // 设置pipeline状态
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_print("Failed to set pipeline to playing state\n");
        return -1;
    }

    return 0;
}

// 推送帧数据到GStreamer
int GST_Push_Frame(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, int width, int height)
{
    if (!appsrc)
        return -1;

    // 计算buffer大小
    int y_size = width * height;
    int uv_size = y_size / 4;
    int total_size = y_size + 2 * uv_size;

    // 创建buffer
    GstBuffer* buffer = gst_buffer_new_allocate(NULL, total_size, NULL);
    if (!buffer)
    {
        g_print("Failed to allocate buffer\n");
        return -1;
    }

    // 填充数据
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);

    // 复制Y平面数据
    memcpy(map.data, y_data, y_size);
    // 复制U平面数据
    memcpy(map.data + y_size, u_data, uv_size);
    // 复制V平面数据
    memcpy(map.data + y_size + uv_size, v_data, uv_size);

    gst_buffer_unmap(buffer, &map);

    // 推送buffer
    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer);
    if (ret != GST_FLOW_OK)
    {
        g_print("Failed to push buffer\n");
        return -1;
    }

    return 0;
}

// 主循环函数
void* VO_GST_Streaming()
{
    // 初始化GStreamer
    if (GST_Init() < 0)
    {
        g_print("Failed to initialize GStreamer\n");
        return NULL;
    }

    // 创建pipeline，指定输入和输出分辨率
    if (GST_Create_Pipeline(640, 512, 1920, 1080) < 0)
    {
        g_print("Failed to create pipeline\n");
        GST_Deinit();
        return NULL;
    }

    // 主循环
    g_main_loop_run(main_loop);

    // 清理资源
    GST_Deinit();
    return NULL;
}