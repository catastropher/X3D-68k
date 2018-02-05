#include "nsocket.h"

nn_ch_t ch = NULL;
nn_oh_t oh = NULL;
nn_nh_t nh = NULL;

// positive check
//#define NNCALL(f,p) do {if ((f p) < 0) exit_with_error("on " #f "()");} while(0)
#define NNCALL(f,p) do {if ((f p) < 0) return count--;} while(0)
// non-null check
//#define NNCALLP(f,p) do {if ((f p) == NULL) exit_with_error("on " #f "()");} while(0)
#define NNCALLP(f,p) do {if ((f p) == NULL) return count--;} while(0)

BOOL connected = FALSE;
unsigned int timeout = 50;

nn_ch_t* ns_getch()
{
	return ch;
}

int count = -1;

int ns_init()
{
    count = -1;
    
	NNCALLP(oh = TI_NN_CreateOperationHandle, ());
	NNCALL(TI_NN_NodeEnumInit, (oh));
	NNCALL(TI_NN_NodeEnumNext, (oh, &nh));
	NNCALL(TI_NN_NodeEnumDone, (oh));
	NNCALL(TI_NN_DestroyOperationHandle, (oh));
	
	if (!nh)
		return -9;
	
	NNCALL(TI_NN_Connect, (nh, 32767, &ch));
	
	return 0;
}

int ns_connect(char* host, short port)
{
	char msg[50];
	sprintf(msg, "%s:%d", host, port);
	
	NNCALL(TI_NN_Write, (ch, msg, strlen(msg) + 1));
	
	int status;
	uint32_t recv_size;
	NNCALL(TI_NN_Read, (ch, 5000, &status, sizeof(int), &recv_size));
	
	return status;
}

int ns_send(void* buf, unsigned int len)
{
	NNCALL(TI_NN_Write, (ch, buf, len));
	return 0;
}

int ns_recv(void* buf, unsigned int len)
{
	uint32_t recv_size = -1;
	NNCALL(TI_NN_Read, (ch, timeout, buf, len, (uint32_t*)&recv_size));
	
	return recv_size;
}

void ns_set_timeout(unsigned int timeout_ms)
{
	timeout = timeout_ms;
}

unsigned int ns_get_pktsize()
{
	return TI_NN_GetConnMaxPktSize(ch);
}

unsigned int ns_get_timeout()
{
	return timeout;
}

void ns_stop()
{
	if(ch)
		TI_NN_Disconnect(ch);
}
