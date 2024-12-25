// pseudo_cl.c
#include "pseudo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 读取内核源代码
static char* read_kernel_source(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open kernel file\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source = (char*)malloc(size + 1);
    if (!source) {
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
    
    // 获取平台
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS) {
        printf("Failed to get platform\n");
        return false;
    }

    // 获取设备
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS) {
        printf("Failed to get device\n");
        return false;
    }

    // 创建上下文
    cl->context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        printf("Failed to create context\n");
        return false;
    }

    // 创建命令队列
    cl->queue = clCreateCommandQueue(cl->context, device, 0, &err);
    if (err != CL_SUCCESS) {
        printf("Failed to create command queue\n");
        goto cleanup_context;
    }

    // 读取并创建程序
    char* source = read_kernel_source("/root/app/pseudo/pseudo.cl");
    if (!source) {
        goto cleanup_queue;
    }

    cl->program = clCreateProgramWithSource(cl->context, 1, 
                                          (const char**)&source, NULL, &err);
    free(source);
    if (err != CL_SUCCESS) {
        printf("Failed to create program\n");
        goto cleanup_queue;
    }

    // 编译程序
    err = clBuildProgram(cl->program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(cl->program, device, CL_PROGRAM_BUILD_LOG, 
                            0, NULL, &log_size);
        char* log = (char*)malloc(log_size);
        clGetProgramBuildInfo(cl->program, device, CL_PROGRAM_BUILD_LOG, 
                            log_size, log, NULL);
        printf("Build error: %s\n", log);
        free(log);
        goto cleanup_program;
    }

    // 创建内核
    cl->kernel_black_hot = clCreateKernel(cl->program, "black_hot_nv12", &err);
    cl->kernel_white_hot = clCreateKernel(cl->program, "white_hot_nv12", &err);
    cl->kernel_color_map = clCreateKernel(cl->program, "color_map_nv12", &err);
    
    if (err != CL_SUCCESS) {
        printf("Failed to create kernels\n");
        goto cleanup_program;
    }

    // 创建缓冲区
    size_t input_size = width * height * sizeof(uint16_t);
    size_t y_size = width * height;
    size_t uv_size = y_size / 2;

    cl->d_input = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, 
                                input_size, NULL, &err);
    cl->d_y_out = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY, 
                                y_size, NULL, &err);
    cl->d_uv_out = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY, 
                                 uv_size, NULL, &err);

    if (err != CL_SUCCESS) {
        printf("Failed to create buffers\n");
        goto cleanup_kernels;
    }

    cl->initialized = true;
    return true;

