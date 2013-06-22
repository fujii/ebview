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
#include "eb.h"
#include <pthread.h>

#ifndef __WIN32__
#include <sys/wait.h>
#endif

gint launch_external(gchar *cmd, gboolean wait){

	gint status;
	GError *error = NULL;
	gboolean ok;
	if (wait) {
		ok = g_spawn_command_line_sync(cmd, NULL, NULL, &status, &error);
		if (!ok)
			popup_warning(_("Failed to execute command. Please check setting."));
		if (ok) {
			LOG(LOG_DEBUG, "Child exited with status %d\n", status);
		}
		return status;
	} else {
		ok = g_spawn_command_line_async(cmd, &error);
		if (!ok)
			popup_warning(_("Failed to execute command. Please check setting."));
		return 0;
	}
}

#ifdef __WIN32__
static gchar *g_filename=NULL;
static pthread_t tid;

static void *play_background(void *arg)
{
	MCI_OPEN_PARMS open_parms;
	MCI_PLAY_PARMS play_parms;
	MCI_GENERIC_PARMS parms;

	open_parms.wDeviceID = 0;
	open_parms.lpstrDeviceType = "waveaudio";
	open_parms.lpstrElementName = g_filename;
	if(mciSendCommand(0,MCI_OPEN,MCI_WAIT|MCI_OPEN_TYPE|MCI_OPEN_ELEMENT,(DWORD)&open_parms) == 0){
		play_parms.dwFrom = 0;
		mciSendCommand(open_parms.wDeviceID,MCI_PLAY,MCI_WAIT|MCI_FROM,(DWORD)&play_parms);
		mciSendCommand(open_parms.wDeviceID,MCI_STOP,MCI_WAIT,(DWORD)&parms);
		mciSendCommand(open_parms.wDeviceID,MCI_CLOSE,MCI_WAIT,(DWORD)NULL);
	}
}
#endif

void play_multimedia(gchar *filename, gint type)
{
	gchar cmd[512];
	gchar *p;
	gchar *template;

	LOG(LOG_DEBUG, "IN : play_multimedia(%s, %d)", filename, type);

	switch(type){
	case TAG_TYPE_MOVIE:
		template = strdup(mpeg_template);
		break;
	case TAG_TYPE_SOUND:
#ifdef __WIN32__
		if(bplay_sound_internally){
			pthread_attr_t thread_attr;
			gint rc;

			if(g_filename)
				g_free(g_filename);
			g_filename = g_strdup(filename);

			pthread_attr_init (&thread_attr) ;
			pthread_attr_setstacksize (&thread_attr, 512*1024) ;

			LOG(LOG_DEBUG, "thread_create");
			rc = pthread_create(&tid, &thread_attr, play_background, (void *)NULL);
			if(rc != 0){
				LOG(LOG_CRITICAL, "pthread_create: %s", strerror(errno));
				LOG(LOG_DEBUG, "OUT : thread_search()");
				exit(1);
			}
			LOG(LOG_DEBUG, "thread_created");
			pthread_attr_destroy(&thread_attr);

			return;
		}
#endif
		template = strdup(wave_template);
		break;
	default:
		return;
		break;
	}

	if((template == NULL) || 
	   (strlen(template) == 0)){
#ifdef __WIN32__
		HINSTANCE hi;
		hi = ShellExecute(NULL, "open", filename, NULL, NULL, SW_SHOWNORMAL);
		if(hi <= 32){
			LOG(LOG_CRITICAL, "ShellExecute() failed : %d", hi);
		}
#else
		if(template)
			g_free(template);
#endif
		LOG(LOG_DEBUG, "OUT : play_multimedia()");
		return;
	}

	p = strstr(template, "%f");
	if(p != NULL){
		*p = '%';
		p++;
		*p = 's';
	}

	sprintf(cmd, template, filename);

	launch_external(cmd, FALSE);

	free(template);

	LOG(LOG_DEBUG, "OUT : play_multimedia()");
}

void launch_web_browser(gchar *url){
	gchar *p;
	gchar *template;
	gchar cmd[512];


	LOG(LOG_DEBUG, "IN : launch_web_browser()");

	if((browser_template == NULL) || 
	   (strlen(browser_template) == 0)){
#ifdef __WIN32__
		HINSTANCE hi;
		hi = ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
		if(hi <= 32){
			LOG(LOG_CRITICAL, "ShellExecute() failed : %d", hi);
		}
#else
		LOG(LOG_CRITICAL, _("Web browser not set"));
#endif
		return;
	}

	template = strdup(browser_template);
	p = strstr(template, "%f");
	if(p != NULL){
	  *p = '%';
	  p++;
	  *p = 's';
	}
	sprintf(cmd, template, url);

	launch_external(cmd, FALSE);
	g_free(template);

	LOG(LOG_DEBUG, "OUT : launch_web_browser()");
}

