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
#include <sys/types.h>

#include "bmh.h"
#include "dialog.h"
#include "dictbar.h"
#include "dirtree.h"
#include "dump.h"
#include "eb.h"
#include "ebview.h"
#include "external.h"
#include "grep.h"
#include "headword.h"
#include "history.h"
#include "jcode.h"
#include "link.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "misc.h"
#include "multi.h"
#include "pixmap.h"
#include "pref_io.h"
#include "render.h"
#include "preference.h"
#include "shortcut.h"
#include "statusbar.h"
#include "selection.h"
#include "textview.h"
#include "websearch.h"

GtkWidget *container_child(GtkWidget *container);

GtkWidget *hidden_window;
GtkWidget *entry_box=NULL;
GtkWidget *note_bar=NULL;
GtkWidget *note_tree=NULL;
GtkWidget *note_text=NULL;
GtkWidget *main_area=NULL;
GtkWidget *pane=NULL;

extern GtkTextBuffer *text_buffer;
extern GtkWidget *dict_scroll;
extern GtkWidget *main_view;
extern guint context_id;

extern GList *word_history;

static gint about_usage = 1;
static gboolean style_set=FALSE;
static gint eb_web=0;
static gulong handler_method;
static gulong handler_notebook;
static gint note_page=0;
static gboolean entry_focus_in=FALSE;


GdkPixbuf *pixbuf_popup;
GdkPixbuf *pixbuf_popup_checked;

GdkPixbuf *pixbuf_auto;
GdkPixbuf *pixbuf_auto_checked;

GdkAtom clipboard_atom = GDK_NONE;
gchar *clipboard=NULL;

void start_search(){
	const gchar *word;
	gchar *euc_str;
	gint method;

	LOG(LOG_DEBUG, "IN : start_search");

	if(GTK_WIDGET_MAPPED(main_window) != TRUE)
		return;

	word = gtk_entry_get_text(GTK_ENTRY(word_entry));
	euc_str = iconv_convert("utf-8", "euc-jp", word);
	remove_space(euc_str);
	if(strlen(euc_str) == 0){
		popup_warning(_("Please enter search word."));
		g_free(euc_str);
		return;
	}


	if(strlen(euc_str) != 0){
		method = ebook_search_method();
		if(method == SEARCH_METHOD_INTERNET){
			web_search();
		} else 	if(method == SEARCH_METHOD_GREP){
			clear_message();
			grep_search(euc_str);
		} else {
			clear_message();
			ebook_search(euc_str, method);
			if(search_result == NULL)
				push_message(_("No hit."));
		}
		save_word_history(word);
		gtk_editable_select_region(GTK_EDITABLE(word_entry), 
					   0,
					   GTK_ENTRY(word_entry)->text_length);
	}
	g_free(euc_str);

	LOG(LOG_DEBUG, "OUT : start_search");
}

#if 0
static void toggle_auto_callback(GtkWidget *widget, gpointer *data){
	GList *children;

	LOG(LOG_DEBUG, "IN : toggle_auto_callback()");
	
        bauto_lookup = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_auto));

	if(bauto_lookup){
		auto_lookup_start();

		if(button_popup){
			gtk_widget_set_sensitive(button_popup, TRUE);
		}

	} else {
		auto_lookup_stop();

		if(button_popup)
			gtk_widget_set_sensitive(button_popup, FALSE);

	}

	children = gtk_container_get_children(GTK_CONTAINER(button_auto));
	g_assert(GTK_IS_IMAGE(children->data));
	if(bauto_lookup)
		gtk_image_set_from_pixbuf(GTK_IMAGE(children->data), pixbuf_auto_checked);
	else
		gtk_image_set_from_pixbuf(GTK_IMAGE(children->data), pixbuf_auto);

	g_list_free(children);


	save_preference();

	LOG(LOG_DEBUG, "OUT : toggle_auto_callback()");
}

static void toggle_popup_callback(GtkWidget *widget,gpointer *data){
	GList *children;

	LOG(LOG_DEBUG, "IN : toggle_popup_callback()");

	bshow_popup = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_popup));

	children = gtk_container_get_children(GTK_CONTAINER(button_popup));
	g_assert(GTK_IS_IMAGE(children->data));
	if(bshow_popup)
		gtk_image_set_from_pixbuf(GTK_IMAGE(children->data), pixbuf_popup_checked);
	else
		gtk_image_set_from_pixbuf(GTK_IMAGE(children->data), pixbuf_popup);

	g_list_free(children);

	save_preference();

	LOG(LOG_DEBUG, "OUT : toggle_popup_callback()");
}

