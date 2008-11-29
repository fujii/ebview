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
#include "bmh.h"

/*
Search using BMH method.
pat : pattern
text : text
n : length of text
*/

//static guchar skip[MAX_CHAR];
//static gint m = 0;

BMH_TABLE *bmh_prepare(guchar *pat, gboolean ignore_case)
{
	gint k;
	BMH_TABLE *table;
	gint i;

	LOG(LOG_DEBUG, "IN : bmh_prepare(%s, %d)", pat, ignore_case);

	table = g_new(BMH_TABLE, 1);

	table->pat = g_strdup(pat);
	table->u_pat = NULL;
	table->l_pat = NULL;
	table->length = strlen(pat);
	table->ignore_case = ignore_case;

	// skip[x] will be the length of chars after the occurrence of x in pat.
	// Is x does not occur, it will be the length of pat.
	for( k=0; k<MAX_CHAR; k++ )
		table->skip[k] = table->length;
	for( k=0; k<table->length - 1; k++ ){
		if(ignore_case == TRUE){
			table->skip[toupper(pat[k])] = table->length - k - 1;
			table->skip[tolower(pat[k])] = table->length - k - 1;
		} else {
			table->skip[pat[k]] = table->length - k - 1;
		}
	}

	if(ignore_case == TRUE){
		table->u_pat = g_new(guchar, table->length+1);
		table->u_pat[table->length] = '\0';
		table->l_pat = g_new(guchar, table->length+1);
		table->l_pat[table->length] = '\0';
		for(i=0 ; i < table->length ; i++){
			table->u_pat[i] = toupper(table->pat[i]);
			table->l_pat[i] = tolower(table->pat[i]);
		}
	}

	LOG(LOG_DEBUG, "OUT : bmh_prepare()");

	return(table);
}

void bmh_free(BMH_TABLE *table)
{
	if(table == NULL)
		return;
	
	if(table->pat)
		g_free(table->pat);

	if(table->u_pat)
		g_free(table->u_pat);

	if(table->l_pat)
		g_free(table->l_pat);

	g_free(table);
}

guchar *bmh_search(BMH_TABLE *table, guchar *text, gint n)
{
	gint i, j, k;

	if(table->length==0)
		return(text);

	// k will be the last chars after matching first char.
	// Check from the last characters.
	// If it does not match shift by the value of skip table.
	for(k=table->length-1; k < n; k += table->skip[text[k] & (MAX_CHAR-1)] ) {
		for(j=table->length-1, i=k; j>=0 ; j--){
			if(table->ignore_case == TRUE) {
				if((text[i] != table->u_pat[j]) && 
				   (text[i] != table->l_pat[j]))
					break;
			} else {
				if(text[i] != table->pat[j])
					break;
			}
			i--;
		}
		if(j == (-1))
			return(text+i+1);
	}
	return(NULL);
}

guchar *simple_search(guchar *pat, guchar *text, gint n, gboolean ignore_case)
{
	gint i, j, k, m;

	m = strlen(pat);
	if( m==0 ) return( text );

	for( k=m-1; k < n; k ++) {
		for( j=m-1, i=k; j>=0 ; j-- ){
			if(ignore_case == TRUE) {
				if(toupper(text[i]) != toupper(pat[j]))
					break;
			} else {
				if(text[i] != pat[j])
					break;
			}
			i--;
		}
		if( j == (-1) ) return( text+i+1 );
	}
	return( NULL );
}


