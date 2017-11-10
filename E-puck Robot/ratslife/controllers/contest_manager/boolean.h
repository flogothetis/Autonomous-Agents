/*
 * File:         boolean.h
 * Date:         18th Aug 2010
 * Description:  Boolean type
 * Author:       fabien.rohrer@cyberbotics.com
 * Modifications:
 */

#ifndef BOOLEAN_H
#define BOOLEAN_H

#if !defined(bool) && !defined(true) && !defined(false)
typedef enum {false, true} bool;
#endif

#endif