void toggle_auto(){

	gint active;

	LOG(LOG_DEBUG, "IN : toggle_auto()");

        active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_auto));
	if(active)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_auto), FALSE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_auto), TRUE);

	LOG(LOG_DEBUG, "OUT : toggle_auto()");
}

void toggle_popup(){
	gint active;

	LOG(LOG_DEBUG, "IN : toggle_popup()");

        active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_popup));
	if(active)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_popup), FALSE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_popup), TRUE);

	LOG(LOG_DEBUG, "OUT : toggle_popup()");
}
#endif

static gint method_changed (GtkWidget *combo){

	gint method;
	
	// You cannot show menu and copyright here
	// because they will be shown before you release the mouse button.
	// Menu and copyright should be selected by main menu.

	LOG(LOG_DEBUG, "IN : method_changed()");

	g_signal_handler_block(G_OBJECT(note_tree), handler_notebook);

	method = ebook_search_method();

	if(method == SEARCH_METHOD_MULTI){
		if(note_page != 1)
			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 1);

		gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 1);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(note_bar), 0);

	} else 	if(method == SEARCH_METHOD_INTERNET){
		eb_web = 1;
		if(note_page != 2)
			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 2);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 0);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(note_bar), 0);

	} else 	if(method == SEARCH_METHOD_FULL_TEXT){
		if(note_page != 0)
			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 0);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(note_bar), 0);

	} else 	if(method == SEARCH_METHOD_GREP){
		gtk_notebook_set_current_page(GTK_NOTEBOOK(note_bar), 1);
	} else {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(note_bar), 0);
	}

	g_signal_handler_unblock(G_OBJECT(note_tree), handler_notebook);

	change_search_menu(method);

	LOG(LOG_DEBUG, "OUT : method_changed()");
	return(FALSE);
}


static gint entry_activate_event(GtkWidget *widget, GdkEventKey *event){

	LOG(LOG_DEBUG, "IN : entry_activate_event()");

	start_search(NULL, NULL);

	LOG(LOG_DEBUG, "OUT : entry_activate_event()");
	return(FALSE);
}

void show_about()
{
	gchar *lang;
	gchar buff[65536];
	gchar filename[512];
	FILE *fp;
	gint len;

	LOG(LOG_DEBUG, "IN : show_about()");

#ifdef __WIN32__
	sprintf(filename, "%s%sabout.jp", package_dir, DIR_DELIMITER);
#else
	lang = getenv("LANG");
	if(lang == NULL){
		sprintf(filename, "%s%sabout.en", package_dir, DIR_DELIMITER);
	} else if(strncmp(lang, "ja_JP", 5) == 0){
		sprintf(filename, "%s%sabout.jp", package_dir, DIR_DELIMITER);
	} else {
		sprintf(filename, "%s%sabout.en", package_dir, DIR_DELIMITER);
	}
#endif

	fp = fopen(filename, "r");
	if(fp == NULL){
		LOG(LOG_CRITICAL, _("Couldn't find %s. Check installation."), filename);
		return;
	}

	len = fread(buff, 1, 65535, fp);
	fclose(fp);

	buff[len] = '\0';
	if(len != 0)
		show_text(NULL, buff, NULL);
	set_current_result(NULL);

	about_usage = 1;

	LOG(LOG_DEBUG, "OUT : show_about()");
}

void show_usage()
{

	gchar *lang;
	gchar filename[512];
	gchar *tmp_str;

	LOG(LOG_DEBUG, "IN : show_usage()");

#ifdef __WIN32__
	sprintf(filename, "%s%shelp%sindex.html", package_dir,
		DIR_DELIMITER, DIR_DELIMITER);
#else
	lang = getenv("LANG");
	if(lang == NULL){
		sprintf(filename, "file://%s/help/en/index.html", package_dir);
	} else if(strncmp(lang, "ja_JP", 5) == 0){
		sprintf(filename, "file://%s/help/ja/index.html", package_dir);
	} else {
		sprintf(filename, "file://%s/help/en/index.html", package_dir);
	}
#endif
	tmp_str = iconv_convert("utf-8", "euc-jp", _("Help will be shown in external web browser."));
	show_text(0, tmp_str, NULL);
	g_free(tmp_str);

	launch_web_browser(filename);

	set_current_result(NULL);

	about_usage = 2;

	LOG(LOG_DEBUG, "OUT : show_usage()");
}

