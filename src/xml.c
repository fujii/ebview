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
#include "xml.h"
#include "xmlinternal.h"
#include "jcode.h"

//#define XML_TRACE

static void xml_save_file_internal(GNode *node, gpointer data);
static void xml_print_tree_internal(GNode *node, gpointer data);
static void xml_destroy_tree_internal(GNode *node, gpointer data);

struct special_char {
	guchar special;
	gchar  *encoded;
};

static struct special_char special[] = {{'&', "&amp;"}, {'\"', "&quot;"}, {'<', "&lt;"}, {'>', "&gt;"}, {0, NULL}};



xmlDoc *xml_doc_new()
{

	GNode *root;
	xmlDoc *doc;
	NODE_DATA *node_data;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_doc_new()");
#endif

	doc = (xmlDoc *)calloc(sizeof(xmlDoc), 1);
	g_assert(doc != NULL);

	doc->version = NULL;
	doc->encoding = NULL;
	
	node_data = (NODE_DATA *)calloc(sizeof(NODE_DATA), 1);
	g_assert(node_data != NULL);

	node_data->name = NULL;
	node_data->content = NULL;
	node_data->attr = NULL;
	node_data->depth = 0;
	node_data->doc = doc;
	root = g_node_new((gpointer)node_data);

	doc->root = root;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : xml_doc_new()");
#endif
	return(doc);
}

xmlDoc *xml_parse_file(gchar *filename)
{
	unsigned int l;
	GNode *root;
	xmlDoc *doc;
	NODE_DATA *node_data;
	char buff[65535];
	FILE *fp;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_parse_file(%s)", filename);
#endif

	fp = fopen(filename, "r");
	if(fp == NULL){
		LOG(LOG_CRITICAL, "fopen: %s", strerror(errno));
		return(NULL);
	}

	//サイズは最大65535バイト
	memset(buff, 0, sizeof(buff));
	l = fread(buff, 1, sizeof(buff), fp);
	fclose(fp);
	if(l <= 0){
		return(NULL);
	}
	
	doc = (xmlDoc *)calloc(sizeof(xmlDoc), 1);
	doc->version = NULL;
	doc->encoding = NULL;
	
	node_data = (NODE_DATA *)calloc(sizeof(NODE_DATA), 1);
	node_data->name = NULL;
	node_data->content = NULL;
	node_data->attr = NULL;
	node_data->depth = 0;
	node_data->doc = doc;
	root = g_node_new((gpointer)node_data);

	doc->root = root;

	parse_buffer(root, buff, l);

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : xml_parse_file()");
#endif
	return(doc);
}

void indent_tag(FILE *fp, int l){
	int i;
	for(i=0;i<l-1;i++)
		fprintf(fp, "  ");
}

xmlResult xml_save_file(gchar *filename, xmlDoc *doc)
{
	FILE *fp;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_save_file(%s)", filename);
#endif

	fp = fopen(filename, "w");
	if(fp == NULL){
		LOG(LOG_CRITICAL, "fopen: %s", strerror(errno));
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : xml_save_file()=NG");
#endif
		return(XML_NG);
	}

	fprintf(fp, "<?xml");
	if(doc->version){
		fprintf(fp, " version=\"%s\"", doc->version);
	}
	if(doc->encoding){
		fprintf(fp, " encoding=\"%s\"", doc->encoding);
	}

	fprintf(fp, "?>\n");

	xml_save_file_internal(doc->root, fp);

	fclose(fp);

#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : xml_save_file()");
#endif

	return(XML_OK);
}

