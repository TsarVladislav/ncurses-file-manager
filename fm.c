#include "fm.h"
static struct tab tb[MAX_TABS];
int main()
{
    struct pmessage ms;
    int cur_win = 0;
    char cwd[PATH_MAX];
    pid_t editor_pid;
    int c;
    int ret;
    int in_fd, out_fd, rd_count, wt_count;
    char buffer[BUF_SIZE];
    pthread_t thread;
    init_screen();
    refresh();
    create_window(cur_win, COLS/2 - 1, LINES -5, 1, 1);
    create_window(1, COLS/2, LINES -5, COLS/2 + 1, 1);
    print_menu(cur_win);
    print_menu(1);
    while(( c = wgetch(tb[cur_win].win.fm)) != KEY_F(1)){;
            switch(c){
                case KEY_UP:
                    scroll_up(cur_win);
                    break;
                case KEY_DOWN:
                    scroll_down(cur_win);
                    break;
                case 10:
                    if(isdir(tb[cur_win].files[tb[cur_win].cur_pos - 1])){
                        chdir(tb[cur_win].files[tb[cur_win].cur_pos - 1]);
                        getcwd(cwd, sizeof(cwd));
                        strncpy(tb[cur_win].cwd, cwd, strlen(cwd)+1);
                        list_files(cur_win);
                        clrtoeol();
                    } else {
                        editor_pid=fork();
                        if (editor_pid==0) { /* child process */
                            static char *argv[3];
                            wmove(stdscr,LINES -3, 0);
                            argv[0]="/home/vlad/Eltex/11.07.17/failmanager/editor/editor";
                            argv[1]=tb[cur_win].files[tb[cur_win].cur_pos - 1];
                            argv[2] = NULL;
                            execv("/home/vlad/Eltex/11.07.17/failmanager/editor/editor",argv);
                            exit(127); /* only if execv fails */
                        } else { /* pid!=0; parent process */
                            waitpid(editor_pid,0,0); /* wait for child to exit */
                            init_screen();
                            refresh();
                        }
                    }
                    break;
                case KEY_F(5):
                    if(!isdir(tb[cur_win].files[tb[cur_win].cur_pos - 1])){
                        in_fd = open(tb[cur_win].files[tb[cur_win].cur_pos - 1], O_RDONLY);
                        if(in_fd < 0)
                            end_screen();

                        sprintf(ms.dest, "%s", tb[(cur_win + 1)%MAX_TABS].cwd);
                        strcat(ms.dest, "/");
                        strcat(ms.dest, tb[cur_win].files[tb[cur_win].cur_pos - 1]);

                        sprintf(ms.origin, "%s", tb[cur_win].cwd);
                        strcat(ms.origin, "/");
                        strcat(ms.origin, tb[cur_win].files[tb[cur_win].cur_pos - 1]);

                        out_fd = creat(ms.dest, OUTPUT_MODE);
                        if(out_fd < 0)
                            end_screen();

                        ret = pthread_create(&thread, NULL, copy_message, &ms);
                        if(ret){
                            fprintf(stderr, "Ошибка - pthread_create() return code -- %d\n", ret);
                            end_screen();
                        }
                        while(1){
                            rd_count = read(in_fd, buffer, BUF_SIZE);
                            if(rd_count <= 0)
                                break;
                            wt_count = write(out_fd, buffer, rd_count);
                            if(wt_count <=0)
                                end_screen();
                        }
                        close(in_fd);
                        close(out_fd);
                        if(rd_count != 0){
                            end_screen();
                        }
                    }
                    pthread_join( thread, NULL);
                    chdir(tb[(cur_win + 1)% MAX_TABS].cwd);
                    list_files((cur_win + 1)% MAX_TABS);
                    print_menu((cur_win + 1) % MAX_TABS);
                    chdir(tb[cur_win].cwd);
                    list_files(cur_win);
                    break;
                case 9:
                    cur_win = (cur_win + 1) % MAX_TABS;
                    chdir(tb[cur_win].cwd);
                    list_files(cur_win);
                break;
                default:
                    refresh();
                    break;
            }
        print_menu(cur_win);
        refresh();
    }
    end_screen();
    return 0;
}

void scroll_down(int win)
{
    if(tb[win].cur_pos == tb[win].number_of_files)
        tb[win].cur_pos = 1;
    else
        ++tb[win].cur_pos;
}
void scroll_up(int win)
{
    if(tb[win].cur_pos == 1)
        tb[win].cur_pos = tb[win].number_of_files;
    else
        --tb[win].cur_pos;
}

