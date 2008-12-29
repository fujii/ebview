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

#define SPLASH_WIDTH  250
#define SPLASH_HEIGHT 120

static GtkWidget *splash = NULL;
static GtkWidget *splash_label1 = NULL;
static GtkWidget *splash_label2 = NULL;

void
splash_update(const gchar *text1, const gchar *text2)
{
	if (!splash)
		return;
	gtk_label_set_text(GTK_LABEL(splash_label1), text1);
	gtk_label_set_text(GTK_LABEL(splash_label2), text2);
	gdk_threads_enter();
	while (gtk_events_pending())
		gtk_main_iteration();
	gdk_threads_leave();
}

void
splash_destroy(void)
{
	if (!splash)
		return;
	gtk_widget_destroy(splash);
	splash = NULL;
}

void
splash_create(void)
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *frame;

	splash = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_type_hint(GTK_WINDOW(splash),
				 GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
	gtk_widget_set_size_request(splash, SPLASH_WIDTH, SPLASH_HEIGHT);
	gtk_window_set_position(GTK_WINDOW(splash), GTK_WIN_POS_CENTER);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (splash), frame);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),
						 "<span size=\"x-large\" foreground=\"#a04040\" weight=\"ultrabold\">"
						 "Welcome to " PACKAGE_NAME
						 "</span>");
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(label), TRUE, TRUE, 0);

	splash_label1 = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(splash_label1), TRUE, TRUE, 0);

	splash_label2 = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(splash_label2), TRUE, TRUE, 0);

	gtk_widget_show_all(splash);
}
