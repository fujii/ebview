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
#include "xmlinternal.h"
#include "headword.h"
#include "history.h"
#include "jcode.h"
#include "cellrendererebook.h"

#define EB_INDEX_STYLE_ASIS 1

#define MULTI_HACK

extern GList *group_list;
extern GtkWidget *note_tree;
extern GtkWidget *note_text;
extern GtkWidget *entry_box;

GtkWidget *container_child(GtkWidget *container);

typedef struct {
	GdkPixmap *pixbuff;
	gchar *text;
} CANDIDATE_DATA;

enum
{
	CANDIDATE_TITLE_COLUMN,
	CANDIDATE_BOOK_COLUMN,
	CANDIDATE_N_COLUMNS
};

gint global_multi_code;

//static GList *multi_search_list=NULL;
static GtkWidget *multi_view=NULL;
static GtkTreeStore *multi_store=NULL;
static GtkWidget *candidate_view=NULL;
static GtkTreeStore *candidate_store=NULL;

static GtkWidget *candidate_scroll=NULL;
static GtkWidget *entry_table=NULL;
static GtkWidget *multi_entry[EB_MAX_MULTI_ENTRIES];
static BOOK_INFO *global_book_info;
static gint global_entry_id;


static void start_multi_search(GtkWidget *widget, gpointer data);
static void clear_candidate();
static void show_candidate(BOOK_INFO *binfo, gint code);
static void candidate_pressed(GtkWidget *widget, gpointer data);
static void candidate_selection_changed(GtkTreeSelection *selection, gpointer data);

void show_multi()
{
	EB_Error_Code error_code;
	BOOK_INFO *binfo;
	gchar label[256];
	gchar *utf_str;
	gint i;

 	EB_Multi_Search_Code multi_codes[EB_MAX_MULTI_SEARCHES];
	int multi_count;
	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;
	GtkTreeIter   dict_parent_iter;
	GtkTreeIter   dict_child_iter;
	gboolean has_active;
	gboolean active;

	LOG(LOG_DEBUG, "IN : show_multi()");
	
	gtk_tree_store_clear(multi_store);

	has_active = FALSE;

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &dict_parent_iter) == TRUE){
		do { 
			gtk_tree_model_get (GTK_TREE_MODEL(dict_store), 
					    &dict_parent_iter, 
					    DICT_ACTIVE_COLUMN, &active,
					    -1);

			if(active) {
				has_active = TRUE;
				break;
			}
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &dict_parent_iter) == TRUE);
	}


	if(has_active == FALSE) {
		LOG(LOG_INFO, "no active group");
		return;
	}

	if(gtk_tree_model_iter_children(GTK_TREE_MODEL(dict_store), &dict_child_iter, &dict_parent_iter) == TRUE){
		do {
			gtk_tree_model_get(GTK_TREE_MODEL(dict_store),
					   &dict_child_iter,
					   DICT_ACTIVE_COLUMN, &active,
					   DICT_MEMBER_COLUMN, &binfo,
					   -1);

			if(active == FALSE)
				continue;

			if(binfo->search_method[SEARCH_METHOD_MULTI] != TRUE)
				continue;


			error_code = eb_multi_search_list(binfo->book, multi_codes, &multi_count);
			if(error_code  != EB_SUCCESS) {
				LOG(LOG_CRITICAL, "Failed to list multi search : %s\n",
				    eb_error_message(error_code));
				LOG(LOG_DEBUG, "OUT : show_multi()");
				return;
			}

			gtk_tree_store_append(GTK_TREE_STORE(multi_store),
					      &parent_iter, NULL);
			gtk_tree_store_set(GTK_TREE_STORE(multi_store),
					   &parent_iter,
					   MULTI_TYPE_COLUMN, 0,
					   MULTI_TITLE_COLUMN, binfo->subbook_title,
					   -1);

			for(i=0 ; i < multi_count ; i ++){
				error_code = eb_multi_title(binfo->book, multi_codes[i], label);
				if(error_code  != EB_SUCCESS) {
					LOG(LOG_CRITICAL, "Failed to get multi title : %s",
						ebook_error_message(error_code));
					return;
				}
				utf_str = iconv_convert("euc-jp", "utf-8", label);

				gtk_tree_store_append(GTK_TREE_STORE(multi_store),
						      &child_iter, &parent_iter);
				gtk_tree_store_set(GTK_TREE_STORE(multi_store),
						   &child_iter,
						   MULTI_TYPE_COLUMN, 1,
						   MULTI_TITLE_COLUMN, utf_str,
						   MULTI_CODE_COLUMN, multi_codes[i],
						   MULTI_BOOK_COLUMN, binfo,
						   -1);
				g_free(utf_str);
			}


		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &dict_child_iter) == TRUE);

	}

	clear_candidate();
	gtk_tree_view_expand_all(GTK_TREE_VIEW(multi_view));

	LOG(LOG_DEBUG, "OUT : show_multi()");
}

