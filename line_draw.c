#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

struct vec {
    int x;
    int y;
};

struct vec vec_add(struct vec v1, struct vec v2) {
    struct vec v_result = {v1.x + v2.x, v1.y + v2.y};
    return v_result;
}

struct vec vec_diff(struct vec p1, struct vec p2) {
    struct vec p_result = {p1.x - p2.x, p1.y - p2.y};
    return p_result;
}

//turndir =  1 -> clockwise
//turndir = -1 -> counter-clockwise
struct vec turn(struct vec cur_dir, int turn_dir) {
    if (cur_dir.x == 0) {
        cur_dir.x = turn_dir * cur_dir.y;
        cur_dir.y = 0;
    }
    else {
        cur_dir.y = turn_dir * cur_dir.x;
        cur_dir.x = 0;
    }
    return cur_dir;
}

struct trailer {
    struct vec pos;
    struct vec dir;
    struct vec prev_dir;
};

int max_x, max_y; //GLOBAL
void move_trailer(struct trailer *t) {
    wchar_t box_draws[3][3] =
    {
        {ACS_ULCORNER, ACS_BLOCK, ACS_URCORNER},
        {ACS_BLOCK,    ACS_BLOCK, ACS_BLOCK},
        {ACS_LLCORNER, ACS_BLOCK, ACS_LRCORNER},
    };
    int random_choice = rand() % 100;
    if (random_choice > 90){
        random_choice = rand() % 2;
        int turn_dir = ((double) (random_choice) / 2 - 0.25) * 4;
        t->dir = turn(t->dir, turn_dir);
    }
    if (t->dir.x == 0)
        mvaddch(t->pos.x, t->pos.y, ACS_HLINE);
    else
        mvaddch(t->pos.x, t->pos.y, ACS_VLINE);
    if (t->prev_dir.x != t->dir.x && t->prev_dir.x != -t->dir.x) {
        //set the previous character to the appropriate corner piece
        struct vec sussy_diff = vec_diff(t->prev_dir, t->dir);
        mvaddch(t->pos.x, t->pos.y, box_draws[1 + sussy_diff.x][1 + sussy_diff.y]);
    }
    t->prev_dir = t->dir;
 
    t->pos = vec_add(t->pos, t->dir);
    if (t->pos.x < 0) {
        t->pos.x = max_x - 1;
    }
    if (t->pos.y < 0) {
        t->pos.y = max_y - 1;
    }
    t->pos.x = t->pos.x % max_x;
    t->pos.y = t->pos.y % max_y;
}

int main() {
    setlocale(LC_CTYPE, "");
    WINDOW *W = initscr();
    cbreak();
    noecho();
    intrflush(W, FALSE);
    nodelay(W, TRUE);
    clear();

    int prev_curs = curs_set(0); //invisible cursor
    if (prev_curs == ERR) {
        fprintf(stderr, "%s", "Your terminal doesn't support invisible cursors!");
    }
   
    struct trailer all_trailers[5];
    for (size_t i = 0; i < sizeof(all_trailers) / sizeof(struct trailer); i++) {
        all_trailers[i].pos = (struct vec){0, 0};
        all_trailers[i].dir = (struct vec){0, 1};
        all_trailers[i].prev_dir = all_trailers[i].dir;
    }
    srand(12);
    while(true) {
        refresh();
        getmaxyx(W, max_x, max_y);
        for (size_t i = 0; i < sizeof(all_trailers) / sizeof(struct trailer); i++) {
            move_trailer(&all_trailers[i]);
        }
        napms(50); // wait
    }
    curs_set(prev_curs);
    endwin();
    return EXIT_SUCCESS;
}

