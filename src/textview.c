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
#include "dialog.h"
#include "dictbar.h"
#include "grep.h"
#include "headword.h"
#include "history.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "statusbar.h"
#include "link.h"
#include "jcode.h"
#include "misc.h"
#include "pref_io.h"
#include "textview.h"
#include "websearch.h"

GtkTextBuffer *text_buffer=NULL;
GtkWidget *main_view;
GtkWidget *dict_scroll=NULL;
static GtkTextTagTable *tag_table=NULL;
static void search_selection();

static GtkItemFactory *text_item_factory;

static GtkItemFactoryEntry text_menu_items[] = {
	{ N_("/Search This Word"),    NULL, search_selection, 0, NULL },
	{ N_("/Copy To Clipboard"),    NULL, copy_to_clipboard, 0, NULL },
	{ N_("/Display"),    NULL, NULL, 0, "<Branch>" },
	{ N_("/Display/Menu bar"),    NULL, show_menu_bar, 0, NULL },
	{ N_("/Display/Dictionary Selection Bar"),    NULL, show_dict_bar, 0, NULL },
	{ N_("/Display/Status Bar"),    NULL, show_status_bar, 0, NULL },
	{ N_("/Display/Tree Frame Tab"),    NULL, show_tree_tab, 0, NULL },
};

static void search_selection()
{
	GtkTextIter start;
	GtkTextIter end;
	gchar *text;
	gchar *euc_str;
	gint method;

	LOG(LOG_DEBUG, "IN : search_selection()");

	gtk_text_buffer_get_selection_bounds(text_buffer, &start, &end);
	text = gtk_text_buffer_get_text(text_buffer, &start, &end, FALSE);

	if(strlen(text) == 0)
		return;
	gtk_entry_set_text(GTK_ENTRY(word_entry), text);
	euc_str = iconv_convert("utf-8", "euc-jp", text);

	method = ebook_search_method();
	if(method == SEARCH_METHOD_INTERNET){
		web_search();
	} else 	if(method == SEARCH_METHOD_GREP){
		clear_message();
		grep_search(euc_str);
	} else {
		clear_message();
		ebook_search(euc_str, method);
		if(search_result == NULL)
			push_message(_("No hit."));
	}

	save_word_history(text);
	gtk_editable_select_region(GTK_EDITABLE(word_entry), 
				   0,
				   GTK_ENTRY(word_entry)->text_length);
//	show_result_tree();
	g_free(euc_str);

	LOG(LOG_DEBUG, "OUT : search_selection()");
}

void copy_to_clipboard()
{
	gtk_text_buffer_copy_clipboard(text_buffer, gtk_clipboard_get(NULL));
}

