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
#include "selection.h"
#include "preference.h"
#include "headword.h"
#include "mainwindow.h"

static 	GtkWidget *fontsel_dlg;
static 	GtkWidget *entry_normal;
static 	GtkWidget *entry_bold;
static 	GtkWidget *entry_italic;
static 	GtkWidget *entry_super;
static  gint font_no;


static void ok_fontsel(GtkWidget *widget,gpointer *data){
	gchar *fontname;

	LOG(LOG_DEBUG, "IN : ok_fontsel()");

	fontname = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(fontsel_dlg));

	LOG(LOG_DEBUG, "fontname = %s", fontname);

	switch(font_no){
	case 0:
		gtk_entry_set_text(GTK_ENTRY(entry_normal), fontname); 
		break;
	case 1:
		gtk_entry_set_text(GTK_ENTRY(entry_bold), fontname); 
		break;
	case 2:
		gtk_entry_set_text(GTK_ENTRY(entry_italic), fontname); 
		break;
	case 3:
		gtk_entry_set_text(GTK_ENTRY(entry_super), fontname); 
		break;
	}

	gtk_grab_remove(fontsel_dlg);

	gtk_widget_destroy(fontsel_dlg);

	LOG(LOG_DEBUG, "OUT : ok_fontsel()");

}

static void delete_fontsel( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	LOG(LOG_DEBUG, "IN : delete_fontsel()");
	ok_fontsel(NULL, NULL);
	LOG(LOG_DEBUG, "OUT : delete_fontsel()");
}

static void show_fontsel(GtkWidget *widget,gpointer *data){
	const gchar *fontname=NULL;

	LOG(LOG_DEBUG, "IN : show_fontsel()");

	font_no = (gint)data;

	fontsel_dlg = gtk_font_selection_dialog_new("Please select font");

	g_signal_connect(G_OBJECT (fontsel_dlg), "delete_event",
			 G_CALLBACK(delete_fontsel), NULL);

	g_signal_connect(G_OBJECT(GTK_FONT_SELECTION_DIALOG (fontsel_dlg)->ok_button), "clicked",
			 G_CALLBACK(ok_fontsel), NULL);

	g_signal_connect_swapped(G_OBJECT(GTK_FONT_SELECTION_DIALOG (fontsel_dlg)->cancel_button), "clicked",
				 G_CALLBACK(gtk_widget_destroy), (gpointer)fontsel_dlg);

	gtk_widget_destroy(GTK_FONT_SELECTION_DIALOG (fontsel_dlg)->apply_button);

	switch(font_no){
	case 0:
		fontname = gtk_entry_get_text(GTK_ENTRY(entry_normal));
		break;
	case 1:
		fontname = gtk_entry_get_text(GTK_ENTRY(entry_bold));
		break;
	case 2:
		fontname = gtk_entry_get_text(GTK_ENTRY(entry_italic));
		break;
	case 3:
		fontname = gtk_entry_get_text(GTK_ENTRY(entry_super));
		break;
	}

	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(fontsel_dlg), fontname);

	gtk_widget_show_all(fontsel_dlg);
	gtk_grab_add(fontsel_dlg);

	LOG(LOG_DEBUG, "OUT : show_fontsel()");
}

gboolean pref_end_font()
{
	const gchar *fontname;

	LOG(LOG_DEBUG, "IN : pref_end_font()");
	
	// Program aborts if you unload. Why ?
	//unload_font();


	fontname = gtk_entry_get_text(GTK_ENTRY(entry_normal));
	free(fontset_normal);
	fontset_normal = strdup(fontname);

	fontname = gtk_entry_get_text(GTK_ENTRY(entry_bold));
	free(fontset_bold);
	fontset_bold = strdup(fontname);

	fontname = gtk_entry_get_text(GTK_ENTRY(entry_italic));
	free(fontset_italic);
	fontset_italic = strdup(fontname);

	fontname = gtk_entry_get_text(GTK_ENTRY(entry_super));
	free(fontset_superscript);
	fontset_superscript = strdup(fontname);

	return(TRUE);

	LOG(LOG_DEBUG, "OUT : pref_end_font()");
}

GtkWidget *pref_start_font(){
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *label;
	GtkAttachOptions xoption=0, yoption=0;

	LOG(LOG_DEBUG, "IN : pref_start_font()");

	vbox = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new(3, 5, FALSE);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , table,FALSE, FALSE, 0);

	label = gtk_label_new(_("Normal"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
			 xoption, yoption, 10, 10);	

	entry_normal = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_normal), fontset_normal);
	gtk_widget_set_size_request(entry_normal,200,20);
	gtk_table_attach(GTK_TABLE(table), entry_normal, 1, 2, 0, 1,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_fontsel), (gpointer)0);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 0, 1,
			 xoption, yoption, 10, 10);	


	label = gtk_label_new(_("Bold"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
			 xoption, yoption, 10, 10);	

	entry_bold = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_bold), fontset_bold);
	gtk_widget_set_size_request(entry_bold,200,20);
	gtk_table_attach(GTK_TABLE(table), entry_bold, 1, 2, 1, 2,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_fontsel), (gpointer)1);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 1, 2,
			 xoption, yoption, 10, 10);	


	label = gtk_label_new(_("Italic"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
			 xoption, yoption, 10, 10);	

	entry_italic = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_italic), fontset_italic);
	gtk_widget_set_size_request(entry_italic,200,20);
	gtk_table_attach(GTK_TABLE(table), entry_italic, 1, 2, 2, 3,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_fontsel), (gpointer)2);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 2, 3,
			 xoption, yoption, 10, 10);	

	label = gtk_label_new(_("Superscript"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
			 xoption, yoption, 10, 10);	


	entry_super = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_super), fontset_superscript);
	gtk_widget_set_size_request(entry_super,200,20);
	gtk_table_attach(GTK_TABLE(table), entry_super, 1, 2, 3, 4,
			 xoption, yoption, 10, 10);	

	button = gtk_button_new_with_label(_("Choose"));
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(show_fontsel), (gpointer)3);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 3, 4,
			 xoption, yoption, 10, 10);	

	LOG(LOG_DEBUG, "OUT : pref_start_font()");

	return(vbox);
}

