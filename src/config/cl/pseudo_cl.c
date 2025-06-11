#include "pseudo_cl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PseudoCL pseudo_cl = {0};

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
    char* source = read_kernel_source(CL_PSEUDO_PATH);
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
    cl->kernel_isotherms = clCreateKernel(cl->program, "color_map_isotherms", &err);

    if (err != CL_SUCCESS)
    {
        printf("Failed to create kernels\n");
        goto cleanup_program;
    }

    // Buffer
    size_t input_size = width * height * sizeof(uint16_t);
    size_t y_size = width * height;
    size_t uv_size = y_size / 2;
    size_t temps_size = width * height * sizeof(float);
    // 3 * u * v = 6
    size_t uv_maps_size = 6 * sizeof(uint8_t);

    // 创建所有需要的缓冲区
    cl->d_input = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, input_size, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Failed to create input buffer: %d\n", err);
        goto cleanup_kernels;
    }

    cl->d_y_out = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY, y_size, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Failed to create Y output buffer: %d\n", err);
        goto cleanup_input;
    }

    cl->d_uv_out = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY, uv_size, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Failed to create UV output buffer: %d\n", err);
        goto cleanup_y_out;
    }

    cl->d_temps = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, temps_size, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Failed to create temps buffer: %d\n", err);
        goto cleanup_uv_out;
    }

    cl->d_uv_maps = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, uv_maps_size, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Failed to create UV maps buffer: %d\n", err);
        goto cleanup_temps;
    }

    cl->initialized = true;
    return true;

cleanup_temps:
    clReleaseMemObject(cl->d_temps);
cleanup_uv_out:
    clReleaseMemObject(cl->d_uv_out);
cleanup_y_out:
    clReleaseMemObject(cl->d_y_out);
cleanup_input:
    clReleaseMemObject(cl->d_input);