void create_text_buffer()
{
	gint i;

	LOG(LOG_DEBUG, "IN : create_text_buffer()");

	if(text_buffer != NULL){
		g_object_unref(G_OBJECT(text_buffer));
/*
		g_object_unref(G_OBJECT(text_buffer));
		g_object_unref(tag_keyword);
		g_object_unref(tag_bold);
		g_object_unref(tag_link);
		g_object_unref(tag_sound);
		g_object_unref(tag_movie);
		g_object_unref(tag_italic);
		g_object_unref(tag_subscript);
		g_object_unref(tag_superscript);
		g_object_unref(tag_gaiji);
		g_object_unref(tag_plain);
		g_object_unref(tag_colored);
		g_object_unref(tag_reverse);
		for(i=0; i < MAX_INDENT ; i ++){
			g_object_unref(tag_indent[i]);
		}
*/

	}


	tag_table = gtk_text_tag_table_new();
	text_buffer = gtk_text_buffer_new (tag_table);
/*
	g_object_set(text_buffer,
		     "tag-table", tag_table);
*/
	tag_keyword = gtk_text_tag_new("keyword");
	g_object_set(tag_keyword,
		     "weight", PANGO_WEIGHT_BOLD, 
		     "foreground", color_str[COLOR_KEYWORD],
		     "font", fontset_normal,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_keyword);

	tag_bold = gtk_text_tag_new("bold");
	g_object_set(tag_bold,
		     "weight", PANGO_WEIGHT_BOLD, 
		     "font", fontset_bold,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_bold);

	tag_link = gtk_text_tag_new("link");
	g_object_set(tag_link,
		     "foreground", color_str[COLOR_LINK],
		     "font", fontset_normal,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_link);

	tag_sound = gtk_text_tag_new("sound");
	g_object_set(tag_sound,
		     "foreground", color_str[COLOR_SOUND],
		     "font", fontset_normal,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_sound);

	tag_movie = gtk_text_tag_new("movie");
	g_object_set(tag_movie,
		     "foreground", color_str[COLOR_MOVIE],
		     "font", fontset_normal,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_movie);

	tag_italic = gtk_text_tag_new("italic");
	g_object_set(tag_italic,
		     "style", PANGO_STYLE_ITALIC,
		     "font", fontset_italic,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_italic);

	tag_superscript = gtk_text_tag_new("superscript");
	g_object_set(tag_superscript,
		     "rise", 5 * PANGO_SCALE,
		     "rise", 3,
		     "font", fontset_superscript,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_superscript);

	tag_subscript = gtk_text_tag_new("subscript");
	g_object_set(tag_subscript,
//		     "rise", -5 * PANGO_SCALE,
		     "rise", -3,
		     "font", fontset_superscript,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_subscript);

	tag_gaiji = gtk_text_tag_new("gaiji");
	g_object_set(tag_gaiji,
		     "rise", -2 * PANGO_SCALE,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_gaiji);

	tag_center = gtk_text_tag_new("center");
	g_object_set(tag_center,
		     "justification", GTK_JUSTIFY_CENTER,
		     "font", fontset_normal,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_center);

	tag_plain = gtk_text_tag_new("plain");
	g_object_set(tag_plain,
		     "wrap_mode", GTK_WRAP_WORD,
		     "font", fontset_normal,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_plain);

	tag_colored = gtk_text_tag_new("colored");
	g_object_set(tag_colored,
		     "foreground", color_str[COLOR_EMPHASIS],
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_colored);

	tag_reverse = gtk_text_tag_new("reverse");
	g_object_set(tag_reverse,
		     "background", color_str[COLOR_REVERSE_BG],
//		     "foreground", color_str[COLOR_EMPHASIS],
//		     "foreground", "#ff0000",
//		     "font", fontset_normal,
		     NULL);
	gtk_text_tag_table_add(tag_table, tag_reverse);

	for(i=0; i < MAX_INDENT ; i ++){
		gchar name[16];
		sprintf(name, "tag%d", i);
		tag_indent[i] = gtk_text_tag_new(name);
		g_object_set(tag_indent[i],
			     "left_margin", i * INDENT_LEFT_MARGIN + INITIAL_LEFT_MARGIN,
			     NULL);
		gtk_text_tag_table_add(tag_table, tag_indent[i]);
	}

/*
	tag_keyword = gtk_text_buffer_create_tag(text_buffer, "keyword", 
					 "weight", PANGO_WEIGHT_BOLD, 
					 "foreground", color_str[COLOR_KEYWORD],
					 "font", fontset_normal,
					 NULL);
	tag_bold = gtk_text_buffer_create_tag(text_buffer, "bold", 
					 "weight", PANGO_WEIGHT_BOLD, 
					 "font", fontset_bold,
					 NULL);
	tag_link = gtk_text_buffer_create_tag(text_buffer, "link", 
					 "foreground", color_str[COLOR_LINK],
					 "font", fontset_normal,
					 NULL);
	tag_sound = gtk_text_buffer_create_tag(text_buffer, "sound", 
					 "foreground", color_str[COLOR_SOUND],
					 "font", fontset_normal,
					 NULL);
	tag_movie = gtk_text_buffer_create_tag(text_buffer, "movie", 
					 "foreground", color_str[COLOR_MOVIE],
					 "font", fontset_normal,
					 NULL);
	tag_italic = gtk_text_buffer_create_tag(text_buffer, "italic",
					"style", PANGO_STYLE_ITALIC,
					"font", fontset_italic,
					NULL);
	tag_superscript = gtk_text_buffer_create_tag(text_buffer, "superscript",
//					     "rise", 5 * PANGO_SCALE,
					     "rise", 3,
					     "font", fontset_superscript,
					     NULL);
	tag_subscript = gtk_text_buffer_create_tag(text_buffer, "subscript",
//					   "rise", -5 * PANGO_SCALE,
					   "rise", -3,
					   "font", fontset_superscript,
					  NULL);
	tag_gaiji = gtk_text_buffer_create_tag(text_buffer, "gaiji",
					   "rise", -2 * PANGO_SCALE,
					  NULL);
	tag_center = gtk_text_buffer_create_tag(text_buffer, "center",
					  "justification", GTK_JUSTIFY_CENTER,
					 "font", fontset_normal,
					  NULL);
	tag_plain = gtk_text_buffer_create_tag(text_buffer, "plain",
					 "wrap_mode", GTK_WRAP_WORD,
					 "font", fontset_normal,
					  NULL);
	for(i=0; i < MAX_INDENT ; i ++){
		gchar name[16];
		sprintf(name, "tag%d", i);
		tag_indent[i] = gtk_text_buffer_create_tag(text_buffer, name,
					 "left_margin", i * INDENT_LEFT_MARGIN + INITIAL_LEFT_MARGIN,
					  NULL);
	}
	*/

	LOG(LOG_DEBUG, "OUT : create_text_buffer()");

}

