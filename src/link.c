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

#include "dump.h"
#include "eb.h"
#include "external.h"
#include "history.h"
#include "mainwindow.h"

static GList *link_list=NULL;

void set_link(TAG *tag)
{
	TAG *t;

	LOG(LOG_DEBUG, "IN : set_link(type=%d, start=%d, end=%d)", tag->type, tag->start, tag->end);

	g_assert(tag != NULL);

	t = calloc(sizeof(TAG), 1);
	t->type = tag->type;
	t->start = tag->start;
	t->end = tag->end;
	t->page = tag->page;
	t->offset = tag->offset;
	t->size = tag->size;
	if(strlen(tag->filename) <= 255)
		strcpy(t->filename, tag->filename);

	link_list = g_list_append(link_list, t);

	LOG(LOG_DEBUG, "OUT : set_link()");
}


void clear_link()
{
	GList *item;

	LOG(LOG_DEBUG, "IN : clear_link()");

	item = g_list_first(link_list);
	while(item){
		g_free(item->data);
		item = g_list_next(item);
	}
	g_list_free(link_list);
	link_list = NULL;

	LOG(LOG_DEBUG, "OUT : clear_link()");
}

TAG *scan_link(guint offset)
{
	GList *item;
	TAG *tag;

	//	LOG(LOG_DEBUG, "IN : scan_link(%d)", offset);

	item = g_list_first(link_list);
	while(item){
		tag = (TAG *)(item->data);
		if((tag->start <= offset) && 
		   (offset <= tag->end)){
			LOG(LOG_DEBUG, "OUT : scan_link() = found");
			return(tag);
		}
		item = g_list_next(item);
	}

	//	LOG(LOG_DEBUG, "OUT : scan_link()");
	return(NULL);
}

static gchar *lastfile=NULL;
static gint count=0;

gboolean follow_link(guint offset)
{
	TAG *tag;
	gchar filename[512];
	RESULT result;

	LOG(LOG_DEBUG, "IN : follow_link(%d)", offset);

	tag = scan_link(offset);

	if(tag){

		if(tag->type & TAG_TYPE_LINK){
			result.type = RESULT_TYPE_EB;
			result.heading = NULL;
			result.word = NULL;
			result.data.eb.book_info = current_result->data.eb.book_info;
			result.data.eb.pos_text.page = tag->page;
			result.data.eb.pos_text.offset = tag->offset;
			result.data.eb.plain_heading = NULL;
			result.data.eb.dict_title = NULL;

			show_result(&result, TRUE, FALSE);
		} else if (tag->type & TAG_TYPE_SOUND){
			sprintf(filename, "%s%sebview-%d-%d.wav", temp_dir, DIR_DELIMITER, getpid(), count++);
			ebook_output_wave(current_result->data.eb.book_info, 
					  filename, 
					  tag->page,
					  tag->offset,
					  tag->size);
			play_multimedia(filename, TAG_TYPE_SOUND);

			if(lastfile){
				unlink(lastfile);
				g_free(lastfile);
			}
			lastfile = strdup(filename);
					
			// Link to movie
		} else if (tag->type & TAG_TYPE_MOVIE){
			sprintf(filename, "%s%sebview-%d-%d.mpg", temp_dir, DIR_DELIMITER, getpid(), count++);
			unlink(filename);
			ebook_output_mpeg(current_result->data.eb.book_info, 
					  tag->filename,
					  filename);
			play_multimedia(filename, TAG_TYPE_MOVIE);

			if(lastfile){
				unlink(lastfile);
				g_free(lastfile);
			}
			lastfile = strdup(filename);
		}

		LOG(LOG_DEBUG, "OUT : follow_link() = TRUE");
		return(TRUE);
	}

	LOG(LOG_DEBUG, "OUT : follow_link() = FALSE");
	return(FALSE);
}
