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
#include "global.h"

#include "dictbar.h"
#include "statusbar.h"
#include "history.h"
#include "ebview.h"
#include "eb.h"
#include "jcode.h"

#ifndef __WIN32__
#include <gdk/gdkx.h>
#endif

extern GList *group_list;
extern GtkWidget *combo_group;
extern GtkWidget *combo_dirgroup;
extern GtkWidget *word_entry;

void next_dict_group(){

	GtkTreeIter  iter;
	gchar *title;
	gboolean active;
	gint method;

	method = ebook_search_method();

	if(method == SEARCH_METHOD_GREP)
		goto GREP;

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
					   &iter, 
					   DICT_ACTIVE_COLUMN, &active,
					   -1);
			if(active == TRUE)
				break;

	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	} else {
		return;
	}

	if(gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == FALSE)
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter);

	gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
			   &iter, 
			   DICT_TITLE_COLUMN, &title,
			   -1);

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_group)->entry), title);

	g_free(title);

/*
	if((method == SEARCH_METHOD_INTERNET) || (method == SEARCH_METHOD_GREP)){
		select_any_search();
	}
*/
	return;

 GREP:

	title = (gchar *)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_dirgroup)->entry));
	if(strcmp(title, _("Manual Select")) == 0){
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirgroup_store), &iter);
		gtk_tree_model_get(GTK_TREE_MODEL(dirgroup_store), 
				   &iter, 
				   DIRGROUP_TITLE_COLUMN, &title,
				   -1);

		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_dirgroup)->entry), title);
		g_free(title);

		return;
	}

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirgroup_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(dirgroup_store), 
					   &iter, 
					   DIRGROUP_ACTIVE_COLUMN, &active,
					   -1);
			if(active == TRUE)
				break;

	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dirgroup_store), &iter) == TRUE);
	} else {
		return;
	}

	if(gtk_tree_model_iter_next(GTK_TREE_MODEL(dirgroup_store), &iter) == FALSE) {
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_dirgroup)->entry), _("Manual Select"));
	} else {
		gtk_tree_model_get(GTK_TREE_MODEL(dirgroup_store), 
				   &iter, 
				   DIRGROUP_TITLE_COLUMN, &title,
				   -1);

		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_dirgroup)->entry), title);
		g_free(title);
	}
}

void previous_dict_group(){

	GtkTreeIter  previous_iter;
	GtkTreeIter  iter;
	gboolean active;
	gchar *title=NULL;
	gint i;
	gint method;

	method = ebook_search_method();

	if(method == SEARCH_METHOD_GREP)
		goto GREP;

	i = 0;
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
					   &iter, 
					   DICT_ACTIVE_COLUMN, &active,
					   -1);
			if(active == TRUE)
				break;

			previous_iter = iter;
			i++;
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	} else {
		return;
	}

	if(i != 0){
		gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
				   &previous_iter, 
				   DICT_TITLE_COLUMN, &title,
				   -1);

	} else {
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter);
		do { 
			g_free(title);
			gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
					   &iter, 
					   DICT_TITLE_COLUMN, &title,
					   -1);
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	}

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_group)->entry), title);

	g_free(title);

/*
	if((method == SEARCH_METHOD_INTERNET) || (method == SEARCH_METHOD_GREP)){
		select_any_search();
	}
*/

	return;

 GREP:

	// 
	title = (gchar *)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_dirgroup)->entry));
	if(strcmp(title, _("Manual Select")) == 0){
		title = NULL;
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirgroup_store), &iter) == TRUE){
			do { 
				g_free(title);
				gtk_tree_model_get(GTK_TREE_MODEL(dirgroup_store), 
						   &iter, 
						   DIRGROUP_TITLE_COLUMN, &title,
						   -1);
			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dirgroup_store), &iter) == TRUE);
			gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_dirgroup)->entry), title);
			g_free(title);
		}

		return;
	}


	i = 0;
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirgroup_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(dirgroup_store), 
					   &iter, 
					   DIRGROUP_ACTIVE_COLUMN, &active,
					   -1);
			if(active == TRUE)
				break;

			previous_iter = iter;
			i++;
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dirgroup_store), &iter) == TRUE);
	} else {
		return;
	}

	if(i != 0){
		gtk_tree_model_get(GTK_TREE_MODEL(dirgroup_store), 
				   &previous_iter, 
				   DIRGROUP_TITLE_COLUMN, &title,
				   -1);

		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_dirgroup)->entry), title);
		g_free(title);

	} else {
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_dirgroup)->entry), _("Manual Select"));
	}

}

