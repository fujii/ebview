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

/* This source derives from gtkcellrenerertext.c of GTK+-2.0.9
 * Here is an original copyright.
*/

/* gtkcellrenderertext.c
 * Copyright (C) 2000  Red Hat, Inc.,  Jonathan Blandford <jrb@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include "cellrendererebook.h"
#include "defs.h"
#include "global.h"
#include "xmlinternal.h"
#include "jcode.h"
#include "render.h"


static void gtk_cell_renderer_ebook_init       (GtkCellRendererEbook      *cellebook);
static void gtk_cell_renderer_ebook_class_init (GtkCellRendererEbookClass *class);
static void gtk_cell_renderer_ebook_finalize   (GObject                  *object);

static void gtk_cell_renderer_ebook_get_property  (GObject                  *object,
						   guint                     param_id,
						   GValue                   *value,
						   GParamSpec               *pspec);
static void gtk_cell_renderer_ebook_set_property  (GObject                  *object,
						   guint                     param_id,
						   const GValue             *value,
						   GParamSpec               *pspec);
static void gtk_cell_renderer_ebook_get_size   (GtkCellRenderer          *cell,
						GtkWidget                *widget,
						GdkRectangle             *cell_area,
						gint                     *x_offset,
						gint                     *y_offset,
						gint                     *width,
						gint                     *height);
static void gtk_cell_renderer_ebook_render     (GtkCellRenderer          *cell,
						GdkWindow                *window,
						GtkWidget                *widget,
						GdkRectangle             *background_area,
						GdkRectangle             *cell_area,
						GdkRectangle             *expose_area,
						GtkCellRendererState      flags);

static void cell_renderer_ebook_render_ebook(GtkCellRenderer *cell,
					     GdkWindow *window,
					     GtkWidget *widget,
					     GtkStateType state,
					     gchar *text,
					     BOOK_INFO *binfo,
					     gint origin_x,
					     gint origin_y,
					     gboolean render);

enum {
	PROP_0,
	PROP_TEXT,
	PROP_BOOK,
};

static gpointer parent_class;


GtkType
gtk_cell_renderer_ebook_get_type (void)
{
	static GtkType cell_ebook_type = 0;

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_get_type()");

	if (!cell_ebook_type)
	{
		static const GTypeInfo cell_ebook_info =
		{
			sizeof (GtkCellRendererEbookClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) gtk_cell_renderer_ebook_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (GtkCellRendererEbook),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gtk_cell_renderer_ebook_init,
		};

		cell_ebook_type = g_type_register_static (GTK_TYPE_CELL_RENDERER, "GtkCellRendererEbook", &cell_ebook_info, 0);
	}

//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_get_type()");
	return cell_ebook_type;
}

static void
gtk_cell_renderer_ebook_init (GtkCellRendererEbook *cellebook)
{
//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_init()");

	GTK_CELL_RENDERER (cellebook)->xalign = 0.0;
	GTK_CELL_RENDERER (cellebook)->yalign = 0.5;
	GTK_CELL_RENDERER (cellebook)->xpad = 2;
	GTK_CELL_RENDERER (cellebook)->ypad = 2;
	cellebook->width = 0;
	cellebook->height = 0;

//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_init()");
}

static void
gtk_cell_renderer_ebook_class_init (GtkCellRendererEbookClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (class);

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_class_init()");

	parent_class = g_type_class_peek_parent (class);
  
	object_class->finalize = gtk_cell_renderer_ebook_finalize;
  
	object_class->get_property = gtk_cell_renderer_ebook_get_property;
	object_class->set_property = gtk_cell_renderer_ebook_set_property;

	cell_class->get_size = gtk_cell_renderer_ebook_get_size;
	cell_class->render = gtk_cell_renderer_ebook_render;
  
	g_object_class_install_property (object_class,
					 PROP_TEXT,
					 g_param_spec_string ("text",
							      _("Text"),
							      _("Text to render"),
							      NULL,
							      G_PARAM_READWRITE));
  

	g_object_class_install_property (object_class,
					 PROP_BOOK,
					 g_param_spec_pointer ("book",
							      _("BookInfo"),
							      _("Book Information"),
							      G_PARAM_READWRITE));
  
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_class_init()");

}

static void
gtk_cell_renderer_ebook_finalize (GObject *object)
{
	GtkCellRendererEbook *cellebook = GTK_CELL_RENDERER_EBOOK (object);

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_finalize()");

	if (cellebook->text)
		g_free (cellebook->text);

	(* G_OBJECT_CLASS (parent_class)->finalize) (object);

//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_finalize()");
}

static void
gtk_cell_renderer_ebook_get_property (GObject        *object,
				      guint           param_id,
				      GValue         *value,
				      GParamSpec     *pspec)
{
	GtkCellRendererEbook *cellebook = GTK_CELL_RENDERER_EBOOK (object);

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_get_property()");

	switch (param_id)
	{
	case PROP_TEXT:
		g_value_set_string (value, cellebook->text);
		break;
	case PROP_BOOK:
		g_value_set_pointer(value, cellebook->binfo);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_get_property()");
}


static void
gtk_cell_renderer_ebook_set_property (GObject      *object,
				      guint         param_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	GtkCellRendererEbook *cellebook = GTK_CELL_RENDERER_EBOOK (object);

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_set_property()");

	switch (param_id)
	{
	case PROP_TEXT:
		if (cellebook->text)
			g_free (cellebook->text);
		cellebook->text = g_strdup (g_value_get_string (value));
//		g_object_notify (object, "text");
		break;
	case PROP_BOOK:
		cellebook->binfo = (BOOK_INFO *)g_value_get_pointer(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_set_property()");
}

/**
 * gtk_cell_renderer_ebook_new:
 * 
 * Creates a new #GtkCellRendererEbook. Adjust how text is drawn using
 * object properties. Object properties can be
 * set globally (with g_object_set()). Also, with #GtkTreeViewColumn,
 * you can bind a property to a value in a #GtkTreeModel. For example,
 * you can bind the "text" property on the cell renderer to a string
 * value in the model, thus rendering a different string in each row
 * of the #GtkTreeView
 * 
 * Return value: the new cell renderer
 **/
