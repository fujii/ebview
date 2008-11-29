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
#include "dump.h"
#include "history.h"
#include "mainwindow.h"
#include "pref_io.h"

GList *history_list = NULL;
GList *current_in_history = NULL;
GList *word_history=NULL;
GList *directory_history=NULL;

extern gint skip_result;

void history_back()
{
	RESULT *result;
	GList *previous;

	LOG(LOG_DEBUG, "IN : history_back()");

	if(current_in_history == NULL) {
		LOG(LOG_DEBUG, "OUT : history_back() = nop1");
		return;
	}

	previous = g_list_previous(current_in_history);
	if(previous == NULL){
		LOG(LOG_DEBUG, "OUT : history_back() = nop2");
		return;
	}
	
	result = (RESULT *)(previous->data);
	g_assert(result != NULL);

	show_result(result, FALSE, FALSE);
	
	current_in_history = previous;

	LOG(LOG_DEBUG, "OUT : history_back()");
}

void history_forward()
{
	RESULT *result;
	GList *next;

	LOG(LOG_DEBUG, "IN : history_forward()");

	if(current_in_history == NULL){
		LOG(LOG_DEBUG, "OUT : history_forward() = nop1");
		return;
	}

	next = g_list_next(current_in_history);
	if(next == NULL){
		LOG(LOG_DEBUG, "OUT : history_forward() = nop2");
		return;
	}
	
	result = (RESULT *)(next->data);
	g_assert(result != NULL);

	show_result(result, FALSE, FALSE);
	
	current_in_history = next;

	LOG(LOG_DEBUG, "OUT : history_forward()");
}

void save_result_history(RESULT *rp)
{

	RESULT *result;

	GList *next;

	LOG(LOG_DEBUG, "IN : save_history()");

	g_assert(rp != NULL);

	// 現在表示内容がヒストリの最後でない場合には
	// 以降を削除する
	if(current_in_history){
		next = g_list_next(current_in_history);
		while(next){
			result = (RESULT *)(next->data);
			history_list = g_list_remove(history_list, next->data);
			free_result(result);
			next = g_list_next(current_in_history);
		}
	}

	result = duplicate_result(rp);

	history_list = g_list_append(history_list, result);
	current_in_history = g_list_last(history_list);

	LOG(LOG_DEBUG, "OUT : save_history()");
}

static GList *check_duplicate_entry(GList *list, const char *word){
	GList *l;

	l = list;
	while(l != NULL){
		if(strcmp(l->data, word) == 0)
			return(l);
		l = g_list_next(l);
	}
	return(NULL);
}

void save_word_history(const gchar *word){
	GList *list;
	gint length;

	LOG(LOG_DEBUG, "IN : save_word_history()");

	// 既にリストにあるときは、いちばん上にもってくる
	list = check_duplicate_entry(word_history, word);
	if(list){
		word_history = g_list_remove(word_history, list->data);
	}

	list = g_list_first(word_history);
	length =0;
	while(list != NULL){
		list = g_list_next(list);
		length++;
	}

	if(word_history == NULL){
		word_history = g_list_append(word_history, g_strdup(word));	
	} else if(length >= max_remember_words){
		list = g_list_nth(word_history, max_remember_words-1);
		free(list->data);
		word_history = g_list_remove(word_history, list->data);
		word_history = g_list_prepend(word_history, g_strdup(word));
	} else {
		word_history = g_list_prepend(word_history, g_strdup(word));	
	}

	gtk_combo_set_popdown_strings( GTK_COMBO(combo_word), word_history) ;

	save_history();

	LOG(LOG_DEBUG, "OUT : save_word_history()");
}


void copy_result(RESULT *to, RESULT *from)
{
	LOG(LOG_DEBUG, "IN : copy_result()");

	g_assert(from != NULL);
	g_assert(to != NULL);

	if(from->heading)
		to->heading = g_strdup(from->heading);
	if(from->word)
		to->word = g_strdup(from->word);
	to->type = from->type;

	if(from->type == RESULT_TYPE_EB){

		to->data.eb.book_info = from->data.eb.book_info;
		to->data.eb.search_method = from->data.eb.search_method;
		if(from->data.eb.plain_heading)
			to->data.eb.plain_heading = g_strdup(from->data.eb.plain_heading);
		if(from->data.eb.dict_title)
			to->data.eb.dict_title = g_strdup(from->data.eb.dict_title);
		to->data.eb.pos_heading = from->data.eb.pos_heading;
		to->data.eb.pos_text = from->data.eb.pos_text;

	} else 	if(from->type == RESULT_TYPE_GREP){
		if(from->data.grep.filename)
			to->data.grep.filename = g_strdup(from->data.grep.filename);
		to->data.grep.page = from->data.grep.page;
		to->data.grep.line = from->data.grep.line;
		to->data.grep.offset = from->data.grep.offset;
	} else {
		LOG(LOG_INFO, "copy_result : Unknown type %d", from->type);
	}

	LOG(LOG_DEBUG, "OUT : copy_result()");
}

RESULT *duplicate_result(RESULT *rp)
{
	RESULT *result;

	LOG(LOG_DEBUG, "IN : duplicate_result()");
	
	g_assert(rp != NULL);

	result = g_new0(RESULT, 1);
	copy_result(result, rp);

	LOG(LOG_DEBUG, "OUT : duplicate_result()");

	return(result);
}

void set_current_result(RESULT *rp)
{
	RESULT *old;

	LOG(LOG_DEBUG, "IN : set_current_result()");

	if(current_result == rp){
		LOG(LOG_DEBUG, "OUT : set_current_result() = NOP");
		return;
	}

	old = current_result;

	if(rp != NULL)
		current_result = duplicate_result(rp);
	else
		current_result = NULL;

	if(old != NULL)
		free_result(old);

	LOG(LOG_DEBUG, "OUT : set_current_result()");
}

void free_result(RESULT *rp)
{

	LOG(LOG_DEBUG, "IN : free_result()");

	if(rp->heading)
		g_free(rp->heading);
	if(rp->word)
		g_free(rp->word);
	if(rp->type == RESULT_TYPE_EB){
		if(rp->data.eb.plain_heading)
			g_free(rp->data.eb.plain_heading);
		if(rp->data.eb.dict_title)
			g_free(rp->data.eb.dict_title);
	} else 	if(rp->type == RESULT_TYPE_GREP){
		if(rp->data.grep.filename)
			g_free(rp->data.grep.filename);
	} else {
		LOG(LOG_INFO, "free_result : Unknown type %d", rp->type);
	}
	g_free(rp);

	LOG(LOG_DEBUG, "OUT : free_result()");
}

void clear_search_result()
{
	GList *l;

	LOG(LOG_DEBUG, "IN : clear_search_result()");

	if(!search_result)
		return;

	l = g_list_first(search_result);
		free_result((RESULT *)(l->data));
	while(l != NULL){
		l = g_list_next(l);
	}
	g_list_free(search_result);
	search_result = NULL;
	skip_result = 0;

	LOG(LOG_DEBUG, "OUT : clear_search_result()");
}