cleanup_kernels:
    if (cl->kernel_black_hot) clReleaseKernel(cl->kernel_black_hot);
    if (cl->kernel_white_hot) clReleaseKernel(cl->kernel_white_hot);
    if (cl->kernel_color_map) clReleaseKernel(cl->kernel_color_map);
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

    if (cl->d_input) clReleaseMemObject(cl->d_input);
    if (cl->d_y_out) clReleaseMemObject(cl->d_y_out);
    if (cl->d_uv_out) clReleaseMemObject(cl->d_uv_out);
    if (cl->d_lut_y) clReleaseMemObject(cl->d_lut_y);
    if (cl->d_lut_u) clReleaseMemObject(cl->d_lut_u);
    if (cl->d_lut_v) clReleaseMemObject(cl->d_lut_v);
    
    if (cl->kernel_black_hot) clReleaseKernel(cl->kernel_black_hot);
    if (cl->kernel_white_hot) clReleaseKernel(cl->kernel_white_hot);
    if (cl->kernel_color_map) clReleaseKernel(cl->kernel_color_map);
    
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
    
    // 写入输入数据
    err = clEnqueueWriteBuffer(cl->queue, cl->d_input, CL_TRUE, 0,
                              width * height * sizeof(uint16_t),
                              input, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Failed to write input data: %d\n", err);
        return -1;
    }

    // 选择并设置内核
    cl_kernel active_kernel = NULL;
    switch(pseudo_type) {
        case 8: // PSEUDO_BLACK_HOT
            active_kernel = cl->kernel_black_hot;
            break;
        case 9: // PSEUDO_WHITE_HOT
            active_kernel = cl->kernel_white_hot;
            break;
        default:
            if(lut) {
                active_kernel = cl->kernel_color_map;
                
                // 创建和上传LUT数据，注意检查大小
                size_t lut_size = lut->size * sizeof(uint8_t);
                
                // 创建LUT缓冲区
                cl->d_lut_y = clCreateBuffer(cl->context, CL_MEM_READ_ONLY,
                                           lut_size, NULL, &err);
                if (err != CL_SUCCESS) {
                    printf("Failed to create LUT Y buffer: %d\n", err);
                    return -1;
                }

                cl->d_lut_u = clCreateBuffer(cl->context, CL_MEM_READ_ONLY,
                                           lut_size, NULL, &err);
                if (err != CL_SUCCESS) {
                    printf("Failed to create LUT U buffer: %d\n", err);
                    goto cleanup_lut_y;
                }

                cl->d_lut_v = clCreateBuffer(cl->context, CL_MEM_READ_ONLY,
                                           lut_size, NULL, &err);
                if (err != CL_SUCCESS) {
                    printf("Failed to create LUT V buffer: %d\n", err);
                    goto cleanup_lut_u;
                }

                // 写入LUT数据
                err = clEnqueueWriteBuffer(cl->queue, cl->d_lut_y, CL_TRUE, 0,
                                         lut_size, lut->y, 0, NULL, NULL);
                err |= clEnqueueWriteBuffer(cl->queue, cl->d_lut_u, CL_TRUE, 0,
                                          lut_size, lut->u, 0, NULL, NULL);
                err |= clEnqueueWriteBuffer(cl->queue, cl->d_lut_v, CL_TRUE, 0,
                                          lut_size, lut->v, 0, NULL, NULL);
                
                if (err != CL_SUCCESS) {
                    printf("Failed to write LUT data: %d\n", err);
                    goto cleanup_lut_v;
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
        err |= clSetKernelArg(active_kernel, 5, sizeof(cl_mem), &cl->d_lut_y);
        err |= clSetKernelArg(active_kernel, 6, sizeof(cl_mem), &cl->d_lut_u);
        err |= clSetKernelArg(active_kernel, 7, sizeof(cl_mem), &cl->d_lut_v);
        err |= clSetKernelArg(active_kernel, 8, sizeof(int), &lut->size);
        err |= clSetKernelArg(active_kernel, 9, sizeof(int), &width);
        err |= clSetKernelArg(active_kernel, 10, sizeof(int), &height);
    } else {
        err |= clSetKernelArg(active_kernel, 5, sizeof(int), &width);
        err |= clSetKernelArg(active_kernel, 6, sizeof(int), &height);
    }

    if (err != CL_SUCCESS) {
        printf("Failed to set kernel arguments: %d\n", err);
        goto cleanup_lut;
    }

    // 确保工作组大小合适
    size_t global_work_size[2] = {
        ((width + 15) / 16) * 16,  // 向上取整到16的倍数
        ((height + 15) / 16) * 16
    };
    size_t local_work_size[2] = {16, 16};

    // 执行内核
    err = clEnqueueNDRangeKernel(cl->queue, active_kernel, 2, NULL,
                                global_work_size, local_work_size,
                                0, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Failed to execute kernel: %d\n", err);
        goto cleanup_lut;
    }

    // 读取结果
    err = clEnqueueReadBuffer(cl->queue, cl->d_y_out, CL_TRUE, 0,
                            width * height, y_out,
                            0, NULL, NULL);
    err |= clEnqueueReadBuffer(cl->queue, cl->d_uv_out, CL_TRUE, 0,
                             width * height / 2, uv_out,
                             0, NULL, NULL);
    
    if (err != CL_SUCCESS) {
        printf("Failed to read results: %d\n", err);
        goto cleanup_lut;
    }

    clFinish(cl->queue);

cleanup_lut:
    // 清理LUT缓冲区
    if (active_kernel == cl->kernel_color_map) {
        if (cl->d_lut_y) clReleaseMemObject(cl->d_lut_y);
        if (cl->d_lut_u) clReleaseMemObject(cl->d_lut_u);
        if (cl->d_lut_v) clReleaseMemObject(cl->d_lut_v);
        cl->d_lut_y = cl->d_lut_u = cl->d_lut_v = NULL;
    }
    return (err == CL_SUCCESS) ? 0 : -1;

cleanup_lut_v:
    clReleaseMemObject(cl->d_lut_v);
cleanup_lut_u:
    clReleaseMemObject(cl->d_lut_u);
cleanup_lut_y:
    clReleaseMemObject(cl->d_lut_y);
    return -1;
}