GtkCellRenderer *
gtk_cell_renderer_ebook_new (void)
{
//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_new()");
	return GTK_CELL_RENDERER (g_object_new (gtk_cell_renderer_ebook_get_type (), NULL));
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_new()");
}

static void
gtk_cell_renderer_ebook_get_size (GtkCellRenderer *cell,
				  GtkWidget       *widget,
				  GdkRectangle    *cell_area,
				  gint            *x_offset,
				  gint            *y_offset,
				  gint            *width,
				  gint            *height)
{
	GtkCellRendererEbook *cellebook = (GtkCellRendererEbook *) cell;

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_get_size()");

	if(width)
		*width = 100;
	if(height)
		*height = 20;
	if(x_offset)
		*x_offset = 2;
	if(y_offset)
		*y_offset = 2;

	if(cellebook->text && width && height){

		// Calculate the size
	        // If the last parameter is FALSE, no actual drawing.
		cellebook->width = 0;
		cellebook->height = 0;

		cell_renderer_ebook_render_ebook(cell,
						 NULL,
						 widget,
						 GTK_STATE_NORMAL,
						 cellebook->text,
						 cellebook->binfo,
						 0, 0,
						 FALSE);

		*width = cellebook->width + cell->xpad * 2;
//		*height = cellebook->height + cell->ypad * 3;
		*height = font_height + cell->ypad * 4;

		cellebook->width = 0;
		cellebook->height = 0;

	}

//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_get_size()");
}

static void cell_renderer_ebook_render_gaiji(GtkCellRenderer *cell,
					     GdkWindow *window,
					     GtkWidget *widget,
					     GtkStateType state,
					     BOOK_INFO *binfo,
					     gint *x, gint *y,
					     gchar *code,
					     gboolean render)
{
	GtkCellRendererEbook *cellebook = (GtkCellRendererEbook *) cell;
	gchar *color_name;
	gint width, height;
	GdkPixbuf *pixbuf;
	gint l_y;


//	LOG(LOG_DEBUG, "IN : cell_renderer_ebook_render_gaiji(code=%s)", code);

	color_name = gtk_color_selection_palette_to_string(
		&(widget->style->fg[state]), 1);
	//color_name = strdup("Black");
	pixbuf = load_xbm(binfo, code, &width, &height, color_name);

	// Is 0.8  appropriate ?
        l_y = *y + font_ascent - height * 0.8;
	if(l_y < 0)
		l_y = 0;

	if(render){
		gdk_pixbuf_render_to_drawable_alpha (pixbuf,
                                         window,
                                         /* pixbuf 0, 0 is at pix_rect.x, pix_rect.y */
					     0, 0,
					     *x,
					     //*y,
					     l_y,
					     width,
					     height,
					     GDK_PIXBUF_ALPHA_FULL,
					     0,
					     GDK_RGB_DITHER_NORMAL,
					     0, 0);
	}

	*x += width + 2;

	if(height > cellebook->height)
		cellebook->height = height;
	cellebook->width += width + 2;


	gdk_pixbuf_unref(pixbuf);
	g_free(color_name);

//	LOG(LOG_DEBUG, "OUT : cell_renderer_ebook_render_gaiji()");
}