gint motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	gint x, y;
	GdkModifierType mask;
	GtkTextIter iter;
	guint offset;
	gint buffer_x, buffer_y;
	GdkRectangle location;
	gboolean too_far=FALSE;
	
#ifdef __WIN32__
	HCURSOR hCursor;
#else
	GdkCursor *cursor;
#endif

//	LOG(LOG_DEBUG, "IN : motion_notify_event(x=%f,y=%f (%d %d))", event->x, event->y, buffer_x, buffer_y);

	// If you don't convert position as buffer origin, 
	// position will be invalid when scrolling
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget),
					      GTK_TEXT_WINDOW_TEXT,
					      (gint)(event->x),
					      (gint)(event->y),
					      &buffer_x,
					      &buffer_y);

	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget),
					   &iter,
					   buffer_x,
					   buffer_y);
	offset = gtk_text_iter_get_offset(&iter);

	gtk_text_view_get_iter_location(GTK_TEXT_VIEW(widget),
					&iter,
					&location);
	if((buffer_x > location.x + font_width) || (buffer_x < location.x - font_width))
		too_far = TRUE;
	else
		too_far = FALSE;
	
#ifdef __WIN32__
	if(scan_link(offset) && !too_far){
		hCursor = LoadCursor(NULL, IDC_HAND);
		// Because IDC_HAND can not be used in NT
		if(hCursor == 0)
			hCursor = LoadCursor(NULL, IDC_ARROW);
	} else {
		hCursor = LoadCursor(NULL, IDC_IBEAM);
	}
	SetCursor(hCursor);
#else
	if(scan_link(offset) && !too_far){
		cursor = gdk_cursor_new (CURSOR_LINK);
	} else {
		cursor = gdk_cursor_new(CURSOR_NORMAL);
	}
	gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), cursor);
	gdk_cursor_destroy (cursor);

	gdk_window_get_pointer(widget->window, &x, &y, &mask);
#endif

//	LOG(LOG_DEBUG, "OUT : motion_notify_event()");

	if(event->state & GDK_BUTTON1_MASK)
		return(FALSE);
	else 
		return(TRUE);
}

gint leave_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
#ifdef __WIN32__
	HCURSOR hCursor;
#else
	GdkCursor *cursor;
#endif

	LOG(LOG_DEBUG, "IN : leave_notify_event()");

#ifdef __WIN32__
	hCursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(hCursor);
#else
	cursor = gdk_cursor_new(CURSOR_NORMAL);
	gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), 
						       GTK_TEXT_WINDOW_TEXT),
			      cursor);
	gdk_cursor_destroy (cursor);
#endif
	LOG(LOG_DEBUG, "OUT : leave_notify_event()");
	return(FALSE);
}

