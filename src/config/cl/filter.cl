// mean_filter.cl
__kernel void mean_filter_nv12(
    __global uchar* input,
    __global uchar* output,
    int width,
    int height,
    int window_size)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    if(x >= width || y >= height)
        return;
        
    int half_window = window_size / 2;
    int sum = 0;
    int count = 0;
    
    // 计算窗口内的平均值
    for(int wy = -half_window; wy <= half_window; wy++) {
        for(int wx = -half_window; wx <= half_window; wx++) {
            int nx = x + wx;
            int ny = y + wy;
            
            // 边界检查
            if(nx >= 0 && nx < width && ny >= 0 && ny < height) {
                sum += input[ny * width + nx];
                count++;
            }
        }
    }
    
    // 写入结果
    output[y * width + x] = (uchar)(sum / count);
}