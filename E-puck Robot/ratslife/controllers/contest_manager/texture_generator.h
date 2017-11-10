/*
 * File:         texture_generator.h
 * Date:         30th Aug 2010
 * Description:  Interface to generate the wall textures
 * Author:       fabien.rohrer@cyberbotics.com
 * Modifications:
 */

#ifndef TEXTURE_GENERATOR_H
#define TEXTURE_GENERATOR_H

#include <webots/types.h>

void update_wall_textures(WbDeviceTag display);
void empty_the_temporary_path();

#endif
