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
#include "dialog.h"
#include "external.h"

static GList *grep_file_list=NULL;

static void check_cache_size();

static gchar *create_dir_hier(gchar *path)
{
	gchar *p;
	gchar *p0;
	
	gchar buff[512];
	gchar parent[512];
	gint r;

	LOG(LOG_DEBUG, "IN : create_dir_hier(%s)", path);

#ifdef __WIN32__
	if(path[0] == '\\')
		strcpy(buff, &path[1]);
	else if((path[1] == ':') && (path[2] == '\\')){
		buff[0] = path[0];
		strcpy(&buff[1], &path[2]);
	} else
		strcpy(buff, path);
#else
	if(path[0] == '/')
		strcpy(buff, &path[1]);
	else
		strcpy(buff, path);
#endif

	p = buff;
	p0 = buff;
	strcpy(parent, cache_dir);

	while(1){
		if(*p == '\0') {
			strcat(parent, DIR_DELIMITER);
			strcat(parent, p0);
			break;
		}
#ifdef __WIN32__
		if(*p == '\\')
#else
		if(*p == '/')
#endif
		{
			*p = '\0';
			strcat(parent, DIR_DELIMITER);
			strcat(parent, p0);
#ifdef __WIN32__
			r = mkdir(parent);
#else
			r = mkdir(parent, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
#endif
			if((r != 0) && (errno != EEXIST)){
				LOG(LOG_DEBUG, "OUT : create_dir_hier() = NULL : %d", errno);
				return(NULL);
			}
			p0 = p = p + 1;
		} else 
			p++;
	}

	strcat(parent, ".cache");

	LOG(LOG_DEBUG, "OUT : create_dir_hier() = %s", parent);
	return(strdup(parent));
}

gboolean match_extension(gchar *filename, gchar  *exts)
{
	gchar *p;
	gchar ext[512];
	gint i;

	LOG(LOG_DEBUG, "IN : match_extension(%s, %s)", filename, exts);

	i = 0;
	for(p=exts, i=0; ; p++) {
		if(*p == ' ')
			continue;
		else if ((*p == ',')){
			ext[i] = '\0';

			if((strlen(filename) >= strlen(ext)) &&
			   (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0))
			{
				LOG(LOG_DEBUG, "OUT : match_extension() = TRUE");
				return(TRUE);
			} else {
				i = 0;
			}
			continue;
		} else if ((*p == '\0')){
			ext[i] = '\0';

			if((strlen(filename) >= strlen(ext)) &&
			   (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0))
			{
				LOG(LOG_DEBUG, "OUT : match_extension() = TRUE");
				return(TRUE);
			} else {
				LOG(LOG_DEBUG, "OUT : match_extension() = FALSE");
				return(FALSE);
			}
		}
		ext[i] = *p;
		i++;
	}
}

static gchar *get_filter_command(gchar *path)
{
	GtkTreeIter   iter;
	gint found = 0;
	gchar *ext;
	gchar *filter_command;

	LOG(LOG_DEBUG, "IN : get_filter_command(%s)", path);

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(filter_store), &iter) == TRUE){
		do { 
			gtk_tree_model_get(GTK_TREE_MODEL(filter_store), 
					   &iter, 
					   FILTER_EXT_COLUMN, &ext,
					   FILTER_FILTER_COMMAND_COLUMN, &filter_command,
					   -1);

			if(match_extension(path, ext) == TRUE){
				found = 1;
				break;
			}
			g_free(ext);
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(filter_store), &iter) == TRUE);
	}

	if(found == 1){
		if((filter_command == NULL) || (strlen(filter_command) == 0)){
			LOG(LOG_DEBUG, "OUT : get_filter_command() = NULL");
			return(NULL);
		}
		LOG(LOG_DEBUG, "OUT : get_filter_command()");
		return(filter_command);
	} else {
		LOG(LOG_DEBUG, "OUT : get_filter_command() = NULL");
		return(NULL);
	}

}

