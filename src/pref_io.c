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
#include "xml.h"
#include "eb.h"
#include "pref_shortcut.h"
#include "statusbar.h"
#include "splash.h"
#include "dirtree.h"
#include "misc.h"


extern GList *word_history;
extern GList *directory_history;
extern GList *active_dir_list;

#define PREF_TYPE_INTEGER  0
#define PREF_TYPE_BOOLEAN  1
#define PREF_TYPE_STRING   2
#define PREF_TYPE_FLOAT   3

struct _preferences {
	gchar *name;
	gint type;
	void *addr;
};

struct _preferences preferences[] = {
	{"log_level", PREF_TYPE_INTEGER, &ebview_log_level},
	{"max_search", PREF_TYPE_INTEGER, &max_search},
	{"max_heading", PREF_TYPE_INTEGER, &max_heading},
	{"max_remember_words", PREF_TYPE_INTEGER, &max_remember_words},
	{"dict_button_length", PREF_TYPE_INTEGER, &dict_button_length},
	{"auto_interval", PREF_TYPE_INTEGER, &auto_interval},
	{"auto_minchar", PREF_TYPE_INTEGER, &auto_minchar},
	{"auto_maxchar", PREF_TYPE_INTEGER, &auto_maxchar},
	{"show_menu_bar", PREF_TYPE_BOOLEAN, &bshow_menu_bar},
	{"show_status_bar", PREF_TYPE_BOOLEAN, &bshow_status_bar},
	{"show_dict_bar", PREF_TYPE_BOOLEAN, &bshow_dict_bar},
	{"show_tree_tab", PREF_TYPE_BOOLEAN, &bshow_tree_tab},
	{"beep_on_nohit", PREF_TYPE_BOOLEAN, &bbeep_on_nohit},
	{"ignore_locks", PREF_TYPE_BOOLEAN, &bignore_locks},
	{"ignore_case", PREF_TYPE_BOOLEAN, &bignore_case},
	{"suppress_hidden_files", PREF_TYPE_BOOLEAN, &bsuppress_hidden_files},
	{"show_popup_title", PREF_TYPE_BOOLEAN, &bshow_popup_title},
	{"ending_correction", PREF_TYPE_BOOLEAN, &bending_correction},
	{"ending_only_nohit", PREF_TYPE_BOOLEAN, &bending_only_nohit},
	//{"selection_mode", PREF_TYPE_INTEGER, &selection_mode},
	{"popup_width", PREF_TYPE_INTEGER, &popup_width},
	{"popup_height", PREF_TYPE_INTEGER, &popup_height},
	{"window_x", PREF_TYPE_INTEGER, &window_x},
	{"window_y", PREF_TYPE_INTEGER, &window_y},
	{"window_width", PREF_TYPE_INTEGER, &window_width},
	{"window_height", PREF_TYPE_INTEGER, &window_height},
	{"tree_width", PREF_TYPE_INTEGER, &tree_width},
	{"tree_height", PREF_TYPE_INTEGER, &tree_height},
	{"pane_direction", PREF_TYPE_INTEGER, &pane_direction},
	{"tab_position", PREF_TYPE_INTEGER, &tab_position},
	{"wave_template", PREF_TYPE_STRING, &wave_template},
	{"mpeg_template", PREF_TYPE_STRING, &mpeg_template},
	{"font_normal", PREF_TYPE_STRING, &fontset_normal},
	{"font_bold", PREF_TYPE_STRING, &fontset_bold},
	{"font_italic", PREF_TYPE_STRING, &fontset_italic},
	{"font_superscript", PREF_TYPE_STRING, &fontset_superscript},
	{"color_link", PREF_TYPE_STRING, &color_str[COLOR_LINK]},
	{"color_keyword", PREF_TYPE_STRING, &color_str[COLOR_KEYWORD]},
	{"color_sound", PREF_TYPE_STRING, &color_str[COLOR_SOUND]},
	{"color_movie", PREF_TYPE_STRING, &color_str[COLOR_MOVIE]},
	{"color_emphasis", PREF_TYPE_STRING, &color_str[COLOR_EMPHASIS]},
	{"color_reverse_bg", PREF_TYPE_STRING, &color_str[COLOR_REVERSE_BG]},
	{"browser_template", PREF_TYPE_STRING, &browser_template},
	{"open_template", PREF_TYPE_STRING, &open_template},
	{"line_space", PREF_TYPE_INTEGER, &line_space},
	{"smooth_scroll", PREF_TYPE_INTEGER, &bsmooth_scroll},
	{"scroll_step", PREF_TYPE_INTEGER, &scroll_step},
	{"scroll_time", PREF_TYPE_INTEGER, &scroll_time},
	{"scroll_margin", PREF_TYPE_INTEGER, &scroll_margin},
	{"sort_by_dictionary", PREF_TYPE_INTEGER, &bsort_by_dictionary},
	{"emphasize_keyword", PREF_TYPE_INTEGER, &bemphasize_keyword},
	{"show_image", PREF_TYPE_INTEGER, &bshow_image},
	{"show_splash", PREF_TYPE_INTEGER, &bshow_splash},
	{"show_filename", PREF_TYPE_INTEGER, &bshow_filename},
	{"heading_auto_calc", PREF_TYPE_INTEGER, &bheading_auto_calc},
	{"enable_button_color", PREF_TYPE_INTEGER, &benable_button_color},
	{"word_search_automatic", PREF_TYPE_INTEGER, &bword_search_automatic},
	{"additional_lines", PREF_TYPE_INTEGER, &additional_lines},
	{"additional_chars", PREF_TYPE_INTEGER, &additional_chars},
	{"cache_size", PREF_TYPE_INTEGER, &cache_size},
	{"max_bytes_to_guess", PREF_TYPE_INTEGER, &max_bytes_to_guess}
};

