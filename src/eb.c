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
#include "history.h"
#include "hook.h"
#include "jcode.h"
#include "headword.h"
#include "dialog.h"
#include "xmlinternal.h"
#include "thread_search.h"
#include "dirtree.h"


#define MAX_HITS            50
#define MAXLEN_HEADING     65535
#define MAXLEN_TEXT      65535

#define EBOOK_MAX_KEYWORDS 256
#define MAX_BUFSIZE 65535

extern GList *group_list;
extern GList *book_list;
extern gint global_multi_code;

gint ebook_simple_search(BOOK_INFO *binfo, char *word, gint method, gchar *title);
static gint ebook_ending_search(BOOK_INFO *binfo, char *word, gint method, gchar *title);
static void sort_result(gchar *word);
static void plain_heading();

extern EB_Hookset text_hookset;
extern EB_Hookset heading_hookset;
extern EB_Hookset candidate_hookset;

static gboolean ebook_initialized = FALSE;

gboolean full_text_search_ignore_case = TRUE;

EB_Error_Code ebook_set_subbook(BOOK_INFO *binfo);

static gchar *ebook_message = NULL;

gchar *ebook_error_message(error_code)
{
	const gchar *message;

	if(ebook_message)
		g_free(ebook_message);

	message = eb_error_message(error_code);
	ebook_message = iconv_convert(fs_codeset, "utf-8", message);
	return(ebook_message);
}


gint ebook_search_method(){
	const char *text;
	int i;

	LOG(LOG_DEBUG, "IN : ebook_search_method()");

	text = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_method)->entry));
	for(i=0 ; search_method[i].name != 0 ; i ++){
		if(strcmp(text, search_method[i].name) == 0){
			LOG(LOG_DEBUG, "OUT : ebook_search_method()=%d", search_method[i].code);
			return(search_method[i].code);
		}
	}

	LOG(LOG_DEBUG, "OUT : ebook_search_method()=%d", SEARCH_METHOD_UNKNOWN);

	return(SEARCH_METHOD_UNKNOWN);
}

BOOK_INFO *load_book(const char *book_path, int subbook_no, gchar *appendix_path, gint appendix_subbook_no, gchar *fg, gchar *bg)
{
	EB_Error_Code error_code;
	BOOK_INFO *binfo;
	EB_Subbook_Code sublist[EB_MAX_SUBBOOKS];
	int subcount;
	char buff[512];
#if 0
	GList *book_item;
#endif

	LOG(LOG_DEBUG, "IN : load_book(%s, %d, %s, %d, %s %s)", book_path, subbook_no, appendix_path, appendix_subbook_no, fg, bg);

	if(book_path == NULL){
		LOG(LOG_DEBUG, "OUT : load_book() = NULL");
		return(NULL);
	}

	// Search if there is the same book already.
#if 0
	book_item = g_list_first(book_list);
	while(book_item != NULL){
		binfo = (BOOK_INFO *)(book_item->data);
		if((strcmp(binfo->book_path, book_path) == 0) && 
		   (binfo->subbook_no == subbook_no)) {
			if((binfo->appendix_path != NULL) &&
			   (appendix_path != NULL) &&
			   (strcmp(binfo->appendix_path, appendix_path) == 0) && 
			   (binfo->appendix_subbook_no == appendix_subbook_no)) {
				return(binfo);
			} else {
				unload_book(binfo);
				g_free(binfo);
				break;
			}
		}
		book_item = g_list_next(book_item);
	}
#endif

	binfo = (BOOK_INFO *)calloc(sizeof(BOOK_INFO),1);
	if(binfo == NULL){
		LOG(LOG_ERROR, "No memory");
		exit(1);
	}

	binfo->book_path = fs_to_unicode((gchar *)book_path);
	binfo->subbook_no = subbook_no;
	if(appendix_path){
		binfo->appendix_path = fs_to_unicode(appendix_path);
		binfo->appendix_subbook_no = appendix_subbook_no;
	}
                                                                                

	binfo->book = (EB_Book *) malloc(sizeof(EB_Book));
	eb_initialize_book(binfo->book);
	error_code = eb_bind(binfo->book, 
			     book_path);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to bind the book : %s",
			ebook_error_message(error_code));
		goto FAILED;
	}

	error_code = eb_subbook_list(
		binfo->book, 
		sublist, 
		&subcount);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to get a subbook list : %s", 
			ebook_error_message(error_code));
		goto FAILED;
	}


	error_code = eb_subbook_directory2(
		binfo->book, 
		sublist[binfo->subbook_no], 
		buff);
	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to get the directory : %s",
			ebook_error_message(error_code));
		goto FAILED;
	}
	binfo->subbook_dir = strdup(buff);

	error_code = eb_subbook_title2(
		binfo->book, 
		sublist[binfo->subbook_no], 
		buff);
	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to get the title : %s",
			ebook_error_message(error_code));
		goto FAILED;
	}
	binfo->subbook_title = iconv_convert("euc-jp", "utf-8", buff);


	if(binfo->appendix_path != NULL){
		binfo->appendix = (EB_Appendix *) malloc(sizeof(EB_Appendix));
		eb_initialize_appendix(binfo->appendix);
		
		error_code = eb_bind_appendix(binfo->appendix,
					      appendix_path);
		if(error_code != EB_SUCCESS){
			LOG(LOG_CRITICAL, "Failed to bind appendix : %s",
				ebook_error_message(error_code));
			goto FAILED;
		}
	}



	ebook_set_subbook(binfo);

	binfo->available = TRUE;

	if(eb_have_word_search(binfo->book)){
		binfo->search_method[SEARCH_METHOD_WORD] = TRUE;
	}

	if(eb_have_endword_search(binfo->book)){
		binfo->search_method[SEARCH_METHOD_ENDWORD] = TRUE;
	}

	if(eb_have_exactword_search(binfo->book)){
		binfo->search_method[SEARCH_METHOD_EXACTWORD] = TRUE;
	}
	
	if(eb_have_keyword_search(binfo->book)){
		binfo->search_method[SEARCH_METHOD_KEYWORD] = TRUE;
	}
/*	
	eb_multi_search_list(binfo->book, multi_list, &multi_count);
	for (i = 0; i < multi_count; i++) {
		binfo->search_method[SEARCH_METHOD_MULTI1+i] = TRUE;
	}
*/
	if(eb_have_multi_search(binfo->book)){
		binfo->search_method[SEARCH_METHOD_MULTI] = TRUE;
	}
	
	if(eb_have_menu(binfo->book)){
		binfo->search_method[SEARCH_METHOD_MENU] = TRUE;
	}

	if(eb_have_copyright(binfo->book)){
		binfo->search_method[SEARCH_METHOD_COPYRIGHT] = TRUE;
	}

	if (eb_have_font(binfo->book, EB_FONT_16)){
		error_code = eb_set_font(binfo->book, EB_FONT_16);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to set font : subbook=%s\n%s",
				binfo->subbook_title, 
				ebook_error_message(error_code));
		}
	}

	if(bg && (strlen(bg) != 0))
		binfo->bg = strdup(bg);

	if(fg && (strlen(fg) != 0))
		binfo->fg = strdup(fg);

	book_list = g_list_append(book_list, binfo);

	LOG(LOG_DEBUG, "OUT : load_book()");
	return(binfo);

 FAILED:
	binfo->available = FALSE;

	LOG(LOG_DEBUG, "OUT : load_book() = NULL");

	return(NULL);
}

static void free_gaiji(BOOK_INFO *binfo){
	
	GList *lists[8];
	GList *item;
	gint i;

	LOG(LOG_DEBUG, "IN : free_gaiji()");

	lists[0] = binfo->gaiji_narrow16;
	lists[1] = binfo->gaiji_narrow24;
	lists[2] = binfo->gaiji_narrow30;
	lists[3] = binfo->gaiji_narrow48;
	lists[4] = binfo->gaiji_wide16;
	lists[5] = binfo->gaiji_wide24;
	lists[6] = binfo->gaiji_wide30;
	lists[7] = binfo->gaiji_wide48;

	
	
	for(i=0; i < 8 ; i ++){
		item = g_list_first(lists[i]);
		while(item){
			free(item->data);
			item = g_list_next(item);
		}
		g_list_free(lists[i]);
	}

	binfo->gaiji_narrow16 = NULL;
	binfo->gaiji_narrow24 = NULL;
	binfo->gaiji_narrow30 = NULL;
	binfo->gaiji_narrow48 = NULL;
	binfo->gaiji_wide16 = NULL;
	binfo->gaiji_wide24 = NULL;
	binfo->gaiji_wide30 = NULL;
	binfo->gaiji_wide48 = NULL;

	LOG(LOG_DEBUG, "OUT : free_gaiji()");
}

void unload_book(BOOK_INFO *binfo)
{

	LOG(LOG_DEBUG, "IN : unload_book()");

	eb_unset_subbook(binfo->book);
	eb_finalize_book(binfo->book);
	free(binfo->book_path);
	free(binfo->appendix_path);
	free(binfo->subbook_dir);
	free(binfo->subbook_title);

	free_gaiji(binfo);

	book_list = g_list_remove(book_list, binfo);

	LOG(LOG_DEBUG, "OUT : unload_book()");
	return;
}

