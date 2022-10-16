// texteffects.c: Implementation of text effect library for SilverMUD.
// Barry Kane, 2021.
#include <stdio.h>
#include <ctype.h>
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

void slowPrintNcurses(char * stringToPrint, int delay, WINDOW * window, bool bolded)
{
	int characterIndex = 0;
	if(bolded)
	{
		wattron(window, A_BOLD);
	}
	while(stringToPrint[characterIndex] != '\0')
	{
		waddch(window, stringToPrint[characterIndex]);
		// Refresh the ncurses screen.
		wrefresh(window);
		usleep(delay);
		characterIndex++;
	}
	if(bolded)
	{
		wattroff(window, A_BOLD);
	}
	wrefresh(window);
}

void bruteForcePrintNcurses(char * stringToPrint, int delay, WINDOW * window, bool bolded)
{
	int characterIndex = 0;
	if(bolded)
	{
		wattron(window, A_BOLD);
	}
	while(stringToPrint[characterIndex] != '\0')
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
	if(bolded)
	{
		wattroff(window, A_BOLD);
	}
	wrefresh(window);
}

void wrapString(char * stringToWrap, int stringLength, int screenWidth)
{
	int characterCount = 0;
	for(int index = 0; index < stringLength; index++)
	{
		if(stringToWrap[index] == '\n')
		{
			characterCount = 0;
		}
		else
		{
			characterCount++;
		}
		if(characterCount == screenWidth)
		{
			while(!isspace(stringToWrap[index]) && index > 0)
			{
				index--;
			}
			if(index == 0)
			{
				return;
			}
			stringToWrap[index] = '\n';
			index++;
			characterCount = 0;
		}
	}
}
