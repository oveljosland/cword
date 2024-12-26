/*--- includes ---*/
#include <_stdio.h>
#include <errno.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>



/*--- defines ---*/
#define CTRL_KEY(k) ((k) & 0x1f) // 0001 1111



/*--- data ---*/
struct termios original_termios; // original terminal io settings



/*--- terminal --- */
void die(const char *s) {
    perror(s);
    exit(1);
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
        die("tcsetattr");
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
        die("tcgetattr");
    atexit(disable_raw_mode); // disable raw mode on exit

    struct termios raw = original_termios;
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

void editor_process_keypress() {
    char c = editor_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}

/*--- init ---*/
int main() {
    enable_raw_mode();

    while (1) {
        editor_process_keypress();
    }

    return 0;
}