void show_home()
{
	LOG(LOG_DEBUG, "IN : show_home()");

	launch_web_browser("http://ebview.sourceforge.net/");

	LOG(LOG_DEBUG, "OUT : show_home()");
}

static void dict_history_back(GtkWidget *widget, gpointer *data)
{
	LOG(LOG_DEBUG, "IN : dict_history_back()");
	history_back();
	LOG(LOG_DEBUG, "OUT : dict_history_back()");
}

static void dict_history_forward(GtkWidget *widget, gpointer *data)
{
	LOG(LOG_DEBUG, "IN : dict_history_forward()");
	history_forward();
	LOG(LOG_DEBUG, "OUT : dict_history_forward()");
}


static void dict_forward_text(GtkWidget *widget, gpointer *data)
{
	gint page, offset;
	EB_Error_Code error_code;
	RESULT result;

	LOG(LOG_DEBUG, "IN : dict_forward_text()");

	if(current_result  == NULL) {
		LOG(LOG_DEBUG, "OUT : dict_forward_text()");
		return;
	}

	if(current_result->type != RESULT_TYPE_EB){
		LOG(LOG_DEBUG, "OUT : dict_forward_text()");
		return;
	}

	error_code = ebook_forward_text(current_result->data.eb.book_info);
	if(error_code != EB_SUCCESS){
		LOG(LOG_DEBUG, "OUT : dict_forward_text() = %d", error_code);
		return;
	}

	ebook_tell_text(current_result->data.eb.book_info, &page, &offset);

	result.type = RESULT_TYPE_EB;
	result.data.eb.book_info = current_result->data.eb.book_info;
	result.data.eb.pos_text.page = page;
	result.data.eb.pos_text.offset = offset;
	result.data.eb.dict_title = g_strdup(current_result->data.eb.dict_title);
	
	show_result(&result, TRUE, FALSE);
	
	LOG(LOG_DEBUG, "OUT : dict_forward_text()");
}


static void dict_backward_text(GtkWidget *widget, gpointer *data)
{
	gint page, offset;
	EB_Error_Code error_code;
	RESULT result;

	LOG(LOG_DEBUG, "IN : dict_backward_text()");
	
	if(current_result  == NULL){
		LOG(LOG_DEBUG, "OUT : dict_backward_text()");
		return;
	}

	if(current_result->type != RESULT_TYPE_EB){
		LOG(LOG_DEBUG, "OUT : dict_forward_text()");
		return;
	}

	error_code = ebook_backward_text(current_result->data.eb.book_info);
	if(error_code != EB_SUCCESS){
		LOG(LOG_DEBUG, "OUT : dict_backward_text() = %d", error_code);
		return;
	}

	ebook_tell_text(current_result->data.eb.book_info, &page, &offset);

	result.type = RESULT_TYPE_EB;
	result.data.eb.book_info = current_result->data.eb.book_info;
	result.data.eb.pos_text.page = page;
	result.data.eb.pos_text.offset = offset;
	result.data.eb.book_info = current_result->data.eb.book_info;
	result.data.eb.dict_title = g_strdup(current_result->data.eb.dict_title);
	
	show_result(&result, TRUE, FALSE);
	
	LOG(LOG_DEBUG, "OUT : dict_backward_text()");
}

void go_up(){
	LOG(LOG_DEBUG, "IN : go_up()");
	dict_forward_text(NULL, NULL);
	LOG(LOG_DEBUG, "OUT : go_up()");
}

void go_down(){
	LOG(LOG_DEBUG, "IN : go_down()");
	dict_backward_text(NULL, NULL);
	LOG(LOG_DEBUG, "OUT : go_down()");
}

