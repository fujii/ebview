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
#include "link.h"
#include "render.h"
#include "eb.h"
#include "history.h"
#include "popup.h"
#include "textview.h"
#include "jcode.h"
#include "pixmap.h"

GtkWidget *popup=NULL;
static GtkWidget *popup_scroll=NULL;
static GtkWidget *title_label=NULL;
static GtkWidget *image_pushpin=NULL;
GtkWidget *popup_view=NULL;

static const int title_height = 22;
static gboolean bbutton_down=FALSE;
static gboolean bpushpin_down=FALSE;
static gfloat previous_x;
static gfloat previous_y;
static gint prev_x;
static gint prev_y;

static gint align_x = 10;
static gint align_y = 10;

GList *current_in_result=NULL;
extern GtkWidget *main_view;
extern GtkTextBuffer *text_buffer;
extern GtkTextTagTable *tag_table;

//static CONTENT_AREA *popup_area=NULL;


static void update_result(RESULT *result);

gint close_popup(GtkWidget *widget, gpointer data)
{

	LOG(LOG_DEBUG, "IN : close_popup()");

	if(popup != NULL){
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(popup_view), NULL);
		gtk_widget_destroy(popup_view);
		gtk_widget_destroy(popup);
	
		popup = NULL;
		bpushpin_down = FALSE;
	}

	LOG(LOG_DEBUG, "OUT : close_popup()");
	return(TRUE);
}


static gint popup_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	RESULT *rp;
	GtkTextIter iter;
	guint offset;
	gint buffer_x, buffer_y;

	LOG(LOG_DEBUG, "IN : popup_button_press_event()");

	if(event->type == GDK_BUTTON_PRESS){
		if (event->button == 1){
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
	
			if(follow_link(offset) == TRUE){
				LOG(LOG_DEBUG, "OUT : popup_button_press_event() = TRUE");
				return(TRUE);
			} else {
				if(bpushpin_down == FALSE){
					gtk_widget_destroy(popup);
					popup = NULL;
				}
			}
		} else if ((event->button == 2) || (event->button == 3)){
			if(!current_in_result)
				return(TRUE);

			if (event->button == 2) {
				if(g_list_previous(current_in_result) == NULL){
					return(TRUE);
				}
				current_in_result = g_list_previous(current_in_result);
			} else {
				if(g_list_next(current_in_result) == NULL){
					return(TRUE);
				}
				current_in_result = g_list_next(current_in_result);
			}
			if(current_in_result == NULL)
				return(TRUE);
			rp = (RESULT *)(current_in_result->data);
			if(current_in_result){
				show_popup(rp);
			}
		}
	}

	LOG(LOG_DEBUG, "OUT : popup_button_press_event()");
	return(TRUE);
}

static gint title_click_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	RESULT *rp;
	GdkModifierType mask;
	static GdkWindow *root_win = NULL;

	LOG(LOG_DEBUG, "IN : title_click_event()");


	if(event->type == GDK_BUTTON_PRESS){
		if ((event->button == 2) || (event->button == 3)){
			return(FALSE);
		}

		if((strcmp(data, "<") == 0) || 
		   (strcmp(data, ">") == 0)){

			if(strcmp(data, "<") == 0){
				if(g_list_previous(current_in_result) == NULL){
					return(FALSE);
				}
				current_in_result = g_list_previous(current_in_result);
			} else {
				if(g_list_next(current_in_result) == NULL){
					return(FALSE);
				}
				current_in_result = g_list_next(current_in_result);
			}

			if(current_in_result == NULL)
				return(0);
			rp = (RESULT *)(current_in_result->data);
			if(current_in_result){
				update_result(rp);
			}

		} else if(strcmp(data, "X") == 0){
			gtk_widget_destroy(popup);
			popup = NULL;
		} else if(strcmp(data, "t") == 0){
			bbutton_down = TRUE;
			root_win = gdk_window_foreign_new (GDK_ROOT_WINDOW ());
			gdk_window_get_pointer (root_win, &prev_x, &prev_y, &mask);
			previous_x = event->x;
			previous_y = event->y;
		} else if(strcmp(data, "p") == 0){
			GdkPixbuf *pixbuf;
			bbutton_down = TRUE;

			if(bpushpin_down == FALSE){
				bpushpin_down = TRUE;

				pixbuf = create_pixbuf(IMAGE_PUSH_ON);
				gtk_image_set_from_pixbuf(GTK_IMAGE(image_pushpin), pixbuf);
				destroy_pixbuf(pixbuf);
			} else {
				bpushpin_down = FALSE;

				pixbuf = create_pixbuf(IMAGE_PUSH_OFF);
				gtk_image_set_from_pixbuf(GTK_IMAGE(image_pushpin), pixbuf);
				destroy_pixbuf(pixbuf);

				gtk_widget_destroy(popup);
				popup = NULL;

			}
		}

	} else if((event->button == 1) && (event->type == GDK_2BUTTON_PRESS)){
		// Double click
	}
	
	LOG(LOG_DEBUG, "OUT : title_click_event()");
	return(TRUE);
}