void check_search_method()
{

#if 0
	BOOK_INFO *binfo;
	GList *book_item;
#endif
	gint method_count=0;

	LOG(LOG_DEBUG, "IN : check_search_method()");

#if 0
	menu_word_search = FALSE;
	menu_endword_search = FALSE;
	menu_exactword_search = FALSE;
	menu_keyword_search = FALSE;
	menu_menu = FALSE;
	menu_copyright = FALSE;
	menu_multi_search = FALSE;

	book_item = g_list_first(book_list);
	while(book_item != NULL){
		binfo = (BOOK_INFO *)(book_item->data);
		if(binfo->search_method[SEARCH_METHOD_WORD] == TRUE)
			menu_word_search = TRUE;
		if(binfo->search_method[SEARCH_METHOD_ENDWORD] == TRUE)
			menu_endword_search = TRUE;
		if(binfo->search_method[SEARCH_METHOD_EXACTWORD] == TRUE)
			menu_exactword_search = TRUE;
		if(binfo->search_method[SEARCH_METHOD_KEYWORD] == TRUE)
			menu_keyword_search = TRUE;
		if(binfo->search_method[SEARCH_METHOD_MENU] == TRUE)
			menu_menu = TRUE;
		if(binfo->search_method[SEARCH_METHOD_COPYRIGHT] == TRUE)
			menu_copyright = TRUE;
		if(binfo->search_method[SEARCH_METHOD_MULTI] == TRUE)
			menu_multi_search = TRUE;

		book_item = g_list_next(book_item);
	}

	method_count = 0;

	search_method[method_count].code = SEARCH_METHOD_AUTOMATIC;
	search_method[method_count].name = strdup(_("Automatic Search"));
	method_count ++;

	if(menu_exactword_search == TRUE){
		search_method[method_count].code = SEARCH_METHOD_EXACTWORD;
		search_method[method_count].name = strdup(_("Exactword Search"));
		method_count ++;
	}

	if(menu_word_search == TRUE){
		search_method[method_count].code = SEARCH_METHOD_WORD;
		search_method[method_count].name = strdup(_("Forward Search"));
		method_count ++;
	}

	if(menu_endword_search == TRUE){
		search_method[method_count].code = SEARCH_METHOD_ENDWORD;
		search_method[method_count].name = strdup(_("Backward Search"));
		method_count ++;
	}

	if(menu_keyword_search == TRUE){
		search_method[method_count].code = SEARCH_METHOD_KEYWORD;
		search_method[method_count].name = strdup(_("Keyword Search"));
		method_count ++;
	}

	if(menu_multi_search == TRUE){
		search_method[method_count].code = SEARCH_METHOD_MULTI;
		search_method[method_count].name = strdup(_("Multiword Search"));
		method_count ++;
	}

/*
	if(menu_menu == TRUE){
		search_method[method_count].code = SEARCH_METHOD_MENU;
		search_method[method_count].name = strdup(_("Menu"));
		method_count ++;
	}

	if(menu_copyright == TRUE){
		search_method[method_count].code = SEARCH_METHOD_COPYRIGHT;
		search_method[method_count].name = strdup(_("Copyright"));
		method_count ++;
	}
*/

	search_method[method_count].code = SEARCH_METHOD_FULL_TEXT;
	search_method[method_count].name = strdup(_("Full Text Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_INTERNET;
	search_method[method_count].name = strdup(_("Internet Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_GREP;
	search_method[method_count].name = strdup(_("File Search"));
	method_count ++;


	search_method[method_count].name = NULL;

#endif

	method_count = 0;

	search_method[method_count].code = SEARCH_METHOD_AUTOMATIC;
	search_method[method_count].name = strdup(_("Automatic Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_EXACTWORD;
	search_method[method_count].name = strdup(_("Exactword Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_WORD;
	search_method[method_count].name = strdup(_("Forward Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_ENDWORD;
	search_method[method_count].name = strdup(_("Backward Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_KEYWORD;
	search_method[method_count].name = strdup(_("Keyword Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_MULTI;
	search_method[method_count].name = strdup(_("Multiword Search"));
	method_count ++;

/*
	search_method[method_count].code = SEARCH_METHOD_MENU;
	search_method[method_count].name = strdup(_("Menu"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_COPYRIGHT;
	search_method[method_count].name = strdup(_("Copyright"));
	method_count ++;
*/

	search_method[method_count].code = SEARCH_METHOD_FULL_TEXT;
	search_method[method_count].name = strdup(_("Full Text Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_INTERNET;
	search_method[method_count].name = strdup(_("Internet Search"));
	method_count ++;

	search_method[method_count].code = SEARCH_METHOD_GREP;
	search_method[method_count].name = strdup(_("File Search"));
	method_count ++;

	search_method[method_count].name = NULL;


	LOG(LOG_DEBUG, "OUT : check_search_method()");

}

gint ebook_start(){
	EB_Error_Code error_code;

	LOG(LOG_DEBUG, "IN : ebook_start()");

	error_code = eb_initialize_library();
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to initialize library : %s",
			ebook_error_message(error_code));
		return(1);
	}

	error_code = initialize_hooksets();
	if(error_code != EB_SUCCESS){
		return(1);
	}
	
	ebook_initialized = TRUE;

	LOG(LOG_DEBUG, "OUT : ebook_start()");
	return(0);

}

gint ebook_end(){

	BOOK_INFO *binfo;
	GList *book_item;

	LOG(LOG_DEBUG, "IN : ebook_end()");

	if(ebook_initialized == TRUE) {
		LOG(LOG_DEBUG, "OUT : ebook_end()");
		return(0);
	}

	book_item = g_list_first(book_list);
	while(book_item != NULL){
		binfo = (BOOK_INFO *)(book_item->data);
		eb_unset_subbook(binfo->book);
		eb_finalize_book(binfo->book);
	        free(binfo->book_path);
	        free(binfo->appendix_path);
	        free(binfo->subbook_dir);
	        free(binfo->subbook_title);
		
		free_gaiji(binfo);

		g_list_remove(book_list, book_item->data);
		book_item = g_list_next(book_item);
	}
	
	finalize_hooksets();

	book_list = NULL;
	ebook_initialized = FALSE;

	LOG(LOG_DEBUG, "OUT : ebook_end()");
	return(0);
}

void split_word(const gchar *word, gchar **keywords)
{
	gint i,j;
	gchar *p;
	gchar buff[512];
	gint quoted=0;

	LOG(LOG_DEBUG, "IN : split_word(%s)", word);

	p = (gchar *)word;
	i = 0;
	j = 0;
	keywords[0] = NULL;

	if(strlen(word) > 512){
		return;
	}
	
	while(1){
		switch(*p){
		case '\"':
			if(quoted)
				quoted = 0;
			else
				quoted = 1;
			break;
		case ' ':
		case '\t':
		case '\n':
			if(quoted){
				buff[j] = *p;
				j ++;
			} else {
				buff[j] = '\0';
				keywords[i] = strdup(buff);
				i ++;
				keywords[i] = NULL;
				j = 0;
			}
			break;
		default:
			buff[j] = *p;
			j ++;
		}
		p++;
		if(*p == '\0'){
			buff[j] = '\0';
			if(strlen(buff) != 0){
				keywords[i] = strdup(buff);
				i ++;
			}
			keywords[i] = NULL;
			break;
		} else if(i == EBOOK_MAX_KEYWORDS) {
			break;
		}
	}
	
	for(i=0; ; i++){
		if(keywords[i] == NULL)
			break;
	}
	
	LOG(LOG_DEBUG, "OUT : split_word()");
}

void cat_word(char *string, char **words){
	gint i;
	gint len = 0;

	LOG(LOG_DEBUG, "IN : cat_word()");

	sprintf(string, "%s", words[0]);	
	len = strlen(words[0]);

	for(i=1 ; words[i] != NULL ; i++){
		strcat(string, " ");
		strcat(string, words[i]);
		len = len + strlen(words[i]) + 1;
	}
	string[len] = '\0';

	LOG(LOG_DEBUG, "OUT : cat_word() = %s", string);
}

void free_words(char **words){
	gint i;

	LOG(LOG_DEBUG, "IN : free_words()");

	for(i=0; words[i] != NULL ; i++)
		free(words[i]);

	LOG(LOG_DEBUG, "OUT : free_words()");
}

static gboolean check_duplicate_hit(EB_Position pos){
	GList *l;
	RESULT *rp;

	l = search_result;
	while(l != NULL){
		rp = (RESULT *)(l->data);
		if((rp->data.eb.pos_text.page == pos.page) &&
		   (rp->data.eb.pos_text.offset == pos.offset))
			return(TRUE);
		l = g_list_next(l);
	}
	return(FALSE);
}

static gint count_result(GList *result)
{
	return(g_list_length(result));
}

EB_Error_Code ebook_my_backward_text(BOOK_INFO *binfo)
{

	EB_Error_Code error_code=EB_SUCCESS;
	EB_Position text_position;
	char data[EB_SIZE_PAGE+4];
	int i, length;

	int start_page;
	int end_page;
	int current_page;
	int current_offset;
	int offset;
	int read_page;

	int stop_code;
	

	error_code = ebook_set_subbook(binfo);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to set subbook : %s",
			ebook_error_message(error_code));
		return(error_code);
	}

	start_page = binfo->book->subbook_current->text.start_page;
	end_page = binfo->book->subbook_current->text.end_page;

	error_code = eb_tell_text(binfo->book, &text_position);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to tell text : %s",
			ebook_error_message(error_code));
		return(error_code);
	}

	current_page = text_position.page;
	current_offset = text_position.offset;

	offset = current_offset;

	stop_code = binfo->book->text_context.auto_stop_code;

	for(read_page = current_page ; read_page >= start_page ; read_page --){

		text_position.page = read_page;
		text_position.offset = 0;

		error_code = eb_seek_text(binfo->book, &text_position);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to seek text : %s",
				ebook_error_message(error_code));
			return(error_code);
		}

		memset(data, 0, EB_SIZE_PAGE + 4);

		error_code = eb_read_rawtext(binfo->book, 
					     EB_SIZE_PAGE+2, 
					     data,
					     &length);
		if (error_code != EB_SUCCESS || length != EB_SIZE_PAGE+2){
			LOG(LOG_CRITICAL, "Failed to read rawtext : %s",
				ebook_error_message(error_code));
			return(error_code);
		}

		if(stop_code != -1)
			binfo->book->text_context.auto_stop_code = stop_code;


		for(i=offset-2 ; i >= 0 ; i -=2){
			if(((binfo->appendix != NULL) &&
			    (eb_uint2(&data[i]) == binfo->appendix->subbook_current->stop_code0) &&
			    (eb_uint2(&data[i+2]) == binfo->appendix->subbook_current->stop_code1)) || 

			   ((binfo->appendix == NULL) &&
			    (eb_uint2(&data[i]) == 0x1f41) &&
			    (eb_uint1(&data[i+2]) == 0x01)) || 
			   (eb_uint2(&data[i]) == 0x1f02))
			{

				text_position.page = read_page;
				text_position.offset = i;

				error_code = eb_seek_text(binfo->book, &text_position);
				if (error_code != EB_SUCCESS) {
					LOG(LOG_CRITICAL, "Failed to seek text : %s",
						ebook_error_message(error_code));
					return(error_code);
				}

				if(stop_code != -1)
					binfo->book->text_context.auto_stop_code = stop_code;

				return(EB_SUCCESS);

			}

		}
	
		offset = EB_SIZE_PAGE + 2;
	}

	text_position.page = current_page;
	text_position.offset = current_offset;

	error_code = eb_seek_text(binfo->book, &text_position);
	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to seek text : %s",
			ebook_error_message(error_code));
		return(error_code);
	}

	return(EB_ERR_FAIL_SEEK_TEXT);

}

gint ascii_to_jisx2080_table [] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x00
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x08
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x10
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x18
	0x2121, 0x212a, 0x2140, 0x2174, 0x2170, 0x2173, 0x2175, 0x2147, // 0x20
	0x214a, 0x214b, 0x2176, 0x215c, 0x2124, 0x215d, 0x2125, 0x213f, // 0x28
	0x2330, 0x2331, 0x2332, 0x2333, 0x2334, 0x2335, 0x2336, 0x2337, // 0x30
	0x2338, 0x2339, 0x2127, 0x2128, 0x2163, 0x2161, 0x2164, 0x2129, // 0x38
	0x2177, 0x2341, 0x2342, 0x2343, 0x2344, 0x2345, 0x2346, 0x2347, // 0x40
	0x2348, 0x2349, 0x234a, 0x234b, 0x234c, 0x234d, 0x234e, 0x234f, // 0x48
	0x2350, 0x2351, 0x2352, 0x2353, 0x2354, 0x2355, 0x2356, 0x2357, // 0x50
	0x2358, 0x2359, 0x235a, 0x214e, 0x216f, 0x214f, 0x2130, 0x2132, // 0x58
	0x212e, 0x2361, 0x2362, 0x2363, 0x2364, 0x2365, 0x2366, 0x2367, // 0x60
	0x2368, 0x2369, 0x236a, 0x236b, 0x236c, 0x236d, 0x236e, 0x236f, // 0x68
	0x2370, 0x2371, 0x2372, 0x2373, 0x2374, 0x2375, 0x2376, 0x2377, // 0x70
	0x2378, 0x2379, 0x237a, 0x2150, 0x2143, 0x2151, 0x2141, 0x0000  // 0x78
};

static gchar *euc2jis(gchar *inbuf){
	guchar *euc_p;
	guchar *jisbuf=NULL;
	guchar *jis_p;

	euc_p = inbuf;
	jis_p = jisbuf = malloc(strlen(euc_p)*2);

	while(*euc_p != '\0'){
		if(( 0x20 <= *euc_p) && (*euc_p <= 0x7e) && (ascii_to_jisx2080_table[*euc_p] != 0x00)){
			*jis_p = (ascii_to_jisx2080_table[*euc_p] & 0xff00) >> 8;
			jis_p ++;
			*jis_p = ascii_to_jisx2080_table[*euc_p] & 0xff;
			jis_p ++;
		}
		else if(iseuc(euc_p)){
			*jis_p = *euc_p - 0x80;
			jis_p ++;
			euc_p++;
			*jis_p = *euc_p - 0x80;
			jis_p ++;
		}

		else if(*euc_p == 0x20){
			*jis_p = 0x21;
			jis_p ++;
			*jis_p = 0x21;
			jis_p ++;
		}

		else {
			*jis_p = 0x21;
			jis_p ++;
			*jis_p = 0x29;
			jis_p ++;
		}
			
		euc_p ++;
	}
	*jis_p = '\0';

	return(jisbuf);
}

static gint ebook_full_search_old(BOOK_INFO *binfo, char *word, gint method, gchar *title)
{

	EB_Error_Code error_code=EB_SUCCESS;
	EB_Position text_position;
	char data[EB_SIZE_PAGE];
	char *jisword;
	char *word_p;
	int i, length;
	char *p;
	char heading[MAXLEN_TEXT + 1];
	RESULT *rp;

	int start_page;
	int end_page;
	int current_page;

	int stop_code;
	int page_count=0;

	LOG(LOG_DEBUG, "IN : ebook_full_search(%s)", word);

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	start_page = binfo->book->subbook_current->text.start_page;
	end_page = binfo->book->subbook_current->text.end_page;

	// Read once in order to determine auto_stop_code
	p = ebook_get_text(binfo, start_page, 0);
	if(p)
		free(p);
	stop_code = binfo->book->text_context.auto_stop_code;

	jisword = euc2jis(word);
	word_p = jisword;

	for(current_page = start_page ; current_page <= end_page ; current_page ++){

		set_progress((float)(current_page - start_page) / (end_page - start_page + 1));
		page_count ++;

		text_position.page = current_page;
		text_position.offset = 0;

		error_code = eb_seek_text(binfo->book, &text_position);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to seek text : %s",
				ebook_error_message(error_code));
			return(error_code);
		}

		error_code = eb_read_rawtext(binfo->book, 
					     EB_SIZE_PAGE, 
					     data,
					     &length);
		if (error_code != EB_SUCCESS || length != EB_SIZE_PAGE){
			LOG(LOG_CRITICAL, "Failed to read rawtext : %s",
				ebook_error_message(error_code));
			return(error_code);
		}

		for(i=0 ; i < EB_SIZE_PAGE ; i +=2){

			// skip control characters
			if(isjisp(&data[i]) != TRUE){
				continue;

			// match
			} else if((data[i] == *word_p) && 
				  (data[i+1] == *(word_p+1))){

				// See if keyword continues
				if (*(word_p+2) != '\0'){
					word_p += 2;
					continue;
				}
			// In case ignoring upper letters and lower letters
			} else if(full_text_search_ignore_case){
				guint c1, c2;

				c1 = eb_uint2(&data[i]);
				c2 = eb_uint2(word_p);

				// Alphabet
				if ((0x2341 <= c1) && (c1 <= 0x237a) &&
				    (0x2341 <= c2) && (c2 <= 0x237a)){
					// Convert to lower and do match
					if(((0x2361 <= c1) ? (c1 - 0x20) : c1) == ((0x2361 <= c2) ? (c2 - 0x20) : c2)){
						// See if keyword continues
						if (*(word_p+2) != '\0'){
							word_p += 2;
							continue;
						}
					} else {
						goto NO_MATCH;
					}
				} else {
					goto NO_MATCH;
				}
			} else {
				goto NO_MATCH;
			}


			// Match then read

			text_position.page = current_page;
			text_position.offset = i;

			error_code = eb_seek_text(binfo->book, &text_position);
			if (error_code != EB_SUCCESS) {
				LOG(LOG_CRITICAL, "Failed to seek text : %s",
					ebook_error_message(error_code));
				// Do nothing
				goto NO_MATCH;
			}

			// Because eb_seek_text() will clear auto_stop_code,
			// restore old value.
			if(stop_code != -1)
				binfo->book->text_context.auto_stop_code = stop_code;

			error_code = ebook_my_backward_text(binfo);
			if(error_code != EB_SUCCESS){
				LOG(LOG_CRITICAL, "Failed to back text : %s",
					ebook_error_message(error_code));
				// Do nothing
				goto NO_MATCH;
			}

				
			error_code = eb_tell_text(binfo->book, &text_position);
			if(error_code != EB_SUCCESS){
				LOG(LOG_CRITICAL, "Failed to tell text : %s",
					ebook_error_message(error_code));
				// Do nothing
				goto NO_MATCH;
			}

			if(check_duplicate_hit(text_position) == TRUE){
				// Do nothing
				goto NO_MATCH;
			}


			error_code = eb_read_text(binfo->book, binfo->appendix, &heading_hookset, 
						  NULL, MAXLEN_TEXT, heading, &length);
			if (error_code != EB_SUCCESS) {
				LOG(LOG_CRITICAL, "Failed to read text : %s",
					ebook_error_message(error_code));
				// Do nothing
				goto NO_MATCH;
			}
			heading[length] = '\0';

			p = strchr(heading, '\n');
			if(p != NULL)
				*p = '\0';


			rp = (RESULT *)calloc(sizeof(RESULT),1);
			if(rp == NULL){
				LOG(LOG_ERROR, "No memory");
				exit(1);
			}

			rp->heading = iconv_convert("euc-jp", "utf-8", heading);
			rp->word = iconv_convert("euc-jp", "utf-8", word);
			rp->type = RESULT_TYPE_EB;
			rp->data.eb.book_info = binfo;
			rp->data.eb.search_method = method;
			rp->data.eb.dict_title = strdup(title);
		
			rp->data.eb.pos_heading = text_position;
			rp->data.eb.pos_text = text_position;

			add_result(rp);

			if((binfo->book->text_context.auto_stop_code != stop_code) &&
			   (binfo->book->text_context.auto_stop_code != -1))
				stop_code = binfo->book->text_context.auto_stop_code;
			pthread_testcancel();

		NO_MATCH:
			word_p = jisword;
			continue;
		}
	}

	free(jisword);

	LOG(LOG_DEBUG, "OUT : ebook_full_search()");
	return(error_code);
}

// Full text search using BMH method.
static gint ebook_full_search(BOOK_INFO *binfo, char *word, gint method, gchar *title)
{

	EB_Error_Code error_code=EB_SUCCESS;
	EB_Position text_position;
	char data[EB_SIZE_PAGE];
	char *jisword;
	char *word_p;
	int length;
	char *p;
	char heading[MAXLEN_TEXT + 1];
	RESULT *rp;

	int start_page;
	int end_page;
	int current_page;

	int stop_code;
	int page_count=0;

	BMH_TABLE *bmh;
	gchar *start_p;
	gint word_len;

	LOG(LOG_DEBUG, "IN : ebook_full_search(%s)", word);

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	start_page = binfo->book->subbook_current->text.start_page;
	end_page = binfo->book->subbook_current->text.end_page;

	// Read once in order to get auto_stop_code.
	p = ebook_get_text(binfo, start_page, 0);
	if(p)
		free(p);
	stop_code = binfo->book->text_context.auto_stop_code;

	jisword = euc2jis(word);
	word_p = jisword;

	bmh = bmh_prepare(jisword, TRUE);

	word_len = strlen(jisword);
	memset(data, 0, word_len);

	for(current_page = start_page ; current_page <= end_page ; current_page ++){

		set_progress((float)(current_page - start_page) / (end_page - start_page + 1));
		page_count ++;

		text_position.page = current_page;
		text_position.offset = 0;

		error_code = eb_seek_text(binfo->book, &text_position);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to seek text : %s",
				ebook_error_message(error_code));
			return(error_code);
		}

		error_code = eb_read_rawtext(binfo->book, 
					     EB_SIZE_PAGE, 
					     &data[word_len],
					     &length);
		if (error_code != EB_SUCCESS || length != EB_SIZE_PAGE){
			LOG(LOG_CRITICAL, "Failed to read rawtext : %s",
				ebook_error_message(error_code));
			return(error_code);
		}

		start_p = data;
		while(start_p){

			start_p = bmh_search(bmh, start_p, EB_SIZE_PAGE + word_len - (start_p - data));
		
			if(start_p == NULL)
				break;

			// Match then read

			if((start_p - data) <= word_len) {
				text_position.page = current_page - 1;
				text_position.offset = EB_SIZE_PAGE - word_len;
			} else {
				text_position.page = current_page;
				text_position.offset = start_p - data - word_len;
			}

			error_code = eb_seek_text(binfo->book, &text_position);
			if (error_code != EB_SUCCESS) {
				LOG(LOG_CRITICAL, "Failed to seek text : %s",
					ebook_error_message(error_code));
				// Do nothing
				goto NO_MATCH;
			}
		
			// Because eb_seek_text() will clear auto_stop_code,
			// restore old value.
			if(stop_code != -1)
				binfo->book->text_context.auto_stop_code = stop_code;

			error_code = ebook_my_backward_text(binfo);
			if(error_code != EB_SUCCESS){
				LOG(LOG_CRITICAL, "Failed to back text : %s",
					ebook_error_message(error_code));
				// Do nothing
				goto NO_MATCH;
			}

				
			error_code = eb_tell_text(binfo->book, &text_position);
			if(error_code != EB_SUCCESS){
				LOG(LOG_CRITICAL, "Failed to tell text : %s",
					ebook_error_message(error_code));
				// Do nothing
				goto NO_MATCH;
			}

			if(check_duplicate_hit(text_position) == TRUE){
				// Do nothing
				goto NO_MATCH;
			}


			error_code = eb_read_text(binfo->book, binfo->appendix, &heading_hookset, 
						  NULL, MAXLEN_TEXT, heading, &length);
			if (error_code != EB_SUCCESS) {
				LOG(LOG_CRITICAL, "Failed to read text : %s",
					ebook_error_message(error_code));
				// Do nothing
				goto NO_MATCH;
			}
			heading[length] = '\0';

			p = strchr(heading, '\n');
			if(p != NULL)
				*p = '\0';


			rp = (RESULT *)calloc(sizeof(RESULT),1);
			if(rp == NULL){
				LOG(LOG_ERROR, "No memory");
				exit(1);
			}
		
			rp->heading = iconv_convert("euc-jp", "utf-8", heading);
			rp->word = iconv_convert("euc-jp", "utf-8", word);
			rp->type = RESULT_TYPE_EB;
			rp->data.eb.book_info = binfo;
			rp->data.eb.search_method = method;
			rp->data.eb.dict_title = strdup(title);
		
			rp->data.eb.pos_heading = text_position;
			rp->data.eb.pos_text = text_position;

			add_result(rp);

			if((binfo->book->text_context.auto_stop_code != stop_code) &&
			   (binfo->book->text_context.auto_stop_code != -1))
				stop_code = binfo->book->text_context.auto_stop_code;
			pthread_testcancel();

		NO_MATCH:
			start_p += word_len;
			continue;
		}

	}

	bmh_free(bmh);
	free(jisword);

	LOG(LOG_DEBUG, "OUT : ebook_full_search()");
	return(error_code);
}

// Sort by dictionary, then search method
static gint ebook_search2(char *g_word, GtkTreeIter *parent)
{

	int j;
	int method;
	EB_Error_Code error_code=EB_SUCCESS;
	GtkTreeIter iter;

	gchar *word  = strdup(g_word);

	LOG(LOG_DEBUG, "IN : ebook_search2()");

	clear_search_result();

	method = ebook_search_method();


	if(gtk_tree_model_iter_children(GTK_TREE_MODEL(dict_store), &iter, parent) == TRUE){
		do {

			gint type;
			gchar *title;
			gboolean active;
			BOOK_INFO *binfo;
			
			gtk_tree_model_get(GTK_TREE_MODEL(dict_store),
					   &iter,
					   DICT_TYPE_COLUMN, &type,
					   DICT_TITLE_COLUMN, &title,
					   DICT_ACTIVE_COLUMN, &active,
					   DICT_MEMBER_COLUMN, &binfo,
					   -1);

			if(active != TRUE)
				continue;
			if(binfo == NULL)
				continue;
			if(binfo->available == FALSE)
				continue;


			if(method == SEARCH_METHOD_AUTOMATIC){
				for(j=SEARCH_METHOD_MIN ; j<=SEARCH_METHOD_MAX ; j++){
					
				// Except word search and backward search.
					if((j == SEARCH_METHOD_WORD) && (!bword_search_automatic))
						continue;

					if(j == SEARCH_METHOD_ENDWORD)
						continue;

					if(binfo->search_method[j] == TRUE){
						if(bending_correction == 0){
							error_code = ebook_simple_search(binfo, word, j, title);
						} else {
							error_code = ebook_ending_search(binfo, word, j, title);
						}
						if (error_code != EB_SUCCESS){
							if(error_code == EB_ERR_TOO_MANY_WORDS)
								continue;
							LOG(LOG_CRITICAL, "Failed to search : %s",
							    ebook_error_message(error_code));
							goto END;
						}
					}
				}
			} else if((method == SEARCH_METHOD_FULL_TEXT) || 
				  (method == SEARCH_METHOD_FULL_HEADING)){
				set_cancel_dlg_text(binfo->subbook_title);
				error_code = ebook_full_search(binfo, word, method, title);							goto END;
			} else {
				if(binfo->search_method[method] == TRUE){
					if(bending_correction == 0){
						error_code = ebook_simple_search(binfo, word, method, title);
					} else {
						error_code = ebook_ending_search(binfo, word, method, title);
					}
				}
			}

			g_free (title);
			if(error_code != EB_SUCCESS)
				goto END;

		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	}

 END:
	LOG(LOG_DEBUG, "OUT : ebook_search2()");
	return 0;
}

// Sort by search method, then dictionary
static gint ebook_search3(char *g_word, GtkTreeIter *parent)
{

	int j;
	int method;
	EB_Error_Code error_code=EB_SUCCESS;
	GtkTreeIter iter;

	gchar *word  = strdup(g_word);

	LOG(LOG_DEBUG, "IN : ebook_search3()");

	clear_search_result();

	method = ebook_search_method();


	if(method == SEARCH_METHOD_AUTOMATIC){
		for(j=SEARCH_METHOD_MIN ; j<=SEARCH_METHOD_MAX ; j++){
					
			// Except word search and backward search
			if((j == SEARCH_METHOD_WORD) && (!bword_search_automatic))
				continue;
			
			if(j == SEARCH_METHOD_ENDWORD)
				continue;

			if(gtk_tree_model_iter_children(GTK_TREE_MODEL(dict_store), &iter, parent) == TRUE){
				do {
					gint type;
					gchar *title;
					gboolean active;
					BOOK_INFO *binfo;
			
					gtk_tree_model_get(GTK_TREE_MODEL(dict_store),
							   &iter,
							   DICT_TYPE_COLUMN, &type,
							   DICT_TITLE_COLUMN, &title,
							   DICT_ACTIVE_COLUMN, &active,
							   DICT_MEMBER_COLUMN, &binfo,
							   -1);

					if(active != TRUE)
						continue;
					if(binfo == NULL)
						continue;
					if(binfo->available == FALSE)
						continue;


					if(binfo->search_method[j] == TRUE){
						if(bending_correction == 0){
							error_code = ebook_simple_search(binfo, word, j, title);
						} else {
							error_code = ebook_ending_search(binfo, word, j, title);
						}
						if (error_code != EB_SUCCESS){
							if(error_code == EB_ERR_TOO_MANY_WORDS)
								continue;
							LOG(LOG_CRITICAL, "Failed to search : %s",
								ebook_error_message(error_code));
							goto END;
						}
					}

					g_free (title);

				} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
			}

		}

	}

 END:
	plain_heading();
	sort_result(g_word);

	LOG(LOG_DEBUG, "OUT : ebook_search3()");
	return 0;
}

static void plain_heading()
{
	RESULT *rp;
	GList *l;
	gint len;
	gchar *p;
	gchar *pp;
	guchar body[65536];
	gint i;
	gunichar ch;

	gchar start_tag[512];
	gchar tag_name[512];

	for(l = search_result ; l != NULL; l = g_list_next(l)){
		rp = (RESULT *)(l->data);


		len = g_utf8_strlen(rp->heading, -1);
		p = rp->heading;
		pp = body;

		for(i=0;i<len;i++){
			ch = g_utf8_get_char(p);
			if(ch == '<'){
				get_start_tag(p, start_tag);
				get_tag_name(start_tag, tag_name);

				if(strcmp(tag_name, "gaiji") == 0){
					skip_start_tag(&p, tag_name);
				} else if(strcmp(tag_name, "sub") == 0){
					skip_end_tag(&p, tag_name);
				} else if(strcmp(tag_name, "sup") == 0){
					skip_end_tag(&p, tag_name);
					
				}
			} else if(g_unichar_isspace(ch)){
				p = g_utf8_next_char(p);
			} else {
				pp += g_unichar_to_utf8(ch,
							pp);
				*pp = '\0';
				p = g_utf8_next_char(p);
			}
		}

		rp->data.eb.plain_heading = g_strdup(body);

	}
}

static void sort_result(gchar *word)
{
	RESULT *rp;
	GList *l;
	GList *insert;
	gchar l_word[512];
	gunichar ch;
	gint i;
	gint len;
	gchar *p;
        gchar *pp;

	char *keywords[EBOOK_MAX_KEYWORDS + 1];

	len = g_utf8_strlen(word, -1);

	p = word;
	pp = l_word;

	for(i=0;i<len;i++){
		ch = g_utf8_get_char(p);
		if(g_unichar_isspace(ch) == FALSE){
			pp += g_unichar_to_utf8(ch, pp);
			*pp = '\0';
		}
		p = g_utf8_next_char(p);
	}

	insert = g_list_first(search_result);

	// Exact match
	for(l = search_result ; l != NULL; ){
		rp = (RESULT *)(l->data);

		if((strstr(rp->data.eb.plain_heading, l_word) == rp->data.eb.plain_heading) &&
		   (strlen(rp->data.eb.plain_heading) == strlen(l_word))){
			if(l != insert){
				l = g_list_next(l);
				search_result = g_list_remove(search_result, rp);
				search_result = g_list_insert_before(search_result, insert, rp);
				continue;
			} else {
				insert = g_list_next(insert);
			}
		}
		l = g_list_next(l);
	}

	// Forward match
	for(l = insert ; l != NULL; ){
		rp = (RESULT *)(l->data);

		if(strstr(rp->data.eb.plain_heading, l_word) == rp->data.eb.plain_heading){
			if(l != insert){
				l = g_list_next(l);
				search_result = g_list_remove(search_result, rp);
				search_result = g_list_insert_before(search_result, insert, rp);
				continue;
			} else {
				insert = g_list_next(insert);
			}
		}
		l = g_list_next(l);
	}

	// Partial match
	for(l = insert ; l != NULL; ){
		rp = (RESULT *)(l->data);

		if(strstr(rp->data.eb.plain_heading, l_word) != NULL){
			if(l != insert){
				l = g_list_next(l);
				search_result = g_list_remove(search_result, rp);
				search_result = g_list_insert_before(search_result, insert, rp);
				continue;
			} else {
				insert = g_list_next(insert);
			}
		}
		l = g_list_next(l);
	}


	// keyword match
	split_word(word, keywords);
	for(l = insert ; l != NULL; ){
		rp = (RESULT *)(l->data);

		for(i=0 ; ; i++){
			if(keywords[i] == NULL){
				if(l != insert){
					l = g_list_next(l);
					search_result = g_list_remove(search_result, rp);
					search_result = g_list_insert_before(search_result, insert, rp);
					continue;
				} else {
					insert = g_list_next(insert);
				}
			}
			if(strstr(rp->data.eb.plain_heading, l_word) == NULL)
				break;
		}
		l = g_list_next(l);
	}

	free_words(keywords);

}

static void *ebook_search_thread(void *arg)
{
	char word[MAX_BUFSIZE];
	gboolean active;
	GtkTreeIter iter;
	gint state;


	LOG(LOG_DEBUG, "IN : ebook_search_thread(%s)", arg);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &state);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &state);


	strncpy(word, arg, sizeof(word) -1);
	word[sizeof(word)-1] = '\0';

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get (GTK_TREE_MODEL(dict_store), 
					    &iter, 
					    DICT_ACTIVE_COLUMN, &active,
					    -1);
			if(active == TRUE){
				if((ebook_search_method() == SEARCH_METHOD_AUTOMATIC) &&
				   (bsort_by_dictionary == FALSE)){
					ebook_search3(word, &iter);
				} else {
					ebook_search2(word, &iter);
				}
				thread_end();
				break;
			}
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	}

	LOG(LOG_DEBUG, "OUT : ebook_search_thread()");

	return(NULL);
}

