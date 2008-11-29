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

#ifndef __JCODE_H__
#define __JCODE_H__

#include "defs.h"
#include "global.h"

enum {
	KCODE_EUC,
	KCODE_JIS,
	KCODE_SJIS,
	KCODE_ASCII,
	KCODE_UNKNOWN
};

gchar *iconv_convert(const gchar *icode, const gchar *ocode, const gchar *inbuf);
gchar *iconv_convert2(const gchar *icode, const gchar *ocode, const gchar *orig);
inline gboolean isjisp(const gchar *buff);
gboolean iseuckanji(const guchar *buff);
gboolean iseuchiragana(const guchar *buff);
gboolean iseuckatakana(const guchar *buff);
gboolean iseuc(const guchar *buff);
gint guess_kanji(gint imax, guchar *buf);
void katakana_to_hiragana(gchar *word);
void hiragana_to_katakana(gchar *word);


void hex_dump(const gchar *buf);



#define  _EUC(str)  euc2locale(str)
#define  _LOCALE(str)  euc2locale(str)

#ifndef HAVE_ICONV_H
#error iconv() required!
#endif

#endif /* __JCODE_H__ */
