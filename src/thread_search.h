/*  Copyright (C) 2001-2004  Kenichi Suto
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __THREAD_SEARCH_H__
#define __THREAD_SEARCH_H__

#include "defs.h"
#include <pthread.h>


void thread_search(gboolean cancelable, gchar *text, void *(* func)(void *), void *arg);
void add_result(RESULT *rp);
void set_progress(gfloat progress);
void thread_end();
void set_cancel_dlg_text(gchar *text);

#endif /* __THREAD_SEARCH_H__ */