static gchar *gword=NULL;

gint ebook_search(const char *word, gint method)
{
	LOG(LOG_DEBUG, "IN : ebook_search(%s)", word);

	if(gword != NULL)
		g_free(gword);

	gword = strdup(word);

	if(method == SEARCH_METHOD_FULL_TEXT){
		// Cancelable
		thread_search(TRUE, _("Fulltext search"),
			      ebook_search_thread, (void *)gword);
	} else {
		// Non-cancelable
		thread_search(FALSE, _("Fulltext search"),
			      ebook_search_thread, (void *)gword);
	}

	LOG(LOG_DEBUG, "OUT : ebook_search()");

	return(0);

}

gint ebook_search_auto(char *g_word, gint method)
{
	GtkTreeIter iter;

	LOG(LOG_DEBUG, "IN : ebook_search_auto()");

	if(method == SEARCH_METHOD_FULL_TEXT) {
		LOG(LOG_DEBUG, "OUT : ebook_search_auto()");
		return(0);
	}

	// Search group named "selection" and use it if exests
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter) == TRUE){
		do { 
			gchar *title;

			gtk_tree_model_get (GTK_TREE_MODEL(dict_store), 
					    &iter, 
					    DICT_TITLE_COLUMN, &title,
					    -1);
			if(strcasecmp(title, "selection") == 0){
				g_free(title);
				ebook_search2(g_word, &iter);
				return(0);
			}
			g_free(title);
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	}

	// If there is no "selection" group, use active group
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dict_store), &iter) == TRUE){
		do { 
			gboolean active;

			gtk_tree_model_get (GTK_TREE_MODEL(dict_store), 
					    &iter, 
					    DICT_ACTIVE_COLUMN, &active,
					    -1);
			if(active == TRUE){
				ebook_search2(g_word, &iter);
				return(0);
			}
	       } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dict_store), &iter) == TRUE);
	}

	LOG(LOG_DEBUG, "OUT : ebook_search_auto()");
	return(0);
}


