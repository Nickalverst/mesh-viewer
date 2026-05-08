#include <stdio.h>
#include <stdlib.h>

struct objContent
{
    int n_vertices;
    float* vertices;
    int n_faceElements;
    int* faceElements;
};
objContent readfile(char* path);
char *read_file_into_string(const char *filename);