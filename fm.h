#define _GNU_SOURCE
#include <ncurses.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#define MAX_TABS 2
#define BUF_SIZE 4096
#define OUTPUT_MODE 0700


struct winprop{
    WINDOW *fm;
    int width;
    int height;
};
struct tab{
    struct winprop win;
    char cwd[PATH_MAX + 1];
    char (*files)[PATH_MAX + 1];
    int cur_pos;
    int number_of_files;
};

struct pmessage{
    char origin[PATH_MAX];
    char dest[PATH_MAX];
};
void *copy_message(void *);

void scroll_down(int win);
void scroll_up(int win);
void init_screen();
void end_screen();
void create_window(int cur_win, int width, int height, int x, int y);
void list_files(int cur_win);
void print_menu(int cur_win);
int isdir(const char *path);
