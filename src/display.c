#include "display.h"

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
