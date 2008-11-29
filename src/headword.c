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

#include "eb.h"
#include "history.h"
#include "dump.h"
#include "dirtree.h"
#include "statusbar.h"
#include "mainwindow.h"
#include "cellrendererebook.h"
#include "jcode.h"
#include "grep.h"
#include "pixmap.h"

extern GtkWidget *note_tree;

static GtkTreeStore *heading_store=NULL;
static GtkWidget *tree_scroll;
static GtkWidget *tree_view;
static GtkTreeViewColumn *dict_column;
static GtkWidget *button_prev_hit;
static GtkWidget *button_next_hit;
static gint num_heading;
static 	GtkWidget *image_next;
static 	GtkWidget *image_prev;

gint skip_result=0;

enum
{
	HEADING_TYPE_COLUMN,
	HEADING_TITLE_COLUMN,
	HEADING_DICT_COLUMN,
	HEADING_DICTFGCOLOR_COLUMN,
	HEADING_DICTBGCOLOR_COLUMN,
	HEADING_RESULT_COLUMN,
	HEADING_BOOK_COLUMN,
	HEADING_N_COLUMNS
};


static void show_location(RESULT *result){
	gchar msg[512];

	if(result->type == RESULT_TYPE_EB){
		sprintf(msg, "%s : HEADING: page=%08x offset=%03x   CONTENT: page=%08x offset=%03x",
			result->data.eb.book_info->subbook_title,
			result->data.eb.pos_heading.page, 
			result->data.eb.pos_heading.offset,
			result->data.eb.pos_text.page, 
			result->data.eb.pos_text.offset);
	} else if(result->type == RESULT_TYPE_GREP){
		gchar *f1, *f2;
		f1 = generic_to_native(result->data.grep.filename);
		f2 = fs_to_unicode(f1);
		sprintf(msg, "%s : page=%d, line=%d, offset=%d",
			f2,
			result->data.grep.page,
			result->data.grep.line,
			result->data.grep.offset);
		g_free(f1);
		g_free(f2);
	} else {
		return;
	}
	status_message(msg);

};

void show_result_tree()
{

	RESULT *rp;
	GList *l;
	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;
	BOOK_INFO *last_book = NULL;
	gchar *last_file=NULL;
	gint i;
	gchar buff[5];
	gint heading_count;


	LOG(LOG_DEBUG, "IN : show_result_tree()");

	if(heading_store != NULL) {
		gtk_tree_store_clear(heading_store);
	} else {
		heading_store = gtk_tree_store_new (HEADING_N_COLUMNS,
						    G_TYPE_INT,
						    G_TYPE_STRING,
						    G_TYPE_STRING,
						    G_TYPE_STRING,
						    G_TYPE_STRING,
						    G_TYPE_POINTER,
						    G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), GTK_TREE_MODEL(heading_store));
	}

