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

#include "ebview.h"
#include "mainwindow.h"
#include "mainmenu.h"
#include "dictbar.h"
#include "dump.h"
#include "multi.h"
#include "menu.h"
#include "preference.h"
#include "pref_io.h"
#include "selection.h"
#include "shortcut.h"
#include "statusbar.h"
#include "textview.h"

#include <gdk/gdkkeysyms.h>


static GtkWidget *menuitem_automatic;
static GtkWidget *menuitem_word;
static GtkWidget *menuitem_endword;
static GtkWidget *menuitem_exactword;
static GtkWidget *menuitem_keyword;
static GtkWidget *menuitem_multi;
static GtkWidget *menuitem_menu;
static GtkWidget *menuitem_copyright;
static GtkWidget *menuitem_fulltext;
static GtkWidget *menuitem_internet;
static GtkWidget *menuitem_grep;

GtkWidget *display_menubar;
GtkWidget *display_statusbar;
GtkWidget *display_dictbar;
GtkWidget *display_treetab;

static GtkWidget *menuitem_emphasize;
static GtkWidget *menuitem_image;

static GtkWidget *menuitem_filename;
static GtkWidget *menuitem_sortbydict;

extern GtkWidget *note_tree;
extern GtkWidget *note_text;
extern GtkWidget *pane;

static GtkWidget *menubar=NULL;
static GtkWidget *item_search=NULL;

void set_tab_position(GtkPositionType position);

