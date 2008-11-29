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
#include "cellrenderercolor.h"
#include "defs.h"
#include "global.h"
#include "xmlinternal.h"
#include "jcode.h"
#include "render.h"


static void gtk_cell_renderer_color_init       (GtkCellRendererColor      *cellcolor);
static void gtk_cell_renderer_color_class_init (GtkCellRendererColorClass *class);
static void gtk_cell_renderer_color_finalize   (GObject                  *object);

static void gtk_cell_renderer_color_get_property  (GObject                  *object,
						   guint                     param_id,
						   GValue                   *value,
						   GParamSpec               *pspec);
static void gtk_cell_renderer_color_set_property  (GObject                  *object,
						   guint                     param_id,
						   const GValue             *value,
						   GParamSpec               *pspec);
static void gtk_cell_renderer_color_get_size   (GtkCellRenderer          *cell,
						GtkWidget                *widget,
						GdkRectangle             *cell_area,
						gint                     *x_offset,
						gint                     *y_offset,
						gint                     *width,
						gint                     *height);
static void gtk_cell_renderer_color_render     (GtkCellRenderer          *cell,
						GdkWindow                *window,
						GtkWidget                *widget,
						GdkRectangle             *background_area,
						GdkRectangle             *cell_area,
						GdkRectangle             *expose_area,
						GtkCellRendererState      flags);

static void cell_renderer_color_render_color(GtkCellRenderer *cell,
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
	PROP_COLOR,
};

static gpointer parent_class;


GtkType
gtk_cell_renderer_color_get_type (void)
{
	static GtkType cell_color_type = 0;

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_get_type()");

	if (!cell_color_type)
	{
		static const GTypeInfo cell_color_info =
		{
			sizeof (GtkCellRendererColorClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) gtk_cell_renderer_color_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (GtkCellRendererColor),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gtk_cell_renderer_color_init,
		};

		cell_color_type = g_type_register_static (GTK_TYPE_CELL_RENDERER, "GtkCellRendererColor", &cell_color_info, 0);
	}

//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_get_type()");
	return cell_color_type;
}

static void
gtk_cell_renderer_color_init (GtkCellRendererColor *cellcolor)
{
//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_init()");

	GTK_CELL_RENDERER (cellcolor)->xalign = 0.0;
	GTK_CELL_RENDERER (cellcolor)->yalign = 0.5;
	GTK_CELL_RENDERER (cellcolor)->xpad = 2;
	GTK_CELL_RENDERER (cellcolor)->ypad = 2;

//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_init()");
}

static void
gtk_cell_renderer_color_class_init (GtkCellRendererColorClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (class);

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_class_init()");

	parent_class = g_type_class_peek_parent (class);
  
	object_class->finalize = gtk_cell_renderer_color_finalize;
  
	object_class->get_property = gtk_cell_renderer_color_get_property;
	object_class->set_property = gtk_cell_renderer_color_set_property;

	cell_class->get_size = gtk_cell_renderer_color_get_size;
	cell_class->render = gtk_cell_renderer_color_render;
  
	g_object_class_install_property (object_class,
					 PROP_COLOR,
					 g_param_spec_string ("color",
							      _("Color"),
							      _("Color"),
							      NULL,
							      G_PARAM_READWRITE));
  
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_class_init()");

}

static void
gtk_cell_renderer_color_finalize (GObject *object)
{
	GtkCellRendererColor *cellcolor = GTK_CELL_RENDERER_COLOR (object);

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_finalize()");

	if (cellcolor->color)
		g_free (cellcolor->color);

	(* G_OBJECT_CLASS (parent_class)->finalize) (object);

//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_finalize()");
}

static void
gtk_cell_renderer_color_get_property (GObject        *object,
				      guint           param_id,
				      GValue         *value,
				      GParamSpec     *pspec)
{
	GtkCellRendererColor *cellcolor = GTK_CELL_RENDERER_COLOR (object);

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_get_property()");

	switch (param_id)
	{
	case PROP_COLOR:
		g_value_set_string (value, cellcolor->color);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_get_property()");
}


static void
gtk_cell_renderer_color_set_property (GObject      *object,
				      guint         param_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	GtkCellRendererColor *cellcolor = GTK_CELL_RENDERER_COLOR (object);

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_set_property()");

	switch (param_id)
	{
	case PROP_COLOR:
		if (cellcolor->color)
			g_free (cellcolor->color);
		cellcolor->color = g_strdup (g_value_get_string (value));
//		g_object_notify (object, "color");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_set_property()");
}

GtkCellRenderer *
gtk_cell_renderer_color_new (void)
{
//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_new()");
	return GTK_CELL_RENDERER (g_object_new (gtk_cell_renderer_color_get_type (), NULL));
//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_new()");
}

static void
gtk_cell_renderer_color_get_size (GtkCellRenderer *cell,
				  GtkWidget       *widget,
				  GdkRectangle    *cell_area,
				  gint            *x_offset,
				  gint            *y_offset,
				  gint            *width,
				  gint            *height)
{
	GtkCellRendererColor *cellcolor = (GtkCellRendererColor *) cell;

//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_get_size()");

	if(width)
		*width = 40;
	if(height)
		*height = 10;
	if(x_offset)
		*x_offset = 2;
	if(y_offset)
		*y_offset = 2;

//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_get_size()");
}

static void
gtk_cell_renderer_color_render (GtkCellRenderer      *cell,
				GdkWindow            *window,
				GtkWidget            *widget,
				GdkRectangle         *background_area,
				GdkRectangle         *cell_area,
				GdkRectangle         *expose_area,
				GtkCellRendererState  flags)

{
	GtkCellRendererColor *cellcolor = (GtkCellRendererColor *) cell;
	GtkStateType state;
	gint x_offset;
	gint y_offset;

	GtkWidget *button;


//	LOG(LOG_DEBUG, "IN : gtk_cell_renderer_color_render()");


	gtk_cell_renderer_color_get_size (cell, widget, cell_area, &x_offset, &y_offset, NULL, NULL);

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

	gtk_paint_option


//	LOG(LOG_DEBUG, "OUT : gtk_cell_renderer_color_render()");
}

