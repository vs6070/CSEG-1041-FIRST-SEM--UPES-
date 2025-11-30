/*
 * snake_game_noflicker.c
 *
 * Snake game (terminal) with reduced flicker and Windows SDK fallback for
 * ENABLE_VIRTUAL_TERMINAL_PROCESSING.
 *
 * Controls: W/A/S/D or arrow keys. Press Q to quit.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
  #define PLATFORM_WINDOWS 1
  #include <windows.h>
  #include <conio.h>

  #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
  #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
  #endif

#else
  #define PLATFORM_POSIX 1
  #include <termios.h>
  #include <unistd.h>
  #include <sys/select.h>
#endif


#define BOARD_WIDTH 40
#define BOARD_HEIGHT 20
#define FRAME_TIME_MS 120
#define MAX_SNAKE_LENGTH (BOARD_WIDTH * BOARD_HEIGHT)

#define CHAR_BORDER_HORIZONTAL '-'
#define CHAR_BORDER_VERTICAL   '|'
#define CHAR_SNAKE_HEAD       'O'
#define CHAR_SNAKE_BODY       'o'
#define CHAR_FOOD             '*'
#define CHAR_EMPTY            ' '

typedef enum { DIR_UP=0, DIR_RIGHT, DIR_DOWN, DIR_LEFT } Direction;
typedef struct { int x, y; } Point;
typedef struct {
    Point snake[MAX_SNAKE_LENGTH];
    int snake_len;
    Direction dir;
    Point food;
    int score;
    int game_over;
} GameState;



#if defined(PLATFORM_POSIX)
static struct termios orig_termios;


void enable_raw_mode(void) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}
int kbhit_posix(void) {
    fd_set readfds;
    struct timeval tv = {0,0};
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    return select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv) > 0;
}
int getch_posix(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) <= 0) return -1;
    return c;
}
#endif


void sleep_ms(int ms) {
#if defined(PLATFORM_WINDOWS)
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}


void enable_ansi_windows(void) {
#if defined(PLATFORM_WINDOWS)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}


int rand_int(int min, int max) {
    if (min > max) { int t = min; min = max; max = t; }
    return (rand() % (max - min + 1)) + min;
}

/* ---------------- Game logic ---------------- */

void init_game(GameState *g) {
    g->snake_len = 3;
    int sx = BOARD_WIDTH / 4;
    int sy = BOARD_HEIGHT / 2;
    g->snake[0].x = sx + 2; g->snake[0].y = sy;
    g->snake[1].x = sx + 1; g->snake[1].y = sy;
    g->snake[2].x = sx + 0; g->snake[2].y = sy;
    g->dir = DIR_RIGHT;
    g->score = 0;
    g->game_over = 0;

    int fx, fy, on_snake, i;
    do {
        fx = rand_int(1, BOARD_WIDTH);
        fy = rand_int(1, BOARD_HEIGHT);
        on_snake = 0;
        for (i = 0; i < g->snake_len; ++i)
            if (g->snake[i].x == fx && g->snake[i].y == fy) { on_snake = 1; break; }
    } while (on_snake);
    g->food.x = fx; g->food.y = fy;
}

int snake_contains(const GameState *g, int x, int y, int ignore_head) {
    int start = ignore_head ? 1 : 0;
    for (int i = start; i < g->snake_len; ++i)
        if (g->snake[i].x == x && g->snake[i].y == y) return 1;
    return 0;
}

void place_food(GameState *g) {
    int fx, fy, attempts = 0;
    do {
        fx = rand_int(1, BOARD_WIDTH);
        fy = rand_int(1, BOARD_HEIGHT);
        attempts++;
        if (attempts > 10000) break;
    } while (snake_contains(g, fx, fy, 0));
    g->food.x = fx; g->food.y = fy;
}

void update_direction(GameState *g, int ch) {
    if (ch == 'w' || ch == 'W') { if (g->dir != DIR_DOWN) g->dir = DIR_UP; }
    else if (ch == 's' || ch == 'S') { if (g->dir != DIR_UP) g->dir = DIR_DOWN; }
    else if (ch == 'a' || ch == 'A') { if (g->dir != DIR_RIGHT) g->dir = DIR_LEFT; }
    else if (ch == 'd' || ch == 'D') { if (g->dir != DIR_LEFT) g->dir = DIR_RIGHT; }
    else if (ch == 'q' || ch == 'Q') g->game_over = 1;
}

