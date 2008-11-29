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

#include "pixmap.h"

#include "../pixmaps/ebview.xpm"
#include "../pixmaps/book_open.xpm"
#include "../pixmaps/book_closed.xpm"
#include "../pixmaps/ebook.xpm"
#include "../pixmaps/cdrom.xpm"
#include "../pixmaps/left.xpm"
#include "../pixmaps/right.xpm"
#include "../pixmaps/up.xpm"
#include "../pixmaps/down.xpm"
#include "../pixmaps/globe.xpm"
#include "../pixmaps/html.xpm"
#include "../pixmaps/search.xpm"
#include "../pixmaps/item.xpm"
#include "../pixmaps/paste.xpm"
#include "../pixmaps/paste2.xpm"
//#include "../pixmaps/new.xpm"
#include "../pixmaps/popup.xpm"
#include "../pixmaps/popup2.xpm"
#include "../pixmaps/list.xpm"
#include "../pixmaps/multi.xpm"
#include "../pixmaps/small-left.xpm"
#include "../pixmaps/small-right.xpm"
#include "../pixmaps/small-close.xpm"
#include "../pixmaps/push-off.xpm"
#include "../pixmaps/push-on.xpm"
#include "../pixmaps/file.xpm"
#include "../pixmaps/folder_closed.xpm"
#include "../pixmaps/folder_open.xpm"

/*
void load_pixmaps()
{
	GdkColor transparent = { 0 };

	book_closed_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &book_closed_mask, 
			&transparent, book_closed_xpm);

	book_open_pixmap = gdk_pixmap_create_from_xpm_d (
		main_window->window, &book_open_mask, 
		&transparent, book_open_xpm);

	cdrom_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &cdrom_mask, 
			&transparent, cdrom_xpm);

	ebook_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &ebook_mask, 
			&transparent, ebook_xpm);

	left_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &left_mask, 
			&transparent, left_xpm);

	right_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &right_mask, 
			&transparent, right_xpm);

	up_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &up_mask,
 			&transparent, up_xpm);

	down_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &down_mask, 
			&transparent, down_xpm);

	globe_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &globe_mask, 
			&transparent, globe_xpm);

	search_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &search_mask, 
			&transparent, search_xpm);

	item_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &item_mask, 
			&transparent, item_xpm);

	paste_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &paste_mask, 
			&transparent, paste_xpm);

	popup_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &popup_mask, 
			&transparent, popup_xpm);

	html_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &html_mask, 
			&transparent, html_xpm);

	list_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &list_mask, 
			&transparent, list_xpm);

	multi_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &multi_mask, 
			&transparent, multi_xpm);

	small_left_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &small_left_mask, 
			&transparent, small_left_xpm);

	small_right_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &small_right_mask, 
			&transparent, small_right_xpm);

	small_close_pixmap = gdk_pixmap_create_from_xpm_d (
			main_window->window, &small_close_mask, 
			&transparent, small_close_xpm);

	ebook_pixbuf = gdk_pixbuf_new_from_xpm_data(ebook_xpm);
}
*/

/*
GtkWidget *create_pixmap_button(GdkPixmap *pixmap, GdkBitmap *mask){
	GtkWidget *button;
	GtkWidget *image;

	button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(button), 0);
	image = gtk_image_new_from_pixmap(pixmap, mask);
	gtk_container_add(GTK_CONTAINER(button), image);
	return(button);
}

GtkWidget *create_pixmap_toggle_button(GdkPixmap *pixmap, GdkBitmap *mask){
	GtkWidget *button;
	GtkWidget *image;

	button = gtk_toggle_button_new();
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(button), 0);
	image = gtk_image_new_from_pixmap(pixmap, mask);
	gtk_container_add(GTK_CONTAINER(button), image);
	return(button);
}
*/

static  char **pixmaps[] = {
	ebview_xpm,
	book_open_xpm, 
	book_closed_xpm, 
	cdrom_xpm,
	ebook_xpm,
	left_xpm,
	right_xpm,
	up_xpm,
	down_xpm,
	globe_xpm,
	search_xpm,
	item_xpm,
	paste_xpm,
	popup_xpm,
	html_xpm,
	list_xpm,
	multi_xpm,
	small_left_xpm,
	small_right_xpm,
	small_close_xpm,
	push_off,
	push_on,
	paste2_xpm,
	popup2_xpm,
	file_xpm,
	folder_closed_xpm,
	folder_open_xpm
};

GtkWidget *create_button_with_image(ImageNumber number){
	GtkWidget *button;
	GtkWidget *image;
	GdkPixbuf *pixbuf;

	button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(button), 0);
	pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)pixmaps[number]);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add(GTK_CONTAINER(button), image);
	gdk_pixbuf_unref(pixbuf);
	return(button);
}


GtkWidget *create_toggle_button_with_image(ImageNumber number){
	GtkWidget *button;
	GtkWidget *image;
	GdkPixbuf *pixbuf;

	button = gtk_toggle_button_new();
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(button), 0);
	pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)pixmaps[number]);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add(GTK_CONTAINER(button), image);
	gdk_pixbuf_unref(pixbuf);
	return(button);
}

GtkWidget *create_image(ImageNumber number){
	GtkWidget *image;
	GdkPixbuf *pixbuf;

	pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)pixmaps[number]);
	image = gtk_image_new_from_pixbuf(pixbuf);
	return(image);
}

GdkPixbuf *create_pixbuf(ImageNumber number){
	GdkPixbuf *pixbuf;

	pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)pixmaps[number]);
	return(pixbuf);
}

void destroy_pixbuf(GdkPixbuf *pixbuf){
	gdk_pixbuf_unref(pixbuf);
}
