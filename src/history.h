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

#ifndef __HISTORY_H_
#define __HISTORY_H_

#include "defs.h"

void save_result_history(RESULT *result);
void history_back();
void history_forward();

void save_word_history(const gchar *word);

void copy_result(RESULT *to, RESULT *from);
RESULT *duplicate_result(RESULT *rp);
void set_current_result(RESULT *rp);
void free_result(RESULT *rp);
void clear_search_result();

#endif /* __HISTORY_H_ */
