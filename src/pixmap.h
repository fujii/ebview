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

#ifndef __PIXMAP_H__
#define __MULTI_H__

#include "defs.h"
/*
void load_pixmaps();
GtkWidget *create_pixmap_button(GdkPixmap *pixmap, GdkBitmap *mask);
GtkWidget *create_pixmap_toggle_button(GdkPixmap *pixmap, GdkBitmap *mask);
*/
typedef enum {
	IMAGE_EBVIEW,
	IMAGE_BOOK_OPEN,
	IMAGE_BOOK_CLOSED,
	IMAGE_CDROM,
	IMAGE_EBOOK,
	IMAGE_LEFT,
	IMAGE_RIGHT,
	IMAGE_UP,
	IMAGE_DOWN,
	IMAGE_GLOBE,
	IMAGE_SEARCH,
	IMAGE_ITEM,
	IMAGE_SELECTION,
	IMAGE_POPUP,
	IMAGE_HTML,
	IMAGE_LIST,
	IMAGE_MULTI,
	IMAGE_SMALL_LEFT,
	IMAGE_SMALL_RIGHT,
	IMAGE_SMALL_CLOSE,
	IMAGE_PUSH_OFF,
	IMAGE_PUSH_ON,
	IMAGE_SELECTION2,
	IMAGE_POPUP2,
	IMAGE_FILE,
	IMAGE_FOLDER_CLOSED,
	IMAGE_FOLDER_OPEN
} ImageNumber;


GtkWidget *create_button_with_image(ImageNumber number);
GtkWidget *create_toggle_button_with_image(ImageNumber number);
GtkWidget *create_image(ImageNumber number);

GdkPixbuf *create_pixbuf(ImageNumber number);
void destroy_pixbuf(GdkPixbuf *pixbuf);

#endif /* __MULTI_H__ */