//	gtk_button_set_label(GTK_BUTTON(button_next_hit), "");
//	gtk_button_set_label(GTK_BUTTON(button_prev_hit), "");

	gtk_widget_set_sensitive(button_next_hit, FALSE);
	gtk_widget_set_sensitive(button_prev_hit, FALSE);

	if(search_result == NULL){
		// No hit
		LOG(LOG_DEBUG, "OUT : show_result_tree() : NOP");
		return;
	}

	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_tree), 0);
	gtk_adjustment_set_value(
		gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(tree_scroll)), 0);

	if(skip_result != 0){
		gtk_widget_set_sensitive(button_prev_hit, TRUE);
//		gtk_button_set_label(GTK_BUTTON(button_prev_hit), "<<");

	}

	if(ebook_search_method() == SEARCH_METHOD_GREP)
		goto GREP;


	if(bsort_by_dictionary)
		goto SORT_BY_DICT;

	for(i=0, l = search_result ; l != NULL; i ++, l = g_list_next(l)){
		if((skip_result != 0) && (i < skip_result)){
			continue;
		}

		if(i >= num_heading + skip_result) {
			gtk_widget_set_sensitive(button_next_hit, TRUE);
//			gtk_button_set_label(GTK_BUTTON(button_next_hit), ">>");

			break;
		}

		rp = (RESULT *)(l->data);
		gtk_tree_store_append(heading_store, &child_iter, NULL);

		memset(buff, 0, sizeof(buff));
		if(rp->data.eb.dict_title == NULL){
			gtk_tree_store_set (heading_store, &child_iter,
					    HEADING_TYPE_COLUMN, 1,
					    HEADING_TITLE_COLUMN, rp->heading,
					    HEADING_DICT_COLUMN, NULL,
					    HEADING_DICTFGCOLOR_COLUMN, NULL,
					    HEADING_DICTBGCOLOR_COLUMN, NULL,
					    HEADING_BOOK_COLUMN, rp->data.eb.book_info,
					    HEADING_RESULT_COLUMN, rp,
					    -1);
		} else {
			g_unichar_to_utf8(g_utf8_get_char(rp->data.eb.dict_title), buff);
			gtk_tree_store_set (heading_store, &child_iter,
					    HEADING_TYPE_COLUMN, 1,
					    HEADING_TITLE_COLUMN, rp->heading,
					    HEADING_DICT_COLUMN, buff,
					    HEADING_DICTFGCOLOR_COLUMN, rp->data.eb.book_info->fg,
					    HEADING_DICTBGCOLOR_COLUMN, rp->data.eb.book_info->bg,
					    HEADING_BOOK_COLUMN, rp->data.eb.book_info,
					    HEADING_RESULT_COLUMN, rp,
					    -1);
		}

		last_book = rp->data.eb.book_info;

	}

	gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
	goto END;

 SORT_BY_DICT:

	heading_count = 0;
	for(i=0, l = search_result ; l != NULL; i ++, l = g_list_next(l)){
		if((skip_result != 0) && (i < skip_result)){
			continue;
		}

		if(heading_count >= num_heading) {
			gtk_widget_set_sensitive(button_next_hit, TRUE);
//			gtk_button_set_label(GTK_BUTTON(button_next_hit), ">>");
			break;
		}

		rp = (RESULT *)(l->data);

		// Dictionary name is different than former result.

		if(last_book != rp->data.eb.book_info){
			gtk_tree_store_append(heading_store, &parent_iter, NULL);
			gtk_tree_store_set (heading_store, &parent_iter,
					    HEADING_TYPE_COLUMN, 0,
					    HEADING_TITLE_COLUMN, rp->data.eb.book_info->subbook_title,
					    -1);
			last_book = rp->data.eb.book_info;
			heading_count ++;
		}

		gtk_tree_store_append(heading_store, &child_iter, &parent_iter);
		gtk_tree_store_set (heading_store, &child_iter,
				    HEADING_TYPE_COLUMN, 1,
				    HEADING_TITLE_COLUMN, rp->heading,
				    HEADING_DICT_COLUMN, NULL,
				    HEADING_BOOK_COLUMN, rp->data.eb.book_info,
				    HEADING_RESULT_COLUMN, rp,
				    -1);

		last_book = rp->data.eb.book_info;

		heading_count ++;
	}

	gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
	goto END;



 GREP:

	heading_count = 0;
	for(i=0, l = search_result ; l != NULL; i ++, l = g_list_next(l)){
		if((skip_result != 0) && (i < skip_result)){
			continue;
		}

		if(heading_count >= num_heading) {
			gtk_widget_set_sensitive(button_next_hit, TRUE);
//			gtk_button_set_label(GTK_BUTTON(button_next_hit), ">>");
			break;
		}

		rp = (RESULT *)(l->data);
		g_assert(rp != NULL);

		// File name is different than former result.
		if(bshow_filename){
			if((last_file == NULL) ||
			   (strcmp(last_file, rp->data.grep.filename) != 0)){
#ifdef __WIN32__			
				gchar *tmp1, *tmp2;
				tmp1 = generic_to_native(rp->data.grep.filename);
				tmp2 = fs_to_unicode(tmp1);
#else
				gchar *tmp2;

				tmp2 = rp->data.grep.filename;
#endif
				gtk_tree_store_append(heading_store, &parent_iter, NULL);
				gtk_tree_store_set (heading_store, &parent_iter,
						    HEADING_TYPE_COLUMN, 0,
						    HEADING_TITLE_COLUMN, tmp2,
						    -1);
#ifdef __WIN32__
				g_free(tmp1);
				g_free(tmp2);
#endif

				last_file = strdup(rp->data.grep.filename);
				heading_count ++;
			}
			gtk_tree_store_append(heading_store, 
				      &child_iter, &parent_iter);
		} else {
			gtk_tree_store_append(heading_store, 
					      &child_iter, NULL);
		}
		gtk_tree_store_set (heading_store, &child_iter,
				    HEADING_TYPE_COLUMN, 1,
				    HEADING_TITLE_COLUMN, rp->heading,
				    HEADING_DICT_COLUMN, NULL,
//				    HEADING_BOOK_COLUMN, rp->data.eb.book_info,
				    HEADING_RESULT_COLUMN, rp,
				    -1);

		heading_count ++;
	}

	gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));

	goto END;

 END:

	if(bheading_auto_calc){
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tree_scroll),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_NEVER);
	} else {
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tree_scroll),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
	}

	LOG(LOG_DEBUG, "OUT : show_result_tree()");
}

