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

#include "bmh.h"
#include "eb.h"
#include "jcode.h"
#include "link.h"
#include "xml.h"
#include "xmlinternal.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#define IMAGE_TYPE_JPEG          1
#define IMAGE_TYPE_COLOR_BMP     2
#define IMAGE_TYPE_MONO_BMP      3
#define IMAGE_TYPE_GRAY_BMP      4

gint calculate_gaiji_size(gint height){
	gint size=16;

	if(height < 24)
		size = 16;
	else if(height < 30)
		size = 24;
	else if(height < 48)
		size = 30;
	else
		size = 48;


	return(size);
}

GdkPixbuf *load_xbm(BOOK_INFO *binfo, gchar *name, gint *w, gint *h, gchar *color){
	gint width, height;
	gchar **data = NULL;
	GList **gaiji_cache=NULL;
	GList *gaiji_item;
	GAIJI_CACHE *gaiji_p=NULL;
	gint found=0;
	gint char_no;
	
	GdkPixbuf *pixbuf;

	gint size;

	char_no = strtol(&name[1], NULL, 16);
	found = 0;

	size = calculate_gaiji_size(font_height);
	size = check_gaiji_size(binfo, size);

	if(name[0] == 'h'){
		switch(size){
		case 16:
			gaiji_cache = &(binfo->gaiji_narrow16);
			break;
		case 24:
			gaiji_cache = &(binfo->gaiji_narrow24);
			break;
		case 30:
			gaiji_cache = &(binfo->gaiji_narrow30);
			break;
		case 48:
			gaiji_cache = &(binfo->gaiji_narrow48);
			break;
		}
	} else {
		switch(size){
		case 16:
			gaiji_cache = &(binfo->gaiji_wide16);
			break;
		case 24:
			gaiji_cache = &(binfo->gaiji_wide24);
			break;
		case 30:
			gaiji_cache = &(binfo->gaiji_wide30);
			break;
		case 48:
			gaiji_cache = &(binfo->gaiji_wide48);
			break;
		}
	}

	if(gaiji_cache == NULL){
		LOG(LOG_INFO, "gaiji_chache == NULL");
		return(NULL);
	}

	gaiji_item = g_list_first(*gaiji_cache);
	while(gaiji_item != NULL){
		gaiji_p = gaiji_item->data;
		if(gaiji_p->code == char_no){
			found = 1;
			break;
		}
		gaiji_item = g_list_next(gaiji_item);
	}

	if(found){

		data = gaiji_p->data;
		width = gaiji_p->width;
		height = gaiji_p->height;

		// Rewrite it since you cannot tell which color is in cache
		g_free(data[2]);
		if(color == NULL)
			data[2] = g_strdup_printf(". 	c Black");
		else
			data[2] = g_strdup_printf(". 	c %s", color);



		pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)data);
	} else {

		data = read_gaiji_as_xpm(binfo, name, size, &width, &height, color);
		if(data == NULL){
			LOG(LOG_CRITICAL, "failed to read gaiji : %s", name);
			return(NULL);
		}
		pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)data);
		gaiji_p = (GAIJI_CACHE *)calloc(sizeof(GAIJI_CACHE), 1);
		if(gaiji_p == NULL){
			LOG(LOG_ERROR, "No memory");
			exit(1);
		}
		gaiji_p->code = char_no;
		gaiji_p->data = data;
		gaiji_p->width = width;
		gaiji_p->height = height;

		*gaiji_cache = g_list_append(*gaiji_cache, gaiji_p);

	}

	*w = width;
	*h = height;

	return(pixbuf);
}

