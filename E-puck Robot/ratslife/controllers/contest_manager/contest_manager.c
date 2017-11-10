/*
 * File:         contest_manager.c
 * Date:         18th Aug 2010
 * Description:  Create a random maze and handle the contest
 * Author:       fabien.rohrer@cyberbotics.com
 * Modifications:
 */

#include <stdlib.h>

#include <webots/robot.h>
#include <webots/supervisor.h>

#include "maze_generator.h"
#include "maze_builder.h"
#include "texture_generator.h"
#include "round_manager.h"

#define TIME_STEP 64

#define MAZE_WIDTH 10
#define MAZE_HEIGHT 10

static Maze *maze = NULL;

static void step(int s) {
  int ret = wb_robot_step(s);
  if (ret == -1) {
    
    // cleanup stuff
    if (maze) {
      delete_maze(maze);
      free(maze);
      cleanup_round();
      empty_the_temporary_path();
    }
    
    wb_robot_cleanup();
    exit(0);
  }
  return;
}

int main(int argc, char **argv)
{
  wb_robot_init();
  
  empty_the_temporary_path();
  
#ifdef DEBUG
  // in order to be able to run the debugger (useful if the generation of the maze crash)
  step(TIME_STEP);
#endif

  // init the maze structure
  maze = (Maze *) malloc(sizeof(Maze));
  init_maze(maze, MAZE_WIDTH, MAZE_HEIGHT);
  while (!generate_random_maze(maze)) {
    delete_maze(maze);
    init_maze(maze, MAZE_WIDTH, MAZE_HEIGHT);
  }
  
#ifdef DEBUG
  // display the maze into the debug display
  WbDeviceTag debug_display = wb_robot_get_device("debug_display");
  display_maze(maze, debug_display);
#endif

  // place the blocks
  build_maze(maze);
  
  // generate the textures of the walls
  WbDeviceTag helper_display = wb_robot_get_device("helper_display");
  update_wall_textures(helper_display);

  // setup the round
  init_round();
  
  // make sure everything has been set up before running the contest
  step(0);

  // run the round
  while(1) {
    run_round();
    step(TIME_STEP);
  }
  
  return 0;
}
