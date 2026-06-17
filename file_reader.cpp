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

    // accumulate normals per vertex
    int nTriangles = new_obj.n_faceElements / 3;
    for (int t = 0; t < nTriangles; ++t) {
        int idx = t * 3;
        int v0 = new_obj.faceElements[idx + 0];
        int v1 = new_obj.faceElements[idx + 1];
        int v2 = new_obj.faceElements[idx + 2];
        if (v0 < 0 || v1 < 0 || v2 < 0) continue;

        int n0 = new_obj.faceNormalIndices[idx + 0];
        int n1 = new_obj.faceNormalIndices[idx + 1];
        int n2 = new_obj.faceNormalIndices[idx + 2];

        // compute geometric normal once if any vertex lacks an explicit normal
        float faceN[3] = {0.0f, 0.0f, 0.0f};
        if (n0 < 0 || n1 < 0 || n2 < 0) {
            float *p0 = &new_obj.vertices[v0*3];
            float *p1 = &new_obj.vertices[v1*3];
            float *p2 = &new_obj.vertices[v2*3];
            float u[3] = { p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2] };
            float v_[3] = { p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2] };
            faceN[0] = u[1]*v_[2] - u[2]*v_[1];
            faceN[1] = u[2]*v_[0] - u[0]*v_[2];
            faceN[2] = u[0]*v_[1] - u[1]*v_[0];
            float flen = sqrtf(faceN[0]*faceN[0] + faceN[1]*faceN[1] + faceN[2]*faceN[2]);
            if (flen > 1e-6f) { faceN[0] /= flen; faceN[1] /= flen; faceN[2] /= flen; }
            else { faceN[0] = faceN[1] = faceN[2] = 0.0f; }
        }

        // helper to accumulate a normal to a vertex and increment count
        auto accum = [&](int vIdx, int nIdx) {
            if (vIdx < 0) return;
            if (nIdx >= 0) {
                float *src = &new_obj.normals[nIdx * 3];
                new_obj.vertexNormals[vIdx*3 + 0] += src[0];
                new_obj.vertexNormals[vIdx*3 + 1] += src[1];
                new_obj.vertexNormals[vIdx*3 + 2] += src[2];
            } else {
                new_obj.vertexNormals[vIdx*3 + 0] += faceN[0];
                new_obj.vertexNormals[vIdx*3 + 1] += faceN[1];
                new_obj.vertexNormals[vIdx*3 + 2] += faceN[2];
            }
            vn_count[vIdx] += 1;
        };

        accum(v0, n0);
        accum(v1, n1);
        accum(v2, n2);
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