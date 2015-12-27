#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "serial.h"
#include "buspirate.h"

//static struct BP_t pBP;
//static uint8_t BP_reversebyte(uint8_t c);
//static char bpbuf[4096];
//static int bpbufcnt;

extern int disable_comport;
extern int dumphandle;
extern int verbose;
extern int modem;

// low lever send command, get reply function
uint32_t BP_WriteToPirate(int fd, char *val) {
    int res = -1;
    char ret = 0;

    serial_write(fd, val, 1);

    if (disable_comport != 1) {  //if comport is enable, we need a response from the port
	res = serial_read(fd, &ret, 1);

	if( ret != 0x01) {
	    if (modem==TRUE){
		printf("Modem responded with %i byte and with a value of 0X%X\n",res,ret);
	    } else {
		printf("ERROR\n");
		return -1;
	    }
	}
    }

    return 0;
}

/*
** Put the Buspirate into raw binary mode by sending 20 nulls
** expect BBIO1
** Then put it into SPI raw mode by sending it 0x01
** expect SPI1
*/
void BP_EnableMode(int fd, char bbmode) {
    int ret;
    char tmp[100] = { [0 ... 99] = 0x00 };
    int done = 0;
    //int cmd_sent = 0;
    int tries = 0;

    printf(" Entering binary mode...\n");
    if (fd == (-1)) {   //added because the fd has already returned null
	printf("Port does not exist!");
	return;
    }

    // -[ BBIO1 - Binary mode ]-------------------------------------------------
    while (!done) {
	tmp[0] = 0x00;

	serial_write(fd, tmp, 1);
	tries++;
	usleep(1);

	ret = serial_read(fd, tmp, 5);

	if (modem==TRUE) {
	    // ???
	    printf("\nModem Responded = %i\n", ret);
	    done=1;
	} else {
	    if (ret != 5 && tries > 20) {
		fprintf(stderr, "Buspirate did not respond correctly :( %i \n", ret );
		exit(-1);
	    } else if (strncmp(tmp, "BBIO1", 5) == 0) {
		printf("read returned %i:%s\n", ret, tmp);
		done=1;
	    }
	}

	if (tries > 25){
	    printf("Buspirate:Too many tries in serial read! -exiting \n - chip not detected, or not readable/writable\n");
	    exit(-1);
	}
    } // -----------------------------------------------------------------------
    printf("BP found\n");

    // -[ SPI mode ]------------------------------------------------------------
    // 00000001 - Enter raw SPI mode, display version string
    //
    // Once in raw bitbang mode, send 0x01 to enter raw SPI mode. The
    // Bus Pirate responds 'SPIx', where x is the raw SPI protocol
    // version (currently 1). Get the version string at any time by
    // sending 0x01 again.
    done = 0;
    tmp[0] = bbmode;
    tmp[1] = 0x01;
    //printf("Sending 0X%02X to port\n",tmp[0]);
    serial_write(fd, tmp, 1);
    //tries++;
    usleep(1);

    ret = serial_read(fd, tmp, 5);

    if (modem == TRUE) {
            printf("Modem Responded = %i with value %#X\n",ret,tmp[0]);
    } else {
	if ( (ret >= 4) && (strncmp(tmp, "SPI1", 4) == 0)) {
	    printf("In SPI mode (%d/%s)\n", ret, tmp);
	} else {
	    fprintf(stderr, "Buspirate did not respond correctly :( %i [ ", ret );
	    // reusing tries instead of creating a new var, yeah, just lazy ;-)
	    for(tries = 0; tries < ret; tries++) {
		fprintf(stderr, "0x%02x ", tmp[tries]);
	    }
	    fprintf(stderr, "] %s\n", tmp);

	    printf("Sending 0X%02X to port\n", 0x01);
	    tmp[0] = 0x01;
	    tmp[1] = 0x01;
	    serial_write(fd, tmp, 1);
	    usleep(1);

	    if ( (ret==4) && (strncmp(tmp, "SPI1", 4) == 0)) {
		fprintf(stderr, "Yea: %s [ ", tmp ); //
	    } else {
		ret = serial_read(fd, tmp, 5);

		printf("Sending 0X%02X to port\n", 0x01);
		tmp[0] = 0x01;
		tmp[1] = 0x01;
		serial_write(fd, tmp, 2);
		usleep(1);

		ret = serial_read(fd, tmp, 5);

		if ( (ret==4) && (strncmp(tmp, "SPI1", 4) == 0)) {
		    fprintf(stderr, "Yea: %s [ ", tmp ); //
		} else {
		    if((ret==5) && (strncmp(tmp, "SPI1", 5) == 0)) {
			fprintf(stderr, "Yea: %s [ ", tmp ); //
		    } else {
			fprintf(stderr, "Buspirate did not respond correctly :( %i [ ", ret );
			// reusing tries instead of creating a new var, yeah, just lazy ;-)
			for(tries = 0; tries < ret; tries++) {
			    fprintf(stderr, "0x%02x ", tmp[tries]);
			}
			fprintf(stderr, "] %s\n", tmp);

			exit(-1);
		    }
		}
	    } // End if SPI1 else
	}
    }
}