void multi_select_row(GtkWidget *widget, gint row, gint column, GdkEventButton *bevent, gpointer user_data)
{
/*
	GtkWidget *node;
	MULTI_SEARCH *idata;
	gchar *text;

	g_return_if_fail (GTK_IS_CLIST (widget));

	node = (GtkWidget *)gtk_ctree_node_nth(GTK_CTREE(widget), row);
	idata = gtk_ctree_node_get_row_data(GTK_CTREE(widget), 
					    GTK_CTREE_NODE(node));

	if(idata != NULL) {
		show_candidate(
			idata->book_info,
			idata->code);
	}

	return;
*/
}

static void show_candidate(BOOK_INFO *binfo, gint code){
	EB_Error_Code error_code;
	gint entry_count;
	gchar name[256];
	EB_Position position;
	gint i;
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *frame;
	GtkAttachOptions xoption, yoption;
	gboolean have_candidate=FALSE;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	LOG(LOG_DEBUG, "IN : show_candidate()");

//	xoption = GTK_EXPAND | GTK_SHRINK;
//	yoption = GTK_EXPAND | GTK_SHRINK;
	xoption = 0;
	yoption = 0;

	error_code = eb_multi_entry_count(binfo->book, code, &entry_count);
	if(error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to get multi entry count : %s",
			eb_error_message(error_code));
		LOG(LOG_DEBUG, "OUT : show_candidate()");
		return;
	}

	global_book_info = binfo;
	global_multi_code = code;

	clear_candidate();


	for(i=0; i < EB_MAX_MULTI_ENTRIES; i ++){
		multi_entry[i] = NULL;
	}
	
	frame = gtk_frame_new(_("Keyword"));
	gtk_box_pack_start(GTK_BOX(entry_box), frame, FALSE, FALSE, 0);

	entry_table = gtk_table_new(3, EB_MAX_MULTI_ENTRIES+1, FALSE);
	gtk_container_add (GTK_CONTAINER (frame), entry_table);

	for(i=0 ; i < entry_count ; i ++){
		gchar *utf_str;
		error_code = eb_multi_entry_label(binfo->book, code, i, name);
		if(error_code  != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to get multi title : %s",
				eb_error_message(error_code));
			LOG(LOG_DEBUG, "OUT : show_candidate()");
			return;
		}

		error_code = eb_multi_entry_candidates(binfo->book, code, i, &position);
		if(error_code == EB_ERR_NO_CANDIDATES){
			have_candidate = FALSE;
		} else if(error_code != EB_SUCCESS){
			LOG(LOG_CRITICAL, "Failed to get multi candidates : %s\n",
				eb_error_message(error_code));
			LOG(LOG_DEBUG, "OUT : show_candidate()");
			return;
		} else {
			have_candidate = TRUE;
		}
			
		utf_str = iconv_convert("euc-jp", "utf-8", name);
		label = gtk_label_new(utf_str);
		g_free(utf_str);

		gtk_table_attach(GTK_TABLE(entry_table), label, 0, 1, i, i+1,
				 xoption, yoption, 5, 1);
		multi_entry[i] = gtk_entry_new();
		g_signal_connect(G_OBJECT (multi_entry[i]), "activate",
				 G_CALLBACK(start_multi_search),
				 (gpointer)NULL);
		gtk_table_attach(GTK_TABLE(entry_table), multi_entry[i], 1, 2, i, i+1,
				 xoption, yoption, 5, 1);
		
		if(have_candidate == TRUE){
			button = gtk_button_new_with_label(_("Candidates"));
			g_signal_connect(G_OBJECT (button), "pressed",
					 G_CALLBACK(candidate_pressed),
					 (gpointer)i);

			gtk_table_attach(GTK_TABLE(entry_table), button, 2, 3, i, i+1,
					 xoption, yoption, 5, 1);
		}

	}

	button = gtk_button_new_with_label(_("Start search"));
	g_signal_connect(G_OBJECT (button), "pressed",
			 G_CALLBACK(start_multi_search),
			 (gpointer)NULL);

	gtk_box_pack_start(GTK_BOX(entry_box), button, FALSE, TRUE, 2);



	frame = gtk_frame_new(_("Candidates"));
	gtk_box_pack_start(GTK_BOX(entry_box), frame, TRUE, TRUE, 0);


	candidate_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (candidate_scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (frame), candidate_scroll);


	if(candidate_store == NULL){
		candidate_store = gtk_tree_store_new(CANDIDATE_N_COLUMNS,
						     G_TYPE_STRING,
						     G_TYPE_POINTER);

	} else {
		gtk_tree_store_clear(GTK_TREE_STORE(candidate_store));
	}


	candidate_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(candidate_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(candidate_view), FALSE);
	gtk_container_add (GTK_CONTAINER (candidate_scroll), candidate_view);

	renderer = gtk_cell_renderer_ebook_new();
