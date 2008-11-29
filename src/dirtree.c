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

#include "jcode.h"
#include "grep.h"
#include "pixmap.h"
#include "pref_io.h"
#include "dirtree.h"

static GtkTreeStore *directory_store=NULL;
GtkWidget *directory_view=NULL;

GList *active_dir_list=NULL;

static GdkPixbuf *pixbuf_file;
static GdkPixbuf *pixbuf_folder_closed;
static GdkPixbuf *pixbuf_folder_open;

gchar *unicode_to_fs(gchar *from)
{
	gchar *to;

	to = iconv_convert("utf-8", fs_codeset, from);

	return(to);
}

gchar *fs_to_unicode(gchar *from)
{
	gchar *to;

	to = iconv_convert(fs_codeset, "utf-8", from);

	return(to);
}

gchar *native_to_generic(gchar *from)
{
#ifdef __WIN32__
	gchar buff[512];
	gint i, j;
#endif
	gchar *p;
	LOG(LOG_DEBUG, "IN : native_to_generic(%s)", from);

#ifdef __WIN32__
//	if((from[1] != ':') || (from[2] != '\\')){
	if(from[1] != ':'){
		LOG(LOG_DEBUG, "OUT : native_to_generic() = NULL");
		return(NULL);
	}

	buff[0] = '/';
	buff[1] = from[0];

	i = j = 2;
	while(1){
		if(from[i] == '\\')
			buff[j] = '/';
		else 
			buff[j] = from[i];
		if(from[i] == '\0')
			break;
		i ++;
		j ++;
	}
	
	p = fs_to_unicode(buff);

	LOG(LOG_DEBUG, "OUT : native_to_generic() = %s", p);
	return(p);
#else
	if(from[0] != '/') {
		LOG(LOG_DEBUG, "OUT : native_to_generic() = NULL");
		return(NULL);
	}

	p = fs_to_unicode(from);

	LOG(LOG_DEBUG, "OUT : native_to_generic() = %s", p);
	return(p);
#endif
}

gchar *generic_to_native(gchar *from)
{
#ifdef __WIN32__
	gchar buff[512];
	gint i, j;
#endif
	gchar *p;

	LOG(LOG_DEBUG, "IN : generic_to_native(%s)", from);

	if(from[0] != '/') {
		LOG(LOG_DEBUG, "OUT : generic_to_native() = NULL");
		return(NULL);
	}

#ifdef __WIN32__

	p = unicode_to_fs(from);
	if(p == NULL) {
		LOG(LOG_DEBUG, "OUT : generic_to_native() = NULL");
		return(NULL);
	}
	
	buff[0] = p[1];
	buff[1] = ':';

	i = j = 2;
	while(1){
		if(p[i] == '/')
			buff[j] = '\\';
		else 
			buff[j] = p[i];
		if(p[i] == '\0')
			break;
		i ++;
		j ++;
	}

	if(strlen(buff) == 2)
		strcat(buff, "\\");

	g_free(p);

	LOG(LOG_DEBUG, "OUT : generic_to_native() = %s", buff);
	return(strdup(buff));
#else
	p = unicode_to_fs(from);

	LOG(LOG_DEBUG, "OUT : generic_to_native() = %s", p);
	return(p);
#endif
}



static gchar *compose_full_path(GtkTreePath *path);

enum
{
	DIR_ACTIVATABLE_COLUMN,
	DIR_ACTIVE_COLUMN,
	DIR_PIXBUF_COLUMN,
	DIR_PIXBUF_CLOSED_COLUMN,
	DIR_PIXBUF_OPEN_COLUMN,
	DIR_NAME_COLUMN,
	DIR_N_COLUMNS
};

static gint compare_func(gconstpointer a, gconstpointer b){
	return(strcmp(a,b));
}

static gint reverse_compare_func(gconstpointer a, gconstpointer b){
	gint r;

	r = strcmp(a,b);
	if(r < 0)
		return(1);
	else if(r > 0)
		return(-1);
	else
		return(0);
}


