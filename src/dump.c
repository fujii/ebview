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
#include "jcode.h"

static GtkWidget *hex_view = NULL;
static GtkWidget *text_view = NULL;
static GtkWidget *entry_hex_page;
static GtkWidget *entry_text_page;
static GtkWidget *entry_text_offset;
static GtkTextBuffer *text_buffer;
static GtkTextBuffer *hex_buffer;

GtkWidget *hex_dlg=NULL;
GtkWidget *text_dlg=NULL;

static void hex_close(GtkWidget *widget,gpointer *data){

	LOG(LOG_DEBUG, "IN : hex_close()");

	gtk_widget_destroy(hex_dlg);
	hex_dlg = NULL;

	LOG(LOG_DEBUG, "OUT : hex_close()");
}

static void text_close(GtkWidget *widget,gpointer *data){

	LOG(LOG_DEBUG, "IN : text_close()");

	gtk_widget_destroy(text_dlg);
	text_dlg = NULL;

	LOG(LOG_DEBUG, "OUT : text_close()");
}

static void hex_dump_page(GtkWidget *widget, gpointer *data){
	gint page;
	const gchar *p;
	gchar *p_hex=NULL;
	gchar *p_char=NULL;
	gchar *text;
	gchar hex_buff[512];
	gchar char_buff[512];
	gint i;
	GtkTextIter iter;
	GtkTextIter start, end;
	gchar *tmp_str;

	LOG(LOG_DEBUG, "IN : hex_dump_page()");
	
	p = gtk_entry_get_text(GTK_ENTRY(entry_hex_page));
	if(strlen(p)==0) {
		LOG(LOG_DEBUG, "OUT : hex_dump_page()");
		return;
	}

	page = strtol(p, NULL, 16);

	if(current_result == NULL){
		LOG(LOG_DEBUG, "OUT : hex_dump_page() : current_result == NULL");
		return;
	}

	text = ebook_get_rawtext(current_result->data.eb.book_info,
				 page,
				 0);
	if(text == NULL)
	{
		LOG(LOG_DEBUG, "OUT : hex_dump_page() : text == NULL");
		return;
	}


	gtk_text_buffer_get_bounds (hex_buffer, &start, &end);
	gtk_text_buffer_delete(hex_buffer, &start, &end);
	
	gtk_text_buffer_get_start_iter (hex_buffer, &iter);

	gtk_text_buffer_insert (hex_buffer, &iter,
				"Offset (Absolute)  00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F  0123456789ABCDEF\n", -1);



	for( i = 0 ;  i < EB_SIZE_PAGE ; i=i+2){

                // アドレスを表示
		if((i % 16) == 0){
			p_hex = hex_buff;
			p_char = char_buff;
			sprintf(p_hex, "0x%02x ", (i / 16));
			p_hex += 5;
			sprintf(p_hex, "(0x%08x)  ", (page - 1) * EB_SIZE_PAGE + i);
			p_hex += 14;

			sprintf(p_char, " ");
			p_char += 1;
		}

		sprintf(p_hex, "%02x ", (unsigned char)text[i]);
		p_hex += 3;
		sprintf(p_hex, "%02x ", (unsigned char)text[i+1]);
		p_hex += 3;

		if(isjisp(&text[i])) {
			*p_char = text[i] + 0x80;
			p_char ++;
			*p_char = text[i+1] + 0x80;
			p_char ++;
			*p_char = '\0';
		} else {
			sprintf(p_char, "..");
			p_char +=2;
		}

		if((i % 16) == 14){
			gtk_text_buffer_insert (hex_buffer, &iter,
						hex_buff, -1);

			tmp_str = iconv_convert("euc-jp", "utf-8", char_buff);
			gtk_text_buffer_insert (hex_buffer, &iter,
						tmp_str, -1);
			g_free(tmp_str);

			gtk_text_buffer_insert (hex_buffer, &iter,
						"\n", -1);
		}
	}

	free(text);

	LOG(LOG_DEBUG, "OUT : hex_dump_page()");
}

static void back_page(GtkWidget *widget,gpointer *data){
	gint page;
	const gchar *p;
	gchar buff[64];

	LOG(LOG_DEBUG, "IN : back_page()");

	p = gtk_entry_get_text(GTK_ENTRY(entry_hex_page));
	page = strtol(p, NULL, 16);
	if(page == 0){
		LOG(LOG_DEBUG, "OUT : back_page() : page == 0");
		return;
	}
	page --;

	sprintf(buff, "%08x",page);
	gtk_entry_set_text(GTK_ENTRY(entry_hex_page), buff);
	hex_dump_page(NULL, NULL);

	LOG(LOG_DEBUG, "OUT : back_page()");
}

