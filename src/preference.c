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

#include <sys/stat.h>

#ifndef __WIN32__
#include <langinfo.h>
#endif

#ifdef __WIN32__
#include <pango/pangowin32.h>
#else
#include <pango/pangox.h>
#endif

#include "dictbar.h"
#include "selection.h"
#include "textview.h"
#include "popup.h"
#include "mainwindow.h"
#include "misc.h"

#include "pref_color.h"
#include "pref_dictgroup.h"
#include "pref_dirgroup.h"
#include "pref_external.h"
#include "pref_font.h"
#include "pref_grep.h"
#include "pref_gui.h"
#include "pref_io.h"
#include "pref_search.h"
#include "pref_selection.h"
#include "pref_shortcut.h"
#include "pref_stemming.h"
#include "pref_weblist.h"

#ifdef __WIN32__
#include <shlobj.h>
#endif

#define DEFAULT_WINDOW_WIDTH    670
#define DEFAULT_WINDOW_HEIGHT   440

#define PREF_DIALOG_WIDTH 640
#define PREF_DIALOG_HEIGHT 480

extern gchar *exe_path;

GtkWidget *pref_dlg;
static GtkWidget *note_pref;

extern void print_dict_group();

struct pref_def {
	gchar *title;
	gboolean is_child;
	GtkWidget *(* start_func)();
	gboolean (* end_func)();
};

struct pref_def prefs[] = {
	{ N_("Appearance"), FALSE, NULL, NULL},
	{ N_("Font"), TRUE, pref_start_font, pref_end_font},
	{ N_("Color"), TRUE, pref_start_color, pref_end_color},
	{ N_("Misc."), TRUE, pref_start_gui, pref_end_gui},
	{ N_("Dictionary Search"), FALSE, NULL, NULL},
	{ N_("Dictionary Group"), TRUE, pref_start_dictgroup, pref_end_dictgroup},
	{ N_("Selection"), TRUE, pref_start_selection, pref_end_selection},
	{ N_("Stemming"), TRUE, pref_start_stemming, pref_end_stemming},
	{ N_("Misc"), TRUE, pref_start_search, pref_end_search},
	{ N_("File Search"), FALSE, NULL, NULL},
	{ N_("Directory Group"), TRUE, pref_start_dirgroup, pref_end_dirgroup},
	{ N_("Filter"), TRUE, pref_start_filter, pref_end_filter},
	{ N_("Cache"), TRUE, pref_start_cache, pref_end_cache},
	{ N_("Misc."), TRUE, pref_start_grep, pref_end_grep},
	{ N_("Shortcut"), FALSE, pref_start_shortcut, pref_end_shortcut},
	{ N_("Internet Search"), FALSE, pref_start_weblist, pref_end_weblist},
	{ N_("External Program"), FALSE, pref_start_external, pref_end_external},
	{NULL, FALSE, NULL, NULL}};