gint button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	GtkTextIter iter;
	guint offset;
	gint buffer_x, buffer_y;
	GdkRectangle location;
	gboolean too_far=FALSE;

	
	LOG(LOG_DEBUG, "IN : button_press_event()");

	if((event->type == GDK_BUTTON_PRESS) &&
		(event->button == 1)){

		// If you don't convert position as buffer origin, 
		// position will be invalid when scrolling
		gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget),
						      GTK_TEXT_WINDOW_TEXT,
						      (gint)(event->x),
						      (gint)(event->y),
						      &buffer_x,
						      &buffer_y);

		gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget),
						   &iter,
						   buffer_x,
						   buffer_y);

		offset = gtk_text_iter_get_offset(&iter);

		gtk_text_view_get_iter_location(GTK_TEXT_VIEW(widget),
					&iter,
					&location);
		if((buffer_x >= location.x + font_width) || (buffer_x <= location.x - font_width))
			too_far = TRUE;
		else
			too_far = FALSE;
	

		if(scan_link(offset) && !too_far){	
			if(follow_link(offset) == TRUE)
				return(TRUE);
		}

	} else 	if((event->type == GDK_BUTTON_PRESS) &&
		((event->button == 2) || (event->button == 3))){
		gtk_item_factory_popup(GTK_ITEM_FACTORY(text_item_factory), 
				       event->x_root, event->y_root, 
				       event->button, event->time);
		LOG(LOG_DEBUG, "OUT : button_press_event() = TRUE");
		return(TRUE);

	}

	//gdk_window_get_pointer(widget->window, &x, &y, &mask);
	LOG(LOG_DEBUG, "OUT : button_press_event() = FALSE");
	return(FALSE);
}

GtkWidget *create_main_view()
{
	gint nmenu_items;	
	gint i;

	LOG(LOG_DEBUG, "IN : create_main_view()");

	dict_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dict_scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);


	create_text_buffer();

	main_view = gtk_text_view_new_with_buffer(text_buffer);

	// You must continue to grab event.
	// Otherwise, event stops when the cursor is at the area with no character.
	g_signal_connect(G_OBJECT(main_view),"motion_notify_event",
			 G_CALLBACK(motion_notify_event), (gpointer)NULL);
	
	g_signal_connect(G_OBJECT(main_view),"button_press_event",
			 G_CALLBACK(button_press_event), (gpointer)NULL);

	g_signal_connect(G_OBJECT(main_view),"leave_notify_event",
			 G_CALLBACK(leave_notify_event), (gpointer)NULL);

	
	gtk_text_view_set_editable(GTK_TEXT_VIEW(main_view), FALSE);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(main_view), 10);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(main_view), 10);
	gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(main_view), line_space);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(main_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(main_view), GTK_WRAP_WORD);
	gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(main_view), GTK_TEXT_WINDOW_LEFT, 1);
	gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(main_view), GTK_TEXT_WINDOW_RIGHT, 1);
	gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(main_view), GTK_TEXT_WINDOW_TOP, 1);
	gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(main_view), GTK_TEXT_WINDOW_BOTTOM, 1);

	if(line_space < 1)
		line_space = 3;

	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(main_view), line_space);


	gtk_container_add (GTK_CONTAINER (dict_scroll), main_view);

	nmenu_items = sizeof (text_menu_items) / sizeof (text_menu_items[0]);
	for(i=0 ; i<nmenu_items ; i++){
		text_menu_items[i].path = _(text_menu_items[i].path);
	}
	text_item_factory = gtk_item_factory_new (GTK_TYPE_MENU, "<text>", 
						  NULL);
	gtk_item_factory_create_items (text_item_factory, nmenu_items, 
				       text_menu_items, NULL);


	LOG(LOG_DEBUG, "OUT : create_main_view()");
	return(dict_scroll);
}

void scroll_mainview_down(){
	GtkTextIter iter;
	GdkRectangle rect;
	gint distance;
	gint i;

	LOG(LOG_DEBUG, "IN : scroll_mainview_down()");


	gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(main_view), &rect);

	distance = rect.height - scroll_margin;

	if(bsmooth_scroll == TRUE){
		for(i=0 ; i < scroll_step; i ++){
			gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(main_view),
							   &iter,
							   rect.x,
							   rect.y+ (distance / scroll_step)*(i+1));

			gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(main_view),
						     &iter,
						     0.0,
						     TRUE,
						     0.0, 0.0);
#ifdef __WIN32__
			Sleep(scroll_time / scroll_step / 1000);
#else
			usleep(scroll_time / scroll_step);
