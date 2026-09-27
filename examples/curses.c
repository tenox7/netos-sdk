/* curses: a box, the keys you press, q quits */
#include <curses.h>

int main(void)
{
	int c, n = 0;

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	box(stdscr, 0, 0);
	mvprintw(1, 2, "netOS curses, %dx%d %s - press keys, q quits", COLS, LINES, termname());
	refresh();
	while ((c = getch()) != 'q') {
		mvprintw(3 + n++ % (LINES - 4), 2, "%-16s", keyname(c));
		refresh();
	}
	endwin();
	return 0;
}