void game_step(GameState *g) {
    Point new_head = g->snake[0];
    if (g->dir == DIR_UP) new_head.y -= 1;
    else if (g->dir == DIR_DOWN) new_head.y += 1;
    else if (g->dir == DIR_LEFT) new_head.x -= 1;
    else if (g->dir == DIR_RIGHT) new_head.x += 1;

    if (new_head.x < 1 || new_head.x > BOARD_WIDTH || new_head.y < 1 || new_head.y > BOARD_HEIGHT) {
        g->game_over = 1; return;
    }
    if (snake_contains(g, new_head.x, new_head.y, 0)) { g->game_over = 1; return; }

    for (int i = g->snake_len - 1; i > 0; --i) g->snake[i] = g->snake[i - 1];
    g->snake[0] = new_head;

    if (g->snake[0].x == g->food.x && g->snake[0].y == g->food.y) {
        if (g->snake_len < MAX_SNAKE_LENGTH) {
            g->snake[g->snake_len] = g->snake[g->snake_len - 1];
            g->snake_len++;
        }
        g->score += 1;
        place_food(g);
    }
}

/* ---------------- Input handling ---------------- */

int handle_input(GameState *g) {
#if defined(PLATFORM_WINDOWS)
    if (!_kbhit()) return 0;
    int c = _getch();
    if (c == 0 || c == 0xE0) {
        int c2 = _getch();
        if (c2 == 72) update_direction(g, 'w'); /* up */
        else if (c2 == 80) update_direction(g, 's'); /* down */
        else if (c2 == 75) update_direction(g, 'a'); /* left */
        else if (c2 == 77) update_direction(g, 'd'); /* right */
    } else update_direction(g, c);
    return 1;
#else
    if (!kbhit_posix()) return 0;
    int c = getch_posix();
    if (c == 27) { /* ESC sequence */
        if (!kbhit_posix()) return 1;
        int c2 = getch_posix(); if (c2 != 91) return 1;
        if (!kbhit_posix()) return 1;
        int c3 = getch_posix();
        if (c3 == 'A') update_direction(g, 'w');
        else if (c3 == 'B') update_direction(g, 's');
        else if (c3 == 'C') update_direction(g, 'd');
        else if (c3 == 'D') update_direction(g, 'a');
    } else update_direction(g, c);
    return 1;
#endif
}


void hide_cursor(void) { printf("\x1b[?25l"); fflush(stdout); }
void show_cursor(void) { printf("\x1b[?25h"); fflush(stdout); }

void render_frame(const GameState *g) {
    int line_len = BOARD_WIDTH + 2 + 1;
    int lines = BOARD_HEIGHT + 2 + 2;
    int bufsize = (line_len * lines) + 256;
    char *buf = (char*)malloc(bufsize);
    if (!buf) return;

    char *p = buf;
    /* Move cursor home */
    *p++ = '\x1b'; *p++ = '['; *p++ = 'H';

    /* Top border */
    *p++ = '+';
    for (int x = 0; x < BOARD_WIDTH; ++x) *p++ = CHAR_BORDER_HORIZONTAL;
    *p++ = '+';
    *p++ = '\n';

    /* Board */
    for (int y = 1; y <= BOARD_HEIGHT; ++y) {
        *p++ = CHAR_BORDER_VERTICAL;
        for (int x = 1; x <= BOARD_WIDTH; ++x) {
            char ch = CHAR_EMPTY;
            if (g->food.x == x && g->food.y == y) ch = CHAR_FOOD;
            else if (g->snake[0].x == x && g->snake[0].y == y) ch = CHAR_SNAKE_HEAD;
            else {
                for (int i = 1; i < g->snake_len; ++i)
                    if (g->snake[i].x == x && g->snake[i].y == y) { ch = CHAR_SNAKE_BODY; break; }
            }
            *p++ = ch;
        }
        *p++ = CHAR_BORDER_VERTICAL;
        *p++ = '\n';
    }

    /* Bottom border */
    *p++ = '+';
    for (int x = 0; x < BOARD_WIDTH; ++x) *p++ = CHAR_BORDER_HORIZONTAL;
    *p++ = '+';
    *p++ = '\n';

    /* Info lines */
    int n = snprintf(p, bufsize - (p - buf), "Score: %d\tLength: %d\nControls: W/A/S/D or Arrow keys. Press Q to quit.\n",
                     g->score, g->snake_len);
    if (n > 0) p += n;

    fwrite(buf, 1, p - buf, stdout);
    fflush(stdout);
    free(buf);
}

/* ---------------- Main ---------------- */
int main(void) {
    srand((unsigned int) time(NULL));
    enable_ansi_windows();

    GameState game;
    init_game(&game);

#if defined(PLATFORM_POSIX)
    enable_raw_mode();
#endif

    /* Clear once and hide cursor */
    printf("\x1b[2J");
    hide_cursor();

    printf("SNAKE GAME (noflicker). Press Enter to start...");
    fflush(stdout);
    getchar();

    while (!game.game_over) {
        handle_input(&game);
        game_step(&game);
        render_frame(&game);
        sleep_ms(FRAME_TIME_MS);
    }

    
    show_cursor();
#if defined(PLATFORM_POSIX)
    disable_raw_mode();
#endif

    
    printf("\nGAME OVER!\nFinal Score: %d\nFinal Length: %d\nThank you for playing.\n",
           game.score, game.snake_len);
    fflush(stdout);
    return 0;
    }