static gint title_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	bbutton_down = FALSE;
	return(FALSE);
}

gint title_motion_event(GtkWidget *widget, GdkEventMotion *event)
{
	gint mov_x, mov_y;
	gint win_x, win_y;

	gint xp, yp;
	GdkModifierType mask;
	static GdkWindow *root_win = NULL;

	//LOG(LOG_DEBUG, "IN : title_motion_event()");

	if((event->state & GDK_BUTTON1_MASK) &&
	   bbutton_down){

		root_win = gdk_window_foreign_new (GDK_ROOT_WINDOW ());
		gdk_window_get_pointer (root_win, &xp, &yp, &mask);

	        mov_x = xp - prev_x;
	        mov_y = yp - prev_y;

 	        gdk_window_get_root_origin(popup->window, &win_x, &win_y);

		gtk_window_move(GTK_WINDOW(popup),
				win_x + mov_x,
				win_y + mov_y);
		prev_x = xp;
		prev_y = yp;
	}

	//LOG(LOG_DEBUG, "OUT : title_motion_event()");
	return(FALSE);
}

static void move_popup_window() {
	GdkModifierType mask;
	gint pos_x, pos_y;
	gint pointer_x, pointer_y;
	gint root_x, root_y;
	gint window_width, window_height;
	GdkWindow *root_win = NULL;

#ifdef __WIN32__
	root_x = GetSystemMetrics(SM_CXSCREEN);
	root_y = GetSystemMetrics(SM_CYSCREEN);
#else
	root_win = gdk_window_foreign_new (GDK_ROOT_WINDOW ());
	gdk_window_get_size(root_win, &root_x, &root_y);
#endif

	window_width = popup_width;
	window_height = popup_height;

	gdk_window_get_pointer(root_win, &pointer_x, &pointer_y, &mask);
	pos_x = pointer_x + align_x;
	pos_y = pointer_y + align_y;

	if(pos_x + window_width > root_x){
		pos_x = root_x - window_width;
	}
	
	if(bshow_popup_title) {
		if(pos_y + window_height + title_height > root_y){
			pos_y = root_y - window_height - title_height;
		}
	} else {
		if(pos_y + window_height > root_y){
			pos_y = root_y - window_height;
		}
	}

	gtk_window_move(GTK_WINDOW(popup), pos_x, pos_y);
}

