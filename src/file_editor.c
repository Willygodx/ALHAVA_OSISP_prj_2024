#include "display.h"
#include "file_manager.h"
#include "file_operations.h"

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
