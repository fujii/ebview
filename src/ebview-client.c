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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __WIN32__
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#endif

#include "../config.h"

int main(int argc, char **argv) {
#ifdef __WIN32__
	printf("Not supported on Windows\n");
	exit(1);
#else
	struct sockaddr_un address;
	int sock;
	size_t addrLength;
	char buff[512];
	char *p;
	int i;
	int len, write_len;
	pid_t pid;

	if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}

	address.sun_family = AF_UNIX;    /* Unix domain socket */
//	strcpy(address.sun_path, "./sample-socket");
	sprintf(address.sun_path, "%s/.%s/.remote-sock", getenv("HOME"), PACKAGE);

	/* The total length of the address includes the sun_family 
	   element */

#ifdef __FreeBSD__
        addrLength = sizeof(address.sun_len) + sizeof(address.sun_family) + strlen(address.sun_path) + 1;
        address.sun_len = addrLength;
#else
        addrLength = sizeof(address.sun_family) + strlen(address.sun_path);
#endif

	if (connect(sock, (struct sockaddr *) &address, addrLength)){
		perror("connect");
		goto LAUNCH_NEW;
	}

	p = &buff[1];
	*p = argc;
	p++;

	for(i=0 ; i < argc ; i ++){
		strcpy(p, argv[i]);
		p = p + strlen(argv[i]) + 1;
	}

	len = p - buff - 1;

	if(len >= 256){
		printf("String too long\n");
		close(sock);
		exit(1);
	}

	buff[0] = (unsigned char )len;

	printf("Sending %d bytes of data...", len);

	write_len = write(sock, buff, len+1);
	if(write_len != len+1){
		perror("write");
		printf("Write failed\n");
	}

	close(sock);
	printf("done\n");
    
	return 0;

 LAUNCH_NEW:

	pid = fork();
	if(pid == -1){
		perror("fork");
		exit(1);
	}

	if(pid == 0){
		execvp("ebview", argv);
	} else {
		// Parent
		exit(0);
	}

	return(0);
#endif
}
