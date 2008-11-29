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

#include "pref_io.h"

static gint tag = 0;
guint context_id;

extern GtkWidget *display_statusbar;

gint clear_status_message(gpointer data)
{
	gtk_statusbar_pop(GTK_STATUSBAR(status_bar), context_id);
	tag = 0;
	return(FALSE);
}
void status_message(gchar *msg)
{
	if(tag != 0){
		gtk_timeout_remove(tag);
	}
	gtk_statusbar_pop(GTK_STATUSBAR(status_bar), context_id);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), context_id, msg);
//	tag = gtk_timeout_add(5000, clear_status_message, NULL);
}

void show_status_bar()
{
	gtk_widget_show(status_bar);
	bshow_status_bar = 1;
	save_preference();


	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_statusbar),
				       bshow_status_bar);
}

void hide_status_bar()
{
	gtk_widget_hide(status_bar);
	bshow_status_bar = 0;
	save_preference();

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_statusbar),
				       bshow_status_bar);

}

void toggle_status_bar(){
	if(bshow_status_bar == 1)
		hide_status_bar();
	else 
		show_status_bar();
}

