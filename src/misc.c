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


void beep(){
#ifdef __WIN32__
	Beep(1000, 200);
#else
	FILE *fp;

	if(!bbeep_on_nohit)
		return;

	fp = fopen("/dev/console", "w");
	if(fp == NULL){
		return;
	}
	fprintf(fp, "\a");
	fclose(fp);
#endif
}


void remove_space(gchar *f){
	char *p;
	int i;

	p = f;

	// Delete preceding spaces.
	for(i=0;;i++){
		if((f[i] == ' ') || (f[i] == '\t') || (f[i] == '\n') ||
		   (f[i] == ',') || (f[i] == '.') || (f[i] == '!') ||
		   (f[i] == ':') || (f[i] == ';') || (f[i] == '*') ||
		   (f[i] == '(') || (f[i] == ')') )
			p++;
		else
			break;
	}
	strcpy(f,p);

	// Delete following saces
	for(i=(strlen(p) -1);;i--){
		if((f[i] == ' ') || (f[i] == '\t') || (f[i] == '\n') ||
		   (f[i] == ',') || (f[i] == '.') || (f[i] == '!') ||
		   (f[i] == ':') || (f[i] == ';') || (f[i] == '*') ||
		   (f[i] == '(') || (f[i] == ')') )
			f[i] = '\0';
		else
			break;
	}

/*
	if((p = (char *)index(p, '\n')) != NULL)
		*p = '\0';
*/
}

#ifdef __WIN32__
char *getdcwd(int drv, char *path, int len){
	return(_getdcwd(drv, path, len));
}
#endif

gboolean find_file(gchar *filename)
{
#if 0	
	struct stat buf;
	gint rc;
#endif

	LOG(LOG_DEBUG, "IN : find_file(%s)", filename);
#if 0	
	rc = stat(filename, &buf);
	if(rc != 0) {
		LOG(LOG_DEBUG, "OUT : find_file() = FALSE");
		return(FALSE);
	}

	LOG(LOG_DEBUG, "OUT : find_file() = TRUE");
	return(TRUE);
#endif
	if(g_file_test(filename, G_FILE_TEST_IS_REGULAR) || g_file_test(filename, G_FILE_TEST_IS_SYMLINK)){
		LOG(LOG_DEBUG, "OUT : find_file() = TRUE");
		return(TRUE);
	} else {
		LOG(LOG_DEBUG, "OUT : find_file() = FALSE");
		return(FALSE);
	}

}

gboolean find_config_file(gchar *filename){
	gchar fullpath[512];
	gboolean ret;

	LOG(LOG_DEBUG, "IN : find_config_file(%s)", filename);

	sprintf(fullpath, "%s%s%s", user_dir, DIR_DELIMITER, filename);

        ret = find_file(fullpath);

	LOG(LOG_DEBUG, "OUT : find_config_file()");
	return(ret);
}

gboolean find_or_copy_file(gchar *filename)
{

	gchar fullpath[512];

	LOG(LOG_DEBUG, "IN : find_or_copy_file(%s)", filename);

	sprintf(fullpath, "%s%s%s", user_dir, DIR_DELIMITER, filename);

	if(find_file(fullpath) == FALSE){
		// Copy default file
		gchar srcfile[512];
		gchar command[512];
#ifdef __WIN32__
		sprintf(srcfile, "%s%s%s", package_dir, DIR_DELIMITER, filename);
		sprintf(command, "copy \"%s%s%s\" \"%s\"", package_dir, DIR_DELIMITER, filename, fullpath);
#else
		sprintf(srcfile, "%s%s%s", package_dir, DIR_DELIMITER, filename);
		sprintf(command, "cp %s%s%s %s", package_dir, DIR_DELIMITER, filename, fullpath);
#endif
		if(find_file(srcfile) != TRUE){
			LOG(LOG_CRITICAL, _("Couldn't find %s. Check installation."), filename);
			LOG(LOG_DEBUG, "IN : find_or_copy_file() = FALSE");
			return(FALSE);
		}

#ifdef __WIN32__
		CopyFile(srcfile, fullpath, TRUE);
#else
		system(command);
#endif

		// If file does not exist after copy, then error.
		if(find_file(fullpath) == FALSE){
#ifndef __WIN32__
			LOG(LOG_CRITICAL, _("Couldn't open %s. Check installation."), filename);
#endif
			LOG(LOG_DEBUG, "IN : find_or_copy_file() = FALSE");
			return(FALSE);
		}
	}

	LOG(LOG_DEBUG, "IN : find_or_copy_file() = TRUE");
	return(TRUE);
}


