// texteffects.c: Implementation of text effect library for SilverMUD.
// Barry Kane, 2021.
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

void slowPrint(char * stringToPrint, int delay)
{
	int characterIndex = 0;
	while(stringToPrint[characterIndex] != '\0')
	{
		putchar(stringToPrint[characterIndex]);
		// Flush the buffer so there's no line buffering.
		fflush(stdout);
		usleep(delay);
		characterIndex++;
	}
}

void slowPrintNcurses(char * stringToPrint, int delay, WINDOW * window)
{
	int characterIndex = 0;
	while(stringToPrint[characterIndex] != '\0')
	{
		waddch(window, stringToPrint[characterIndex]);
		// Refresh the ncurses screen.
		wrefresh(window);
		usleep(delay);
		characterIndex++;
	}
	wrefresh(window);
}