struct special_char {
	guchar special;
	gchar  *encoded;
};

static struct special_char special[] = {{'&', "&amp;"}, {'\"', "&quot;"}, {0, NULL}};


static gchar *replace_special_char(gchar *text){
	gchar buff[65536];
	gchar *p;
	gint i;
	gint j;

	p = text;
	j = 0;

	while(*p){
		for(i=0; ; i ++){
			if(special[i].encoded == NULL) {
				buff[j] = *p;
				j++;
				break;
			}
			if(*p == special[i].special){
				strcpy(&buff[j], special[i].encoded);
				j += strlen(special[i].encoded);
				break;
			}
		}
		p++;
	}

	buff[j] = '\0';
	return(g_strdup(buff));
}



static void cell_renderer_ebook_render_string(GtkCellRenderer *cell,
					      GdkWindow *window,
					      GtkWidget *widget,
					      GtkStateType state,
					      gchar *text,
					      gint length,
					      gint *x, gint *y,
					      gboolean render)
{
	GtkCellRendererEbook *cellebook = (GtkCellRendererEbook *) cell;
	PangoLayout *layout;
	gchar *str;
	gchar *tmp_str;
	PangoRectangle rect;
	PangoAttrList *attrs;
	gchar *parsed_text;
	gchar *color_name;


//	LOG(LOG_DEBUG, "IN : cell_renderer_ebook_render_string(text=%s,w=%d, h=%d)",text, *x, *y );

	color_name = gtk_color_selection_palette_to_string(
		&(widget->style->fg[state]), 1);

	str = g_strndup(text, length);

/*
	tmp_str = str;
	str = replace_special_char(tmp_str);
	g_free(tmp_str);
*/
	tmp_str = str;
	str = g_strdup_printf("<span foreground=\"%s\" font_desc=\"%s\" >%s</span>",
			      color_name,
			      fontset_normal,
			      tmp_str);
	g_free(tmp_str);

	pango_parse_markup (str, -1, 0, &attrs, &parsed_text, NULL, NULL);

	if(parsed_text){
		layout = gtk_widget_create_pango_layout(widget, parsed_text);
		pango_layout_set_attributes(layout, attrs);
	} else {
		layout = gtk_widget_create_pango_layout(widget, str);
	}

	pango_layout_get_pixel_extents (layout, NULL, &rect);

	if(render){
		gdk_draw_layout(window, 
				widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
				*x, *y+1,
				layout);
	}

	if(rect.height > cellebook->height)
		cellebook->height = rect.height;
	cellebook->width += rect.width;

	*x += rect.width;
	//y += rect.height;

	g_free(str);
	if(parsed_text)
		g_free(parsed_text);
	
//	LOG(LOG_DEBUG, "OUT : cell_renderer_ebook_render_string()");
}