//	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(NULL,
							  renderer,
							  "text", CANDIDATE_TITLE_COLUMN,
							  "book", CANDIDATE_BOOK_COLUMN,
							  NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(candidate_view), column);

	select = gtk_tree_view_get_selection(GTK_TREE_VIEW (candidate_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(select), "changed",
			 G_CALLBACK( candidate_selection_changed),
			 NULL);

	gtk_widget_show_all(entry_box);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(note_text), 1);

	LOG(LOG_DEBUG, "OUT : show_candidate()");

}

static void clear_candidate(){
	GList *list;

	LOG(LOG_DEBUG, "IN : clear_candidate()");

	if(candidate_store)
		gtk_tree_store_clear(GTK_TREE_STORE(candidate_store));

	while(1) {
		if(!GTK_IS_CONTAINER(entry_box))
			break;
		list = gtk_container_get_children(GTK_CONTAINER(entry_box));
		if(list == NULL)
			break;
		gtk_container_remove(GTK_CONTAINER(entry_box), list->data);
	}

	LOG(LOG_DEBUG, "OUT : clear_candidate()");
}

static void show_candidate_tree(BOOK_INFO *binfo, gint page, gint offset, GtkTreeIter *parent)
{
	gchar *text;
	gchar *p;
	gchar start_tag[512];
	gchar end_tag[512];
	gchar tag_name[512];
	gchar attr[512];
	gchar body[65536];
	gchar *content;
	gchar *candidate;
	gint  content_length;
	gint  body_length;
	gint  l_page=0, l_offset=0;
	GtkTreeIter iter;


	LOG(LOG_DEBUG, "IN : show_candiate_tree()");


	text = ebook_get_candidate(global_book_info, page, offset);

	body_length = 0;
	p = text;

	while(*p != '\0'){
		if(*p == '<'){
			if(body_length != 0){
				LOG(LOG_INFO, "candidate format error0");
				body_length = 0;
			}

			get_start_tag(p, start_tag);
			get_tag_name(start_tag, tag_name);

			if(strcmp(tag_name, "candidate") == 0){
				gchar *utf_str;

				get_end_tag(p, tag_name, end_tag);

				l_page = l_offset = 0;
				get_attr(end_tag, "page", attr);
				l_page = strtol(attr, NULL, 16);
				get_attr(end_tag, "offset", attr);
				l_offset = strtol(attr, NULL, 16);

				get_content(p, tag_name, &content, &content_length);
				candidate = g_strndup(content, content_length);
				utf_str = iconv_convert("euc-jp", "utf-8", candidate);

				gtk_tree_store_append(candidate_store, &iter, parent);
				gtk_tree_store_set (candidate_store, &iter,
						    CANDIDATE_TITLE_COLUMN, utf_str,
						    CANDIDATE_BOOK_COLUMN, binfo,
						    -1);
				g_free(candidate);
				g_free(utf_str);

				if(l_page == 0){
					// Leaf
				} else {
					// Not leaf
					show_candidate_tree(binfo, l_page, l_offset, &iter);
				}

				skip_end_tag(&p, tag_name);

			} else {
				LOG(LOG_INFO, "candidate format error1");
				body[body_length] = *p;
				body_length ++;
				body[body_length] = '\0';
				p++;
				LOG(LOG_INFO, "%s", body);
			}
		} else if (*p == '\n'){
			p++;
		} else {
			LOG(LOG_INFO, "candidate format error2");
			body[body_length] = *p;
			body_length ++;
			body[body_length] = '\0';
			p++;
		}

	}

	if(body_length != 0){
		LOG(LOG_INFO, "candidate format erro3");
	}


	free(text);

	LOG(LOG_DEBUG, "OUT : show_candiate_tree()");
}