void initialize_preference(){
	gchar *home_dir;
	GDir *dir;
	const gchar *name;
	gchar fullpath[512];

#ifdef __WIN32__
	gchar *p;
	gchar *rc_path;
	LPITEMIDLIST idl;
	LPMALLOC     im;
	gchar apppath[_MAX_PATH];
#endif

	LOG(LOG_DEBUG, "IN : initialize_preference()");

	bbeep_on_nohit = TRUE;
	selection_mode = SELECTION_DO_NOTHING;
	bignore_locks = TRUE;
	line_space = 0.3;
	gaiji_adjustment = 2;

	max_search = 500;
	max_heading = 50;
	max_remember_words = 10;
	dict_button_length = 5;
	auto_interval = 1000;
	auto_minchar = 3;
	auto_maxchar = 64;
	bshow_menu_bar = 1;
	bshow_status_bar = 1;
	bshow_dict_bar = 1;
	bshow_tree_tab = 1;
	bending_correction = 1;
	bending_only_nohit = 1;
	bshow_popup_title = 1;
	bignore_case = 1;
	bsuppress_hidden_files = 1;
	popup_width = 300;
	popup_height = 200;
	window_x = 0;
	window_y = 0;
	window_width = DEFAULT_WINDOW_WIDTH;
	window_height = DEFAULT_WINDOW_HEIGHT;
	tree_width = 185;
	tree_height = 344;
	pane_direction = 0;
	tab_position = GTK_POS_TOP;
	scroll_step = 10;
	scroll_time = 100000;
	scroll_margin= 30;
	bsmooth_scroll = 1;
	bsort_by_dictionary = 0;
	bemphasize_keyword = 1;
	bshow_image = 1;
	bshow_splash = 1;
	bword_search_automatic = 1;
	bshow_filename = 1;
	bheading_auto_calc = 1;
	benable_button_color = 1;
	bplay_sound_internally=1;

	additional_lines = 10;
	additional_chars = 12;
	cache_size = 50;
	max_bytes_to_guess = 5000;

#ifdef __WIN32__
	fs_codeset = strdup("Shift_JIS");
#else
	fs_codeset = nl_langinfo(CODESET);
#endif


#ifdef __WIN32__
	mpeg_template = strdup("");
	wave_template = strdup("");
	browser_template = strdup("");
	open_template = strdup("\"C:\\Program Files\\Hidemaru\\hidemaru.exe\" /j%l %f");
#else
	mpeg_template = strdup("plaympeg %f");
	wave_template = strdup("playwave %f");
	browser_template = strdup("gnome-moz-remote %f");
	open_template = strdup("emacs +%l %f");
#endif

#ifdef __WIN32__
	fontset_normal = strdup("ms gothic 9");
	fontset_bold = strdup("ms gothic 9");
	fontset_italic = strdup("times new roman Italic 9");
	fontset_superscript = strdup("ms gothic 7");
#else
	fontset_normal = strdup("Kochi Mincho 12");
	fontset_bold = strdup("Kochi Gothic 12");
	fontset_italic = strdup("Sans Italic 12");
	fontset_superscript = strdup("Kochi Gothic 8");
#endif
	color_str[COLOR_LINK] = strdup("#0000c0");
	color_str[COLOR_KEYWORD] = strdup("#c00000");
	color_str[COLOR_SOUND] = strdup("#00c000");
	color_str[COLOR_MOVIE] = strdup("#00c000");
	color_str[COLOR_EMPHASIS] = strdup("#ff0000");
	color_str[COLOR_REVERSE_BG] = strdup("#c0c0c0");


	web_store = gtk_tree_store_new(WEB_N_COLUMNS,
				       G_TYPE_INT,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING);

	dict_store = gtk_tree_store_new(DICT_N_COLUMNS,
					G_TYPE_INT,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_INT,
					G_TYPE_STRING,
					G_TYPE_INT,
					G_TYPE_BOOLEAN,
					G_TYPE_POINTER,
					G_TYPE_BOOLEAN,
					G_TYPE_STRING,
					G_TYPE_STRING);

	stemming_en_store = gtk_list_store_new(STEMMING_N_COLUMNS,
					    G_TYPE_STRING,
					    G_TYPE_STRING);

	stemming_ja_store = gtk_list_store_new(STEMMING_N_COLUMNS,
					    G_TYPE_STRING,
					    G_TYPE_STRING);

	shortcut_store = gtk_list_store_new(SHORTCUT_N_COLUMNS,
					    G_TYPE_UINT,
					    G_TYPE_UINT,
					    G_TYPE_STRING,
					    G_TYPE_STRING,
					    G_TYPE_STRING,
					    G_TYPE_POINTER);

	filter_store = gtk_list_store_new(FILTER_N_COLUMNS,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_BOOLEAN);

	dirgroup_store = gtk_list_store_new(DIRGROUP_N_COLUMNS,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_BOOLEAN);

	// Set directories to external variable.
	// package_dir : Directory which has standard config file.
	// user_dir : Directory which has user defined config file.
	// temp_dir : Directory which has temporary files.
	// cache_dir : Directory which has cache files.

#ifdef __WIN32__
	p = strrchr(exe_path, '\\');
	if(p != NULL){
		*p = '\0';
		home_dir = exe_path;
		package_dir = g_strdup_printf("%s%sdata", exe_path, DIR_DELIMITER);
	} else {
		home_dir = ".";
		package_dir = g_strdup_printf(".%sdata", DIR_DELIMITER);
	}


	SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &idl);
	SHGetPathFromIDList(idl, (LPSTR)&apppath);

	if(SUCCEEDED(SHGetMalloc(&im))){
		im->lpVtbl->Free(im, idl);
		im->lpVtbl->Release(im);
	}

	user_dir = g_strdup_printf("%s%sEBView", apppath, DIR_DELIMITER);
	temp_dir = g_strdup_printf("%s%stmp", user_dir, DIR_DELIMITER);
	cache_dir = g_strdup_printf("%s%scache", user_dir, DIR_DELIMITER);

#else
	home_dir = getenv("HOME");
	package_dir = PACKAGEDIR;

	user_dir = g_strdup_printf("%s%s.%s", home_dir, DIR_DELIMITER, PACKAGE);
	temp_dir = g_strdup_printf("%s%stmp", user_dir, DIR_DELIMITER);
	cache_dir = g_strdup_printf("%s%scache", user_dir, DIR_DELIMITER);

