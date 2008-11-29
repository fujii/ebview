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
#include "dialog.h"
#include "grep.h"
#include "selection.h"
#include "headword.h"
#include "mainwindow.h"
#include "misc.h"
#include "popup.h"
#include "history.h"
#include "jcode.h"

static gint tag_timeout=0;
static gchar previous[256];
static gboolean auto_lookup_suspended = FALSE;

extern GList *current_in_result;
extern GtkWidget *hidden_window;

void bring_to_top(GtkWidget *win){
#ifdef __WIN32__
	HWND hWnd;
	int nTargetID, nForegroundID;
	BOOL res;

	hWnd = GDK_WINDOW_HWND (win->window);

	/* From  http://techtips.belution.com/ja/vc/0012/ */
	nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	nTargetID = GetWindowThreadProcessId(hWnd, NULL );

	AttachThreadInput(nTargetID, nForegroundID, TRUE );

	// SPI_GETFOREGROUNDLOCKTIMEOUT will be undefined. Why ?
	/*
	SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);
	SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,(LPVOID)0,0);
	SetForegroundWindow(hWnd);
	SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,sp_time,0);
	*/

	res = SetForegroundWindow(hWnd);

	AttachThreadInput(nTargetID, nForegroundID, FALSE);

	if(!res){
		SetFocus(hWnd);
	}
#else
	gtk_window_present(GTK_WINDOW(win));
#endif
}

static gboolean validate_euc_str(guchar *str){
	guchar *p;

	p = str;
	while(*p){
		if (iseuc(p)) {
			p +=2;
		} else if(isprint(*p)) {
			p ++;
		} else if(isspace(*p)) {
			*p = 0x20;
			p++;
		} else {
			return(FALSE);
		}
	}
	return(TRUE);
}

static void search_selected(gchar *str)
{
	gchar *euc_str;
	gint method;
	glong len;

	LOG(LOG_DEBUG, "IN : search_selected(%s)", str);

	if(selection_mode <= SELECTION_DO_NOTHING) {
		LOG(LOG_DEBUG, "OUT : search_selected() = NOP1");
		return;
	}

	if(strcmp(previous, str) == 0){
		// Do nothing if the word is the save as before.
		LOG(LOG_DEBUG, "same as before");
		;
	} else {

		euc_str = iconv_convert("utf-8", "euc-jp", str);
		if(validate_euc_str(euc_str) == FALSE) {
			g_free(euc_str);
			LOG(LOG_DEBUG, "OUT : search_selected() = INVALID");
			return;
		}
		remove_space(euc_str);

		len = g_utf8_strlen(str, -1);

		if((auto_minchar <= len) && (len <= auto_maxchar)) {
			gtk_entry_set_text(GTK_ENTRY(word_entry), str);

			method = ebook_search_method();
			if((method == SEARCH_METHOD_INTERNET) ||
			   (method == SEARCH_METHOD_MULTI) ||
			   (method == SEARCH_METHOD_FULL_TEXT)){
				LOG(LOG_DEBUG, "OUT : search_selected() = NOP2");
				return;
			}

			if(selection_mode <= SELECTION_COPY_ONLY) {
				LOG(LOG_DEBUG, "OUT : search_selected() = COPY");
				return;
			}
			
			clear_message();
			clear_search_result();

			if(method == SEARCH_METHOD_GREP){
				grep_search(euc_str);
				show_result_tree();
				select_first_item();
				if(selection_mode == SELECTION_SEARCH_TOP)
					bring_to_top(main_window);
				save_word_history(str);
			} else {
				ebook_search_auto(euc_str, method);
				if(search_result){
					if(selection_mode == SELECTION_POPUP) {
						show_result_in_popup();
					} else {
						show_result_tree();
						select_first_item();
						if(selection_mode == SELECTION_SEARCH_TOP)
							bring_to_top(main_window);
					}
					save_word_history(str);
				} else {
					current_in_result = NULL;
					set_current_result(NULL);
					if(selection_mode == SELECTION_POPUP) {
						beep();
					} else {
						if(selection_mode == SELECTION_SEARCH_TOP)
							bring_to_top(main_window);
						push_message(_("No hit."));
					}
				}
			}
			
			sprintf(previous, "%s", str);
		} else {
			LOG(LOG_DEBUG, "OUT : search_selected() = LENGTH");
		}
		g_free(euc_str);
	}

	LOG(LOG_DEBUG, "OUT : search_selected()");
}

void
selection_received (GtkWidget *widget, GtkSelectionData *data)
{
	gchar *str;
	gchar **list;
	gint count;

	LOG(LOG_DEBUG, "IN : selection_received()");

	if((data == NULL) || (data->data == NULL) || (data->length < 0)){
		LOG(LOG_DEBUG, "no data");
		goto END;
	}

	// No conversion required for STRING type.
	if (data->type == GDK_TARGET_STRING){
		str = g_strndup(data->data, data->length);

	// Convert to UTF-8 for COMPOUND_TEXT type.
	} else if ((data->type == gdk_atom_intern ("COMPOUND_TEXT", FALSE)) ||
		   (data->type == gdk_atom_intern ("TEXT", FALSE))){
		count = gdk_text_property_to_utf8_list (data->type,
							data->format, 
							data->data,
							data->length,
							&list);


		if((count == 0) || (list == NULL)){
			goto END;
		}
		str = g_strdup(list[0]);
		g_strfreev(list);

	} else {
		LOG(LOG_DEBUG, "unknown data type");
		goto END;
	}

	remove_space(str);
	search_selected(str);
	g_free(str);

 END:

	if(selection_mode != SELECTION_DO_NOTHING){
		auto_lookup_start();
	}

	LOG(LOG_DEBUG, "OUT : selection_received()");
	return;
}