cleanup_kernels:
    if (cl->kernel_black_hot)
        clReleaseKernel(cl->kernel_black_hot);
    if (cl->kernel_white_hot)
        clReleaseKernel(cl->kernel_white_hot);
    if (cl->kernel_color_map)
        clReleaseKernel(cl->kernel_color_map);
    if (cl->kernel_isotherms)
        clReleaseKernel(cl->kernel_isotherms);
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
    if (cl->d_temps)
        clReleaseMemObject(cl->d_temps);
    if (cl->d_uv_maps)
        clReleaseMemObject(cl->d_uv_maps);

    if (cl->kernel_black_hot)
        clReleaseKernel(cl->kernel_black_hot);
    if (cl->kernel_white_hot)
        clReleaseKernel(cl->kernel_white_hot);
    if (cl->kernel_color_map)
        clReleaseKernel(cl->kernel_color_map);
    if (cl->kernel_isotherms)
        clReleaseKernel(cl->kernel_isotherms);

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
                         float min_val,
                         float scale_min,
                         float scale_max)
{
    if (!cl->initialized)
        return -1;

    cl_int err;

    // Input data
    err = clEnqueueWriteBuffer(cl->queue, cl->d_input, CL_TRUE, 0, width * height * sizeof(uint16_t), input, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to write input data: %d\n", err);
        return -1;
    }

    // Choose Kernel
    cl_kernel active_kernel = NULL;
    switch (pseudo_type)
    {
    case PSEUDO_BLACK_HOT:
        active_kernel = cl->kernel_black_hot;
        break;
    case PSEUDO_WHITE_HOT:
        active_kernel = cl->kernel_white_hot;
        break;
    default:
        if (lut)
        {
            active_kernel = cl->kernel_color_map;

            // 获取当前LUT大小
            size_t current_lut_size = 0;
            if (cl->d_lut_y)
            {
                clGetMemObjectInfo(cl->d_lut_y, CL_MEM_SIZE, sizeof(size_t), &current_lut_size, NULL);
            }

            // 如果LUT大小改变，需要重新创建buffer
            size_t new_lut_size = lut->size * sizeof(uint8_t);
            if (current_lut_size != new_lut_size)
            {
                // 释放旧的buffer
                if (cl->d_lut_y)
                {
                    clReleaseMemObject(cl->d_lut_y);
                    cl->d_lut_y = NULL;
                }
                if (cl->d_lut_u)
                {
                    clReleaseMemObject(cl->d_lut_u);
                    cl->d_lut_u = NULL;
                }
                if (cl->d_lut_v)
                {
                    clReleaseMemObject(cl->d_lut_v);
                    cl->d_lut_v = NULL;
                }

                // 创建新的buffer
                cl->d_lut_y = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, new_lut_size, NULL, &err);
                if (err != CL_SUCCESS)
                {
                    printf("Failed to create LUT Y buffer: %d\n", err);
                    return -1;
                }

                cl->d_lut_u = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, new_lut_size, NULL, &err);
                if (err != CL_SUCCESS)
                {
                    printf("Failed to create LUT U buffer: %d\n", err);
                    goto cleanup_lut_y;
                }

                cl->d_lut_v = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, new_lut_size, NULL, &err);
                if (err != CL_SUCCESS)
                {
                    printf("Failed to create LUT V buffer: %d\n", err);
                    goto cleanup_lut_u;
                }
            }

            // Write lut data
            err = clEnqueueWriteBuffer(cl->queue, cl->d_lut_y, CL_TRUE, 0, new_lut_size, lut->y, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("Failed to write LUT Y data: %d\n", err);
                goto cleanup_lut_v;
            }

            err = clEnqueueWriteBuffer(cl->queue, cl->d_lut_u, CL_TRUE, 0, new_lut_size, lut->u, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("Failed to write LUT U data: %d\n", err);
                goto cleanup_lut_v;
            }

            err = clEnqueueWriteBuffer(cl->queue, cl->d_lut_v, CL_TRUE, 0, new_lut_size, lut->v, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("Failed to write LUT V data: %d\n", err);
                goto cleanup_lut_v;
            }
        }
        break;
    }

    if (!active_kernel)
    {
        printf("Invalid kernel selection\n");
        return -1;
    }

    // Set kernel arguments
    err = clSetKernelArg(active_kernel, 0, sizeof(cl_mem), &cl->d_input);
    err |= clSetKernelArg(active_kernel, 1, sizeof(cl_mem), &cl->d_y_out);
    err |= clSetKernelArg(active_kernel, 2, sizeof(cl_mem), &cl->d_uv_out);
    err |= clSetKernelArg(active_kernel, 3, sizeof(float), &scale);
    err |= clSetKernelArg(active_kernel, 4, sizeof(float), &min_val);

    if (active_kernel == cl->kernel_color_map)
    {
        err |= clSetKernelArg(active_kernel, 5, sizeof(cl_mem), &cl->d_lut_y);
        err |= clSetKernelArg(active_kernel, 6, sizeof(cl_mem), &cl->d_lut_u);
        err |= clSetKernelArg(active_kernel, 7, sizeof(cl_mem), &cl->d_lut_v);
        err |= clSetKernelArg(active_kernel, 8, sizeof(int), &lut->size);
        err |= clSetKernelArg(active_kernel, 9, sizeof(int), &width);
        err |= clSetKernelArg(active_kernel, 10, sizeof(int), &height);
        err |= clSetKernelArg(active_kernel, 11, sizeof(float), &scale_min);
        err |= clSetKernelArg(active_kernel, 12, sizeof(float), &scale_max);
    }
    else
    {
        err |= clSetKernelArg(active_kernel, 5, sizeof(int), &width);
        err |= clSetKernelArg(active_kernel, 6, sizeof(int), &height);
    }

    if (err != CL_SUCCESS)
    {
        printf("Failed to set kernel arguments: %d\n", err);
        return -1;
    }

    // Set work size
    size_t global_work_size[2] = {((width + 15) / 16) * 16, ((height + 15) / 16) * 16};
    size_t local_work_size[2] = {16, 16};

    // Execute kernel
    err = clEnqueueNDRangeKernel(cl->queue, active_kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to execute kernel: %d\n", err);
        return -1;
    }

    // Read results
    err = clEnqueueReadBuffer(cl->queue, cl->d_y_out, CL_TRUE, 0, width * height, y_out, 0, NULL, NULL);
    err |= clEnqueueReadBuffer(cl->queue, cl->d_uv_out, CL_TRUE, 0, width * height / 2, uv_out, 0, NULL, NULL);

    if (err != CL_SUCCESS)
    {
        printf("Failed to read results: %d\n", err);
        return -1;
    }

    clFinish(cl->queue);
    return 0;

