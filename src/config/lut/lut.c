#include "lut.h"

// clang-format off
static struct YUV420P_LUT luts[LUT_TYPE_COUNT] = {
    {NULL, NULL, NULL, 0},
    {NULL, NULL, NULL, 0},
    {NULL, NULL, NULL, 0},
    {NULL, NULL, NULL, 0},
    {NULL, NULL, NULL, 0},
    {NULL, NULL, NULL, 0},
    {NULL, NULL, NULL, 0},
    {NULL, NULL, NULL, 0}
};
// clang-format on

static int init_single_lut(struct YUV420P_LUT* lut, const char* bin_file)
{
    FILE* fp = fopen(bin_file, "rb");
    if (!fp)
    {
        perror("Failed to open color map file");
        return -1;
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 计算颜色数量
    int color_count = file_size / 3;

    // 分配内存
    uint8_t* rgb_data = (uint8_t*)malloc(file_size);
    if (!rgb_data)
    {
        fclose(fp);
        perror("Failed to allocate RGB buffer");
        return -1;
    }

    // 读取RGB数据
    if (fread(rgb_data, 1, file_size, fp) != file_size)
    {
        free(rgb_data);
        fclose(fp);
        perror("Failed to read color map file");
        return -1;
    }
    fclose(fp);

    // 分配LUT内存
    lut->y = (uint8_t*)malloc(color_count);
    lut->u = (uint8_t*)malloc(color_count);
    lut->v = (uint8_t*)malloc(color_count);

    if (!lut->y || !lut->u || !lut->v)
    {
        free(rgb_data);
        if (lut->y)
            free(lut->y);
        if (lut->u)
            free(lut->u);
        if (lut->v)
            free(lut->v);
        lut->y = lut->u = lut->v = NULL;
        perror("Failed to allocate LUT buffers");
        return -1;
    }

    // 转换BGR到YUV
    for (int i = 0; i < color_count; i++)
    {
        uint8_t b = rgb_data[i * 3];
        uint8_t g = rgb_data[i * 3 + 1];
        uint8_t r = rgb_data[i * 3 + 2];

        lut->y[i] = RGB2Y(r, g, b);
        lut->u[i] = RGB2U(r, g, b);
        lut->v[i] = RGB2V(r, g, b);
    }

    lut->size = color_count;
    free(rgb_data);

    // printf("Loaded color map: %d colors\n", color_count);
    return 0;
}

static void free_single_lut(struct YUV420P_LUT* lut)
{
    if (lut->y)
        free(lut->y);
    if (lut->u)
        free(lut->u);
    if (lut->v)
        free(lut->v);
    lut->y = lut->u = lut->v = NULL;
    lut->size = 0;
}

int Init_LUT(int type, const char* bin_file)
{
    if (type >= LUT_TYPE_COUNT)
    {
        return -1;
    }

    Free_LUT(type);

    return init_single_lut(&luts[type], bin_file);
}

void Free_LUT(int type)
{
    if (type < LUT_TYPE_COUNT)
    {
        free_single_lut(&luts[type]);
    }
}

void Free_All_LUTs(void)
{
    for (int i = 0; i < LUT_TYPE_COUNT; i++)
    {
        free_single_lut(&luts[i]);
    }
}

const struct YUV420P_LUT* Get_LUT(int type)
{
    if (type >= LUT_TYPE_COUNT)
        return NULL;
    return &luts[type];
}