gint menuitem_handler (GtkWidget *widget, gchar *string)
{

	LOG(LOG_DEBUG, "IN : menuitem_handler(%s)", string);

	if(bstarting_up)
		return(0);
	

	if(strcmp(string, "file.exit") == 0){
		exit_program(NULL, NULL);
		return(0);
	}

	if(strcmp(string, "search.all") == 0){
		select_any_search();
		return(0);
	}

	if(strcmp(string, "search.exact") == 0){
		select_exactword_search();
		return(0);
	}

	if(strcmp(string, "search.word") == 0){
		select_word_search();
		return(0);
	}

	if(strcmp(string, "search.endword") == 0){
		select_endword_search();
		return(0);
	}

	if(strcmp(string, "search.keyword") == 0){
		select_keyword_search();
		return(0);
	}

	if(strcmp(string, "search.multi") == 0){
		select_multi_search();
		show_multi();
		return(0);
	}

	if(strcmp(string, "search.fulltext") == 0){
		select_fulltext_search();
		return(0);
	}

	if(strcmp(string, "search.internet") == 0){
		select_internet_search();
		return(0);
	}

	if(strcmp(string, "search.grep") == 0){
		select_grep_search();
		return(0);
	}

	if(strcmp(string, "search.menu") == 0){
		show_menu();
		return(0);
	}


	if(strcmp(string, "search.copyright") == 0){
		show_copyright();
		return(0);
	}

	if(strcmp(string, "view.menubar") == 0){
		if(GTK_CHECK_MENU_ITEM(display_menubar)->active) {
			show_menu_bar();
		} else {
			hide_menu_bar();
		}
		return(0);
	}

	if(strcmp(string, "view.statusbar") == 0){
		if(GTK_CHECK_MENU_ITEM(display_statusbar)->active) {
			show_status_bar();
		} else {
			hide_status_bar();
		}
		return(0);
	}

	if(strcmp(string, "view.dictbar") == 0){
		if(GTK_CHECK_MENU_ITEM(display_dictbar)->active) {
			show_dict_bar();
		} else {
			hide_dict_bar();
		}
		return(0);
	}

	if(strcmp(string, "view.treetab") == 0){
		if(GTK_CHECK_MENU_ITEM(display_treetab)->active) {
			show_tree_tab();
		} else {
			hide_tree_tab();
		}
		return(0);
	}

	if(strcmp(string, "view.emphasize") == 0){
		if(GTK_CHECK_MENU_ITEM(menuitem_emphasize)->active) {
			bemphasize_keyword = TRUE;
		} else {
			bemphasize_keyword = FALSE;
		}

		if(current_result != NULL){
			show_result(current_result, FALSE, TRUE);
		}
		save_preference();
		return(0);
	}

	if(strcmp(string, "view.image") == 0){
		if(GTK_CHECK_MENU_ITEM(menuitem_image)->active) {
			bshow_image = TRUE;
		} else {
			bshow_image = FALSE;
		}

		if(current_result != NULL){
			show_result(current_result, FALSE, TRUE);
		}
		save_preference();
		return(0);
	}

	if(strcmp(string, "view.expand") == 0){
		expand_lines();
		return(0);
	}

	if(strcmp(string, "view.shrink") == 0){
		shrink_lines();
		return(0);
	}

	if(strcmp(string, "view.increase") == 0){
		increase_font_size();
		return(0);
	}

	if(strcmp(string, "view.decrease") == 0){
		decrease_font_size();
		return(0);
	}

	if(strcmp(string, "result.sort") == 0){
		if(GTK_CHECK_MENU_ITEM(menuitem_sortbydict)->active) {
			bsort_by_dictionary = TRUE;
		} else {
			bsort_by_dictionary = FALSE;
		}

		save_preference();

		return(0);
	}

	if(strcmp(string, "result.filename") == 0){
		if(GTK_CHECK_MENU_ITEM(menuitem_filename)->active) {
			bshow_filename = TRUE;
		} else {
			bshow_filename = FALSE;
		}

		save_preference();

		return(0);
	}

	if(strcmp(string, "split.horizontal") == 0){
		split_horizontal();
		return(0);
	}

	if(strcmp(string, "split.vertical") == 0){
		split_vertical();
		return(0);
	}

	if(strcmp(string, "tab.top") == 0){
		set_tab_position(GTK_POS_TOP);
		return(0);
	}

	if(strcmp(string, "tab.bottom") == 0){
		set_tab_position(GTK_POS_BOTTOM);
		return(0);
	}

	if(strcmp(string, "tab.right") == 0){
		set_tab_position(GTK_POS_RIGHT);
		return(0);
	}

	if(strcmp(string, "tab.left") == 0){
		set_tab_position(GTK_POS_LEFT);
		return(0);
	}


/*
	if(strcmp(string, "pref.dict") == 0){
		preference_dictgroup();
		return(0);
	}

	if(strcmp(string, "pref.ending") == 0){
		preference_ending();
		return(0);
	}

	if(strcmp(string, "pref.shortcut") == 0){
		preference_shortcut();
		return(0);
	}

	if(strcmp(string, "pref.web") == 0){
		preference_weblist();
		return(0);
	}

	if(strcmp(string, "pref.external") == 0){
		preference_external();
		return(0);
	}

	if(strcmp(string, "pref.font") == 0){
		preference_font();
		return(0);
	}

	if(strcmp(string, "pref.color") == 0){
		preference_color();
		return(0);
	}

	if(strcmp(string, "pref.misc") == 0){
		misc_preference();
		return(0);
	}

*/

	if(strcmp(string, "pref") == 0){
		show_preference();
		return(0);
	}

	if(strcmp(string, "dump.hex") == 0){
		dump_hex();
		return(0);
	}

	if(strcmp(string, "dump.text") == 0){
		dump_text();
		return(0);
	}

	if(strcmp(string, "help.usage") == 0){
		show_usage();
		return(0);
	}

	if(strcmp(string, "help.home") == 0){
		show_home();
		return(0);
	}

	if(strcmp(string, "help.about") == 0){
		show_about();
		return(0);
	}

	if(strncmp(string, "sc_", 3) == 0){
		perform_shortcut(string);
		return(0);
	}

	if(strcmp(string, "selection.nothing") == 0){
		selection_mode = SELECTION_DO_NOTHING;
		auto_lookup_stop();
		return(0);
	}

	if(strcmp(string, "selection.copy") == 0){
		selection_mode = SELECTION_COPY_ONLY;
		auto_lookup_start();
		return(0);
	}

	if(strcmp(string, "selection.search") == 0){
		selection_mode = SELECTION_SEARCH;
		auto_lookup_start();
		return(0);
	}

	if(strcmp(string, "selection.searchtop") == 0){
		selection_mode = SELECTION_SEARCH_TOP;
		auto_lookup_start();
		return(0);
	}

	if(strcmp(string, "selection.popup") == 0){
		selection_mode = SELECTION_POPUP;
		auto_lookup_start();
		return(0);
	}

	LOG(LOG_CRITICAL, "Unknown menu : %s\n", string);
	return(0);
}

