#ifndef ALHAVA_OSISP_PRJ_2024_DISPLAY_H
#define ALHAVA_OSISP_PRJ_2024_DISPLAY_H

#include "file_manager.h"
#include <sys/statvfs.h>
#include <ncurses.h>
#include <ncurses.h>
#include <stdio.h>

void displayDiskSpace(const char *path);
void draw_file_list(WINDOW *win, const char *path, int *cursor_y, int num_files, int max_x, int max_y);

#endif //ALHAVA_OSISP_PRJ_2024_DISPLAY_H
