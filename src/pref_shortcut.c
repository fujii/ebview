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

#include "xml.h"
#include <gdk/gdkkeysyms.h>
#include "mainwindow.h"
#include "headword.h"
#include "shortcutfunc.h"
#include "shortcut.h"
#include "mainmenu.h"
#include "dictbar.h"
#include "statusbar.h"
#include "shortcut.h"
#include "pref_io.h"
#include "textview.h"

#ifndef __WIN32__
#include <gdk/gdkx.h>
#endif

static GtkWidget *shortcut_view;
static GtkWidget *command_view;
static GtkWidget *label_key;
static GtkWidget *check_lock;
static GtkListStore *command_store;

static GtkWidget *grab_dlg;
static gboolean grab=FALSE;
static GdkEventKey grabbed_event;
static guint timeout_id;

enum
{
	COMMAND_NAME_COLUMN,
	COMMAND_DESCRIPTION_COLUMN,
	COMMAND_COMMAND_COLUMN,
	COMMAND_N_COLUMNS
};


struct _shortcut_command commands[] = {
	{ N_("Toggle Menu Mar"), toggle_menu_bar},
	{ N_("Toggle Status Bar"), toggle_status_bar},
	{ N_("Toggle Dictionary Bar"), toggle_dict_bar},
	{ N_("Switch Pane Direction"), switch_direction},
	{ N_("Select Automatic Search"), select_any_search},
	{ N_("Select Exactword Search"), select_exactword_search},
	{ N_("Select Word Search"), select_word_search},
	{ N_("Select Endword Search"), select_endword_search},
	{ N_("Select Keyword Search"), select_keyword_search},
	{ N_("Select Multi Search"), select_multi_search},
	{ N_("Select Fulltext Search"), select_fulltext_search},
	{ N_("Select Internet Search"), select_internet_search},
	{ N_("Select File Search"), select_grep_search},
	{ N_("Next Dictionary Group"), next_dict_group},
	{ N_("Previous Dictionary Group"), previous_dict_group},
	{ N_("Toggle Dictionary No. 1"), toggle_dictionary1},
	{ N_("Toggle Dictionary No. 2"), toggle_dictionary2},
	{ N_("Toggle Dictionary No. 3"), toggle_dictionary3},
	{ N_("Toggle Dictionary No. 4"), toggle_dictionary4},
	{ N_("Toggle Dictionary No. 5"), toggle_dictionary5},
	{ N_("Toggle Dictionary No. 6"), toggle_dictionary6},
	{ N_("Toggle Dictionary No. 7"), toggle_dictionary7},
	{ N_("Toggle Dictionary No. 8"), toggle_dictionary8},
	{ N_("Toggle Dictionary No. 9"), toggle_dictionary9},
	{ N_("Toggle Dictionary No. 10"), toggle_dictionary10},
	{ N_("Next Hit"), item_next},
	{ N_("Previous Hit"), item_previous},
	{ N_("Copy To Clipboard"), copy_to_clipboard},
	{ N_("Paste From Clipboard"), paste_from_clipboard},
	{ N_("Start Search"), start_search},
	{ N_("Go Back In History"), go_back},
	{ N_("Go Forward In History"), go_forward},
	{ N_("Show Previous Text"), go_up},
	{ N_("Show Next Text"), go_down},
//	{ N_("Toggle Selection Search"), toggle_auto},
//	{ N_("Toggle Popup"), toggle_popup},
	{ N_("Show Help"), show_usage},
	{ N_("Clear Word"), clear_word},
	{ N_("Quit Program"), quit},
	{ N_("Iconify Window"), iconify},
	{ N_("Scroll Mainview Down"), scroll_mainview_down},
	{ N_("Scroll Mainview Up"), scroll_mainview_up},
	{ N_("Next Hits"), next_heading},
	{ N_("Prev. Hits"), previous_heading},
	{ N_("Increase Font Size"), increase_font_size},
	{ N_("Decrease Font Size"), decrease_font_size},
	{ N_("Expand Lines"), expand_lines},
	{ N_("Shrink Lines"), shrink_lines},
	{NULL, NULL}};


