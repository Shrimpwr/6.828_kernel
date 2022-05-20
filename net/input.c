#include "ns.h"
#include <kern/e1000.h>

extern union Nsipc nsipcbuf[2];

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	int r;
	int s = 0;
	char buf[PACKETBUFSIZE];

	while (1) {
		// 	- read a packet from the device driver
		// handle the situation that rx_desc_ring is empty 
		while ((r = sys_rx_try_recvpack(buf)) < 0)
			sys_yield();

		nsipcbuf[s].pkt.jp_len = r;
		memcpy(nsipcbuf[s].pkt.jp_data, buf, r);

		//	- send it to the network server
		ipc_send(ns_envid, NSREQ_INPUT, (void *)&nsipcbuf[s], PTE_P | PTE_U);
		s ^= 1;
	}
}
