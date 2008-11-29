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

#ifndef __EB_H_
#define __EB_H_

#include "defs.h"

BOOK_INFO *load_book(const char *book_path, int subbook_no, gchar *appendix_path, gint appendix_subbook_no, gchar *fg, gchar *bg);
void unload_book(BOOK_INFO *binfo);
void check_search_method();
gint ebook_start();
gint ebook_end();
void split_word(const gchar *word, gchar **keywords);
void cat_word(char *string, char **words);
void free_words(char **words);
gint ebook_search(const char *g_word, gint method);
gint ebook_search_auto(char *g_word, gint method);
gint ebook_simple_search(BOOK_INFO *binfo, char *word, gint method, gchar *title);
gint ebook_search_method();

gchar *ebook_get_heading(BOOK_INFO *binfo, int page, int offset);
gchar *ebook_get_text(BOOK_INFO *binfo, int page, int offset);
gchar *ebook_get_candidate(BOOK_INFO *binfo, int page, int offset);
EB_Error_Code ebook_forward_text(BOOK_INFO *binfo);
EB_Error_Code ebook_backward_text(BOOK_INFO *binfo);
void ebook_tell_text(BOOK_INFO *binfo, gint *page, gint *offset);
EB_Error_Code ebook_menu(BOOK_INFO *binfo, EB_Position *pos);
EB_Error_Code ebook_copyright(BOOK_INFO *binfo, EB_Position *pos);
gchar *ebook_error_message(EB_Error_Code error_code);

guchar *read_gaiji_as_bitmap(BOOK_INFO *binfo, gchar *name, gint size, gint *width, gint *height);
guchar *read_gaiji_as_xbm(BOOK_INFO *binfo, gchar *name, gchar *fname, guint fg, guint bg);
gchar **read_gaiji_as_xpm(BOOK_INFO *binfo, gchar *name, gint size, gint *width, gint *height, gchar *color);
gint check_gaiji_size(BOOK_INFO *binfo, gint prefered_size);
EB_Error_Code ebook_output_wave(BOOK_INFO *binfo, gchar *filename, gint page, gint offset, gint size);
EB_Error_Code ebook_output_mpeg(BOOK_INFO *binfo, gchar *srcname, gchar *destname);
EB_Error_Code ebook_output_color(BOOK_INFO *binfo, gchar *filename, gint page, gint offset);
EB_Error_Code ebook_output_gray(BOOK_INFO *binfo, gchar *filename, gint page, gint offset, gint width, gint height);
EB_Error_Code ebook_output_mono(BOOK_INFO *binfo, gchar *filename, gint page, gint offset, gint width, gint height);
gchar *ebook_get_rawtext(BOOK_INFO *binfo, gint page, gint offset);
EB_Error_Code ebook_set_subbook(BOOK_INFO *binfo);

#endif /* __EB_H_ */

