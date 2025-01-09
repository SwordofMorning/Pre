#include "tm_cl.h"

TMCL tm_cl = {0};

bool TMCL_Init(TMCL* cl, int width, int height)
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
    const char* source = read_kernel_source("/root/app/cl/tm.cl");
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
    cl->kernel_tm = clCreateKernel(cl->program, "compute_temperature", &err);
    if (err != CL_SUCCESS)
        goto cleanup_program;

    // 创建缓冲区
    size_t input_size = width * height * sizeof(uint16_t);
    size_t output_size = width * height * sizeof(float);
    cl->d_input = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, input_size, NULL, &err);
    cl->d_output = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY, output_size, NULL, &err);

    cl->initialized = true;
    return true;

cleanup_program:
    clReleaseProgram(cl->program);
cleanup_queue:
    clReleaseCommandQueue(cl->queue);
cleanup_context:
    clReleaseContext(cl->context);
    return false;
}

int TMCL_Process(TMCL* cl, uint16_t* input, float* output, int width, int height, float a, float b, float c)
{
    if (!cl->initialized)
        return -1;

    cl_int err;
    size_t input_size = width * height * sizeof(uint16_t);
    size_t output_size = width * height * sizeof(float);

    // 写入输入数据
    err = clEnqueueWriteBuffer(cl->queue, cl->d_input, CL_FALSE, 0, input_size, input, 0, NULL, NULL);
    if (err != CL_SUCCESS)
        return -1;

    // 设置内核参数
    err = clSetKernelArg(cl->kernel_tm, 0, sizeof(cl_mem), &cl->d_input);
    err |= clSetKernelArg(cl->kernel_tm, 1, sizeof(cl_mem), &cl->d_output);
    err |= clSetKernelArg(cl->kernel_tm, 2, sizeof(float), &a);
    err |= clSetKernelArg(cl->kernel_tm, 3, sizeof(float), &b);
    err |= clSetKernelArg(cl->kernel_tm, 4, sizeof(float), &c);
    err |= clSetKernelArg(cl->kernel_tm, 5, sizeof(int), &width);
    err |= clSetKernelArg(cl->kernel_tm, 6, sizeof(int), &height);

    if (err != CL_SUCCESS)
        return -1;

    // 设置工作组大小
    size_t global_work_size[2] = {((width + 15) / 16) * 16, ((height + 15) / 16) * 16};
    size_t local_work_size[2] = {16, 16};

    // 执行内核
    err = clEnqueueNDRangeKernel(cl->queue, cl->kernel_tm, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    if (err != CL_SUCCESS)
        return -1;

    // 读回结果
    err = clEnqueueReadBuffer(cl->queue, cl->d_output, CL_TRUE, 0, output_size, output, 0, NULL, NULL);
    if (err != CL_SUCCESS)
        return -1;

    return 0;
}

void TMCL_Cleanup(TMCL* cl)
{
    if (!cl->initialized)
        return;

    if (cl->d_input)
        clReleaseMemObject(cl->d_input);
    if (cl->d_output)
        clReleaseMemObject(cl->d_output);
    if (cl->kernel_tm)
        clReleaseKernel(cl->kernel_tm);
    if (cl->program)
        clReleaseProgram(cl->program);
    if (cl->queue)
        clReleaseCommandQueue(cl->queue);
    if (cl->context)
        clReleaseContext(cl->context);

    cl->initialized = false;
}