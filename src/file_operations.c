#include "file_operations.h"

void open_file(const char *path) {
    char command[MAX_PATH + 10];
    snprintf(command, sizeof(command), "open \"%s\"", path);
    system(command);
}

void create_file(const char *path) {
    char filename[MAX_PATH];
    WINDOW *create_win = newwin(5, 40, LINES / 2 - 2, COLS / 2 - 20);
    box(create_win, 0, 0);
    mvwprintw(create_win, 1, 1, "Enter file name: ");
    wrefresh(create_win);
    echo();
    wgetnstr(create_win, filename, MAX_PATH);
    noecho();
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);
    FILE *file = fopen(full_path, "w");
    if (file == NULL) {
        mvwprintw(create_win, 3, 1, "Failed to create file.");
    } else {
        fclose(file);
        mvwprintw(create_win, 3, 1, "File created successfully.");
    }
    wrefresh(create_win);
    wgetch(create_win);
    delwin(create_win);
}

void create_directory(const char *path) {
    char dirname[MAX_PATH];
    WINDOW *create_win = newwin(5, 40, LINES / 2 - 2, COLS / 2 - 20);
    box(create_win, 0, 0);
    mvwprintw(create_win, 1, 1, "Enter directory name: ");
    wrefresh(create_win);
    echo();
    wgetnstr(create_win, dirname, MAX_PATH);
    noecho();
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, dirname);
    int status = mkdir(full_path, 0777);
    if (status == -1) {
        mvwprintw(create_win, 3, 1, "Failed to create directory.");
    } else {
        mvwprintw(create_win, 3, 1, "Directory created successfully.");
    }
    wrefresh(create_win);
    wgetch(create_win);
    delwin(create_win);
}

bool delete_directory_recursive(const char *path) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return false;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            struct stat file_stat;
            if (lstat(full_path, &file_stat) == 0) {
                if (S_ISDIR(file_stat.st_mode)) {
                    if (!delete_directory_recursive(full_path)) {
                        closedir(dir);
                        return false;
                    }
                } else {
                    if (remove(full_path) != 0) {
                        closedir(dir);
                        return false;
                    }
                }
            } else {
                perror("lstat");
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);

    if (rmdir(path) != 0) {
        perror("rmdir");
        return false;
    }

    return true;
}

void delete_file_or_directory(const char *path, const char *name) {
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, name);

    char message[MAX_PATH + 50];
    snprintf(message, sizeof(message), "Are you sure you want to delete '%s'? (y/n): ", name);
    WINDOW *delete_win = newwin(5, 50, LINES / 2 - 2, COLS / 2 - 25);
    box(delete_win, 0, 0);
    mvwprintw(delete_win, 1, 1, "%s", message);
    wrefresh(delete_win);
    echo();
    char confirmation;
    confirmation = wgetch(delete_win);
    noecho();
    if (confirmation == 'y' || confirmation == 'Y') {
        struct stat file_stat;
        if (lstat(full_path, &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                if (delete_directory_recursive(full_path)) {
                    mvwprintw(delete_win, 3, 1, "Deleted successfully.");
                } else {
                    mvwprintw(delete_win, 3, 1, "Failed to delete.");
                }
            } else {
                if (remove(full_path) == 0) {
                    mvwprintw(delete_win, 3, 1, "Deleted successfully.");
                } else {
                    mvwprintw(delete_win, 3, 1, "Failed to delete.");
                }
            }
        } else {
            mvwprintw(delete_win, 3, 1, "Failed to delete.");
        }
    } else {
        mvwprintw(delete_win, 3, 1, "Deletion cancelled.");
    }
    wrefresh(delete_win);
    wgetch(delete_win);
    delwin(delete_win);
}

void move_file_or_directory(const char *source_path, const char *destination_path, const char *name) {
    char source_full_path[MAX_PATH];
    char destination_full_path[MAX_PATH];
    snprintf(source_full_path, sizeof(source_full_path), "%s/%s", source_path, name);
    snprintf(destination_full_path, sizeof(destination_full_path), "%s/%s", destination_path, name);

    struct stat source_stat;
    if (lstat(source_full_path, &source_stat) == 0) {
        if (S_ISDIR(source_stat.st_mode)) {
            char command[MAX_PATH * 2 + 15];
            snprintf(command, sizeof(command), "mv \"%s\" \"%s\"", source_full_path, destination_full_path);
            system(command);
        } else {
            rename(source_full_path, destination_full_path);
        }
    } else {
        perror("lstat");
    }
}

bool confirm_move(const char *destination_path, const char *name) {
    char message[MAX_PATH + MAX_PATH + 50];
    snprintf(message, sizeof(message), "Are you sure you want to move '%s' to '%s'? (y/n): ", name, destination_path);
    WINDOW *confirm_win = newwin(5, 70, LINES / 2 - 2, COLS / 2 - 35);
    box(confirm_win, 0, 0);
    mvwprintw(confirm_win, 1, 1, "%s", message);
    wrefresh(confirm_win);
    echo();
    char confirmation;
    confirmation = wgetch(confirm_win);
    noecho();
    delwin(confirm_win);

    return (confirmation == 'y' || confirmation == 'Y');
}

