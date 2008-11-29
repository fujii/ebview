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


#include "defs.h"

void start_search();
void do_search(GtkWidget *widget, gpointer *data);
void show_about();
void show_usage();
void show_home();
void create_main_window();
void restart_main_window();
GtkWidget *create_dict_window();
void claim_clipboard_owner();
void show_text(BOOK_INFO *binfo, char *text, gchar *word);
void show_result(RESULT *result, gboolean bsave_history, gboolean breverse_keyword);
void show_dict(RESULT *result, gboolean bsave_history, gboolean breverse_keyword);


void toggle_auto();
void toggle_popup();

void select_any_search();
void select_exactword_search();
void select_word_search();
void select_endword_search();
void select_keyword_search();
void select_multi_search();
void select_fulltext_search();
void select_internet_search();
void select_grep_search();

void go_up();
void go_down();
