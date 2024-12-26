/*--- includes ---*/
#include <_stdio.h>
#include <errno.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/ttycom.h>
#include <termios.h>
#include <unistd.h>



/*--- defines ---*/
#define CTRL_KEY(k) ((k) & 0x1f) // 0001 1111



/*--- data ---*/
struct editor_config {
    int screenrows;
    int screencols;
    struct termios original_termios; // original terminal io settings
};
struct editor_config E;




/*--- terminal --- */
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.original_termios) == -1)
        die("tcsetattr");
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &E.original_termios) == -1)
        die("tcgetattr");
    atexit(disable_raw_mode); // disable raw mode on exit

    struct termios raw = E.original_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); 
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
 
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

char editor_read_key() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

int get_cursor_position(int *rows, int *cols) {
    if (write(STDIN_FILENO, "\x1b[6n", 4) != 4) return -1;

    printf("\r\n");
    char c;

    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d, ('%c')\r\n", c, c);
        }
    }
    editor_read_key();

    return -1;
}


int get_window_size(int *rows, int *cols) {
    struct winsize ws;
    
    if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        /* fallback solution: move cursor 999 down (C) and 999 right (B) to get the size of the window */
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12)  != 12) return -1;
        return get_cursor_position(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*--- output ---*/
void editor_draw_rows() {
    int y;
    for (y = 0; y < E.screenrows; ++y) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editor_refresh_screen() {
    write(STDOUT_FILENO, "\x1b[2J", 4); // "\x1b" is the escape char (27)
    write(STDOUT_FILENO, "\x1b[H", 3); // H: cursor position, default 1;1H

    editor_draw_rows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}


/*--- input ---*/

void editor_process_keypress() {
    char c = editor_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

/*--- init ---*/
void init_editor() {
    if (get_window_size(&E.screenrows, &E.screencols) == -1) die("get_window_size");
}

int main() {
    enable_raw_mode();
    init_editor();

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }

    return 0;
}
