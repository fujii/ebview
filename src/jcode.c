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

#include "jcode.h"
#include <iconv.h>

void hex_dump(const gchar *buf){

	const gchar *p =buf;

	while(*p != '\0'){
		g_print("%02x ", (unsigned char)*p);
		p++;
	}
	g_print("\n");
}

// Translation talbe.
// Comment : Ku and Ten
static gchar *replace_table[] = {
	"(1)", "(2)", "(3)", "(4)", "(5)", "(6)", "(7)", "(8)", // 13-01 - 08
	"(9)", "(10)", "(11)", "(12)", "(13)", "(14)", "(15)", "(16)", // 13-09 - 16
	"(17)", "(18)", "(19)", "(20)", "I", "II", "III", "IV", // 13-17 - 24
	"V", "VI", "VII", "VIII", "IX", "X", NULL, "ミリ", // 13-25 - 32
	"キロ", "センチ", "メートル", "グラム", "トン", "アール", "ヘクタール", "リットル",  // 13-33 - 40
	"ワット", "カロリー", "ドル", "セント", "パーセント", "ミリバール" "ページ", "mm", // 13-41 - 48
	"cm", "km", "mg", "kg", "cc", "m2", NULL, NULL, // 13-49 - 56
	NULL, NULL, NULL, NULL, NULL, NULL, "平成", "\"", // 13-57 - 64
	"\"", "No.", "K.K.", "TEL", "(上)", "(中)", "(下)", "(左)", // 13-65 - 72
	"(右)", "(株)", "(有)", "(代)", "明治", "大正", "昭和",	NULL, // 13-73 - 80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 13-81 - 88
	NULL, NULL, NULL, NULL // 13-89 - 92
 };

void replace_char(const guchar *icode, const guchar *ocode, guchar **inbuf, guchar **outbuf, gint *isize, gint *osize){
	guchar *in, *out;
	guchar *str;
	guchar *utf_str;
	gint len;

	in = *inbuf;
	out = *outbuf;

	if(strcasecmp(icode, "euc-jp") == 0) {
		if((in[0] != 0xad) || (in[1] < 0xa1) || (in[1] > 0xfc))
			goto UNKNOWN;
	
		str = replace_table[in[1] - 0xa1];
		if(str == NULL)
			goto UNKNOWN;

	} else if(strcasecmp(icode, "shift_jis") == 0) {
		if((in[0] != 0x87) || (in[1] < 0x40) || (in[1] > 0x9c))
			goto UNKNOWN;
	
		str = replace_table[in[1] - 0x40];
		if(str == NULL)
			goto UNKNOWN;

	} else if(strcasecmp(icode, "iso-2022-jp") == 0){
		if((in[0] != 0x2d) || (in[1] < 0x21) || (in[1] > 0x7c))
			goto UNKNOWN;
	
		str = replace_table[in[1] - 0x21];
		if(str == NULL)
			goto UNKNOWN;

	} else {
		goto UNKNOWN;
	}
	
	utf_str = iconv_convert("euc-jp", "utf-8", str);
	if(utf_str == NULL)
		goto UNKNOWN;
	len = strlen(utf_str);
	strcpy(out, utf_str);
	g_free(utf_str);

	*osize -= len;
	*isize -= 2;
	*inbuf += 2;
	*outbuf += len;
	
	return;

 UNKNOWN:

	// skip
	*isize -= 2;
	*inbuf += 2;

	return;
}

