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

#include <pthread.h>

#define SPLASH_WIDTH  250
#define SPLASH_HEIGHT 120

static gint tag_timeout;
static gboolean loading_dictgroup=0;

GtkWidget *splash=NULL;
GtkWidget *splash_label=NULL;

static void *load_thread(void *arg)
{
	load_dictgroup();

	loading_dictgroup=0;
	return(NULL);
}

void load_dictgroup_background()
{
	gint rc;
	pthread_attr_t thread_attr;
	pthread_t tid;

	LOG(LOG_DEBUG, "IN : load_dictgroup_background()");

 	pthread_attr_init (&thread_attr) ;
 	pthread_attr_setstacksize (&thread_attr, 512*1024) ;

	LOG(LOG_DEBUG, "thread_create");
	rc = pthread_create(&tid, &thread_attr, load_thread, (void *)NULL);
	if(rc != 0){
		LOG(LOG_ERROR, "pthread_create: %s", strerror(errno));
		exit(1);
	}
	LOG(LOG_DEBUG, "OUT : load_dictgroup_background()");
	pthread_attr_destroy(&thread_attr);

}

static gint load_watch_thread(gpointer data){
	if(loading_dictgroup){
//		LOG(LOG_DEBUG, "OUT : watch_thread() : CONTINUE");
		return(1);
	}
	gtk_timeout_remove(tag_timeout);

	gtk_widget_destroy(splash);

	gtk_main_quit();

	return(0);
}

void splash_message(gchar *msg)
{
	if((splash != NULL) && (splash_label != NULL))
		gtk_label_set_text(GTK_LABEL(splash_label), msg);
}

void show_splash()
{
	gint x, y;
	GdkWindow *root_win = NULL;
	gint root_x, root_y;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *frame;

#ifdef __WIN32__
	root_x = GetSystemMetrics(SM_CXSCREEN);
	root_y = GetSystemMetrics(SM_CYSCREEN);
#else
	root_win = gdk_window_foreign_new (GDK_ROOT_WINDOW ());
	gdk_window_get_size(root_win, &root_x, &root_y);
#endif

	x = (root_x - SPLASH_WIDTH) /2;
	y = (root_y - SPLASH_HEIGHT) /2;

	splash = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_set_size_request(splash, SPLASH_WIDTH, SPLASH_HEIGHT);
	gtk_window_move(GTK_WINDOW(splash), x, y);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (splash), frame);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);


	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<span size=\"x-large\" foreground=\"#a04040\" weight=\"ultrabold\">Welcome to EBView</span>");
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(label), TRUE, TRUE, 0);

	label = gtk_label_new(_("Loading dictionary..."));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(label), TRUE, TRUE, 0);

	splash_label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(splash_label), TRUE, TRUE, 0);

	gtk_widget_show_all(splash);

	loading_dictgroup = 1;

	tag_timeout = gtk_timeout_add(200, load_watch_thread, NULL);
	load_dictgroup_background();

	gtk_main();
}