void note_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, gint page_num, gpointer data)
{
	LOG(LOG_DEBUG, "IN : note_switch_page()");

	g_signal_handler_block(G_OBJECT(GTK_COMBO(combo_method)->entry),
			       handler_method);

	note_page = page_num;

	if((note_text) && GTK_IS_WIDGET(note_text)) {
		switch (page_num) {
		case 0:
			eb_web = 0;
			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 0);

			if(strcmp(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry)), _("Internet Search")) == 0)
				gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Automatic Search"));
			break;

		case 1:
			eb_web = 0;
			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_bar), 0);
			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 1);
			
			if(strcmp(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry)), _("Multiword Search")) != 0)
				gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Multiword Search"));

			break;
		case 2:
			eb_web = 1;
			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_bar), 0);

			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 0);
			if(strcmp(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry)), _("Internet Search")) != 0)
			gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Internet Search"));

			break;
		case 3:
			eb_web = 0;
			gtk_notebook_set_current_page(GTK_NOTEBOOK(note_bar), 1);
			
			if(strcmp(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry)), _("File Search")) != 0)
				gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("File Search"));

			break;

		}
	}

	g_signal_handler_unblock(G_OBJECT(GTK_COMBO(combo_method)->entry),
				 handler_method);
	LOG(LOG_DEBUG, "OUT : note_switch_page()");
}

static void delete_event( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	LOG(LOG_DEBUG, "IN : delete_event()");

	exit_program(widget, data);

	LOG(LOG_DEBUG, "OUT : delete_event()");
}

static gboolean focus_in_event( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	LOG(LOG_DEBUG, "IN : focus_in_event()");

	gtk_window_set_focus(GTK_WINDOW(main_window), word_entry);
	entry_focus_in = TRUE;

	LOG(LOG_DEBUG, "OUT : focus_in_event()");
	return(FALSE);
}

static void style_set_event( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	LOG(LOG_DEBUG, "IN : style_set_event()");

	style_set = TRUE;

	LOG(LOG_DEBUG, "OUT : style_set_event()");
}

static gint key_press_event(GtkWidget *widget, GdkEventKey *event){

	if(entry_focus_in)
		return(FALSE);

	if(ebook_search_method() != SEARCH_METHOD_MULTI){
		gtk_window_set_focus(GTK_WINDOW(main_window), word_entry);
	}

	entry_focus_in = TRUE;
	return(FALSE);
}
static gint entry_focus_out_event(GtkWidget *widget, GdkEventKey *event)
{
	entry_focus_in = FALSE;
#ifdef __WIN32__
	gtk_editable_select_region(GTK_EDITABLE(word_entry), 
				   0,
				   GTK_ENTRY(word_entry)->text_length);
#endif
	return(FALSE);
}

static gint entry_focus_in_event(GtkWidget *widget, GdkEventKey *event)
{
	entry_focus_in = TRUE;
	return(FALSE);
}

static void unset_focus(GtkWidget *widget, gpointer data){
	GTK_WIDGET_UNSET_FLAGS(widget, GTK_CAN_FOCUS);
	if(GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget), unset_focus, NULL);

}

void create_main_window()
{
	gchar title[32];
	GtkWidget *vbox;
	GtkWidget *widget;
	GdkPixbuf *pixbuf;

	LOG(LOG_DEBUG, "IN: create_main_window()");

	tooltip = gtk_tooltips_new();

	hidden_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	hidden_entry = gtk_entry_new();
	
	g_signal_connect (G_OBJECT(hidden_entry), "selection_received",
			    G_CALLBACK(selection_received), NULL);

	gtk_container_add (GTK_CONTAINER (hidden_window), hidden_entry);
	gtk_widget_realize(hidden_window);

	sprintf(title, "EBView %s", VERSION);
	main_window = gtk_widget_new (GTK_TYPE_WINDOW,
				"type", GTK_WINDOW_TOPLEVEL,
				 "title", title,
				 "allow-shrink", TRUE,
				 "allow-grow", TRUE,
				 "default-width", window_width,
				 "default-height", window_height,
				NULL);

	gtk_window_move(GTK_WINDOW(main_window), window_x, window_y);
	gtk_window_set_wmclass(GTK_WINDOW(main_window), "Main", "EBView");

	g_signal_connect (G_OBJECT (main_window), "delete_event",
			    G_CALLBACK(delete_event), NULL);

	g_signal_connect (G_OBJECT (main_window), "style_set",
			    G_CALLBACK(style_set_event), NULL);

	g_signal_connect(G_OBJECT(main_window),"key_press_event",
			 G_CALLBACK(key_press_event), NULL);

#if 0
	g_signal_connect (G_OBJECT(window), "remote_command",
			    G_CALLBACK(remote_command), NULL);
#endif

	gtk_widget_realize(main_window);
	pixbuf = create_pixbuf(IMAGE_EBVIEW);
	gtk_window_set_icon (GTK_WINDOW(main_window), pixbuf);
	destroy_pixbuf(pixbuf);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add (GTK_CONTAINER (main_window), vbox);

	widget = create_dict_window();
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(widget), TRUE, TRUE, 0);
	
	gtk_widget_show(vbox);

	gtk_widget_set_size_request(note_tree, tree_width, tree_height);
	gtk_widget_show_all (main_window);

	gtk_selection_owner_set(main_window, GDK_SELECTION_PRIMARY,GDK_CURRENT_TIME);

	if(!bshow_menu_bar)
		hide_menu_bar();
	if(!bshow_dict_bar)
		hide_dict_bar();
	if(!bshow_status_bar)
		hide_status_bar();

	install_shortcut();


	// Prevents the cursor from going to widgets except the keyword entry box.
	gtk_container_foreach(GTK_CONTAINER(main_window), unset_focus, NULL);
	GTK_WIDGET_SET_FLAGS(word_entry, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(main_view, GTK_CAN_FOCUS);

	LOG(LOG_DEBUG, "OUT: create_main_window()");

}