extern GtkWidget *dict_box;

void toggle_dictionary(gint number){
	GList *children, *child;
	GtkWidget *w;
	gint idx;
	gboolean active;

	children = gtk_container_get_children(GTK_CONTAINER(dict_box));

	idx = 0;
	child = g_list_first(children);
	while(child != NULL){
		w = (GtkWidget *)(child->data);
		if(GTK_IS_BUTTON(w)){
			if((number - 1) == idx){
				active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
				if(active == TRUE)
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), FALSE);
				else
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), TRUE);
				break;
			}
			else
				idx ++;
		}
		child = g_list_next(child);
	}
}

void toggle_dictionary1(){
	toggle_dictionary(1);
}

void toggle_dictionary2(){
	toggle_dictionary(2);
}

void toggle_dictionary3(){
	toggle_dictionary(3);
}

void toggle_dictionary4(){
	toggle_dictionary(4);
}

void toggle_dictionary5(){
	toggle_dictionary(5);
}

void toggle_dictionary6(){
	toggle_dictionary(6);
}

void toggle_dictionary7(){
	toggle_dictionary(7);
}

void toggle_dictionary8(){
	toggle_dictionary(8);
}

void toggle_dictionary9(){
	toggle_dictionary(9);
}

void toggle_dictionary10(){
	toggle_dictionary(10);
}

void go_back(){
	history_back();
}

void go_forward(){
	history_forward();
}

void clear_word(){
	gtk_entry_set_text(GTK_ENTRY(word_entry), "");
	gtk_widget_grab_focus(word_entry);
}

void quit(){
	exit_program(NULL, NULL);
}

void iconify(){
#ifdef __WIN32__
#else
    XIconifyWindow (GDK_DISPLAY (), 
		    GDK_WINDOW_XWINDOW(main_window->window),
		    DefaultScreen (GDK_DISPLAY ()));
#endif
}

void paste_from_clipboard()
{
#ifdef __WIN32__
	HWND hwnd;
	LRESULT retval;
	HANDLE hText;
	char *pText;
	gchar *str;
#else
	gchar *str=NULL;
	GtkClipboard* clipboard;
#endif
	gint position;
	gint start, end;

	LOG(LOG_DEBUG, "IN : paste_clipboard()");

#ifdef __WIN32__    
	hwnd = GDK_WINDOW_HWND (main_window->window);
	OpenClipboard(hwnd);
	hText = GetClipboardData(CF_TEXT);
	if(hText == NULL) {
		CloseClipboard();
		LOG(LOG_DEBUG, "OUT : paste_clipboard() : NOP");
		return;
	} else {
		pText = GlobalLock(hText);
		GlobalUnlock(hText);

		str = iconv_convert(fs_codeset, "utf-8", pText);
		CloseClipboard();
	}
#else
	clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	str = gtk_clipboard_wait_for_text(clipboard);
	if(str == NULL){
		LOG(LOG_DEBUG, "OUT : paste_clipboard() : NOP");
		return;
	}

#endif
	gtk_editable_get_selection_bounds(GTK_EDITABLE(word_entry), &start, &end);
	gtk_editable_delete_text(GTK_EDITABLE(word_entry), start, end);

	position = gtk_editable_get_position(GTK_EDITABLE(word_entry));
	gtk_editable_insert_text(GTK_EDITABLE(word_entry), str, strlen(str), &position);
	gtk_editable_set_position(GTK_EDITABLE(word_entry), position);
	g_free(str);

	LOG(LOG_DEBUG, "OUT : paste_clipboard()");

}
