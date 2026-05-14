#include "file_reader.hpp"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    objContent new_obj;
    new_obj.vertices = (float*) malloc(sizeof(float) * vertex_capacity);
    new_obj.faceElements = (int*) malloc(sizeof(int) * face_capacity);
    FILE* fptr = fopen(path, "r");

    if (fptr == NULL) {
        printf("The file is not opened.\n");
        exit(1);
    }

    char line[256];
    int i = 0;
    int j = 0;
    while (fgets(line, sizeof(line), fptr)) {
        char *p = line;

        if (i + 3 >= vertex_capacity) {
            vertex_capacity *= 2;
            new_obj.vertices = (float*) realloc(new_obj.vertices, sizeof(float) * vertex_capacity);
        }

        if (strncmp(p, "v ", 2) == 0) {
            if (sscanf(p, "v %f %f %f", &new_obj.vertices[i], 
                                        &new_obj.vertices[i+1], 
                                        &new_obj.vertices[i+2]) == 3) {
                i += 3;
            }
        } else if (strncmp(p, "f ", 2) == 0) {
            if (j + 6 >= face_capacity) {
                face_capacity *= 2;
                new_obj.faceElements = (int*) realloc(new_obj.faceElements, sizeof(int) * face_capacity);
            }
            int v[4];

            int matched = sscanf(
                p,
                "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d",
                &v[0], &v[1], &v[2], &v[3]
            );

            if (matched != 4) {
                matched = sscanf(
                    p,
                    "f %d/%*d %d/%*d %d/%*d",
                    &v[0], &v[1], &v[2]
                );
            }

            if (matched != 4) {
                matched = sscanf(
                    p,
                    "f %d//%*d %d//%*d %d//%*d %d//%*d",
                    &v[0], &v[1], &v[2], &v[3]
                );
            } 

            if (matched == 4) {
                new_obj.faceElements[j++] = v[0] - 1;
                new_obj.faceElements[j++] = v[1] - 1;
                new_obj.faceElements[j++] = v[2] - 1;

                new_obj.faceElements[j++] = v[0] - 1;
                new_obj.faceElements[j++] = v[2] - 1;
                new_obj.faceElements[j++] = v[3] - 1;

                continue;
            }

            if (matched != 3) {
                matched = sscanf(
                    p,
                    "f %d//%*d %d//%*d %d//%*d",
                    &v[0], &v[1], &v[2]
                );
            }

            if (matched != 3) {
                matched = sscanf(
                    p,
                    "f %d %d %d",
                    &v[0], &v[1], &v[2]
                );
            }

            if (matched == 3) {
                new_obj.faceElements[j++] = v[0] - 1;
                new_obj.faceElements[j++] = v[1] - 1;
                new_obj.faceElements[j++] = v[2] - 1;
            }
        }
    }

    new_obj.n_vertices = i;
    new_obj.n_faceElements = j;

    fclose(fptr);
    
    return new_obj;
}