void restart_main_window()
{
	LOG(LOG_DEBUG, "IN : restart_main_window()");


	calculate_font_size();
	create_text_buffer();

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(main_view), text_buffer);

#if 0
	// Save current size and position
	gdk_window_get_root_origin(main_window->window, &window_x, &window_y);
	window_width = main_window->allocation.width;
	window_height = main_window->allocation.height;

	tree_width = note_tree->allocation.width;
	tree_height = note_tree->allocation.height;


	gtk_widget_destroy(note_tree);
	gtk_widget_destroy(main_window);

	create_main_window();
#endif

	if(current_result != NULL){
		show_result_tree();
		show_result(current_result, FALSE, TRUE);
	} else {
		//show_about();
	}

	show_multi();

	LOG(LOG_DEBUG, "OUT: restart_main_window()");
}

GtkWidget *create_dict_window()
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *image;
	GtkWidget *menu_box;
	GtkWidget *menubar;
	GtkWidget *dictbar;
	GtkWidget *separator;
	GtkWidget *button_up, *button_down;
	gint i;
	GList *method_list=NULL;
	GtkWidget *widget;

	LOG(LOG_DEBUG, "IN : create_dict_window()");

	vbox = gtk_vbox_new(FALSE, 0);

	menu_box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), menu_box, FALSE, TRUE, 0);


	menubar = create_main_menu();
	gtk_box_pack_start(GTK_BOX(menu_box), menubar, TRUE, TRUE, 0);

	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),separator, FALSE, FALSE, 0);


	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 1);


	label = gtk_label_new(_("Search Word"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);


	combo_word = gtk_combo_new();
	gtk_box_pack_start(GTK_BOX(hbox), combo_word, TRUE, TRUE, 0);

	word_entry = GTK_COMBO(combo_word)->entry;
	gtk_combo_disable_activate(GTK_COMBO(combo_word));
	gtk_combo_set_case_sensitive(GTK_COMBO(combo_word), TRUE);

	g_signal_connect(G_OBJECT(word_entry),"activate",
			 G_CALLBACK(entry_activate_event), NULL);

	g_signal_connect(G_OBJECT(word_entry),"focus_out_event",
			 G_CALLBACK(entry_focus_out_event), NULL);

	g_signal_connect(G_OBJECT(word_entry),"focus_in_event",
			 G_CALLBACK(entry_focus_in_event), NULL);

	gtk_tooltips_set_tip(tooltip, word_entry, _("Type word here. You can type multiple space-separated words for keyword search. For file search, specify words or regular expression."),"Private");

	gtk_window_set_focus(GTK_WINDOW(main_window), word_entry);

	if(word_history != NULL)	
		gtk_combo_set_popdown_strings( GTK_COMBO(combo_word), word_history) ;
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_word)->entry), "");


	button_start = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button_start), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(hbox), button_start, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button_start), "clicked",
			 G_CALLBACK(start_search),
			 (gpointer)button_start);

	gtk_tooltips_set_tip(tooltip, button_start, _("Start search"),"Private");
	image = gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button_start), image);


	separator = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox), separator, FALSE, FALSE, 5);

	// Search method
	for(i=0 ; search_method[i].name != 0 ; i ++){
		search_method[i].name = _(search_method[i].name);
	}

	combo_method = gtk_combo_new();
	gtk_widget_set_size_request(GTK_WIDGET(combo_method), 150, 10);
	gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo_method)->entry), FALSE);

	for(i=0 ; ; i ++){
		if(search_method[i].name == NULL)
			break;
		method_list = g_list_append(method_list, search_method[i].name);
	}
	if(i != 0)
		gtk_combo_set_popdown_strings( GTK_COMBO(combo_method), method_list) ;

	gtk_box_pack_start(GTK_BOX (hbox), combo_method, FALSE, TRUE, 0);
	gtk_tooltips_set_tip(tooltip, GTK_COMBO(combo_method)->entry, _("Select search method."),"Private");
	handler_method = g_signal_connect(G_OBJECT (GTK_COMBO(combo_method)->entry), "changed",
					  G_CALLBACK(method_changed),
					  NULL);