static gint button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkTreePath *path;

	LOG(LOG_DEBUG, "IN : button_press_event()");
	if ((event->type == GDK_BUTTON_PRESS) &&
	    (event->button == 1)){

		// invert characters on mouse
		if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(directory_view),
					      (gint)(event->x),
					      (gint)(event->y),
					      &path,
						 NULL, NULL, NULL) == FALSE){
			return(FALSE);
		}

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(directory_view));
		gtk_tree_selection_select_path(selection, path);

		if(event->state & GDK_CONTROL_MASK){
			gboolean active;
			gchar *dirname;
			GList *l;

			dirname = compose_full_path(path);

			gtk_tree_model_get_iter(GTK_TREE_MODEL(directory_store), &iter, path);
			gtk_tree_model_get(GTK_TREE_MODEL(directory_store), &iter, DIR_ACTIVE_COLUMN, &active, -1);
			if(active){
				gtk_tree_store_set(directory_store, 
						   &iter,
						   DIR_ACTIVE_COLUMN, FALSE,
						   -1);

				l = g_list_first(active_dir_list);
				while(l){
					if(strcmp(l->data, dirname) == 0){
						active_dir_list = g_list_remove(active_dir_list, l->data);
						g_free(l->data);
						g_free(dirname);
						break;
					}
					l = g_list_next(l);
				}
			} else {
				gtk_tree_store_set(directory_store, 
						   &iter,
						   DIR_ACTIVE_COLUMN, TRUE,
						   -1);

				active_dir_list = g_list_append(active_dir_list, dirname);

			}
		}
		
		return(TRUE);
	}

	if ((event->type == GDK_2BUTTON_PRESS) &&
	    (event->button == 1)){
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(directory_view));
		if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE) {
			return(TRUE);
		}

		path = gtk_tree_model_get_path(GTK_TREE_MODEL(directory_store),
					       &iter);

		if(gtk_tree_model_iter_has_child(GTK_TREE_MODEL(directory_store), &iter) == TRUE){
			if(gtk_tree_view_row_expanded(GTK_TREE_VIEW(directory_view), path)){
				gtk_tree_view_collapse_row(GTK_TREE_VIEW(directory_view), path);
			} else {
				gtk_tree_view_expand_row(GTK_TREE_VIEW(directory_view), path, FALSE);
			}
		} else {
			RESULT result;
			gchar *fullname;
			gchar *tmpp;

			tmpp = compose_full_path(path);
			fullname = generic_to_native(tmpp);
			if(g_file_test(fullname, G_FILE_TEST_IS_REGULAR) == TRUE){
				result.heading = NULL;
				result.word = NULL;
				result.type = RESULT_TYPE_GREP;
				result.data.grep.filename = tmpp;
				result.data.grep.page = 1;
				result.data.grep.line = 1;
				result.data.grep.offset = 0;

				open_file(&result);
			}
			
			g_free(fullname);
			g_free(tmpp);

		}
		gtk_tree_path_free(path);
		return(TRUE);
	}


	LOG(LOG_DEBUG, "OUT : button_press_event() = FALSE");
	return(FALSE);
}


static gchar *compose_full_path(GtkTreePath *path)
{
	gchar *dirname=NULL;
	gchar buff[512];
	GtkTreePath *tmp_path;
	GtkTreeIter parent;
	gchar *name;

	LOG(LOG_DEBUG, "IN : compose_full_path()");


	// Compose full path by going up the path.

	tmp_path = gtk_tree_path_copy(path);
	while(1)
	{
		gtk_tree_model_get_iter(GTK_TREE_MODEL(directory_store), &parent, tmp_path);
		gtk_tree_model_get(GTK_TREE_MODEL(directory_store), &parent, DIR_NAME_COLUMN, &name, -1);
		
		if((name[1] == ':') && (name[2] == '\\')){
			buff[0] = name[0];
			buff[1] = '\0';
		} else 
			strcpy(buff, name);

		if(dirname){
			if(buff[strlen(buff)-1] == '/') {
				strcat(buff, dirname);
			} else {
				strcat(buff, "/");
				strcat(buff, dirname);
			}
			g_free(dirname);
		}
		dirname = strdup(buff);
		
		g_free(name);
		gtk_tree_path_up(tmp_path);
		if(gtk_tree_path_get_depth(tmp_path) <= 0)
			break;
       }
	gtk_tree_path_free(tmp_path);

	sprintf(buff, "/%s", dirname);
	g_free(dirname);
	dirname = strdup(buff);

	LOG(LOG_DEBUG, "OUT : compose_full_path() = %s", dirname);

	return(dirname);
}

static void row_collapsed(GtkTreeView *treeview,
			GtkTreeIter *iter,
			GtkTreePath *path,
			gpointer user_data)
{
/*
	// Closed folder icon
	gtk_tree_store_set(directory_store, 
			   iter,
			   DIR_PIXBUF_COLUMN, pixbuf_folder_closed,
			   DIR_PIXBUF_CLOSED_COLUMN, pixbuf_folder_closed,
			   DIR_PIXBUF_OPEN_COLUMN, pixbuf_folder_open,
			   -1);
*/
}

