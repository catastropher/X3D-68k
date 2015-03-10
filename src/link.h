// Header File
// Created 3/9/2015; 11:07:24 AM
#pragma once

enum {
	LINK_STRING,
	LINK_CONNECT,
	LINK_CONNECT_RESPOND
};

#define NOT_CONNECTED 255

extern unsigned char calc_id;

void init_link();
void cleanup_link();
char link_send_data(void* data, unsigned long size, unsigned char throw);
char link_recv_data(void* data, unsigned short size, unsigned char throw);

short link_recv_byte(unsigned char throw);
short link_send_byte(unsigned char b, unsigned char throw);

char link_connect();

char link_send_string(const char* str);
char link_recv_string(char* dest);