gint ebook_simple_search2(BOOK_INFO *binfo, char *word, gint method, gchar *title)
{
	EB_Error_Code error_code=EB_SUCCESS;
	int i, len, total_hits=0;
	EB_Hit hits[MAX_HITS];
	int hitcount;
	char heading[MAXLEN_HEADING + 1];
	char *keywords[EBOOK_MAX_KEYWORDS + 1];
	RESULT *rp;

	LOG(LOG_DEBUG, "IN : ebook_simple_search2(%s, %s, %d, %s)", binfo->subbook_title, word, method, title);

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS){
		LOG(LOG_DEBUG, "OUT : ebook_simple_search2()");
		return(error_code);
	}

	switch(method){
	case SEARCH_METHOD_WORD:
		error_code = eb_search_word(binfo->book, word);		
		break;
	case SEARCH_METHOD_ENDWORD:
		error_code = eb_search_endword(binfo->book, word);		
		break;
	case SEARCH_METHOD_EXACTWORD:
		error_code = eb_search_exactword(binfo->book, word);		
		break;
	case SEARCH_METHOD_KEYWORD:
		split_word(word, keywords);
		error_code = eb_search_keyword(binfo->book,
					       (const char * const *)keywords);
		free_words(keywords);
		break;
	case SEARCH_METHOD_MULTI:
		split_word(word, keywords);

		for(i=0;i<EBOOK_MAX_KEYWORDS;i++){
			if(keywords[i] == NULL)
				break;
		}
		
		error_code = eb_search_multi(binfo->book,
					       global_multi_code,
					       (const char * const *)keywords);
		free_words(keywords);
		break;
	default:
		error_code = EB_ERR_NO_SUCH_SEARCH;
		break;
	}

	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to search : %s",
			ebook_error_message(error_code));
		return(error_code);
	}

	total_hits = count_result(search_result);

	while(1){
		error_code = eb_hit_list(binfo->book, 
					 MAX_HITS, 
					 hits, 
					 &hitcount);
		if(error_code != EB_SUCCESS){
			return(error_code);
		}

		if(hitcount == 0)
			break;
		LOG(LOG_DEBUG, "%d HIT", hitcount);

		for(i = 0 ; i < hitcount ; i ++){
			if((max_search != 0) && (total_hits >= max_search))
				return(EB_SUCCESS);

			if(check_duplicate_hit(hits[i].text) == TRUE)
				continue;

			rp = (RESULT *)calloc(sizeof(RESULT),1);
			if(rp == NULL){
				LOG(LOG_ERROR, "No memory");
				exit(1);
			}

			error_code = eb_seek_text(binfo->book, 
						  &(hits[i].heading));

			if(error_code != EB_SUCCESS){
				return(error_code);
			}

			error_code = eb_read_heading(binfo->book, 
						     binfo->appendix, 
						     &heading_hookset, 
						     NULL, 
						     MAXLEN_HEADING, 
						     heading, 
						     &len);
			if (error_code != EB_SUCCESS) {
				return(error_code);
			}
			heading[len] = '\0';

			rp->heading = iconv_convert("euc-jp", "utf-8", heading);
			if(method == SEARCH_METHOD_MULTI)
				rp->word = NULL;
			else
				rp->word = iconv_convert("euc-jp", "utf-8", word);

			rp->type = RESULT_TYPE_EB;
			rp->data.eb.book_info = binfo;
			rp->data.eb.search_method = method;
			rp->data.eb.dict_title = strdup(title);

			rp->data.eb.pos_heading = hits[i].heading;
			rp->data.eb.pos_text = hits[i].text;

			add_result(rp);

			total_hits ++;

		}
	}

	LOG(LOG_DEBUG, "OUT : ebook_simple_search2()");
	return(error_code);
}