gchar *get_cache_file(gchar *path)
{
	gchar *outfile;
	gchar *command;
	struct stat istat;
	struct stat ostat;
	gchar buff[512];
	gint i;
	gint r;
	gchar *p;
	gint length;
	GError *error=NULL;
	gchar *contents;
	gchar tmpout[512];

	LOG(LOG_DEBUG, "IN : get_cache_file(%s)", path);

	command = get_filter_command(path);
	if(command == NULL)
	{
		// No filter defined.
		// Regard as as text and read original file.
		if(g_file_get_contents(path, &contents, &length, &error) == FALSE){
			LOG(LOG_DEBUG, "OUT : get_cache_file() = NULL");
			return(NULL);
		}
		LOG(LOG_DEBUG, "OUT : get_cache_file() = NOP");
		return(contents);
	}

	outfile = create_dir_hier(path);
	if(outfile == NULL){
		LOG(LOG_CRITICAL, "Failed to create cache for %s", path);
		push_message("failed to create cache.\n");
		return(NULL);
	}

	r = stat(path, &istat);
	if(r != 0) {
		// Cannot happen.
		LOG(LOG_CRITICAL, "file %s does not exist", path);
		push_message("unknown error.\n");
		g_free(command);
		return(NULL);
	}

	r = stat(outfile, &ostat);
	
	if(r != 0) {
		// No file
		;
	} else if((ostat.st_size != 0) &&(
		ostat.st_ctime > MAX(istat.st_ctime, istat.st_mtime))){
		// Cache is newer.
		LOG(LOG_DEBUG, "cache file is new");
		g_free(command);

		if(g_file_get_contents(outfile, &contents, &length, &error) == FALSE){
			LOG(LOG_DEBUG, "OUT : get_cache_file() = NULL");
			push_message("failed to read.\n");
			return(NULL);
		}
		LOG(LOG_DEBUG, "OUT : get_cache_file()");
		return(contents);
	} else {
		if(unlink(outfile) != 0){
			g_free(command);
			LOG(LOG_CRITICAL, "Failed to unlink %s", outfile);
			push_message("failed to unlink cache.\n");
			return(NULL);
		}
	}

	// Apply filter

	sprintf(tmpout, "%s%s%s", temp_dir, DIR_DELIMITER, "filter.tmp");
	unlink(tmpout);

	p = command;
	i = 0;
	while(1){
		if (*p == '\0'){
			buff[i] = '\0';
			break;
		}
		if(*p == '%'){
			switch (*(p+1)){
			case 'f':
#ifdef __WIN32__
				buff[i] = '\"';
				i++;
#endif
				strcpy(&buff[i], path);
				i = i + strlen(path);
#ifdef __WIN32__
				buff[i] = '\"';
				i++;
#endif
				p = p + 2;
				break;
			case 'o':
#ifdef __WIN32__
				buff[i] = '\"';
				i++;
#endif
				strcpy(&buff[i], tmpout);
				i = i + strlen(tmpout);
#ifdef __WIN32__
				buff[i] = '\"';
				i++;
#endif
				p = p + 2;
				break;
			}
		} else {
			buff[i] = *p;
			p++;
			i++;
		}
	}

	r = launch_external(buff, TRUE);

	check_cache_size();
	
	if(r == 0){
		rename(tmpout, outfile);
		if(g_file_get_contents(outfile, &contents, &length, &error) == FALSE){
			LOG(LOG_DEBUG, "OUT : get_cache_file() = NULL");
			push_message("failed to read.\n");
			return(NULL);
		}
		LOG(LOG_DEBUG, "OUT : get_cache_file()");
		return(contents);
	} else {
		LOG(LOG_DEBUG, "OUT : get_cache_file() = NULL");
			push_message("failed to execute filter.\n");
		return(NULL);
	}
}

typedef struct {
	gchar *name;
	time_t ctime;
	off_t  size;
} CACHE_INFO;

static gint totalsize=0;

static void list_file_recursive(gchar *dirname, gint depth)
{
	GDir *dir;
	const gchar *name;
	gchar fullpath[512];
	struct stat fstat;
	gint r;
	CACHE_INFO *cache;

	
	if((dir = g_dir_open(dirname, 0, NULL)) == NULL){
		LOG(LOG_CRITICAL, "Failed to open directory %s", dirname);
		LOG(LOG_DEBUG, "OUT : list_file_recursive()");
		return;
	}

	while((name = g_dir_read_name(dir)) != NULL){
		if(strcmp(dirname,"/")==0){
			sprintf(fullpath,"/%s",name);
		} else {
			sprintf(fullpath,"%s%s%s",dirname, DIR_DELIMITER, name);
		}

		if(g_file_test(fullpath, G_FILE_TEST_IS_REGULAR) == TRUE){
			r = stat(fullpath, &fstat);
			if(r != 0){
				LOG(LOG_CRITICAL, "Failed to stat file %s : %s", fullpath, strerror(errno));
				return;
			}
			cache = g_new(CACHE_INFO, 1);
			cache->name = strdup(fullpath);
			cache->ctime = fstat.st_ctime;
			cache->size = fstat.st_size;
			grep_file_list = g_list_append(grep_file_list, cache);
			totalsize += cache->size;

		} else if(g_file_test(fullpath, G_FILE_TEST_IS_DIR) == TRUE){
				list_file_recursive(fullpath, depth+1);
		}
	}
	g_dir_close(dir);
}

static gint compare_func(gconstpointer a, gconstpointer b){
	if(((CACHE_INFO *)(a))->ctime < ((CACHE_INFO *)(b))->ctime)
		return(-1);
	else
		return(1);
}


static void check_cache_size()
{
	GList *l;

	LOG(LOG_DEBUG, "IN : check_cache_size()");

	// Create file list.
	// First, clear.
	l = g_list_first(grep_file_list);
	while(l != NULL){
		g_free(((CACHE_INFO *)(l->data))->name);
		g_free(l->data);
		l = g_list_next(l);
	}
	if(grep_file_list) {
		g_list_free(grep_file_list);
	}
	grep_file_list = NULL;

	totalsize=0;

	// Search recursively
	list_file_recursive(cache_dir, 0);

	// Do nothing if within maximum cache size.
	if(totalsize < cache_size * 1000000){
		LOG(LOG_DEBUG, "OUT : check_cache_size() = NOP");
		return;
	}

	// Sort by date
	g_list_sort(grep_file_list, compare_func);


	l = g_list_first(grep_file_list);
	while(totalsize >= cache_size * 1000000){
		if(l == NULL)
			break;
		LOG(LOG_DEBUG, "unlink %s",((CACHE_INFO *)(l->data))->name);
		unlink(((CACHE_INFO *)(l->data))->name);
		totalsize -= ((CACHE_INFO *)(l->data))->size;
		l = g_list_next(l);
	}

	LOG(LOG_DEBUG, "OUT : check_cache_size()");
}