static void xml_save_file_internal(GNode *node, gpointer data){
	FILE *fp;
	NODE_DATA *node_data;
	gchar *tmp_p;
	gchar *utf_str;

#ifdef XML_TRACE
		LOG(LOG_DEBUG, "IN : xml_save_file_internal()");
#endif

	fp = (FILE *)data;
	node_data = (NODE_DATA *)(node->data);

	if(node_data->name){
		indent_tag(fp, node_data->depth);
		fprintf(fp, "<%s",node_data->name);
		if(node_data->attr != NULL){
			GList *list;
			NODE_ATTR *attr;
			list = g_list_first(node_data->attr);
			while(list){
				attr = (NODE_ATTR *)(list->data);
				tmp_p = special_to_encoded(attr->value);
				utf_str = iconv_convert("utf-8",
							node_data->doc->encoding,
							tmp_p);

				fprintf(fp, " %s=\"%s\"", attr->name, utf_str);
				g_free(tmp_p);
				g_free(utf_str);
				list = g_list_next(list);
			}
		}
		fprintf(fp, ">");

		if(G_NODE_IS_LEAF(node)){
			if (node_data->content != NULL){
				tmp_p = special_to_encoded(node_data->content);
				utf_str = iconv_convert("utf-8",
							node_data->doc->encoding,
							tmp_p);

				fprintf(fp, "%s", utf_str);
				g_free(tmp_p);
				g_free(utf_str);
			}
		} else {
			fprintf(fp, "\n");
			g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)xml_save_file_internal, (gpointer)fp);
			indent_tag(fp, node_data->depth);
		}

		if(node_data->name)
			fprintf(fp, "</%s>\n",node_data->name);
	} else {
		g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)xml_save_file_internal, (gpointer)fp);
	}

#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : xml_save_file_internal()");
#endif

}

xmlResult xml_print_tree(xmlDoc *doc){
	xml_print_tree_internal((GNode *)doc->root, NULL);
	return(XML_OK);
}

static void print_indent(int l){
	int i;
	for(i=0;i<l;i++)
		printf("| ");
	printf("|- ");
}

static void print_indent2(int l){
	int i;
	for(i=0;i<l;i++)
		printf("| ");
	printf("|    ");
}

static void xml_print_tree_internal(GNode *node, gpointer data){
	NODE_DATA *node_data;

	node_data = (NODE_DATA *)(node->data);

	if(node_data->name){
		print_indent(node_data->depth);
		printf("%s", node_data->name);
		if(node_data->attr != NULL){
			GList *list;
			NODE_ATTR *attr;
			list = g_list_first(node_data->attr);
			while(list){
				attr = (NODE_ATTR *)(list->data);
				g_print(" %s=%s", attr->name, attr->value);
				list = g_list_next(list);
			}
		}
		g_print("\n");

		if(G_NODE_IS_LEAF(node)){
			print_indent2(node_data->depth);
			if(node_data->content != NULL){
				printf(">%s<\n", node_data->content);
			} else {
				printf("NULL\n");
			}
		} else {
			g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)xml_print_tree_internal, (gpointer)NULL);
		}
	} else {
		g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)xml_print_tree_internal, (gpointer)NULL);
	}
}


xmlNode *xml_add_child(xmlNode *parent, gchar *name, gchar *content){
	NODE_DATA *node_data;
	GNode *child;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_add_child()");
#endif

	if(name == NULL) {
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : xml_add_child() = NULL");
#endif
		return(NULL);
	}

	node_data = (NODE_DATA *)calloc(sizeof(NODE_DATA), 1);
	if(!node_data){
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : xml_add_child() = NULL");
#endif
		return(NULL);
	}
	node_data->name = g_strdup(name);
	node_data->attr = NULL;
	node_data->depth = ((NODE_DATA *)(parent->data))->depth + 1;
	node_data->doc = ((NODE_DATA *)(parent->data))->doc;
	if(content == NULL)
		node_data->content = NULL;
	else
		node_data->content = g_strdup(content);

	child = g_node_new((gpointer)node_data);
	g_node_append(parent, child);

#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : xml_add_child()");
#endif
	return(child);
}

xmlNode *xml_get_child(xmlNode *node){
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_get_child()");
#endif
	return(node->children);
}

xmlNode *xml_get_next(xmlNode *node){
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_get_next()");
#endif
	return(node->next);
}

gchar *xml_get_content(xmlNode *node){
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_get_content()");
#endif
	if(((NODE_DATA *)(node->data))->content)
		return(((NODE_DATA *)(node->data))->content);
	else
		return(g_strdup(""));
}

gchar *xml_get_name(xmlNode *node){
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_get_name()");
#endif

	return(((NODE_DATA *)(node->data))->name);
}