static void forward_page(GtkWidget *widget,gpointer *data){
	gint page;
	const gchar *p;
	gchar buff[64];

	LOG(LOG_DEBUG, "IN : forward_page()");

	p = gtk_entry_get_text(GTK_ENTRY(entry_hex_page));
	page = strtol(p, NULL, 16);
	if(page == 0){
		LOG(LOG_DEBUG, "OUT : forward_page() : page == 0");
		return;
	}
	page ++;

	sprintf(buff, "%08x",page);
	gtk_entry_set_text(GTK_ENTRY(entry_hex_page), buff);
	hex_dump_page(NULL, NULL);

	LOG(LOG_DEBUG, "OUT : forward_page()");
}


void dump_hex(){

	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *hex_scroll;
	gchar buff[16];

	LOG(LOG_DEBUG, "IN : dump_hex()");

	if(hex_dlg == NULL){

		hex_dlg = gtk_dialog_new();
		gtk_window_set_title (GTK_WINDOW (hex_dlg), "Hex dump");

		g_signal_connect(G_OBJECT (hex_dlg), "delete_event",
				 G_CALLBACK(hex_close), NULL);

	
		button = gtk_button_new_with_label(_("Close"));
		GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (hex_dlg)->action_area), button,
				    TRUE, TRUE, 0);
		g_signal_connect(G_OBJECT (button), "clicked",
				 G_CALLBACK(hex_close), (gpointer)hex_dlg);
		
		hbox = gtk_hbox_new(FALSE,5);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
		gtk_box_pack_start (GTK_BOX(GTK_DIALOG(hex_dlg)->vbox)
				    , hbox, FALSE, FALSE, 0);
		
		label = gtk_label_new(_("page"));
		gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
		
		entry_hex_page = gtk_entry_new();
		gtk_widget_set_size_request(entry_hex_page,100,20);
		gtk_box_pack_start (GTK_BOX(hbox), entry_hex_page, FALSE, FALSE, 0);
		g_signal_connect(G_OBJECT(entry_hex_page), "activate",
				 G_CALLBACK(hex_dump_page), (gpointer)NULL);

		button = gtk_button_new_with_label("  >>  ");
		GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
		gtk_box_pack_end(GTK_BOX (hbox), button,
				    FALSE, FALSE, 0);
		g_signal_connect(G_OBJECT (button), "clicked",
				 G_CALLBACK(forward_page), NULL);

		button = gtk_button_new_with_label("  <<  ");
		GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
		gtk_box_pack_end(GTK_BOX (hbox), button,
				    FALSE, FALSE, 0);
		g_signal_connect(G_OBJECT (button), "clicked",
				 G_CALLBACK(back_page), NULL);

		
		hbox = gtk_hbox_new(FALSE,5);
		gtk_box_pack_start (GTK_BOX(GTK_DIALOG(hex_dlg)->vbox)
				    , hbox,TRUE, TRUE, 0);

		hex_scroll = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hex_scroll),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
		gtk_widget_set_size_request(hex_scroll, 800, 400);
		gtk_box_pack_start(GTK_BOX(hbox),
				   hex_scroll,FALSE, FALSE, 0);


		hex_buffer = gtk_text_buffer_new (NULL);
		hex_view = gtk_text_view_new_with_buffer(hex_buffer);

		gtk_text_view_set_editable(GTK_TEXT_VIEW(hex_view), FALSE);
		gtk_text_view_set_left_margin(GTK_TEXT_VIEW(hex_view), 10);
		gtk_text_view_set_right_margin(GTK_TEXT_VIEW(hex_view), 10);
		gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(hex_view), 3);
		gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(hex_view), 3);
		gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(hex_view), FALSE);
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(hex_view), GTK_WRAP_WORD);

		gtk_container_add (GTK_CONTAINER (hex_scroll), hex_view);


		gtk_widget_show_all(hex_dlg);
	}

	if((current_result != NULL) && (current_result->type == RESULT_TYPE_EB)){
		sprintf(buff, "%08x",current_result->data.eb.pos_text.page);
		gtk_entry_set_text(GTK_ENTRY(entry_hex_page), buff);
		hex_dump_page(NULL, NULL);
	}

	LOG(LOG_DEBUG, "OUT : dump_hex()");
}

