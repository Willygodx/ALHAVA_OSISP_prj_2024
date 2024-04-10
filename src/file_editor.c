#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <sys/statvfs.h>

#define MAX_PATH 1024
#define BUF_SIZE 4096

typedef struct {
    char name[MAX_PATH];
    off_t size;
    time_t modify_time;
    mode_t mode;
} FileInfo;

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

void displayDiskSpace(const char *path) {
    struct statvfs stats;
    if (statvfs(path, &stats) == 0) {
        unsigned long long totalSpace = (unsigned long long)stats.f_blocks * stats.f_frsize;
        unsigned long long freeSpace = (unsigned long long)stats.f_bfree * stats.f_frsize;

        double totalSpaceGB = (double)totalSpace / (1024.0 * 1024 * 1024);
        double freeSpaceGB = (double)freeSpace / (1024.0 * 1024 * 1024);

        mvprintw(LINES - 1, 90, "%.2fG / %.2fG", freeSpaceGB, totalSpaceGB);
    } else {
        printw("Error getting disk space information");
    }
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

void draw_file_list(WINDOW *win, const char *path, int *cursor_y, int num_files, int max_x, int max_y) {
    FileInfo *files_info = get_files_info(path, &num_files);
    if (files_info == NULL) {
        return;
    }
    wclear(win);
    box(win, 0, 0);

    mvwprintw(win, 0, 2, "%s", path);

    mvwprintw(win, 1, 2, "Name");
    mvwprintw(win, 1, (max_x / 5) * 2 - 15, "Size");
    mvwprintw(win, 1, (max_x / 5) * 2 - 3, "Modify Date");

    mvwvline(win, 1, (max_x / 5) * 2 - 16, ACS_VLINE, max_y - 6);
    mvwvline(win, 1, (max_x / 5) * 2 - 4, ACS_VLINE, max_y - 6);

    if (strcmp(path, "/") != 0) {
        if (*cursor_y == 3) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, 3, 2, "../");
        if (*cursor_y == 3) {
            wmove(win, 3, 2);
        }
        wattroff(win, COLOR_PAIR(1));
        mvwprintw(win, 3, (max_x / 5) * 2 - 15, "UP--<DIR>");
        mvwprintw(win, 3, (max_x / 5) * 2 - 3, "");
        if (*cursor_y == 3) {
            wattroff(win, A_REVERSE);
        }
    }

    int max_visible_files = max_y - 7;
    int start_index = 0;
    if (*cursor_y > max_visible_files + 2) {
        start_index = *cursor_y - max_visible_files - 2;
    }

    for (int i = start_index; i < num_files && i < start_index + max_visible_files; i++) {
        int offset = 0;
        if (strcmp(path, "/") != 0) {
            offset = 1;
        }
        int display_index = i - start_index + 3 + offset;

        char size_str[20];
        if (S_ISDIR(files_info[i].mode)) {
            wattron(win, COLOR_PAIR(1));
            snprintf(size_str, sizeof(size_str), "<DIR>");
        } else {
            wattroff(win, COLOR_PAIR(1));
            snprintf(size_str, sizeof(size_str), "%lld", files_info[i].size);
        }

        if (i + 3 + offset == *cursor_y) {
            wattron(win, A_REVERSE);
            mvwprintw(win, display_index, 2, "%s", files_info[i].name);
            mvwprintw(win, display_index, (max_x / 5) * 2 - 15, "%s", size_str);
            char date_str[20];
            if (strftime(date_str, sizeof(date_str), "%d/%m/%y %H:%M", localtime(&files_info[i].modify_time)) == 0) {
                strcpy(date_str, "ERROR");
            }
            mvwprintw(win, display_index, (max_x / 5) * 2 - 3, "%s", date_str);
            wattroff(win, A_REVERSE);
        } else {
            mvwprintw(win, display_index, 2, "%s", files_info[i].name);
            mvwprintw(win, display_index, (max_x / 5) * 2 - 15, "%s", size_str);
            char date_str[20];
            if (strftime(date_str, sizeof(date_str), "%d/%m/%y %H:%M", localtime(&files_info[i].modify_time)) == 0) {
                strcpy(date_str, "ERROR");
            }
            mvwprintw(win, display_index, (max_x / 5) * 2 - 3, "%s", date_str);
        }
    }

    mvprintw(max_y - 1, 0, "A - add file, S - add directory, D - delete, C - copy, M - move, R - rename, Q - quit");
    displayDiskSpace("/");

    free(files_info);
    wrefresh(win);
}

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

