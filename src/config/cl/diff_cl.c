#include "diff_cl.h"

DiffCL diff_cl = {0};

bool DiffCL_Init(DiffCL* cl, int width, int height)
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;

    // 获取平台和设备
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS)
        return false;

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS)
        return false;

    // 创建上下文
    cl->context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS)
        return false;

    // 创建命令队列
    cl->queue = clCreateCommandQueue(cl->context, device, 0, &err);
    if (err != CL_SUCCESS)
        goto cleanup_context;

    // 读取并创建程序
    const char* source = read_kernel_source("/app/cl/diff.cl");
    if (!source)
        goto cleanup_queue;

    cl->program = clCreateProgramWithSource(cl->context, 1, &source, NULL, &err);
    free((void*)source);
    if (err != CL_SUCCESS)
        goto cleanup_queue;

    // 构建程序
    err = clBuildProgram(cl->program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
        goto cleanup_program;

    // 创建内核
    cl->kernel_diff = clCreateKernel(cl->program, "compute_diff", &err);
    if (err != CL_SUCCESS)
        goto cleanup_program;

    // 创建缓冲区
    size_t buffer_size = width * height * sizeof(uint16_t);
    cl->d_current = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    cl->d_last = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    cl->d_output = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY, buffer_size, NULL, &err);

    // 分配CPU端存储空间
    cl->last_frame = (uint16_t*)malloc(buffer_size);
    if (!cl->last_frame)
        goto cleanup_buffers;

    memset(cl->last_frame, 0, buffer_size);

    cl->initialized = true;
    return true;

cleanup_buffers:
    if (cl->d_current)
        clReleaseMemObject(cl->d_current);
    if (cl->d_last)
        clReleaseMemObject(cl->d_last);
    if (cl->d_output)
        clReleaseMemObject(cl->d_output);
cleanup_program:
    clReleaseProgram(cl->program);
cleanup_queue:
    clReleaseCommandQueue(cl->queue);
cleanup_context:
    clReleaseContext(cl->context);
    return false;
}

int DiffCL_Diff(DiffCL* cl, uint16_t* input, uint16_t* output, int width, int height, float rate)
{
    if (!cl->initialized)
        return -1;

    cl_int err;
    size_t buffer_size = width * height * sizeof(uint16_t);

    // 写入当前帧和上一帧数据
    err = clEnqueueWriteBuffer(cl->queue, cl->d_current, CL_FALSE, 0, buffer_size, input, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(cl->queue, cl->d_last, CL_FALSE, 0, buffer_size, cl->last_frame, 0, NULL, NULL);
    if (err != CL_SUCCESS)
        return -1;

    // 设置内核参数
    err = clSetKernelArg(cl->kernel_diff, 0, sizeof(cl_mem), &cl->d_current);
    err |= clSetKernelArg(cl->kernel_diff, 1, sizeof(cl_mem), &cl->d_last);
    err |= clSetKernelArg(cl->kernel_diff, 2, sizeof(cl_mem), &cl->d_output);
    err |= clSetKernelArg(cl->kernel_diff, 3, sizeof(float), &rate);
    err |= clSetKernelArg(cl->kernel_diff, 4, sizeof(int), &width);
    err |= clSetKernelArg(cl->kernel_diff, 5, sizeof(int), &height);

    if (err != CL_SUCCESS)
        return -1;

    // 设置工作组大小
    size_t global_work_size[2] = {((width + 15) / 16) * 16, ((height + 15) / 16) * 16};
    size_t local_work_size[2] = {16, 16};

    // 执行内核
    err = clEnqueueNDRangeKernel(cl->queue, cl->kernel_diff, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    if (err != CL_SUCCESS)
        return -1;

    // 读回结果
    err = clEnqueueReadBuffer(cl->queue, cl->d_output, CL_TRUE, 0, buffer_size, output, 0, NULL, NULL);
    if (err != CL_SUCCESS)
        return -1;

    // 更新last_frame
    memcpy(cl->last_frame, input, buffer_size);

    return 0;
}

void DiffCL_Cleanup(DiffCL* cl)
{
    if (!cl->initialized)
        return;

    if (cl->last_frame)
        free(cl->last_frame);
    if (cl->d_current)
        clReleaseMemObject(cl->d_current);
    if (cl->d_last)
        clReleaseMemObject(cl->d_last);
    if (cl->d_output)
        clReleaseMemObject(cl->d_output);
    if (cl->kernel_diff)
        clReleaseKernel(cl->kernel_diff);
    if (cl->program)
        clReleaseProgram(cl->program);
    if (cl->queue)
        clReleaseCommandQueue(cl->queue);
    if (cl->context)
        clReleaseContext(cl->context);

    cl->initialized = false;
}