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
#include "reg.h"

REG_TABLE *regex_prepare(guchar *pat, gboolean ignore_case)
{
	REG_TABLE *reg;

	reg = g_new(REG_TABLE, 1);

	if(ignore_case == TRUE){
		if(0 != regcomp(reg, pat, REG_ICASE|REG_EXTENDED)){
			LOG(LOG_CRITICAL, "regcomp: %s", strerror(errno));
			g_free(reg);
			return(NULL);
		}
	} else {
		if(0 != regcomp(reg, pat, REG_EXTENDED)){
			LOG(LOG_CRITICAL, "regcomp: %s", strerror(errno));
			g_free(reg);
			return(NULL);
		}
	}

	return(reg);
}

void regex_free(REG_TABLE *reg)
{
	regfree(reg);
	g_free(reg);
}

guchar *regex_search(REG_TABLE *reg, guchar *text){
	regmatch_t pmatch[1];

	if(REG_NOMATCH == regexec(reg, text, 1, pmatch, 0)){
		return(FALSE);
	} else {
		if(pmatch[0].rm_so == -1){
			return(NULL);
		}
		return(text+pmatch[0].rm_so);
	}
}

