/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef MAIN_H
#define MAIN_H

#define USE_PRINTK 1
//#undef USE_PRINTK

#ifdef USE_PRINTK
#include "printk.h"
#else
#ifdef DEBUG
#define printk pspDebugScreenPrintf

static inline void printk_init()
{
	pspDebugScreenInit();
}

#else
#define printk(...)
#define printk_init(...)
#endif
#endif

#endif