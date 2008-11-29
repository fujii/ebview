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

#include "eb.h"
#include "global.h"
#include "headword.h"
#include "history.h"

extern GList *group_list;

void show_menu()
{
	EB_Position pos;
	RESULT *rp;
	EB_Error_Code err;

	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;

	clear_search_result();

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE){
		do { 

			gboolean active;
			BOOK_INFO *binfo;

			gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
					   &parent_iter, 
					   DICT_ACTIVE_COLUMN, &active,
					   -1);

			if(active == TRUE){
				if(gtk_tree_model_iter_children(GTK_TREE_MODEL(dict_store), &child_iter, &parent_iter) == TRUE){
					do {
						gtk_tree_model_get(GTK_TREE_MODEL(dict_store),
								   &child_iter,
								   DICT_ACTIVE_COLUMN, &active,
								   DICT_MEMBER_COLUMN, &binfo,
								   -1);

						if(active != TRUE)
							continue;
						if(binfo->search_method[SEARCH_METHOD_MENU] != TRUE)
							continue;

						err = ebook_menu(binfo, &pos);
						if(err != EB_SUCCESS)
							continue;

						rp = (RESULT *)calloc(sizeof(RESULT), 1);
//						rp->heading = strdup(_("menu"));
						rp->heading = g_strdup_printf("%s : %s", _("menu"), binfo->subbook_title);

						rp->type = RESULT_TYPE_EB;
						rp->data.eb.book_info = binfo;
						rp->data.eb.pos_text = pos;
						search_result = g_list_append(search_result, rp);

						
					} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &child_iter) == TRUE);
				}
			}

	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE);
	}

	show_result_tree();
}

void show_copyright()
{
	EB_Position pos;
	RESULT *rp;
	EB_Error_Code err;

	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;


	clear_search_result();

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE){
		do { 
			gboolean active;
			BOOK_INFO *binfo;

			gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
					   &parent_iter, 
					   DICT_ACTIVE_COLUMN, &active,
					   -1);

			if(active == TRUE){
				if(gtk_tree_model_iter_children(GTK_TREE_MODEL(dict_store), &child_iter, &parent_iter) == TRUE){
					do {
						gtk_tree_model_get(GTK_TREE_MODEL(dict_store),
								   &child_iter,
								   DICT_ACTIVE_COLUMN, &active,
								   DICT_MEMBER_COLUMN, &binfo,
								   -1);

						if(active != TRUE)
							continue;
						if(binfo->search_method[SEARCH_METHOD_COPYRIGHT] != TRUE)
							continue;

						err = ebook_copyright(binfo, &pos);
						if(err != EB_SUCCESS)
							continue;

						rp = (RESULT *)calloc(sizeof(RESULT), 1);
//						rp->heading = strdup(_("copyright"));
						rp->heading = g_strdup_printf("%s : %s", _("copyright"), binfo->subbook_title);
						rp->type = RESULT_TYPE_EB;
						rp->data.eb.book_info = binfo;
						rp->data.eb.pos_text = pos;
						search_result = g_list_append(search_result, rp);

						
					} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &child_iter) == TRUE);
				}
			}

	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE);
	}

	show_result_tree();
}