cleanup_lut_v:
    if (cl->d_lut_v)
    {
        clReleaseMemObject(cl->d_lut_v);
        cl->d_lut_v = NULL;
    }
cleanup_lut_u:
    if (cl->d_lut_u)
    {
        clReleaseMemObject(cl->d_lut_u);
        cl->d_lut_u = NULL;
    }
cleanup_lut_y:
    if (cl->d_lut_y)
    {
        clReleaseMemObject(cl->d_lut_y);
        cl->d_lut_y = NULL;
    }
    return -1;
}

int PseudoCL_ProcessIsotherms(PseudoCL* cl,
                              uint16_t* input,
                              uint8_t* y_out,
                              uint8_t* uv_out,
                              int width,
                              int height,
                              const struct YUV420P_LUT* lut,
                              float scale,
                              float min_val,
                              float* temps,
                              float threshold_min,
                              float threshold_max,
                              uint8_t* uv_maps)
{
    if (!cl->initialized)
        return -1;

    cl_int err = CL_SUCCESS;

    // 1. 处理输入数据
    err = clEnqueueWriteBuffer(cl->queue, cl->d_input, CL_TRUE, 0, width * height * sizeof(uint16_t), input, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to write input data: %d\n", err);
        return -1;
    }

    // 2. 处理色表数据（如果需要）
    if (lut)
    {
        size_t current_lut_size = 0;
        if (cl->d_lut_y)
        {
            clGetMemObjectInfo(cl->d_lut_y, CL_MEM_SIZE, sizeof(size_t), &current_lut_size, NULL);
        }

        size_t new_lut_size = lut->size * sizeof(uint8_t);
        if (current_lut_size != new_lut_size)
        {
            // 释放旧的LUT缓冲区
            if (cl->d_lut_y)
            {
                clReleaseMemObject(cl->d_lut_y);
                cl->d_lut_y = NULL;
            }
            if (cl->d_lut_u)
            {
                clReleaseMemObject(cl->d_lut_u);
                cl->d_lut_u = NULL;
            }
            if (cl->d_lut_v)
            {
                clReleaseMemObject(cl->d_lut_v);
                cl->d_lut_v = NULL;
            }

            // 创建新的LUT缓冲区
            cl->d_lut_y = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, new_lut_size, NULL, &err);
            if (err != CL_SUCCESS)
            {
                printf("Failed to create LUT Y buffer: %d\n", err);
                return -1;
            }

            cl->d_lut_u = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, new_lut_size, NULL, &err);
            if (err != CL_SUCCESS)
            {
                goto cleanup_lut_y;
            }

            cl->d_lut_v = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, new_lut_size, NULL, &err);
            if (err != CL_SUCCESS)
            {
                goto cleanup_lut_u;
            }
        }

        // 写入LUT数据
        err = clEnqueueWriteBuffer(cl->queue, cl->d_lut_y, CL_TRUE, 0, new_lut_size, lut->y, 0, NULL, NULL);
        if (err != CL_SUCCESS)
            goto cleanup_lut_v;

        err = clEnqueueWriteBuffer(cl->queue, cl->d_lut_u, CL_TRUE, 0, new_lut_size, lut->u, 0, NULL, NULL);
        if (err != CL_SUCCESS)
            goto cleanup_lut_v;

        err = clEnqueueWriteBuffer(cl->queue, cl->d_lut_v, CL_TRUE, 0, new_lut_size, lut->v, 0, NULL, NULL);
        if (err != CL_SUCCESS)
            goto cleanup_lut_v;
    }

    // 3. 写入温度数据
    err = clEnqueueWriteBuffer(cl->queue, cl->d_temps, CL_TRUE, 0, width * height * sizeof(float), temps, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to write temperature data: %d\n", err);
        goto cleanup_lut;
    }

    // 4. 写入UV映射表
    err = clEnqueueWriteBuffer(cl->queue, cl->d_uv_maps, CL_TRUE, 0, 6 * sizeof(uint8_t), uv_maps, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to write UV maps data: %d\n", err);
        goto cleanup_lut;
    }

    // 5. 设置kernel参数
    err = clSetKernelArg(cl->kernel_isotherms, 0, sizeof(cl_mem), &cl->d_input);
    err |= clSetKernelArg(cl->kernel_isotherms, 1, sizeof(cl_mem), &cl->d_y_out);
    err |= clSetKernelArg(cl->kernel_isotherms, 2, sizeof(cl_mem), &cl->d_uv_out);
    err |= clSetKernelArg(cl->kernel_isotherms, 3, sizeof(float), &scale);
    err |= clSetKernelArg(cl->kernel_isotherms, 4, sizeof(float), &min_val);

    if (lut)
    {
        err |= clSetKernelArg(cl->kernel_isotherms, 5, sizeof(cl_mem), &cl->d_lut_y);
        err |= clSetKernelArg(cl->kernel_isotherms, 6, sizeof(cl_mem), &cl->d_lut_u);
        err |= clSetKernelArg(cl->kernel_isotherms, 7, sizeof(cl_mem), &cl->d_lut_v);
        err |= clSetKernelArg(cl->kernel_isotherms, 8, sizeof(int), &lut->size);
    }
    else
    {
        // 如果没有LUT，使用NULL
        cl_mem null_mem = NULL;
        err |= clSetKernelArg(cl->kernel_isotherms, 5, sizeof(cl_mem), &null_mem);
        err |= clSetKernelArg(cl->kernel_isotherms, 6, sizeof(cl_mem), &null_mem);
        err |= clSetKernelArg(cl->kernel_isotherms, 7, sizeof(cl_mem), &null_mem);
        int zero = 0;
        err |= clSetKernelArg(cl->kernel_isotherms, 8, sizeof(int), &zero);
    }

    err |= clSetKernelArg(cl->kernel_isotherms, 9, sizeof(int), &width);
    err |= clSetKernelArg(cl->kernel_isotherms, 10, sizeof(int), &height);
    err |= clSetKernelArg(cl->kernel_isotherms, 11, sizeof(cl_mem), &cl->d_temps);
    err |= clSetKernelArg(cl->kernel_isotherms, 12, sizeof(float), &threshold_min);
    err |= clSetKernelArg(cl->kernel_isotherms, 13, sizeof(float), &threshold_max);
    err |= clSetKernelArg(cl->kernel_isotherms, 14, sizeof(cl_mem), &cl->d_uv_maps);

    if (err != CL_SUCCESS)
    {
        printf("Failed to set kernel arguments: %d\n", err);
        goto cleanup_lut;
    }

    // 6. 执行kernel
    size_t global_work_size[2] = {((width + 15) / 16) * 16, ((height + 15) / 16) * 16};
    size_t local_work_size[2] = {16, 16};

    err = clEnqueueNDRangeKernel(cl->queue, cl->kernel_isotherms, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to execute kernel: %d\n", err);
        goto cleanup_lut;
    }

    // 7. 读取结果
    err = clEnqueueReadBuffer(cl->queue, cl->d_y_out, CL_TRUE, 0, width * height, y_out, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to read Y output: %d\n", err);
        goto cleanup_lut;
    }

    err = clEnqueueReadBuffer(cl->queue, cl->d_uv_out, CL_TRUE, 0, width * height / 2, uv_out, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to read UV output: %d\n", err);
        goto cleanup_lut;
    }

    clFinish(cl->queue);
    return 0;

cleanup_lut_v:
    if (cl->d_lut_v)
    {
        clReleaseMemObject(cl->d_lut_v);
        cl->d_lut_v = NULL;
    }
cleanup_lut_u:
    if (cl->d_lut_u)
    {
        clReleaseMemObject(cl->d_lut_u);
        cl->d_lut_u = NULL;
    }
cleanup_lut_y:
    if (cl->d_lut_y)
    {
        clReleaseMemObject(cl->d_lut_y);
        cl->d_lut_y = NULL;
    }
cleanup_lut:
    return -1;
}