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

#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include "defs.h"

void create_text_buffer();
GtkWidget *create_main_view();
gint motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
gint button_press_event(GtkWidget *widget, GdkEventButton *event);
void scroll_mainview_down();
void scroll_mainview_up();
void copy_to_clipboard();
void clear_text_buffer();
void expand_lines();
void shrink_lines();
void increase_font_size();
void decrease_font_size();


#endif /* __TEXTVIEW_H__ */