static void row_expanded(GtkTreeView *treeview,
			GtkTreeIter *iter,
			GtkTreePath *path,
			gpointer user_data)
{
	gchar *dirname=NULL;
	gchar *tmpp;
	gchar *name;
	gchar *fullpath=NULL;
	GDir *dir;
	GtkTreeIter child;
	GtkTreeIter grand_child;
	GList *dir_list = NULL;
	GList *file_list = NULL;
	GList *l;
	gint count=0;
	

	LOG(LOG_DEBUG, "IN : row_expanded()");

	tmpp = compose_full_path(path);
	dirname = generic_to_native(tmpp);
	g_free(tmpp);

	if((dir = g_dir_open(dirname, 0, NULL)) == NULL){
		LOG(LOG_CRITICAL, "Failed to open directory %s", dirname);
		return;
	}

	// Create entry list in the directory.
	while((name = (gchar *)g_dir_read_name(dir)) != NULL){
		fullpath = g_strdup_printf("%s%s%s", dirname, DIR_DELIMITER, name);
		if(g_file_test(fullpath, G_FILE_TEST_IS_REGULAR) == TRUE){
			tmpp = iconv_convert(fs_codeset, "utf-8", name);
			if((tmpp == NULL) || (strlen(tmpp) == 0)){
				LOG(LOG_CRITICAL, "Failed to convert filename from %s", fs_codeset);
				continue;
			}
			file_list = g_list_append(file_list, tmpp);
		} else if((g_file_test(fullpath, G_FILE_TEST_IS_DIR) == TRUE)){
			tmpp = iconv_convert(fs_codeset, "utf-8", name);
			if((tmpp == NULL) || (strlen(tmpp) == 0)){
				LOG(LOG_CRITICAL, "Failed to convert filename from %s", fs_codeset);
				continue;
			}
			dir_list = g_list_append(dir_list, tmpp);
		}
	}
	g_dir_close(dir);
	g_free(dirname);

	// Since insertion is done by "prepend", reverse sort.
	g_list_sort(dir_list, reverse_compare_func);
	g_list_sort(file_list, reverse_compare_func);


	l = g_list_first(file_list);
	while(l){
		gchar *name;
		name = l->data;

		// Files and directories starting by dot will not be shown
		if((name[0] == '.') && bsuppress_hidden_files){
			g_free(l->data);
			l = g_list_next(l);
			continue;
		}

		gtk_tree_store_prepend(directory_store, &child, iter);
		gtk_tree_store_set(directory_store, 
				   &child,
				   DIR_NAME_COLUMN, l->data,
				   DIR_PIXBUF_COLUMN, pixbuf_file,
				   DIR_PIXBUF_CLOSED_COLUMN, pixbuf_folder_closed,
				   DIR_PIXBUF_OPEN_COLUMN, pixbuf_folder_open,
				   DIR_ACTIVATABLE_COLUMN, TRUE,
				   DIR_ACTIVE_COLUMN, FALSE,
				   -1);
		g_free(l->data);
		l = g_list_next(l);
		count ++;
	}

	l = g_list_first(dir_list);
	while(l){
		gchar *name;
		name = l->data;

		// Files and directories starting by dot will not be shown
		if((name[0] == '.') && bsuppress_hidden_files){
			g_free(l->data);
			l = g_list_next(l);
			continue;
		}
		gtk_tree_store_prepend(directory_store, &child, iter);
		gtk_tree_store_set(directory_store, 
				   &child,
				   DIR_NAME_COLUMN, l->data,
				   DIR_PIXBUF_COLUMN, pixbuf_folder_open,
				   DIR_PIXBUF_CLOSED_COLUMN, pixbuf_folder_closed,
				   DIR_PIXBUF_OPEN_COLUMN, pixbuf_folder_open,
				   DIR_ACTIVATABLE_COLUMN, TRUE,
				   DIR_ACTIVE_COLUMN, FALSE,
				   -1);

		// Put dummy in order to show triangle mark.
		gtk_tree_store_append(directory_store, &grand_child, &child);
		gtk_tree_store_set(directory_store, 
				   &grand_child,
				   DIR_NAME_COLUMN, "DUMMY",
				   DIR_ACTIVATABLE_COLUMN, TRUE,
				   DIR_ACTIVE_COLUMN, FALSE,
				   -1);

		g_free(l->data);
		l = g_list_next(l);
		count++;
	}

	g_list_free(dir_list);
	g_list_free(file_list);


	// Because insertion is done by "prepend", remove the rest
	while(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(directory_store),
					    &child,
					    iter,
					    count)){
		gtk_tree_store_remove(GTK_TREE_STORE(directory_store), 
				      &child);

	}

