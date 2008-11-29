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

#include "preference.h"
#include "selection.h"
#include "pref_io.h"

static 	GtkWidget *colorsel_dlg;
static 	GtkWidget *entry_link;
static 	GtkWidget *entry_keyword;
static 	GtkWidget *entry_emphasis;
static 	GtkWidget *entry_sound;
static 	GtkWidget *entry_movie;
static 	GtkWidget *entry_reverse_bg;

gint color_no;

static void ok_colorsel(GtkWidget *widget,gpointer *data){

	GdkColor color;
	gchar *color_name;
	
	LOG(LOG_DEBUG, "IN : ok_colorsel()");

	gtk_grab_remove(colorsel_dlg);

	gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(colorsel_dlg)->colorsel), &color);
	color_name = gtk_color_selection_palette_to_string(&color, 1);

	switch(color_no){
	case COLOR_LINK:
		gtk_entry_set_text(GTK_ENTRY(entry_link), color_name);
		break;
	case COLOR_KEYWORD:
		gtk_entry_set_text(GTK_ENTRY(entry_keyword), color_name);
		break;
	case COLOR_SOUND:
		gtk_entry_set_text(GTK_ENTRY(entry_sound), color_name);
		break;
	case COLOR_MOVIE:
		gtk_entry_set_text(GTK_ENTRY(entry_movie), color_name);
		break;
	case COLOR_EMPHASIS:
		gtk_entry_set_text(GTK_ENTRY(entry_emphasis), color_name);
		break;
	case COLOR_REVERSE_BG:
		gtk_entry_set_text(GTK_ENTRY(entry_reverse_bg), color_name);
		break;
	}


	gtk_widget_destroy(colorsel_dlg);

	LOG(LOG_DEBUG, "OUT : ok_colorsel()");
}

static void delete_colorsel( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	LOG(LOG_DEBUG, "IN : delete_colorsel()");

	ok_colorsel(NULL, NULL);

	LOG(LOG_DEBUG, "OUT : delete_colorsel()");
}

static void show_colorsel(GtkWidget *widget,gpointer *data){

	GdkColor color;
	const gchar *text=NULL;
	
	LOG(LOG_DEBUG, "IN : show_colorsel()");
	
	color_no = (gint)data;


	colorsel_dlg = gtk_color_selection_dialog_new(_("Choose Color"));

	g_signal_connect (G_OBJECT(colorsel_dlg), "delete_event",
			  G_CALLBACK(delete_colorsel), NULL);

	g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(colorsel_dlg)->ok_button), "clicked",
			 G_CALLBACK(ok_colorsel), NULL);

	g_signal_connect_swapped(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(colorsel_dlg)->cancel_button), "clicked",
				G_CALLBACK(gtk_widget_destroy), (gpointer)colorsel_dlg);


	g_assert(color_no < NUM_COLORS);

	switch(color_no){
	case COLOR_LINK:
		text = gtk_entry_get_text(GTK_ENTRY(entry_link));
		break;
	case COLOR_KEYWORD:
		text = gtk_entry_get_text(GTK_ENTRY(entry_keyword));
		break;
	case COLOR_SOUND:
		text = gtk_entry_get_text(GTK_ENTRY(entry_sound));
		break;
	case COLOR_MOVIE:
		text = gtk_entry_get_text(GTK_ENTRY(entry_movie));
		break;
	case COLOR_EMPHASIS:
		text = gtk_entry_get_text(GTK_ENTRY(entry_emphasis));
		break;
	case COLOR_REVERSE_BG:
		text = gtk_entry_get_text(GTK_ENTRY(entry_reverse_bg));
		break;
	}
	
/*
	if(text[0] == '#'){
		strncpy(colorname, &text[1], 2);
		colorname[2] = '\0';
		color_val = strtol(colorname, NULL, 16);
		color.red = color_val * 256;

		strncpy(colorname, &text[3], 2);
		colorname[2] = '\0';
		color_val = strtol(colorname, NULL, 16);
		color.green = color_val * 256;

		strncpy(colorname, &text[5], 2);
		colorname[2] = '\0';
		color_val = strtol(colorname, NULL, 16);
		color.blue = color_val * 256;
	} else {
*/
	gdk_color_parse(text, &color);
/*
	}
*/
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(colorsel_dlg)->colorsel), &color);

	gtk_widget_show_all(colorsel_dlg);

	gtk_grab_add(colorsel_dlg);

	LOG(LOG_DEBUG, "OUT : show_colorsel()");
}