static void draw_string2(CANVAS *canvas, DRAW_TEXT *text, TAG *tag)
{
	gchar *euc_str;
	gchar *utf_str;

	gint tag_count=0;
	GtkTextTag *tags[8];

	//LOG(LOG_DEBUG, "IN : draw_string2()");

	euc_str = g_strndup(text->text, text->length);
	utf_str = iconv_convert("euc-jp", "utf-8", euc_str);

	if(tag == NULL){
		if((0 <= canvas->indent)  && (canvas->indent < MAX_INDENT)){
			gtk_text_buffer_insert_with_tags(
				canvas->buffer, canvas->iter, 
				utf_str, -1,
				tag_plain, tag_indent[canvas->indent], NULL);
		} else {
			gtk_text_buffer_insert_with_tags(
				canvas->buffer, canvas->iter, 
				utf_str, -1,
				tag_plain, NULL);
		}
		goto END;
	}

	tag->start = gtk_text_iter_get_offset(canvas->iter);

	if((!(tag->type & TAG_TYPE_EMPHASIS)) && 
	   (!(tag->type & TAG_TYPE_ITALIC)) && 
	   (!(tag->type & TAG_TYPE_SUPERSCRIPT)) && 
	   (!(tag->type & TAG_TYPE_SUBSCRIPT)))
		tags[tag_count++] = tag_plain;


	if(tag->type & TAG_TYPE_KEYWORD){
		tags[tag_count++] = tag_keyword;
	}

	if(tag->type & TAG_TYPE_EMPHASIS){
		tags[tag_count++] = tag_bold;
	}

	if(tag->type & TAG_TYPE_ITALIC){
		tags[tag_count++] = tag_italic;
        }

	if(tag->type & TAG_TYPE_SUBSCRIPT){
		tags[tag_count++] = tag_subscript;
	}

        if(tag->type & TAG_TYPE_SUPERSCRIPT){
		tags[tag_count++] = tag_superscript;
	}

	if(tag->type & TAG_TYPE_CENTER){
		tags[tag_count++] = tag_center;
	}

	if(tag->type & TAG_TYPE_LINK){
		tags[tag_count++] = tag_link;
	}

	if(tag->type & TAG_TYPE_SOUND){
		tags[tag_count++] = tag_sound;
	}

	if(tag->type & TAG_TYPE_MOVIE){
		tags[tag_count++] = tag_movie;
	}

	if(tag->type & TAG_TYPE_COLORED){
		if((!(tag->type & TAG_TYPE_KEYWORD)) && 
		   (!(tag->type & TAG_TYPE_LINK)) && 
		   (!(tag->type & TAG_TYPE_SOUND)) && 
		   (!(tag->type & TAG_TYPE_MOVIE)))
		tags[tag_count++] = tag_colored;
	}

	if((0 <= canvas->indent)  && (canvas->indent < MAX_INDENT)){
		tags[tag_count++] = tag_indent[canvas->indent];
	}

	if(tag_count > 8){
		LOG(LOG_INFO, "Too many nested tags. Truncated to 8.");
		tag_count = 8;
	}

	switch(tag_count){
	case 1:
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			utf_str, -1,
			tags[0], NULL);
		break;
	case 2:
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			utf_str, -1,
			tags[0], tags[1], NULL);
		break;
	case 3:
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			utf_str, -1,
			tags[0], tags[1], tags[2], NULL);
		break;
	case 4:
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			utf_str, -1,
			tags[0], tags[1], tags[2], tags[3], NULL);
		break;
	case 5:
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			utf_str, -1,
			tags[0], tags[1], tags[2], tags[3], tags[4], NULL);
		break;
	case 6:
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			utf_str, -1,
			tags[0], tags[1], tags[2], tags[3], tags[4], tags[5], NULL);
		break;
	case 7:
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			utf_str, -1,
			tags[0], tags[1], tags[2], tags[3], tags[4], tags[5], tags[6], NULL);
		break;
	case 8:
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			utf_str, -1,
			tags[0], tags[1], tags[2], tags[3], tags[4], tags[5], tags[6], tags[7], NULL);
		break;
	}

	tag->end = gtk_text_iter_get_offset(canvas->iter);

	if((tag->type & TAG_TYPE_LINK) || (tag->type & TAG_TYPE_SOUND) ||
	   (tag->type & TAG_TYPE_MOVIE)){
		set_link(tag);
	}
 END:

	g_free(euc_str);
	g_free(utf_str);

	//LOG(LOG_DEBUG, "OUT : draw_string2()");

}

