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

#ifndef __DIRTREE_H__
#define __DIRTREE_H__

GtkWidget *create_directory_tree();
void refresh_directory_tree();
GList *get_active_dir_list();
gchar *get_selected_directory();

gchar *native_to_generic(gchar *from);
gchar *generic_to_native(gchar *from);
gchar *fs_to_unicode(gchar *from);
gchar *unicode_to_fs(gchar *from);

#endif /* __DIRTREE_H__ */