//void candidate_select_row(GtkWidget *widget, gint row, gint column, GdkEventButton *bevent, gpointer user_data)
static void candidate_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
	gchar *title;

	LOG(LOG_DEBUG, "IN : candidate_selection_changed()");

        if (gtk_tree_selection_get_selected (selection, &model, &iter) == FALSE)
        {
		LOG(LOG_DEBUG, "OUT : heading_selection_changed");
		return;
	}

	if(gtk_tree_model_iter_has_child(model, &iter) == TRUE){
		LOG(LOG_DEBUG, "OUT : heading_selection_changed");
		return;
	}

	gtk_tree_model_get (model, &iter, CANDIDATE_TITLE_COLUMN, &title, -1);
	gtk_entry_set_text(GTK_ENTRY(multi_entry[global_entry_id]), title);

	g_free (title);



/*
	GtkCTreeNode *node;
	GtkWidget *last_candidate;
	gboolean is_leaf;
	gchar *text;
	CANDIDATE_DATA *cdata;
	

	g_return_if_fail (GTK_IS_CLIST (widget));

	node = gtk_ctree_node_nth(GTK_CTREE(widget), row);


	last_candidate = current_candidate;
	current_candidate = (GtkWidget *)node;
	draw_candidate_text((GtkWidget *)last_candidate);

	gtk_ctree_get_node_info(GTK_CTREE(widget), node, &text, NULL, NULL, NULL, NULL, NULL, &is_leaf, NULL);

	cdata = gtk_ctree_node_get_row_data(GTK_CTREE(candidate_tree), GTK_CTREE_NODE(node));

	if(is_leaf == TRUE){
		switch (bevent->type)
		{
		case GDK_BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:
			gtk_entry_set_text(GTK_ENTRY(multi_entry[global_entry_id]), cdata->text);
			break;
		case GDK_2BUTTON_PRESS:
			start_multi_search(NULL, NULL);
			break;
			
		default:
			break;
		}


	}

	draw_candidate_text((GtkWidget *)node);
*/

	LOG(LOG_DEBUG, "OUT : candidate_selection_changed()");
}

