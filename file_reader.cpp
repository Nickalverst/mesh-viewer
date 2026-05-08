#include "obj_reader.hpp"
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
    objContent new_obj;
    new_obj.vertices = (float*) malloc(sizeof(float) * 100000);
    new_obj.faceElements = (int*) malloc(sizeof(int) * 100000);
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

        if (*p == 'v') {
            if (sscanf(p, "v %f %f %f", &new_obj.vertices[i], 
                                        &new_obj.vertices[i+1], 
                                        &new_obj.vertices[i+2]) == 3) {
                i += 3;
            } else printf("Erro ao escanear linha.\n");
        } else if (*p == 'f') {
            int v1, v2, v3;
            int matched = sscanf(p, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &v1, &v2, &v3);
            if (matched != 3) {
                matched = sscanf(p, "f %d %d %d", &v1, &v2, &v3);
            } else printf("Erro ao escanear linha %s.\n", line);
            if (matched == 3) {
                new_obj.faceElements[j] = v1 - 1;
                new_obj.faceElements[j+1] = v2 - 1;
                new_obj.faceElements[j+2] = v3 - 1;
                j += 3;
            }
        }
    }

    new_obj.n_vertices = i;
    new_obj.n_faceElements = j;

    fclose(fptr);
    
    return new_obj;
}
