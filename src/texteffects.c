// texteffects.c: Implementation of text effect library for SilverMUD.
// Barry Kane, 2021.
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <ncurses.h>

// A character by character print, similar to a serial terminal with lower baud rate:
void slowPrint(const char * stringToPrint, int delay)
{
	int characterIndex = 0;
	while (stringToPrint[characterIndex] != '\0')
	{
		putchar(stringToPrint[characterIndex]);
		// Flush the buffer so there's no line buffering.
		fflush(stdout);
		usleep(delay);
		characterIndex++;
	}
}

// The same, altered to work with ncurses:
void slowPrintNcurses(const char * stringToPrint, int delay, WINDOW * window, bool bolded)
{
	int characterIndex = 0;
	if (bolded)
	{
		wattron(window, A_BOLD);
	}
	while (stringToPrint[characterIndex] != '\0')
	{
		waddch(window, stringToPrint[characterIndex]);
		// Refresh the ncurses screen.
		wrefresh(window);
		usleep(delay);
		characterIndex++;
	}
	if (bolded)
	{
		wattroff(window, A_BOLD);
	}
	wrefresh(window);
}

// A character by character "brute-force" print, similar to Hollywood hacking scenes:
void bruteforcePrint(const char * stringToPrint, int delay)
{
	unsigned int characterIndex = 0;
	while (stringToPrint[characterIndex] != '\0')
	{
		for(unsigned char currentCharacter = 32; currentCharacter <= stringToPrint[characterIndex]; currentCharacter++)
		{
			putchar(stringToPrint[currentCharacter]);
			fflush(stdout);
			usleep(delay);
			putchar(8);
			fflush(stdout);
		}
		putchar(stringToPrint[characterIndex]);
		characterIndex++;
	}
}

// The same, altered to work with ncurses:
void bruteforcePrintNcurses(const char * stringToPrint, int delay, WINDOW * window, bool bolded)
{
	int characterIndex = 0;
	if (bolded)
	{
		wattron(window, A_BOLD);
	}
	while (stringToPrint[characterIndex] != '\0')
	{
		for(char currentCharacter = 32; currentCharacter <= stringToPrint[characterIndex]; currentCharacter++)
		{
			waddch(window, currentCharacter);
			wrefresh(window);
			usleep(delay);
			waddch(window, 8);
			wrefresh(window);
		}
		waddch(window, stringToPrint[characterIndex]);
		characterIndex++;
	}
	if (bolded)
	{
		wattroff(window, A_BOLD);
	}
	wrefresh(window);
}

// Word-wrap a string to a given width:
void wrapString(char * stringToWrap, int stringLength, int screenWidth)
{
	int characterCount = 0;
	for(int index = 0; index < stringLength; index++)
	{
		if (stringToWrap[index] == '\n')
		{
			characterCount = 0;
		}
		else
		{
			characterCount++;
		}
		if (characterCount == screenWidth)
		{
			while (!isspace(stringToWrap[index]) && index > 0)
			{
				index--;
			}
			if (index == 0)
			{
				return;
			}
			stringToWrap[index] = '\n';
			index++;
			characterCount = 0;
		}
	}
}