gboolean load_preference()
{
	gchar filename[512];

	xmlDoc *doc=NULL;
	xmlNode *root;
	xmlNode *xpreference;
	xmlNode *xnode;

	LOG(LOG_DEBUG, "IN : load_preference()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_PREFERENCE);

	if(find_file(filename) == FALSE) {
		LOG(LOG_INFO, _("Couldn't open preference. Will use default value."));
		goto FAILED;
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	root = doc->root;

	xpreference = xml_get_child(root);
	if(strcmp(xml_get_name(xpreference), "preference") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xnode = xml_get_child(xpreference);
	while(xnode){
		gint items;
		gint i;

		items = sizeof (preferences) / sizeof (preferences[0]);
		for(i=0 ; i<items ; i++){
			if(strcmp(xml_get_name(xnode), preferences[i].name) == 0){
				switch(preferences[i].type){
				case PREF_TYPE_INTEGER:
				case PREF_TYPE_BOOLEAN:
					*(int *)(preferences[i].addr) = atoi(xml_get_content(xnode));
					break;
				case PREF_TYPE_STRING:
					*(gchar **)(preferences[i].addr) = strdup(xml_get_content(xnode));
					break;
				case PREF_TYPE_FLOAT:
					sscanf(xml_get_content(xnode), "%f",(float *)(preferences[i].addr));
					break;
				default:
					break;
				}
				
			}
		}
		xnode = xml_get_next(xnode);
	}

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : load_preference() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_preference() = FALSE");
	return(FALSE);

}

gboolean save_preference()
{
	gchar filename[512];
	xmlDoc *doc;
	xmlNode *xpreference;
	gint items;
	gint i;
	gchar buff[512];

	LOG(LOG_DEBUG, "IN : save_preference()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_PREFERENCE);
	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xpreference = xml_add_child(doc->root, "preference", NULL);

	items = sizeof (preferences) / sizeof (preferences[0]);
	for(i=0 ; i<items ; i++){
		switch(preferences[i].type){
		case PREF_TYPE_INTEGER:
		case PREF_TYPE_BOOLEAN:
			sprintf(buff, "%d", *((int *)preferences[i].addr));
			xml_add_child(xpreference, preferences[i].name, buff);
			break;
		case PREF_TYPE_FLOAT:
			sprintf(buff, "%f", *((float *)preferences[i].addr));
			xml_add_child(xpreference, preferences[i].name, buff);
			break;
		case PREF_TYPE_STRING:
//			if((preferences[i].addr == NULL) 
			xml_add_child(xpreference, preferences[i].name, *((gchar **)preferences[i].addr));
			break;
		default:
			break;
		}
	}

	xml_save_file(filename, doc);

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : save_preference()");
	return(TRUE);
}

gboolean load_dictgroup()
{
	char filename[512];

	char *book_name;
	char *book_path;
	int subbook_no;
	char *appendix_path;
	int appendix_subbook_no;
	int active;
	char *fg, *bg;
	BOOK_INFO *binfo;

	xmlDoc *doc;
	xmlNode *xroot;
	xmlNode *xdictgroup;
	xmlNode *xgroup;
	xmlNode *xdict;
	xmlNode *xnode;

	gchar *fs_book_path;
	gchar *fs_appendix_path;

	
	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;

	LOG(LOG_DEBUG, "IN : load_dictgroup");


	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_DICTGROUP);

	if(find_file(filename) == FALSE){
		LOG(LOG_INFO, "%s not found. Dictionary group will be empty.", filename);
		return(TRUE);
	}


	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xroot = doc->root;

	xdictgroup = xml_get_child(xroot);
	if(strcmp(xml_get_name(xdictgroup), "dictgroup") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xgroup = xml_get_child(xdictgroup);
	while(xgroup){
		if(strcmp(xml_get_name(xgroup), "group") != 0){
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}

		gtk_tree_store_append(dict_store, &parent_iter, NULL);
		gtk_tree_store_set(dict_store, &parent_iter,
				   DICT_TYPE_COLUMN, 0,
				   DICT_TITLE_COLUMN, strdup(xml_get_attr(xgroup, "name")),
				   DICT_ACTIVE_COLUMN, atoi(xml_get_attr(xgroup, "active")),
				   DICT_EDITABLE_COLUMN, TRUE,
				   -1);
		xdict = xml_get_child(xgroup);
		while(xdict){

			if(strcmp(xml_get_name(xdict), "dict") != 0){
				LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
				goto FAILED;
			}

			xnode = xml_get_child(xdict);

			book_name = NULL;
			book_path = NULL;
			subbook_no = 0;
			active = 0;
			appendix_path = NULL;
			appendix_subbook_no = 0;
			bg = NULL;
			fg = NULL;

			while(xnode){
				gchar *name;
				name = xml_get_name(xnode);
				if(strcmp(name, "name") == 0){
					book_name = xml_get_content(xnode);
				} else if(strcmp(name, "path") == 0){
					book_path = xml_get_content(xnode);
				} else if(strcmp(name, "subbook") == 0){
					subbook_no = atoi(xml_get_content(xnode));
				} else if(strcmp(name, "appendix_path") == 0){
					appendix_path = xml_get_content(xnode);
					if(strlen(appendix_path) == 0)
						appendix_path = NULL;
				} else if(strcmp(name, "appendix_subbook") == 0){
					appendix_subbook_no = atoi(xml_get_content(xnode));
				} else if(strcmp(name, "active") == 0){
					active = atoi(xml_get_content(xnode));
				} else if(strcmp(name, "fg") == 0){
					fg = xml_get_content(xnode);
				} else if(strcmp(name, "bg") == 0){
					bg = xml_get_content(xnode);
				} else {
					LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
					goto FAILED;
				}
				xnode = xml_get_next(xnode);
			}

			if(!book_name || (strlen(book_name) == 0))
				book_name = NULL;

			if(!book_path || (strlen(book_path) == 0))
				book_path = NULL;

			if(!book_name && !book_path){
				xdict = xml_get_next(xdict);
				continue;
			}

			if(!fg || (strlen(fg) == 0))
				//fg = DEFAULT_DICT_FGCOLOR;
				fg = NULL;

			if(!bg || (strlen(bg) == 0))
//				bg = DEFAULT_DICT_BGCOLOR;
				bg = NULL;

			fflush(stdout);

			if(book_path)
				fs_book_path = unicode_to_fs(book_path);
			else
				fs_book_path = NULL;

			if(appendix_path)
				fs_appendix_path = unicode_to_fs(appendix_path);
			else
				fs_appendix_path = NULL;

			binfo = load_book(fs_book_path, subbook_no, 
					  fs_appendix_path, appendix_subbook_no, fg, bg);

			splash_message(fs_book_path);

			g_free(fs_book_path);
			g_free(fs_appendix_path);

			if(binfo != NULL){
				gtk_tree_store_append(dict_store, &child_iter, &parent_iter);
				gtk_tree_store_set (dict_store, &child_iter,
						    DICT_TYPE_COLUMN, 1,
						    DICT_TITLE_COLUMN, book_name,
						    DICT_PATH_COLUMN, book_path,
						    DICT_SUBBOOK_NO_COLUMN, subbook_no,
						    DICT_APPENDIX_PATH_COLUMN, appendix_path,
						    DICT_APPENDIX_SUBBOOK_NO_COLUMN, appendix_subbook_no,
						    DICT_ACTIVE_COLUMN, active,
						    DICT_MEMBER_COLUMN, binfo,
						    DICT_EDITABLE_COLUMN, FALSE,
						    DICT_FGCOLOR_COLUMN, fg,
						    DICT_BGCOLOR_COLUMN, bg,
						    -1);
			}

			xdict = xml_get_next(xdict);
		}
		xgroup = xml_get_next(xgroup);
	}

	xml_destroy_document(doc);

	check_search_method();

	LOG(LOG_DEBUG, "OUT : load_dictgroup() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	check_search_method();

	LOG(LOG_DEBUG, "OUT : load_dictgroup() = FALSE");
	return(FALSE);

}


gboolean save_dictgroup()
{
	char filename[512];

	xmlDoc *doc;
	xmlNode *xdictgroup;
	xmlNode *xgroup;
	xmlNode *xdict;

	gchar buff[512];

	gchar *title;
	gboolean active;
	BOOK_INFO *binfo;
	gchar *fg, *bg;

	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;

	LOG(LOG_DEBUG, "IN : save_dictgroup()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_DICTGROUP);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xdictgroup = xml_add_child(doc->root, "dictgroup", NULL);

	g_assert(xdictgroup != NULL);

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE){
		do { 
			gtk_tree_model_get (GTK_TREE_MODEL(dict_store), 
					    &parent_iter, 
					    DICT_TITLE_COLUMN, &title,
					    DICT_ACTIVE_COLUMN, &active,
					    -1);

			g_assert(title != NULL);
			g_assert(strlen(title) != 0);

			xgroup = xml_add_child(xdictgroup, "group", NULL);
			xml_set_attr(xgroup, "name", title);
			sprintf(buff, "%d", active);	
			xml_set_attr(xgroup, "active", buff);

			g_free(title);

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

				       g_assert(title != NULL);
				       g_assert(binfo != NULL);
				       g_assert(strlen(title) != 0);


				       xdict = xml_add_child(xgroup, "dict", NULL);
				       g_assert(xdict != NULL);

				       xml_add_child(xdict, "name", title);
				       xml_add_child(xdict, "path", binfo->book_path);
				       sprintf(buff, "%d", binfo->subbook_no);
				       xml_add_child(xdict, "subbook", buff);
				       xml_add_child(xdict, "appendix_path", binfo->appendix_path);
				       sprintf(buff, "%d", binfo->appendix_subbook_no);
				       xml_add_child(xdict, "appendix_subbook", buff);
				       sprintf(buff, "%d", active);
				       xml_add_child(xdict, "active", buff);
				       xml_add_child(xdict, "bg", bg);
				       xml_add_child(xdict, "fg", fg);

			       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &child_iter) == TRUE);
		       }
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &parent_iter) == TRUE);
	}

	xml_save_file(filename, doc);
	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : save_dictgroup()");
	return(TRUE);
}



gboolean load_stemming_en()
{
	gchar filename[512];

	xmlDoc  *doc;
	xmlNode *root;
	xmlNode *xending;
	xmlNode *xentry;
	xmlNode *xnode;

	gchar *inflected;
	gchar *normal;

	GtkTreeIter   iter;

	LOG(LOG_DEBUG, "IN : load_stemming_en()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_STEMMING_EN);

	if(find_file(filename) == FALSE) {
		return(FALSE);
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}


	root = doc->root;

	xending = xml_get_child(root);
	if(strcmp(xml_get_name(xending), "endinglist") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xentry = xml_get_child(xending);
	while(xentry){
		if(strcmp(xml_get_name(xentry), "entry") != 0){
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}

		xnode = xml_get_child(xentry);

		inflected = NULL;
		normal=NULL;

		while(xnode){
			gchar *tagname;

			tagname = xml_get_name(xnode);
			if(strcmp(tagname, "inflected") == 0){
				inflected = xml_get_content(xnode);
			} else if(strcmp(tagname, "normal") == 0){
				normal = xml_get_content(xnode);
			} else {
				LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
				goto FAILED;
			}
			xnode = xml_get_next(xnode);
		}

		if(!inflected && !normal){
			xentry = xml_get_next(xentry);
			continue;
		}

		if(strlen(inflected) == 0){
			xentry = xml_get_next(xentry);
			continue;
		}

		if(strlen(normal) == 0)
			normal = NULL;

		gtk_list_store_append(stemming_en_store, &iter);
		gtk_list_store_set(stemming_en_store, &iter,
				   STEMMING_PATTERN_COLUMN, inflected,
				   STEMMING_NORMAL_COLUMN, normal,
				   -1);

		xentry = xml_get_next(xentry);
	}

	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_stemming_en() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_stemming_en() = FALSE");
	return(FALSE);
}


gboolean load_stemming_ja()
{
	gchar filename[512];

	xmlDoc  *doc;
	xmlNode *root;
	xmlNode *xending;
	xmlNode *xentry;
	xmlNode *xnode;

	gchar *inflected;
	gchar *normal;

	GtkTreeIter   iter;

	LOG(LOG_DEBUG, "IN : load_stemming_ja()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_STEMMING_JA);

	if(find_file(filename) == FALSE) {
		return(FALSE);
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}


	root = doc->root;

	xending = xml_get_child(root);
	if(strcmp(xml_get_name(xending), "endinglist") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xentry = xml_get_child(xending);
	while(xentry){
		if(strcmp(xml_get_name(xentry), "entry") != 0){
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}

		xnode = xml_get_child(xentry);

		inflected = NULL;
		normal=NULL;

		while(xnode){
			gchar *tagname;
			tagname = xml_get_name(xnode);
			if(strcmp(tagname, "inflected") == 0){
				inflected = xml_get_content(xnode);
			} else if(strcmp(tagname, "normal") == 0){
				normal = xml_get_content(xnode);
			} else {
				LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
				goto FAILED;
			}
			xnode = xml_get_next(xnode);
		}

		if(!inflected && !normal){
			xentry = xml_get_next(xentry);
			continue;
		}

		if(strlen(inflected) == 0){
			xentry = xml_get_next(xentry);
			continue;
		}

		if(strlen(normal) == 0)
			normal = NULL;

		gtk_list_store_append(stemming_ja_store, &iter);
		gtk_list_store_set(stemming_ja_store, &iter,
			STEMMING_PATTERN_COLUMN, inflected,
			STEMMING_NORMAL_COLUMN, normal,
			-1);

		xentry = xml_get_next(xentry);
	}

	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_stemming_ja() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_stemming_ja() = FALSE");
	return(FALSE);
}



gboolean save_stemming_en()
{
	gchar filename[512];
	xmlDoc *doc;
	xmlNode *xending;
	xmlNode *xentry;


	gchar *pattern;
	gchar *normal;

	GtkTreeIter  iter;

	LOG(LOG_DEBUG, "IN : save_stemming_en()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_STEMMING_EN);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xending = xml_add_child(doc->root, "endinglist", NULL);


	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(stemming_en_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(stemming_en_store), 
					   &iter,
					   STEMMING_PATTERN_COLUMN, &pattern,
					   STEMMING_NORMAL_COLUMN, &normal,
					   -1);

			xentry = xml_add_child(xending, "entry", NULL);
			xml_add_child(xentry, "inflected", pattern);
			xml_add_child(xentry, "normal", normal);

			g_free (pattern);
			g_free (normal);

	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(stemming_en_store), &iter) == TRUE);
	}

	xml_save_file(filename, doc);

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : save_stemming_en()");

	return(TRUE);

}

gboolean save_stemming_ja()
{
	gchar filename[512];
	xmlDoc *doc;
	xmlNode *xending;
	xmlNode *xentry;


	gchar *pattern;
	gchar *normal;

	GtkTreeIter  iter;

	LOG(LOG_DEBUG, "IN : save_stemming_ja()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_STEMMING_JA);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xending = xml_add_child(doc->root, "endinglist", NULL);


	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(stemming_ja_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(stemming_ja_store), 
					   &iter,
					   STEMMING_PATTERN_COLUMN, &pattern,
					   STEMMING_NORMAL_COLUMN, &normal,
					   -1);

			xentry = xml_add_child(xending, "entry", NULL);
			xml_add_child(xentry, "inflected", pattern);
			xml_add_child(xentry, "normal", normal);

			g_free (pattern);
			g_free (normal);

	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(stemming_ja_store), &iter) == TRUE);
	}

	xml_save_file(filename, doc);

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : save_stemming_ja()");

	return(TRUE);

}

gboolean load_weblist()
{
	char filename[512];
	char *name=NULL;
	char *home=NULL;
	char *pre=NULL;
	char *post=NULL;
	char *glue=NULL;
	char *code=NULL;

	xmlDoc  *doc;
	xmlNode *root;
	xmlNode *xsearchengine;
	xmlNode *xgroup;
	xmlNode *xengine;
	xmlNode *xnode;

	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;

	LOG(LOG_DEBUG, "IN : load_weblist()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_WEBLIST);

	if(find_file(filename) == FALSE){
		return(FALSE);
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	root = doc->root;

	xsearchengine = xml_get_child(root);

	if(strcmp(xml_get_name(xsearchengine), "searchengine") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}
	
	xgroup = xml_get_child(xsearchengine);

	while(xgroup){
		if(strcmp(xml_get_name(xgroup), "group") != 0){
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}

		gtk_tree_store_append(web_store, &parent_iter, NULL);
		gtk_tree_store_set (web_store, &parent_iter,
				    WEB_TYPE_COLUMN, 0,
				    WEB_TITLE_COLUMN, strdup(xml_get_attr(xgroup, "name")),
				    -1);

		xengine = xml_get_child(xgroup);
		
		while(xengine){
			if(strcmp(xml_get_name(xengine), "engine") != 0){
				LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
				goto FAILED;
			}

			name = home = pre = post = glue = code = NULL;

			xnode = xml_get_child(xengine);
			while(xnode){
				gchar *tagname;
				tagname = xml_get_name(xnode);
				if(strcmp(tagname, "name") == 0){
					name = xml_get_content(xnode);
				} else if(strcmp(tagname, "home") == 0){
					home = xml_get_content(xnode);
				} else if(strcmp(tagname, "pre") == 0){
					pre = xml_get_content(xnode);
				} else if(strcmp(tagname, "post") == 0){
					post = xml_get_content(xnode);
				} else if(strcmp(tagname, "glue") == 0){
					glue = xml_get_content(xnode);
				} else if(strcmp(tagname, "charcode") == 0){
					code = xml_get_content(xnode);
				} else {
					LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
					goto FAILED;
				}
				xnode = xml_get_next(xnode);
			}


			if((!name) || (!pre)) {
				LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
				goto FAILED;
			}

			gtk_tree_store_append(web_store, &child_iter, &parent_iter);

			if(strlen(home) == 0)
				home = NULL;
			if(strlen(pre) == 0)
				pre = NULL;
			if(strlen(post) == 0)
				post = NULL;
			if(strlen(glue) == 0)
				glue = NULL;
			if(strlen(code) == 0)
				code = NULL;

			gtk_tree_store_set (web_store, &child_iter,
					    WEB_TYPE_COLUMN, 1,
					    WEB_TITLE_COLUMN, name,
					    WEB_HOME_COLUMN, home,
					    WEB_PRE_COLUMN, pre,
					    WEB_POST_COLUMN, post,
					    WEB_GLUE_COLUMN, glue,
					    WEB_CODE_COLUMN, code,
					    -1);

			xengine = xml_get_next(xengine);
		}
		xgroup = xml_get_next(xgroup);
	}
	
	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : load_weblist() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_weblist() = FALSE");
	return(FALSE);


}


gboolean save_weblist()
{
	char filename[512];

	xmlDoc  *doc;
	xmlNode *xsearchengine;
	xmlNode *xgroup;
	xmlNode *xengine;

	gint type;
	gchar *title=NULL;
	gchar *home=NULL;
	gchar *pre=NULL;
	gchar *post=NULL;
	gchar *glue=NULL;
	gchar *code=NULL;

	GtkTreeIter   parent_iter;
	GtkTreeIter   child_iter;

	LOG(LOG_DEBUG, "IN : save_weblist()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_WEBLIST);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xsearchengine = xml_add_child(doc->root, "searchengine", NULL);

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(web_store), &parent_iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(web_store), 
					   &parent_iter, 
					   WEB_TYPE_COLUMN, &type,
					   WEB_TITLE_COLUMN, &title,
					   -1);
       
			xgroup = xml_add_child(xsearchengine, "group", NULL);
			xml_set_attr(xgroup, "name", title);

			g_free (title);

		       if(gtk_tree_model_iter_children(GTK_TREE_MODEL(web_store), &child_iter, &parent_iter) == TRUE){
			       do {
				       title=NULL;
				       home=NULL;
				       pre=NULL;
				       post=NULL;
				       glue=NULL;
				       code=NULL;

				       gtk_tree_model_get(GTK_TREE_MODEL(web_store),
							  &child_iter,
							  WEB_TYPE_COLUMN, &type,
							  WEB_TITLE_COLUMN, &title,
							  WEB_HOME_COLUMN, &home,
							  WEB_PRE_COLUMN, &pre,
							  WEB_POST_COLUMN, &post,
							  WEB_GLUE_COLUMN, &glue,
							  WEB_CODE_COLUMN, &code,
							  -1);

				       xengine = xml_add_child(xgroup, "engine", NULL);

				       xml_add_child(xengine, "name", title);
				       xml_add_child(xengine, "home", home);
				       xml_add_child(xengine, "pre", pre);
				       xml_add_child(xengine, "post", post);
				       xml_add_child(xengine, "glue", glue);
				       xml_add_child(xengine, "charcode", code);

				       g_free (title);
				       g_free (home);
				       g_free (pre);
				       g_free (post);
				       g_free (glue);
				       g_free (code);

			       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(web_store), &child_iter) == TRUE);
		       }
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(web_store), &parent_iter) == TRUE);
	}

	xml_save_file(filename, doc);

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : save_weblist()");
	return(TRUE);
}