/*
	// Closed folder icon
	gtk_tree_store_set(directory_store, 
			   iter,
			   DIR_PIXBUF_COLUMN, pixbuf_folder_open,
			   -1);
		
*/
	LOG(LOG_DEBUG, "OUT : row_expanded()");

	return;
}

static void item_toggled (GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
	GtkTreeModel *model = (GtkTreeModel *)directory_store;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
	GtkTreeIter iter;
	gboolean toggle_item;

	gchar *dirname;
	GList *l;

	LOG(LOG_DEBUG, "IN : item_toggled()");

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, DIR_ACTIVE_COLUMN, &toggle_item, -1);

	toggle_item ^= 1;

	gtk_tree_store_set (GTK_TREE_STORE (directory_store), &iter, DIR_ACTIVE_COLUMN,
			    toggle_item, -1);

	dirname = compose_full_path(path);

	if(toggle_item)
		active_dir_list = g_list_append(active_dir_list, dirname);
	else {
		l = g_list_first(active_dir_list);
		while(l){
			if(strcmp(l->data, dirname) == 0){
				active_dir_list = g_list_remove(active_dir_list, l->data);
				g_free(l->data);
				g_free(dirname);
				break;
			}
			l = g_list_next(l);
		}
	}

	gtk_tree_path_free (path);

	save_dirlist();

	LOG(LOG_DEBUG, "OUT : item_toggled()");
}

static void show_directory()
{
	GtkTreeIter parent;
	GtkTreeIter child;

#ifdef __WIN32__
	gchar *p;
	gchar buff[128];
	char tmp[128];

	LOG(LOG_DEBUG, "IN : show_directory()");

	GetLogicalDriveStrings(sizeof(buff), buff);

	p = buff;
	while(*p != '\0')
	{
		if((tolower(p[0]) != 'a') && (tolower(p[0]) != 'b'))
		{
			g_snprintf(tmp, sizeof(tmp), "%c:\\", toupper(p[0]));
			gtk_tree_store_append(directory_store, &parent, NULL);
			gtk_tree_store_set(directory_store, 
					   &parent,
					   DIR_NAME_COLUMN, tmp,
					   DIR_PIXBUF_COLUMN, pixbuf_file,
					   DIR_PIXBUF_CLOSED_COLUMN, pixbuf_folder_closed,
					   DIR_PIXBUF_OPEN_COLUMN, pixbuf_folder_open,
					   DIR_ACTIVATABLE_COLUMN, TRUE,
					   DIR_ACTIVE_COLUMN, FALSE,
					   -1);
			// Put dummy in order to show triangle mark.
			gtk_tree_store_append(directory_store, &child, &parent);
			gtk_tree_store_set(directory_store, 
					   &child,
					   DIR_NAME_COLUMN, "DUMMY",
					   -1);
		}
		p += (strlen(p) + 1);
	}
#else
	GDir *dir;
	GList *dir_list=NULL;
	GList *file_list=NULL;
	GList *l;
	gchar fullpath[512];
	const gchar *name;

	LOG(LOG_DEBUG, "IN : show_directory()");

	if((dir = g_dir_open("/", 0, NULL)) == NULL){
		LOG(LOG_CRITICAL, "Failed to open directory /");
		return;
	}

	while((name = g_dir_read_name(dir)) != NULL){
		sprintf(fullpath,"/%s",name);
		if(g_file_test(fullpath, G_FILE_TEST_IS_REGULAR) == TRUE){
			file_list = g_list_append(file_list, fs_to_unicode((gchar *)name));

		} else if(g_file_test(fullpath, G_FILE_TEST_IS_DIR) == TRUE){
			dir_list = g_list_append(dir_list, fs_to_unicode((gchar *)name));				}
	}
	g_dir_close(dir);

	g_list_sort(dir_list, compare_func);
	g_list_sort(file_list, compare_func);

	l = g_list_first(dir_list);
	while(l){
		gtk_tree_store_append(directory_store, &parent, NULL);
		gtk_tree_store_set(directory_store, 
				   &parent,
				   DIR_NAME_COLUMN, l->data,
				   DIR_PIXBUF_COLUMN, pixbuf_folder_open,
				   DIR_PIXBUF_CLOSED_COLUMN, pixbuf_folder_closed,
				   DIR_PIXBUF_OPEN_COLUMN, pixbuf_folder_open,
				   DIR_ACTIVATABLE_COLUMN, TRUE,
				   DIR_ACTIVE_COLUMN, FALSE,
				   -1);

		// Put dummy in order to show triangle mark.
		gtk_tree_store_append(directory_store, &child, &parent);
		gtk_tree_store_set(directory_store, 
				   &child,
				   DIR_NAME_COLUMN, "DUMMY",
				   -1);
		g_free(l->data);
		l = g_list_next(l);
	}

	l = g_list_first(file_list);
	while(l){
		gtk_tree_store_append(directory_store, &parent, NULL);
		gtk_tree_store_set(directory_store, 
				   &parent,
				   DIR_NAME_COLUMN, l->data,
				   DIR_PIXBUF_COLUMN, pixbuf_file,
				   DIR_PIXBUF_CLOSED_COLUMN, pixbuf_folder_closed,
				   DIR_PIXBUF_OPEN_COLUMN, pixbuf_folder_open,
				   DIR_ACTIVATABLE_COLUMN, TRUE,
				   DIR_ACTIVE_COLUMN, FALSE,
				   -1);
		g_free(l->data);
		l = g_list_next(l);
	}


	g_list_free(dir_list);
	g_list_free(file_list);
#endif

	LOG(LOG_DEBUG, "OUT : show_directory()");
}

