// texteffects.h: Header file for the texteffects library for SilverMUD.
// Barry Kane, 2021.
#ifndef TEXTEFFECTS_H_
#define TEXTEFFECTS_H_
#include <stdio.h>
#include <ncurses.h>

// A character by character print, similar to a serial terminal with lower baud rate.
void slowPrint(char * stringToPrint, int delay);

// The same, altered to work with ncurses.
void slowPrintNcurses(char * stringToPrint, int delay, WINDOW * window, bool bolded);

// A character by character "brute-force" print, similar to Hollywood hacking scenes.
void bruteforcePrint(char * stringToPrint, int delay);

// The same, altered to work with ncurses.
void bruteforcePrintNcurses(char * stringToPrint, int delay, WINDOW * window, bool bolded);

// A string containing an ASCII art version of the Silverkin Industries logo.
char * logostring =
	"                                                         ///////\n"
    "                                 //////////////////////////////////////////\n"
    "                   ///////////////////////////////////////////////////////////\n"
    "        //////////                                  ////////////////////////////\n"
    "   ###   #   #    #     #  #####   ###   #   #   #   #   #       /////////////////\n"
    "  ##     #   #     #   #   ##      #  #  ###     #   ##  #          //////////////\n"
    "    ##   #   #      # #    #       ###   #  #    #   # # #              /////////\n"
    "  ###    #   ###     #     #####   #  #  #   #   #   #   #               ///////\n"
    "         #  ##   #  #####   #   #   ### ### ###  # #####  ###             //////\n"
    "         #  # #  #  #    #  #   #  ##    #  #  # # ##    ##                ////\n"
    "         #  #  # #  #    #  #   #    ##  #  ###  # #       ##               //\n"
    "         #  #  ###  #####   #####  ###   #  #  # # ####  ###                /\n";

void wrapString(char * stringToWrap, int stringLength, int screenWidth);

#endif