gint copy_clipboard_win(gpointer data){
	gchar *str=NULL;
	GtkClipboard* clipboard;

	LOG(LOG_DEBUG, "IN : copy_clipboard()");

#ifdef __WIN32__
	clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
#else
	clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
#endif

	str = gtk_clipboard_wait_for_text(clipboard);
	if(str == NULL){
		goto END;
	}

	remove_space(str);
	search_selected(str);
	g_free(str);

 END:
	if(selection_mode != SELECTION_DO_NOTHING){
		auto_lookup_start();
	}

	LOG(LOG_DEBUG, "OUT : copy_clipboard()");
	return (FALSE);
}

gint copy_clipboard_x(gpointer data){
	static GdkAtom ctext_atom = GDK_NONE;

	LOG(LOG_DEBUG, "IN : copy_clipboard()");

	/*
	xwindow = XGetSelectionOwner (gdk_display_get_default(), GDK_SELECTION_PRIMARY);
	if (xwindow == None){
		return(TRUE);
	}
	*/

	gtk_entry_set_text(GTK_ENTRY(hidden_entry), "");

	// Ask for COMPOUND_TEXT
	if (ctext_atom == GDK_NONE){
		ctext_atom = gdk_atom_intern ("COMPOUND_TEXT", FALSE);
	}

#ifdef __WIN32__
	gtk_selection_convert (hidden_entry,
			       GDK_SELECTION_CLIPBOARD,
			       ctext_atom, 
			       GDK_CURRENT_TIME);
#else
	gtk_selection_convert (hidden_entry,
			       GDK_SELECTION_PRIMARY,
			       ctext_atom, 
			       GDK_CURRENT_TIME);
#endif
	
	LOG(LOG_DEBUG, "OUT : copy_clipboard()");
	return (FALSE);
}


#ifdef __WIN32__
static gboolean registered=FALSE;
static WNDPROC OrgWndProc = NULL;
static HWND next_hwnd = NULL;
static HWND hidden_hwnd;

LRESULT CALLBACK
HiddenWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT retval;
    HANDLE hText;
    char *pText;
    gchar *str;


    switch (msg) {
    case WM_DESTROY:
	    ChangeClipboardChain(hwnd , next_hwnd);
	    PostQuitMessage(0);
	    break;
    case WM_DRAWCLIPBOARD:
	    OpenClipboard(hwnd);
	    hText = GetClipboardData(CF_TEXT);

	    if(hText != NULL) {
		    pText = GlobalLock(hText);
		    GlobalUnlock(hText);
	    }
	    CloseClipboard();

	    //remove_space(pText);
	    str = iconv_convert(fs_codeset, "utf-8", pText);
	    search_selected(str);
	    g_free(str);

	    if(next_hwnd != NULL)
		    SendMessage(next_hwnd, msg, wParam, lParam);
	    break;
    case WM_CHANGECBCHAIN:
	    if((HWND)wParam == next_hwnd)
		    next_hwnd = (HWND)lParam;
	    break;
    default:
	    break;
    }

    retval = OrgWndProc(hwnd, msg, wParam, lParam);
    return retval;
}

#endif

void auto_lookup_start()
{
	
	if(selection_mode == SELECTION_DO_NOTHING)
		return;
	
#ifdef __WIN32__
	if(registered == FALSE) {
		if(OrgWndProc == NULL){
			hidden_hwnd = GDK_WINDOW_HWND (hidden_window->window);
			OrgWndProc = (WNDPROC)GetWindowLong(hidden_hwnd, GWL_WNDPROC);
			SetWindowLong(hidden_hwnd, GWL_WNDPROC, (LONG)HiddenWndProc);
		}
		next_hwnd = SetClipboardViewer(hidden_hwnd);
	}
	registered = TRUE;
#else

	if(tag_timeout != 0)
		gtk_timeout_remove(tag_timeout);
	tag_timeout = gtk_timeout_add(auto_interval, copy_clipboard_x, NULL);

#endif
	auto_lookup_suspended = FALSE;
}

void auto_lookup_stop()
{

#ifdef __WIN32__
	if(registered == TRUE) {
		hidden_hwnd = GDK_WINDOW_HWND (hidden_window->window);
		ChangeClipboardChain(hidden_hwnd, next_hwnd);
		next_hwnd = NULL;
		registered = FALSE;
	}
#else
	if(tag_timeout != 0)
		gtk_timeout_remove(tag_timeout);
	tag_timeout = 0;
#endif

	auto_lookup_suspended = FALSE;
}

void auto_lookup_suspend()
{
	if((selection_mode != SELECTION_DO_NOTHING) && (auto_lookup_suspended == FALSE)) {
		auto_lookup_stop();
		auto_lookup_suspended = TRUE;
	}
}

void auto_lookup_resume()
{
	if((selection_mode != SELECTION_DO_NOTHING) && (auto_lookup_suspended == TRUE)) {
		auto_lookup_start();
		auto_lookup_suspended = FALSE;
	}
}
