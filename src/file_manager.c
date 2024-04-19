#include "file_manager.h"

int count_files(const char *path) {
    int count = 0;
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            count++;
        }
    }

    closedir(dir);
    return count;
}

char* get_filename_by_index(const char *path, int index) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return NULL;
    }

    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if (count == index) {
                closedir(dir);
                return strdup(entry->d_name);
            }
            count++;
        }
    }

    closedir(dir);
    return NULL;
}

FileInfo* get_files_info(const char *path, int *num_files) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return NULL;
    }

    *num_files = count_files(path);
    FileInfo *files_info = (FileInfo*)malloc(*num_files * sizeof(FileInfo));
    if (files_info == NULL) {
        perror("malloc");
        closedir(dir);
        return NULL;
    }

    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            struct stat file_stat;
            if (lstat(full_path, &file_stat) == 0) {
                strcpy(files_info[count].name, entry->d_name);
                files_info[count].size = file_stat.st_size;
                files_info[count].modify_time = file_stat.st_mtime;
                files_info[count].mode = file_stat.st_mode;
                count++;
            } else {
                perror("lstat");
            }
        }
    }

    closedir(dir);
    return files_info;
}