gint ebook_simple_search(BOOK_INFO *binfo, char *word, gint method, gchar *title)
{
	gchar *l_word=NULL;
	EB_Error_Code error_code=EB_SUCCESS;

	LOG(LOG_DEBUG, "IN : ebook_simple_search()");

	// If the keyword is Japanese, try Hiragana and Katakana

	if(iseuc(word)){
		l_word = g_strdup(word);
		katakana_to_hiragana(l_word);
		error_code = ebook_simple_search2(binfo, l_word, method, title);
		if(error_code != EB_SUCCESS){
			goto END;
		}
		
		hiragana_to_katakana(l_word);
		error_code = ebook_simple_search2(binfo, l_word, method, title);
	} else {
		// Not Japanese
		error_code = ebook_simple_search2(binfo, word, method, title);
	}
 END:
	g_free(l_word);
	LOG(LOG_DEBUG, "OUT : ebook_simple_search()");
	return(error_code);
}

static gint ebook_ending_search(BOOK_INFO *binfo, char *word, gint method, gchar *title)
{
	EB_Error_Code error_code=EB_SUCCESS;
	gint i;
	gint len_word, len_ending;
	char *keywords[EBOOK_MAX_KEYWORDS + 1];
	char new_word[MAX_BUFSIZE];
	char new_key[256];
	char *save_key;

	GtkTreeIter   iter;

	LOG(LOG_DEBUG, "IN : ebook_ending_search(method=%d)", method);


	g_assert(binfo != NULL);
	g_assert(word != NULL);

	error_code = ebook_simple_search(binfo, word, method, title);

	if((bending_only_nohit == 1) && (count_result(search_result) != 0)){
		return(error_code);
	}

	split_word(word, keywords);

	for(i=0; keywords[i] != NULL ; i++){

		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(stemming_en_store), &iter) == TRUE){
			do { 
				gchar *pattern;
				gchar *normal;

				gtk_tree_model_get(GTK_TREE_MODEL(stemming_en_store), 
						   &iter, 
						   STEMMING_PATTERN_COLUMN, &pattern,
						   STEMMING_NORMAL_COLUMN, &normal,
						   -1);


				len_word = strlen(keywords[i]);
				len_ending = strlen(pattern);
				if(len_word < len_ending){
					continue;
				}

				if(strcmp(&keywords[i][len_word - len_ending], pattern) == 0){
					memcpy(new_key, keywords[i], len_word - len_ending);
					if(normal)
						sprintf(&new_key[len_word - len_ending],"%s", normal);
					else
						new_key[len_word - len_ending] = '\0';
					save_key = keywords[i];
					keywords[i] = new_key;
					cat_word(new_word, keywords);
					error_code = ebook_simple_search(binfo, new_word, method, title);				
					keywords[i] = save_key;
					if(error_code != EB_SUCCESS){
						return(error_code);
					}
				}

				g_free(pattern);
				g_free(normal);

			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(stemming_en_store), &iter) == TRUE);
		}


	}

	// Japanese stemming
	// Japanese keyword must not be multiple words.

	if(keywords[1] == NULL){

		if((count_result(search_result) != 0) &&
		   (bending_only_nohit == 1))
			goto END;

		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(stemming_ja_store), &iter) == TRUE){
			do { 
				gchar *pattern;
				gchar *normal;
				gchar *tmp;

				gtk_tree_model_get(GTK_TREE_MODEL(stemming_ja_store), 
						   &iter, 
						   STEMMING_PATTERN_COLUMN, &pattern,
						   STEMMING_NORMAL_COLUMN, &normal,
						   -1);

				tmp = iconv_convert("utf-8", "euc-jp", pattern);
				g_free(pattern);
				pattern = tmp;

				tmp = iconv_convert("utf-8", "euc-jp", normal);
				g_free(normal);
				normal = tmp;

				len_word = strlen(keywords[0]);
				len_ending = strlen(pattern);
				if(len_word < len_ending){
					continue;
				}


				for(i=0; i<len_word; i+=2){
					if(strncmp(&keywords[0][i], pattern, len_ending) == 0){
						memcpy(new_key, keywords[0], i);
						sprintf(&new_key[i],"%s", normal);
						error_code = ebook_simple_search(binfo, new_key, method, title);				
						if(error_code != EB_SUCCESS){
							return(error_code);
						}
					}
				}


				g_free(pattern);
				g_free(normal);

			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(stemming_ja_store), &iter) == TRUE);
		}

		// If there is no hit, try using only Kanji part.
		if((count_result(search_result) == 0) && 
		   (iseuckanji(keywords[0]))){
			len_word = strlen(keywords[0]);
			strcpy(new_key, keywords[0]);
			for(i=0; i<len_word; i+=2){
				if(!iseuckanji(&new_key[i])) {
					new_key[i] = '\0';
					break;
				}
			}

			error_code = ebook_simple_search(binfo, new_key, method, title);
			if(error_code != EB_SUCCESS){
				return(error_code);
			}
		}
	}

 END:
	free_words(keywords);

	LOG(LOG_DEBUG, "OUT : ebook_ending_search()");
	return(error_code);
}

