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

#ifndef __LOG_H__
#define __LOG_H__

typedef enum {
	LOG_ERROR     = 1 << 2,
	LOG_CRITICAL  = 1 << 3,
	LOG_WARNING   = 1 << 4,
	LOG_MESSAGE   = 1 << 5,
	LOG_INFO      = 1 << 6,
	LOG_DEBUG     = 1 << 7,
} LOG_LEVEL;

extern gint ebview_log_level;

#define LOG(...) { if (ebview_log_level) log_func (__FILE__, __LINE__, __VA_ARGS__); } 

void set_log_level(gint level);
void log_func(const gchar *file, gint line, LOG_LEVEL level, const gchar *message, ...);

#endif /* __LOG_H__ */
