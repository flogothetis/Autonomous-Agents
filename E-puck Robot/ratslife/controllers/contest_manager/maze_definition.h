/*
 * File:         maze_definition.h
 * Date:         18th Aug 2010
 * Description:  Structure defining a maze
 * Author:       fabien.rohrer@cyberbotics.com
 * Modifications:
 */

#ifndef MAZE_DEFINITION_H
#define MAZE_DEFINITION_H

#include "boolean.h"
#include "linked_list.h"
// enum
typedef enum {North, South, East, West, None} Orientation;

// prototypes
struct _Link;
struct _Cell;
struct _Maze;

// link between two cells
typedef struct _Link {
  bool wall; // is there a wall between cell_A and cell_B ?
  bool removable; // Cannot be removed if located close to a feeder
  struct _Cell *cell_A;
  struct _Cell *cell_B;
} Link;

// a cell is made of up to 4 links
typedef struct _Cell {
  int pos_x;
  int pos_y;
  bool visited;
  Orientation feeder;
  Orientation init_pos;
  int link_number;
  struct _Link **links; // array of links
} Cell;

// the maze
typedef struct _Maze {
  int width;
  int height;
  struct _Cell **cells; // array of cells
  LLIST *links; // list of links
} Maze;

#endif
