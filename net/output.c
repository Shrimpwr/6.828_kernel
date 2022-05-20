#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
    int r;
    envid_t from_env_store;

	while (1) {
		// 	- read a packet from the network server
        if ((r = ipc_recv(&from_env_store, (void *)&nsipcbuf, 0)) < 0)
            panic("output: ipc_recv(): %e", r);

        if (from_env_store != ns_envid)
            panic("output: unexpected IPC sender!\n");

        if (r != NSREQ_OUTPUT)
            panic("output: unexpected IPC type!\n");

		//	- send the packet to the device driver
		// handle the situation that tx_desc_ring is full
		while ((r = sys_tx_sendpack(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) < 0)
			sys_yield();
	}
}