#endif

	if((dir = g_dir_open(user_dir, 0, NULL)) == NULL){
#ifdef __WIN32__
		if(mkdir(user_dir) != 0){
#else
		if(mkdir(user_dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0){
#endif
			LOG(LOG_CRITICAL, "Failed to create directory : %s\n", user_dir);
			exit(1);
		}
	} else {
		g_dir_close(dir);
	}

	// Check temporary directory and create it if it does not exist
	if((dir = g_dir_open(temp_dir, 0, NULL)) == NULL){
#ifdef __WIN32__
		if(mkdir(temp_dir) != 0){
#else
		if(mkdir(temp_dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0){
#endif
			LOG(LOG_CRITICAL, "Failed to create directory : %s\n", temp_dir);
			exit(1);
		}
	} else {
		while((name = g_dir_read_name(dir)) != NULL){
			sprintf(fullpath,"%s%s%s",temp_dir, DIR_DELIMITER, name);
			unlink(fullpath);
		}
		g_dir_close(dir);
	}


	// Check cache directory and create it if it does not exist
	if((dir = g_dir_open(cache_dir, 0, NULL)) == NULL){
#ifdef __WIN32__
		if(mkdir(cache_dir) != 0){
#else
		if(mkdir(cache_dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0){
#endif
			LOG(LOG_CRITICAL, "Failed to create directory : %s\n", cache_dir);
			exit(1);
		}
	} else {
		g_dir_close(dir);
	}


	find_or_copy_file(FILENAME_STEMMING_EN);
	find_or_copy_file(FILENAME_STEMMING_JA);
	find_or_copy_file(FILENAME_WEBLIST);
	find_or_copy_file(FILENAME_SHORTCUT);
	find_or_copy_file(FILENAME_FILTER);

#ifdef __WIN32__

	// Enable customization
	find_or_copy_file(FILENAME_GTKRC);
	rc_path = g_strdup_printf("%s%s%s", user_dir, DIR_DELIMITER, FILENAME_GTKRC);
	gtk_rc_parse(rc_path);
	g_free(rc_path);
#endif

        LOG(LOG_DEBUG, "exe_path = %s", exe_path);
        LOG(LOG_DEBUG, "package_dir = %s", package_dir);
        LOG(LOG_DEBUG, "ser_dir = %s", user_dir);
        LOG(LOG_DEBUG, "temp_dir = %s", temp_dir);
        LOG(LOG_DEBUG, "cache_dir = %s", cache_dir);

	LOG(LOG_DEBUG, "OUT : initialize_preference()");
}

static gboolean ok_pref(GtkWidget *widget,gpointer *data){
	gint i;

	LOG(LOG_DEBUG, "IN : ok_pref()");

	for( i = 0 ; ; i ++){

		if(prefs[i].title == NULL)
			break;
		if(prefs[i].end_func != NULL)
			if(prefs[i].end_func() != TRUE)
				return(TRUE);

	}

	save_preference();

	gtk_grab_remove(pref_dlg);

	gtk_widget_destroy(pref_dlg);

	close_popup(NULL, NULL);

	restart_main_window();

	auto_lookup_resume();

	LOG(LOG_DEBUG, "OUT : ok_pref()");
	return(FALSE);
}


void  calculate_font_size(){
		
	PangoFontDescription* desc;
	PangoLanguage* lang;
	PangoFontMap* fontmap;
#ifndef __WIN32__
	Display *display;
#endif
	PangoContext *context;
	PangoFontset* fontset;
	PangoFontMetrics* metrics;
	gint ascent;
	gint descent;
	gint width;
	gint height;

	font_height = 16;
	font_width =  8;
	font_ascent = 2;
	font_descent = 4;


	desc = pango_font_description_from_string(fontset_normal);
	lang =  pango_language_from_string("ja");

#ifdef __WIN32__
	fontmap = pango_win32_font_map_for_display();
#else
	display = gdk_x11_drawable_get_xdisplay(main_window->window);
	if(display == NULL){
		LOG(LOG_INFO, "display == NULL");
		return;
	}

	fontmap = pango_x_font_map_for_display(display);
#endif

	if(fontmap == NULL){
		LOG(LOG_INFO, "fontmap == NULL");
		return;
	}


	context = gtk_widget_get_pango_context(main_window);
	if(context == NULL){
		LOG(LOG_INFO, "context == NULL");
		return;
	}

	fontset = pango_font_map_load_fontset(fontmap,
					      context,
					      desc,
					      lang);
	if(fontset == NULL){
		LOG(LOG_INFO, "fontset == NULL");
		return;
	}

	metrics = pango_fontset_get_metrics(fontset);
	if(metrics == NULL){
		LOG(LOG_INFO, "metrics == NULL");
		return;
	}

	ascent = pango_font_metrics_get_ascent(metrics);
	descent = pango_font_metrics_get_descent(metrics);
	width = pango_font_metrics_get_approximate_char_width(metrics);
	height = ascent + descent;

	font_height = height / 1000;
	font_width = width / 1000 + 2;
	font_ascent = ascent / 1000;
	font_descent = descent / 1000;

}


static gboolean delete_event( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	gboolean ret;

	LOG(LOG_DEBUG, "IN : delete_event()");
	
	ret = ok_pref(NULL, NULL);

	LOG(LOG_DEBUG, "OUT : delete_event()");
	return(ret);
	
}

enum
{
	PREF_TITLE_COLUMN,
	PREF_NUMBER_COLUMN,
	PREF_N_COLUMNS
};


static void preflist_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gint number;
        gchar *title;

	LOG(LOG_DEBUG, "IN :preflist_selection_changed");

        if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
        {
		LOG(LOG_DEBUG, "OUT : weblist_selection_changed");
		return;
	}

	gtk_tree_model_get (model, &iter, PREF_TITLE_COLUMN, &title, -1);
	g_free (title);

	gtk_tree_model_get (model, &iter, PREF_NUMBER_COLUMN, &number, -1);
	if(number >= 0)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(note_pref), number);

	LOG(LOG_DEBUG, "OUT : preflist_selection_changed()");

}

void show_preference()
{
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *widget;
	GtkWidget *frame;

	GtkTreeStore *pref_store;
	GtkWidget *preflist_view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;

	gint i;

	LOG(LOG_DEBUG, "IN : show_preference()");

	auto_lookup_suspend();

	pref_dlg = gtk_dialog_new();
	gtk_window_set_position(GTK_WINDOW(pref_dlg), GTK_WIN_POS_CENTER_ALWAYS);

	gtk_grab_add(pref_dlg);

	g_signal_connect(G_OBJECT (pref_dlg), "delete_event",
			 G_CALLBACK(delete_event), NULL);


	hbox = gtk_hbox_new(FALSE,10);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(pref_dlg)->vbox)
		      , hbox,TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);


	pref_store = gtk_tree_store_new (PREF_N_COLUMNS,
					 G_TYPE_STRING,
					 G_TYPE_INT);

	preflist_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pref_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(preflist_view), TRUE);

	gtk_box_pack_start (GTK_BOX(hbox)
			    , preflist_view, FALSE, FALSE, 0);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Items"),
							  renderer,
							  "text", PREF_TITLE_COLUMN,
							  NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (preflist_view), column);

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (preflist_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
			  G_CALLBACK (preflist_selection_changed),
			  NULL);


	vbox = gtk_vbox_new(FALSE,10);
	gtk_box_pack_start (GTK_BOX(hbox)
			    , vbox,FALSE, FALSE, 0);


	note_pref = gtk_notebook_new();
	gtk_notebook_set_show_border(GTK_NOTEBOOK(note_pref), FALSE);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(note_pref), FALSE);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , note_pref,TRUE, TRUE, 0);


	for( i = 0 ; prefs[i].title != NULL ; i ++){
		if(prefs[i].is_child == TRUE){
			gtk_tree_store_append(pref_store, &child_iter, &parent_iter);
			gtk_tree_store_set(pref_store, &child_iter,
					   PREF_TITLE_COLUMN, _(prefs[i].title),
					   PREF_NUMBER_COLUMN, i,
					   -1);
		} else {
			gtk_tree_store_append(pref_store, &parent_iter, NULL);
			gtk_tree_store_set(pref_store, &parent_iter,
					   PREF_TITLE_COLUMN, _(prefs[i].title),
					   PREF_NUMBER_COLUMN, i,
					   -1);
		}

		label = gtk_label_new(prefs[i].title);
		frame = gtk_frame_new(NULL);
		gtk_notebook_append_page(GTK_NOTEBOOK(note_pref), 
					 frame,
					 label);

		hbox = gtk_hbox_new(FALSE, 0);
		gtk_widget_set_size_request(hbox, PREF_DIALOG_WIDTH, PREF_DIALOG_HEIGHT);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
		gtk_container_add(GTK_CONTAINER(frame), hbox);

		if(prefs[i].start_func == NULL)
			widget = gtk_label_new("");
		else
			widget = prefs[i].start_func();

		if(widget != NULL)
			gtk_box_pack_start(GTK_BOX(hbox), widget, 
					   TRUE, TRUE, 0);
	}

	button = gtk_button_new_with_label(_("Ok"));
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pref_dlg)->action_area), button,
			    TRUE, TRUE, 0);
	
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(ok_pref), (gpointer)pref_dlg);
	gtk_widget_grab_default (button);


	gtk_widget_show_all(pref_dlg);

	gtk_tree_view_expand_all(GTK_TREE_VIEW(preflist_view));


	LOG(LOG_DEBUG, "OUT : show_preference()");
}
