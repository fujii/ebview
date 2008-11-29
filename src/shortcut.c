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

#include <gdk/gdkkeysyms.h>

extern struct _shortcut_command commands[];

gint menuitem_handler(GtkWidget *widget, gchar *string);

GtkAccelGroup *accel_group=NULL;

static GList *accel_item_list=NULL;

gboolean accel_handler(GtkAccelGroup *accelgroup,
		       GObject *arg1,
		       guint arg2,
		       GdkModifierType arg3,
		       gpointer user_data)
{

	GdkEventKey event;
	gboolean ret;

	LOG(LOG_DEBUG, "IN : accel_handler()");

	event.keyval = user_data;
	event.state = arg3;

	ret = perform_shortcut_by_event(&event);

	LOG(LOG_DEBUG, "OUT : accel_handler()");

	return(ret);
}

void install_shortcut(){
	GtkTreeIter  iter;
	guint state;
	guint keyval;
	GtkWidget *item;
	gint idx;
	gchar *str;
	struct _shortcut_command *command;
	GClosure *closure;

	LOG(LOG_DEBUG, "IN : install_shortcut()");

	idx = 0;

	if(accel_group != NULL) {
		gtk_window_remove_accel_group(GTK_WINDOW(main_window),
					      accel_group);
		g_object_unref(accel_group);
	}
		
	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group (GTK_WINDOW (main_window), accel_group);


	g_signal_connect(G_OBJECT(accel_group), "accel-activate",
			 G_CALLBACK(accel_handler),
			 (gpointer)NULL);


	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(shortcut_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(shortcut_store), 
					   &iter,
					   SHORTCUT_STATE_COLUMN, &state,
					   SHORTCUT_KEYVAL_COLUMN, &keyval,
					   SHORTCUT_COMMAND_COLUMN, &command,
					   -1);

//			g_snprintf(string, 64, "sc_%s", command->name);
			str = g_strdup_printf("sc_%s", command->name);
			if((state == 0) && (keyval == GDK_Return))
				continue;

			closure = g_cclosure_new(G_CALLBACK(accel_handler), (gpointer)keyval, NULL);


			gtk_accel_group_connect(accel_group, keyval, state, GTK_ACCEL_VISIBLE, closure);
/*
			gtk_widget_add_accelerator (main_window, "activate", accel_group,
						    keyval, state, GTK_ACCEL_VISIBLE);
			accel_item_list = g_list_append(accel_item_list, item);
*/
//			gtk_widget_show(item);

		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(shortcut_store), &iter) == TRUE);
	}

	LOG(LOG_DEBUG, "OUT : install_shortcut()");
}

void install_shortcut_old(){
	GtkTreeIter  iter;
	guint state;
	guint keyval;
	GtkWidget *item;
	gint idx;
	gchar *str;
	struct _shortcut_command *command;

	LOG(LOG_DEBUG, "IN : install_shortcut()");

	idx = 0;

	accel_group = gtk_accel_group_new();

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(shortcut_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(shortcut_store), 
					   &iter,
					   SHORTCUT_STATE_COLUMN, &state,
					   SHORTCUT_KEYVAL_COLUMN, &keyval,
					   SHORTCUT_COMMAND_COLUMN, &command,
					   -1);

//			g_snprintf(string, 64, "sc_%s", command->name);
			str = g_strdup_printf("sc_%s", command->name);
			if((state == 0) && (keyval == GDK_Return))
				continue;

			item = gtk_menu_item_new();
			g_signal_connect(G_OBJECT(item), "activate",
					 G_CALLBACK(menuitem_handler),
					 (gpointer)str);

			gtk_widget_add_accelerator (item, "activate", accel_group,
						    keyval, state, GTK_ACCEL_VISIBLE);
			accel_item_list = g_list_append(accel_item_list, item);
			gtk_widget_show(item);

		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(shortcut_store), &iter) == TRUE);
	}

	LOG(LOG_DEBUG, "OUT : install_shortcut()");
}

void uninstall_shortcut(){
	GList *item;
	GtkWidget *widget;
	
	LOG(LOG_DEBUG, "IN : uninstall_shortcut()");

#if 0
	item = g_list_first(accel_item_list);
	while(item){
		widget = (GtkWidget *)(item->data);
		gtk_widget_destroy(widget);
		item = g_list_next(item);
	}
	g_list_free(accel_item_list);
	accel_item_list = NULL;
	g_object_unref(accel_group);
#endif

	LOG(LOG_DEBUG, "OUT : uninstall_shortcut()");
}

gboolean perform_shortcut_by_event(GdkEventKey *event){

	GtkTreeIter  iter;

	guint state;
	guint keyval;
	struct _shortcut_command *command;

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(shortcut_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(shortcut_store), 
					   &iter,
					   SHORTCUT_STATE_COLUMN, &state,
					   SHORTCUT_KEYVAL_COLUMN, &keyval,
					   SHORTCUT_COMMAND_COLUMN, &command,
					   -1);

			if(event->keyval == keyval){
				if(event->state == state){
					command->func();
					return(TRUE);
				}

				if((bignore_locks) &&
				   ((event->state | GDK_MOD2_MASK | GDK_LOCK_MASK) == (state | GDK_MOD2_MASK | GDK_LOCK_MASK))){
					command->func();
					return(TRUE);
				}
			}
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(shortcut_store), &iter) == TRUE);
	}

	return(FALSE);
}

gboolean perform_shortcut(gchar *name){

	gint i;

	LOG(LOG_DEBUG, "IN : perform_shortcut(%s)", name);

	for(i=0; ; i ++){
		if(commands[i].name == NULL)
			break;

		if(strcmp(commands[i].name, &name[3]) == 0){
			commands[i].func();
			LOG(LOG_DEBUG, "OUT : perform_shortcut() = TRUE");
			return(TRUE);
		}
	}

	LOG(LOG_DEBUG, "OUT : perform_shortcut() = FALSE");
	return(FALSE);
}