gchar *iconv_convert(const gchar *icode, const gchar *ocode, const gchar *orig){
	iconv_t cd;
	int r = 0;
	int buflen;
	size_t isize;
	size_t osize;
	guchar *outbuf;
	guchar *result;
	guchar *inbuf;
	size_t origsize;

	g_assert(icode != NULL);
	g_assert(ocode != NULL);
	g_assert(orig != NULL);

	//LOG(LOG_DEBUG, "IN : iconv_convert(%s, %s, %s)", icode, ocode, orig);

	if(strlen(orig) == 0){
		result = malloc(1);
		result[0] = '\0';
		LOG(LOG_DEBUG, "OUT : iconv_convert() OUT1");
		return(result);
	}

	if(strcasecmp(icode, ocode) == 0){
		LOG(LOG_DEBUG, "OUT : iconv_convert() OUT2");
		return(g_strdup(orig));
	}

	cd = iconv_open( ocode, icode );
	if( (int)cd == -1 ) {
		LOG(LOG_DEBUG, "OUT : iconv_convert() OUT3");
		return(NULL);
	}

	//hex_dump(inbuf);

	inbuf = (gchar *)orig;
		
	origsize = isize = strlen(inbuf);
	osize = buflen = isize * 2;
	result = outbuf = malloc(osize);
	memset(result, 0x00, osize);


	while(1){
#ifdef __FreeBSD__
		r = iconv(cd, (const char **)&inbuf, &isize, &outbuf, &osize);
#else
		r = iconv(cd, (char **)&inbuf, &isize, (char **)&outbuf, &osize);
#endif

#ifdef __WIN32__
		if((r == -1) && (errno == EILSEQ) && (strcasecmp(icode, "utf-8") == 0)){
			*outbuf = 0x5c;
			outbuf ++;

			osize -= 1;
			isize -= 1;
			
			inbuf += 1;
			continue;
		} 
#endif
		// Replace undefined characters
		if((r == -1) && (errno == EILSEQ)) {
			LOG(LOG_INFO, "Couldn't convert %02x %02x. Skipped.", (guchar)(*inbuf), (guchar)(*(inbuf+1)));
			replace_char(icode, ocode, &inbuf, &outbuf, &isize, &osize);
			continue;
		}
		break;
	}

	iconv_close(cd);

	if(r != 0){
		int i;

		LOG(LOG_INFO, "iconv failed at location %d (%s -> %s)", origsize - isize, icode, ocode);
		LOG(LOG_INFO, "original size %d", origsize);


		for(i=0; i < 10 ; i ++) {
			if(inbuf[i] == '\0')
				break;
			LOG(LOG_INFO, "[%02x](%c) ", (unsigned char)inbuf[i], (unsigned char)inbuf[i]);
		}
		LOG(LOG_CRITICAL, "iconv : %s", strerror(errno));
		result[0] = '\0';

		LOG(LOG_DEBUG, "OUT : iconv_convert() OUT4");
		return(result);
	}

	//LOG(LOG_DEBUG, "OUT : iconv_convert()");

	return(result);
}


gchar *iconv_convert2(const gchar *icode, const gchar *ocode, const gchar *orig){
	iconv_t cd;
	int r = 0;
	int buflen;
	size_t isize;
	size_t osize;
	char *outbuf;
	char *result;
	char *inbuf;

	g_assert(icode != NULL);
	g_assert(ocode != NULL);
	g_assert(orig != NULL);

	if(strcasecmp(icode, ocode) == 0){
		return(g_strdup(orig));
	}

	cd = iconv_open( ocode, icode );
	if( (int)cd == -1 ) {
		return(NULL);
	}


	inbuf = (gchar *)orig;
		
	isize = strlen(inbuf);
	osize = buflen = isize * 2;
	result = outbuf = malloc(osize);
	memset(result, 0x00, osize);

	while(1){
#ifdef __FreeBSD__
		r = iconv(cd, (const char **)&inbuf, &isize, &outbuf, &osize);
#else
		r = iconv(cd, &inbuf, &isize, &outbuf, &osize);
#endif
		if((r == -1) && (errno == EILSEQ)){
			LOG(LOG_INFO, "Couldn't convert %02x %02x. Skipped.", (guchar)(*inbuf), (guchar)(*(inbuf+1)));
			*outbuf = 0x20;
			outbuf ++;
			*outbuf = 0x20;
			outbuf ++;

			osize -= 2;
			isize -= 2;
			
			inbuf += 2;
		} else {
			break;
		}
	}

	iconv_close(cd);

	if(r != 0){
		LOG(LOG_CRITICAL, "iconv : %s", strerror(errno));
		result[0] = '\0';
		return(result);
	}

	//hex_dump(result);

	return(result);
}

inline gboolean isjisp(const gchar *buff){
        g_assert(buff != NULL);

        if((buff[0] >= 0x21) && (buff[0] <= 0x74) &&
           (buff[1] >= 0x21) && (buff[1] <= 0x7E))
                return(TRUE);
        return(FALSE);
}

gboolean iseuckanji(const guchar *buff){
	g_assert(buff != NULL);

	if((buff[0] >= 0xb0) && (buff[0] <= 0xf4) && 
	   (buff[1] >= 0xa1) && (buff[1] <= 0xfe))
		return(TRUE);
	return(FALSE);
}

gboolean iseuckatakana(const guchar *buff){
	g_assert(buff != NULL);

	if((buff[0] == 0xa5) &&
	   (buff[1] >= 0xa1) && (buff[1] <= 0xf6))
		return(TRUE);
	return(FALSE);
}

gboolean iseuchiragana(const guchar *buff){
	g_assert(buff != NULL);

	if((buff[0] == 0xa4) &&
	   (buff[1] >= 0xa1) && (buff[1] <= 0xf3))
		return(TRUE);
	return(FALSE);
}

void katakana_to_hiragana(gchar *word)
{
	gint i=0;

	g_assert(word != NULL);

	while(word[i] != '\0'){
		if(isalpha(word[i])) {
			i++;
			continue;
		}
		if(iseuc(&word[i])) {
			if(iseuckatakana(&word[i])) {
				word[i] = 0xa4;
			}
			i += 2;
			continue;
		}
		i++;
	}

}

