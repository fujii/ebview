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
#include "cellrenderercolor.h"
#include "eb.h"
#include "xml.h"
#include "dialog.h"
#include "dictbar.h"
#include "jcode.h"
#include "pref_io.h"
#include "dirtree.h"

static GtkWidget *dict_view;
static GtkWidget *entry_group_name;
static GtkWidget *entry_directory_name;
static GtkWidget *spin_search_depth;
static GtkWidget *entry_title;
static GtkWidget *entry_book_path;
static GtkWidget *entry_appendix_path;
static GtkWidget *entry_subbook_no;
static GtkWidget *entry_appendix_subbook_no;
static GtkWidget *label_color;
static GtkWidget *colorsel_dlg;;

static gchar fg_color[16];
static gchar bg_color[16];
static gint color_no;

static GtkTreeIter last_iter;
static BOOK_INFO *last_binfo=NULL;
static gboolean last_active;
static gboolean edited=FALSE;
static gboolean rewinding=FALSE;

extern GtkWidget *pref_dlg;
extern GtkWidget *web_view;

enum
{
	BOOKLIST_TITLE_COLUMN,
	BOOKLIST_PATH_COLUMN,
	BOOKLIST_SUBBOOK_NO_COLUMN,
	BOOKLIST_N_COLUMNS
};



GList *book_list=NULL;

void print_dict_group();
void my_gtk_tree_store_swap (GtkTreeStore *tree_store, GtkTreeIter  *a, GtkTreeIter  *b);
static gboolean update_last_dictionary();

extern GtkWidget *combo_method;

extern void remove_space(gchar *f);

gboolean pref_end_dictgroup()
{

#if 0
	GtkTreeIter parent_iter;
	gchar *title;
#endif

	LOG(LOG_DEBUG, "IN : pref_end_dictgroup()");

	if(update_last_dictionary() == FALSE)
		return(FALSE);

#if 0
	// "Disk Search Result" というグループがあったら削除する
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
					   &parent_iter, 
					   DICT_TITLE_COLUMN, &title,
					   -1);
			
			if(strcmp(title, "Disk Search Result") == 0){
				g_free(title);
				gtk_tree_store_remove(GTK_TREE_STORE(dict_store), &parent_iter);
				break;
			}
			g_free(title);
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE);
	}
#endif

	save_dictgroup();
	update_dict_bar();

	LOG(LOG_DEBUG, "OUT : pref_end_dictgroup()");
	return(TRUE);
}

#if 0
static void add_dict(GtkWidget *widget,gpointer *data)
{
	gint type;
	const gchar *title;
	const gchar *path;
	gint subbook_no;
	gchar *appendix_path=NULL;
	gint appendix_subbook_no=0;

	BOOK_INFO *binfo=NULL;


	GtkTreeIter iter;
	GtkTreeIter child_iter;
	GtkTreeSelection *selection;
	GtkTreePath *tree_path;

	LOG(LOG_DEBUG, "IN : add_dict()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(booklist_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(booklist_store), 
				   &iter, 
				   BOOKLIST_TITLE_COLUMN, &title,
				   BOOKLIST_PATH_COLUMN, &path,
				   BOOKLIST_SUBBOOK_NO_COLUMN, &subbook_no,
				   -1);
	}


	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view));
	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE){
		popup_warning(_("Please select group."));
		LOG(LOG_DEBUG, "OUT : add_dict()");
		return;
	}
		

	gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
			   &iter, 
			   DICT_TYPE_COLUMN, &type,
			   -1);

	//binfo = load_book(path, atoi(subbook_no));
	binfo = load_book(path, subbook_no,
			  appendix_path, appendix_subbook_no, NULL, NULL);
	if(binfo == NULL){
		LOG(LOG_CRITICAL, _("Failed to load dictionary."));
		return;
	}

	if(type == 0){
		gtk_tree_store_append(dict_store, &child_iter, &iter);
	} else {
		gtk_tree_store_insert_after(dict_store, &child_iter, NULL, &iter);
	}

	gtk_tree_store_set (dict_store, &child_iter,
			    DICT_TYPE_COLUMN, 1,
			    DICT_TITLE_COLUMN, title,
			    DICT_ACTIVE_COLUMN, FALSE,
			    DICT_MEMBER_COLUMN, binfo,
			    DICT_FGCOLOR_COLUMN, NULL,
			    DICT_BGCOLOR_COLUMN, NULL,
			    -1);

	tree_path = gtk_tree_model_get_path(GTK_TREE_MODEL(dict_store), &iter);

	gtk_tree_view_expand_row(GTK_TREE_VIEW(dict_view), tree_path, FALSE);
	gtk_tree_path_free (tree_path);

	LOG(LOG_DEBUG, "OUT : add_dict()");
}
#endif