int main() {
    char path1[MAX_PATH];
    char path2[MAX_PATH];

    initscr();
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    WINDOW *win1 = newwin(max_y - 4, max_x / 2 - 2, 2, 2);
    WINDOW *win2 = newwin(max_y - 4, max_x / 2 - 2, 2, max_x / 2 + 2);

    int ch;
    int active_win = 1;
    int cursor_y1 = 3;
    int cursor_y2 = 3;
    getcwd(path1, MAX_PATH);
    getcwd(path2, MAX_PATH);

    int num_files1, num_files2;
    num_files1 = count_files(path1);
    num_files2 = count_files(path2);

    draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
    draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);

    while ((ch = getch()) != 'q') {
        switch (ch) {
            case '\t':
                active_win = (active_win == 1) ? 2 : 1;
                break;
            case KEY_UP:
                if (active_win == 1 && cursor_y1 > 3) {
                    cursor_y1--;
                    draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                } else if (active_win == 2 && cursor_y2 > 3) {
                    cursor_y2--;
                    draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                }
                break;
            case KEY_DOWN:
                if (active_win == 1 && cursor_y1 < num_files1 + 3) {
                    cursor_y1++;
                    draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                } else if (active_win == 2 && cursor_y2 < num_files2 + 3) {
                    cursor_y2++;
                    draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                }
                break;
            case '\n':
                if (active_win == 1) {
                    if (cursor_y1 == 3 && strcmp(path1, "/") != 0) {
                        char *last_slash = strrchr(path1, '/');
                        if (last_slash != NULL) {
                            *last_slash = '\0';
                        }
                        chdir(path1);
                        getcwd(path1, MAX_PATH);
                        num_files1 = count_files(path1);
                        draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                    } else {
                        char *selected_file = get_filename_by_index(path1, cursor_y1 - 4);
                        if (selected_file != NULL) {
                            char full_path[MAX_PATH];
                            snprintf(full_path, sizeof(full_path), "%s/%s", path1, selected_file);

                            struct stat file_stat;
                            if (stat(full_path, &file_stat) == 0) {
                                if (S_ISDIR(file_stat.st_mode)) {
                                    chdir(full_path);
                                    getcwd(path1, MAX_PATH);
                                    num_files1 = count_files(path1);
                                    cursor_y1 = 3;
                                    draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                                } else {
                                    open_file(full_path);
                                }
                            } else {
                                perror("stat");
                            }
                            free(selected_file);
                        }
                    }
                } else {
                    if (cursor_y2 == 3 && strcmp(path2, "/") != 0) {
                        char *last_slash = strrchr(path2, '/');
                        if (last_slash != NULL) {
                            *last_slash = '\0';
                        }
                        chdir(path2);
                        getcwd(path2, MAX_PATH);
                        num_files2 = count_files(path2);
                        draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                    } else {
                        char *selected_file = get_filename_by_index(path2, cursor_y2 - 4);
                        if (selected_file != NULL) {
                            char full_path[MAX_PATH];
                            snprintf(full_path, sizeof(full_path), "%s/%s", path2, selected_file);

                            struct stat file_stat;
                            if (stat(full_path, &file_stat) == 0) {
                                if (S_ISDIR(file_stat.st_mode)) {
                                    chdir(full_path);
                                    getcwd(path2, MAX_PATH);
                                    num_files2 = count_files(path2);
                                    cursor_y2 = 3;
                                    draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                                } else {
                                    open_file(full_path);
                                }
                            } else {
                                perror("stat");
                            }
                            free(selected_file);
                        }
                    }
                }
                break;
            case 'a':
                if (active_win == 1) {
                    create_file(path1);
                } else {
                    create_file(path2);
                }
                num_files1 = count_files(path1);
                num_files2 = count_files(path2);
                draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                break;
            case 's':
                if (active_win == 1) {
                    create_directory(path1);
                } else {
                    create_directory(path2);
                }
                num_files1 = count_files(path1);
                num_files2 = count_files(path2);
                draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                break;
            case 'd':
                if (active_win == 1) {
                    char *selected_file = get_filename_by_index(path1, cursor_y1 - 4);
                    if (selected_file != NULL) {
                        delete_file_or_directory(path1, selected_file);
                        free(selected_file);
                    }
                } else {
                    char *selected_file = get_filename_by_index(path2, cursor_y2 - 4);
                    if (selected_file != NULL) {
                        delete_file_or_directory(path2, selected_file);
                        free(selected_file);
                    }
                }
                num_files1 = count_files(path1);
                num_files2 = count_files(path2);
                draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                break;
            case 'm':
                if (active_win == 1) {
                    char *selected_file = get_filename_by_index(path1, cursor_y1 - 4);
                    if (selected_file != NULL) {
                        if (confirm_move(path2, selected_file)) {
                            move_file_or_directory(path1, path2, selected_file);
                        }
                        free(selected_file);
                    }
                } else {
                    char *selected_file = get_filename_by_index(path2, cursor_y2 - 4);
                    if (selected_file != NULL) {
                        if (confirm_move(path1, selected_file)) {
                            move_file_or_directory(path2, path1, selected_file);
                        }
                        free(selected_file);
                    }
                }
                num_files1 = count_files(path1);
                num_files2 = count_files(path2);
                draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                break;
            case 'c':
                if (active_win == 1) {
                    char *selected_file = get_filename_by_index(path1, cursor_y1 - 4);
                    if (selected_file != NULL) {
                        if (confirm_copy(path2, selected_file)) {
                            copy_file_or_directory(path1, path2, selected_file);
                        }
                        free(selected_file);
                    }
                } else {
                    char *selected_file = get_filename_by_index(path2, cursor_y2 - 4);
                    if (selected_file != NULL) {
                        if (confirm_copy(path1, selected_file)) {
                            copy_file_or_directory(path2, path1, selected_file);
                        }
                        free(selected_file);
                    }
                }
                num_files1 = count_files(path1);
                num_files2 = count_files(path2);
                draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                break;
            case 'r':
                if (active_win == 1) {
                    char *selected_file = get_filename_by_index(path1, cursor_y1 - 4);
                    if (selected_file != NULL) {
                        char new_name[MAX_PATH];
                        edit_name(new_name);
                        if (strlen(new_name) > 0) {
                            rename_file_or_directory(path1, selected_file, new_name);
                            num_files1 = count_files(path1);
                            draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                        }
                        free(selected_file);
                    }
                } else {
                    char *selected_file = get_filename_by_index(path2, cursor_y2 - 4);
                    if (selected_file != NULL) {
                        char new_name[MAX_PATH];
                        edit_name(new_name);
                        if (strlen(new_name) > 0) {
                            rename_file_or_directory(path2, selected_file, new_name);
                            num_files2 = count_files(path2);
                            draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                        }
                        free(selected_file);
                    }
                }
                num_files1 = count_files(path1);
                num_files2 = count_files(path2);
                draw_file_list(win1, path1, &cursor_y1, num_files1, max_x, max_y);
                draw_file_list(win2, path2, &cursor_y2, num_files2, max_x, max_y);
                break;
        }
    }

    endwin();
    return 0;
}