gboolean pref_end_shortcut()
{

	LOG(LOG_DEBUG, "IN : pref_end_shortcut()");

	bignore_locks = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_lock));
	uninstall_shortcut();
	install_shortcut();
	save_shortcut();
	
	LOG(LOG_DEBUG, "OUT : pref_end_shortcut()");
	return(TRUE);
}

gboolean ungrab_server(gpointer data){
	LOG(LOG_DEBUG, "IN : ungrab_server()");

	if(grab == FALSE)
		return(TRUE);

	gtk_timeout_remove(timeout_id);
#ifndef __WIN32__
	gdk_x11_ungrab_server();
#endif
	if(GTK_IS_WIDGET(grab_dlg))
		gtk_widget_destroy(grab_dlg);

	grab = FALSE;

	LOG(LOG_DEBUG, "OUT : ungrab_server()");
	return(TRUE);
}

static void grab_server(GtkWidget *widget,gpointer *data){

	LOG(LOG_DEBUG, "IN : grab_server()");

	if(grab == TRUE)
		return;

#ifndef __WIN32__
	gdk_x11_grab_server();
	timeout_id = gtk_timeout_add(10000, ungrab_server, NULL);
	grab = TRUE;
#endif

	LOG(LOG_DEBUG, "OUT : grab_server()");
}

void key_val_to_string(guint state, guint keyval, gchar *key){

	LOG(LOG_DEBUG, "IN : key_val_to_string(state=%d, keyval=%d)", state, keyval);

	key[0] = '\0';

	if(state & GDK_CONTROL_MASK){
		strcat(key, "Ctrl + ");
	}

	if(state & GDK_SHIFT_MASK){
		strcat(key, "Shift + ");
	}

	if(state & GDK_LOCK_MASK){
		strcat(key, "Lock + ");
	}

	if(state & GDK_MOD1_MASK){ // Alt
		strcat(key, "Alt + ");
	}

	if(state & GDK_MOD2_MASK){ // Num Lock
		strcat(key, "NumLock + ");
	}

	if(state & GDK_MOD3_MASK){ 
		strcat(key, "Mod3 + ");
	}

	if(state & GDK_MOD4_MASK){
		strcat(key, "Mod4 + ");
	}

	if(state & GDK_MOD5_MASK){ // Scroll Lock
		strcat(key, "ScrollLock + ");
	}

	if(state & GDK_BUTTON1_MASK){
		strcat(key, "Button1 + ");
	}

	if(state & GDK_BUTTON2_MASK){
		strcat(key, "Button2 + ");
	}

	if(state & GDK_BUTTON3_MASK){
		strcat(key, "Button3 + ");
	}

	if(state & GDK_BUTTON4_MASK){
		strcat(key, "Button4 + ");
	}

	if(state & GDK_BUTTON5_MASK){
		strcat(key, "Button5 + ");
	}

	if(state & GDK_RELEASE_MASK){
		strcat(key, "Release + ");
	}

	strcat(key, gdk_keyval_name(keyval));

	LOG(LOG_DEBUG, "OUT : key_val_to_string(keyval = %s)", key);
}

static gint window_key_event(GtkWidget *widget, GdkEventKey *event){
	gchar key[256];

	LOG(LOG_DEBUG, "IN : window_key_event(keyval=%d)", event->keyval);
	
//	if(grab != TRUE)
//		return(FALSE);

	switch (event->keyval){
	case GDK_Shift_L:
	case GDK_Shift_R:
	case GDK_Control_L:
	case GDK_Control_R:
	case GDK_Meta_L:
	case GDK_Meta_R:
	case GDK_Alt_L:
	case GDK_Alt_R:
	case GDK_Caps_Lock:
	case GDK_Shift_Lock:
	case GDK_Scroll_Lock:
	case GDK_Num_Lock:
	case GDK_Kana_Lock:
		return(FALSE);
		break;
	}

	if(bignore_locks)
		event->state = event->state & (~GDK_LOCK_MASK) & (~GDK_MOD2_MASK);
	key_val_to_string(event->state, event->keyval, key);

	gtk_label_set_text(GTK_LABEL(label_key), key);

	gdk_keyboard_ungrab(GDK_CURRENT_TIME);

	ungrab_server(NULL);

	grab = FALSE;
	grabbed_event = *event;

	LOG(LOG_DEBUG, "OUT : window_key_event()");

	return(TRUE);
}