void init_screen()
{
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    use_default_colors();
    noecho();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_CYAN, -1);
    init_pair(3, COLOR_GREEN, -1);
    keypad(stdscr, TRUE);
    curs_set(0);
    raw();
    wmove(stdscr, LINES - 1, 0);
    wattron(stdscr,A_REVERSE); 
    wprintw(stdscr, "выход на F1\n");
    wattroff(stdscr,A_REVERSE); 
}

void end_screen()
{
    endwin();
}

void create_window(int cur_win, int width, int height, int x, int y)
{
    char cwd[PATH_MAX];
    sprintf(tb[cur_win].cwd,"%s", getcwd(cwd, sizeof(cwd)));
    tb[cur_win].number_of_files = 0;
    chdir(tb[cur_win].cwd);
    list_files(cur_win);
    tb[cur_win].win.fm = newwin(height, width, y, x);
    tb[cur_win].win.width = width;;
    tb[cur_win].win.height = height;;
    keypad(tb[cur_win].win.fm, TRUE);

}

/* получаю список файлов в директории */
void list_files(int cur_win)
{
    struct dirent **files;
    int i;

    if(tb[cur_win].number_of_files)
        free(tb[cur_win].files);
    /* сделай нормальную сортировку */
    tb[cur_win].number_of_files = scandir(tb[cur_win].cwd, &files, NULL, versionsort);
    tb[cur_win].files = malloc(tb[cur_win].number_of_files * PATH_MAX);

    if(!tb[cur_win].files){
        fprintf(stderr, "Нет памяти...\n");
        /* сделать более умное завершение программы в таком случае */
        end_screen();
        exit(1);
    }

    for(i = 0 ; i < tb[cur_win].number_of_files; i++){
        strncpy(tb[cur_win].files[i], files[i]->d_name, strlen(files[i]->d_name)+1);
        free(files[i]);
    }
    free(files);
    tb[cur_win].cur_pos = 1;
}
void print_menu(int cur_win)
{


    int x, y, i;
    static int start = 0;
    x = 2;
    y = 2;
    wclear(tb[cur_win].win.fm);
    box(tb[cur_win].win.fm, 0, 0);
    /* вынести в функцию */
    /* сделать нормальный скролл */
    if(tb[cur_win].cur_pos < tb[cur_win].win.height - 2)
        start = 0;
    else
        start = tb[cur_win].cur_pos - 1-  tb[cur_win].cur_pos % (tb[cur_win].win.height - 2); 

    /* выводи только часть строк */
    for(i = start; i < start + tb[cur_win].win.height -3 && i < tb[cur_win].number_of_files; ++i){
        if(tb[cur_win].cur_pos == i + 1){
            wattron(tb[cur_win].win.fm, COLOR_PAIR(1)); 
            mvwprintw(tb[cur_win].win.fm, y, x, "%s", tb[cur_win].files[i]);
            wattroff(tb[cur_win].win.fm, COLOR_PAIR(1));
        }else{
            if(isdir(tb[cur_win].files[i])){
                wattron(tb[cur_win].win.fm, COLOR_PAIR(2)); 
                mvwprintw(tb[cur_win].win.fm, y, x, "%s", tb[cur_win].files[i]);
                wattroff(tb[cur_win].win.fm, COLOR_PAIR(2));
            } else {
                mvwprintw(tb[cur_win].win.fm, y, x, "%s", tb[cur_win].files[i]);
            }
        }
        ++y;
    }
    wrefresh(tb[cur_win].win.fm);
}

int isdir(const char *path)
{
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

/* сообщение при копировании */
void *copy_message(void *mss)
{
    size_t size = 1;
    size_t prsize = -1;
    struct pmessage *ms = (struct pmessage *) mss;
    struct stat st;
    stat(ms->origin, &st);
    size = st.st_size;
    while(prsize != size){
        usleep(100);
        move(LINES - 2,0);
        clrtoeol();
        stat(ms->dest, &st);
        prsize = st.st_size;
        wattron(stdscr, COLOR_PAIR(3)); 
        mvwprintw(stdscr,LINES -2, 0, " %d%% ", 100 * prsize/size);
        wprintw(stdscr, "[%.*s", 10 * prsize/size, "||||||||||");
        mvwprintw(stdscr,LINES -2, 16, "]\n");
        wattroff(stdscr, COLOR_PAIR(3));
        refresh();
    }
    move(LINES - 2,0);
    clrtoeol();
    refresh();
    return 0;
}
