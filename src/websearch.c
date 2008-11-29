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
#include "statusbar.h"
#include "dialog.h"
#include "jcode.h"
#include "eb.h"
#include "external.h"

#ifndef __WIN32__
#include <pthread.h>
#include <sys/wait.h>
#endif

#define MAX_ENGINES 100


void go_home();
void web_search();

static GtkWidget *weblist_view;
//static GtkCTreeNode *current_node;
static GtkItemFactory *tree_item_factory;
static GtkItemFactoryEntry tree_menu_items[] = {
	{ N_("/Go Home"),    NULL, go_home, 0, NULL },
	{ N_("/Search"),    NULL, web_search, 0, NULL },
};


extern GdkPixmap *book_open_pixmap;
extern GdkPixmap *book_closed_pixmap;
extern GdkBitmap *book_open_mask;
extern GdkBitmap *book_closed_mask;


extern GtkWidget *dict_viewport;
extern GtkWidget *dict_scroll;
extern GtkWidget *progress_web;
extern GList *web_list;


struct _engine {
	gchar *group;
	gchar *title;
	gchar *url;
	gchar *delimit;
	gchar *rest;
};

struct _engine engines[MAX_ENGINES];


static unsigned char *codeconv(unsigned char *str, const char *ocode)
{
	g_assert(str != NULL);
	if(ocode == NULL)
		return(strdup(str));

	if((strcmp(ocode, "euc-jp") != 0) &&
	   (strcmp(ocode, "shift_jis") != 0) &&
	   (strcmp(ocode, "iso-2022-jp") != 0) &&
	   (strcmp(ocode, "utf-8") != 0))
		return(strdup(str));

	return(iconv_convert("utf-8", ocode, str));
}


//void do_web_search(GtkWidget *widget, gpointer data){
void web_search()
{
	gchar url[512];
	gint i, j;
	unsigned char c;
	gchar *keywords[EB_MAX_KEYWORDS + 1];
	unsigned char *kanji_str;
	const char *word;

	GtkTreeIter iter;
	GtkTreeSelection *selection;

	gint type;
	gchar *pre;
	gchar *post;
	gchar *glue;
	gchar *code;

	word = gtk_entry_get_text(GTK_ENTRY(word_entry));

	if(strlen(word) == 0){
		popup_warning(_("Please enter search word."));
		return;
	}

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(weblist_view));
	if ((gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE) ||
	    (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(web_store), &iter)))
	{
		popup_warning(_("Please select web site"));
		return;
	}

	gtk_tree_model_get (GTK_TREE_MODEL(web_store), 
			    &iter, 
			    WEB_TYPE_COLUMN, &type,
			    WEB_PRE_COLUMN, &pre,
			    WEB_POST_COLUMN, &post,
			    WEB_GLUE_COLUMN, &glue,
			    WEB_CODE_COLUMN, &code,
			    -1);

	if(type == 0){
	}

	split_word(word, keywords);

	status_message("Lanuching web browser...");

	url[0] = '\0';
	strcat(url, pre);

	for(i=0 ; ; i++){
		if(keywords[i] == NULL)
			break;
		if((i != 0) && (glue))
			strcat(url, glue);

		if(strcmp(code, "ascii") == 0) {
			sprintf(url, "%s%s", url, keywords[i]);
		} else {
			kanji_str = codeconv(keywords[i], code);
			for(j=0 ; j < strlen(kanji_str) ; j ++){
				c = kanji_str[j];
				sprintf(url, "%s%%%02X", url, c);
			}
			free(kanji_str);
		}
	}
	if(post)
		strcat(url, post);

	launch_web_browser(url);

	g_free(pre);
	g_free(post);
	g_free(glue);
	g_free(code);

}

void go_home()
{
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	gchar *home;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(weblist_view));
	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		popup_warning(_("Please select web site"));
		return;
	}

	gtk_tree_model_get (GTK_TREE_MODEL(web_store), 
			    &iter, 
			    WEB_HOME_COLUMN, &home,
			    -1);

	launch_web_browser(home);

	g_free(home);
}

static void weblist_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gint type;
        gchar *title;

	LOG(LOG_DEBUG, "IN :weblist_selection_changed");

        if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
        {
		LOG(LOG_DEBUG, "OUT : weblist_selection_changed");
		return;
	}

	gtk_tree_model_get (model, &iter, WEB_TYPE_COLUMN, &type, -1);
	gtk_tree_model_get (model, &iter, WEB_TITLE_COLUMN, &title, -1);
	g_free (title);

	if(type == 1){
		//
	}

	LOG(LOG_DEBUG, "OUT : web_selection_changed");

}

static gint button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	LOG(LOG_DEBUG, "IN : button_press_event()");
//KENKEN
	if ((event->type == GDK_BUTTON_PRESS) && 
	    ((event->button == 2) || (event->button == 3))){
		gtk_item_factory_popup (GTK_ITEM_FACTORY (tree_item_factory), 
					event->x_root, event->y_root, 
					event->button, event->time);
		return(TRUE);
	} else 	if ((event->type == GDK_2BUTTON_PRESS) && 
		    (event->button == 1)){

		GtkTreeIter iter;
		GtkTreeSelection *selection;
		GtkTreePath *path;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(weblist_view));
		if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE) {
			return(TRUE);
		}

		path = gtk_tree_model_get_path(GTK_TREE_MODEL(web_store),
					       &iter);

		if(gtk_tree_model_iter_has_child(GTK_TREE_MODEL(web_store), &iter) == TRUE){
			if(gtk_tree_view_row_expanded(GTK_TREE_VIEW(weblist_view), path)){
				gtk_tree_view_collapse_row(GTK_TREE_VIEW(weblist_view), path);
			} else {
				gtk_tree_view_expand_row(GTK_TREE_VIEW(weblist_view), path, FALSE);
			}
		} else {
			web_search();
		}

		gtk_tree_path_free(path);
	}


	LOG(LOG_DEBUG, "OUT : button_press_event() = FALSE");
	return(FALSE);
}



GtkWidget *create_web_tree()
{
	GtkWidget *web_box;

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	gint nmenu_items;
	gint i;

	LOG(LOG_DEBUG, "IN : create_web_tree()");


	web_box = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (web_box),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	weblist_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(web_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(weblist_view), FALSE);

	gtk_container_add (GTK_CONTAINER (web_box), weblist_view);

	g_signal_connect(G_OBJECT(weblist_view),"button_press_event",
			 G_CALLBACK(button_press_event), (gpointer)NULL);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(NULL,
							  renderer,
							  "text", WEB_TITLE_COLUMN,
							  NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (weblist_view), column);

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (weblist_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
/*
	g_signal_connect (G_OBJECT (select), "changed",
			  G_CALLBACK (weblist_selection_changed),
			  NULL);
*/

	nmenu_items = sizeof (tree_menu_items) / sizeof (tree_menu_items[0]);
	for(i=0 ; i<nmenu_items ; i++){
		tree_menu_items[i].path = _(tree_menu_items[i].path);
	}

	tree_item_factory = gtk_item_factory_new (GTK_TYPE_MENU, "<tree>", 
					     NULL);
	gtk_item_factory_create_items (tree_item_factory, nmenu_items, 
				       tree_menu_items, NULL);
	LOG(LOG_DEBUG, "OUT : create_web_tree()");

	return(web_box);
}

