/*--- includes ---*/
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



/*--- init ---*/
int main() {
    enable_raw_mode();

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        if (iscntrl(c)) { // check if c is a control char
            printf("%d\r\n", c); // format byte c as decimal
        } else {
            printf("%d ('%c')\r\n", c, c); // format byte c as character
        }
        if (c == CTRL_KEY('q')) break;
    }

    return 0;
}