static void create_popup_window(){

	gint window_width, window_height;

	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *eventbox;
	GtkWidget *image;
	GtkWidget *frame;
	GtkWidget *separator;

#ifdef __WIN32__
	HWND hWnd;
	long nStyle;
#endif

	LOG(LOG_DEBUG, "IN : create_popup_window()");

	// If there already is an window, use that position.
	// Move if the window is out of the screen.
//	gdk_window_get_position(popup->window, &pos_x, &pos_y);
//	gtk_widget_destroy(popup);
//	redraw = TRUE;

	window_width = popup_width;
	window_height = popup_height;
/*
	popup = gtk_widget_new (GTK_TYPE_WINDOW,
				"type", GTK_WINDOW_TOPLEVEL,
				 "allow-shrink", TRUE,
				 "allow-grow", TRUE,
				 "default-width", window_width,
				 "default-height", window_height,
				NULL);
*/
	popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(popup), TRUE);
	gtk_window_set_default_size(GTK_WINDOW(popup), window_width, window_height);
	gtk_window_set_accept_focus(GTK_WINDOW(popup), FALSE);
	move_popup_window();
	gtk_window_set_wmclass(GTK_WINDOW(popup), "Popup", "EBView");
	g_signal_connect(G_OBJECT(popup), "delete_event",
			 G_CALLBACK(close_popup), NULL);


	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add (GTK_CONTAINER (popup), vbox);


	if(bshow_popup_title){

		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
		gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

		hbox = gtk_hbox_new(FALSE, 0);
		gtk_container_add( GTK_CONTAINER(frame), hbox);


		eventbox = gtk_event_box_new();
		gtk_box_pack_start(GTK_BOX(hbox), eventbox, FALSE, FALSE, 2);
		g_signal_connect(G_OBJECT(eventbox),"button_press_event",
				 G_CALLBACK(title_click_event), (gpointer)"p");
		image_pushpin = create_image(IMAGE_PUSH_OFF);
		gtk_container_add( GTK_CONTAINER(eventbox), image_pushpin);

		separator = gtk_vseparator_new();
		gtk_box_pack_start(GTK_BOX(hbox), separator, FALSE, FALSE, 0);


		eventbox = gtk_event_box_new();
		gtk_box_pack_start(GTK_BOX(hbox), eventbox, FALSE, FALSE, 2);
		g_signal_connect(G_OBJECT(eventbox),"button_press_event",
				 G_CALLBACK(title_click_event), (gpointer)"<");
		image = create_image(IMAGE_SMALL_LEFT);
		gtk_container_add( GTK_CONTAINER(eventbox), image);

		separator = gtk_vseparator_new();
		gtk_box_pack_start(GTK_BOX(hbox), separator, FALSE, FALSE, 0);

		eventbox = gtk_event_box_new();
		gtk_box_pack_start(GTK_BOX(hbox), eventbox, TRUE, TRUE, 2);
		g_signal_connect(G_OBJECT(eventbox),"button_press_event",
				 G_CALLBACK(title_click_event), (gpointer)"t");
		g_signal_connect(G_OBJECT(eventbox),"button_release_event",
				 G_CALLBACK(title_release_event), (gpointer)NULL);
		g_signal_connect(G_OBJECT(eventbox),"motion_notify_event",
				 G_CALLBACK(title_motion_event), (gpointer)NULL);

		title_label = gtk_label_new("x of x");
		gtk_container_add( GTK_CONTAINER(eventbox), title_label);

		separator = gtk_vseparator_new();
		gtk_box_pack_start(GTK_BOX(hbox), separator, FALSE, FALSE, 0);

		eventbox = gtk_event_box_new();
		gtk_box_pack_end(GTK_BOX(hbox), eventbox, FALSE, FALSE, 2);
		g_signal_connect(G_OBJECT(eventbox),"button_press_event",
				 G_CALLBACK(title_click_event), (gpointer)"X");

		image = create_image(IMAGE_SMALL_CLOSE);
		gtk_container_add( GTK_CONTAINER(eventbox), image);

		separator = gtk_vseparator_new();
		gtk_box_pack_end(GTK_BOX(hbox), separator, FALSE, FALSE, 0);

		eventbox = gtk_event_box_new();
		gtk_box_pack_end(GTK_BOX(hbox), eventbox, FALSE, FALSE, 2);
		g_signal_connect(G_OBJECT(eventbox),"button_press_event",
				 G_CALLBACK(title_click_event), (gpointer)">");

		image = create_image(IMAGE_SMALL_RIGHT);
		gtk_container_add( GTK_CONTAINER(eventbox), image);

	}

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	popup_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (popup_scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(frame), popup_scroll);

	popup_view = gtk_text_view_new_with_buffer(text_buffer);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(popup_view), FALSE);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(popup_view), 5);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(popup_view), 5);
	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(popup_view), 3);
	gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(popup_view), 3);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(popup_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(popup_view), GTK_WRAP_WORD);


	g_signal_connect(G_OBJECT(popup_view),"motion_notify_event",
			 G_CALLBACK(motion_notify_event), (gpointer)NULL);
	
	g_signal_connect(G_OBJECT(popup_view),"button_press_event",
			 G_CALLBACK(popup_button_press_event), (gpointer)NULL);

	gtk_container_add (GTK_CONTAINER (popup_scroll), popup_view);

