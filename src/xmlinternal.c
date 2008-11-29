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
#include "xmlinternal.h"

//#define XML_TRACE

void get_tag_name(gchar *text, gchar *tag){
	gchar *p;
	gint i;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : get_tag_name(%s)", tag);
#endif

	p = text;
	for(i=0; ; i++, p++){
		tag[i] = '\0';
		if((*p == ' ') || (*p == '>') || (*p == '\0'))
			break;
		tag[i] = *p;
	}

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_tag_name()");
#endif

}

// Extracts start tag. "<" or ">" will not be included.
void get_start_tag(gchar *text, gchar *tag){
	gchar *p;
	gint i;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : get_start_tag(%s)", tag);
#endif

	p = strchr(text, '<');
	if(p == NULL){
		LOG(LOG_INFO, "get_start_tag: format error");
		tag[0] = '\0';
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_start_tag()");
#endif
		return;
	}

	// Start tag will end by ">"
	p++;
	for(i=0; ; i++, p++){
		if(i == 511)
			break;
		if((*p == '\0') || (*p == '<')){
			tag[0] = '\0';
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_start_tag()");
#endif
			return;
		}
		tag[i] = '\0';
		if(*p == '>')
			break;
		tag[i] = *p;
	}

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_start_tag()");
#endif
}

// Extracts end tag. "<", ">" or "/" will not be included
void get_end_tag(gchar *text, gchar *tag_name, gchar *tag){
	gchar buff[512];
	gchar *p;
	gint i;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : get_end_tag(%s)", tag_name);
#endif

	sprintf(buff, "</%s",tag_name);

	p = strstr(text, buff);
	if(p == NULL){
		LOG(LOG_INFO, "get_end_tag: format error tag_name=%s", tag_name);
		tag[0] = '\0';
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_end_tag()");
#endif
		return;
	}

	// End tag stops at ">"
	p = p + 2;
	for(i=0; ; i++, p++){
		tag[i] = '\0';
		if(*p == '>')
			break;
		tag[i] = *p;
	}
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_end_tag()");
#endif
}

void get_content(gchar *text, gchar *tag_name, gchar **content, gint *content_length){
	gchar buff[512];
	gchar *p;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : get_content(%s)",tag_name);
#endif

	sprintf(buff, "<%s", tag_name);

	p = strstr(text, buff);
	if((p == NULL) || (p != text)){
		LOG(LOG_INFO, "get_content: format error");
		*content = NULL;
		*content_length = 0;
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : get_content()");
#endif
		return;
	}

	// Find the end of the start tag.
	while(1){
		if(*p == '>')
			break;
		p++;
	}

	p++;
	*content = p;


	// Find end tag.
	sprintf(buff, "</%s", tag_name);


	while(1){
		if(*p == '\0'){
			*content_length = p - *content;
			break;
		}

		if(*p == '<'){
			// End tag found.
			if(*(p+1) == '/') {
				// Target end tag?
				if(strstr(p, buff) == p){
					*content_length = p - *content;
					break;
				} else {
				// Otherwise, format maynot be valid.
				}
			} else if(isalpha(*(p+1))) {
				// Sart tag found
				gchar start_tag[512];
				gchar tag_name[512];

				get_start_tag(p, start_tag);
				if(strlen(start_tag) == 0) {
					// Not start tag.
					p++;
					continue;
				}
				
				get_tag_name(start_tag, tag_name);
				skip_end_tag(&p, tag_name);
				
				continue;
			}
		}
		p++;
	}
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_content()");
#endif
}




void get_attr(const gchar *tag, gchar *name, gchar *value){
	gchar buff[512];
	gchar *p;
	gint i;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : get_attr(%s)", name);
#endif
	sprintf(buff, "%s=", name);

	p = strstr(tag, buff);
	if(p == NULL){
		LOG(LOG_DEBUG, "get_attr: attribute %s not found in %s", name, tag);
		value[0] = '\0';
#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_attr()");
#endif
		return;
	}

	p = p + strlen(name)+1;
	if(*p == '\"')
		p++;
	for(i=0; ; i++, p++){
		value[i] = '\0';
		if((*p == '\0') || (*p == ' ') || (*p == '>') || (*p == '\"'))
			break;
		value[i] = *p;
	}

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "OUT : get_attr()");
#endif
}

void skip_start_tag(gchar **text, gchar *tag_name){
	gchar buff[512];
	gchar *p;

#ifdef XML_TRACE
	LOG(LOG_DEBUG, "IN : skip_start_tag(%s)", tag_name);
#endif
	sprintf(buff, "<%s", tag_name);

	p = strstr(*text, buff);
	if(p == NULL){
		LOG(LOG_INFO, "skip_start_tag: format error.");
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : skip_start_tag()");
#endif
		return;
	}

	while(1){
		if(*p == '\0'){
			*text = p;
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : skip_start_tag()");
#endif
			return;
		}
		if(*p == '>')
			break;
		p++;
	}

	p++;
	*text = p;
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : skip_start_tag()");
#endif
}

void skip_end_tag(gchar **text, gchar *tag_name){
	gchar *content;
	gint length;
	gchar buff[512];
	gchar *p;

#ifdef XML_TRACE
		LOG(LOG_DEBUG, "IN : skip_end_tag(%s)", tag_name);
#endif

	sprintf(buff, "</%s", tag_name);

	p = strstr(*text, buff);
	if(p == NULL){
		LOG(LOG_INFO, "skip_end_tag: format error.");
		// Because there may not be end tag, skip start tag.
		skip_start_tag(text, tag_name);
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : skip_end_tag()");
#endif
		return;
	}


	get_content(*text, tag_name, &content, &length);
	p = content + length;
	
	while(1){
		if(*p == '\0'){
			*text = p;
#ifdef XML_TRACE
			LOG(LOG_DEBUG, "OUT : skip_end_tag()");
#endif
			return;
		}
		if(*p == '>')
			break;
		p++;
	}

	p++;
	*text = p;
#ifdef XML_TRACE
		LOG(LOG_DEBUG, "OUT : skip_end_tag()");
#endif
}

