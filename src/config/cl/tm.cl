// temperature measurement
__kernel void compute_temperature(
    __global const ushort* input,
    __global float* output,
    float a,
    float b,
    float c,
    int width,
    int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    if (x >= width || y >= height)
        return;
        
    int idx = y * width + x;
    float val = (float)input[idx];
    output[idx] = a * val * val + b * val + c;
}