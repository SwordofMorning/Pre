#include "cl.h"

char* read_kernel_source(const char* filename)
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