void copy_file(const char *source_path, const char *destination_path, const char *name) {
    char source_file[MAX_PATH];
    char destination_file[MAX_PATH];

    snprintf(source_file, sizeof(source_file), "%s/%s", source_path, name);
    snprintf(destination_file, sizeof(destination_file), "%s/%s", destination_path, name);

    int source_fd = open(source_file, O_RDONLY);
    if (source_fd == -1) {
        perror("Failed to open source file");
        return;
    }

    int destination_fd = open(destination_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (destination_fd == -1) {
        perror("Failed to create destination file");
        close(source_fd);
        return;
    }

    char buf[BUF_SIZE];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(source_fd, buf, BUF_SIZE)) > 0) {
        bytes_written = write(destination_fd, buf, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Failed to write to destination file");
            close(source_fd);
            close(destination_fd);
            return;
        }
    }

    if (bytes_read == -1) {
        perror("Failed to read from source file");
    }

    close(source_fd);
    close(destination_fd);
}

void copy_directory(const char *source_path, const char *destination_path, const char *name) {
    char source_directory[MAX_PATH];
    char destination_directory[MAX_PATH];

    snprintf(source_directory, sizeof(source_directory), "%s/%s", source_path, name);
    snprintf(destination_directory, sizeof(destination_directory), "%s/%s", destination_path, name);

    mkdir(destination_directory, 0777);

    DIR *dir = opendir(source_directory);
    if (dir == NULL) {
        perror("Failed to open source directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char source_path_inner[MAX_PATH];
        char destination_path_inner[MAX_PATH];
        snprintf(source_path_inner, sizeof(source_path_inner), "%s/%s", source_directory, entry->d_name);
        snprintf(destination_path_inner, sizeof(destination_path_inner), "%s/%s", destination_directory, entry->d_name);

        struct stat file_stat;
        if (lstat(source_path_inner, &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                copy_directory(source_directory, destination_directory, entry->d_name);
            } else {
                copy_file(source_directory, destination_directory, entry->d_name);
            }
        } else {
            perror("Failed to get file stat");
        }
    }

    closedir(dir);
}

void copy_file_or_directory(const char *source_path, const char *destination_path, const char *name) {
    char source_file[MAX_PATH];
    char destination_file[MAX_PATH];

    snprintf(source_file, sizeof(source_file), "%s/%s", source_path, name);
    snprintf(destination_file, sizeof(destination_file), "%s/%s", destination_path, name);

    struct stat file_stat;
    if (lstat(source_file, &file_stat) == 0) {
        if (S_ISDIR(file_stat.st_mode)) {
            copy_directory(source_path, destination_path, name);
        } else {
            copy_file(source_path, destination_path, name);
        }
    } else {
        perror("Failed to copy file or directory");
    }
}

bool confirm_copy(const char *destination_path, const char *name) {
    char message[MAX_PATH + MAX_PATH + 50];
    snprintf(message, sizeof(message), "Are you sure you want to copy '%s' to '%s'? (y/n): ", name, destination_path);
    WINDOW *confirm_win = newwin(5, 70, LINES / 2 - 2, COLS / 2 - 35);
    box(confirm_win, 0, 0);
    mvwprintw(confirm_win, 1, 1, "%s", message);
    wrefresh(confirm_win);
    echo();
    char confirmation;
    confirmation = wgetch(confirm_win);
    noecho();
    delwin(confirm_win);

    return (confirmation == 'y' || confirmation == 'Y');
}

void edit_name(char *new_name) {
    WINDOW *edit_win = newwin(5, 40, LINES / 2 - 2, COLS / 2 - 20);
    box(edit_win, 0, 0);
    mvwprintw(edit_win, 1, 1, "Enter new name: ");
    wrefresh(edit_win);
    echo();
    wgetnstr(edit_win, new_name, MAX_PATH);
    noecho();
    delwin(edit_win);
}

bool is_name_exists(const char *path, const char *name) {
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, name);
    struct stat file_stat;
    return (stat(full_path, &file_stat) == 0);
}

void rename_file_or_directory(const char *path, const char *old_name, const char *new_name) {
    char old_path[MAX_PATH];
    char new_path[MAX_PATH];

    snprintf(old_path, sizeof(old_path), "%s/%s", path, old_name);
    snprintf(new_path, sizeof(new_path), "%s/%s", path, new_name);

    if (is_name_exists(path, new_name)) {
        return;
    }

    if (rename(old_path, new_path) == -1) {
        perror("Failed to rename file or directory");
    }
}