void change_search_menu(gint method)
{
	GtkWidget *item;

	LOG(LOG_DEBUG, "IN : change_search_menu(%d)", method);

	switch(method){
	case SEARCH_METHOD_AUTOMATIC:
		item = menuitem_automatic;
		break;
	case SEARCH_METHOD_WORD:
		item = menuitem_word;
		break;
	case SEARCH_METHOD_ENDWORD:
		item = menuitem_endword;
		break;
	case SEARCH_METHOD_EXACTWORD:
		item = menuitem_exactword;
		break;
	case SEARCH_METHOD_KEYWORD:
		item = menuitem_keyword;
		break;
	case SEARCH_METHOD_MULTI:
		item = menuitem_multi;
		break;
	case SEARCH_METHOD_COPYRIGHT:
		item = menuitem_copyright;
		break;
	case SEARCH_METHOD_FULL_TEXT:
		item = menuitem_fulltext;
		break;
	case SEARCH_METHOD_INTERNET:
		item = menuitem_internet;
		break;
	case SEARCH_METHOD_GREP:
		item = menuitem_grep;
		break;
	default:
		LOG(LOG_INFO, "Unknown search method");
		LOG(LOG_DEBUG, "OUT : change_search_menu()");
		return;
		break;
	}



	g_signal_handlers_block_matched(G_OBJECT(menuitem_automatic), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_word), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_endword), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_exactword), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_keyword), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_multi), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_copyright), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_fulltext), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_internet), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_block_matched(G_OBJECT(menuitem_grep), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);

	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_automatic), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_word), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_endword), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_exactword), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_keyword), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_multi), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_copyright), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_fulltext), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_internet), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);
	g_signal_handlers_unblock_matched(G_OBJECT(menuitem_grep), G_SIGNAL_MATCH_FUNC, 0, 0, 0, menuitem_handler, 0);


	LOG(LOG_DEBUG, "OUT : change_search_menu()");
}

