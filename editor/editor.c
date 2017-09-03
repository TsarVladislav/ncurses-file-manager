#include "editor.h"

int main(int argc, char *argv[])
{
    int ch;
    int c;
    int i;
    FILE *fp;
    int y, x;
    char str[1024];
    int n;
    int len;
    int *sizes = NULL;
    len = 0;
    n = 0;

    if(argc != 2){
        printf("Как использовать: %s имя_файла\n", argv[0]);
        return 1;
    }
    fp = fopen(argv[1], "r+");
    if(fp == NULL){
        fprintf(stderr,"Не могу открыть файл\n");
        exit(1);
    }

    initscr();
    keypad(stdscr, TRUE);
    curs_set(2);

    while((ch = fgetc(fp)) != EOF){
        getyx(stdscr, y, x);
        if(ch == '\n'){
            ++n;
            sizes = realloc(sizes, n * sizeof(int));
            sizes[n - 1] = len;
            len = 0;
        } else if(ch == '\t'){
            len += 8;
        } else {
            len++;
        }
        printw("%c", ch);
    }
    move(0, 0);
    y = x = 0;

    refresh();
    /* сделай так, чтобы скипал табы */
    while(( c = getch()) != KEY_F(1)){;
        switch(c){
            case KEY_UP:
                if(y != 0){
                    --y;
                }
                x = 0;
                move(y,x);
                instr(str);
                break;
            case KEY_DOWN:
                ++y;
                x = 0;
                move(y,x);
                instr(str);

                break;
            case KEY_LEFT:
                if(x != 0)
                    --x;
                break;
            case KEY_RIGHT:
                if(x < sizes[y ])
                    ++x;
                break;
            case KEY_F(8):
                fseek(fp, 0, SEEK_SET);
                for(i = 0; i < n; i++){
                    move(i,0);
                    instr(str);
                    fprintf(fp, "%s", str);
                }
                fclose(fp);
                break;
            default:

                break;
            break;
        }
            print_pos(x, sizes[y]);
            move(y,x);
            refresh();
    }
    endwin();
    free(sizes);
    return 0;
}
void print_pos(int x, int n)
{
    move(LINES - 1,0);
    clrtoeol();
    mvwprintw(stdscr, LINES - 1, COLS - 10, "%d/%d\n",x, n );
}


/* удаляет пустоту в конце строки. */
void trim(char * s) {
    char *p = s;
    int l = strlen(p);

    while(p[l - 1] == ' ')
        p[--l] = 0;
    memmove(s, p, l + 1);
}