static void cell_edited(GtkCellRendererText *cell,
			const gchar         *path_string,
			const gchar         *new_text,
			gpointer             data)
{

	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;
	gint column;

	gchar *book_path;
	gint subbook_no;
	gchar *appendix_path=NULL;
	gint appendix_subbook_no=0;
	gchar *fs_book_path;
	gchar *fs_appendix_path;
	gchar *fg;
	gchar *bg;
	gint type;

	BOOK_INFO *binfo;

	column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (cell), "column"));

	gtk_tree_model_get_iter(GTK_TREE_MODEL(dict_store), &iter, path);

	switch(column)
	{
	case DICT_TITLE_COLUMN:
	case DICT_PATH_COLUMN:
	case DICT_APPENDIX_PATH_COLUMN:
	{
		gchar *old_text;

		gtk_tree_model_get(GTK_TREE_MODEL(dict_store), &iter, column, &old_text, -1);
		g_free (old_text);
		gtk_tree_store_set(GTK_TREE_STORE(dict_store), &iter,
				   column, new_text,
				   -1);
	}
	break;
	case DICT_SUBBOOK_NO_COLUMN:
	case DICT_APPENDIX_SUBBOOK_NO_COLUMN:
	{
		gtk_tree_store_set(GTK_TREE_STORE(dict_store), &iter,
				   column, atoi(new_text),
				   -1);
	}
	break;
	case DICT_FGCOLOR_COLUMN:
	case DICT_BGCOLOR_COLUMN:
	{
		gint i;
		gchar *old_text;

		if(new_text[0] != '#')
			return;

//		remove_space(new_text);
		if(strlen(new_text) != 7)
			return;

		if(strlen(new_text) != 0){
			for(i=1; i<7 ; i++){
				if((('0' <= new_text[i]) &&  (new_text[i] <= '9')) ||
				   (('a' <= new_text[i]) &&  (new_text[i] <= 'f'))){
				   } else {
					   return;
				   }
			}
		}

		gtk_tree_model_get(GTK_TREE_MODEL(dict_store), &iter, column, &old_text, -1);
		g_free (old_text);
		gtk_tree_store_set(GTK_TREE_STORE(dict_store), &iter,
				   column, new_text,
				   -1);
	}
	break;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(dict_store), &iter, 
			   DICT_TYPE_COLUMN, &type,
			   DICT_PATH_COLUMN, &book_path,
			   DICT_SUBBOOK_NO_COLUMN, &subbook_no,
			   DICT_APPENDIX_PATH_COLUMN, &appendix_path,
			   DICT_APPENDIX_SUBBOOK_NO_COLUMN, &appendix_subbook_no,
			   DICT_MEMBER_COLUMN, &binfo,
			   DICT_FGCOLOR_COLUMN, &fg,
			   DICT_BGCOLOR_COLUMN, &bg,
			   -1);

	// Group
	if(type == 0)
		return;

//	if(binfo != NULL)
//		unload_book(binfo);

	binfo = NULL;

	if(book_path)
		fs_book_path = unicode_to_fs(book_path);
	else
		fs_book_path = NULL;

	if((appendix_path) && (strlen(appendix_path) != 0))
		fs_appendix_path = unicode_to_fs(appendix_path);
	else
		fs_appendix_path = NULL;

	binfo = load_book(fs_book_path, subbook_no,
			  fs_appendix_path, appendix_subbook_no, fg, bg);


	g_free(fs_book_path);
	g_free(fs_appendix_path);

	g_free(book_path);
	g_free(appendix_path);
	g_free(fg);
	g_free(bg);

	if(binfo == NULL){
		LOG(LOG_CRITICAL, _("Failed to load dictionary."));
		return;
	}

	gtk_tree_store_set(GTK_TREE_STORE(dict_store), &iter,
			   DICT_MEMBER_COLUMN, binfo,
			   -1);

	gtk_tree_path_free (path);
}


static gboolean row_drop_possible (GtkTreeDragDest   *drag_dest,
                                       GtkTreePath       *dest,
                                       GtkSelectionData  *selection_data)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreePath *src;

	LOG(LOG_DEBUG, "IN : row_drop_possible()");

	if(dest == NULL)
		return(FALSE);

	if(GTK_TREE_STORE(drag_dest) == GTK_TREE_STORE(dict_store))
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view));
	else if(GTK_TREE_STORE(drag_dest) == GTK_TREE_STORE(web_store))
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(web_view));
	else
		return(FALSE);

	if(gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
		return(FALSE);

	src = gtk_tree_model_get_path(GTK_TREE_MODEL(drag_dest), &iter);

/*
	printf("src = %s, dest = %s\n", 
	       gtk_tree_path_to_string(src),
	       gtk_tree_path_to_string(dest));
*/

	if(gtk_tree_path_get_depth(src) == gtk_tree_path_get_depth(dest)) {
		return(TRUE);
	} else {
		return(FALSE);
	}

}

static void up_item(GtkWidget *widget,gpointer *data)
{
	GtkTreeIter iter;
	GtkTreeIter prev_iter;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreePath *path_orig;

	LOG(LOG_DEBUG, "IN : up_item()");

	edited = FALSE;


	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		popup_warning(_("Please select dictionary."));
		return;
	}

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(dict_store), &iter);
	path_orig = gtk_tree_path_copy(path);

	gtk_tree_path_prev(path);
	if((gtk_tree_path_compare(path, path_orig) != 0) &&
	   gtk_tree_model_get_iter(GTK_TREE_MODEL(dict_store),
				   &prev_iter,
				   path))
	{
		my_gtk_tree_store_swap(dict_store, &iter, &prev_iter);
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view)),
					       &iter);

	} else if(gtk_tree_path_get_depth(path) > 1){
		gtk_tree_path_up(path);
		if(gtk_tree_path_compare(path, path_orig) != 0){
			gtk_tree_path_free(path_orig);
			path_orig = gtk_tree_path_copy(path);
			gtk_tree_path_prev(path);
			if(gtk_tree_path_compare(path, path_orig) != 0){
				GtkTreeIter parent;
				if(gtk_tree_model_get_iter(GTK_TREE_MODEL(dict_store),
							   &parent,
							   path))
				{
					
					GtkTreeIter new_iter;
					gint type;
					gchar *title;
					gchar *book_path;
					gint subbook_no;
					gchar *appendix_path;
					gint appendix_subbook_no;
					gboolean active;
					BOOK_INFO *binfo;
					gboolean editable;
					gchar *fg, *bg;


					gtk_tree_store_append(GTK_TREE_STORE(dict_store),
							      &new_iter,
							      &parent);


					gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
							   &iter, 
							   DICT_TYPE_COLUMN, &type,
							   DICT_TITLE_COLUMN, &title,
							   DICT_PATH_COLUMN, &book_path,
							   DICT_SUBBOOK_NO_COLUMN, &subbook_no,
							   DICT_APPENDIX_PATH_COLUMN, &appendix_path,
							   DICT_APPENDIX_SUBBOOK_NO_COLUMN, &appendix_subbook_no,
							   DICT_ACTIVE_COLUMN, &active,
							   DICT_MEMBER_COLUMN, &binfo,
							   DICT_EDITABLE_COLUMN, &editable,
							   DICT_FGCOLOR_COLUMN, &fg,
							   DICT_BGCOLOR_COLUMN, &bg,
							   -1);

					gtk_tree_store_set(dict_store, &new_iter,
							   DICT_TYPE_COLUMN, type,
							   DICT_TITLE_COLUMN, title,
							   DICT_PATH_COLUMN, book_path,
							   DICT_SUBBOOK_NO_COLUMN, subbook_no,
							   DICT_APPENDIX_PATH_COLUMN, appendix_path,
							   DICT_APPENDIX_SUBBOOK_NO_COLUMN, appendix_subbook_no,
							   DICT_ACTIVE_COLUMN, active,
							   DICT_MEMBER_COLUMN, binfo,
							   DICT_EDITABLE_COLUMN, editable,
							   DICT_FGCOLOR_COLUMN, fg,
							   DICT_BGCOLOR_COLUMN, bg,

							   -1);
					g_free(title);
					g_free(book_path);
					g_free(appendix_path);
					g_free(fg);
					g_free(bg);

					gtk_tree_store_remove(GTK_TREE_STORE(dict_store),
							      &iter);

					gtk_tree_view_expand_row(GTK_TREE_VIEW(dict_view), path, TRUE);
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view)),
								       &new_iter);

				}
			}
		}
	}

	gtk_tree_path_free(path);
	gtk_tree_path_free(path_orig);

	LOG(LOG_DEBUG, "OUT : up_item()");
	return;
}


