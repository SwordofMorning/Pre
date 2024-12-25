// pseudo_cl.c
#include "pseudo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PseudoCL cl_processor = {0};

static char* read_kernel_source(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        printf("Failed to open kernel file\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source = (char*)malloc(size + 1);
    if (!source)
    {
        fclose(file);
        return NULL;
    }

    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);
    return source;
}

bool PseudoCL_Init(PseudoCL* cl, int width, int height)
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;

    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to get platform\n");
        return false;
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to get device\n");
        return false;
    }

    // Context
    cl->context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Failed to create context\n");
        return false;
    }

    // Command queue
    cl->queue = clCreateCommandQueue(cl->context, device, 0, &err);
    if (err != CL_SUCCESS)
    {
        printf("Failed to create command queue\n");
        goto cleanup_context;
    }

    // Read then create program
    char* source = read_kernel_source("/root/app/pseudo/pseudo.cl");
    if (!source)
        goto cleanup_queue;

    cl->program = clCreateProgramWithSource(cl->context, 1, (const char**)&source, NULL, &err);
    free(source);
    if (err != CL_SUCCESS)
    {
        printf("Failed to create program\n");
        goto cleanup_queue;
    }

    // Build program
    err = clBuildProgram(cl->program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t log_size;
        clGetProgramBuildInfo(cl->program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char* log = (char*)malloc(log_size);
        clGetProgramBuildInfo(cl->program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Build error: %s\n", log);
        free(log);
        goto cleanup_program;
    }

    // Kernel
    cl->kernel_black_hot = clCreateKernel(cl->program, "black_hot_nv12", &err);
    cl->kernel_white_hot = clCreateKernel(cl->program, "white_hot_nv12", &err);
    cl->kernel_color_map = clCreateKernel(cl->program, "color_map_nv12", &err);

    if (err != CL_SUCCESS)
    {
        printf("Failed to create kernels\n");
        goto cleanup_program;
    }

    // Buffer
    size_t input_size = width * height * sizeof(uint16_t);
    size_t y_size = width * height;
    size_t uv_size = y_size / 2;

    cl->d_input = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, input_size, NULL, &err);
    cl->d_y_out = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY, y_size, NULL, &err);
    cl->d_uv_out = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY, uv_size, NULL, &err);

    if (err != CL_SUCCESS)
    {
        printf("Failed to create buffers\n");
        goto cleanup_kernels;
    }

    cl->initialized = true;
    return true;

cleanup_kernels:
    if (cl->kernel_black_hot)
        clReleaseKernel(cl->kernel_black_hot);
    if (cl->kernel_white_hot)
        clReleaseKernel(cl->kernel_white_hot);
    if (cl->kernel_color_map)
        clReleaseKernel(cl->kernel_color_map);
cleanup_program:
    clReleaseProgram(cl->program);
cleanup_queue:
    clReleaseCommandQueue(cl->queue);
cleanup_context:
    clReleaseContext(cl->context);

    cl->initialized = false;
    return false;
}

void PseudoCL_Cleanup(PseudoCL* cl)
{
    if (!cl->initialized)
        return;

    if (cl->d_input)
        clReleaseMemObject(cl->d_input);
    if (cl->d_y_out)
        clReleaseMemObject(cl->d_y_out);
    if (cl->d_uv_out)
        clReleaseMemObject(cl->d_uv_out);
    if (cl->d_lut_y)
        clReleaseMemObject(cl->d_lut_y);
    if (cl->d_lut_u)
        clReleaseMemObject(cl->d_lut_u);
    if (cl->d_lut_v)
        clReleaseMemObject(cl->d_lut_v);

    if (cl->kernel_black_hot)
        clReleaseKernel(cl->kernel_black_hot);
    if (cl->kernel_white_hot)
        clReleaseKernel(cl->kernel_white_hot);
    if (cl->kernel_color_map)
        clReleaseKernel(cl->kernel_color_map);

    clReleaseProgram(cl->program);
    clReleaseCommandQueue(cl->queue);
    clReleaseContext(cl->context);

    cl->initialized = false;
}

