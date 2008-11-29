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

#ifndef __HEADWORD_H_
#define __HEADWORD_H_

#include "defs.h"

void show_result_tree();
void select_first_item();
void item_next();
void item_previous();
void next_heading(GtkWidget *widget, gpointer *data);
void previous_heading(GtkWidget *widget, gpointer *data);

GtkWidget *create_headword_tree();
void update_tree_view();

#endif /* __HEADWORD_H__ */
