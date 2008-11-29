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
#include "pref_io.h"
#include "eb.h"
#include "multi.h"

extern GList *group_list;
extern GtkWidget *display_dictbar;
extern GtkWidget *note_bar;

static GtkWidget *dict_bar;
GtkWidget *dict_box;
GtkWidget *combo_group;

void show_dict_bar()
{

	LOG(LOG_DEBUG, "IN : show_dict_bar()");

	gtk_widget_show(note_bar);
	bshow_dict_bar = 1;
	save_preference();

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_dictbar),
				       bshow_dict_bar);

	LOG(LOG_DEBUG, "OUT : show_dict_bar()");
}

void hide_dict_bar()
{
	LOG(LOG_DEBUG, "IN : hide_dict_bar()");

	gtk_widget_hide(note_bar);
	bshow_dict_bar = 0;
	save_preference();

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(display_dictbar),
				       bshow_dict_bar);

	LOG(LOG_DEBUG, "OUT : hide_dict_bar()");
}

void toggle_dict_bar(){

	LOG(LOG_DEBUG, "IN : toggle_dict_bar()");

	if(bshow_dict_bar == 1)
		hide_dict_bar();
	else 
		show_dict_bar();

	LOG(LOG_DEBUG, "OUT : toggle_dict_bar()");
}

static void dict_toggled(GtkWidget *widget, gpointer data)
{
	gboolean active;
	gboolean button_active;
	int i;

	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;

	LOG(LOG_DEBUG, "IN : dict_toggled(data=%d)");

	button_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));


	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE){
		do { 
			gchar *title;

			gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
					   &parent_iter, 
					   DICT_TITLE_COLUMN, &title,
					   DICT_ACTIVE_COLUMN, &active,
					   -1);
			if(active == TRUE){
				i = 0;
				if(gtk_tree_model_iter_children(GTK_TREE_MODEL(dict_store), &child_iter, &parent_iter) == TRUE){
					do {
						if(i == (int)data) {
							gtk_tree_store_set(dict_store, &child_iter,
									   DICT_ACTIVE_COLUMN, button_active,
									   -1);
							goto END;
						}
						i ++;
					} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &child_iter) == TRUE);
				}
			}
			g_free(title);
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE);
	}

 END:
	if(ebook_search_method() == SEARCH_METHOD_MULTI)
		show_multi();

	save_dictgroup();
	LOG(LOG_DEBUG, "OUT : dict_toggled()");
}

static void add_dict_buttons(GtkWidget *bar)
{
	GtkWidget *toggle;
	GtkWidget *label;
	int idx;
	char name[64];
	char buff[256];

	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;

	LOG(LOG_DEBUG, "IN : add_dict_buttons()");

	idx = 0;

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE){
		do { 
			gchar *title;
			gchar *fg, *bg;
			gboolean active;
			BOOK_INFO *binfo;
			gchar *tip_string;

			gtk_tree_model_get (GTK_TREE_MODEL(dict_store), 
					    &parent_iter, 
					    DICT_ACTIVE_COLUMN, &active,
					    -1);
			if(active == TRUE) {
				if(idx != 0){
					LOG(LOG_CRITICAL, "multipe group active");
					return;
				}

				if(gtk_tree_model_iter_children(GTK_TREE_MODEL(dict_store), &child_iter, &parent_iter) == TRUE){
					do {
						gtk_tree_model_get(GTK_TREE_MODEL(dict_store),
								   &child_iter,
								   DICT_TITLE_COLUMN, &title,
								   DICT_ACTIVE_COLUMN, &active,
								   DICT_MEMBER_COLUMN, &binfo,
								   DICT_FGCOLOR_COLUMN, &fg,
								   DICT_BGCOLOR_COLUMN, &bg,
								   -1);

						if(binfo == NULL) {
							continue;
						}

						g_utf8_strncpy(name, title, dict_button_length);
						if(fg == NULL) {
							if(bg == NULL)
								sprintf(buff, "%s", name);
							else
								sprintf(buff, "<span background=\"%s\">%s</span>",  bg, name);
						} else {
							if(bg == NULL)
								sprintf(buff, "<span foreground=\"%s\">%s</span>", fg, name);
							else
								sprintf(buff, "<span foreground=\"%s\" background=\"%s\">%s</span>", fg, bg, name);
						}

						toggle = gtk_toggle_button_new();
						if(benable_button_color)						
							label = gtk_label_new(buff);
						else 
							label = gtk_label_new(name);
						gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

						gtk_container_add (GTK_CONTAINER (toggle), label);

						gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), active);
						g_signal_connect(G_OBJECT(toggle),"toggled",
								 G_CALLBACK(dict_toggled),
								 (gpointer)idx);
				
						gtk_box_pack_start(GTK_BOX(bar), toggle, FALSE, FALSE, 2);
						if(binfo->available == FALSE) {
							gtk_widget_set_sensitive(toggle, FALSE);
						}

						tip_string = g_strconcat(_("Push to enable this dictionary."), "\n(", title, ")", NULL);
						gtk_tooltips_set_tip(tooltip, toggle, tip_string, "Private");
						g_free(tip_string);
						g_free(title);

						idx ++;

					} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &child_iter) == TRUE);
				}
			}
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE);
	}

	LOG(LOG_DEBUG, "OUT : add_dict_buttons()");
}

