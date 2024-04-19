#ifndef ALHAVA_OSISP_PRJ_2024_FILE_OPERATIONS_H
#define ALHAVA_OSISP_PRJ_2024_FILE_OPERATIONS_H

#include "file_manager.h"
#include "display.h"
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define BUF_SIZE 4096

void open_file(const char *path);
void create_file(const char *path);
void create_directory(const char *path);
void edit_name(char *new_name);
bool is_name_exists(const char *path, const char *name);
bool delete_directory_recursive(const char *path);
void delete_file_or_directory(const char *path, const char *name);
void move_file_or_directory(const char *source_path, const char *destination_path, const char *name);
bool confirm_move(const char *destination_path, const char *name);
void copy_file(const char *source_path, const char *destination_path, const char *name);
void copy_directory(const char *source_path, const char *destination_path, const char *name);
void copy_file_or_directory(const char *source_path, const char *destination_path, const char *name);
bool confirm_copy(const char *destination_path, const char *name);
void rename_file_or_directory(const char *path, const char *old_name, const char *new_name);

#endif //ALHAVA_OSISP_PRJ_2024_FILE_OPERATIONS_H
