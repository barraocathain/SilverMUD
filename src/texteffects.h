// texteffects.h: Header file for the texteffects library for SilverMUD.
// Barry Kane, 2021.
#ifndef TEXTEFFECTS_H_
#define TEXTEFFECTS_H_
#include <stdio.h>
#include <ncurses.h>

// A character by character print, similar to a serial terminal with lower baud rate:
void slowPrint(const char * stringToPrint, int delay);

// The same, altered to work with ncurses:
void slowPrintNcurses(const char * stringToPrint, int delay, WINDOW * window, bool bolded);

// A character by character "brute-force" print, similar to Hollywood hacking scenes:
void bruteforcePrint(const char * stringToPrint, int delay);

// The same, altered to work with ncurses:
void bruteforcePrintNcurses(const char * stringToPrint, int delay, WINDOW * window, bool bolded);

// Word-wrap a string to a given width:
void wrapString(char * stringToWrap, int stringLength, int screenWidth);

// A string containing an ASCII art version of the Silverkin Industries logo.
const char * logostring =
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

#endif