static void text_dump_page(GtkWidget *widget,gpointer *data){

	gint page, offset;
	const gchar *p;
	gchar *text;
	gchar *utf_text;
	GtkTextIter iter;
	GtkTextIter start, end;

	LOG(LOG_DEBUG, "IN : text_dump_page()");

	p = gtk_entry_get_text(GTK_ENTRY(entry_text_page));
	if(strlen(p) == 0)
		return;
	page = strtol(p, NULL, 16);

	p = gtk_entry_get_text(GTK_ENTRY(entry_text_offset));
	if(strlen(p) == 0)
		return;
	offset = strtol(p, NULL, 16);

	if(current_result == NULL)
		return;

	text = ebook_get_text(current_result->data.eb.book_info,
			      page,
			      offset);
	if(text == NULL)
	{
		return;
	}

	gtk_text_buffer_get_bounds (text_buffer, &start, &end);
	gtk_text_buffer_delete(text_buffer, &start, &end);
	
	gtk_text_buffer_get_start_iter (text_buffer, &iter);
	utf_text = iconv_convert("euc-jp", "utf-8", text);
	gtk_text_buffer_insert (text_buffer, &iter,
				utf_text, -1);
	free(text);
	free(utf_text);

	LOG(LOG_DEBUG, "OUT : text_dump_page()");

}

void dump_text(){

	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *text_scroll;
	gchar buff[16];

	LOG(LOG_DEBUG, "IN : dump_text()");

	if(text_dlg == NULL){

		text_dlg = gtk_dialog_new();
		gtk_window_set_title (GTK_WINDOW (text_dlg), "Text dump");

		g_signal_connect(G_OBJECT (text_dlg), "delete_event",
				 G_CALLBACK(text_close), NULL);

		button = gtk_button_new_with_label(_("Close"));
		GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
		gtk_box_pack_start(GTK_BOX (GTK_DIALOG (text_dlg)->action_area), button,
				   TRUE, TRUE, 0);
		g_signal_connect(G_OBJECT (button), "clicked",
				 G_CALLBACK(text_close), (gpointer)text_dlg);

		hbox = gtk_hbox_new(FALSE,5);
		gtk_box_pack_start (GTK_BOX(GTK_DIALOG(text_dlg)->vbox)
				    , hbox, FALSE, FALSE, 0);
		
		label = gtk_label_new(_("page"));
		gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
		
		entry_text_page = gtk_entry_new();
		gtk_widget_set_size_request(entry_text_page,100,20);
		gtk_box_pack_start (GTK_BOX(hbox), entry_text_page, FALSE, FALSE, 0);
		g_signal_connect(G_OBJECT(entry_text_page), "activate",
				 G_CALLBACK(text_dump_page), (gpointer)NULL);
		
		label = gtk_label_new(_("offset"));
		gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
		
		entry_text_offset = gtk_entry_new();
		gtk_widget_set_size_request(entry_text_offset,100,20);
		gtk_box_pack_start (GTK_BOX(hbox), entry_text_offset, FALSE, FALSE, 0);
		g_signal_connect(G_OBJECT(entry_text_offset), "activate",
				 G_CALLBACK(text_dump_page), (gpointer)NULL);

		
		hbox = gtk_hbox_new(FALSE,5);
		gtk_box_pack_start (GTK_BOX(GTK_DIALOG(text_dlg)->vbox)
				    , hbox, FALSE, FALSE, 0);


		text_scroll = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (text_scroll),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
		gtk_widget_set_size_request(text_scroll, 600, 400);
		gtk_box_pack_start(GTK_BOX(hbox),
				   text_scroll,FALSE, FALSE, 0);

		text_buffer = gtk_text_buffer_new (NULL);
		text_view = gtk_text_view_new_with_buffer(text_buffer);

		gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
		gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 10);
		gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 10);
		gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(text_view), 3);
		gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(text_view), 3);
		gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
  

		gtk_widget_set_size_request(text_view, 500, 400);
		gtk_container_add (GTK_CONTAINER (text_scroll), text_view);

		gtk_widget_show_all(text_dlg);

	}

	if((current_result != NULL) && (current_result->type == RESULT_TYPE_EB)){
		sprintf(buff, "%08x",current_result->data.eb.pos_text.page);
		gtk_entry_set_text(GTK_ENTRY(entry_text_page), buff);
		sprintf(buff, "%08x",current_result->data.eb.pos_text.offset);
		gtk_entry_set_text(GTK_ENTRY(entry_text_offset), buff);
		text_dump_page(NULL, NULL);
	}

	LOG(LOG_DEBUG, "OUT : dump_text()");
}


void update_dump()
{
	if(hex_dlg != NULL)
		dump_hex();
	if(text_dlg != NULL)
		dump_text();
}