static gboolean expand_to_path(gchar *name)
{
	gchar *p;
	gint i=0;
	GtkTreeIter iter;
	GtkTreeIter *parent=NULL;
	GtkTreePath *path;

	LOG(LOG_DEBUG, "IN : expand_to_path(%s)", name);

	// Expand to "name", then check "name"
	p = name;
	for(i=0; ; i++){
		gchar *tmp=NULL;

		if((name[i] != '/') && (name[i] != '\0')){
			continue;
		}

		if(&name[i] == p){
#ifdef __WIN32__
			tmp = malloc(4);
			tmp[0] = name[i+1];
			tmp[1] = ':';
			tmp[2] = '\\';
			tmp[3] = '\0';
			i+=2;
			p += 3;
#else			
			// UNIX root
			p ++;
			continue;
#endif
		} else if ((i > 1) && (name[i-1] == ':')){
			// Windows drive root
			tmp = g_strndup(p, &name[i+1] - p);
			p += 3;
		} else {
			tmp = g_strndup(p, &name[i] - p);
			p += &name[i] - p + 1;
		}

		if(gtk_tree_model_iter_children(GTK_TREE_MODEL(directory_store), &iter, parent) == FALSE){
			LOG(LOG_DEBUG, "OUT : expand_to_path() : no children");
			return(FALSE);
		}

		while(1){
			gchar *name;

			gtk_tree_model_get(GTK_TREE_MODEL(directory_store), &iter, DIR_NAME_COLUMN, &name, -1);
			if(strcmp(name, tmp) == 0){
				if(parent) {
					gtk_tree_iter_free(parent);
				}

				path = gtk_tree_model_get_path(GTK_TREE_MODEL(directory_store), &iter);
				if(gtk_tree_view_row_expanded(GTK_TREE_VIEW(directory_view), path) == FALSE)
					gtk_tree_view_expand_row(GTK_TREE_VIEW(directory_view), path, FALSE);
				gtk_tree_path_free(path);

				parent = gtk_tree_iter_copy(&iter);
				g_free(name);
				break;
			}
			if(gtk_tree_model_iter_next(GTK_TREE_MODEL(directory_store), &iter) == FALSE){
				LOG(LOG_DEBUG, "OUT : expand_to_path() : no such child");
				g_free(name);
				g_free(tmp);
				return(FALSE);
			}
		}

		g_free(tmp);

		if(name[i] == '\0')
			break;
	}

	gtk_tree_store_set(GTK_TREE_STORE(directory_store), 
			   parent,
			   DIR_ACTIVE_COLUMN, TRUE,
			   -1);
/*
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(directory_store), parent);
	gtk_tree_view_collapse_row(GTK_TREE_VIEW(directory_view), path);
	gtk_tree_path_free(path);
*/

	gtk_tree_iter_free(parent);

	LOG(LOG_DEBUG, "OUT : expand_to_path()");

	return(TRUE);
}

