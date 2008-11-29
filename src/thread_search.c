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
#include "headword.h"

#include <pthread.h>

#define WATCH_INTERVAL 500 // ms

static pthread_t tid;
static gint tag_timeout;
static gint thread_running = 0;
static gint hit_count=0;
static gfloat search_progress;
static pthread_mutex_t mutex;

static GtkWidget *progress;
static GtkWidget *cancel_dialog=NULL;
static GtkWidget *label_match;
static GtkWidget *label_cancel=NULL;



static void cancel_thread(GtkWidget *widget, gpointer *data)
{

	LOG(LOG_DEBUG, "IN : cancel_thread()");

	gtk_timeout_remove(tag_timeout);

	gtk_grab_remove(cancel_dialog);
	gtk_widget_destroy(cancel_dialog);

	pthread_cancel(tid);
	thread_running = 0;

	cancel_dialog=NULL;
	label_cancel=NULL;

	show_result_tree();

	push_message("\n");
	push_message(_("Canceled"));
	push_message("\n");
	if(search_result == NULL)
		push_message(_("No hit."));

	LOG(LOG_DEBUG, "OUT : cancel_thread()");
}

static void delete_event( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	LOG(LOG_DEBUG, "IN : delete_event()");

	cancel_thread(NULL, NULL);

	LOG(LOG_DEBUG, "OUT : delete_event()");
}

static void show_cancel_dialog(gchar *text)
{
	GtkWidget *button;

	LOG(LOG_DEBUG, "IN : show_cancel_dialog()");

	search_progress = 0.0;
	
//	cancel_dialog = gtk_dialog_new();
	cancel_dialog = gtk_dialog_new_with_buttons(text,
	                             GTK_WINDOW(main_window),
					GTK_DIALOG_DESTROY_WITH_PARENT /* | GTK_DIALOG_NO_SEPARATOR */,
                                     NULL);

	g_signal_connect (G_OBJECT (cancel_dialog), "delete_event",
			    G_CALLBACK(delete_event), NULL);

	gtk_widget_set_size_request(cancel_dialog, 200, -1);
	gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG(cancel_dialog)->vbox), 10);
	
	button = gtk_button_new_with_label(_("Cancel"));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (cancel_dialog)->action_area), button,
				    TRUE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK(cancel_thread), (gpointer)NULL);
	
	label_cancel = gtk_label_new (_("Searching"));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (cancel_dialog)->vbox), label_cancel, TRUE,TRUE, 5);

	progress = gtk_progress_bar_new();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 0);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (cancel_dialog)->vbox), progress, TRUE,TRUE, 5);

	label_match = gtk_label_new ("0 hit");
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (cancel_dialog)->vbox), label_match, TRUE,TRUE, 5);

//	gtk_window_set_position(GTK_WINDOW(cancel_dialog), GTK_WIN_POS_CENTER_ALWAYS);

	gtk_widget_show_all(cancel_dialog); 
	center_dialog(main_window, cancel_dialog);
	gtk_grab_add(cancel_dialog);

	LOG(LOG_DEBUG, "OUT : show_cancel_dialog()");

}

static gint watch_thread(gpointer data){

	char msg[256];

	//LOG(LOG_DEBUG, "IN : watch_thread()");

	if(thread_running){
		pthread_mutex_lock(&mutex);

		sprintf(msg, _("%d hit"), hit_count);
		if(GTK_IS_LABEL(label_match)){
			gtk_label_set_text(GTK_LABEL(label_match), msg);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress), search_progress);
		}

		pthread_mutex_unlock(&mutex);
//		LOG(LOG_DEBUG, "OUT : watch_thread() : CONTINUE");
		return(1);
	}
	gtk_timeout_remove(tag_timeout);

	show_result_tree();
	if(search_result == NULL)
		push_message(_("No hit."));
	else {
		push_message("");
	}

//	if(ebook_search_method() != SEARCH_METHOD_GREP)
		select_first_item();
	gtk_grab_remove(cancel_dialog);
	gtk_widget_destroy(cancel_dialog);
	cancel_dialog=NULL;
	label_cancel=NULL;


	pthread_mutex_destroy(&mutex);

	//LOG(LOG_DEBUG, "OUT : watch_thread()");

	return(0);
}


void thread_search(gboolean cancelable, gchar *text, void *(*func)(void *), void *arg)
{
	gint rc;
	pthread_attr_t thread_attr;
	void *p;

	LOG(LOG_DEBUG, "IN : thread_search(%s)", arg);

	thread_running = 1;
	hit_count = 0;

 	pthread_attr_init (&thread_attr) ;
 	pthread_attr_setstacksize (&thread_attr, 512*1024) ;

	pthread_mutex_init(&mutex, NULL);

	if(cancelable == TRUE) {
		show_cancel_dialog(text);
	}
 
	LOG(LOG_DEBUG, "thread_create");
	rc = pthread_create(&tid, &thread_attr, func, (void *)arg);
	if(rc != 0){
		LOG(LOG_CRITICAL, "pthread_create: %s", strerror(errno));
		LOG(LOG_DEBUG, "OUT : thread_search()");
		exit(1);
	}
	LOG(LOG_DEBUG, "thread_created");
	pthread_attr_destroy(&thread_attr);

	if(cancelable == TRUE) {
		tag_timeout = gtk_timeout_add(WATCH_INTERVAL, watch_thread, NULL);
//		pthread_join(tid, &p);

	} else {
		pthread_join(tid, &p);
		thread_running = 0;
		show_result_tree();
		select_first_item();
	}

	LOG(LOG_DEBUG, "OUT : thread_search()");

}

void add_result(RESULT *rp)
{
	pthread_mutex_lock(&mutex);
	search_result = g_list_append(search_result, rp);
	hit_count ++;
	pthread_mutex_unlock(&mutex);
}

void set_progress(gfloat progress)
{
	pthread_mutex_lock(&mutex);
	search_progress = progress;
	pthread_mutex_unlock(&mutex);
}

void thread_end()
{
	pthread_mutex_lock(&mutex);
	thread_running = 0;
	pthread_mutex_unlock(&mutex);
}

void set_cancel_dlg_text(gchar *text)
{
	if(label_cancel != NULL)
		gtk_label_set_text(GTK_LABEL(label_cancel), text);
}
