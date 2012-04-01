/* gcompris - timer.h
 *
 * Copyright (C) 2001, 2008 Pascal Georges
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIMER_H
#define TIMER_H

#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "gcompris.h"

typedef enum
{
  GCOMPRIS_TIMER_TEXT,
  GCOMPRIS_TIMER_SAND,
  GCOMPRIS_TIMER_BALLOON,
  GCOMPRIS_TIMER_CLOCK,
} TimerList;

/* Callback when the timer is completed */
typedef void          (*GcomprisTimerEnd)       ();

void	 gc_timer_display(int x, int y, TimerList type, int second, GcomprisTimerEnd gcomprisTimerEnd);
void	 gc_timer_add(int second);
void	 gc_timer_end(void);
guint	 gc_timer_get_remaining();
void	 gc_timer_pause(gboolean pause);

#endif