static void expand_history()
{
	GList *l;
	GList *remove_list = NULL;

	LOG(LOG_DEBUG, "IN : expand_history()");

	l = g_list_first(active_dir_list);
	while(l){
		if(expand_to_path(l->data) == FALSE)
			remove_list = g_list_append(remove_list, l->data);
		l = g_list_next(l);
	}

	l = g_list_first(remove_list);
	while(l){
		active_dir_list = g_list_remove(active_dir_list, l->data);
		LOG(LOG_DEBUG, "%s removed from history", l->data);
		g_free(l->data);
		l = g_list_next(l);
	}

	if(remove_list != NULL){
		save_dirlist();
	}
	
	g_list_free(remove_list);

	LOG(LOG_DEBUG, "OUT : expand_history()");
}

GtkWidget *create_directory_tree()
{

	GtkWidget *scroll;

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	LOG(LOG_DEBUG, "IN : create_directory_tree()");

	pixbuf_file = create_pixbuf(IMAGE_FILE);
	pixbuf_folder_closed = create_pixbuf(IMAGE_FOLDER_CLOSED);
	pixbuf_folder_open = create_pixbuf(IMAGE_FOLDER_OPEN);



	directory_store = gtk_tree_store_new(DIR_N_COLUMNS,
					     G_TYPE_BOOLEAN,
					     G_TYPE_BOOLEAN,
					     G_TYPE_OBJECT,
					     G_TYPE_OBJECT,
					     G_TYPE_OBJECT,
					     G_TYPE_STRING);



	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	directory_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(directory_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(directory_view), FALSE);

	gtk_container_add (GTK_CONTAINER (scroll), directory_view);

	g_signal_connect(G_OBJECT(directory_view),"row_expanded",
			 G_CALLBACK(row_expanded), (gpointer)NULL);

	g_signal_connect(G_OBJECT(directory_view),"row_collapsed",
			 G_CALLBACK(row_collapsed), (gpointer)NULL);

	g_signal_connect(G_OBJECT(directory_view),"button_press_event",
			 G_CALLBACK(button_press_event), (gpointer)NULL);


	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect (renderer, "toggled", G_CALLBACK(item_toggled), (gpointer)NULL);

	// checkbox
#define TEST
#ifdef TEST

	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW (directory_view), column);

	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, 
					   "active", DIR_ACTIVE_COLUMN);
	gtk_tree_view_column_add_attribute(column, renderer, 
					   "activatable", DIR_ACTIVATABLE_COLUMN);



	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);

	gtk_tree_view_column_add_attribute(column, renderer, 
					   "pixbuf", DIR_PIXBUF_COLUMN);
	gtk_tree_view_column_add_attribute(column, renderer, 
					   "pixbuf-expander-closed", DIR_PIXBUF_CLOSED_COLUMN);
	gtk_tree_view_column_add_attribute(column, renderer, 
					   "pixbuf-expander-open", DIR_PIXBUF_OPEN_COLUMN);


	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);

	gtk_tree_view_column_add_attribute(column, renderer, 
					   "text", DIR_NAME_COLUMN);


#else

	column = gtk_tree_view_column_new_with_attributes(NULL,
							  renderer,
							  "active", DIR_ACTIVE_COLUMN,
							  "activatable", DIR_ACTIVATABLE_COLUMN,
							  NULL);
#endif

	gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), FALSE);

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (directory_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);



	// Show initial directory.
	show_directory();

	// Expand and check formerly checked directories.
	expand_history();


	LOG(LOG_DEBUG, "OUT : create_directory_tree()");

	return(scroll);

}


GList *get_active_dir_list()
{
	LOG(LOG_DEBUG, "IN : get_active_dir_list()");
	return(g_list_copy(active_dir_list));
}

gchar *get_selected_directory()
{
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	gchar *dirname;

	LOG(LOG_DEBUG, "IN : get_selected_directory()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(directory_view));
	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE) {
		LOG(LOG_DEBUG, "OUT : get_selected_directory() = NULL");
		return(NULL);
	}

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(directory_store),
				       &iter);

	dirname = compose_full_path(path);
	gtk_tree_path_free(path);

	LOG(LOG_DEBUG, "OUT : get_selected_directory()");
	return(dirname);
}


void refresh_directory_tree()
{

	LOG(LOG_DEBUG, "IN : reresh_directory_tree()");

	if(directory_store == NULL)
		return;

	gtk_tree_store_clear(GTK_TREE_STORE(directory_store));

	show_directory();
	expand_history();

	LOG(LOG_DEBUG, "OUT : reresh_directory_tree()");

}