static GtkWidget *create_search_menu()
{
	GtkWidget *menu;
	GtkWidget *item;
	GSList    *group=NULL;

	LOG(LOG_DEBUG, "IN : create_search_menu()");

	// Search method
	menu = gtk_menu_new();

	menuitem_automatic = gtk_radio_menu_item_new_with_label(group, _("Automatic Search"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_automatic);
	g_signal_connect(G_OBJECT(menuitem_automatic), "activate",
			 G_CALLBACK(menuitem_handler), 
			 (gpointer)"search.all");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_automatic));


	menuitem_exactword = gtk_radio_menu_item_new_with_label(group, _("Exactword Search"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_exactword);
	g_signal_connect(G_OBJECT(menuitem_exactword), "activate",
			 G_CALLBACK(menuitem_handler), 
			 (gpointer)"search.exact");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_exactword));
	menuitem_word = gtk_radio_menu_item_new_with_label(group, _("Forward Search"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_word);
	g_signal_connect(G_OBJECT(menuitem_word), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"search.word");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_word));

	menuitem_endword = gtk_radio_menu_item_new_with_label(group, _("Backward Search"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_endword);
	g_signal_connect(G_OBJECT(menuitem_endword), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"search.endword");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_endword));

	menuitem_keyword = gtk_radio_menu_item_new_with_label(group, _("Keyword Search"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_keyword);
	g_signal_connect(G_OBJECT(menuitem_keyword), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"search.keyword");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_keyword));

	menuitem_multi = gtk_radio_menu_item_new_with_label(group, _("Multiword Search"));;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_multi);
	g_signal_connect(G_OBJECT(menuitem_multi), "activate",
			 G_CALLBACK(menuitem_handler), 
			 (gpointer)"search.multi");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_multi));

	menuitem_fulltext = gtk_radio_menu_item_new_with_label(group, _("Fulltext Search"));;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_fulltext);
	g_signal_connect(G_OBJECT(menuitem_fulltext), "activate",
			 G_CALLBACK(menuitem_handler), 
			 (gpointer)"search.fulltext");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_fulltext));

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	menuitem_menu = gtk_radio_menu_item_new_with_label(group, _("Menu"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_menu);
	g_signal_connect(G_OBJECT(menuitem_menu), "activate",
			 G_CALLBACK(menuitem_handler), 
			 (gpointer)"search.menu");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_menu));

	menuitem_copyright = gtk_radio_menu_item_new_with_label(group, _("Copyright"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_copyright);
	g_signal_connect(G_OBJECT(menuitem_copyright), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"search.copyright");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_menu));

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	menuitem_internet = gtk_radio_menu_item_new_with_label(group, _("Internet Search"));;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_internet);
	g_signal_connect(G_OBJECT(menuitem_internet), "activate",
			 G_CALLBACK(menuitem_handler), 
			 (gpointer)"search.internet");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_internet));

	menuitem_grep = gtk_radio_menu_item_new_with_label(group, _("File Search"));;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_grep);
	g_signal_connect(G_OBJECT(menuitem_grep), "activate",
			 G_CALLBACK(menuitem_handler), 
			 (gpointer)"search.grep");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem_grep));

	LOG(LOG_DEBUG, "OUT : create_search_menu()");
	
	return(menu);
}

