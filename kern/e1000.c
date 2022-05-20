#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here

volatile uint32_t *e1000;

__attribute__((__aligned__(16)))
struct e1000_tx_desc tx_desc_ring[NTDRENTRIES];
struct e1000_rx_desc rx_desc_ring[NRDRENTRIES];

__attribute__((__aligned__(PGSIZE)))
char tx_packet_buffers[NTDRENTRIES][PACKETBUFSIZE];
char rx_packet_buffers[NRDRENTRIES][PACKETBUFSIZE];

int tdr_tail;
int rdr_tail;

// attach function
int pci_e1000_attach(struct pci_func *pcif) {
    pci_func_enable(pcif);

    // create a virtual memory mapping for the E1000
    e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    cprintf("Device Status Register: 0x%x\n", e1000[E1000_STATUS >> 2]);

    // transmit initialization
    tx_init();

    // receive initialization
    rx_init();

    return 0;
}

// transmit initialization
void tx_init() {
    // initialize the transmit descriptor ring
    for (int i = 0; i < NTDRENTRIES; ++i) {
        tx_desc_ring[i].buffer_addr = PADDR(tx_packet_buffers[i]);  
    }
    tdr_tail = 0;

    // initialize transmit relating registers
    e1000[E1000_TDBAL >> 2] = PADDR(tx_desc_ring);
    e1000[E1000_TDBAH >> 2] = 0;
    e1000[E1000_TDLEN >> 2] = sizeof(tx_desc_ring);
    e1000[E1000_TDH >> 2] = 0;
    e1000[E1000_TDT >> 2] = 0;
    e1000[E1000_TCTL >> 2] = E1000_TCTL_EN | E1000_TCTL_PSP | 0x40000;
    e1000[E1000_TIPG >> 2] = 10;            // IGPT
    e1000[E1000_TIPG >> 2] |= 4 << 10;      // IGPR1
    e1000[E1000_TIPG >> 2] |= 6 << 20;      // IGPR2
}

// send packet
int tx_send_packet(char *buf, size_t nbytes) {
    // check if size is valid
    if (nbytes > PACKETBUFSIZE)
        panic("tx_send_packet: invalid packet size");
    
    // if RS has been set, the descriptor has been used before, check if it is free
    if (tx_desc_ring[tdr_tail].lower.flags.cmd & E1000_TXD_CMD_RS)
        if ((tx_desc_ring[tdr_tail].lower.flags.cmd & E1000_TXD_STAT_DD) == 0)
            return -E_TXDR_FULL;
    
    // set up the TX descriptor
    memcpy(tx_packet_buffers[tdr_tail], buf, nbytes);
    tx_desc_ring[tdr_tail].lower.data = (E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP);
    tx_desc_ring[tdr_tail].lower.flags.length = nbytes;

    // update tail register
    tdr_tail = (tdr_tail + 1) % NTDRENTRIES;
    e1000[E1000_TDT >> 2] = tdr_tail;
    return 0;
}

// receive initialization
void rx_init() {
    // initialize the receive descriptor ring
    for (int i = 0; i < NRDRENTRIES; ++i) 
        rx_desc_ring[i].buffer_addr = PADDR(rx_packet_buffers[i]);  
    rdr_tail = NRDRENTRIES - 1;

    // set MAC address
    e1000[E1000_RA >> 2] = MACADDRL;
    e1000[(E1000_RA >> 2) + 1] = MACADDRH | E1000_RAH_AV;

    // initialize MTA
    for (int i = 0; i < NMTAENTRIES; ++i)
        e1000[(E1000_MTA >> 2) + i] = 0;

    // initialize receive relating registers
    e1000[E1000_RDBAL >> 2] = PADDR(rx_desc_ring);
    e1000[E1000_RDBAH >> 2] = 0;
    e1000[E1000_RDLEN >> 2] = sizeof(rx_desc_ring);
    e1000[E1000_RDH >> 2] = 0;
    e1000[E1000_RDT >> 2] = NRDRENTRIES - 1;
    e1000[E1000_RCTL >> 2] = (E1000_RCTL_EN | E1000_RCTL_SECRC | E1000_RCTL_BAM) & (~E1000_RCTL_LPE);
}

// receive packet
int rx_recv_packet(char *buf) {
    // check for packet
    uint32_t next_tail = (rdr_tail + 1) % NRDRENTRIES;
    if ((rx_desc_ring[next_tail].status & E1000_RXD_STAT_DD) == 0)
        return -E_RXDR_EMPTY;
    
    // copy the packet content
    rdr_tail = next_tail;
    memcpy(buf, &rx_packet_buffers[rdr_tail], rx_desc_ring[rdr_tail].length);

    // update tail register
    rx_desc_ring[rdr_tail].status &= ~E1000_RXD_STAT_DD; 
    e1000[E1000_RDT >> 2] = rdr_tail;
    return rx_desc_ring[rdr_tail].length;
}