static void down_item(GtkWidget *widget,gpointer *data)
{
	GtkTreeIter iter;
	GtkTreeIter next_iter;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreePath *path_orig;

	LOG(LOG_DEBUG, "IN : down_item()");

	edited = FALSE;


	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		popup_warning(_("Please select dictionary."));
		return;
	}

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(dict_store), &iter);
	path_orig = gtk_tree_path_copy(path);

	gtk_tree_path_next(path);
	if((gtk_tree_path_compare(path, path_orig) != 0) &&
	   gtk_tree_model_get_iter(GTK_TREE_MODEL(dict_store),
				   &next_iter, path))
	{
			my_gtk_tree_store_swap(dict_store, &iter, &next_iter);
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view)),
						       &iter);
	} else if(gtk_tree_path_get_depth(path) > 1){
		gtk_tree_path_up(path);
		if(gtk_tree_path_compare(path, path_orig) != 0){
			gtk_tree_path_free(path_orig);
			path_orig = gtk_tree_path_copy(path);
			gtk_tree_path_next(path);
			if(gtk_tree_path_compare(path, path_orig) != 0){
				GtkTreeIter parent;
				if(gtk_tree_model_get_iter(GTK_TREE_MODEL(dict_store),
							   &parent,
							   path))
				{
					
					GtkTreeIter new_iter;
					gint type;
					gchar *title;
					gchar *book_path;
					gint subbook_no;
					gchar *appendix_path;
					gint appendix_subbook_no;
					gboolean active;
					BOOK_INFO *binfo;
					gboolean editable;
					gchar *fg, *bg;

					gtk_tree_store_prepend(GTK_TREE_STORE(dict_store),
							      &new_iter,
							      &parent);


					gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
							   &iter, 
							   DICT_TYPE_COLUMN, &type,
							   DICT_TITLE_COLUMN, &title,
							   DICT_PATH_COLUMN, &book_path,
							   DICT_SUBBOOK_NO_COLUMN, &subbook_no,
							   DICT_APPENDIX_PATH_COLUMN, &appendix_path,
							   DICT_APPENDIX_SUBBOOK_NO_COLUMN, &appendix_subbook_no,
							   DICT_ACTIVE_COLUMN, &active,
							   DICT_MEMBER_COLUMN, &binfo,
							   DICT_EDITABLE_COLUMN, &editable,
							   DICT_FGCOLOR_COLUMN, &fg,
							   DICT_BGCOLOR_COLUMN, &bg,
							   -1);

					gtk_tree_store_set(dict_store, &new_iter,
							   DICT_TYPE_COLUMN, type,
							   DICT_TITLE_COLUMN, title,
							   DICT_PATH_COLUMN, book_path,
							   DICT_SUBBOOK_NO_COLUMN, subbook_no,
							   DICT_APPENDIX_PATH_COLUMN, appendix_path,
							   DICT_APPENDIX_SUBBOOK_NO_COLUMN, appendix_subbook_no,
							   DICT_ACTIVE_COLUMN, active,
							   DICT_MEMBER_COLUMN, binfo,
							   DICT_EDITABLE_COLUMN, editable,
							   DICT_FGCOLOR_COLUMN, fg,
							   DICT_BGCOLOR_COLUMN, bg,
							   -1);
					g_free(title);
					g_free(book_path);
					g_free(appendix_path);
					g_free(fg);
					g_free(bg);

					gtk_tree_store_remove(GTK_TREE_STORE(dict_store),
							      &iter);

					gtk_tree_view_expand_row(GTK_TREE_VIEW(dict_view), path, TRUE);
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view)),
								       &new_iter);

				}
			}
		}
	}

	gtk_tree_path_free(path);
	gtk_tree_path_free(path_orig);

	LOG(LOG_DEBUG, "OUT : down_item()");
	return;
}

static void remove_item(GtkWidget *widget, gpointer *data)
{

	GtkTreeIter iter;
	GtkTreeSelection *selection;

	LOG(LOG_DEBUG, "IN : remove_item()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		edited = FALSE;
		gtk_tree_store_remove(GTK_TREE_STORE(dict_store), &iter);
	}

	LOG(LOG_DEBUG, "OUT : remove_item()");
}



static void add_group(GtkWidget *widget,gpointer *data)
{

	GtkTreeIter iter;
	char *name;

	LOG(LOG_DEBUG, "IN : add_grop()");

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_group_name));
	name = g_strdup(name);
	remove_space(name);
	if(strlen(name) == 0){
	  return;
	}
	gtk_tree_store_append(GTK_TREE_STORE(dict_store), &iter, NULL);
	gtk_tree_store_set(GTK_TREE_STORE(dict_store), &iter,
			   DICT_TYPE_COLUMN, 0,
			   DICT_TITLE_COLUMN, name,
			   DICT_EDITABLE_COLUMN, TRUE,
			   -1);

	gtk_entry_set_text(GTK_ENTRY(entry_group_name), "");

	g_free(name);
	
	LOG(LOG_DEBUG, "OUT : add_grop()");
}

