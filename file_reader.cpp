#include "file_reader.hpp"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
using namespace std;

// Reads a file into a dynamically allocated string.
// Returns NULL on failure (caller must free the result).
char *read_file_into_string(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }
 
    // Get file size
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        fclose(file);
        return NULL;
    }
 
    long file_size = ftell(file);
    if (file_size == -1) {
        perror("Error getting file size");
        fclose(file);
        return NULL;
    }
 
    if (fseek(file, 0, SEEK_SET) != 0) {
        perror("Error seeking to start of file");
        fclose(file);
        return NULL;
    }
 
    // Allocate buffer
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }
 
    // Read content
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }
 
    // Null-terminate
    buffer[file_size] = '\0';
 
    // Cleanup and return
    fclose(file);
    return buffer;
}

objContent readfile(char* path)  
{
    int vertex_capacity = 1024;
    int face_capacity = 1024;
    int normal_capacity = 1024;

    objContent new_obj;
    new_obj.vertices = (float*) malloc(sizeof(float) * vertex_capacity);
    new_obj.faceElements = (int*) malloc(sizeof(int) * face_capacity);
    new_obj.normals = (float*) malloc(sizeof(float) * normal_capacity);
    new_obj.faceNormalIndices = (int*) malloc(sizeof(int) * face_capacity);

    FILE* fptr = fopen(path, "r");

    if (fptr == NULL) {
        printf("The file is not opened.\n");
        exit(1);
    }

    char line[256];
    int normal_i = 0;
    int vertex_i = 0;
    int j = 0;
    while (fgets(line, sizeof(line), fptr)) {
        char *p = line;

        if (vertex_i + 3 >= vertex_capacity) {
            vertex_capacity *= 2;
            new_obj.vertices = (float*) realloc(new_obj.vertices, sizeof(float) * vertex_capacity);
        }

        if (strncmp(p, "v ", 2) == 0) {
            if (sscanf(p, "v %f %f %f", &new_obj.vertices[vertex_i], 
                                        &new_obj.vertices[vertex_i+1], 
                                        &new_obj.vertices[vertex_i+2]) == 3) {
                vertex_i += 3;
            }
        } else if (strncmp(p, "f ", 2) == 0) {
            if (j + 6 >= face_capacity) {
                face_capacity *= 2;
                new_obj.faceElements = (int*) realloc(new_obj.faceElements, sizeof(int) * face_capacity);
                new_obj.faceNormalIndices = (int*) realloc(new_obj.faceNormalIndices, sizeof(int) * face_capacity);
            }

            // tokenize the face line after the "f "
            char *s = p + 2;
            char *tok = strtok(s, " \t\r\n");
            int vi[4]; // vertex indices parsed from token
            int ni[4]; // normal indices parsed from token (0 if none)
            int cnt = 0;

            // here, a tok looks like "v", "v/vt", "v//vn", or "v/vt/vn". We only care about v and vn for now.
            while (tok && cnt < 4) {
                int v = 0, t = 0, n = 0;
                if (strstr(tok, "//")) {
                    sscanf(tok, "%d//%d", &v, &n);
                } else {
                    int c = sscanf(tok, "%d/%d/%d", &v, &t, &n);
                    if (c == 1) { /* only vertex index parsed, n stays 0 */ }
                    else if (c == 2) { /* v/vt parsed, n stays 0 */ }
                }
                vi[cnt] = v;
                ni[cnt] = n; // keep raw index (1-based) or 0 if missing
                cnt++;
                tok = strtok(NULL, " \t\r\n");
            }

            if (cnt == 4) {
                // triangulate quad into two triangles (0,1,2) and (0,2,3)
                // triangle 1
                new_obj.faceElements[j] = vi[0] - 1; new_obj.faceNormalIndices[j++] = ni[0] ? (ni[0] - 1) : -1;
                new_obj.faceElements[j] = vi[1] - 1; new_obj.faceNormalIndices[j++] = ni[1] ? (ni[1] - 1) : -1;
                new_obj.faceElements[j] = vi[2] - 1; new_obj.faceNormalIndices[j++] = ni[2] ? (ni[2] - 1) : -1;
                // triangle 2
                new_obj.faceElements[j] = vi[0] - 1; new_obj.faceNormalIndices[j++] = ni[0] ? (ni[0] - 1) : -1;
                new_obj.faceElements[j] = vi[2] - 1; new_obj.faceNormalIndices[j++] = ni[2] ? (ni[2] - 1) : -1;
                new_obj.faceElements[j] = vi[3] - 1; new_obj.faceNormalIndices[j++] = ni[3] ? (ni[3] - 1) : -1;
            } else if (cnt == 3) {
                new_obj.faceElements[j] = vi[0] - 1; new_obj.faceNormalIndices[j++] = ni[0] ? (ni[0] - 1) : -1;
                new_obj.faceElements[j] = vi[1] - 1; new_obj.faceNormalIndices[j++] = ni[1] ? (ni[1] - 1) : -1;
                new_obj.faceElements[j] = vi[2] - 1; new_obj.faceNormalIndices[j++] = ni[2] ? (ni[2] - 1) : -1;
            }
        } else if (strncmp(p, "vn ", 3) == 0) {
            if (normal_i + 3 >= normal_capacity) {
                normal_capacity *= 2;
                new_obj.normals = (float*) realloc(new_obj.normals, sizeof(float) * normal_capacity);
            }
            if (sscanf(p, "vn %f %f %f", &new_obj.normals[normal_i], 
                                        &new_obj.normals[normal_i+1], 
                                        &new_obj.normals[normal_i+2]) == 3) {
                normal_i += 3;
            }
        }
    }

    new_obj.n_vertices = vertex_i;
    new_obj.n_faceElements = j;
    new_obj.n_normals = normal_i;

    // allocate per-vertex normals and counters
    new_obj.n_vertexNormals = new_obj.n_vertices * 3;
    new_obj.vertexNormals = (float*) calloc(new_obj.n_vertexNormals, sizeof(float));
    int *vn_count = (int*) calloc(new_obj.n_vertices, sizeof(int));

    for (int k = 0; k < new_obj.n_faceElements; ++k) {
        int vIndex = new_obj.faceElements[k];
        int nIndex = new_obj.faceNormalIndices[k];
        if (vIndex < 0 || nIndex < 0) continue;

        float *out = &new_obj.vertexNormals[vIndex * 3];
        float *src = &new_obj.normals[nIndex * 3];

        out[0] += src[0];
        out[1] += src[1];
        out[2] += src[2];
        vn_count[vIndex] += 1;
    }

    // average and normalize
    for (int vi = 0; vi < new_obj.n_vertices; ++vi) {
        int c = vn_count[vi];
        float *out = &new_obj.vertexNormals[vi*3];
        if (c > 0) {
            out[0] /= c; out[1] /= c; out[2] /= c;
            float len = sqrtf(out[0]*out[0] + out[1]*out[1] + out[2]*out[2]);
            if (len > 1e-6f) { out[0] /= len; out[1] /= len; out[2] /= len; }
        } else {
            // fallback: zero or compute geometric normal from faces if you prefer
            out[0] = out[1] = out[2] = 0.0f;
        }
    }

    free(vn_count);
    fclose(fptr);
    
    return new_obj;
}