static void draw_string(CANVAS *canvas, DRAW_TEXT *text, TAG *tag, gchar *word)
{
	gchar *p;
	gchar *p0;
	gchar *r;
	TAG l_tag;
	DRAW_TEXT l_text;
	gint len;

	//LOG(LOG_DEBUG, "IN : draw_string(word=%s)", word);

	l_tag.type=0;
	l_tag.page=0;
	l_tag.offset=0;
	l_tag.size=0;

	if((word == NULL) || (bemphasize_keyword == FALSE))
		draw_string2(canvas, text, tag);
	else {
		if(tag){
			l_tag = *tag;
			l_tag.type = TAG_TYPE_COLORED | tag->type;
		} else {
			l_tag.type = TAG_TYPE_COLORED;
		}

		p = text->text;
		p0 = text->text;
		len = strlen(word);
		while(p - text->text < text->length) {
			r = simple_search(word, p, len, TRUE);
			if(r == p){
				if(p0 != p){
					l_text.text = p0;
					l_text.length = r - p0;
					draw_string2(canvas, &l_text, tag);
				}
				l_text.text = p;
				l_text.length = len;
				draw_string2(canvas, &l_text, &l_tag);
				p0 = p + len;
				p = p + len;
				continue;
			}
			
			// For Japanese keyword
			if(isascii(*p))
				p++;
			else
				p += 2;
		}

		if(p0 != p){
			l_text.text = p0;
			l_text.length = p - p0;
			draw_string2(canvas, &l_text, tag);
		}

	}

	//LOG(LOG_DEBUG, "OUT : draw_string()");
}

static void draw_gaiji(CANVAS *canvas, BOOK_INFO *binfo, TAG *tag, gchar *code)
{
	gint width;
	gint height;

	GdkPixbuf *pixbuf;

	GtkTextIter start_iter;
	GtkTextIter end_iter;

	gchar color[128];
	gchar *color_name;

	g_assert(canvas != NULL);
	g_assert(binfo != NULL);
	g_assert(code != NULL);

	//LOG(LOG_DEBUG, "IN : draw_gaiji()");

	if(tag) {
		tag->start = gtk_text_iter_get_offset(canvas->iter);
		start_iter = *(canvas->iter);
	}

	color_name = gtk_color_selection_palette_to_string(
		&(main_window->style->fg[GTK_STATE_NORMAL]), 1);
	strcpy(color, color_name);
	g_free(color_name);

	if(tag == NULL){
	} else if(tag->type & TAG_TYPE_LINK){
		strcpy(color, color_str[COLOR_LINK]);
	} else if(tag->type & TAG_TYPE_KEYWORD){
		strcpy(color, color_str[COLOR_KEYWORD]);
	} else if(tag->type & TAG_TYPE_SOUND){
		strcpy(color, color_str[COLOR_SOUND]);
	} else if(tag->type & TAG_TYPE_MOVIE){
		strcpy(color, color_str[COLOR_MOVIE]);
	}

	pixbuf = load_xbm(binfo, code, &width, &height, color);

	gtk_text_buffer_insert_pixbuf(
			canvas->buffer, canvas->iter,
			pixbuf);
	gdk_pixbuf_unref(pixbuf);

	end_iter = *(canvas->iter);
	start_iter = *(canvas->iter);

	if(tag){
		tag->end = gtk_text_iter_get_offset(canvas->iter);

		if(tag->type & TAG_TYPE_LINK){
			gtk_text_buffer_apply_tag(canvas->buffer, tag_link, &start_iter, &end_iter);
			set_link(tag);
		} else if(tag->type & TAG_TYPE_SOUND){
//			gtk_text_buffer_apply_tag(canvas->buffer, tag_sound, &start_iter, &end_iter);
			set_link(tag);
		} else if(tag->type & TAG_TYPE_MOVIE){
//			gtk_text_buffer_apply_tag(canvas->buffer, tag_movie, &start_iter, &end_iter);
			set_link(tag);
		}
	}

	gtk_text_iter_backward_char(&start_iter);

	gtk_text_buffer_apply_tag(canvas->buffer, tag_gaiji, &start_iter, &end_iter);
	if((0 <= canvas->indent)  && (canvas->indent < MAX_INDENT)){
		gtk_text_buffer_apply_tag(canvas->buffer, tag_indent[canvas->indent], &start_iter, &end_iter);
	}

}