#if 0
	separator = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox), separator, FALSE, FALSE, 5);


	button_auto = gtk_toggle_button_new();
	gtk_button_set_relief(GTK_BUTTON(button_auto), GTK_RELIEF_NONE);

	gtk_box_pack_start(GTK_BOX (hbox), button_auto, FALSE, FALSE, 0);

	pixbuf_auto = create_pixbuf(IMAGE_SELECTION);
	pixbuf_auto_checked = create_pixbuf(IMAGE_SELECTION2);
	if(bauto_lookup)
		image = gtk_image_new_from_pixbuf(pixbuf_auto_checked);
	else
		image = gtk_image_new_from_pixbuf(pixbuf_auto);
	gtk_container_add(GTK_CONTAINER(button_auto), image);

	g_signal_connect(G_OBJECT (button_auto), "toggled",
			 G_CALLBACK(toggle_auto_callback),
			 NULL);
	gtk_tooltips_set_tip(tooltip, button_auto, _("When enabled, X selection is searched automatically"),"Private");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_auto), bauto_lookup);

	button_popup = gtk_toggle_button_new();
	gtk_button_set_relief(GTK_BUTTON(button_popup), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX (hbox), button_popup, FALSE, FALSE, 0);

	pixbuf_popup = create_pixbuf(IMAGE_POPUP);
	pixbuf_popup_checked = create_pixbuf(IMAGE_POPUP2);
	if(bshow_popup)
		image = gtk_image_new_from_pixbuf(pixbuf_popup_checked);
	else
		image = gtk_image_new_from_pixbuf(pixbuf_popup);
	gtk_container_add(GTK_CONTAINER(button_popup), image);

	g_signal_connect(G_OBJECT (button_popup), "toggled",
			 G_CALLBACK(toggle_popup_callback),
			 NULL);
	gtk_tooltips_set_tip(tooltip, button_popup, _("When enabled, result of X selection search will be shown in popup window"),"Private");

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_popup), bshow_popup);
	gtk_widget_set_sensitive(button_popup, bauto_lookup);