GtkWidget *create_main_menu()
{
	GtkWidget *menu;
	GtkWidget *item;
	GtkWidget *item_tool;
	GtkWidget *toolbar_menu;
	GtkWidget *split_menu;
	GtkWidget *tab_menu;
	GtkWidget *dump_menu;
	GtkWidget *contents_menu;
	GtkWidget *result_menu;
	GtkWidget *selection_menu;
	GSList    *group=NULL;


	LOG(LOG_DEBUG, "IN : create_main_menu()");

/*
	if(menubar)
		gtk_widget_destroy(menubar);
*/

	menubar = gtk_menu_bar_new();

	// Program menu
	menu = gtk_menu_new();

	item = gtk_menu_item_new_with_label(_("Exit"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler), 
			 (gpointer)"file.exit");
	item = gtk_menu_item_new_with_label(_("File"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item);


	// Display menu
	menu = gtk_menu_new();

	item = gtk_menu_item_new_with_label(_("Show/Hide"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);


	toolbar_menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), toolbar_menu);

	display_menubar = gtk_check_menu_item_new_with_label(_("Menu Bar"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_menubar),
				       bshow_menu_bar);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolbar_menu), display_menubar);
	g_signal_connect(G_OBJECT(display_menubar), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.menubar");


	display_dictbar = gtk_check_menu_item_new_with_label(_("Dictionary Selection Bar"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_dictbar),
				       bshow_dict_bar);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolbar_menu), display_dictbar);
	g_signal_connect(G_OBJECT(display_dictbar), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.dictbar");


	display_statusbar = gtk_check_menu_item_new_with_label(_("Status Bar"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_statusbar), 				       bshow_status_bar);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolbar_menu), display_statusbar);
	g_signal_connect(GTK_OBJECT(display_statusbar), "activate",
				  G_CALLBACK(menuitem_handler),
				  (gpointer)"view.statusbar");

	display_treetab = gtk_check_menu_item_new_with_label(_("Tree Pane Tab"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_treetab), 				       bshow_tree_tab);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolbar_menu), display_treetab);
	g_signal_connect(G_OBJECT(display_treetab), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.treetab");

	// line
	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label(_("Contents"));	
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	contents_menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), contents_menu);


	menuitem_emphasize = gtk_check_menu_item_new_with_label(_("Emphasize Keyword"));	
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_emphasize),
 				       bemphasize_keyword);
	gtk_menu_shell_append(GTK_MENU_SHELL(contents_menu), menuitem_emphasize);
	g_signal_connect(G_OBJECT(menuitem_emphasize), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.emphasize");

	menuitem_image = gtk_check_menu_item_new_with_label(_("Show Image Inline"));	
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_image),
 				       bshow_image);
	gtk_menu_shell_append(GTK_MENU_SHELL(contents_menu), menuitem_image);
	g_signal_connect(G_OBJECT(menuitem_image), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.image");

	// line
	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(contents_menu), item);


	// text size
	item = gtk_menu_item_new_with_label(_("Increase Font Size"));	
	gtk_menu_shell_append(GTK_MENU_SHELL(contents_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.increase");

	item = gtk_menu_item_new_with_label(_("Decrease Font Size"));	
	gtk_menu_shell_append(GTK_MENU_SHELL(contents_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.decrease");

	// line
	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(contents_menu), item);

	// Space between lines
	item = gtk_menu_item_new_with_label(_("Expand Lines"));	
	gtk_menu_shell_append(GTK_MENU_SHELL(contents_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.expand");

	item = gtk_menu_item_new_with_label(_("Shrink Lines"));	
	gtk_menu_shell_append(GTK_MENU_SHELL(contents_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"view.shrink");


	// Result list
	item = gtk_menu_item_new_with_label(_("Result List"));	
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	result_menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), result_menu);

	// Sort by dictionary.
	menuitem_sortbydict = gtk_check_menu_item_new_with_label(_("Sort By Dictionary"));	
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_sortbydict),
 				       bsort_by_dictionary);
	gtk_menu_shell_append(GTK_MENU_SHELL(result_menu), menuitem_sortbydict);
	g_signal_connect(G_OBJECT(menuitem_sortbydict), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"result.sort");

	// Show filename
	menuitem_filename = gtk_check_menu_item_new_with_label(_("Show Filename"));	
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_filename),
 				       bshow_filename);
	gtk_menu_shell_append(GTK_MENU_SHELL(result_menu), menuitem_filename);
	g_signal_connect(G_OBJECT(menuitem_filename), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"result.filename");


	// Line
	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);


	// Pane direction
	item = gtk_menu_item_new_with_label(_("Pane Direction"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	split_menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), split_menu);


	group = NULL;

	item = gtk_radio_menu_item_new_with_label(group, _("Horizontal"));