gint image_count=0;
static void draw_graphic(CANVAS *canvas, BOOK_INFO *binfo, gint type, gint page, gint offset, gint width, gint height)
{
	char filename[512];
	GdkPixbuf *pixbuf;
	EB_Error_Code error_code=EB_SUCCESS;

	GtkTextIter start_iter;
	GtkTextIter end_iter;

	//LOG(LOG_DEBUG, "IN : draw_graphic()");

	g_assert(canvas != NULL);
	g_assert(binfo != NULL);

	if(bshow_image != TRUE){
		//LOG(LOG_DEBUG, "OUT : draw_graphic() = NOP");
		return;
	}

	// Save to file

	sprintf(filename, "%s%s%d-%d.img", temp_dir, DIR_DELIMITER, getpid(), image_count);
	image_count++;

	switch(type){
	case IMAGE_TYPE_COLOR_BMP:
	case IMAGE_TYPE_JPEG:
		error_code = ebook_output_color(binfo, filename, page, offset);
		break;
	case IMAGE_TYPE_MONO_BMP:
		error_code = ebook_output_mono(binfo, filename, page, offset, width, height);
		break;
	case IMAGE_TYPE_GRAY_BMP:
		error_code = ebook_output_gray(binfo, filename, page, offset, width, height);
		break;
	}

	if(error_code != EB_SUCCESS){
		return;
	}

	// Newline
	gtk_text_buffer_insert(canvas->buffer, canvas->iter, "\n", 1);
	
	// Put space before graphics because indent is not effective to graphics.
	if((gtk_text_iter_get_line_offset(canvas->iter) == 0) &&
	    (0 <= canvas->indent)  && (canvas->indent < MAX_INDENT)){
		gtk_text_buffer_insert_with_tags(
			canvas->buffer, canvas->iter, 
			" ", -1,
			tag_plain, tag_indent[canvas->indent], NULL);
	}

	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	if(pixbuf == NULL){
		LOG(LOG_CRITICAL, "Failed to load image file : %s", filename);
		goto END;
	}

	gtk_text_buffer_insert_pixbuf(
			canvas->buffer, canvas->iter,
			pixbuf);
	gdk_pixbuf_unref(pixbuf);

	end_iter = *(canvas->iter);
	start_iter = *(canvas->iter);

/*
	if((0 <= canvas->indent)  && (canvas->indent < MAX_INDENT)){
		gtk_text_buffer_apply_tag(canvas->buffer, tag_indent[canvas->indent], &start_iter, &end_iter);
	}
*/
	// Newline
//	gtk_text_buffer_insert(canvas->buffer, canvas->iter, "\n", 1);

 END:
	unlink(filename);
	//LOG(LOG_DEBUG, "OUT : draw_graphic()");

}



