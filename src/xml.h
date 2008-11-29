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

#ifndef __XML_H__
#define __XML_H__

#include <glib.h>

/*
構造体は xmlDoc と xmlNode のふたつ。
xmlDoc には root があり、これがルートノードとなる。root は名前を持たず、単に子ノードを持つためのものである。
*/

typedef GNode xmlNode;

typedef struct {
	xmlNode *root;
	gchar *version;
	gchar *encoding;
} xmlDoc;

typedef enum {
	XML_OK,
	XML_NG,
} xmlResult;

typedef struct _NODE_DATA {
	char *name;
	char *content;
	GList *attr;
	gint depth;
	xmlDoc *doc;
} NODE_DATA;

typedef struct _NODE_ATTR {
	char *name;
	char *value;
} NODE_ATTR;


xmlResult parse_buffer(GNode *parent, gchar *text, guint length);
gchar *encoded_to_special(gchar *text);
gchar *special_to_encoded(gchar *text);

xmlDoc    *xml_parse_file(gchar *filename);
xmlDoc    *xml_doc_new();
xmlResult xml_save_file(gchar *filename, xmlDoc *doc);
xmlResult xml_print_tree(xmlDoc *doc);
xmlNode   *xml_add_child(xmlNode *parent, gchar *name, gchar *cotent);
xmlNode   *xml_get_child(xmlNode *node);
xmlNode   *xml_get_next(xmlNode *node);
gchar     *xml_get_name(xmlNode *node);
gchar     *xml_get_content(xmlNode *node);
gchar     *xml_get_attr(xmlNode *node, gchar *name);
xmlResult xml_set_attr(xmlNode *node, gchar *name, gchar *value);
xmlResult xml_destroy_document(xmlDoc *doc);


#endif /* __XML_H__ */