void select_first_item()
{
	GtkTreeIter   iter;
	GtkTreeIter   child;
	GtkTreeSelection *select;
	gint type;

	LOG(LOG_DEBUG, "IN : select_first_item()");

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(heading_store),
					 &iter) == FALSE)
		return;

	gtk_tree_model_get(GTK_TREE_MODEL(heading_store), 
			   &iter, HEADING_TYPE_COLUMN, &type, -1);

	if(type == 0){
		if(gtk_tree_model_iter_children(GTK_TREE_MODEL(heading_store),
						&child, &iter) == TRUE){
			gtk_tree_selection_select_iter(select, &child);
		}
	} else {
		gtk_tree_selection_select_iter(select, &iter);
	}

	LOG(LOG_DEBUG, "OUT : select_first_item()");
}

static gint button_press_event(GtkWidget *widget, GdkEventButton *event)
{
        GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	gint type;
	RESULT *rp;

	LOG(LOG_DEBUG, "IN : button_press_event()");

	if ((event->type == GDK_BUTTON_PRESS) && 
	    ((event->button == 2) || (event->button == 3))){
		return(TRUE);
	} else 	if ((event->type == GDK_2BUTTON_PRESS) && 
		    (event->button == 1)){

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
		if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
		{
			return(FALSE);
		}

		gtk_tree_model_get (model, &iter, HEADING_TYPE_COLUMN, &type, -1);
		if(type == 0)
			return(FALSE);

		gtk_tree_model_get (model, &iter, HEADING_RESULT_COLUMN, &rp, -1);
		if(rp->type != RESULT_TYPE_GREP)
			return(FALSE);

		open_file(rp);
	}

	LOG(LOG_DEBUG, "OUT : button_press_event() = FALSE");
	return(FALSE);
}

void item_next()
{

        GtkTreeIter iter;
        GtkTreeIter child;
        GtkTreeModel *model;
	GtkTreeSelection *select;
	GtkTreePath *path;

	LOG(LOG_DEBUG, "IN : item_next()");

	if(!heading_store) {
		LOG(LOG_DEBUG, "OUT : item_next()");
		return;
	}

	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
        if (gtk_tree_selection_get_selected(select, &model, &iter) == FALSE)
        {
		LOG(LOG_DEBUG, "OUT : item_next() = no selection");
		return;
	}

	path = gtk_tree_model_get_path(model, &iter);

	if(((bsort_by_dictionary == FALSE) && 
	    (ebook_search_method() != SEARCH_METHOD_GREP)) ||
	   ((bshow_filename == FALSE) && 
	    (ebook_search_method() == SEARCH_METHOD_GREP))){
		gtk_tree_path_next(path);
		if(gtk_tree_model_get_iter(model, &iter, path) == TRUE){
			gtk_tree_selection_select_iter(select, &iter);
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), 
						     path,
						     NULL,
						     TRUE,
						     0.5,
						     0.0);
		}
		goto END;
	}


	while(1){
		// Break if path is invalid
		if(gtk_tree_model_get_iter(model, &iter, path) == FALSE){
			break;
		}

		// Dictionary name is selected.
		if(gtk_tree_path_get_depth(path) == 1){
			// If there are childs.
			if(gtk_tree_model_iter_children(model, &child, &iter) == TRUE){
				gtk_tree_selection_select_iter(select, &child);

				gtk_tree_path_free(path);
				path = gtk_tree_model_get_path(model, &child);
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), 
							     path,
							     NULL,
							     TRUE,
							     0.5,
							     0.0);


				break;
			}
			gtk_tree_path_next(path);
			continue;
		// Result is selected.
		} else {
			GtkTreePath *next;
			
			next = gtk_tree_path_copy(path);
			gtk_tree_path_next(next);

			// Is there next item ?
			if((gtk_tree_path_compare(path, next) == -1) &&
			   (gtk_tree_model_get_iter(model, &iter, next) == TRUE)){
				gtk_tree_selection_select_iter(select, &iter);
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), 
							     next,
							     NULL,
							     TRUE,
							     0.5,
							     0.0);
				gtk_tree_path_free(next);
				break;
			} else {
				// If there is not, select next dictionary.
				gtk_tree_path_up(path);
				gtk_tree_path_next(path);
				gtk_tree_path_free(next);
				continue;
			}

		}
	}

 END:
	gtk_tree_path_free(path);

	LOG(LOG_DEBUG, "OUT : item_next()");
}