void draw_content(CANVAS *canvas, DRAW_TEXT *text, BOOK_INFO *binfo, TAG *tag, gchar *word){
	gchar *p;
	gchar start_tag[512];
	gchar end_tag[512];
	gchar tag_name[512];
	gchar attr[512];
	gchar code[16];
	gchar body[65536];
	gchar *content;
	gint  content_length;
	gint  body_length;
	gint  l_page=0, l_offset=0, l_size=0;
	gint  l_width, l_height;
	gint  l_indent;
	TAG  l_tag;
	DRAW_TEXT l_text;


	g_assert(canvas != NULL);
	g_assert(text != NULL);
	g_assert(text->text != NULL);


	//LOG(LOG_DEBUG, "IN : draw_content()");

	l_tag.type=0;
	l_tag.page=0;
	l_tag.offset=0;
	l_tag.size=0;

	body_length = 0;
	p = text->text;

	if(text->length >= 65536){
		LOG(LOG_INFO, "Text too long. Truncated to 65535 bytes. (Original %d bytes)", text->length);
		text->length = 65535;
		text->text[65535] = '\0';
	}

	while((p - text->text) <  text->length){
		if(*p == '<'){
			if(body_length != 0){
				l_text.text = body;
				l_text.length = body_length;
				draw_string(canvas, &l_text, tag, word);
				body_length = 0;
			}

			get_start_tag(p, start_tag);
			get_tag_name(start_tag, tag_name);

			if((strcmp(tag_name, "reference") == 0) ||
			   (strcmp(tag_name, "candidate") == 0)){
				get_end_tag(p, tag_name, end_tag);

				get_attr(end_tag, "page", attr);
				l_page = strtol(attr, NULL, 16);
				get_attr(end_tag, "offset", attr);
				l_offset = strtol(attr, NULL, 16);

				get_content(p, tag_name, &content, &content_length);

				if(tag) {
					l_tag = *tag;
					l_tag.type = TAG_TYPE_LINK | tag->type;
				} else {
					l_tag.type = TAG_TYPE_LINK;
				}
				
				l_tag.page = l_page;
				l_tag.offset = l_offset;
				
				l_text.text = content;
				l_text.length = content_length;

				draw_content(canvas, &l_text, binfo, &l_tag, word);
				
				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "keyword") == 0){

				get_content(p, tag_name, &content, &content_length);

				if(tag) {
					l_tag = *tag;
					l_tag.type = TAG_TYPE_KEYWORD | tag->type;
				} else {
					l_tag.type = TAG_TYPE_KEYWORD;
				}
				
				l_text.text = content;
				l_text.length = content_length;

				draw_content(canvas, &l_text, binfo, &l_tag, word);

				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "modification") == 0){

				get_content(p, tag_name, &content, &content_length);
				
				get_attr(start_tag, "method", attr);

				{
					gchar *tmps;
					tmps = g_strndup(content, content_length);
					g_free(tmps);
				}

				if(tag) {
					l_tag = *tag;
					if(attr[0] == '1')
					  l_tag.type = TAG_TYPE_ITALIC | tag->type;
					else
					  l_tag.type = TAG_TYPE_EMPHASIS | tag->type;

				} else {
					if(attr[0] == '1')
					  l_tag.type = TAG_TYPE_ITALIC;
					else 
					  l_tag.type = TAG_TYPE_EMPHASIS;
				}
				
				l_text.text = content;
				l_text.length = content_length;

				draw_content(canvas, &l_text, binfo, &l_tag, word);

				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "gaiji") == 0){

				get_attr(start_tag, "code", code);

				draw_gaiji(canvas, binfo, tag, code);
	
				skip_start_tag(&p, tag_name);

			} else if(strcmp(tag_name, "indent") == 0){

				get_attr(start_tag, "position", attr);
				l_indent = strtol(attr, NULL, 10);
				
				// I'm not sure what the parameter to indent is.

				// Set to the specified location if immediately after the new line or it does not overrup.
/*
				if((canvas->x == h_border + canvas->indent * font_width) || 
				   (canvas->x < h_border + l_indent * font_width))
					canvas->x = h_border + l_indent * font_width;
*/
				canvas->indent = l_indent - 1;

				skip_start_tag(&p, tag_name);

			} else if(strcmp(tag_name, "emphasis") == 0){

				get_content(p, tag_name, &content, &content_length);

				l_text = *text;
				l_text.text = content;
				l_text.length = content_length;

				if(tag) {
					l_tag = *tag;
					l_tag.type = TAG_TYPE_EMPHASIS | tag->type;
				} else {
					l_tag.type = TAG_TYPE_EMPHASIS;
				}

				draw_content(canvas, &l_text, binfo, &l_tag, word);

				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "sub") == 0){

				get_content(p, tag_name, &content, &content_length);
				l_text = *text;
				l_text.text = content;
				l_text.length = content_length;

				if(tag) {
					l_tag = *tag;
					l_tag.type = TAG_TYPE_SUBSCRIPT | tag->type;
				} else {
					l_tag.type = TAG_TYPE_SUBSCRIPT;
				}

				draw_content(canvas, &l_text, binfo, &l_tag, word);

				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "sup") == 0){

				get_content(p, tag_name, &content, &content_length);
				l_text = *text;
				l_text.text = content;
				l_text.length = content_length;

				if(tag) {
					l_tag = *tag;
					l_tag.type = TAG_TYPE_SUPERSCRIPT | tag->type;
				} else {
					l_tag.type = TAG_TYPE_SUPERSCRIPT;
				}

				draw_content(canvas, &l_text, binfo, &l_tag, word);

				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "center") == 0){


				get_content(p, tag_name, &content, &content_length);
				l_text = *text;
				l_text.text = content;
				l_text.length = content_length;

				if(tag) {
					l_tag = *tag;
					l_tag.type = TAG_TYPE_CENTER | tag->type;
				} else {
					l_tag.type = TAG_TYPE_CENTER;
				}

				draw_content(canvas, &l_text, binfo, &l_tag, word);

				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "nonewline") == 0){

				skip_start_tag(&p, tag_name);

			} else if(strcmp(tag_name, "/nonewline") == 0){

				skip_start_tag(&p, tag_name);

			} else if(strcmp(tag_name, "narrow") == 0){

				skip_start_tag(&p, tag_name);

			} else if(strcmp(tag_name, "/narrow") == 0){

				skip_start_tag(&p, tag_name);

			} else if(strcmp(tag_name, "jpeg") == 0){

				get_attr(start_tag, "page", attr);
				l_page = strtol(attr, NULL, 16);
				get_attr(p, "offset", attr);
				l_offset = strtol(attr, NULL, 16);

				draw_graphic(canvas, binfo, IMAGE_TYPE_JPEG, l_page, l_offset, 0, 0);
				skip_start_tag(&p, tag_name);

			} else if(strcmp(tag_name, "bmp") == 0){

				get_attr(start_tag, "page", attr);
				l_page = strtol(attr, NULL, 16);
				get_attr(p, "offset", attr);
				l_offset = strtol(attr, NULL, 16);

				draw_graphic(canvas, binfo, IMAGE_TYPE_COLOR_BMP, l_page, l_offset, 0, 0);
				skip_start_tag(&p, tag_name);

			} else if(strcmp(tag_name, "mono") == 0){

				get_attr(start_tag, "width", attr);
				l_width = strtol(attr, NULL, 10);
				get_attr(start_tag, "height", attr);
				l_height = strtol(attr, NULL, 10);

				get_end_tag(p, tag_name, end_tag);

				get_attr(end_tag, "page", attr);
				l_page = strtol(attr, NULL, 16);
				get_attr(end_tag, "offset", attr);
				l_offset = strtol(attr, NULL, 16);

				draw_graphic(canvas, binfo, IMAGE_TYPE_MONO_BMP, l_page, l_offset, l_width, l_height);
				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "wave") == 0){

				get_end_tag(p, tag_name, end_tag);

				get_attr(end_tag, "page", attr);
				l_page = strtol(attr, NULL, 16);
				get_attr(end_tag, "offset", attr);
				l_offset = strtol(attr, NULL, 16);
				get_attr(end_tag, "size", attr);
				l_size = strtol(attr, NULL, 10);

				get_content(p, tag_name, &content, &content_length);

				if(tag) {
					l_tag = *tag;
					l_tag.type = TAG_TYPE_SOUND | tag->type;
				} else {
					l_tag.type = TAG_TYPE_SOUND;
				}

				l_tag.page = l_page;
				l_tag.offset = l_offset;
				l_tag.size = l_size;
				
				l_text.text = content;
				l_text.length = content_length;

				draw_content(canvas, &l_text, binfo, &l_tag, word);

				skip_end_tag(&p, tag_name);

			} else if(strcmp(tag_name, "mpeg") == 0){
				gchar *utf_str;
				gchar *euc_str;

				get_attr(start_tag, "filename", attr);

				if(tag) {
					l_tag = *tag;
					l_tag.type = TAG_TYPE_MOVIE | tag->type;
				} else {
					l_tag.type = TAG_TYPE_MOVIE;
				}

				l_tag.page = l_page;
				l_tag.offset = l_offset;
				l_tag.size = l_size;
				sprintf(l_tag.filename, "%s", attr);
				
				utf_str = _(" [Movie] ");
				euc_str = iconv_convert("utf-8", "euc-jp", utf_str);
				l_text.text = euc_str;
				l_text.length = strlen(l_text.text);

				draw_content(canvas, &l_text, binfo, &l_tag, word);
				g_free(euc_str);

				skip_start_tag(&p, tag_name);

			} else {
				body[body_length] = *p;
				body_length ++;
				body[body_length] = '\0';
				p++;
			}
		} else {
			body[body_length] = *p;
			body_length ++;
			body[body_length] = '\0';
			p++;
		}

	}

	if(body_length != 0){
		l_text.text = body;
		l_text.length = body_length;
		draw_string(canvas, &l_text, tag, word);
	}

	//LOG(LOG_DEBUG, "OUT : draw_content()");
}