static void remove_entry(GtkWidget *widget,gpointer *data){
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	LOG(LOG_DEBUG, "IN : remove_entry()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(shortcut_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		gtk_list_store_remove (GTK_LIST_STORE(shortcut_store), &iter);
	}

	LOG(LOG_DEBUG, "OUT : remove_entry()");
}

static void add_entry(GtkWidget *widget, gpointer *data){

	struct _shortcut_command *command;

	GtkTreeIter iter;
	GtkTreeSelection *selection;
	const gchar *keystr;

	LOG(LOG_DEBUG, "IN : add_entry()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(command_view));
	keystr = gtk_label_get_text(GTK_LABEL(label_key));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{

		gchar *name;
		gchar *description;

		gtk_tree_model_get (GTK_TREE_MODEL(command_store), 
				    &iter, 
				    COMMAND_NAME_COLUMN, &name,
				    COMMAND_DESCRIPTION_COLUMN, &description,
				    COMMAND_COMMAND_COLUMN, &command,
				    -1);

		gtk_list_store_append (shortcut_store, &iter);
		gtk_list_store_set (shortcut_store, &iter,
				    SHORTCUT_STATE_COLUMN,
				    grabbed_event.state,
				    SHORTCUT_KEYVAL_COLUMN,
				    grabbed_event.keyval,
				    SHORTCUT_NAME_COLUMN,
				    name,
				    SHORTCUT_DESCRIPTION_COLUMN,
				    description,
				    SHORTCUT_COMMAND_COLUMN,
				    command,
				    SHORTCUT_KEYSTR_COLUMN, keystr,
				    -1);

		g_free(name);
		g_free(description);
	}

	LOG(LOG_DEBUG, "OUT : add_entry()");
}

static void lock_changed(GtkWidget *widget,gpointer *data)
{

	LOG(LOG_DEBUG, "IN : lock_changed()");

	bignore_locks = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_lock));

	LOG(LOG_DEBUG, "OUT : lock_changed()");
}

GtkWidget *pref_start_shortcut()
{
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *frame;
//	GtkWidget *label;
	GtkWidget *scroll;

	gint i;

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	GtkTreeIter iter;

	LOG(LOG_DEBUG, "IN : pref_start_shortcut()");

	hbox = gtk_hbox_new(TRUE, 0);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start (GTK_BOX(hbox)
			    , vbox, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(hbox),"key_press_event",
			 G_CALLBACK(window_key_event), NULL);

//	gdk_keyboard_grab(hbox->window, FALSE, GDK_CURRENT_TIME);

	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	frame = gtk_frame_new(_("Shortcut"));
	gtk_box_pack_start (GTK_BOX(vbox), frame,TRUE, TRUE, 0);

	vbox2 = gtk_vbox_new(FALSE,2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 2);
	gtk_container_add (GTK_CONTAINER (frame), vbox2);


	scroll = gtk_scrolled_window_new(NULL, NULL);
	//gtk_widget_set_size_request(scroll,300,200);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC, 
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX(vbox2)
			    ,scroll ,TRUE, TRUE, 0);


	shortcut_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(shortcut_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(shortcut_view), TRUE);

	gtk_container_add (GTK_CONTAINER (scroll), shortcut_view);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Key"),
					  renderer,
					  "text", SHORTCUT_KEYSTR_COLUMN,
					  NULL);
	//gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	//gtk_tree_view_column_set_fixed_width(column, 100);
	gtk_tree_view_append_column (GTK_TREE_VIEW (shortcut_view), column);

	column = gtk_tree_view_column_new_with_attributes(_("Command"),
					  renderer,
					  "text", SHORTCUT_DESCRIPTION_COLUMN,
					  NULL);
	//gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	//gtk_tree_view_column_set_fixed_width(column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW (shortcut_view), column);



	hbox2 = gtk_vbox_new(FALSE,2);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox2,
			    FALSE, FALSE, 0);

	button = gtk_button_new_with_label(_("Remove"));
	gtk_box_pack_end (GTK_BOX (hbox2), button,
			    FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK (remove_entry), (gpointer)NULL);





	// Right half

	vbox = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start (GTK_BOX(hbox)
			    , vbox,TRUE, TRUE, 0);


	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	frame = gtk_frame_new(_("Add"));
	gtk_box_pack_start (GTK_BOX(vbox), frame,TRUE, TRUE, 0);

	vbox2 = gtk_vbox_new(FALSE,2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 2);
	gtk_container_add (GTK_CONTAINER (frame), vbox2);




	hbox2 = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start (GTK_BOX(vbox2)
			    , hbox2, FALSE, FALSE, 0);