void item_previous()
{

        GtkTreeIter iter;
        GtkTreeModel *model;
	GtkTreeSelection *select;
	GtkTreePath *path;
	GtkTreePath *prev;

	LOG(LOG_DEBUG, "IN : item_previous()");

	if(!heading_store) {
		LOG(LOG_DEBUG, "OUT : item_previous()");
		return;
	}

	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
        if (gtk_tree_selection_get_selected(select, &model, &iter) == FALSE)
        {
		LOG(LOG_DEBUG, "OUT : item_previous() = no selection");
		return;
	}

	path = gtk_tree_model_get_path(model, &iter);

	if(((bsort_by_dictionary == FALSE) && 
	    (ebook_search_method() != SEARCH_METHOD_GREP)) ||
	   ((bshow_filename == FALSE) && 
	    (ebook_search_method() == SEARCH_METHOD_GREP))){
		gtk_tree_path_prev(path);
		if(gtk_tree_model_get_iter(model, &iter, path) == TRUE){
			gtk_tree_selection_select_iter(select, &iter);
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), 
						     path,
						     NULL,
						     TRUE,
						     0.5,
						     0.0);
		}
		goto END;
	}


	while(1){
		// Break if path is invalid
		if(gtk_tree_model_get_iter(model, &iter, path) == FALSE){
			break;
		}

		// Dictionary name is selected.
		if(gtk_tree_path_get_depth(path) == 1){
			prev = gtk_tree_path_copy(path);
			gtk_tree_path_prev(prev);
			// If there is dictionary before.
			if(gtk_tree_path_compare(prev, path) == -1){
				gint child_no;
				GtkTreeIter child;

				gtk_tree_model_get_iter(model, &iter, prev);
				child_no = gtk_tree_model_iter_n_children(model, &iter);

				if(child_no == 0){
					gtk_tree_path_free(path);
					path = gtk_tree_path_copy(prev);
					gtk_tree_path_free(prev);
					continue;
				} else {
					gtk_tree_model_iter_nth_child(model, &child, 
								  &iter, child_no-1);
					gtk_tree_selection_select_iter(select, &child);
					
					gtk_tree_path_free(path);
					path = gtk_tree_model_get_path(model, &child);
					gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), 
							     path,
							     NULL,
							     TRUE,
							     0.5,
							     0.0);

					gtk_tree_path_free(prev);
					break;
				}
			} else {
				gtk_tree_path_free(prev);
				break;
			}
		// Result is selected.
		} else {
			prev = gtk_tree_path_copy(path);
			gtk_tree_path_prev(prev);

			// Is there next item ?
			if((gtk_tree_path_compare(prev, path) == -1) &&
			   (gtk_tree_model_get_iter(model, &iter, prev) == TRUE)){
				gtk_tree_selection_select_iter(select, &iter);
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), 
							     prev,
							     NULL,
							     TRUE,
							     0.5,
							     0.0);

				gtk_tree_path_free(prev);
				break;
			} else {
				// If there is not, select the dictionary before.
				gtk_tree_path_up(path);
			}

		}
	}

 END:
	gtk_tree_path_free(path);
	LOG(LOG_DEBUG, "OUT : item_previous()");
}

static void
heading_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gint type;
        RESULT *rp;

	LOG(LOG_DEBUG, "IN : heading_selection_changed");

        if (gtk_tree_selection_get_selected (selection, &model, &iter) == FALSE)
        {
		LOG(LOG_DEBUG, "OUT : heading_selection_changed");
		return;
	}

	gtk_tree_model_get (model, &iter, HEADING_TYPE_COLUMN, &type, -1);
	if(type == 1){
		gtk_tree_model_get (model, &iter, HEADING_RESULT_COLUMN, &rp, -1);
		show_result(rp, TRUE, TRUE);
		show_location(rp);
	}

	LOG(LOG_DEBUG, "OUT : heading_selection_changed");

}

void update_tree_view()
{
	gtk_tree_view_column_set_visible(dict_column, !bsort_by_dictionary);
}

void next_heading(GtkWidget *widget, gpointer *data){

	LOG(LOG_DEBUG, "IN : next_heading()");

	skip_result += num_heading;
	if(skip_result >= g_list_length(search_result)) {
		skip_result -= num_heading;
		return;
	}

	show_result_tree();
	select_first_item();

	LOG(LOG_DEBUG, "OUT : next_heading()");

}