/*
	item = gtk_radio_menu_item_new_with_label(NULL, _("Horizontal"));
*/
	gtk_menu_shell_append(GTK_MENU_SHELL(split_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"split.horizontal");
	if(pane_direction == 0)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_radio_menu_item_new_with_label(group, _("Vertical"));
/*
	item = gtk_radio_menu_item_new_with_label(
		gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item)),
		_("Vertical"));
*/
	gtk_menu_shell_append(GTK_MENU_SHELL(split_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"split.vertical");
	if(pane_direction == 1)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	// Tab position
	item = gtk_menu_item_new_with_label(_("Tab Position"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	tab_menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), tab_menu);

	group = NULL;

	item = gtk_radio_menu_item_new_with_label(group, _("Top"));
	gtk_menu_shell_append(GTK_MENU_SHELL(tab_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"tab.top");
	if(tab_position == GTK_POS_TOP)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_radio_menu_item_new_with_label(group, _("Bottom"));
	gtk_menu_shell_append(GTK_MENU_SHELL(tab_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"tab.bottom");
	if(tab_position == GTK_POS_BOTTOM)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_radio_menu_item_new_with_label(group, _("Left"));
	gtk_menu_shell_append(GTK_MENU_SHELL(tab_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"tab.left");
	if(tab_position == GTK_POS_LEFT)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_radio_menu_item_new_with_label(group, _("Right"));
	gtk_menu_shell_append(GTK_MENU_SHELL(tab_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"tab.right");
	if(tab_position == GTK_POS_RIGHT)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_menu_item_new_with_label(_("View"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item);


	// Search menu
	menu = create_search_menu();

	item_search = gtk_menu_item_new_with_label(_("Search Method"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item_search), menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item_search);

	// Preference menu
	menu = gtk_menu_new();

	item_tool = gtk_menu_item_new_with_label(_("Tools"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item_tool), menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item_tool);

/*
	item = gtk_menu_item_new_with_label(_("Add/Remove Dictionary"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref.dict");

	item = gtk_menu_item_new_with_label(_("Stemming"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref.ending");

	item = gtk_menu_item_new_with_label(_("Shortcut"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref.shortcut");

	item = gtk_menu_item_new_with_label(_("Search Engines"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref.web");

	item = gtk_menu_item_new_with_label(_("External Program"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref.external");

	item = gtk_menu_item_new_with_label(_("Font"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref.font");

	item = gtk_menu_item_new_with_label(_("Color"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref.color");

	item = gtk_menu_item_new_with_label(_("Misc"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref.misc");

*/
	// Selection search
	item = gtk_menu_item_new_with_label(_("Selection"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	selection_menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), selection_menu);

	group = NULL;

	item = gtk_radio_menu_item_new_with_label(group, _("Do Nothing"));
	gtk_menu_shell_append(GTK_MENU_SHELL(selection_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"selection.nothing");
	//if(selection_mode == SELECTION_DO_NOTHING)
	//	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_radio_menu_item_new_with_label(group, _("Copy Only"));
	gtk_menu_shell_append(GTK_MENU_SHELL(selection_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"selection.copy");
	//if(selection_mode == SELECTION_COPY_ONLY)
	//	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_radio_menu_item_new_with_label(group, _("Search In Main Window"));
	gtk_menu_shell_append(GTK_MENU_SHELL(selection_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"selection.search");
	//if(selection_mode == SELECTION_SEARCH)
	//	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_radio_menu_item_new_with_label(group, _("Search In Main Window + Top"));
	gtk_menu_shell_append(GTK_MENU_SHELL(selection_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"selection.searchtop");
	//if(selection_mode == SELECTION_SEARCH_TOP)
	//	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));


	item = gtk_radio_menu_item_new_with_label(group, _("Search In Popup"));
	gtk_menu_shell_append(GTK_MENU_SHELL(selection_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"selection.popup");
	//if(selection_mode == SELECTION_POPUP)
	//	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);


	// Dump
	item = gtk_menu_item_new_with_label(_("Dump"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);


	dump_menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), dump_menu);

	item = gtk_menu_item_new_with_label(_("Hex Dump"));
	gtk_menu_shell_append(GTK_MENU_SHELL(dump_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"dump.hex");

	item = gtk_menu_item_new_with_label(_("Text Dump"));
	gtk_menu_shell_append(GTK_MENU_SHELL(dump_menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"dump.text");

	// line
	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	// Option
	item = gtk_menu_item_new_with_label(_("Options..."));
//	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"pref");

	// Help menu
	menu = gtk_menu_new();

	item = gtk_menu_item_new_with_label(_("Usage"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"help.usage");

	item = gtk_menu_item_new_with_label(_("Show EBView Home"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"help.home");

	item = gtk_menu_item_new_with_label(_("About"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(menuitem_handler),
			 (gpointer)"help.about");


	item = gtk_menu_item_new_with_label(_("Help"));
	//gtk_menu_item_set_right_justified(GTK_MENU_ITEM(item), TRUE);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item);


	gtk_widget_show_all(menubar);

	LOG(LOG_DEBUG, "OUT : create_main_menu()");

	return(menubar);
}

void update_main_menu()
{
	GtkWidget *menu;

	LOG(LOG_DEBUG, "IN : update_main_menu()");

	gtk_menu_item_remove_submenu(GTK_MENU_ITEM(item_search));

	menu = create_search_menu();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item_search), menu);
	gtk_widget_show_all(menu);

	LOG(LOG_DEBUG, "OUT : update_main_menu()");

}

void toggle_menu_bar()
{
	if(bshow_menu_bar == 1){
		hide_menu_bar();
	} else {
		show_menu_bar();
	}
}

void show_menu_bar()
{
	gtk_widget_show(menubar);
	bshow_menu_bar = 1;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_menubar),
				       bshow_menu_bar);

}

void hide_menu_bar()
{
	gtk_widget_hide(menubar);
	bshow_menu_bar = 0;

	gtk_widget_queue_draw(main_window);
	gtk_widget_queue_resize(main_window);
	gtk_container_check_resize(GTK_CONTAINER(main_window));

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_menubar),
				       bshow_menu_bar);

}

void show_tree_tab(){
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(note_tree), TRUE);
	bshow_tree_tab = 1;

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_treetab),
				       bshow_tree_tab);
}

void hide_tree_tab(){
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(note_tree), FALSE);
	bshow_tree_tab = 0;

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_treetab),
				       bshow_tree_tab);
}

void toggle_tree_tab(){
	if(bshow_tree_tab == 1){
		hide_tree_tab();
	} else {
		show_tree_tab();
	}
}

void switch_direction()
{
	if(pane_direction == 0)
		split_vertical();
	else
		split_horizontal();
}

void split_vertical()
{
	GtkWidget *new_pane;
	GtkWidget *parent;
	gint position;

	LOG(LOG_DEBUG, "IN : split_vertical()");


	// Save current size

	position = gtk_paned_get_position(GTK_PANED(pane));

	// Create new pane
	new_pane = gtk_vpaned_new();
	
	g_object_ref(G_OBJECT(note_tree));
	g_object_ref(G_OBJECT(note_text));

	gtk_container_remove(GTK_CONTAINER(pane), note_tree);
	gtk_container_remove(GTK_CONTAINER(pane), note_text);

	gtk_paned_add1(GTK_PANED(new_pane), note_tree);
	gtk_paned_add2(GTK_PANED(new_pane), note_text);

	g_object_unref(G_OBJECT(note_tree));
	g_object_unref(G_OBJECT(note_text));


	parent = pane->parent;
	gtk_container_remove(GTK_CONTAINER(parent), pane);
	gtk_box_pack_start(GTK_BOX(parent), new_pane, TRUE, TRUE, 0);

	pane = new_pane;


	gtk_paned_set_position(GTK_PANED(pane), position);
	
	gtk_widget_show_all(new_pane);

	pane_direction = 1;

	LOG(LOG_DEBUG, "OUT : split_vertical()");
}

void split_horizontal()
{

	GtkWidget *new_pane;
	GtkWidget *parent;
	gint position;

	LOG(LOG_DEBUG, "IN : split_horizontal()");

	// Save current size
	position = gtk_paned_get_position(GTK_PANED(pane));


	// Create new pane
	new_pane = gtk_hpaned_new();
	
	g_object_ref(G_OBJECT(note_tree));
	g_object_ref(G_OBJECT(note_text));

	gtk_container_remove(GTK_CONTAINER(pane), note_tree);
	gtk_container_remove(GTK_CONTAINER(pane), note_text);

	gtk_paned_add1(GTK_PANED(new_pane), note_tree);
	gtk_paned_add2(GTK_PANED(new_pane), note_text);

	g_object_unref(G_OBJECT(note_tree));
	g_object_unref(G_OBJECT(note_text));


	parent = pane->parent;
	gtk_container_remove(GTK_CONTAINER(parent), pane);
	gtk_box_pack_start(GTK_BOX(parent), new_pane, TRUE, TRUE, 0);

	pane = new_pane;


	gtk_paned_set_position(GTK_PANED(pane), position);
	
	gtk_widget_show_all(new_pane);

	pane_direction = 0;
	
	LOG(LOG_DEBUG, "OUT : split_horizontal()");
}

void set_tab_position(GtkPositionType position)
{
	tab_position = (gint)position;
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(note_tree), tab_position);
}