#ifndef __WIN32__
	gtk_widget_realize(popup);
	gdk_window_set_decorations(popup->window, 0);
#endif

	gtk_widget_show_all(popup);

#ifdef __WIN32__
	hWnd = GDK_WINDOW_HWND(popup->window);
	nStyle = GetWindowLong(hWnd, GWL_STYLE );
	nStyle &= ~WS_CAPTION;
	SetWindowLong(hWnd, GWL_STYLE, nStyle );
	SetWindowPos(hWnd,
		     HWND_TOPMOST,
		     pos_x, pos_y,
		     window_width, window_height,
		     SWP_FRAMECHANGED);
#endif

	bbutton_down = FALSE;
	bpushpin_down = FALSE;

	LOG(LOG_DEBUG, "OUT : create_popup_window()");

}


static gint scroll_to_top()
{
	GtkTextIter iter;
	GtkTextMark *mark;

	LOG(LOG_DEBUG, "IN : scroll_to_top()");

	gtk_text_buffer_get_start_iter (text_buffer, &iter);

	mark =  gtk_text_buffer_create_mark(text_buffer,
					    "start",
					    &iter,
					    TRUE);

	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(popup_view),
				     mark,
				     0.0,
				     TRUE,
				     0.0, 0.0);

	gtk_text_buffer_delete_mark(text_buffer, mark);


	LOG(LOG_DEBUG, "OUT : scroll_to_top()");
	return(0);
}
void show_result_in_popup()
{
	current_in_result = search_result;
	show_popup(current_in_result->data);
}

static void update_result(RESULT *result)
{
	gchar *text=NULL;
	GtkTextIter iter;
	CANVAS canvas;
	DRAW_TEXT l_text;
	gint length;
	gchar *euc_str;

	g_assert(result->type == RESULT_TYPE_EB);

	LOG(LOG_DEBUG, "IN : update_result()");

	text = ebook_get_text(result->data.eb.book_info,
			      result->data.eb.pos_text.page, 
			      result->data.eb.pos_text.offset);
	if(text == NULL)
		return;

	// Prevent the window from growing.
	//if(text[strlen(text)-1] == '\n')
	//text[strlen(text)-1] = '\0';

	if(popup == NULL){
		create_popup_window();
	}

	// Program aborts if you enable this line.
	//gtk_text_view_set_buffer(GTK_TEXT_VIEW(main_view), NULL);

	clear_text_buffer();

	gtk_text_buffer_get_start_iter (text_buffer, &iter);

	length = strlen(text);
	if(text[length-1] == '\n'){
		text[length-1] = '\0';
		length --;
	}

	l_text.text = text;
	l_text.length = length;

	canvas.buffer = text_buffer;
	canvas.iter = &iter;
	canvas.indent = 0;

	if(result->word != NULL){
		euc_str = iconv_convert("utf-8", "euc-jp", result->word);
		draw_content(&canvas, &l_text, result->data.eb.book_info, NULL, euc_str);
		g_free(euc_str);
	} else {
		draw_content(&canvas, &l_text, result->data.eb.book_info, NULL, NULL);
	}

	gtk_text_buffer_get_start_iter (text_buffer, &iter);
	gtk_text_buffer_place_cursor(text_buffer, &iter);

	if(bshow_popup_title){
		gchar title[256];
		sprintf(title, "%d of %d", 
			g_list_index(search_result, current_in_result->data) + 1,
			g_list_length(search_result));

		gtk_label_set_text(GTK_LABEL(title_label), title);

	}

	gtk_adjustment_set_value(
		gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(popup_scroll)), 0);

	gtk_adjustment_set_value(
		gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(popup_scroll)), 0);

	g_free(text);
	set_current_result(result);

	gtk_timeout_add(10, scroll_to_top, NULL);

	LOG(LOG_DEBUG, "OUT : update_result()");
}

void show_popup(RESULT *result)
{
	LOG(LOG_DEBUG, "IN : show_popup()");

	update_result(result);
	if(bpushpin_down == FALSE){
		move_popup_window();
	}
	gdk_window_show(GTK_WIDGET(popup)->window);
	gdk_window_focus(GTK_WIDGET(popup)->window, gtk_get_current_event_time());

	LOG(LOG_DEBUG, "OUT : show_popup()");
}