static int enumerate_subbook(char *path)
{
	EB_Book book;
	EB_Error_Code error_code;
	int subcount, i;
	EB_Subbook_Code sublist[EB_MAX_SUBBOOKS];
	char buff[512];
	BOOK_INFO *binfo;
	EB_Multi_Search_Code multi_list[EB_MAX_MULTI_SEARCHES];
	int multi_count;

	GtkTreeIter parent_iter;
	GtkTreeIter child_iter;
	gchar *utf_str;
	gboolean group_exist;
	gchar *title;
	GtkTreePath *tree_path;
	gchar *utf_path;

	LOG(LOG_DEBUG, "IN : enumerate_subbook(%s)", path);

	eb_initialize_book(&book);
	error_code = eb_bind(&book, path);
	if(error_code != EB_SUCCESS){
		LOG(LOG_INFO,"Failed to bind book. Maybe wrong directory.");
		return(1);
	}

	error_code = eb_subbook_list(&book, sublist, &subcount);
	if(error_code != EB_SUCCESS){
		LOG(LOG_INFO,"Failed to bind book. Maybe wrong directory.");
		return(1);
	}
	for(i=0 ; i<subcount ; i++){
		error_code = eb_subbook_directory2(
			&book,
			sublist[i],
			buff);
		if (error_code != EB_SUCCESS){
			LOG(LOG_CRITICAL, _("Failed to get subbook directory."));
			return(1);
		}

		error_code = eb_subbook_title2(
			&book,
			sublist[i],
			buff);
		if (error_code != EB_SUCCESS){
			LOG(LOG_CRITICAL, _("Failed to get title."));
			return(1);
		}

		error_code = eb_set_subbook(&book, sublist[i]); 
		if (error_code != EB_SUCCESS){
			LOG(LOG_CRITICAL, "failed to set subbook %s, %d: %s",
				path, sublist[i],
				ebook_error_message(error_code));
			return(1);
		}

		eb_multi_search_list(&book, multi_list, &multi_count);
		if(eb_have_word_search(&book) ||
		   eb_have_endword_search(&book) ||
		   eb_have_exactword_search(&book) ||
		   eb_have_keyword_search(&book) ||
		   (multi_count != 0))
		{

			utf_str = iconv_convert("euc-jp", "utf-8", buff);
			binfo = load_book(path, sublist[i], NULL, 0, NULL, NULL);
			if(binfo == NULL){
				popup_warning(_("Failed to load book."));
				return(1);
			}


			// "Disk Search Result" というグループはあるか?
			group_exist = FALSE;

			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE){
				do { 
					gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
							   &parent_iter, 
							   DICT_TITLE_COLUMN, &title,
							   -1);

					if(strcmp(title, "Disk Search Result") == 0){
						group_exist = TRUE;
						g_free(title);
						break;
					}
					g_free(title);
				} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE);
		       }

			if(group_exist == FALSE){

				gtk_tree_store_append(GTK_TREE_STORE(dict_store), &parent_iter, NULL);
				gtk_tree_store_set(GTK_TREE_STORE(dict_store), &parent_iter,
						   DICT_TYPE_COLUMN, 0,
						   DICT_TITLE_COLUMN, "Disk Search Result",
						   DICT_EDITABLE_COLUMN, TRUE,
						   -1);

			}

			utf_path = fs_to_unicode(path);

			gtk_tree_store_append(GTK_TREE_STORE(dict_store), &child_iter, &parent_iter);

			gtk_tree_store_set (GTK_TREE_STORE(dict_store), &child_iter,
					    DICT_TYPE_COLUMN, 1,
					    DICT_TITLE_COLUMN, utf_str,
					    DICT_PATH_COLUMN, utf_path,
					    DICT_SUBBOOK_NO_COLUMN, sublist[i],
					    DICT_APPENDIX_PATH_COLUMN, NULL,
					    DICT_APPENDIX_SUBBOOK_NO_COLUMN, 0,
					    DICT_ACTIVE_COLUMN, TRUE,
					    DICT_MEMBER_COLUMN, binfo,
					    DICT_EDITABLE_COLUMN, FALSE,
					    DICT_FGCOLOR_COLUMN, NULL,
					    DICT_BGCOLOR_COLUMN, NULL,
					    -1);

			tree_path = gtk_tree_model_get_path(GTK_TREE_MODEL(dict_store), &parent_iter);
			gtk_tree_view_expand_row(GTK_TREE_VIEW(dict_view), tree_path, TRUE);
			gtk_tree_path_free (tree_path);

			free(utf_str);
		} else {
			LOG(LOG_INFO, "book %s does not have search method.", path);
		}

	}
	eb_finalize_book(&book);

	LOG(LOG_DEBUG, "OUT : enumerate_subbook()");

	return(0);
}

gint max_depth = 0;

static void search_dictionary_recursive(gchar *dirname, gint depth)
{
	GDir *dir;
	const gchar *name;
	gchar fullpath[512];

	LOG(LOG_DEBUG, "IN : search_dictionary_recursive(%s, %d)", dirname, depth);
	
	if((dir = g_dir_open(dirname, 0, NULL)) == NULL){
		LOG(LOG_CRITICAL, "Failed to open directory %s", dirname);
		LOG(LOG_DEBUG, "OUT : search_dictionary_recursive()");
		return;
	}
	
	while((name = g_dir_read_name(dir)) != NULL){
		if(strcmp(dirname,"/")==0){
			sprintf(fullpath,"/%s",name);
		} else {
			sprintf(fullpath,"%s%s%s",dirname, DIR_DELIMITER, name);
		}

		if(g_file_test(fullpath, G_FILE_TEST_IS_REGULAR) == TRUE){
			if((strcasecmp(name, "catalog") == 0) ||
			   (strcasecmp(name, "catalogs") == 0)){
				enumerate_subbook(dirname);
			}
		} else if(g_file_test(fullpath, G_FILE_TEST_IS_DIR) == TRUE){
			if(depth < max_depth)
				search_dictionary_recursive(fullpath, depth+1);
		}
	}
	g_dir_close(dir);

	LOG(LOG_DEBUG, "OUT : search_dictionary_recursive()");
}

