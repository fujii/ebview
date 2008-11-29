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

#include "jcode.h"
#include "mainwindow.h"
#include "textview.h"
#include "statusbar.h"

gboolean active=FALSE;
static gchar g_message[1024];

void center_dialog(GtkWidget *window, GtkWidget *dialog){
	gint window_x, window_y;
	gint window_width, window_height;
	gint dialog_x, dialog_y;
	gint dialog_width, dialog_height;

	gdk_window_get_root_origin(window->window, &window_x, &window_y);
	window_width = window->allocation.width;
	window_height = window->allocation.height;
	dialog_width = dialog->allocation.width;
	dialog_height = dialog->allocation.height;

	if((window_width <= dialog_width) || 
	   (window_height <= dialog_height))
		return;

	dialog_x = window_x + (window_width - dialog_width) / 2;
	dialog_y = window_y + (window_height - dialog_height) / 2;

	gtk_window_move(GTK_WINDOW(dialog), dialog_x, dialog_y);
	
}

gboolean idle_warning(gpointer data){
	GtkWidget *dialog;
	GtkWidget *parent;

	LOG(LOG_DEBUG, "IN : idle_warning");

	if(active == TRUE)
		goto END;

	active  = TRUE;

	parent = gtk_grab_get_current();
	if(parent == NULL)
		parent = main_window;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
					GTK_DIALOG_DESTROY_WITH_PARENT /* | GTK_DIALOG_NO_SEPARATOR */,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_OK,
					g_message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	active  = FALSE;

 END:
	LOG(LOG_DEBUG, "OUT : idle_warning");
	return(FALSE);
}

gboolean idle_error(gpointer data){
	GtkWidget *dialog;
	GtkWidget *parent;

	LOG(LOG_DEBUG, "IN : idle_error");

	if(active == TRUE)
		goto END;

	active  = TRUE;

	parent = gtk_grab_get_current();
	if(parent == NULL)
		parent = main_window;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
					GTK_DIALOG_DESTROY_WITH_PARENT /* | GTK_DIALOG_NO_SEPARATOR */,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					g_message);
					
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	active  = FALSE;

 END:
	LOG(LOG_DEBUG, "OUT : idle_error");
	return(FALSE);
}

void popup_warning(char *message){

	LOG(LOG_DEBUG, "IN : popup_warning");

	g_idle_add(idle_warning, (gpointer)message);
	strcpy(g_message, message);

	LOG(LOG_DEBUG, "OUT : popup_warning");
}

void popup_error(char *message){

	LOG(LOG_DEBUG, "IN : popup_error");

	strcpy(g_message, message);

	g_idle_add(idle_error, (gpointer)message);

	LOG(LOG_DEBUG, "OUT : popup_error");
}


extern GtkTextBuffer *text_buffer;

void push_message(gchar *str){
	GtkTextIter iter;

#ifdef __WIN32__
	// Windows だとなぜか死ぬ
	return;
#endif

	gtk_text_buffer_get_end_iter(text_buffer, &iter);
	gtk_text_buffer_insert(
		text_buffer, &iter, 
		str, -1);
}

void clear_message(){
	clear_text_buffer();
}


gboolean popup_active()
{
	return(active);
}