static void cell_renderer_ebook_render_ebook(GtkCellRenderer *cell,
					     GdkWindow *window,
					     GtkWidget *widget,
					     GtkStateType state,
					     gchar *text,
					     BOOK_INFO *binfo,
					     gint origin_x,
					     gint origin_y,
					     gboolean render)
{

	gchar *p;
	gint  body_length;
	gchar body[65535];
	gchar tag_name[512];
	gchar start_tag[512];
	gchar code[16];
	gint x, y;

//	LOG(LOG_DEBUG, "IN : cell_renderer_ebook_render_ebook()");

	x = origin_x;
	y = origin_y;

	p = text;
	body_length = 0;

	while(*p != '\0'){
		if(*p == '<'){
			get_start_tag(p, start_tag);
			get_tag_name(start_tag, tag_name);

			if(strcmp(tag_name, "gaiji") == 0){
				if(body_length != 0){
					cell_renderer_ebook_render_string(cell,
								  window, 
								  widget,
								  state,
								  body,
								  body_length,
								  &x, &y,
								  render);
					body_length = 0;
				}

				get_attr(start_tag, "code", code);

				cell_renderer_ebook_render_gaiji(cell,
								 window,
								 widget,
								 state,
								 binfo,
								 &x, &y,
								 code,
								 render);
	
				skip_start_tag(&p, tag_name);

			} else if((strcmp(tag_name, "sup") == 0) ||
				  (strcmp(tag_name, "sub") == 0)) {
				gchar *pp;
				pp = p;
				skip_end_tag(&p, tag_name);
				strncpy(&body[body_length], pp, p - pp);
				body_length += p - pp;
				body[body_length] = '\0';
			} else {
/*
				body[body_length] = *p;
				body_length ++;
				body[body_length] = '\0';
				p++;
*/
				body[body_length] = '&';
				body_length ++;
				body[body_length] = 'l';
				body_length ++;
				body[body_length] = 't';
				body_length ++;
				body[body_length] = ';';
				body_length ++;
				body[body_length] = '\0';
				p++;
			}
		} else if(*p == '>'){
			body[body_length] = '&';
			body_length ++;
			body[body_length] = 'g';
			body_length ++;
			body[body_length] = 't';
			body_length ++;
			body[body_length] = ';';
			body_length ++;
			body[body_length] = '\0';
			p++;
		} else if(*p == '&'){
			body[body_length] = '&';
			body_length ++;
			body[body_length] = 'a';
			body_length ++;
			body[body_length] = 'm';
			body_length ++;
			body[body_length] = 'p';
			body_length ++;
			body[body_length] = ';';
			body_length ++;
			body[body_length] = '\0';
			p++;
		} else if(*p == '\"'){
			body[body_length] = '&';
			body_length ++;
			body[body_length] = 'q';
			body_length ++;
			body[body_length] = 'u';
			body_length ++;
			body[body_length] = 'o';
			body_length ++;
			body[body_length] = 't';
			body_length ++;
			body[body_length] = ';';
			body_length ++;
			body[body_length] = '\0';
			p++;
		} else {
			body[body_length] = *p;
			body_length ++;
			body[body_length] = '\0';
			p++;
		}

	}

	if(body_length != 0){
		cell_renderer_ebook_render_string(cell,
						  window, 
						  widget,
						  state,
						  body,
						  body_length,
						  &x, &y,
						  render);
	}

//	LOG(LOG_DEBUG, "OUT : cell_renderer_ebook_render_ebook()");

}

static void
gtk_cell_renderer_ebook_render (GtkCellRenderer      *cell,
				GdkWindow            *window,
				GtkWidget            *widget,
				GdkRectangle         *background_area,
				GdkRectangle         *cell_area,
				GdkRectangle         *expose_area,
				GtkCellRendererState  flags)

{
	GtkCellRendererEbook *cellebook = (GtkCellRendererEbook *) cell;
	GtkStateType state;
	gint x_offset;
	gint y_offset;


//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_ebook_render()");


	gtk_cell_renderer_ebook_get_size (cell, widget, cell_area, &x_offset, &y_offset, NULL, NULL);

	if ((flags & GTK_CELL_RENDERER_SELECTED) == GTK_CELL_RENDERER_SELECTED)
	{
		if (GTK_WIDGET_HAS_FOCUS (widget)){
			state = GTK_STATE_SELECTED;
		}
		else {
			state = GTK_STATE_ACTIVE;
		}
	}
	else
	{
		if (GTK_WIDGET_STATE (widget) == GTK_STATE_INSENSITIVE){
			state = GTK_STATE_INSENSITIVE;
		}
		else {
			state = GTK_STATE_NORMAL;
		}
	}

/*     
	if (state != GTK_STATE_SELECTED){
	{
		GdkColor color;
		GdkGC *gc;

		gc = gdk_gc_new (window);

		gdk_gc_set_foreground(gc, &widget->style->bg[state]);
		gdk_gc_set_background(gc, &widget->style->bg[state]);
      
		gdk_draw_rectangle (window,
				    gc,
				    TRUE,
				    background_area->x,
				    background_area->y,
				    background_area->width,
				    background_area->height);

		g_object_unref (G_OBJECT (gc));
	}
*/

	cell_renderer_ebook_render_ebook(cell,
					 window,
					 widget,
					 state,
					 cellebook->text,
					 cellebook->binfo,
					 cell_area->x + cell->xpad,
					 cell_area->y + cell->ypad,
					 TRUE);
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_ebook_render()");
}