static void search_disk(GtkWidget *widget, gpointer *data)
{
	gchar *dirname;

	LOG(LOG_DEBUG, "IN : search_disk()");

	dirname = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_directory_name));
	dirname = g_strdup(dirname);
	remove_space(dirname);

#ifdef __WIN32__
	if((strlen(dirname) != 1) &&(dirname[strlen(dirname) - 1] == '\\'))
#else
	if((strlen(dirname) != 1) &&(dirname[strlen(dirname) - 1] == '/'))
#endif

		dirname[strlen(dirname) - 1] = '\0';
	if(strlen(dirname) == 0){
		popup_warning(_("Please enter directory name"));
		return;
	}


	max_depth = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_search_depth));

	search_dictionary_recursive(dirname, 0);
	g_free(dirname);

	LOG(LOG_DEBUG, "OUT : search_disk()");
}

static gboolean drag_data_received(GtkTreeDragDest   *drag_dest,
				   GtkTreePath       *dest,
				   GtkSelectionData  *selection_data)
{
	LOG(LOG_DEBUG, "IN : drag_data_received()");
	last_binfo = NULL;
	LOG(LOG_DEBUG, "OUT : drag_data_received()");
	return(FALSE);
	
}

static gboolean update_last_dictionary()
{
	gchar *title;
	gchar *book_path;
	gint subbook_no;
	gchar *appendix_path;
	gint appendix_subbook_no;
	gchar *fg, *bg;
	GtkTreeSelection *select;
	gchar *fs_book_path;
	gchar *fs_appendix_path;
	BOOK_INFO *binfo=NULL;
	gchar *tmp;

	LOG(LOG_DEBUG, "IN : update_last_dictionary()");

	if(edited == FALSE)
		goto END;

	if(popup_active())
		goto END;

	if(last_binfo == NULL) {
		goto END;
	}

	title = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_title));
	if(!title || (strlen(title) == 0)) {
		popup_error(_("Please specify title."));
		goto FAILED;
	}

	book_path = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_book_path));
	if(!book_path || (strlen(book_path) == 0)) {
		popup_error(_("Please specify book path."));
		goto FAILED;
	}

	appendix_path = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_appendix_path));

	tmp = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_subbook_no));
	if(tmp)
		subbook_no = atoi(tmp);
	else {
		popup_error(_("Subbook number incorrect."));
		goto FAILED;
	}

	tmp = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_appendix_subbook_no));
	if(tmp)
		appendix_subbook_no = atoi(tmp);
	else
		appendix_subbook_no = 0;		


	if(book_path)
		fs_book_path = unicode_to_fs(book_path);
	else
		fs_book_path = NULL;

	if((appendix_path) && (strlen(appendix_path) != 0))
		fs_appendix_path = unicode_to_fs(appendix_path);
	else
		fs_appendix_path = NULL;

	if(strlen(fg_color) == 0)
		fg = NULL;
	else
		fg = fg_color;

	if(strlen(bg_color) == 0)
		bg = NULL;
	else
		bg = bg_color;

	binfo = load_book(fs_book_path, subbook_no,
			  fs_appendix_path, appendix_subbook_no, fg, bg);

	if(binfo == NULL)
		goto FAILED;
	gtk_tree_store_set(dict_store, &last_iter,
			   DICT_TYPE_COLUMN, 1,
			   DICT_TITLE_COLUMN, title,
			   DICT_PATH_COLUMN, book_path,
			   DICT_SUBBOOK_NO_COLUMN, subbook_no,
			   DICT_APPENDIX_PATH_COLUMN, appendix_path,
			   DICT_APPENDIX_SUBBOOK_NO_COLUMN, appendix_subbook_no,
			   DICT_ACTIVE_COLUMN, last_active,
			   DICT_MEMBER_COLUMN, binfo,
			   DICT_EDITABLE_COLUMN, FALSE,
			   DICT_FGCOLOR_COLUMN, fg,
			   DICT_BGCOLOR_COLUMN, bg,
			   -1);

	last_binfo = binfo;

	edited = FALSE;

 END:
	LOG(LOG_DEBUG, "OUT : update_last_dictionary() = TRUE");
	return(TRUE);

 FAILED:
	rewinding = TRUE;
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(dict_view), FALSE);
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view));
	gtk_tree_selection_select_iter(select, &last_iter);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(dict_view), TRUE);

	LOG(LOG_DEBUG, "OUT : update_last_dictionary() = FALSE");
	return(FALSE);

}