int PseudoCL_ProcessNV12(PseudoCL* cl,
                         uint16_t* input,
                         uint8_t* y_out,
                         uint8_t* uv_out,
                         int width,
                         int height,
                         int pseudo_type,
                         const struct YUV420P_LUT* lut,
                         float scale,
                         float min_val)
{
    if (!cl->initialized)
        return -1;

    cl_int err;

    // 异步写入输入数据
    err = clEnqueueWriteBuffer(cl->queue, cl->d_input, CL_FALSE, 0,
                              width * height * sizeof(uint16_t),
                              input, 0, NULL, &cl->write_event);
    if (err != CL_SUCCESS) {
        printf("Failed to write input data: %d\n", err);
        return -1;
    }

    // 选择内核
    cl_kernel active_kernel = NULL;
    switch(pseudo_type) {
        case PSEUDO_BLACK_HOT:
            active_kernel = cl->kernel_black_hot;
            break;
        case PSEUDO_WHITE_HOT:
            active_kernel = cl->kernel_white_hot;
            break;
        default:
            if(lut) {
                active_kernel = cl->kernel_color_map;
                
                // 只在LUT类型改变时更新LUT数据
                if (!cl->persistent_lut.initialized || 
                    cl->persistent_lut.current_type != pseudo_type) {
                    
                    // 首次创建或重新创建LUT缓冲区
                    if (!cl->persistent_lut.initialized) {
                        size_t lut_size = lut->size * sizeof(uint8_t);
                        cl->persistent_lut.y = clCreateBuffer(cl->context, 
                            CL_MEM_READ_ONLY, lut_size, NULL, &err);
                        cl->persistent_lut.u = clCreateBuffer(cl->context, 
                            CL_MEM_READ_ONLY, lut_size, NULL, &err);
                        cl->persistent_lut.v = clCreateBuffer(cl->context, 
                            CL_MEM_READ_ONLY, lut_size, NULL, &err);
                        cl->persistent_lut.initialized = true;
                    }
                    
                    // 异步更新LUT数据
                    clEnqueueWriteBuffer(cl->queue, cl->persistent_lut.y, 
                        CL_FALSE, 0, lut->size, lut->y, 0, NULL, NULL);
                    clEnqueueWriteBuffer(cl->queue, cl->persistent_lut.u, 
                        CL_FALSE, 0, lut->size, lut->u, 0, NULL, NULL);
                    clEnqueueWriteBuffer(cl->queue, cl->persistent_lut.v, 
                        CL_FALSE, 0, lut->size, lut->v, 0, NULL, NULL);
                    
                    cl->persistent_lut.current_type = pseudo_type;
                }
            }
            break;
    }

    if (!active_kernel) {
        printf("Invalid kernel selection\n");
        return -1;
    }

    // 设置内核参数
    err = clSetKernelArg(active_kernel, 0, sizeof(cl_mem), &cl->d_input);
    err |= clSetKernelArg(active_kernel, 1, sizeof(cl_mem), &cl->d_y_out);
    err |= clSetKernelArg(active_kernel, 2, sizeof(cl_mem), &cl->d_uv_out);
    err |= clSetKernelArg(active_kernel, 3, sizeof(float), &scale);
    err |= clSetKernelArg(active_kernel, 4, sizeof(float), &min_val);

    if (active_kernel == cl->kernel_color_map) {
        err |= clSetKernelArg(active_kernel, 5, sizeof(cl_mem), &cl->persistent_lut.y);
        err |= clSetKernelArg(active_kernel, 6, sizeof(cl_mem), &cl->persistent_lut.u);
        err |= clSetKernelArg(active_kernel, 7, sizeof(cl_mem), &cl->persistent_lut.v);
        err |= clSetKernelArg(active_kernel, 8, sizeof(int), &lut->size);
        err |= clSetKernelArg(active_kernel, 9, sizeof(int), &width);
        err |= clSetKernelArg(active_kernel, 10, sizeof(int), &height);
    } else {
        err |= clSetKernelArg(active_kernel, 5, sizeof(int), &width);
        err |= clSetKernelArg(active_kernel, 6, sizeof(int), &height);
    }

    // 优化工作组大小
    size_t global_work_size[2] = {((width + 31) / 32) * 32, ((height + 31) / 32) * 32};
    size_t local_work_size[2] = {32, 8}; // 调整为更适合GPU的大小

    // 异步执行内核
    err = clEnqueueNDRangeKernel(cl->queue, active_kernel, 2, NULL,
                                global_work_size, local_work_size,
                                1, &cl->write_event, &cl->kernel_event);

    // 异步读回结果
    err = clEnqueueReadBuffer(cl->queue, cl->d_y_out, CL_FALSE, 0,
                            width * height, y_out,
                            1, &cl->kernel_event, &cl->read_events[0]);
    err |= clEnqueueReadBuffer(cl->queue, cl->d_uv_out, CL_FALSE, 0,
                             width * height / 2, uv_out,
                             1, &cl->kernel_event, &cl->read_events[1]);

    // 等待读取完成
    clWaitForEvents(2, cl->read_events);

    // 释放事件对象
    clReleaseEvent(cl->write_event);
    clReleaseEvent(cl->kernel_event);
    clReleaseEvent(cl->read_events[0]);
    clReleaseEvent(cl->read_events[1]);

    return (err == CL_SUCCESS) ? 0 : -1;
}