static void xml_destroy_tree_internal(GNode *node, gpointer data){
	NODE_DATA *node_data;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_destroy_tree_internal()");
#endif
	node_data = (NODE_DATA *)(node->data);

	if(G_NODE_IS_LEAF(node)){
		if(node_data->name)
			g_free(node_data->name);
		if(node_data->content)
			g_free(node_data->content);
		if(node_data->attr != NULL){
			GList *list;
			NODE_ATTR *attr;
			list = g_list_first(node_data->attr);
			while(list){
				attr = (NODE_ATTR *)(list->data);
				if(attr->name)
					g_free(attr->name);
				if(attr->value)
					g_free(attr->value);
				g_free(attr);
				list = g_list_next(list);
			}
		}
	} else {
		g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)xml_destroy_tree_internal, (gpointer)NULL);
	}

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : xml_destroy_tree_internal()");
#endif
}


xmlResult xml_destroy_document(xmlDoc *doc){

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_destroy_document()");
#endif

	if(doc == NULL) {
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : xml_destroy_document() : NOP");
#endif
		return(XML_OK);
	}

	xml_destroy_tree_internal(doc->root, NULL);
	g_node_destroy((GNode *)doc->root);
	g_free(doc->version);
	g_free(doc->encoding);
	g_free(doc);
	
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : xml_destroy_document()");
#endif
	return(XML_OK);
}


gchar *special_to_encoded(gchar *text){
	gchar buff[65536];
	gchar *p;
	gint i;
	gint j;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : special_to_encoded()");
#endif

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

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : special_to_encoded()");
#endif

	return(g_strdup(buff));
}

gchar *encoded_to_special(gchar *text){
	gchar buff[65536];
	gchar *p;
	gint i;
	gint j;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : encoded_to_special()");
#endif

	p = text;
	j = 0;

	while(*p){

		if(*p == '&'){
			for(i=0; ; i ++){
				if(special[i].encoded == NULL) {
					buff[j] = *p;
					j++;
					p++;
					break;
				}
				if(strstr(p, special[i].encoded) == p){
					buff[j] = special[i].special;
					j ++;
					p += strlen(special[i].encoded);
				}
			}
		} else {
			buff[j] = *p;
			j++;
			p++;
		}
	}

	buff[j] = '\0';

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : encoded_to_special()");
#endif

	return(g_strdup(buff));
}


gchar *xml_get_attr(xmlNode *node, gchar *name){

	GList *list;
	NODE_ATTR *attr;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_get_attr(%s)", name);
#endif


	list = ((NODE_DATA *)(node->data))->attr;

	while(list){
		attr = (NODE_ATTR *)(list->data);
		if(strcmp(attr->name, name) == 0){

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : xml_get_attr()");
#endif
			return(attr->value);
		}
		list = g_list_next(list);
	}

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : xml_get_attr() : NULL");
#endif
	return(NULL);
}

xmlResult xml_set_attr(xmlNode *node, gchar *name, gchar *value){

	NODE_ATTR *attr;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : xml_set_attr(%s, %s)", name, value);
#endif

	attr = (NODE_ATTR *)calloc(sizeof(NODE_ATTR), 1);
	attr->name = g_strdup(name);
	attr->value = g_strdup(value);

	((NODE_DATA *)(node->data))->attr  = g_list_append(((NODE_DATA *)(node->data))->attr, attr);

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : xml_set_attr()");
#endif
	return(XML_OK);
}