gchar *ebook_get_heading(BOOK_INFO *binfo, int page, int offset)
{
	EB_Error_Code error_code;
	int len;
	char heading[MAXLEN_HEADING + 1];
	EB_Position position;
	gchar *p;

	LOG(LOG_DEBUG, "IN : ebook_get_heading()");

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(NULL);

	error_code = eb_seek_text(binfo->book, &position);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to seek text : %s",
		       ebook_error_message(error_code));
		return(NULL);
	}

	error_code = eb_read_heading(binfo->book, binfo->appendix, &text_hookset, NULL, MAXLEN_HEADING, heading, &len);
	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to read heading : %s",
		       ebook_error_message(error_code));
		return(NULL);
	}
	heading[len] = '\0';

	p = malloc(len+1);
	memcpy(p, heading, len+1);
	LOG(LOG_DEBUG, "OUT : ebook_get_heading()");
	return(p);
}

gchar *ebook_get_text(BOOK_INFO *binfo, int page, int offset){
	EB_Error_Code error_code;
	int len;
	char text[MAXLEN_TEXT + 1];
	EB_Position position;
	gchar *p;

	LOG(LOG_DEBUG, "IN : ebook_get_text()");

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(NULL);


	position.page = page;
	position.offset = offset;

	error_code = eb_seek_text(binfo->book, &position);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to seek text : %s",
		       ebook_error_message(error_code));
		LOG(LOG_DEBUG, "OUT : ebook_get_text()=NULL");
		return(NULL);
	}


	error_code = eb_read_text(binfo->book, binfo->appendix, &text_hookset, 
				  NULL, MAXLEN_TEXT, text, &len);
	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to read text : %s",
		       ebook_error_message(error_code));
		LOG(LOG_DEBUG, "OUT : ebook_get_text()=NULL");
		return(NULL);
	}
	text[len] = '\0';

	p = malloc(len+1);
	memcpy(p, text, len+1);

	LOG(LOG_DEBUG, "OUT : ebook_get_text()");
	return(p);
}

gchar *ebook_get_candidate(BOOK_INFO *binfo, int page, int offset)
{
	EB_Error_Code error_code;
	int len;
	char text[MAXLEN_TEXT + 1];
	EB_Position position;
	gchar *p;


	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(NULL);


	position.page = page;
	position.offset = offset;

	error_code = eb_seek_text(binfo->book, &position);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to seek text : %s",
		       ebook_error_message(error_code));
		return(NULL);
	}

	error_code = eb_read_text(binfo->book, binfo->appendix, &candidate_hookset, 
				  NULL, MAXLEN_TEXT, text, &len);
	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to read text : %s",
		       ebook_error_message(error_code));
		return(NULL);
	}
	text[len] = '\0';

	p = malloc(len+1);
	memcpy(p, text, len+1);
	return(p);
}

EB_Error_Code ebook_forward_text(BOOK_INFO *binfo)
{
	EB_Error_Code error_code;

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	error_code = eb_seek_text(binfo->book, &current_result->data.eb.pos_text);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to seek text : %s",
		       ebook_error_message(error_code));
		return(error_code);
	}

	error_code = eb_forward_text(binfo->book, binfo->appendix);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to forward text : %s",
		       ebook_error_message(error_code));
		return(error_code);
	}

	return(EB_SUCCESS);
}

EB_Error_Code ebook_backward_text(BOOK_INFO *binfo)
{
	EB_Error_Code error_code;
	int stop_code = -1;

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	stop_code = binfo->book->text_context.auto_stop_code;

	error_code = eb_seek_text(binfo->book, &current_result->data.eb.pos_text);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to seek text : %s",
		       ebook_error_message(error_code));
		return(error_code);
	}

	if(stop_code != -1)
		binfo->book->text_context.auto_stop_code = stop_code;

	error_code = ebook_my_backward_text(binfo);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to back text : %s",
		       ebook_error_message(error_code));
		return(error_code);
	}

	return(EB_SUCCESS);
}

void ebook_tell_text(BOOK_INFO *binfo, gint *page, gint *offset)
{
	EB_Error_Code error_code;
	EB_Position position;

	error_code = eb_tell_text(binfo->book, &position);
	if(error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to tell text : %s",
		       ebook_error_message(error_code));
		return;
	}

	*page = position.page;
	*offset = position.offset;

	return;
}

EB_Error_Code ebook_menu(BOOK_INFO *binfo, EB_Position *pos)
{
	EB_Error_Code error_code=EB_SUCCESS;

	error_code = eb_menu(binfo->book, pos);
	if(error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to get menu position : %s",
		       ebook_error_message(error_code));
	}
	return(error_code);
}

EB_Error_Code ebook_copyright(BOOK_INFO *binfo, EB_Position *pos)
{
	EB_Error_Code error_code=EB_SUCCESS;

	error_code = eb_copyright(binfo->book, pos);
	if(error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to get copyright position : %s",
		       ebook_error_message(error_code));
	}
	return(error_code);
}

static void ebook_bitmap_to_xbm(const char *bitmap, int width, int height, char *xbm, size_t *xbm_length)
{
    char *xbm_p = xbm;
    const unsigned char *bitmap_p = (const unsigned char *)bitmap;
    int bitmap_size = (width + 7) / 8 * height;
    int hex;
    int i;

    for (i = 0; i < bitmap_size; i++) {
	hex = 0;
        if (*bitmap_p & 0x80)
            hex |= 0x01;
        if (*bitmap_p & 0x40)
            hex |= 0x02;
        if (*bitmap_p & 0x20)
            hex |= 0x04;
        if (*bitmap_p & 0x10)
            hex |= 0x08;
        if (*bitmap_p & 0x08)
            hex |= 0x10;
        if (*bitmap_p & 0x04)
            hex |= 0x20;
        if (*bitmap_p & 0x02)
            hex |= 0x40;
        if (*bitmap_p & 0x01)
            hex |= 0x80;
	bitmap_p++;

	*xbm_p = hex;
	xbm_p ++;
    }
    
    *xbm_length = (xbm_p - xbm);
}

gint check_gaiji_size(BOOK_INFO *binfo, gint prefered_size){
	gint size;

	size = prefered_size;

	switch(size){
	case 48:
		if (eb_have_font(binfo->book, EB_FONT_48)){
			size = 48;
			break;
		}
		size = 30;
	case 30:
		if (eb_have_font(binfo->book, EB_FONT_30)){
			size = 30;
			break;
		}
		size = 24;
	case 24:
		if (eb_have_font(binfo->book, EB_FONT_24)){
			size = 24;
			break;
		}
		size = 16;
	case 16:
		if (eb_have_font(binfo->book, EB_FONT_16)){
			size = 16;
			break;
		}
		LOG(LOG_CRITICAL, "Failed to find 16 dot gaiji : subbook=%s",
			binfo->subbook_title);
		return(-1);

	}

	if(size != prefered_size){
		LOG(LOG_CRITICAL, "Cannot find %d dot gaiji. Use %d dot instead",
			prefered_size, size);
	}

	return(size);
	
}

guchar *read_gaiji_as_bitmap(BOOK_INFO *binfo, gchar *name, gint size, gint *width, gint *height)
{
	EB_Error_Code error_code = EB_SUCCESS;
	gint char_no;
	gchar bitmap_data[EB_SIZE_WIDE_FONT_48];
	guchar *image_data;
	size_t image_size;
	int image_width;
	int image_height;
	EB_Subbook *subbook;


	char_no = strtol(&name[1], NULL, 16);

	subbook = binfo->book->subbook_current;

	switch (size) {
	case 16:
		error_code = eb_set_font(binfo->book, EB_FONT_16);
		break;
	case 24:
		error_code = eb_set_font(binfo->book, EB_FONT_24);
		break;
	case 30:
		error_code = eb_set_font(binfo->book, EB_FONT_30);
		break;
	case 48:
		error_code = eb_set_font(binfo->book, EB_FONT_48);
		break;
	}

	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to set font : subbook=%s\n%s",
			binfo->subbook_title, 
			ebook_error_message(error_code));
				return(NULL);
	}


	error_code = eb_font_height(binfo->book, &image_height);
	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to get font height : subbook=%s\n%s",
			binfo->subbook_title, 
			ebook_error_message(error_code));
		return(NULL);
	}
	
	
	if(name[0] == 'h'){
		if (!eb_have_narrow_font(binfo->book)){
			LOG(LOG_CRITICAL, "%s does not have narrow font",
				binfo->subbook_title);
			return(NULL);
		}
		error_code = eb_narrow_font_width(binfo->book, &image_width);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to get font width : subbook=%s\n%s",
				binfo->subbook_title, 
				ebook_error_message(error_code));
			return(NULL);
		}
		error_code = eb_narrow_font_character_bitmap(
			binfo->book,
			char_no,
			bitmap_data);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to read narrow font : subbook=%s, character=0x%04x\n%s",
				binfo->subbook_title, 
				char_no,
				ebook_error_message(error_code));
			return(NULL);

		}
	} else {
		if (!eb_have_wide_font(binfo->book)){
			LOG(LOG_CRITICAL, "%s does not have wide font",
				binfo->subbook_title);
			return(NULL);
		}
		error_code = eb_wide_font_width(binfo->book, &image_width);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to get font width : subbook=%s\n%s",
				binfo->subbook_title, 
				ebook_error_message(error_code));
			return(NULL);
		}
		error_code = eb_wide_font_character_bitmap(
			binfo->book,
			char_no,
			bitmap_data);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to read wide font : subbook=%s, character=0x%04x\n%s",
				binfo->subbook_title, 
				char_no,
				ebook_error_message(error_code));
			return(NULL);
		}
	}

	image_data = malloc(image_width * image_height / 8);
	ebook_bitmap_to_xbm(bitmap_data, image_width,
			    image_height, image_data, &image_size);
	
	*width = image_width;
	*height = image_height;

	return(image_data);
	
}

