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

#ifndef __DEFS_H__
#define __DEFS_H__

/* Disable old GTK+ functions */
//#define GTK_DISABLE_DEPRECATED 1

#include "../config.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkprivate.h>
#include <locale.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#ifdef __WIN32__
#define ENABLE_NLS 1
#endif
#include "intl.h"

/* Platform specific includes */
#ifdef __WIN32__

#include <windows.h>
#include <gdk/gdkwin32.h>

#else

#include <X11/X.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

#endif /* __WIN32__ */

/* EB Library includes */
#define ENABLE_EBNET
//#define EBCONF_EHABLE_PTHREAD

#include <eb/eb.h>
#include <eb/defs.h>
#include <eb/text.h>
#include <eb/font.h>
#include <eb/appendix.h>
#include <eb/error.h>
#include <eb/binary.h>

#define eb_uint1(p) (*(const unsigned char *)(p))

#define eb_uint2(p) ((*(const unsigned char *)(p) << 8) \
        + (*(const unsigned char *)((p) + 1)))

#define eb_uint3(p) ((*(const unsigned char *)(p) << 16) \
        + (*(const unsigned char *)((p) + 1) << 8) \
        + (*(const unsigned char *)((p) + 2)))

#define eb_uint4(p) ((*(const unsigned char *)(p) << 24) \
        + (*(const unsigned char *)((p) + 1) << 16) \
        + (*(const unsigned char *)((p) + 2) << 8) \
        + (*(const unsigned char *)((p) + 3)))

#define eb_uint4_le(p) ((*(const unsigned char *)(p)) \
        + (*(const unsigned char *)((p) + 1) << 8) \
        + (*(const unsigned char *)((p) + 2) << 16) \
        + (*(const unsigned char *)((p) + 3) << 24))



#define MAX_BUFF 512
#define MAX_BOOKS 128
#define MAX_MULTI_SEARCH 10

#define SEARCH_METHOD_AUTOMATIC     50
#define SEARCH_METHOD_WORD           0
#define SEARCH_METHOD_ENDWORD        1
#define SEARCH_METHOD_EXACTWORD      2
#define SEARCH_METHOD_KEYWORD        3
#define SEARCH_METHOD_MULTI          4
#define SEARCH_METHOD_MENU          10
#define SEARCH_METHOD_COPYRIGHT     11
#define SEARCH_METHOD_FULL_TEXT     12
#define SEARCH_METHOD_FULL_HEADING  13
#define SEARCH_METHOD_INTERNET      14
#define SEARCH_METHOD_GREP          15
#define SEARCH_METHOD_UNKNOWN       99

#define SEARCH_METHOD_MIN            0
#define SEARCH_METHOD_MAX            4


#define CURSOR_LINK   GDK_HAND2
#define CURSOR_NORMAL GDK_LEFT_PTR
#define CURSOR_BUSY GDK_CLOCK
#define CURSOR_SOUND GDK_CLOCK

#define TAG_TYPE_NONE 0
#define TAG_TYPE_LINK 1 << 1
#define TAG_TYPE_SOUND 1 << 2
#define TAG_TYPE_MOVIE 1 << 3
#define TAG_TYPE_EMPHASIS 1 << 4
#define TAG_TYPE_SUBSCRIPT 1 << 5
#define TAG_TYPE_SUPERSCRIPT 1 << 6
#define TAG_TYPE_KEYWORD 1 << 7
#define TAG_TYPE_ITALIC 1 << 8
#define TAG_TYPE_CENTER 1 << 9
#define TAG_TYPE_REVERSE 1 << 10
#define TAG_TYPE_COLORED 1 << 11

#define MAX_INDENT 16
#define INDENT_LEFT_MARGIN 16
#define INITIAL_LEFT_MARGIN 10

#define DATA_TEXT 0
#define DATA_NOENDTAG 1
#define DATA_SPECIAL 2
#define DATA_BRANCH 3

#define COLOR_LINK     0
#define COLOR_KEYWORD  1
#define COLOR_SOUND    2
#define COLOR_MOVIE    3
#define COLOR_EMPHASIS 4
#define COLOR_REVERSE_BG 5
#define NUM_COLORS     6

#define SELECTION_DO_NOTHING    0
#define SELECTION_COPY_ONLY     1
#define SELECTION_SEARCH        2
#define SELECTION_SEARCH_TOP    3
#define SELECTION_POPUP         4


#define HEADING_WIDTH 1024
#define HEADING_HEIGHT 18
#define HEADING_PIXMAP_WIDTH 2048
#define HEADING_PIXMAP_HEIGHT 18
#define LINE_HEIGHT 18
#define DICT_WIDTH 500
#define DICT_HEIGHT 1024*10

#define PIXMAP_BUFFER_WIDTH  1280
#define PIXMAP_BUFFER_HEIGHT 1024*10
#define GAIJI_ADJUSTMENT 2

#define MODE_PLAIN    0
#define MODE_REDRAW   1
#define MODE_LINK     2

#define DIRECTION_FORWARD  0
#define DIRECTION_BACKWARD 1

enum
{
	RESULT_TYPE_EB,
	RESULT_TYPE_GREP
};


#define MAX_DICT_GROUP    255
#define MAX_GROUP_MEMBER  255
#define MAX_KEYWORD_LENGTH  255