static void dict_selection_changed(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;

	gchar *title;
	gchar *book_path;
	gint  subbook_no;
	gchar *appendix_path=NULL;
	gint  appendix_subbook_no=0;
	gboolean active;
	BOOK_INFO *binfo;
	gchar *fg;
	gchar *bg;
	gint  type;
	gchar buff[128];

	LOG(LOG_DEBUG, "IN : dict_selection_changed()");

	if(rewinding == TRUE){
		rewinding = FALSE;
		goto END;
	} else {
		if(update_last_dictionary() == FALSE)
			goto END;
	}

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		//popup_warning(_("Please select group."));
		LOG(LOG_DEBUG, "OUT : dict_selection_changed()");
		return;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(dict_store), &iter,
			   DICT_TYPE_COLUMN, &type,
			   DICT_TITLE_COLUMN, &title,
			   DICT_PATH_COLUMN, &book_path,
			   DICT_SUBBOOK_NO_COLUMN, &subbook_no,
			   DICT_APPENDIX_PATH_COLUMN, &appendix_path,
			   DICT_APPENDIX_SUBBOOK_NO_COLUMN, &appendix_subbook_no,
			   DICT_ACTIVE_COLUMN, &active,
			   DICT_MEMBER_COLUMN, &binfo,
			   DICT_FGCOLOR_COLUMN, &fg,
			   DICT_BGCOLOR_COLUMN, &bg,
			   -1);

	// Group
	if(type == 0)
		goto END;

	if(title != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_title), title);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_title), "");

	if(book_path != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_book_path), book_path);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_book_path), "");

	if(appendix_path != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_appendix_path), appendix_path);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_appendix_path), "");


	sprintf(buff, "%d", subbook_no);
	gtk_entry_set_text(GTK_ENTRY(entry_subbook_no), buff);

	sprintf(buff, "%d", appendix_subbook_no);
	gtk_entry_set_text(GTK_ENTRY(entry_appendix_subbook_no), buff);

	if(fg)
		sprintf(fg_color, "%s", fg);
	else
		fg_color[0] = '\0';

	if(bg)
		sprintf(bg_color, "%s", bg);
	else
		bg_color[0] = '\0';


	if(fg == NULL) {
		if(bg == NULL)
			sprintf(buff, "%s", _("Sample"));
		else
			sprintf(buff, "<span background=\"%s\">%s</span>",  bg_color, _("Sample"));
	} else {
		if(bg == NULL)
			sprintf(buff, "<span foreground=\"%s\">%s</span>", fg_color, _("Sample"));
		else
			sprintf(buff, "<span foreground=\"%s\" background=\"%s\">%s</span>", fg_color, bg_color, _("Sample"));
	}

	gtk_label_set_text(GTK_LABEL(label_color), buff);
	gtk_label_set_use_markup (GTK_LABEL (label_color), TRUE);

	gtk_tree_selection_get_selected(selection, NULL, &last_iter);
	last_binfo = binfo;
	last_active = active;
	edited = TRUE;



 END:
	g_free(title);
	g_free(book_path);
	g_free(appendix_path);
	g_free(fg);
	g_free(bg);

	LOG(LOG_DEBUG, "OUT : dict_selection_changed()");
}

static void ok_colorsel(GtkWidget *widget,gpointer *data){

	GdkColor color;
	gchar *color_name;
	gchar buff[128];
	gchar *title;
	
	LOG(LOG_DEBUG, "IN : ok_colorsel()");

	gtk_grab_remove(colorsel_dlg);

	gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(colorsel_dlg)->colorsel), &color);
	color_name = gtk_color_selection_palette_to_string(&color, 1);

	if(color_no == 0){
		sprintf(fg_color, "%s", color_name);
	} else {
		sprintf(bg_color, "%s", color_name);
	}

	title = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_title));
	if(strlen(fg_color) == 0){
		if(strlen(bg_color) == 0)
			sprintf(buff, _("Sample"));
		else
			sprintf(buff, "<span background=\"%s\">%s</span>", bg_color, _("Sample"));
	} else {
		if(strlen(bg_color) == 0)
			sprintf(buff, "<span foreground=\"%s\">%s</span>", fg_color, _("Sample"));
			else
				sprintf(buff, "<span foreground=\"%s\" background=\"%s\">%s</span>", fg_color, bg_color, _("Sample"));
	}
	gtk_label_set_text(GTK_LABEL(label_color), buff);
	gtk_label_set_use_markup (GTK_LABEL (label_color), TRUE);

	gtk_widget_destroy(colorsel_dlg);

	LOG(LOG_DEBUG, "OUT : ok_colorsel()");
}

static void delete_colorsel( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	LOG(LOG_DEBUG, "IN : delete_colorsel()");

	ok_colorsel(NULL, NULL);

	LOG(LOG_DEBUG, "OUT : delete_colorsel()");
}

static void show_colorsel(GtkWidget *widget,gpointer *data){

	GdkColor color;
	
	LOG(LOG_DEBUG, "IN : show_colorsel()");
	
	color_no = (gint)data;


	colorsel_dlg = gtk_color_selection_dialog_new(_("Choose Color"));

	g_signal_connect (G_OBJECT(colorsel_dlg), "delete_event",
			  G_CALLBACK(delete_colorsel), NULL);

	g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(colorsel_dlg)->ok_button), "clicked",
			 G_CALLBACK(ok_colorsel), NULL);

	g_signal_connect_swapped(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(colorsel_dlg)->cancel_button), "clicked",
				G_CALLBACK(gtk_widget_destroy), (gpointer)colorsel_dlg);


	g_assert(color_no < NUM_COLORS);

	if(color_no == 0){
		gdk_color_parse(fg_color, &color);
	} else {
		gdk_color_parse(bg_color, &color);
	}

	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(colorsel_dlg)->colorsel), &color);

	gtk_widget_realize(colorsel_dlg);
	center_dialog(pref_dlg, colorsel_dlg);
	gtk_widget_show_all(colorsel_dlg);

	gtk_grab_add(colorsel_dlg);

	LOG(LOG_DEBUG, "OUT : show_colorsel()");
}


static void clear_color(GtkWidget *widget,gpointer *data){

	gchar buff[128];
	
	LOG(LOG_DEBUG, "IN : clear_color()");

	fg_color[0] = '\0';
	bg_color[0] = '\0';
	sprintf(buff, _("Sample"));
	gtk_label_set_text(GTK_LABEL(label_color), buff);
	gtk_label_set_use_markup (GTK_LABEL (label_color), FALSE);

	LOG(LOG_DEBUG, "OUT : clear_color()");
}

GtkWidget *pref_start_dictgroup()
{
	GtkWidget *button;
	GtkWidget *vbox_l;
	GtkWidget *vbox_r;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *scroll;
	GtkObject *adj;

	GtkCellRenderer *renderer;
	GtkTreeSelection *select;


	GtkTreeDragDestIface *iface;


	LOG(LOG_DEBUG, "IN : pref_start_dictgroup()");


	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);

	vbox_l = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox_l), 2);
	gtk_box_pack_start (GTK_BOX(hbox)
			    , vbox_l,TRUE, TRUE, 0);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX(vbox_l)
			    , frame,TRUE, TRUE, 0);
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (frame), scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	dict_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(dict_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(dict_view), TRUE);
	gtk_tree_view_expand_all(GTK_TREE_VIEW(dict_view));
	gtk_container_add (GTK_CONTAINER (scroll), dict_view);

	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(dict_view));
	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(select), "changed",
			 G_CALLBACK (dict_selection_changed),
			 NULL);


