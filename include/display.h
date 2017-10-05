/*
 * display.h
 *
 * configuration for the SDOs
 *
 * 2016-06-16, synapticon
 */

#ifndef DISPLAY_H
#define DISPLAY_H


#include <curses.h> // required


typedef struct {
    int row;
    int col;
} Cursor;

#include "ecat_master.h"
#include "operation.h"
#include "cia402.h"


void wmoveclr(WINDOW *wnd, int *row);

int draw(WINDOW *wnd, char c, int row, int column);

void print_help(WINDOW *wnd, int row);

int display_slaves(WINDOW *wnd, int row, PDOOutput *pdo_output, PDOInput *pdo_input, size_t num_slaves, OutputValues output);

#endif /* DISPLAY_H */