gboolean pref_end_color()
{

	const gchar *colorname;
	gint i;
	
	LOG(LOG_DEBUG, "IN : pref_end_color()");

	for(i=0 ; i < NUM_COLORS ; i ++){
		if(color_str[i])
			free(color_str[i]);
	}

	colorname = gtk_entry_get_text(GTK_ENTRY(entry_link));
	color_str[COLOR_LINK] = strdup(colorname);

	colorname = gtk_entry_get_text(GTK_ENTRY(entry_keyword));
	color_str[COLOR_KEYWORD] = strdup(colorname);

	colorname = gtk_entry_get_text(GTK_ENTRY(entry_sound));
	color_str[COLOR_SOUND] = strdup(colorname);

	colorname = gtk_entry_get_text(GTK_ENTRY(entry_movie));
	color_str[COLOR_MOVIE] = strdup(colorname);

	colorname = gtk_entry_get_text(GTK_ENTRY(entry_emphasis));
	color_str[COLOR_EMPHASIS] = strdup(colorname);

	colorname = gtk_entry_get_text(GTK_ENTRY(entry_reverse_bg));
	color_str[COLOR_REVERSE_BG] = strdup(colorname);

//	free_colors();
//	alloc_colors();
	
	LOG(LOG_DEBUG, "OUT : pref_end_color()");
	return(TRUE);
}


GtkWidget *pref_start_color(){
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *label;
	GtkAttachOptions xoption, yoption;

	LOG(LOG_DEBUG, "IN : pref_start_colrosel()");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_size_request(vbox, 300, 200);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	xoption = GTK_SHRINK;
	yoption = GTK_SHRINK;

	table = gtk_table_new(3, 7, FALSE);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , table,FALSE, FALSE, 0);

	label = gtk_label_new(_("Link"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
			 xoption, yoption, 10, 10);	

	entry_link = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_link), color_str[COLOR_LINK]);
	gtk_widget_set_size_request(entry_link,100,20);
	gtk_table_attach(GTK_TABLE(table), entry_link, 1, 2, 0, 1,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_colorsel), (gpointer)0);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 0, 1,
			 xoption, yoption, 10, 10);	



	label = gtk_label_new(_("Keyword"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
			 xoption, yoption, 10, 10);	

	entry_keyword = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_keyword), color_str[COLOR_KEYWORD]);
	gtk_widget_set_size_request(entry_keyword,100,20);
	gtk_table_attach(GTK_TABLE(table), entry_keyword, 1, 2, 1, 2,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_colorsel), (gpointer)1);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 1, 2,
			 xoption, yoption, 10, 10);	



	label = gtk_label_new(_("Sound"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
			 xoption, yoption, 10, 10);	

	entry_sound = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_sound), color_str[COLOR_SOUND]);
	gtk_widget_set_size_request(entry_sound,100,20);
	gtk_table_attach(GTK_TABLE(table), entry_sound, 1, 2, 2, 3,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_colorsel), (gpointer)2);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 2, 3,
			 xoption, yoption, 10, 10);	

	label = gtk_label_new(_("Movie"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
			 xoption, yoption, 10, 10);	


	entry_movie = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_movie), color_str[COLOR_MOVIE]);
	gtk_widget_set_size_request(entry_movie,100,20);
	gtk_table_attach(GTK_TABLE(table), entry_movie, 1, 2, 3, 4,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_colorsel), (gpointer)3);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 3, 4,
			 xoption, yoption, 10, 10);	


	label = gtk_label_new(_("Emphasis"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 4, 5,
			 xoption, yoption, 10, 10);	


	entry_emphasis = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_emphasis), color_str[COLOR_EMPHASIS]);
	gtk_widget_set_size_request(entry_emphasis,100,20);
	gtk_table_attach(GTK_TABLE(table), entry_emphasis, 1, 2, 4, 5,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_colorsel), (gpointer)4);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 4, 5,
			 xoption, yoption, 10, 10);	




	label = gtk_label_new(_("Reverse Background"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 5, 6,
			 xoption, yoption, 10, 10);	


	entry_reverse_bg = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_reverse_bg), color_str[COLOR_REVERSE_BG]);
	gtk_widget_set_size_request(entry_reverse_bg,100,20);
	gtk_table_attach(GTK_TABLE(table), entry_reverse_bg, 1, 2, 5, 6,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_colorsel), (gpointer)5);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 5, 6,
			 xoption, yoption, 10, 10);	

	LOG(LOG_DEBUG, "OUT : pref_start_colrosel()");

	return(vbox);
}

