#include <stdio.h>
#include <stdlib.h>

struct objContent
{
    int n_vertices;
    float* vertices;

    int n_faceElements;
    int* faceElements;

    float* normals; // normals for each vertex
    int n_normals; // not necessarily the same as n_vertices (flat shading)

    int* faceNormalIndices; // indices for normals if they are not per vertex
    int n_faceNormalIndices;

    float* vertexNormals; // per-vertex normals (3 floats per vertex), ready for rendering
    int n_vertexNormals;
};

objContent readfile(char* path);
char *read_file_into_string(const char *filename);