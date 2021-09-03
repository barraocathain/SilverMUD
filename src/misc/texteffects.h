// texteffects.h: Header file for the texteffects library for SilverMUD.
// Barry Kane, 2021.
#ifndef TEXTEFFECTS_H_
#define TEXTEFFECTS_H_
#include <stdio.h>

// A fancy, character by character print. Similar to a serial terminal with lower baud rate.
void slowPrint(char * stringToPrint, int delay);

// The same, altered to work with Ncurses.
void slowPrintNcurses(char * stringToPrint, int delay);

// A string containing an ASCII art version of the Silverkin Industries logo.
char * logostring = "                                                         ///////\n                                 //////////////////////////////////////////\n                   ///////////////////////////////////////////////////////////\n        //////////                                  ////////////////////////////\n   ###   #  #    #     # #####  ###   # #  #  #   #            /////////////////\n  ###    #  #     #   #  ##     #  #  ##   #  ##  #               //////////////\n     ##	 #  #      # #   #      ###   # #  #  # # #                   /////////\n  ####   #  ###     #    #####  #  #  #  # #  #  ##                    ///////\n         #  ##   #  #####   #   #   ### ### ###  # #####  ###          ////// \n         #  # #  #  #    #  #   #  ##    #  #  # # ##    ##             ////\n       	 #  #  # #  #    #  #   #    ##  #  ###	 # #   	   ##	       	//\n         #  #  ###  #####   #####  ###   #  #  # # ####  ###           //\n";

#endif
