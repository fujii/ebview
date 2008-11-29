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

#ifndef __BMH_H__
#define __BMH_H__

#include "defs.h"

#define MAX_CHAR 256

typedef struct {
	guchar skip[MAX_CHAR];
	guchar *pat;
	guchar *u_pat;
	guchar *l_pat;
	gint length;
	gboolean ignore_case;
} BMH_TABLE;

BMH_TABLE *bmh_prepare(guchar *pat, gboolean ignore_case);
void bmh_free(BMH_TABLE *table);
guchar *bmh_search(BMH_TABLE *table, guchar *text, gint n);
guchar *simple_search(guchar *pat, guchar *text, gint n, gboolean ignore_case);

#endif /* __BMH_H__ */