/*
	gtk_tree_view_enable_model_drag_source(
		GTK_TREE_VIEW(dict_view),
		GDK_BUTTON1_MASK,  row_targets, G_N_ELEMENTS(row_targets), 
		GDK_ACTION_MOVE);

	gtk_tree_view_enable_model_drag_dest(
		GTK_TREE_VIEW(dict_view),
		row_targets, G_N_ELEMENTS(row_targets), GDK_ACTION_MOVE);
*/

	iface = GTK_TREE_DRAG_DEST_GET_IFACE (dict_store);
	iface->row_drop_possible = row_drop_possible;

	g_signal_connect(G_OBJECT(dict_view), "drag_data_received",
			 G_CALLBACK(drag_data_received), NULL);

	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(dict_view), TRUE);


	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)DICT_TITLE_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(dict_view),
						     -1, _("Name"), renderer,
						     "text", DICT_TITLE_COLUMN,
						     "editable", DICT_EDITABLE_COLUMN,
						     NULL);

/*
	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)DICT_PATH_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(dict_view),
						     -1, _("Path"), renderer,
						     "text", DICT_PATH_COLUMN,
						     "visible", DICT_TYPE_COLUMN,
						     NULL);

	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)DICT_SUBBOOK_NO_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(dict_view),
						     -1, _("Subbook Number"), renderer,
						     "text", DICT_SUBBOOK_NO_COLUMN,
						     "visible", DICT_TYPE_COLUMN,
						     NULL);

	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)DICT_APPENDIX_PATH_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(dict_view),
						     -1, _("Appendix Path"), renderer,
						     "text", DICT_APPENDIX_PATH_COLUMN,
						     "editable", DICT_EDITABLE_COLUMN,
						     "visible", DICT_TYPE_COLUMN,
						     NULL);

	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)DICT_APPENDIX_SUBBOOK_NO_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(dict_view),
						     -1, _("Appendix Subbook Number"), renderer,
						     "text", DICT_APPENDIX_SUBBOOK_NO_COLUMN,
						     "editable", DICT_EDITABLE_COLUMN,
						     "visible", DICT_TYPE_COLUMN,
						     NULL);

//	renderer = gtk_cell_renderer_color_new();
	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)DICT_BGCOLOR_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(dict_view),
						     -1, _("BG Color"), renderer,
						     "text", DICT_BGCOLOR_COLUMN,
						     "editable", DICT_EDITABLE_COLUMN,
						     "cell-background", DICT_BGCOLOR_COLUMN,
						     "visible", DICT_TYPE_COLUMN,
						     NULL);



	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)DICT_FGCOLOR_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(dict_view),
						     -1, _("FG Color"), renderer,
						     "text", DICT_FGCOLOR_COLUMN,
						     "editable", DICT_EDITABLE_COLUMN,
						     "cell-background", DICT_FGCOLOR_COLUMN,
						     "visible", DICT_TYPE_COLUMN,
						     NULL);



	// すべてのカラムをリサイズできるようにする
	{
		gint i;
		GtkTreeViewColumn *column;

		for(i=0;;i++){
			column = gtk_tree_view_get_column(GTK_TREE_VIEW(dict_view), i);
			if(column == NULL)
				break;
			gtk_tree_view_column_set_resizable(column, TRUE);
		}
	}

*/

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_l),
			   hbox2,FALSE, FALSE, 2);

	label = gtk_label_new(_("Group name"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   label,FALSE, FALSE, 2);

	entry_group_name = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox2),
			   entry_group_name,TRUE, TRUE, 2);
	g_signal_connect(G_OBJECT (entry_group_name), "activate",
			 G_CALLBACK(add_group), (gpointer)NULL);

	button = gtk_button_new_with_label(_("Add"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 2);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(add_group), (gpointer)button);


	button = gtk_button_new_with_label(_("Remove"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 2);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(remove_item), (gpointer)button);


	button = gtk_button_new_with_label(_("Up"));
	gtk_box_pack_start (GTK_BOX(hbox2),
			    button,FALSE,FALSE, 2);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(up_item), (gpointer)button);

	button = gtk_button_new_with_label(_("Down"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 2);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(down_item), (gpointer)button);


	hbox2 = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox_l),
			   hbox2,FALSE, FALSE, 2);

	label = gtk_label_new(_("Path"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   label,FALSE, FALSE, 2);

	entry_directory_name = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox2),
			   entry_directory_name, FALSE, FALSE, 2);
	g_signal_connect(G_OBJECT (entry_directory_name), "activate",
			 G_CALLBACK(search_disk), (gpointer)NULL);

	label = gtk_label_new(_("Depth"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   label,FALSE, FALSE, 2);

	adj = gtk_adjustment_new( 1, //value
				  0, // lower
				  20, //upper
                                  1, // step increment
                                  10,// page_increment,
                                  (gfloat)0.0);
	spin_search_depth = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_box_pack_start(GTK_BOX(hbox2),
			   spin_search_depth,FALSE, FALSE, 2);
	gtk_tooltips_set_tip(tooltip, spin_search_depth, 
			     _("Specify search depth. 0 means to search only specified directory."),"Private");



	button = gtk_button_new_with_label(_("Search Disk"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 2);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(search_disk), (gpointer)button);


	// 右半分

	frame = gtk_frame_new(NULL);
	gtk_box_pack_start (GTK_BOX(hbox)
			    , frame,TRUE, TRUE, 0);
	
	vbox_r = gtk_vbox_new(FALSE,5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox_r), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox_r);


	label = gtk_label_new(_("Name"));
	gtk_box_pack_start (GTK_BOX(vbox_r), label,
			    FALSE, FALSE, 0);

	entry_title = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX(vbox_r), entry_title,
			    FALSE, FALSE, 0);

	label = gtk_label_new(_("Path"));
	gtk_box_pack_start (GTK_BOX(vbox_r), label,
			    FALSE, FALSE, 0);

	entry_book_path = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(entry_book_path), FALSE);
	gtk_box_pack_start (GTK_BOX(vbox_r), entry_book_path,
			    FALSE, FALSE, 0);
	gtk_widget_set_sensitive(entry_book_path, FALSE);

	label = gtk_label_new(_("Subbook Number"));
	gtk_box_pack_start (GTK_BOX(vbox_r), label,
			    FALSE, FALSE, 0);

	entry_subbook_no = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(entry_subbook_no), FALSE);
	gtk_box_pack_start (GTK_BOX(vbox_r), entry_subbook_no,
			    FALSE, FALSE, 0);
	gtk_widget_set_sensitive(entry_subbook_no, FALSE);

	label = gtk_label_new(_("Appendix Path"));
	gtk_box_pack_start (GTK_BOX(vbox_r), label,
			    FALSE, FALSE, 0);

	entry_appendix_path = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX(vbox_r), entry_appendix_path,
			    FALSE, FALSE, 0);

	label = gtk_label_new(_("Appendix Subbook Number"));
	gtk_box_pack_start (GTK_BOX(vbox_r), label,
			    FALSE, FALSE, 0);

	entry_appendix_subbook_no = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX(vbox_r), entry_appendix_subbook_no,
			    FALSE, FALSE, 0);

	
	hbox2 = gtk_hbox_new(FALSE,2);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 2);
	gtk_box_pack_start (GTK_BOX(vbox_r), hbox2,
			    FALSE, FALSE, 0);

	label_color = gtk_label_new(_("Sample"));
	gtk_label_set_use_markup (GTK_LABEL (label_color), TRUE);
	gtk_box_pack_start (GTK_BOX(hbox2), label_color,
			    TRUE, TRUE, 0);

	button = gtk_button_new_with_label(_("FG"));
	gtk_box_pack_start (GTK_BOX(hbox2), button,
			    FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_colorsel), (gpointer)0);

	button = gtk_button_new_with_label(_("BG"));
	gtk_box_pack_start (GTK_BOX(hbox2), button,
			    FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_colorsel), (gpointer)1);

	button = gtk_button_new_with_label(_("Clear"));
	gtk_box_pack_start (GTK_BOX(hbox2), button,
			    FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(clear_color), (gpointer)1);

	fg_color[0] = '\0';
	bg_color[0] = '\0';
	

	LOG(LOG_DEBUG, "OUT : pref_start_dictgroup()");

	return(hbox);

}