static void candidate_pressed(GtkWidget *widget, gpointer data)
{
	EB_Position position;
	EB_Error_Code error_code;

	LOG(LOG_DEBUG, "IN : candidate_pressed()");

	global_entry_id = (gint)data;

	error_code = eb_multi_entry_candidates(global_book_info->book, global_multi_code, global_entry_id, &position);
	if (error_code == EB_ERR_NO_CANDIDATES) {
		return;
	} else if (error_code != EB_SUCCESS) {
		return;
	}


	gtk_tree_store_clear(candidate_store);

	show_candidate_tree(global_book_info, position.page, position.offset, NULL);

	LOG(LOG_DEBUG, "OUT : candidate_pressed()");

}

void search_multi(gchar *word)
{
	EB_Error_Code error_code;
	GtkTreeIter iter;
	GtkTreeIter parent;
	gchar *dic_title;
	gint i;

#ifdef MULTI_HACK
	EB_Search saved_search[EB_MAX_MULTI_ENTRIES];
	EB_Multi_Search *multi;
#endif

	LOG(LOG_DEBUG, "IN : search_multi(%s)", word);
	
	// Find user defined name for this dictionary
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &parent) == TRUE){
		do { 
			gint type;
			gboolean active;
			BOOK_INFO *binfo;
			gchar *title;

			gtk_tree_model_get (GTK_TREE_MODEL(dict_store), 
					    &parent, 
					    DICT_ACTIVE_COLUMN, &active,
					    -1);
			if(active == TRUE){
				if(gtk_tree_model_iter_children(GTK_TREE_MODEL(dict_store), &iter, &parent) == TRUE){
					do {
						gtk_tree_model_get(GTK_TREE_MODEL(dict_store),
								   &iter,
								   DICT_TYPE_COLUMN, &type,
								   DICT_TITLE_COLUMN, &title,
								   DICT_ACTIVE_COLUMN, &active,
								   DICT_MEMBER_COLUMN, &binfo,
								   -1);
						if((global_book_info == binfo) && active){
							dic_title = g_strdup(title);
							g_free(title);
							break;
						}
						g_free(title);
					} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
				}
			}
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &parent) == TRUE);
	}

	error_code = ebook_simple_search(global_book_info, 
					 word,
					 SEARCH_METHOD_MULTI,
					 dic_title);
					 
	if (error_code != EB_SUCCESS) {
		goto FAILED;
	}
	
#ifdef MULTI_HACK	

	if(g_list_length(search_result) != 0){
		goto END;
	}
	
	// Workaround.
	// If no hit, try again assuming everyghing is stored "as is"
	multi = &(global_book_info->book->subbook_current->multis[global_multi_code]);
	for(i=0; i<EB_MAX_MULTI_ENTRIES ; i ++){
		saved_search[i] = multi->entries[i];
		
		multi->entries[i].katakana = EB_INDEX_STYLE_ASIS;
		multi->entries[i].lower = EB_INDEX_STYLE_ASIS;
		multi->entries[i].mark = EB_INDEX_STYLE_ASIS;
		multi->entries[i].long_vowel = EB_INDEX_STYLE_ASIS;
		multi->entries[i].double_consonant = EB_INDEX_STYLE_ASIS;
		multi->entries[i].contracted_sound = EB_INDEX_STYLE_ASIS;
		multi->entries[i].voiced_consonant = EB_INDEX_STYLE_ASIS;
		multi->entries[i].small_vowel = EB_INDEX_STYLE_ASIS;
		multi->entries[i].p_sound = EB_INDEX_STYLE_ASIS;
		multi->entries[i].space = EB_INDEX_STYLE_ASIS;
	}

	error_code = ebook_simple_search(global_book_info, 
					 word,
					 SEARCH_METHOD_MULTI,
					 dic_title);

	for(i=0; i<EB_MAX_MULTI_ENTRIES ; i ++){
		multi->entries[i] = saved_search[i];
	}

	if (error_code != EB_SUCCESS) {
		goto FAILED;
	}


