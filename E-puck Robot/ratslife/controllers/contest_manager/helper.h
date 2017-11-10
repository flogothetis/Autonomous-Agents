/*
 * File:         helper.h
 * Date:         30th Aug 2010
 * Description:  Definition of some generic helper functions
 * Author:       fabien.rohrer@cyberbotics.com
 * Modifications:
 */

#ifndef HELPER_H
#define HELPER_H

#define RANDOM(min,max) (rand()%(max-min) + min) // random integer [min;max[
#define ROUND(x)        ((int)(0.5+x))
#define MAX(a,b)        ((a)<(b)?(b):(a))

enum {CX, CY, CZ, ALPHA};
enum {R0, R1};
enum {
  WHITE  = 0xFFFFFF,
  RED    = 0xFF0000,
  GREEN  = 0x00FF00,
  BLUE   = 0x0000FF,
  YELLOW = 0xFFFF00,
  BLACK  = 0x000000
};

#endif