#define G_NODE(node) ((GNode *)node)
#define VALID_ITER(iter, tree_store) (iter!= NULL && iter->user_data != NULL && tree_store->stamp == iter->stamp)


/**
 * gtk_tree_store_swap:
 * copied from GTK+-2.2
 **/
void
my_gtk_tree_store_swap (GtkTreeStore *tree_store,
		     GtkTreeIter  *a,
		     GtkTreeIter  *b)
{
	GNode *tmp, *node_a, *node_b, *parent_node;
	GNode *a_prev, *a_next, *b_prev, *b_next;
	gint i, a_count, b_count, length, *order;
	GtkTreePath *path_a, *path_b;
	GtkTreeIter parent;

	g_return_if_fail (GTK_IS_TREE_STORE (tree_store));
	g_return_if_fail (VALID_ITER (a, tree_store));
	g_return_if_fail (VALID_ITER (b, tree_store));

	node_a = G_NODE (a->user_data);
	node_b = G_NODE (b->user_data);

	/* basic sanity checking */
	if (node_a == node_b)
		return;

	path_a = gtk_tree_model_get_path (GTK_TREE_MODEL (tree_store), a);
	path_b = gtk_tree_model_get_path (GTK_TREE_MODEL (tree_store), b);

	g_return_if_fail (path_a && path_b);

	gtk_tree_path_up (path_a);
	gtk_tree_path_up (path_b);

	if((gtk_tree_path_get_depth(path_a) != 0) ||
	   (gtk_tree_path_get_depth(path_b) != 0)){
		if (gtk_tree_path_compare (path_a, path_b))
		{
			gtk_tree_path_free (path_a);
			gtk_tree_path_free (path_b);

			g_warning ("Given childs are not in the same level\n");
			return;
		}
	}

	if(gtk_tree_path_get_depth(path_a) == 0){
		parent_node = G_NODE (tree_store->root);
	} else {
		gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_store), &parent, path_a);
		parent_node = G_NODE (parent.user_data);
	}

	gtk_tree_path_free (path_b);

  /* old links which we have to keep around */
	a_prev = node_a->prev;
	a_next = node_a->next;

	b_prev = node_b->prev;
	b_next = node_b->next;

  /* fix up links if the nodes are next to eachother */
	if (a_prev == node_b)
		a_prev = node_a;
	if (a_next == node_b)
		a_next = node_a;

	if (b_prev == node_a)
		b_prev = node_b;
	if (b_next == node_a)
		b_next = node_b;

  /* counting nodes */
	tmp = parent_node->children;
	i = a_count = b_count = 0;
	while (tmp)
	{
		if (tmp == node_a)
			a_count = i;
		if (tmp == node_b)
			b_count = i;

		tmp = tmp->next;
		i++;
	}
	length = i;

  /* hacking the tree */
	if (!a_prev)
		parent_node->children = node_b;
	else
		a_prev->next = node_b;

	if (a_next)
		a_next->prev = node_b;

	if (!b_prev)
		parent_node->children = node_a;
	else
		b_prev->next = node_a;

	if (b_next)
		b_next->prev = node_a;

	node_a->prev = b_prev;
	node_a->next = b_next;

	node_b->prev = a_prev;
	node_b->next = a_next;

  /* emit signal */
	order = g_new (gint, length);
	for (i = 0; i < length; i++)
		if (i == a_count)
			order[i] = b_count;
		else if (i == b_count)
			order[i] = a_count;
		else
			order[i] = i;

	if(gtk_tree_path_get_depth(path_a) == 0){
		gtk_tree_model_rows_reordered (GTK_TREE_MODEL (tree_store),
					       path_a,
					       NULL, order);
	} else {
		gtk_tree_model_rows_reordered (GTK_TREE_MODEL (tree_store),
					       path_a,
					       &parent, order);
	}
	gtk_tree_path_free (path_a);
	g_free (order);
}