//	label = gtk_label_new(_("Key : "));
//	gtk_box_pack_start (GTK_BOX(hbox2)
//			    , label,FALSE, FALSE, 0);

	label_key = gtk_label_new("");
	gtk_box_pack_start (GTK_BOX(hbox2)
			    , label_key,FALSE, FALSE, 0);


	button = gtk_button_new_with_label(_("Grab"));
	gtk_box_pack_end(GTK_BOX (hbox2), button,
			 FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(grab_server), (gpointer)NULL);
	
/*

	eventbox = gtk_event_box_new();
	gtk_box_pack_end(GTK_BOX(hbox2), eventbox, FALSE, FALSE, 2);
//	g_signal_connect(G_OBJECT(eventbox),"button_press_event",
	g_signal_connect(G_OBJECT(eventbox),"key_press_event",
			 G_CALLBACK(window_key_event), NULL);


	label = gtk_label_new(_("Type key here"));
	gtk_container_add( GTK_CONTAINER(eventbox), label);
*/



//	gtk_box_pack_start (GTK_BOX(hbox2)
//			    , label_key,FALSE, FALSE, 0);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	//gtk_widget_set_size_request(scroll,250,200);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC, 
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX(vbox2)
			    ,scroll ,TRUE, TRUE, 0);


	command_store = gtk_list_store_new(COMMAND_N_COLUMNS,
					    G_TYPE_STRING,
					    G_TYPE_STRING,
					    G_TYPE_POINTER);

	for(i=0; ; i++){
		if(commands[i].name == NULL)
			break;

		gtk_list_store_append(GTK_LIST_STORE(command_store), &iter);
		gtk_list_store_set(GTK_LIST_STORE(command_store), &iter,
				   COMMAND_NAME_COLUMN, commands[i].name,
				   COMMAND_DESCRIPTION_COLUMN, _(commands[i].name),
				   COMMAND_COMMAND_COLUMN, &commands[i],
				   -1);
	}



	command_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(command_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(command_view), TRUE);

	gtk_container_add (GTK_CONTAINER (scroll), command_view);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Command"),
					  renderer,
					  "text", COMMAND_DESCRIPTION_COLUMN,
					  NULL);
	//gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	//gtk_tree_view_column_set_fixed_width(column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW (command_view), column);




	button = gtk_button_new_with_label(_("Add"));
	gtk_box_pack_start(GTK_BOX (vbox2), button,
			   FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(add_entry), (gpointer)NULL);


	check_lock = gtk_check_button_new_with_label(_("Ignore locks"));
	gtk_box_pack_start(GTK_BOX(vbox2), check_lock,
			   FALSE, FALSE, 0);
	gtk_tooltips_set_tip(tooltip, check_lock, 
			     _("Ignore Caps Lock and Num Lock key."),"Private");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_lock), bignore_locks);
	g_signal_connect(G_OBJECT(check_lock), "clicked",
			 G_CALLBACK(lock_changed), NULL);
	
	LOG(LOG_DEBUG, "OUT : pref_start_shortcut()");
	return(hbox);
}