#endif

	separator = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox), separator, FALSE, FALSE, 5);

	button_up = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button_up), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(hbox),button_up, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button_up), "pressed",
			 G_CALLBACK(dict_backward_text),
			 NULL);
	gtk_tooltips_set_tip(tooltip, button_up, _("Previous Item"),"Private");

	image = gtk_image_new_from_stock(GTK_STOCK_GO_UP, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button_up), image);


	button_down = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button_down), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(hbox),button_down, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button_down), "pressed",
			 G_CALLBACK(dict_forward_text),
			 NULL);
	gtk_tooltips_set_tip(tooltip, button_down, _("Next Item"),"Private");

	image = gtk_image_new_from_stock(GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button_down), image);


	separator = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox), separator, FALSE, FALSE, 5);

	button_forward = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button_forward), GTK_RELIEF_NONE);
	gtk_box_pack_end(GTK_BOX(hbox),button_forward, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button_forward), "pressed",
			 G_CALLBACK(dict_history_forward),
			 NULL);
	gtk_tooltips_set_tip(tooltip, button_forward, _("show next in history"),"Private");

	image = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button_forward), image);

	button_back = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button_back), GTK_RELIEF_NONE);
	gtk_box_pack_end(GTK_BOX(hbox),button_back, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button_back), "pressed",
			 G_CALLBACK(dict_history_back),
			 NULL);
	gtk_tooltips_set_tip(tooltip, button_back, _("show previous in history"),"Private");
	image = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button_back), image);

	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),separator, FALSE, FALSE, 0);

	// Dictionary bar
	note_bar = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(note_bar), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(note_bar), FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), note_bar, FALSE, FALSE, 0);

	dictbar = create_dict_bar();
	gtk_notebook_append_page(GTK_NOTEBOOK(note_bar), dictbar, NULL);

	dictbar = create_grep_bar();
	gtk_notebook_append_page(GTK_NOTEBOOK(note_bar), dictbar, NULL);


	if(pane_direction == 0)
		pane = gtk_hpaned_new();
	else
		pane = gtk_vpaned_new();
	gtk_box_pack_start(GTK_BOX(vbox), pane, TRUE, TRUE, 0);


	note_tree = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(note_tree), tab_position);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(note_tree), FALSE);
	if(bshow_tree_tab == 1)
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(note_tree), TRUE);
	else 
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(note_tree), FALSE);

	gtk_paned_add1 (GTK_PANED(pane), note_tree);

	handler_notebook = g_signal_connect(G_OBJECT (note_tree), "switch_page",
					    G_CALLBACK(note_switch_page),
					    NULL);
	// EBook page
	widget = create_headword_tree();
	//image = create_image(IMAGE_LIST);
	image = gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_notebook_append_page(GTK_NOTEBOOK(note_tree), widget, image);


	// Multi search tree
	widget =  create_multi_tree();
	//image = create_image(IMAGE_MULTI);
	image = gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_notebook_append_page(GTK_NOTEBOOK(note_tree), widget, image);


	// Web page
	widget = create_web_tree();
	image = create_image(IMAGE_GLOBE);
	//image = gtk_image_new_from_stock("panel-internet", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_notebook_append_page(GTK_NOTEBOOK(note_tree), widget, image);

	// Directory tree page
	widget = create_directory_tree();
	//image = create_image(IMAGE_HTML);
	image = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_notebook_append_page(GTK_NOTEBOOK(note_tree), widget, image);


	// Right half
	note_text = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(note_text), GTK_POS_BOTTOM);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(note_text), FALSE);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(note_text), FALSE);
	gtk_paned_add2 (GTK_PANED(pane), note_text);

	// Text buffer to show text.
	widget = create_main_view();

	label = gtk_label_new(_("Text"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note_text), widget, label);


	// Candidate page

	entry_box = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(entry_box), 5);

	label = gtk_label_new(_("Candidate"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note_text),entry_box, label);



	status_bar = gtk_statusbar_new();
	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar),
						  "mycontext");
	gtk_box_pack_end(GTK_BOX(vbox), status_bar, FALSE, TRUE, 0);


	gtk_widget_show_all(vbox);


	LOG(LOG_DEBUG, "OUT : create_dict_window()");

	return(vbox);
}

#define EBOOK_MAX_KEYWORDS 256

void show_result(RESULT *result, gboolean save_history, gboolean reverse_keyword)
{
	set_current_result(result);

	if(result->type == RESULT_TYPE_EB) {
		show_dict(result, save_history, reverse_keyword);
	} else if(result->type == RESULT_TYPE_GREP) {
		show_file(result);
	} else {
		LOG(LOG_ERROR, "show_result : Unknown type %d", result->type);
		exit(1);
	}
}

void show_dict(RESULT *result, gboolean save_history, gboolean reverse_keyword)
{
	gchar *euc_str;
	gchar *text;

	LOG(LOG_DEBUG, "IN : show_dict(save=%d, reverse=%d)", save_history, reverse_keyword);

	g_assert(result != NULL);

	text = ebook_get_text(result->data.eb.book_info,
			      result->data.eb.pos_text.page,
			      result->data.eb.pos_text.offset);

	if(text == NULL){
		LOG(LOG_DEBUG, "OUT : show_dict()");
	}

	if((reverse_keyword == TRUE) && (result->word != NULL)){
		euc_str = iconv_convert("utf-8", "euc-jp", result->word);

		show_text(result->data.eb.book_info, text, euc_str);
		g_free(euc_str);
	} else {
		show_text(result->data.eb.book_info, text, NULL);
	}
	g_free(text);

	if(save_history)
		save_result_history(result);

	update_dump();

	LOG(LOG_DEBUG, "OUT : show_dict()");
}

