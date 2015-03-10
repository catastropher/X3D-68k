// C Source File
// Created 3/9/2015; 11:07:00 AM

#include "error.h"
#include "link.h"

#include <tigcclib.h>

unsigned char calc_id;

// Opens the link port
void init_link() {
	// Open low-level link communication
	OSLinkOpen();
	
	calc_id = NOT_CONNECTED;
}

// Closes the linkport
void cleanup_link() {
	OSLinkClose();
}

// Sends a block of data through the linkport
// Returns whether successful
char link_send_data(void* data, unsigned long size, unsigned char throw) {
	// Wait until the linkport is ready...
	while(OSCheckSilentLink() != 0) ;
	
	unsigned short val = LIO_SendData(data, size);
	
	errorif(val != 0 && throw, "Failed to send data");

	return val == 0;
}

// Receives a block of data from the linkport
char link_recv_data(void* data, unsigned short size, unsigned char throw) {
	// Wait until the linkport is ready...
	while(OSCheckSilentLink() != 0) ;
	
	unsigned short val = LIO_RecvData(data, size, 1);
	
	errorif(val != 0 && throw, "Failed to receive data");
	
	return val == 0;
}

// Receives a single byte
// Returns -1 if unsuccessful
short link_recv_byte(unsigned char throw) {
	unsigned char byte;
	if(!link_recv_data(&byte, 1, throw))
		return -1;
	else
		return byte;
	
}

// Sends a single byte
// Returns whether successful
short link_send_byte(unsigned char b, unsigned char throw) {
	return link_send_data(&b, 1, throw);
}

// Sends a string through the linkport
// Note: the length of the string must be < 256 characters!
// Returns whether successful
char link_send_string(const char* str) {
	unsigned short length = strlen(str);
	
	if(length >= 256)
		return 0;
	
	unsigned char string_info[] = {LINK_STRING, length};
	
	if(link_send_data(string_info, 2, 0))
		if(link_send_data((void *)str, length, 0))
			return 1;
			
	error("Failed to send data\n");
	return 0;
}

// Receives a string throught the linkport
// The destination buffer should be at least 257 bytes!
// Returns whether successful
char link_recv_string(char* dest) {
	link_recv_byte(0);
	
	short length = link_recv_byte(0);
	
	if(length == -1)
		return 0;
		
	char pass = 0;
	int i;
		
	for(i = 0; i < 20 && !pass; i++)
		pass = link_recv_data(dest, length, 0);
		
	dest[length] = '\0';
	return pass;
}



// Attemps to connect to another calculator
// Returns whether successful
char link_connect() {
	printf("Press enter to connect\n");
	
	while(!_keytest(RR_UP)) ;
	
	init_link();
	
	clrscr();
	
	// If the other calc connected first, they'll try to send first
	short byte = -1;
	short i = 0;
	
	for(i = 0; i < 20 && byte == -1; i++)
		byte = link_recv_byte(0);
	
	
	
	unsigned char success = 0;
	
	if(byte == -1) {
		printf("Waiting to connect\n");
		// We're the first calc to connect, so keep transmitting LINK_CONNECT
		
		do {
			success = link_send_byte(LINK_CONNECT, 0);
			
			// Allow the user to cancel if they want to
			if(_keytest(RR_ESC))
				return 0;

			byte = link_recv_byte(0);
			
			//printf("Received byte: %d\n", byte);
			
			if(_keytest(RR_ESC))
				return 0;
		} while(byte != LINK_CONNECT_RESPOND);
		
		//errorif(byte != LINK_CONNECT, "Wrong byte sent");
		calc_id = 0;
	}
	else {
		// We're second to connect
		errorif(byte != LINK_CONNECT, "Wrong byte received");
		
		// Senk back that we're connected
		do {
			success = link_send_byte(LINK_CONNECT_RESPOND, 0);
			
			// Allow the user to cancel if they want to
			if(_keytest(RR_ESC))
				return 0;
		} while(!success);
		
		calc_id = 1;
	}
	
	while(link_recv_byte(0) != -1) ;
	
	printf("Connected!\nCalc ID: %d\n", calc_id);
	ngetchx();
	
	return 1;
}