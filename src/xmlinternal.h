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

#ifndef __XMLINTERNAL_H__
#define __XMLINTERNAL_H__

#include "defs.h"

void get_tag_name(gchar *text, gchar *tag);
void get_start_tag(gchar *text, gchar *tag);
void get_end_tag(gchar *text, gchar *tag_name, gchar *tag);
void get_content(gchar *text, gchar *tag_name, gchar **content, gint *content_length);
void get_attr(const gchar *tag, gchar *name, gchar *value);
void skip_start_tag(gchar **text, gchar *tag_name);
void skip_end_tag(gchar **text, gchar *tag_name);

#endif /* __XMLINTERNAL_H__ */
