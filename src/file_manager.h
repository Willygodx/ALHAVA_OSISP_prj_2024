#ifndef ALHAVA_OSISP_PRJ_2024_FILE_MANAGER_H
#define ALHAVA_OSISP_PRJ_2024_FILE_MANAGER_H

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_PATH 1024

typedef struct {
    char name[MAX_PATH];
    off_t size;
    time_t modify_time;
    mode_t mode;
} FileInfo;

int count_files(const char *path);
char* get_filename_by_index(const char *path, int index);
FileInfo* get_files_info(const char *path, int *num_files);

#endif //ALHAVA_OSISP_PRJ_2024_FILE_MANAGER_H