static gchar **ebook_bitmap_to_xpm(const gchar *bitmap, gint width, gint height, gchar *color)
{
	gchar **xpm;
	gchar *xpm_p;
	int i, j;
	const unsigned char *bitmap_p = (const unsigned char *)bitmap;

	xpm = g_new(gchar *, height + 4 + gaiji_adjustment);
	
	xpm[0] = g_strdup_printf("%d %d 2 1", width, height+gaiji_adjustment);
	xpm[1] = g_strdup_printf(" 	c None");
	if(color == NULL)
		xpm[2] = g_strdup_printf(". 	c Black");
	else
		xpm[2] = g_strdup_printf(". 	c %s", color);

	for (i = 0; i < gaiji_adjustment; i++) {
		xpm[i+3] = g_new(guchar, width + 1);
		memset(xpm[i+3], ' ', width);
		xpm[i+3][width] = '\0';
	}

	for (;i < height + gaiji_adjustment; i++) {
		xpm[i+3] = g_new(guchar, width + 1);
		xpm_p = xpm[i+3];
		for (j = 0; j + 7 < width; j += 8, bitmap_p++) {
			*xpm_p++ = (*bitmap_p & 0x80) ? '.' : ' ';
			*xpm_p++ = (*bitmap_p & 0x40) ? '.' : ' ';
			*xpm_p++ = (*bitmap_p & 0x20) ? '.' : ' ';
			*xpm_p++ = (*bitmap_p & 0x10) ? '.' : ' ';
			*xpm_p++ = (*bitmap_p & 0x08) ? '.' : ' ';
			*xpm_p++ = (*bitmap_p & 0x04) ? '.' : ' ';
			*xpm_p++ = (*bitmap_p & 0x02) ? '.' : ' ';
			*xpm_p++ = (*bitmap_p & 0x01) ? '.' : ' ';
		}

		if (j < width) {
			if (j++ < width)
				*xpm_p++ = (*bitmap_p & 0x80) ? '.' : ' ';
			if (j++ < width)
				*xpm_p++ = (*bitmap_p & 0x40) ? '.' : ' ';
			if (j++ < width)
				*xpm_p++ = (*bitmap_p & 0x20) ? '.' : ' ';
			if (j++ < width)
				*xpm_p++ = (*bitmap_p & 0x10) ? '.' : ' ';
			if (j++ < width)
				*xpm_p++ = (*bitmap_p & 0x08) ? '.' : ' ';
			if (j++ < width)
				*xpm_p++ = (*bitmap_p & 0x04) ? '.' : ' ';
			if (j++ < width)
				*xpm_p++ = (*bitmap_p & 0x02) ? '.' : ' ';
			if (j++ < width)
				*xpm_p++ = (*bitmap_p & 0x01) ? '.' : ' ';
			bitmap_p++;
		}
		*xpm_p = '\0';
	}
	xpm[i+3] = '\0';

	return(xpm);
}


gchar **read_gaiji_as_xpm(BOOK_INFO *binfo, gchar *name, gint size, gint *width, gint *height, gchar *color)
{
	EB_Error_Code error_code = EB_SUCCESS;
	gint char_no;
	gchar bitmap_data[EB_SIZE_WIDE_FONT_48];
	guchar *image_data;
	int image_width;
	int image_height;
	EB_Subbook *subbook;
	gchar **xpm;

	LOG(LOG_DEBUG, "IN : read_gaiji_as_xpm(name=%s, size=%d)", name, size);

	char_no = strtol(&name[1], NULL, 16);

	subbook = binfo->book->subbook_current;

	switch (size) {
	case 16:
		error_code = eb_set_font(binfo->book, EB_FONT_16);
		break;
	case 24:
		error_code = eb_set_font(binfo->book, EB_FONT_24);
		break;
	case 30:
		error_code = eb_set_font(binfo->book, EB_FONT_30);
		break;
	case 48:
		error_code = eb_set_font(binfo->book, EB_FONT_48);
		break;
	}

	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to set font : subbook=%s\n%s",
			binfo->subbook_title, 
			ebook_error_message(error_code));
				return(NULL);
	}


	error_code = eb_font_height(binfo->book, &image_height);
	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to get font height : subbook=%s\n%s",
			binfo->subbook_title, 
			ebook_error_message(error_code));
		return(NULL);
	}
	
	
	if(name[0] == 'h'){
		if (!eb_have_narrow_font(binfo->book)){
			LOG(LOG_CRITICAL, "%s does not have narrow font",
				binfo->subbook_title);
			return(NULL);
		}
		error_code = eb_narrow_font_width(binfo->book, &image_width);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to get font width : subbook=%s\n%s",
				binfo->subbook_title, 
				ebook_error_message(error_code));
			return(NULL);
		}
		error_code = eb_narrow_font_character_bitmap(
			binfo->book,
			char_no,
			bitmap_data);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to read narrow font : subbook=%s, character=0x%04x\n%s",
				binfo->subbook_title, 
				char_no,
				ebook_error_message(error_code));
			return(NULL);

		}
	} else {
		if (!eb_have_wide_font(binfo->book)){
			LOG(LOG_CRITICAL, "%s does not have wide font",
				binfo->subbook_title);
			return(NULL);
		}
		error_code = eb_wide_font_width(binfo->book, &image_width);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to get font width : subbook=%s\n%s",
				binfo->subbook_title, 
				ebook_error_message(error_code));
			return(NULL);
		}
		error_code = eb_wide_font_character_bitmap(
			binfo->book,
			char_no,
			bitmap_data);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to read wide font : subbook=%s, character=0x%04x\n%s",
				binfo->subbook_title, 
				char_no,
				ebook_error_message(error_code));
			return(NULL);
		}
	}

	image_data = malloc((image_width + 4)* (image_height* 5));
	xpm = ebook_bitmap_to_xpm(bitmap_data, image_width, image_height, color);

	*width = image_width;
	*height = image_height + gaiji_adjustment;

#if 0
	{
		gint i;
		for(i=0; i < size+3 ; i ++){
			printf("%s\n", xpm[i]);
		}
	}
#endif

	LOG(LOG_DEBUG, "OUT : read_gaiji_as_xpm()");

	return(xpm);
	
}

#define GIF_PREAMBLE_LENGTH	38

static const unsigned char gif_preamble[GIF_PREAMBLE_LENGTH] = {
    /*
     * Header. (6 bytes)
     */
    'G', 'I', 'F', '8', '9', 'a',

    /*
     * Logical Screen Descriptor. (7 bytes)
     *   global color table flag = 1.
     *   color resolution = 1 - 1 = 0.
     *   sort flag = 0.
     *   size of global color table = 1 - 1 = 0.
     *   background color index = 0.
     *   the pixel aspect ratio = 0 (unused)
     * Logical screen width and height are set at run time.
     */
    0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,

    /*
     * Global Color Table. (6 bytes)
     * These are set at run time.
     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /*
     * Graphic Control Extension. (8 bytes)
     *   disposal method = 0.
     *   user input flag = 0.
     *   transparency flag = 1.
     *   delay time = 0.
     *   transparent color index = 0.
     */
    0x21, 0xf9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,

    /*
     * Image Descriptor. (10 bytes)
     *   image left position = 0. 
     *   image top position = 0. 
     *   local color table flag = 0.
     *   interlace flag = 0.
     *   sort flag = 0.
     *   size of local color table = 0.
     * Image width and height are set at run time.
     */
    0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /*
     * Code size. (1byte)
     */
    0x03
};


static void ebook_bitmap_to_gif(bitmap, width, height, gif, gif_length, fg, bg)
    const char *bitmap;
    int width;
    int height;
    char *gif;
    guint *gif_length;
    guint fg;
    guint bg;
{
    unsigned char *gif_p = (unsigned char *)gif;
    const unsigned char *bitmap_p = (const unsigned char *)bitmap;
    int i, j;


    /*
     * Copy the default preamble.
     */
    memcpy(gif_p, gif_preamble, GIF_PREAMBLE_LENGTH);

    /*
     * Set logical screen width and height.
     */
    gif_p[6] = width & 0xff;
    gif_p[7] = (width >> 8) & 0xff;
    gif_p[8] = height & 0xff;
    gif_p[9] = (height >> 8) & 0xff;

    /*
     * Set global colors.
     */
    gif_p[13] = (bg >> 16) & 0xff;
    gif_p[14] = (bg >> 8) & 0xff;
    gif_p[15] = bg & 0xff;
    gif_p[16] = (fg >> 16) & 0xff;
    gif_p[17] = (fg >> 8) & 0xff;
    gif_p[18] = fg & 0xff;
    
    /*
     * Set image width and height.
     */
    gif_p[32] = width & 0xff;
    gif_p[33] = (width >> 8) & 0xff;
    gif_p[34] = height & 0xff;
    gif_p[35] = (height >> 8) & 0xff;

    gif_p += GIF_PREAMBLE_LENGTH;

    /*
     * Output image data.
     */
    for (i = 0;  i < height; i++) {
	*gif_p++ = (unsigned char)width;
	for (j = 0; j + 7 < width; j += 8, bitmap_p++) {
	    *gif_p++ = (*bitmap_p & 0x80) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x40) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x20) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x10) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x08) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x04) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x02) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x01) ? 0x81 : 0x80;
	}

	if (j < width) {
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x80) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x40) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x20) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x10) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x08) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x04) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x02) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x01) ? 0x81 : 0x80;
	    bitmap_p++;
	}
    }

    /*
     * Output a trailer.
     */
    memcpy(gif_p, "\001\011\000\073", 4);
    gif_p += 4;

    if (gif_length != NULL)
	*gif_length = ((char *)gif_p - gif);

}


guchar *read_gaiji_as_xbm(BOOK_INFO *binfo, gchar *name, gchar *fname, guint fg, guint bg)
{
	EB_Error_Code error_code;
	gint char_no;
	gchar bitmap_data[EB_SIZE_WIDE_FONT_48];
	guchar image_data[EB_SIZE_FONT_IMAGE];
	size_t image_size;
	int image_width;
	int image_height;
	EB_Subbook *subbook;
	FILE *fp;


	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(NULL);

	char_no = strtol(&name[1], NULL, 16);

	subbook = binfo->book->subbook_current;

	error_code = eb_font_height(binfo->book, &image_height);
	if (error_code != EB_SUCCESS) {
		LOG(LOG_CRITICAL, "Failed to get font height : subbook=%s\n%s",
			binfo->subbook_title, 
			ebook_error_message(error_code));
		return(NULL);
	}
	
	
	if(name[0] == 'h'){
		if (!eb_have_narrow_font(binfo->book)){
			LOG(LOG_CRITICAL, "%s does not have narrow font",
				binfo->subbook_title);
			return(NULL);
		}
		error_code = eb_narrow_font_width(binfo->book, &image_width);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to get font width : subbook=%s\n%s",
				binfo->subbook_title, 
				ebook_error_message(error_code));
			return(NULL);
		}
		error_code = eb_narrow_font_character_bitmap(
			binfo->book,
			char_no,
			bitmap_data);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to read narrow font : subbook=%s, character=0x%04x\n%s",
				binfo->subbook_title, 
				char_no,
				ebook_error_message(error_code));
			return(NULL);

		}
	} else {
		if (!eb_have_wide_font(binfo->book)){
			LOG(LOG_CRITICAL, "%s does not have wide font",
				binfo->subbook_title);
			return(NULL);
		}
		error_code = eb_wide_font_width(binfo->book, &image_width);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to get font width : subbook=%s\n%s",
				binfo->subbook_title, 
				ebook_error_message(error_code));
			return(NULL);
		}
		error_code = eb_wide_font_character_bitmap(
			binfo->book,
			char_no,
			bitmap_data);
		if (error_code != EB_SUCCESS) {
			LOG(LOG_CRITICAL, "Failed to read wide font : subbook=%s, character=0x%04x\n%s",
				binfo->subbook_title, 
				char_no,
				ebook_error_message(error_code));
			return(NULL);
		}
	}


	ebook_bitmap_to_gif(bitmap_data, image_width,
			    image_height, image_data, &image_size, fg, bg);


	fp = fopen(fname, "wb");
	if(fp == NULL){
		LOG(LOG_CRITICAL, "file open failed : %s", fname);
	}
	fwrite(image_data, image_size, 1, fp);	
	fclose(fp);

	return(NULL);
	
}