#endif

 END:
	g_free(dic_title);
	LOG(LOG_DEBUG, "OUT : search_multi()");
	return;

 FAILED:
	g_free(dic_title);
	LOG(LOG_DEBUG, "OUT : search_multi() = ERROR");
	return;

}	

static void start_multi_search(GtkWidget *widget, gpointer data)
{
	const gchar *text;
	gchar word[256];
	gchar *euc_str;
	gint i;
	gchar attr[512];
	guint code;
	gchar *p;

	LOG(LOG_DEBUG, "IN : start_multi_search()");

	clear_search_result();

	word[0] = '\0';

	for(i=0; i < EB_MAX_MULTI_ENTRIES; i ++){
		if(multi_entry[i] == NULL)
			break;

		text = gtk_entry_get_text(GTK_ENTRY(multi_entry[i]));
		if(text == NULL)
			break;

		if(strstr(text, "<gaiji") != NULL){
			get_attr(text, "code", attr);
			code = strtol(&attr[1], NULL, 16);
			p = &word[strlen(word)];

			*p = (code >> 8);
			p ++;
			*p = (code & 0xff);
			p ++;
			*p = ' ';
			p ++;
			*p = '\0';
		} else {
			if(strlen(text) != 0){
				euc_str = iconv_convert("utf-8", "euc-jp", text);
				strcat(word, euc_str);
				strcat(word, " ");
				g_free(euc_str);
			} else {
				strcat(word, " ");
			}
		}

	}
	word[strlen(word) - 1] = '\0';

//	gtk_entry_set_text(GTK_ENTRY(word_entry), word);

	if(strlen(word) == 0) {
		LOG(LOG_DEBUG, "OUT : start_multi_search()");
		return;
	}

	search_multi(word);

	show_result_tree();

	LOG(LOG_DEBUG, "OUT : start_multi_search()");
}
	
static void multi_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gint type;
	guint code;
	BOOK_INFO *binfo;
	gchar *title;

	LOG(LOG_DEBUG, "IN :multi_selection_changed");

        if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
        {
		LOG(LOG_DEBUG, "OUT : multi_selection_changed");
		return;
	}

	gtk_tree_model_get (model, &iter, MULTI_TYPE_COLUMN, &type, -1);
	gtk_tree_model_get (model, &iter, MULTI_CODE_COLUMN, &code, -1);
	gtk_tree_model_get (model, &iter, MULTI_BOOK_COLUMN, &binfo, -1);
	gtk_tree_model_get (model, &iter, MULTI_TITLE_COLUMN, &title, -1);
	g_free (title);

	if(type == 1){
		show_candidate(binfo, code);
	}

	LOG(LOG_DEBUG, "OUT : multi_selection_changed");

}
GtkWidget *create_multi_tree()
{
	GtkWidget *multi_box;

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	LOG(LOG_DEBUG, "IN : create_multi_tree()");
	
	multi_box = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (multi_box),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	multi_store = gtk_tree_store_new(MULTI_N_COLUMNS,
					 G_TYPE_INT,
					 G_TYPE_STRING,
					 G_TYPE_UINT,
					 G_TYPE_POINTER);


	multi_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(multi_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(multi_view), FALSE);

	gtk_container_add (GTK_CONTAINER (multi_box), multi_view);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(NULL,
							  renderer,
							  "text", MULTI_TITLE_COLUMN,
							  NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(multi_view), column);

	select = gtk_tree_view_get_selection(GTK_TREE_VIEW (multi_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(select), "changed",
			 G_CALLBACK( multi_selection_changed),
			 NULL);

	show_multi();

	LOG(LOG_DEBUG, "OUT : create_multi_tree()");

	return(multi_box);
}

