// diff.cl
__kernel void compute_diff(
    __global const ushort* current,
    __global const ushort* last,
    __global ushort* output,
    float rate,
    int width,
    int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    if(x >= width || y >= height)
        return;

    int idx = y * width + x;

    float last_val = convert_float(last[idx]) * rate;
    float curr_val = convert_float(current[idx]);

    float diff = fabs(curr_val - last_val);

    output[idx] = convert_ushort(diff);
}

__kernel void normalize_diff(
    __global ushort* input,
    __global ushort* output,
    ushort min_val,
    ushort max_val,
    int width,
    int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    if(x >= width || y >= height)
        return;
        
    int idx = y * width + x;
    float val = convert_float(input[idx]);
    
    // 应用阈值
    val = fmax(convert_float(min_val), val);
    val = fmin(convert_float(max_val), val);
    
    // 归一化到16位范围
    float normalized = (val - convert_float(min_val)) * 65535.0f / 
                      (convert_float(max_val) - convert_float(min_val));
                      
    output[idx] = convert_ushort(normalized);
}