xmlResult parse_attribute(GNode *node, gchar *tag){
	gchar *p, *p2, *p3;
	NODE_ATTR *attr;
	gint end;
	gint quoted;
	gchar *tmp_val;
	gchar *special_str;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : parse_attribute(%s)", tag);
#endif

	p = strchr(tag, ' ');
	if(p == NULL){
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : parse_attribute()");
#endif
		return(XML_OK);
	}

	
	p++;

	while(1){
		p2 = strchr(p, '=');
		if(p2 == NULL){
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : parse_attribute()");
#endif
			return(XML_OK);
		}

		attr = (NODE_ATTR *)calloc(sizeof(NODE_ATTR), 1);
		attr->name = g_strndup(p, p2 - p);
		p2 ++;

		quoted = 0;
		if(*p2 == '\"'){
			quoted = 1;
			p2++;
		}

		p3 = p2;
		while(1){
			if(quoted) {
				if(*p3 == '\"') {
					end = 0;
					break;
				}
				if((*p3 == '\0') || (*p3 == '>')) {
					end = 1;
					break;
				}
			} else {
				if((*p3 == ' ') || (*p3 == '\"')) {
					end = 0;
					break;
				}
				if((*p3 == '\0') || (*p3 == '>')) {
					end = 1;
					break;
				}
			}
			p3 ++;
		}
		tmp_val = g_strndup(p2, p3 - p2);
		special_str = encoded_to_special(tmp_val);
		attr->value = 
			iconv_convert(
				((NODE_DATA *)(node->data))->doc->encoding,
				"UTF-8", 
				special_str);
		g_free(special_str);

		g_free(tmp_val);
		((NODE_DATA *)node->data)->attr = g_list_append(((NODE_DATA *)node->data)->attr, attr);
		if(end) {
#ifdef XML_TRACE
			LOG(LOG_DEBUG, "OUT : parse_attribute()");
#endif
			return(XML_OK);
		}
		else {
			p3 ++;
			p = p3 + 1;
		}
	}

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : parse_attribute()");
#endif
}

void parse_declaration(GNode *node, gchar *tag)
{
	gchar attr[512];
	xmlDoc *doc;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : parse_declaration()");
#endif
	g_assert(node != NULL);
	doc = ((NODE_DATA *)(node->data))->doc;
	g_assert(doc != NULL);
	get_attr(tag, "version", attr);
	doc->version = g_strdup(attr);
	get_attr(tag, "encoding", attr);
	doc->encoding = g_strdup(attr);

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : parse_declaration()");
#endif


}

xmlResult parse_buffer(GNode *parent, gchar *text, guint length)
{
	gchar *p;
	gchar start_tag[512];
	gchar tag_name[512];
	gchar body[65536];
	gchar *content;
	gint  content_length;
	gint  body_length;
	GNode *node;
	gboolean no_end_tag;
	gchar *special_str;

	g_assert(text != NULL);

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : parse_buffer()");
#endif

	body_length = 0;
	p = text;

	while((p - text) <  length){
		if(*p == '<'){
			if(body_length != 0){
				((NODE_DATA *)(parent->data))->content = encoded_to_special(body);
				body[0] = '\0';
				body_length = 0;
			}
			
			no_end_tag = FALSE;

			get_start_tag(p, start_tag);

			// <xxx/>の場合には対応するエンドタグがない
			if(start_tag[strlen(start_tag) - 1] == '/'){
				start_tag[strlen(start_tag) - 1] = '\0';
				no_end_tag = TRUE;
			}
			
			get_tag_name(start_tag, tag_name);

			// 宣言部分
			if(start_tag[0] == '?'){
				if(start_tag[strlen(start_tag) - 1] == '?'){
					start_tag[strlen(start_tag) - 1] = '\0';
				}
				parse_declaration(parent, &start_tag[1]);
				skip_start_tag(&p, tag_name);
				continue;
			}

			node = xml_add_child(parent, tag_name, NULL);
			parse_attribute(node, start_tag);

			if(no_end_tag == FALSE){
				get_content(p, tag_name, &content, &content_length);
				parse_buffer(node, content, content_length);
				skip_end_tag(&p, tag_name);
			} else {
				skip_start_tag(&p, tag_name);
			}

		} else if (*p == '\n') {
			p++;
		} else {
			body[body_length] = *p;
			body_length ++;
			body[body_length] = '\0';
			p++;
		}

	}

	if(body_length != 0){
		special_str = encoded_to_special(body);
		((NODE_DATA *)(parent->data))->content = 
			iconv_convert(
				((NODE_DATA *)(parent->data))->doc->encoding,
				"UTF-8", 
				special_str);
		g_free(special_str);
	}

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : parse_buffer()");
#endif
	return XML_OK;
}

