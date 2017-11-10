/*
 * File:         maze_generator.h
 * Date:         18th Aug 2010
 * Description:  Interface to generate a maze
 * Author:       fabien.rohrer@cyberbotics.com
 * Modifications:
 */

#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include "maze_definition.h"
#include <webots/types.h>

void init_maze(Maze *maze, int width, int height);
bool generate_random_maze(Maze *maze);
void delete_maze(Maze *maze);
void display_maze(Maze *maze, WbDeviceTag display);

#endif