#define FILENAME_PREFERENCE   "preference.xml"
#define FILENAME_DICTGROUP    "dictgroup.xml"
#define FILENAME_STEMMING_EN  "endinglist.xml"
#define FILENAME_STEMMING_JA  "endinglist-ja.xml"
#define FILENAME_SHORTCUT     "shortcut.xml"
#define FILENAME_WEBLIST      "searchengines.xml"
#define FILENAME_HISTORY      "history.xml"
#define FILENAME_DIRLIST      "dirlist.xml"
#define FILENAME_FILTER       "filter.xml"
#define FILENAME_DIRGROUP     "dirgroup.xml"
#define FILENAME_GTKRC        "gtkrc"

#ifdef __WIN32__
#define DIR_DELIMITER "\\"
#else
#define DIR_DELIMITER "/"
#endif

#define DEFAULT_DICT_BGCOLOR "#80d0b0"
#define DEFAULT_DICT_FGCOLOR "#000000"

typedef struct {
	gint code;
	gchar **data;
	gint width;
	gint height;
} GAIJI_CACHE;

typedef struct {
        gboolean available;
	EB_Book *book;
	EB_Appendix *appendix;
	char *book_path;
	char *appendix_path;
	char *fg;
	char *bg;
	EB_Subbook_Code subbook_no;
	EB_Subbook_Code appendix_subbook_no;
	char *subbook_dir;
  	char *subbook_title;
	GList *gaiji_narrow16;
	GList *gaiji_narrow24;
	GList *gaiji_narrow30;
	GList *gaiji_narrow48;
	GList *gaiji_wide16;
	GList *gaiji_wide24;
	GList *gaiji_wide30;
	GList *gaiji_wide48;
	gboolean search_method[20];
} BOOK_INFO;

struct _search_method {
	int  code;
	char *name;
};

typedef struct {
	gint type;
	guint start;
	guint end;
	gint page;
	gint offset;
	gint size;
	gchar filename[256];
} TAG;

typedef struct _result_eb {
	BOOK_INFO *book_info;
	gint search_method;
	gchar *plain_heading;
	gchar *dict_title;
	EB_Position pos_heading;
	EB_Position pos_text;
} RESULT_EB;

typedef struct _result_grep{
	gchar *filename;
	gint page;
	gint line;
	gint offset;
} RESULT_GREP;

typedef struct {
	gchar *heading;
	gchar *word;
	gint type;
	union {
		RESULT_EB eb;
		RESULT_GREP grep;
//		struct _result_eb eb;
//		struct _result_grep grep;
	} data ;
} RESULT;

typedef struct {
	BOOK_INFO *book_info;
	gint code;
	gchar *text;
} MULTI_SEARCH;

typedef struct {
	gchar *text;
	gint length;
} DRAW_TEXT;

typedef struct {
	GtkTextBuffer *buffer;
	GtkTextIter *iter;
	gint indent;
} CANVAS;


// Enums for GtkTreeStore

enum
{
	WEB_TYPE_COLUMN,
	WEB_TITLE_COLUMN,
	WEB_HOME_COLUMN,
	WEB_PRE_COLUMN,
	WEB_POST_COLUMN,
	WEB_GLUE_COLUMN,
	WEB_CODE_COLUMN,
	WEB_N_COLUMNS
};

enum
{
	DICT_TYPE_COLUMN,
	DICT_TITLE_COLUMN,
	DICT_PATH_COLUMN,
	DICT_SUBBOOK_NO_COLUMN,
	DICT_APPENDIX_PATH_COLUMN,
	DICT_APPENDIX_SUBBOOK_NO_COLUMN,
	DICT_ACTIVE_COLUMN,
	DICT_MEMBER_COLUMN,
	DICT_EDITABLE_COLUMN,
	DICT_BGCOLOR_COLUMN,
	DICT_FGCOLOR_COLUMN,
	DICT_N_COLUMNS
};

// Enums for GtkListStore

enum
{
	STEMMING_PATTERN_COLUMN,
	STEMMING_NORMAL_COLUMN,
	STEMMING_N_COLUMNS
};

enum
{
	SHORTCUT_STATE_COLUMN, // guint
	SHORTCUT_KEYVAL_COLUMN, // guint
	SHORTCUT_NAME_COLUMN,
	SHORTCUT_DESCRIPTION_COLUMN,
	SHORTCUT_KEYSTR_COLUMN,
	SHORTCUT_COMMAND_COLUMN,  // struct _shortcut_command *
	SHORTCUT_N_COLUMNS
};

enum
{
	MULTI_TYPE_COLUMN,
	MULTI_TITLE_COLUMN,
	MULTI_CODE_COLUMN,
	MULTI_BOOK_COLUMN,
	MULTI_N_COLUMNS
};

enum
{
	FILTER_EXT_COLUMN,
	FILTER_FILTER_COMMAND_COLUMN,
	FILTER_OPEN_COMMAND_COLUMN,
	FILTER_EDITABLE_COLUMN,
	FILTER_N_COLUMNS
};

enum
{
	DIRGROUP_TITLE_COLUMN,
	DIRGROUP_LIST_COLUMN,
	DIRGROUP_ACTIVE_COLUMN,
	DIRGROUP_N_COLUMNS
};



struct _shortcut_command {
	gchar *name;
	void (* func)();
};

#include "log.h"

#endif /* __DEFS_H__ */

