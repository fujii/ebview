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

#ifndef __PREF_IO_H__
#define __PREF_IO_H__

#include "defs.h"

gboolean load_preference();
gboolean save_preference();
gboolean load_dictgroup();
gboolean save_dictgroup();
gboolean load_stemming_en();
gboolean load_stemming_ja();
gboolean save_stemming_en();
gboolean save_stemming_ja();
gboolean load_shortcut();
gboolean save_shortcut();
gboolean load_weblist();
gboolean save_weblist();
gboolean load_history();
gboolean save_history();
gboolean load_dirlist();
gboolean save_dirlist();
gboolean load_filter();
gboolean save_filter();
gboolean load_dirgroup();
gboolean save_dirgroup();

#endif /* __PREF_IO_H__ */