extern struct _shortcuts shortcuts;
extern struct _shortcut_command commands[];

gboolean load_shortcut()
{
	gchar filename[512];

	xmlDoc  *doc;
	xmlNode *root;
	xmlNode *xshortcutdef;
	xmlNode *xshortcut;
	xmlNode *xnode;

	gint state = 0;
	gint keyval = 0;
	gchar *command;

	GtkTreeIter   iter;

	gchar keystr[64];

	void (* func)();

	gint i;

	LOG(LOG_DEBUG, "IN : load_shortcut()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_SHORTCUT);

	if(find_file(filename) == FALSE){
		return(FALSE);
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	root = doc->root;

	xshortcutdef = xml_get_child(root);
	if(strcmp(xml_get_name(xshortcutdef), "shortcutdef") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xshortcut = xml_get_child(xshortcutdef);
	while(xshortcut){
		if(strcmp(xml_get_name(xshortcut), "shortcut") != 0){
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}

		xnode = xml_get_child(xshortcut);

		command = NULL;
		while(xnode){
			gchar *tagname;
			tagname = xml_get_name(xnode);
			if(strcmp(tagname, "modifier") == 0){
				state = strtol(xml_get_content(xnode), NULL, 16);
			} else if(strcmp(tagname, "value") == 0){
				keyval = strtol(xml_get_content(xnode), NULL, 16);
			} else if(strcmp(tagname, "command") == 0){
				command = xml_get_content(xnode);
			} else {
				LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
				goto FAILED;
			}
			xnode = xml_get_next(xnode);
		}

		if((keyval == 0) || (command == NULL)) {
			xshortcut = xml_get_next(xshortcut);
			continue;
		}

		func = NULL;
		for(i=0 ; ; i ++){
			if(commands[i].name == NULL)
				break;
			if(strcmp(commands[i].name, command) == 0){
				func = commands[i].func;
				break;
			}
		}

		if(func == NULL){
			xshortcut = xml_get_next(xshortcut);
			continue;
		}

		key_val_to_string(state, keyval, keystr);

		gtk_list_store_append(shortcut_store, &iter);
		gtk_list_store_set(shortcut_store, &iter,
				   SHORTCUT_STATE_COLUMN, state,
				   SHORTCUT_KEYVAL_COLUMN, keyval,
				   SHORTCUT_NAME_COLUMN, command,
				   SHORTCUT_DESCRIPTION_COLUMN, _(command),
				   SHORTCUT_KEYSTR_COLUMN, keystr,
				   SHORTCUT_COMMAND_COLUMN, &commands[i],
				   -1);
		xshortcut = xml_get_next(xshortcut);
	}

	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_shortcut() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_shortcut() = FALSE");
	return(FALSE);
}

gboolean save_shortcut()
{
	gchar filename[512];
	xmlDoc *doc;
	xmlNode *xshortcutdef;
	xmlNode *xshortcut;
	gchar buff[16];

	GtkTreeIter  iter;

	guint state;
	guint keyval;

	struct _shortcut_command *command;

	LOG(LOG_DEBUG, "IN : save_shortcut()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_SHORTCUT);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xshortcutdef = xml_add_child(doc->root, "shortcutdef", NULL);


	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(shortcut_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(shortcut_store), 
					   &iter,
					   SHORTCUT_STATE_COLUMN, &state,
					   SHORTCUT_KEYVAL_COLUMN, &keyval,
					   SHORTCUT_COMMAND_COLUMN, &command,
					   -1);

			xshortcut = xml_add_child(xshortcutdef, "shortcut", NULL);
			sprintf(buff, "0x%04x", state);
			xml_add_child(xshortcut, "modifier", buff);
			sprintf(buff, "0x%04x", keyval);
			xml_add_child(xshortcut, "value", buff);
			xml_add_child(xshortcut, "command", command->name);
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(shortcut_store), &iter) == TRUE);
	}

	xml_save_file(filename, doc);

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : save_shortcut()");

	return(TRUE);

}

gboolean save_history()
{
	gchar filename[512];
	xmlDoc *doc;
	xmlNode *xword;
	GList *l;

	LOG(LOG_DEBUG, "IN : save_history()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_HISTORY);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xword = xml_add_child(doc->root, "word", NULL);

	for(l=g_list_first(word_history) ; l != NULL ; l = g_list_next(l)){
		xml_add_child(xword, "entry", l->data);
	}

	xml_save_file(filename, doc);
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : save_history()");

	return(TRUE);

}

gboolean load_history()
{
	gchar filename[512];

	xmlDoc  *doc;
	xmlNode *root;
	xmlNode *xword;
	xmlNode *xnode;

	LOG(LOG_DEBUG, "IN : load_history()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_HISTORY);

	if(find_file(filename) == FALSE) {
		return(FALSE);
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	root = doc->root;

	xword = xml_get_child(root);
	if(strcmp(xml_get_name(xword), "word") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xnode = xml_get_child(xword);
	while(xnode){
		gchar *tagname;
		gchar *tmp;
		tagname = xml_get_name(xnode);
		if(strcmp(tagname, "entry") != 0){
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}
		tmp = xml_get_content(xnode);
		if(strlen(tmp) != 0)
			word_history = g_list_append(word_history, strdup(tmp));
		xnode = xml_get_next(xnode);
	}

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : load_history() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_history() = FALSE");
	return(FALSE);
}

gboolean save_dirlist()
{
	gchar filename[512];
	xmlDoc *doc;
	xmlNode *xdirectory;
	GList *l;

	LOG(LOG_DEBUG, "IN : save_dirlist()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_DIRLIST);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");


	xdirectory = xml_add_child(doc->root, "dirlist", NULL);
	for(l=g_list_first(active_dir_list) ; l != NULL ; l = g_list_next(l)){
		xml_add_child(xdirectory, "entry", l->data);
	}

	xml_save_file(filename, doc);
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : save_dirlist()");

	return(TRUE);

}

gboolean load_dirlist()
{
	gchar filename[512];

	xmlDoc  *doc;
	xmlNode *root;
	xmlNode *xdirectory;
	xmlNode *xnode;

	LOG(LOG_DEBUG, "IN : load_dirlist()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_DIRLIST);

	if(find_file(filename) == FALSE) {
		return(FALSE);
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	root = doc->root;

	xdirectory = xml_get_child(root);
	if(strcmp(xml_get_name(xdirectory), "dirlist") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xnode = xml_get_child(xdirectory);
	while(xnode){
		gchar *tagname;
		gchar *tmp;
		tagname = xml_get_name(xnode);
		if(strcmp(tagname, "entry") != 0) {
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}
		tmp = xml_get_content(xnode);
		active_dir_list = g_list_append(active_dir_list, strdup(tmp));
		xnode = xml_get_next(xnode);
	}

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : load_dirlist() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_dirlist() = FALSE");
	return(FALSE);
}

gboolean load_filter()
{
	gchar filename[512];

	xmlDoc  *doc;
	xmlNode *root;
	xmlNode *xfilterdef;
	xmlNode *xfilter;
	xmlNode *xnode;

	gchar *ext;
	gchar *filter_command;
	gchar *open_command;

	GtkTreeIter   iter;

	LOG(LOG_DEBUG, "IN : load_filter()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_FILTER);

	if(find_file(filename) == FALSE){
		return(FALSE);
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	root = doc->root;

	xfilterdef = xml_get_child(root);
	if(strcmp(xml_get_name(xfilterdef), "filterdef") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xfilter = xml_get_child(xfilterdef);
	while(xfilter){
		if(strcmp(xml_get_name(xfilter), "filter") != 0){
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}

		xnode = xml_get_child(xfilter);

		ext = NULL;
		filter_command = NULL;
		open_command = NULL;

		while(xnode){
			gchar *tagname;
			tagname = xml_get_name(xnode);
			if(strcmp(tagname, "extension") == 0){
				ext = xml_get_content(xnode);
			} else if(strcmp(tagname, "filter_command") == 0){
				filter_command = xml_get_content(xnode);
			} else if(strcmp(tagname, "open_command") == 0){
				open_command = xml_get_content(xnode);
			} else {
				LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
				goto FAILED;
			}
			xnode = xml_get_next(xnode);
		}

		if(strlen(ext) == 0)
			ext = NULL;

		if(strlen(filter_command) == 0)
			filter_command = NULL;

		if(strlen(open_command) == 0)
			open_command = NULL;

		if((filter_command == 0) && (open_command == NULL)) {
			xfilter = xml_get_next(xfilter);
			continue;
		}

		gtk_list_store_append(filter_store, &iter);
		gtk_list_store_set(filter_store, &iter,
				   FILTER_EXT_COLUMN, ext,
				   FILTER_FILTER_COMMAND_COLUMN, filter_command,
				   FILTER_OPEN_COMMAND_COLUMN, open_command,
				   FILTER_EDITABLE_COLUMN, TRUE,
				   -1);
		xfilter = xml_get_next(xfilter);
	}

	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_filter() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_filter() = FALSE");
	return(FALSE);
}


gboolean save_filter()
{
	gchar filename[512];
	xmlDoc *doc;
	xmlNode *xfilterdef;
	xmlNode *xfilter;

	gchar *ext;
	gchar *filter_command;
	gchar *open_command;

	GtkTreeIter  iter;


	LOG(LOG_DEBUG, "IN : save_filter()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_FILTER);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xfilterdef = xml_add_child(doc->root, "filterdef", NULL);

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(filter_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(filter_store), 
					   &iter,
					   FILTER_EXT_COLUMN, &ext,
					   FILTER_FILTER_COMMAND_COLUMN, &filter_command,
					   FILTER_OPEN_COMMAND_COLUMN, &open_command,
					   -1);

			xfilter = xml_add_child(xfilterdef, "filter", NULL);
			xml_add_child(xfilter, "extension", ext);
			xml_add_child(xfilter, "filter_command", filter_command);
			xml_add_child(xfilter, "open_command", open_command);
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(filter_store), &iter) == TRUE);
	}

	xml_save_file(filename, doc);

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : save_filter()");

	return(TRUE);

}


gboolean save_dirgroup()
{
	gchar filename[512];
	xmlDoc *doc;
	xmlNode *xdirgroup;
	xmlNode *xgroup;


	gchar *title;
	gchar *list;
	gboolean active;
	gchar buff[512];
	gchar *p, *pp;

	GtkTreeIter  iter;

	LOG(LOG_DEBUG, "IN : save_dirgroup()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_DIRGROUP);

	doc = xml_doc_new();
	doc->encoding = strdup("euc-jp");
	doc->version = strdup("1.0");

	xdirgroup = xml_add_child(doc->root, "dirgroup", NULL);


	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirgroup_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(dirgroup_store), 
					   &iter,
					   DIRGROUP_TITLE_COLUMN, &title,
					   DIRGROUP_LIST_COLUMN, &list,
					   DIRGROUP_ACTIVE_COLUMN, &active,
					   -1);

			if(strcmp(title, _("Manual Select")) == 0){
				g_free (title);
				g_free (list);
				continue;
			}

			xgroup = xml_add_child(xdirgroup, "group", NULL);
			xml_set_attr(xgroup, "name", title);
			sprintf(buff, "%d", active);	
			xml_set_attr(xgroup, "active", buff);

			p = list;
			pp = NULL;
			while(1){
				pp = strchr(p, '\n');
				if(pp == NULL){
					xml_add_child(xgroup, "dir", p);
					break;
				} else {
					*pp = '\0';
					if(strlen(p) != 0)
						xml_add_child(xgroup, "dir", p);
					*pp = '\n';
					p = pp + 1;
				}
			}

			g_free (title);
			g_free (list);

	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dirgroup_store), &iter) == TRUE);
	}

	xml_save_file(filename, doc);

	xml_destroy_document(doc);

	LOG(LOG_DEBUG, "OUT : save_dirgroup()");

	return(TRUE);

}

gboolean load_dirgroup()
{
	gchar filename[512];

	xmlDoc  *doc;
	xmlNode *root;
	xmlNode *xdirgroup;
	xmlNode *xgroup;
	xmlNode *xnode;

	GtkTreeIter   iter;

	LOG(LOG_DEBUG, "IN : load_dirgroup()");

	sprintf(filename, "%s%s%s", user_dir, DIR_DELIMITER, FILENAME_DIRGROUP);

	if(find_file(filename) == FALSE){
		return(TRUE);
	}

	doc = xml_parse_file(filename);
	if(doc == NULL){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}


	root = doc->root;

	xdirgroup = xml_get_child(root);
	if(strcmp(xml_get_name(xdirgroup), "dirgroup") != 0){
		LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
		goto FAILED;
	}

	xgroup = xml_get_child(xdirgroup);
	while(xgroup){
		gchar buff[65535];

		if(strcmp(xml_get_name(xgroup), "group") != 0){
			LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
			goto FAILED;
		}

		xnode = xml_get_child(xgroup);

		buff[0] = '\0';

		while(xnode){
			gchar *dir;

			if(strcmp(xml_get_name(xnode), "dir") == 0){
				 dir = xml_get_content(xnode);
				 if(strlen(buff) != 0)
					 strcat(buff, "\n");
				 strcat(buff, dir);
			} else {
				LOG(LOG_ERROR, _("Failed to parse %s. Check contents."), filename);
				goto FAILED;
			}
			xnode = xml_get_next(xnode);
		}

		gtk_list_store_append(dirgroup_store, &iter);
		gtk_list_store_set(dirgroup_store, &iter,
				   DIRGROUP_TITLE_COLUMN, xml_get_attr(xgroup, "name"),
				   DIRGROUP_LIST_COLUMN, buff,
				   DIRGROUP_ACTIVE_COLUMN, atoi(xml_get_attr(xgroup, "active")),

				   -1);

		xgroup = xml_get_next(xgroup);
	}

	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_dirgroup() = TRUE");
	return(TRUE);

 FAILED:
	xml_destroy_document(doc);
	LOG(LOG_DEBUG, "OUT : load_dirgroup() = FALSE");
	return(FALSE);
}