EB_Error_Code ebook_output_wave(BOOK_INFO *binfo, gchar *filename, gint page, gint offset, gint size)
{
	EB_Position pos;
	char binary_data[EB_SIZE_PAGE];
	EB_Error_Code error_code;
	EB_Position end_position;
	ssize_t read_length;
	FILE *fp;

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	pos.page = page;
	pos.offset = offset;

	end_position.page = pos.page
		+ (size / EB_SIZE_PAGE);
	end_position.offset = pos.offset
		+ (size % EB_SIZE_PAGE);
	if (EB_SIZE_PAGE <= end_position.offset) {
		end_position.offset -= EB_SIZE_PAGE;
		end_position.page++;
	}

	/*
	 * Read sound data.
	 */
	error_code = eb_set_binary_wave(binfo->book, 
					&pos, &end_position);
	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to set binary wave : %s",
				ebook_error_message(error_code));
		return(error_code);
	}

	fp = fopen(filename, "wb");
	if(fp == NULL){
		LOG(LOG_CRITICAL, "Failed to open file : %s",
			filename);
		return(EB_ERR_BAD_FILE_NAME);
	}

	for (;;) {
		error_code = eb_read_binary(binfo->book, 
					    EB_SIZE_PAGE,
					    binary_data, &read_length);
		if (error_code != EB_SUCCESS || read_length == 0){
			fclose(fp);
			return(error_code);
		}

		// If there are extra data (32 bytes) before fmt chunk,remove them.
		if((strncmp("fmt ", &binary_data[44], 4) == 0) &&
		   (strncmp("fmt ", &binary_data[12], 4) != 0)){
			LOG(LOG_CRITICAL, "Warning: extra header found in WAVE data.");
			fwrite(binary_data, 12, 1, fp);
			fwrite(&binary_data[44], read_length - 44, 1, fp);
		} else {
			fwrite(binary_data, read_length, 1, fp);
		}
	}

	/* not reached */
	return(EB_SUCCESS);
}

EB_Error_Code ebook_output_mpeg(BOOK_INFO *binfo, gchar *srcname, gchar *destname)
{
	char binary_data[EB_SIZE_PAGE];
	guint argv[4];
	EB_Error_Code error_code;
	ssize_t read_length;
	FILE *fp;

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	if((error_code = eb_decompose_movie_file_name(argv, srcname)) != EB_SUCCESS)
		return(error_code);

	/*
	 * Read sound data.
	 */
	error_code = eb_set_binary_mpeg(binfo->book, argv);
	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to set binary mpeg : %s",
				ebook_error_message(error_code));
		return(error_code);
	}

	fp = fopen(destname, "wb");
	if(fp == NULL){
		LOG(LOG_CRITICAL, "Failed to open file : %s", destname);
		return(EB_ERR_BAD_FILE_NAME);
	}

	for (;;) {
		error_code = eb_read_binary(binfo->book, 
					    EB_SIZE_PAGE,
					    binary_data, &read_length);
		if (error_code != EB_SUCCESS || read_length == 0){
			fclose(fp);
			return(error_code);
		}
		fwrite(binary_data, read_length, 1, fp);
	}

	/* not reached */
	return(EB_SUCCESS);
}

EB_Error_Code ebook_output_color(BOOK_INFO *binfo, gchar *filename, gint page, gint offset)
{
	EB_Position pos;
	char binary_data[EB_SIZE_PAGE];
	EB_Error_Code error_code;
	ssize_t read_length;
	FILE *fp;
	
	pos.page = page;
	pos.offset = offset;

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	error_code = eb_set_binary_color_graphic(binfo->book, 
						 &pos);

	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to set binary color graphic : %s",
			ebook_error_message(error_code));
		return(error_code);
	}
	
	fp = fopen(filename, "wb");
	if(fp == NULL){
		LOG(LOG_CRITICAL, "Failed to open file : %s",
			filename);
		return(EB_ERR_BAD_FILE_NAME);
	}

	for (;;) {
		error_code = eb_read_binary(binfo->book, EB_SIZE_PAGE,
					    binary_data, &read_length);
		if (error_code != EB_SUCCESS || read_length == 0){
			fclose(fp);
			return(error_code);
		}
		fwrite(binary_data, read_length, 1, fp);
	}

	/* not reached */
	return(EB_SUCCESS);
}


EB_Error_Code ebook_output_gray(BOOK_INFO *binfo, gchar *filename, gint page, gint offset, gint width, gint height)
{
	EB_Position pos;
	char binary_data[EB_SIZE_PAGE];
	EB_Error_Code error_code;
	ssize_t read_length;
	FILE *fp;
	
	pos.page = page;
	pos.offset = offset;

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	error_code = eb_set_binary_gray_graphic(binfo->book, 
						 &pos, width, height);
	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to set binary gray graphic : %s",
			ebook_error_message(error_code));
		return(error_code);
	}
	
	fp = fopen(filename, "wb");
	if(fp == NULL){
		LOG(LOG_CRITICAL, "Failed to open file : %s",
			filename);
		return(EB_ERR_BAD_FILE_NAME);
	}

	for (;;) {
		error_code = eb_read_binary(binfo->book, EB_SIZE_PAGE,
					    binary_data, &read_length);
		if (error_code != EB_SUCCESS || read_length == 0){
			fclose(fp);
			return(error_code);
		}
		fwrite(binary_data, read_length, 1, fp);
	}

	/* not reached */
	return(EB_SUCCESS);
}

EB_Error_Code ebook_output_mono(BOOK_INFO *binfo, gchar *filename, gint page, gint offset, gint width, gint height)
{
	FILE *fp;
	EB_Error_Code error_code;
	EB_Position pos;
	char *binary_data;
	ssize_t read_length;
	gint data_size;
	gchar *bmp_data;
	gint bmp_length;

#ifdef COLOR_HACK
	
	guchar fg[4];
	guchar bg[4];
	GdkColor color;

	color = dict_area->area->style->fg[GTK_STATE_NORMAL];
	fg[0] = (guchar)color.red;
	fg[1] = (guchar)color.green;
	fg[2] = (guchar)color.blue;
	fg[3] = 0x0;

	color = dict_area->area->style->bg[GTK_STATE_NORMAL];
	bg[0] = (guchar)color.red;
	bg[1] = (guchar)color.green;
	bg[2] = (guchar)color.blue;
	bg[3] = 0x0;

#endif

	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(error_code);

	fp = fopen(filename, "wb");
	if(fp == NULL){
		LOG(LOG_CRITICAL, "Failed to open file : %s",
			filename);
		return(EB_ERR_BAD_FILE_NAME);
	}

	pos.page = page;
	pos.offset = offset;

	eb_seek_text(binfo->book, &pos);

	error_code = eb_set_binary_mono_graphic(binfo->book, 
					&pos, width, height);


	// Workaround for dictionaries such as Super Tougou Jisho 2000, 
	// whose graphics data is in Honmon2.
	// Fixed in eb-3.3.
	if (error_code != EB_SUCCESS){

		if((width % 8) != 0)
			width = (width / 8)*8 + 8;

		data_size = width * height / 8;

		binary_data = malloc(data_size);
		bmp_data = malloc(data_size*10);


		pos.page = page;
		pos.offset = offset;

		eb_seek_text(binfo->book, &pos);
		error_code = eb_read_rawtext(binfo->book, 
					     data_size, 
					     binary_data,
					     &read_length);
		if (error_code != EB_SUCCESS || read_length == 0){
			return(error_code);
		}

		 eb_bitmap_to_bmp(binary_data,
				  width,
				  height,
				  bmp_data,
				  &bmp_length);

		fwrite(bmp_data, bmp_length, 1, fp);

#ifdef COLOR_HACK
		fseek(fp, 54, SEEK_SET);
		fwrite(bg, 4, 1, fp);
		fseek(fp, 58, SEEK_SET);
		fwrite(fg, 4, 1, fp);
#endif
		fclose(fp);
		free(binary_data);
		free(bmp_data);
		return(EB_SUCCESS);
	}


	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to set binary mono : %s",
				ebook_error_message(error_code));
		return(error_code);
	}

	for (;;) {
		char binary_data[EB_SIZE_PAGE];

		error_code = eb_read_binary(binfo->book, 
					    EB_SIZE_PAGE,
					    binary_data, &read_length);
		if (error_code != EB_SUCCESS || read_length == 0){
			fclose(fp);
			return(error_code);
		}
#ifdef COLOR_HACK
		memcpy(&binary_data[54], bg, 4);
		memcpy(&binary_data[58], fg, 4);
#endif
		fwrite(binary_data, read_length, 1, fp);
	}

	/* not reached */
	fclose(fp);
	return(EB_SUCCESS);
}

gchar *ebook_get_rawtext(BOOK_INFO *binfo, gint page, gint offset)
{
	EB_Position pos;
	char *binary_data;
	EB_Error_Code error_code;
	ssize_t read_length;

	binary_data = malloc(EB_SIZE_PAGE);
	
	if((error_code = ebook_set_subbook(binfo)) != EB_SUCCESS)
		return(NULL);

	pos.page = page;
	pos.offset = offset;

	eb_seek_text(binfo->book, &pos);
	error_code = eb_read_rawtext(binfo->book, 
				     EB_SIZE_PAGE, 
				     binary_data, 
				     &read_length);
	if (error_code != EB_SUCCESS || read_length == 0){
		return(NULL);
	}

	return(binary_data);
}

EB_Error_Code ebook_set_subbook(BOOK_INFO *binfo)
{

	EB_Error_Code error_code;

	error_code = eb_set_subbook(binfo->book, binfo->subbook_no); 
	if (error_code != EB_SUCCESS){
		LOG(LOG_CRITICAL, "Failed to set subbook %s, %d : %s",
			binfo->book_path, binfo->subbook_no,
			ebook_error_message(error_code));
		return(error_code);
	}


	if(binfo->appendix != NULL){
		error_code = eb_set_appendix_subbook(binfo->appendix, 
					    binfo->appendix_subbook_no); 
		if (error_code != EB_SUCCESS){
			LOG(LOG_CRITICAL, "Failed to set appendix subbook : %s",
				ebook_error_message(error_code));
			return(error_code);
		}
	}
	return(EB_SUCCESS);

}

