__kernel void color_map_nv12(
    __global ushort* input,
    __global uchar* y_out,
    __global uchar* uv_out,
    float scale,
    float min_val,
    __global uchar* lut_y,
    __global uchar* lut_u,
    __global uchar* lut_v,
    int lut_size,
    int width,
    int height,
    float scale_min,    // 新增参数
    float scale_max)    // 新增参数
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    if(x >= width || y >= height)
        return;
        
    int idx = y * width + x;
    float mapped = (input[idx] - min_val) * scale;
    mapped = clamp(mapped, 0.0f, 255.0f);
    
    // 新的映射逻辑
    float lut_min = scale_min * lut_size;
    float lut_max = scale_max * lut_size;
    
    // 将[0,255]映射到[lut_min,lut_max]
    float color_idx_f = (mapped * (lut_size - 1) / 255.0f);
    
    int color_idx;
    if(color_idx_f <= lut_min) {
        color_idx = 0;
    } else if(color_idx_f >= lut_max) {
        color_idx = lut_size - 1;
    } else {
        // 将[lut_min,lut_max]重新映射到[0,lut_size-1]
        color_idx = (int)((color_idx_f - lut_min) * (lut_size - 1) / (lut_max - lut_min));
    }
    
    color_idx = clamp(color_idx, 0, lut_size - 1);
    
    // 写入Y分量
    y_out[idx] = lut_y[color_idx];
    
    // UV处理
    if((x < width/2) && (y < height/2)) {
        int uv_idx = y * width + x * 2;
        
        uint sum = 0;
        for(int di = 0; di < 2; di++) {
            for(int dj = 0; dj < 2; dj++) {
                sum += input[(y*2+di) * width + (x*2+dj)];
            }
        }
        
        float avg_mapped = ((sum / 4) - min_val) * scale;
        avg_mapped = clamp(avg_mapped, 0.0f, 255.0f);
        
        float uv_color_idx_f = (avg_mapped * (lut_size - 1) / 255.0f);
        
        int uv_color_idx;
        if(uv_color_idx_f <= lut_min) {
            uv_color_idx = 0;
        } else if(uv_color_idx_f >= lut_max) {
            uv_color_idx = lut_size - 1;
        } else {
            uv_color_idx = (int)((uv_color_idx_f - lut_min) * (lut_size - 1) / (lut_max - lut_min));
        }
        
        uv_color_idx = clamp(uv_color_idx, 0, lut_size - 1);
        
        uv_out[uv_idx] = lut_u[uv_color_idx];
        uv_out[uv_idx + 1] = lut_v[uv_color_idx];
    }
}

__kernel void black_hot_nv12(
    __global ushort* input,
    __global uchar* y_out,
    __global uchar* uv_out,
    float scale,
    float min_val,
    int width,
    int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    // 边界检查
    if(x >= width || y >= height)
        return;
        
    // Y分量处理
    int idx = y * width + x;
    float mapped = (input[idx] - min_val) * scale;
    mapped = clamp(mapped, 0.0f, 255.0f);
    y_out[idx] = 255 - (uchar)mapped;  // 黑热图像需要反转
    
    // UV处理 (设置为128，表示无色差)
    if((x < width/2) && (y < height/2)) {
        int uv_idx = y * width + x * 2;
        uv_out[uv_idx] = 128;     // U
        uv_out[uv_idx + 1] = 128; // V
    }
}

__kernel void white_hot_nv12(
    __global ushort* input,
    __global uchar* y_out,
    __global uchar* uv_out,
    float scale,
    float min_val,
    int width,
    int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    // 边界检查
    if(x >= width || y >= height)
        return;
        
    // Y分量处理
    int idx = y * width + x;
    float mapped = (input[idx] - min_val) * scale;
    mapped = clamp(mapped, 0.0f, 255.0f);
    y_out[idx] = (uchar)mapped;  // 白热图像直接使用映射值
    
    // UV处理 (设置为128，表示无色差)
    if((x < width/2) && (y < height/2)) {
        int uv_idx = y * width + x * 2;
        uv_out[uv_idx] = 128;     // U
        uv_out[uv_idx + 1] = 128; // V
    }
}