#include "../src/config/lut/lut.h"
#include "../src/config/config.h"
#include "../src/media/algo/algo.h"
#include <iostream>
#include <string>

// 测试数据生成
uint16_t* Generate_Test_Pattern(int width, int height)
{
    uint16_t* test_data = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    if (!test_data) {
        return NULL;
    }

    // 计算每行应该增加的值
    // 因为要覆盖0-65535的范围，所以用65535除以行数
    uint16_t step = 65535 / (height - 1);  // -1确保最后一行是65535
    
    // 按行填充数据
    for(int i = 0; i < height; i++)
    {
        uint16_t row_value = i * step;  // 每行的值
        for(int j = 0; j < width; j++)
        {
            test_data[i * width + j] = row_value;
        }
    }

    return test_data;
}

// 测试函数
void Test_Pseudo_Color(std::string filename)
{
    // 生成测试数据
    uint16_t* test_data = Generate_Test_Pattern(640, 512);
    if (!test_data) {
        printf("Failed to allocate test data\n");
        return;
    }

    // 分配输出缓冲区
    uint8_t* y_out = (uint8_t*)malloc(640 * 512);
    uint8_t* u_out = (uint8_t*)malloc(640 * 512 / 4);
    uint8_t* v_out = (uint8_t*)malloc(640 * 512 / 4);

    if (!y_out || !u_out || !v_out) {
        printf("Failed to allocate output buffers\n");
        free(test_data);
        if (y_out) free(y_out);
        if (u_out) free(u_out);
        if (v_out) free(v_out);
        return;
    }

    // 调用Pseudo函数
    Pseudo(test_data, y_out, u_out, v_out, 640, 512);

    // 保存结果到文件
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        // 写入Y分量
        fwrite(y_out, 1, 640 * 512, fp);
        // 写入U分量
        fwrite(u_out, 1, 640 * 512 / 4, fp);
        // 写入V分量
        fwrite(v_out, 1, 640 * 512 / 4, fp);
        fclose(fp);
    }

    // 打印一些值用于验证
    printf("Test Pattern Values:\n");
    for(int i = 0; i < 512; i += 32)  // 每隔32行打印一次
    {
        printf("Row %3d: value=%5u, Y=%3u, U=%3u, V=%3u\n",
               i,
               test_data[i * 640],
               y_out[i * 640],
               u_out[(i/2) * (640/2)],
               v_out[(i/2) * (640/2)]);
    }

    // 清理
    free(test_data);
    free(y_out);
    free(u_out);
    free(v_out);
}

int main()
{
    Config_Init();
    
    usr.pseudo = PSEUDO_IRONBOW_FORWARD;
    Test_Pseudo_Color("PSEUDO_IRONBOW_FORWARD.yuv");
    usr.pseudo = PSEUDO_IRONBOW_REVERSE;
    Test_Pseudo_Color("PSEUDO_IRONBOW_REVERSE.yuv");
    usr.pseudo = PSEUDO_LAVA_FORWARD;
    Test_Pseudo_Color("PSEUDO_LAVA_FORWARD.yuv");
    usr.pseudo = PSEUDO_LAVA_REVERSE;
    Test_Pseudo_Color("PSEUDO_LAVA_REVERSE.yuv");
    usr.pseudo = PSEUDO_RAINBOW_FORWARD;
    Test_Pseudo_Color("PSEUDO_RAINBOW_FORWARD.yuv");
    usr.pseudo = PSEUDO_RAINBOW_REVERSE;
    Test_Pseudo_Color("PSEUDO_RAINBOW_REVERSE.yuv");
    usr.pseudo = PSEUDO_RAINBOWHC_FORWARD;
    Test_Pseudo_Color("PSEUDO_RAINBOWHC_FORWARD.yuv");
    usr.pseudo = PSEUDO_RAINBOWHC_REVERSE;
    Test_Pseudo_Color("PSEUDO_RAINBOWHC_REVERSE.yuv");
    
    Config_Exit();
    return 0;
}