static void update_dict_button()
{
	GList *children;
	GtkBoxChild *child;
	
	LOG(LOG_DEBUG, "IN : update_dict_button()");

	gtk_widget_hide(dict_box);

	// Re-creaet buttons

	children = GTK_BOX(dict_box)->children;
	while(children){
		child = children->data;
		children = children->next;
		if(GTK_IS_TOGGLE_BUTTON(child->widget))
			gtk_widget_destroy(child->widget);
	}

	add_dict_buttons(dict_box);
	gtk_widget_show_all(dict_box);

	LOG(LOG_DEBUG, "OUT : update_dict_button()");

}

static gint group_changed (GtkWidget *combo){
	const gchar *text;

	GtkTreeIter   iter;

	LOG(LOG_DEBUG, "IN : group_changed()");

	text = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_group)->entry));

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter) == TRUE){
		do { 
			gchar *title;
			gtk_tree_model_get (GTK_TREE_MODEL(dict_store), 
					    &iter, 
					    DICT_TITLE_COLUMN, &title,
					    -1);

			if(strcmp(title, text) == 0){
				gtk_tree_store_set (dict_store, &iter,
						    DICT_ACTIVE_COLUMN, TRUE,
						    -1);
			} else {
				gtk_tree_store_set (dict_store, &iter,
						    DICT_ACTIVE_COLUMN, FALSE,
						    -1);
			}


	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	}

	update_dict_button();

	save_dictgroup();

	if(ebook_search_method() == SEARCH_METHOD_MULTI)
		show_multi();


	LOG(LOG_DEBUG, "OUT : group_changed()");

	return(FALSE);
}


GtkWidget *create_dict_bar()
{
	GList *list=NULL;
	gchar *old_group=NULL;
	GList *children;
	GtkBoxChild *child;
	gboolean active_found;
	gboolean old_found;
	GtkTreeIter active_iter;
	GtkTreeIter old_iter;
	GtkTreeIter iter;
	gchar *title;


	LOG(LOG_DEBUG, "IN : create_dict_bar()");

	if(dict_bar){
	        old_group = strdup(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_group)->entry)));

	        children = GTK_BOX(dict_bar)->children;
		while(children){
		        child = children->data;
			children = children->next;
			gtk_widget_destroy(child->widget);
		}
	} else {
	        dict_bar = gtk_hbox_new(FALSE, 0);
	}
	
	combo_group = gtk_combo_new();
	gtk_widget_set_size_request(GTK_WIDGET(combo_group), 120, 10);
	gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo_group)->entry), FALSE);
	gtk_box_pack_start(GTK_BOX(dict_bar), combo_group, FALSE, FALSE, 0);

	gtk_container_set_border_width(GTK_CONTAINER(dict_bar), 1);

	gtk_tooltips_set_tip(tooltip, GTK_COMBO(combo_group)->entry, _("Select dictionary group."),"Private");


	active_found = FALSE;
	old_found = FALSE;

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter) == TRUE){
		do { 
			gchar *title;
			gboolean active;
			gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
					   &iter, 
					   DICT_TITLE_COLUMN, &title,
					   DICT_ACTIVE_COLUMN, &active,
					   -1);

			if(active == TRUE){
				active_found = TRUE;
				active_iter = iter;
			}
			if(old_group && (strcmp(title, old_group) == 0)){
				old_found = TRUE;
				old_iter = iter;
			}
			list = g_list_append(list, g_strdup(title));
			g_free(title);
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	}

	if(g_list_length(list) != 0)
	        gtk_combo_set_popdown_strings( GTK_COMBO(combo_group), list) ;

	if(active_found == TRUE){
		gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
				   &active_iter, 
				   DICT_TITLE_COLUMN, &title,
				   -1);
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_group)->entry), title);
		g_free(title);
	} else if (old_found == TRUE){
		gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
				   &old_iter, 
				   DICT_TITLE_COLUMN, &title,
				   -1);
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_group)->entry), title);
		g_free(title);

		gtk_tree_store_set(GTK_TREE_STORE(dict_store),
				   &old_iter,
				   DICT_ACTIVE_COLUMN, TRUE,
				   -1);
		
	} else {
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter) == TRUE){
		gtk_tree_model_get(GTK_TREE_MODEL(dict_store), 
				   &iter, 
				   DICT_TITLE_COLUMN, &title,
				   -1);
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_group)->entry), title);
		g_free(title);

		gtk_tree_store_set(GTK_TREE_STORE(dict_store), &iter,
				   DICT_ACTIVE_COLUMN, TRUE,
				   -1);
		}
	}
  

	g_signal_connect(G_OBJECT (GTK_COMBO(combo_group)->entry), "changed",
			 G_CALLBACK(group_changed),
			 NULL);


	// Re-create buttons
	dict_box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX (dict_bar), dict_box, FALSE, FALSE, 0);
	add_dict_buttons(dict_box);

	gtk_widget_show_all(dict_bar);

	LOG(LOG_DEBUG, "OUT : create_dict_bar()");

	return(dict_bar);
}

void update_dict_bar()
{
	
	LOG(LOG_DEBUG, "IN : update_dict_bar()");

	gtk_widget_hide(dict_bar);
	create_dict_bar();
	gtk_widget_show_all(dict_bar);

	LOG(LOG_DEBUG, "OUT : update_dict_bar()");

}


