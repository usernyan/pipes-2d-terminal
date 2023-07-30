#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

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

struct line_layout {
    wchar_t* strings_box[3][3];
};

struct line_layout bold_lines = {
    {
        {L"┏", L"━", L"┓"},
        {L"┃", L"█", L"┃"},
        {L"┗", L"━", L"┛"},
    }
};

struct line_layout double_lines = {
    {
        {L"╔", L"═", L"╗"},
        {L"║", L"▒", L"║"},
        {L"╚", L"═", L"╝"},
    }
};

struct trailer {
    struct vec pos;
    struct vec dir;
    struct vec prev_dir;
    int color_pair_idx;
    struct line_layout *layout;
};

bool term_has_colors = false; //GLOBAL
bool use_colors = true; //GLOBAL
int max_x, max_y; //GLOBAL
void move_trailer(struct trailer *t) {
    int random_choice = rand() % 100;
    if (random_choice > 90){
        random_choice = rand() % 2;
        int turn_dir = ((double) (random_choice) / 2 - 0.25) * 4;
        t->dir = turn(t->dir, turn_dir);
    }
    if (use_colors)
        attron(COLOR_PAIR(t->color_pair_idx));
    if (t->prev_dir.x != t->dir.x && t->prev_dir.x != -t->dir.x) {
        //set the previous character to the appropriate corner piece
        struct vec sussy_diff = vec_diff(t->prev_dir, t->dir);
        mvaddwstr(t->pos.x, t->pos.y, t->layout->strings_box[1 + sussy_diff.x][1 + sussy_diff.y]);
    }
    else if (t->dir.x == 0)
        mvaddwstr(t->pos.x, t->pos.y, t->layout->strings_box[0][1]);
    else
        mvaddwstr(t->pos.x, t->pos.y, t->layout->strings_box[1][0]);
    t->prev_dir = t->dir;
     if (use_colors)
        attroff(COLOR_PAIR(t->color_pair_idx));

    t->pos = vec_add(t->pos, t->dir);
    if (t->pos.x < 0)
        t->pos.x = max_x - 1;
    if (t->pos.y < 0)
        t->pos.y = max_y - 1;
    t->pos.x = t->pos.x % max_x;
    t->pos.y = t->pos.y % max_y;
}

int max(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}

void usage_exit() {
    fprintf(stderr, "Usage: pipes2d [-n] [number of pipes]");
    exit(EXIT_FAILURE);
}

//TODO: Make the count, color, and character set of pipes controllable through command-line arguments
int main(int argc, char *argv[]) {
    //argument parsing
    int opt;
    while ((opt = getopt(argc, argv, "n")) != -1) {
        switch (opt) {
            case 'n': use_colors = false; break;
            default:
                usage_exit();
        }
    }
    setlocale(LC_CTYPE, "");
    WINDOW *W = initscr();
    getmaxyx(W, max_x, max_y);

    int num_trailers = max_x * max_y / 1000;
    num_trailers = max(num_trailers, 5);
    if (optind < argc) {
        num_trailers = strtol(argv[optind], NULL, 10);
    }

    term_has_colors = has_colors();
    use_colors = use_colors && term_has_colors;
    int num_col_pairs = 8;
    if (use_colors) {
        start_color();
        //TODO: this should make black colors pairs leave a transparent bg, but it isn't. figure out why
        use_default_colors();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
        init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(4, COLOR_RED, COLOR_BLACK);
        init_pair(5, COLOR_YELLOW, COLOR_BLACK);
        init_pair(6, COLOR_GREEN, COLOR_BLACK);
        init_pair(7, COLOR_RED, COLOR_BLACK);
        init_pair(8, COLOR_BLUE, COLOR_BLACK);
    }
    cbreak();
    noecho();
    intrflush(W, FALSE);
    nodelay(W, TRUE);
    clear();

    int prev_curs = curs_set(0); //invisible cursor
    if (prev_curs == ERR) {
        fprintf(stderr, "%s", "Your terminal doesn't support invisible cursors!");
    }
   
    struct trailer *all_trailers = malloc(num_trailers * sizeof(struct trailer));
    for (size_t i = 0; i < num_trailers; i++) {
        struct trailer *t = &all_trailers[i];
        t->pos = (struct vec){0, 0};
        t->dir = (struct vec){0, 1};
        t->prev_dir = t->dir;
        t->color_pair_idx = i % num_col_pairs + 1;
        t->layout = &bold_lines;
    }
    srand(time(NULL));
    getmaxyx(W, max_x, max_y);
    //clear time is the amount of time needed for all snakes to cumulatively travel the entire area of the screen
    int cur_ticks = 0;
    int ticks_min = 100;
    wchar_t user_input;
    while(true) {
        user_input = getch();
        if (user_input == L'q') {
            break;
        }
        getmaxyx(W, max_x, max_y);
        cur_ticks++;
        int clear_ticks = ticks_min;
        if (num_trailers != 0)
            clear_ticks = max_x * max_y / num_trailers;
        clear_ticks = max(clear_ticks, ticks_min);
        if (cur_ticks >= clear_ticks) {
            clear();
            cur_ticks = 0;
        }
        for (size_t i = 0; i < num_trailers; i++) {
            move_trailer(&all_trailers[i]);
        }
        refresh();
        napms(50); // wait
    }
    free(all_trailers);
    //TODO: make sure this code runs even if we exit unexpectedly
    curs_set(prev_curs);
    endwin();
    return EXIT_SUCCESS;
}

