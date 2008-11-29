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

#include <stdio.h>
#include <stdarg.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>

#include "dialog.h"
#include "log.h"

#define STDERR stderr


gint ebview_log_level = LOG_MESSAGE;
//gint ebview_log_level = LOG_DEBUG;

void set_log_level(gint level){
	ebview_log_level = level;
}

void log_func(const gchar *file, gint line, LOG_LEVEL level, const gchar *message, ...){
	va_list ap;
	gchar format[1024];
	gchar str[1024];

	if(level <= ebview_log_level) {
		switch(level){
		case LOG_ERROR:
			sprintf(format, "%s:%d ERROR : ", file, line);
			break;
		case LOG_CRITICAL:
			sprintf(format, "%s:%d CRITICAL : ", file, line);
			break;
		case LOG_WARNING:
			sprintf(format, "%s:%d WARNING : ", file, line);
			break;
		case LOG_MESSAGE:
			sprintf(format, "%s:%d MESSAGE : ", file, line);
			break;
		case LOG_INFO:
			sprintf(format, "%s:%d INFO : ", file, line);
			break;
		case LOG_DEBUG:
			sprintf(format, "%s:%d DEBUG : ", file, line);
			break;
		}

		strcat(format, message);

		va_start(ap, message);
		g_vprintf(format, ap);
		g_printf("\n");
		//g_logv(G_LOG_DOMAIN, level, format, ap);
	}

	// Show dialog box.

	if(level <= LOG_MESSAGE){
		vsprintf(str, message, ap);

		switch(level){
		case LOG_ERROR:
		case LOG_CRITICAL:
			popup_error(str);
			break;
		case LOG_WARNING:
		case LOG_MESSAGE:
			popup_warning(str);
			break;
		default:
			break;
		}

		va_end(ap);
	}
}