void hiragana_to_katakana(gchar *word)
{
	gint i=0;

	g_assert(word != NULL);

	while(word[i] != '\0'){
		if(isalpha(word[i])) {
			i++;
			continue;
		}
		if(iseuc(&word[i])) {
			if(iseuchiragana(&word[i])) {
				word[i] = 0xa5;
			}
			i += 2;
			continue;
		}
		i++;
	}

}


gboolean iseuc(const guchar *buff){
	g_assert(buff != NULL);

	if((buff[0] >= 0xa1) && (buff[0] <= 0xf4) && 
	   (buff[1] >= 0xa1) && (buff[1] <= 0xfe))
		return(TRUE);
	return(FALSE);
}

/*
 * Copied from kf.c
 *
 Original author:
Haruhiko Okumura <okumura@matsusaka-u.ac.jp>
Copyright (c) 1995-2000 Haruhiko Okumura
 *
*/

#define JIS0208_1978  "\x1b\x24\x40"              // ESC $ @
#define JIS0208_1983  "\x1b\x24\x42"              // ESC $ B
#define JIS0208_1990  "\x1b\x26\x40\x1b\x24\x42"  // ESC & @ ESC $ B
#define JIS0212       "\x1b\x24\x28\x44"          // ESC $ ( D
#define JIS_ASC       "\x1b\x28\x42"              // ESC ( B
#define JIS_ASC2      "\x1b\x28\x44"              // ESC ( J
#define JIS_KANA      "\x1b\x28\x49"              // ESC ( I

#define isjis(c) (((c)>=0x21 && (c)<=0x7e))
#define iseuc(c) (((c)>=0xa1 && (c)<=0xfe))

/* First byte of ShiftJIS */
#define issjis1(c) (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xe0 && (c)<=0xef))

/* Second byte of ShiftJIS */
#define issjis2(c) ((c)>=0x40 && (c)<=0xfc && (c)!=0x7f)

/* 1-byte kana */
#define ishankana(c) ((c)>=0xa0 && (c)<=0xdf)


gint guess_kanji(gint imax, guchar *buf)
{
	int i, bad_euc, bad_sjis;
	for (i = 0; i < imax; i++) {
		if(buf[i+5] == '\0')
			break;
		if((strncmp(&buf[i], JIS0208_1978, strlen(JIS0208_1978)) == 0) ||
		   (strncmp(&buf[i], JIS0208_1983, strlen(JIS0208_1983)) == 0) ||
		   (strncmp(&buf[i], JIS0208_1990, strlen(JIS0208_1990)) == 0) ||
		   (strncmp(&buf[i], JIS0212, strlen(JIS0212)) == 0) ||
		   (strncmp(&buf[i], JIS_ASC, strlen(JIS_ASC)) == 0) ||
		   (strncmp(&buf[i], JIS_ASC2, strlen(JIS_ASC2)) == 0) ||
		   (strncmp(&buf[i], JIS_KANA, strlen(JIS_KANA)) == 0))
			return(KCODE_JIS);
	}

	bad_euc = 0;
	for (i = 0; i < imax; i++) {
		if(buf[i+2] == '\0')
			break;

		if (iseuc(buf[i]) && ++i < imax) {
			if (! iseuc(buf[i])) {  bad_euc += 10;  i--;  }
			else if (buf[i-1] >= 0xd0) bad_euc++; /* Dai 2 Suijun */
			/* 1999-02-01 bug fixed.  Thanks: massangeana */
		} else if (buf[i] == 0x8e && ++i < imax) {
			if (ishankana(buf[i])) bad_euc++;
			else {  bad_euc += 10;  i--;  }
		} else if (buf[i] >= 0x80) bad_euc += 10;
	}
	bad_sjis = 0;
	for (i = 0; i < imax; i++) {
		if(buf[i+2] == '\0')
			break;
		if (issjis1(buf[i]) && ++i < imax) {
			if (! issjis2(buf[i])) {  bad_sjis += 10;  i--;  }
			else if ((unsigned) (buf[i-1] * 256U + buf[i]) >= 0x989f)
				bad_sjis++;  /* Dai 2 Suijun */
		} else if (buf[i] >= 0x80) {
			if (ishankana(buf[i])) bad_sjis++;
			else                   bad_sjis += 10;
		}
	}

	if(bad_sjis < bad_euc)
		return(KCODE_SJIS);
	else if (bad_sjis > bad_euc)
		return(KCODE_EUC);
	else if ((bad_euc == 0) && (bad_sjis == 0))
		return(KCODE_ASCII);
	else
		return(KCODE_UNKNOWN);
}