#endif
		}
	} else {
		gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(main_view),
						   &iter,
						   rect.x,
						   rect.y+ distance);

		gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(main_view),
					     &iter,
					     0.0,
					     TRUE,
					     0.0, 0.0);
	}

	LOG(LOG_DEBUG, "OUT : scroll_mainview_down()");

}
void scroll_mainview_up(){
	GtkTextIter iter;
	GdkRectangle rect;
	gint distance;
	gint i;

	LOG(LOG_DEBUG, "IN : scroll_mainview_up()");


	gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(main_view), &rect);

	distance = rect.height - scroll_margin;

	if(bsmooth_scroll == TRUE){
		for(i=0 ; i < scroll_step; i ++){
			gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(main_view),
							   &iter,
							   rect.x,
							   rect.y - (distance / scroll_step)*(i+1));

			gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(main_view),
						     &iter,
						     0.0,
						     TRUE,
						     0.0, 0.0);

#ifdef __WIN32__
			Sleep(scroll_time / scroll_step / 1000);
#else
			usleep(scroll_time / scroll_step);
#endif
		}
	} else {
		gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(main_view),
						   &iter,
						   rect.x,
						   rect.y - distance);

		gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(main_view),
					     &iter,
					     0.0,
					     TRUE,
					     0.0, 0.0);
	}

	LOG(LOG_DEBUG, "OUT : scroll_mainview_up()");

}

void clear_text_buffer()
{
	GtkTextIter start, end;

	LOG(LOG_DEBUG, "IN : clear_text_buffer()");

	gtk_text_buffer_get_bounds (text_buffer, &start, &end);
	gtk_text_buffer_delete(text_buffer, &start, &end);
	clear_link();

	LOG(LOG_DEBUG, "OUT : clear_text_buffer()");
}

void expand_lines()
{
	LOG(LOG_DEBUG, "IN : expand_lines()");

	line_space ++;

	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(main_view), line_space);
	gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(main_view), line_space);
	if(current_result != NULL){
		show_result(current_result, FALSE, TRUE);
	}

	save_preference();

	LOG(LOG_DEBUG, "OUT : expand_lines()");
}

void shrink_lines()
{
	LOG(LOG_DEBUG, "IN : shrink_lines()");

	line_space --;
	if(line_space < 0)
		line_space = 0;

	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(main_view), line_space);
	gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(main_view), line_space);
	if(current_result != NULL){
		show_result(current_result, FALSE, TRUE);
	}

	save_preference();

	LOG(LOG_DEBUG, "OUT : shrink_lines()");
}

static void font_resize(gchar **font, gint increment){
	gchar *old;
	gint size;
	gchar *p;
	gchar buff[8];

	LOG(LOG_DEBUG, "IN : font_resize(%s, %d)", *font, increment);

	g_assert(font != NULL);

	old = g_strdup(*font);

	remove_space(old);
	p = strrchr(old, ' ');
	if(p == NULL){
		LOG(LOG_INFO, "Invalid font format : %s", font);
		g_free(old);
		LOG(LOG_DEBUG, "OUT : font_resize()");
		return;
	}
	*p = '\0';
	p++;
	if(!isdigit(*p)){
		LOG(LOG_INFO, "Invalid font format : %s", font);
		LOG(LOG_DEBUG, "OUT : font_resize()");
		g_free(old);
		return;
	}
	size = (gint)strtol(p, NULL, 10);
	size += increment;

	// Smallest size is 1
	if(size < 1)
		size = 1;
	sprintf(buff, "%d", size);
	g_free(*font);
	*font = g_strconcat(old, " ", buff, NULL);
	g_free(old);
	
	LOG(LOG_DEBUG, "OUT : font_resize(%s)", *font);
}
void increase_font_size()
{
	LOG(LOG_DEBUG, "IN : increase_font_size()");

	font_resize(&fontset_normal, 1);
	font_resize(&fontset_bold, 1);
	font_resize(&fontset_superscript, 1);
	font_resize(&fontset_italic, 1);

	restart_main_window();

	save_preference();

	LOG(LOG_DEBUG, "OUT : increase_font_size()");
}

void decrease_font_size()
{
	LOG(LOG_DEBUG, "IN : dencrease_font_size()");

	font_resize(&fontset_normal, -1);
	font_resize(&fontset_bold, -1);
	font_resize(&fontset_superscript, -1);
	font_resize(&fontset_italic, -1);

	restart_main_window();

	save_preference();

	LOG(LOG_DEBUG, "OUT : dencrease_font_size()");
}