void show_text(BOOK_INFO *binfo, char *text, gchar *word)
{
	gint length;
	DRAW_TEXT l_text;
	GtkTextIter iter;
	CANVAS canvas;

	LOG(LOG_DEBUG, "IN : show_text()");

	g_assert(text != NULL);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 0);
#if 0
	{

		gchar *utf_str;
		PangoLayout *layout;
		PangoContext *context;

		utf_str = iconv_convert("euc-jp", "utf-8", text);

	layout = gtk_widget_create_pango_layout(main_area, utf_str);

	gdk_draw_layout(main_area->window, 
		      main_area->style->fg_gc[GTK_WIDGET_STATE(main_area)],
		      10,10,
		      layout);
	g_free(utf_str);
	return;

	}
#endif
	clear_text_buffer();
	gtk_text_buffer_get_start_iter (text_buffer, &iter);

	length = strlen(text);
	if(text[length-1] == '\n'){
		text[length-1] = '\0';
		length --;
	}

	// Rewind scroll bar position
	gtk_adjustment_set_value(
		gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(dict_scroll)), 0);

	gtk_adjustment_set_value(
		gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(dict_scroll)), 0);

	l_text.text = text;
	l_text.length = length;

	canvas.buffer = text_buffer;
	canvas.iter = &iter;
	canvas.indent = 0;

	draw_content(&canvas, &l_text, binfo, NULL, word);

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(main_view), text_buffer);
	gtk_adjustment_set_value(
		gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(dict_scroll)), 0);

	// Rewind cursor position
	gtk_text_buffer_get_start_iter(text_buffer, &iter);
	gtk_text_buffer_place_cursor(text_buffer, &iter);

	LOG(LOG_DEBUG, "OUT : show_text()");

}

void select_any_search()
{

	LOG(LOG_DEBUG, "IN : select_any_search()");

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Automatic Search"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 0);
	change_search_menu(SEARCH_METHOD_AUTOMATIC);

	LOG(LOG_DEBUG, "OUT : select_any_search()");
}

void select_exactword_search()
{
	LOG(LOG_DEBUG, "IN : select_exactword_search()");

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Exactword Search"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 0);

	change_search_menu(SEARCH_METHOD_EXACTWORD);

	LOG(LOG_DEBUG, "OUT : select_exactword_search()");
}

void select_word_search()
{
	LOG(LOG_DEBUG, "IN : select_word_search()");

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Forward Search"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 0);

	change_search_menu(SEARCH_METHOD_WORD);

	LOG(LOG_DEBUG, "OUT : select_word_search()");
}

void select_endword_search()
{
	LOG(LOG_DEBUG, "IN : select_endword_search()");

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Backward Search"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 0);

	change_search_menu(SEARCH_METHOD_ENDWORD);

	LOG(LOG_DEBUG, "OUT : select_endword_search()");
}

void select_keyword_search()
{
	LOG(LOG_DEBUG, "IN : select_keyword_search()");

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Keyword Search"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 0);

	change_search_menu(SEARCH_METHOD_KEYWORD);

	LOG(LOG_DEBUG, "OUT : select_keyword_search()");
}

void select_multi_search()
{
	LOG(LOG_DEBUG, "IN : select_multi_search()");

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Multiword Search"));;
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 1);

	change_search_menu(SEARCH_METHOD_MULTI);

	LOG(LOG_DEBUG, "OUT : select_multi_search()");
}

void select_fulltext_search()
{
	LOG(LOG_DEBUG, "IN : select_fulltext_search()");

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Fulltext Search"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 0);

	change_search_menu(SEARCH_METHOD_FULL_TEXT);

	LOG(LOG_DEBUG, "OUT : select_fulltext_search()");
}


void select_internet_search()
{
	LOG(LOG_DEBUG, "IN : select_internet_search()");

	eb_web = 1;
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 0);
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("Internet Search"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 2);

	change_search_menu(SEARCH_METHOD_INTERNET);

	LOG(LOG_DEBUG, "OUT : select_internet_search()");
}

void select_grep_search()
{
	LOG(LOG_DEBUG, "IN : select_grep_search()");

	eb_web = 0;
	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 0);
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry), _("File Search"));

	change_search_menu(SEARCH_METHOD_GREP);

	LOG(LOG_DEBUG, "OUT : select_grep_search()");
}


