/*
 * This file is part of the Bus Pirate project (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project and http://dangerousprototypes.com
 *
 * To the extent possible under law, the project has
 * waived all copyright and related or neighboring rights to Bus Pirate. This
 * work is published from United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * OS independent serial interface
 *
 * Heavily based on Pirate-Loader:
 * http://the-bus-pirate.googlecode.com/svn/trunk/bootloader-v4/pirate-loader/source/pirate-loader.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>

#include "serial.h"

extern int disable_comport;
extern char *dumpfile;

int serial_setup(int fd, speed_t speed) {
    struct termios t_opt;

#ifdef NJC
    int s = tcgetattr(cm11, &oldsb);

    if (s < 0) {
      perror("ttopen tcgetattr");
      exit(1);
    }

    newsb = oldsb;

    newsb.c_iflag = IGNBRK | IGNPAR;
    newsb.c_oflag = 0;
    newsb.c_lflag = ISIG;
    newsb.c_cflag = (CLOCAL | B4800 | CS8 | CREAD);

    for (s = 0; s < NCC; s++) {
	newsb.c_cc[s] = 0;
    }
    
    newsb.c_cc[VMIN]   = 1;
    newsb.c_cc[VTIME]  = 0;

    tcsetattr(cm11, TCSADRAIN, &newsb);
#else
    /* set the serial port parameters */
    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &t_opt);

    cfsetispeed(&t_opt, speed);
    cfsetospeed(&t_opt, speed);

    t_opt.c_cflag |= (CLOCAL | CREAD);
    t_opt.c_cflag &= ~PARENB;
    t_opt.c_cflag &= ~CSTOPB;
    t_opt.c_cflag &= ~CSIZE;
    t_opt.c_cflag |= CS8;
    t_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    t_opt.c_iflag &= ~(IXON | IXOFF | IXANY);
    t_opt.c_oflag &= ~OPOST;
    t_opt.c_cc[VMIN] = 0;
    //t_opt.c_cc[VTIME] = 10;
    t_opt.c_cc[VTIME] = 1;

    tcflush(fd, TCIFLUSH);

    tcsetattr(fd, TCSANOW, &t_opt);
#endif
    return 0;
}

int serial_write(int fd, char *buf, int size) {
    int ret = 0;

    ret = write(fd, buf, size);

    fprintf(stderr, "W> ");
    int i;
    for(i = 0; i < size; i++) {
	fprintf(stderr, "0x%02x ", (unsigned char) buf[i]);
    }
    fprintf(stderr, "size = %d (%d)\n", size, ret);

    return ret;
}

int serial_read(int fd, char *buf, int size) {
    int len = 0;
    int ret = 0;
    int timeout = 0;

    while (len < size) {
	//fprintf(stderr, "FD = %d &buffer = %p (%d)\n" , fd, buf, sizeof(buf));
	ret = read(fd, buf+len, size-len);
	if (ret == -1){
	    return -1;
	}

	if (ret == 0) {
	    timeout++;

	    if (timeout >= 10) {
		break;		// Break out and return from this function
	    } else {
		continue;	// Just continue the loop
	    }
	}

	len += ret;
    }

    if( len > 255) {
	buf[255] = 0x00;	// Buffer's length is 256
    } else {
	buf[len] = 0x00;
    }
    return len;
}

int serial_open(char *port) {
    int fd;
    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd == -1) {
	fprintf(stderr, "Could not open serial port.");
	return -1;
    }

    return fd;
}

int serial_close(int fd) {
    close(fd);

    return 0;
}