void previous_heading(GtkWidget *widget, gpointer *data){

	LOG(LOG_DEBUG, "IN : previous_heading()");

	if(skip_result <= 0)
		return;

	skip_result -= num_heading;
	if(skip_result < 0)
		skip_result = 0;

	show_result_tree();
	select_first_item();

	LOG(LOG_DEBUG, "OUT : previous_heading()");

}

gboolean configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
	if(bheading_auto_calc){
		num_heading = (tree_scroll->allocation.height - 12) / (font_height + 10);
		if(num_heading < 1)
			num_heading = 1;
	} else {
		num_heading = max_heading;
	}

	return(FALSE);
}



GtkWidget *create_headword_tree(){
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkWidget *vbox;
	GtkWidget *hbox;

	vbox = gtk_vbox_new(FALSE, 0);


	tree_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), tree_scroll, TRUE, TRUE, 0);

	if(bheading_auto_calc){
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tree_scroll),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_NEVER);
	} else {
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tree_scroll),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
	}

	tree_view = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);
	gtk_container_add (GTK_CONTAINER(tree_scroll), tree_view);

	g_signal_connect(G_OBJECT(tree_view),"button_press_event",
			 G_CALLBACK(button_press_event), (gpointer)NULL);

	g_signal_connect(G_OBJECT(tree_view),"expose_event",
			 G_CALLBACK(configure_event), (gpointer)NULL);


/*
//	renderer = gtk_cell_renderer_text_new();
	renderer = gtk_cell_renderer_ebook_new();
	column = gtk_tree_view_column_new_with_attributes(NULL,
							  renderer,
							  "text", HEADING_TITLE_COLUMN,
							  "book", HEADING_BOOK_COLUMN,
							  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW (tree_view), column);
*/
	renderer = gtk_cell_renderer_text_new();
	dict_column = gtk_tree_view_column_new_with_attributes("D", renderer,
							       "text", HEADING_DICT_COLUMN,
							       "foreground", HEADING_DICTFGCOLOR_COLUMN,
							       "background", HEADING_DICTBGCOLOR_COLUMN,

							       NULL);
	gtk_tree_view_insert_column(GTK_TREE_VIEW(tree_view),
				    dict_column, -1);
	gtk_tree_view_column_set_sizing(dict_column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	renderer = gtk_cell_renderer_ebook_new();
	column = gtk_tree_view_column_new_with_attributes ("Head", renderer,
							   "text", HEADING_TITLE_COLUMN,
							   "book", HEADING_BOOK_COLUMN,
							   NULL);
	gtk_tree_view_insert_column(GTK_TREE_VIEW(tree_view),
				     column, -1);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	select = gtk_tree_view_get_selection(GTK_TREE_VIEW (tree_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
			  G_CALLBACK (heading_selection_changed),
			  NULL);

	hbox = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

/*
	button_prev_hit = gtk_button_new_with_label("");
	g_signal_connect(G_OBJECT (button_prev_hit), "pressed",
			 G_CALLBACK(previous_heading),
			 NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button_prev_hit, TRUE, TRUE, 0);
	gtk_tooltips_set_tip(tooltip, button_prev_hit, _("Go to previous hit list."),"Private");
*/
	button_prev_hit = gtk_button_new();
	g_signal_connect(G_OBJECT (button_prev_hit), "pressed",
			 G_CALLBACK(previous_heading),
			 NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button_prev_hit, TRUE, TRUE, 0);
	gtk_tooltips_set_tip(tooltip, button_prev_hit, _("Go to previous hit list."),"Private");

	image_prev = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button_prev_hit), image_prev);

/*
	button_next_hit = gtk_button_new_with_label("");
	g_signal_connect(G_OBJECT (button_next_hit), "pressed",
			 G_CALLBACK(next_heading),
			 NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button_next_hit, TRUE, TRUE, 0);
	gtk_tooltips_set_tip(tooltip, button_next_hit, _("Go to next hit list."),"Private");
*/

	button_next_hit = gtk_button_new();
	g_signal_connect(G_OBJECT (button_next_hit), "pressed",
			 G_CALLBACK(next_heading),
			 NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button_next_hit, TRUE, TRUE, 0);
	gtk_tooltips_set_tip(tooltip, button_next_hit, _("Go to next hit list."),"Private");

	image_next = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button_next_hit), image_next);

	gtk_widget_set_sensitive(button_next_hit, FALSE);
	gtk_widget_set_sensitive(button_prev_hit, FALSE);


	update_tree_view();

	return(vbox);

}

