#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios original_termios; // original terminal io settings

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_termios);
    atexit(disable_raw_mode); // disable raw mode on exit

    struct termios raw = original_termios;
    raw.c_lflag &= ~(IXON); // disables C-s and C-q
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); 
 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main() {
    enable_raw_mode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
        if (iscntrl(c)) { // check if c is a control char
            printf("%d\n", c); // format byte c as decimal
        } else {
            printf("%d ('%c')\n", c, c); // format byte c as character
        }
    }

    return 0;
}
