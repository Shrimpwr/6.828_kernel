#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H
#include <kern/pci.h>
#include <inc/error.h>
#include <inc/string.h>

/* PCI Device IDs */
#define E1000_DEV_ID_82540EM             0x100E
#define E1000_VENDOR_ID                  0x8086 

/* Register Set. (82543, 82544)
 *
 * Registers are defined to be 32 bits and  should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 *
 * RW - register is both readable and writable
 * RO - register is read only
 * WO - register is write only
 * R/clr - register is read only and is cleared when read
 * A - register array
 */
#define E1000_CTRL     0x00000  /* Device Control - RW */
#define E1000_CTRL_DUP 0x00004  /* Device Control Duplicate (Shadow) - RW */
#define E1000_STATUS   0x00008  /* Device Status - RO */
#define E1000_RCTL     0x00100  /* RX Control - RW */
#define E1000_RDTR1    0x02820  /* RX Delay Timer (1) - RW */
#define E1000_RDBAL1   0x02900  /* RX Descriptor Base Address Low (1) - RW */
#define E1000_RDBAH1   0x02904  /* RX Descriptor Base Address High (1) - RW */
#define E1000_RDLEN1   0x02908  /* RX Descriptor Length (1) - RW */
#define E1000_RDH1     0x02910  /* RX Descriptor Head (1) - RW */
#define E1000_RDT1     0x02918  /* RX Descriptor Tail (1) - RW */
#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TCTL_EXT 0x00404  /* Extended TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TBT      0x00448  /* TX Burst Timer - RW */
#define E1000_RDBAL    0x02800  /* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH    0x02804  /* RX Descriptor Base Address High - RW */
#define E1000_RDLEN    0x02808  /* RX Descriptor Length - RW */
#define E1000_RDH      0x02810  /* RX Descriptor Head - RW */
#define E1000_RDT      0x02818  /* RX Descriptor Tail - RW */
#define E1000_RDTR     0x02820  /* RX Delay Timer - RW */
#define E1000_RDBAL0   E1000_RDBAL /* RX Desc Base Address Low (0) - RW */
#define E1000_RDBAH0   E1000_RDBAH /* RX Desc Base Address High (0) - RW */
#define E1000_RDLEN0   E1000_RDLEN /* RX Desc Length (0) - RW */
#define E1000_RDH0     E1000_RDH   /* RX Desc Head (0) - RW */
#define E1000_RDT0     E1000_RDT   /* RX Desc Tail (0) - RW */
#define E1000_RDTR0    E1000_RDTR  /* RX Delay Timer (0) - RW */
#define E1000_RXDCTL   0x02828  /* RX Descriptor Control queue 0 - RW */
#define E1000_RXDCTL1  0x02928  /* RX Descriptor Control queue 1 - RW */
#define E1000_TDFH     0x03410  /* TX Data FIFO Head - RW */
#define E1000_TDFT     0x03418  /* TX Data FIFO Tail - RW */
#define E1000_TDFHS    0x03420  /* TX Data FIFO Head Saved - RW */
#define E1000_TDFTS    0x03428  /* TX Data FIFO Tail Saved - RW */
#define E1000_TDFPC    0x03430  /* TX Data FIFO Packet Count - RW */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */
#define E1000_TIDV     0x03820  /* TX Interrupt Delay Value - RW */
#define E1000_TXDCTL   0x03828  /* TX Descriptor Control - RW */
#define E1000_TADV     0x0382C  /* TX Interrupt Absolute Delay Val - RW */
#define E1000_TDBAL1   0x03900  /* TX Desc Base Address Low (1) - RW */
#define E1000_TDBAH1   0x03904  /* TX Desc Base Address High (1) - RW */
#define E1000_TDLEN1   0x03908  /* TX Desc Length (1) - RW */
#define E1000_TDH1     0x03910  /* TX Desc Head (1) - RW */
#define E1000_TDT1     0x03918  /* TX Desc Tail (1) - RW */
#define E1000_TXDCTL1  0x03928  /* TX Descriptor Control (1) - RW */
#define E1000_MTA      0x05200  /* Multicast Table Array - RW Array */
#define E1000_RA       0x05400  /* Receive Address - RW Array */

/* Transmit Descriptor */
struct e1000_tx_desc {
    uint64_t buffer_addr;       /* Address of the descriptor's data buffer */
    union {
        uint32_t data;
        struct {
            uint16_t length;    /* Data buffer length */
            uint8_t cso;        /* Checksum offset */
            uint8_t cmd;        /* Descriptor control */
        } flags;
    } lower;
    union {
        uint32_t data;
        struct {
            uint8_t status;     /* Descriptor status */
            uint8_t css;        /* Checksum start */
            uint16_t special;
        } fields;
    } upper;
};

/* Transmit Descriptor bit definitions */
#define E1000_TXD_DTYP_D     0x00100000 /* Data Descriptor */
#define E1000_TXD_DTYP_C     0x00000000 /* Context Descriptor */
#define E1000_TXD_POPTS_IXSM 0x01       /* Insert IP checksum */
#define E1000_TXD_POPTS_TXSM 0x02       /* Insert TCP/UDP checksum */
#define E1000_TXD_CMD_EOP    0x01000000 /* End of Packet */
#define E1000_TXD_CMD_IFCS   0x02000000 /* Insert FCS (Ethernet CRC) */
#define E1000_TXD_CMD_IC     0x04000000 /* Insert Checksum */
#define E1000_TXD_CMD_RS     0x08000000 /* Report Status */
#define E1000_TXD_CMD_RPS    0x10000000 /* Report Packet Sent */
#define E1000_TXD_CMD_DEXT   0x20000000 /* Descriptor extension (0 = legacy) */
#define E1000_TXD_CMD_VLE    0x40000000 /* Add VLAN tag */
#define E1000_TXD_CMD_IDE    0x80000000 /* Enable Tidv register */
#define E1000_TXD_STAT_DD    0x00000001 /* Descriptor Done */
#define E1000_TXD_STAT_EC    0x00000002 /* Excess Collisions */
#define E1000_TXD_STAT_LC    0x00000004 /* Late Collisions */
#define E1000_TXD_STAT_TU    0x00000008 /* Transmit underrun */
#define E1000_TXD_CMD_TCP    0x01000000 /* TCP packet */
#define E1000_TXD_CMD_IP     0x02000000 /* IP packet */
#define E1000_TXD_CMD_TSE    0x04000000 /* TCP Seg enable */
#define E1000_TXD_STAT_TC    0x00000004 /* Tx Underrun */

/* Transmit Control */
#define E1000_TCTL_RST    0x00000001    /* software reset */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_BCE    0x00000004    /* busy check enable */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */
#define E1000_TCTL_SWXOFF 0x00400000    /* SW Xoff transmission */
#define E1000_TCTL_PBE    0x00800000    /* Packet Burst Enable */
#define E1000_TCTL_RTLC   0x01000000    /* Re-transmit on late collision */
#define E1000_TCTL_NRTU   0x02000000    /* No Re-transmit on underrun */
#define E1000_TCTL_MULR   0x10000000    /* Multiple request support */

/* Receive Descriptor */
struct e1000_rx_desc {
    uint64_t buffer_addr; /* Address of the descriptor's data buffer */
    uint16_t length;     /* Length of data DMAed into data buffer */
    uint16_t csum;       /* Packet checksum */
    uint8_t status;      /* Descriptor status */
    uint8_t errors;      /* Descriptor Errors */
    uint16_t special;
};

/* Receive Descriptor bit definitions */
#define E1000_RXD_STAT_DD       0x01    /* Descriptor Done */
#define E1000_RXD_STAT_EOP      0x02    /* End of Packet */
#define E1000_RXD_STAT_IXSM     0x04    /* Ignore checksum */
#define E1000_RXD_STAT_VP       0x08    /* IEEE VLAN Packet */
#define E1000_RXD_STAT_UDPCS    0x10    /* UDP xsum caculated */
#define E1000_RXD_STAT_TCPCS    0x20    /* TCP xsum calculated */
#define E1000_RXD_STAT_IPCS     0x40    /* IP xsum calculated */
#define E1000_RXD_STAT_PIF      0x80    /* passed in-exact filter */
#define E1000_RXD_STAT_IPIDV    0x200   /* IP identification valid */
#define E1000_RXD_STAT_UDPV     0x400   /* Valid UDP checksum */
#define E1000_RXD_STAT_ACK      0x8000  /* ACK Packet indication */
#define E1000_RXD_ERR_CE        0x01    /* CRC Error */
#define E1000_RXD_ERR_SE        0x02    /* Symbol Error */
#define E1000_RXD_ERR_SEQ       0x04    /* Sequence Error */
#define E1000_RXD_ERR_CXE       0x10    /* Carrier Extension Error */
#define E1000_RXD_ERR_TCPE      0x20    /* TCP/UDP Checksum Error */
#define E1000_RXD_ERR_IPE       0x40    /* IP Checksum Error */
#define E1000_RXD_ERR_RXE       0x80    /* Rx Data Error */
#define E1000_RXD_SPC_VLAN_MASK 0x0FFF  /* VLAN ID is in lower 12 bits */
#define E1000_RXD_SPC_PRI_MASK  0xE000  /* Priority is in upper 3 bits */
#define E1000_RXD_SPC_PRI_SHIFT 13
#define E1000_RXD_SPC_CFI_MASK  0x1000  /* CFI is bit 12 */
#define E1000_RXD_SPC_CFI_SHIFT 12

#define E1000_RXDEXT_STATERR_CE    0x01000000
#define E1000_RXDEXT_STATERR_SE    0x02000000
#define E1000_RXDEXT_STATERR_SEQ   0x04000000
#define E1000_RXDEXT_STATERR_CXE   0x10000000
#define E1000_RXDEXT_STATERR_TCPE  0x20000000
#define E1000_RXDEXT_STATERR_IPE   0x40000000
#define E1000_RXDEXT_STATERR_RXE   0x80000000

#define E1000_RXDPS_HDRSTAT_HDRSP        0x00008000
#define E1000_RXDPS_HDRSTAT_HDRLEN_MASK  0x000003FF

/* Receive Control */
#define E1000_RCTL_RST            0x00000001    /* Software reset */
#define E1000_RCTL_EN             0x00000002    /* enable */
#define E1000_RCTL_SBP            0x00000004    /* store bad packet */
#define E1000_RCTL_UPE            0x00000008    /* unicast promiscuous enable */
#define E1000_RCTL_MPE            0x00000010    /* multicast promiscuous enab */
#define E1000_RCTL_LPE            0x00000020    /* long packet enable */
#define E1000_RCTL_LBM_NO         0x00000000    /* no loopback mode */
#define E1000_RCTL_LBM_MAC        0x00000040    /* MAC loopback mode */
#define E1000_RCTL_LBM_SLP        0x00000080    /* serial link loopback mode */
#define E1000_RCTL_LBM_TCVR       0x000000C0    /* tcvr loopback mode */
#define E1000_RCTL_DTYP_MASK      0x00000C00    /* Descriptor type mask */
#define E1000_RCTL_DTYP_PS        0x00000400    /* Packet Split descriptor */
#define E1000_RCTL_RDMTS_HALF     0x00000000    /* rx desc min threshold size */
#define E1000_RCTL_RDMTS_QUAT     0x00000100    /* rx desc min threshold size */
#define E1000_RCTL_RDMTS_EIGTH    0x00000200    /* rx desc min threshold size */
#define E1000_RCTL_MO_SHIFT       12            /* multicast offset shift */
#define E1000_RCTL_MO_0           0x00000000    /* multicast offset 11:0 */
#define E1000_RCTL_MO_1           0x00001000    /* multicast offset 12:1 */
#define E1000_RCTL_MO_2           0x00002000    /* multicast offset 13:2 */
#define E1000_RCTL_MO_3           0x00003000    /* multicast offset 15:4 */
#define E1000_RCTL_MDR            0x00004000    /* multicast desc ring 0 */
#define E1000_RCTL_BAM            0x00008000    /* broadcast enable */
/* these buffer sizes are valid if E1000_RCTL_BSEX is 0 */
#define E1000_RCTL_SZ_2048        0x00000000    /* rx buffer size 2048 */
#define E1000_RCTL_SZ_1024        0x00010000    /* rx buffer size 1024 */
#define E1000_RCTL_SZ_512         0x00020000    /* rx buffer size 512 */
#define E1000_RCTL_SZ_256         0x00030000    /* rx buffer size 256 */
/* these buffer sizes are valid if E1000_RCTL_BSEX is 1 */
#define E1000_RCTL_SZ_16384       0x00010000    /* rx buffer size 16384 */
#define E1000_RCTL_SZ_8192        0x00020000    /* rx buffer size 8192 */
#define E1000_RCTL_SZ_4096        0x00030000    /* rx buffer size 4096 */
#define E1000_RCTL_VFE            0x00040000    /* vlan filter enable */
#define E1000_RCTL_CFIEN          0x00080000    /* canonical form enable */
#define E1000_RCTL_CFI            0x00100000    /* canonical form indicator */
#define E1000_RCTL_DPF            0x00400000    /* discard pause frames */
#define E1000_RCTL_PMCF           0x00800000    /* pass MAC control frames */
#define E1000_RCTL_BSEX           0x02000000    /* Buffer size extension */
#define E1000_RCTL_SECRC          0x04000000    /* Strip Ethernet CRC */
#define E1000_RCTL_FLXBUF_MASK    0x78000000    /* Flexible buffer size */
#define E1000_RCTL_FLXBUF_SHIFT   27            /* Flexible buffer shift */

/* Receive Address */
#define E1000_RAH_AV  0x80000000        /* Receive descriptor valid */

/* MAC Address */
#define MACADDRL 0x12005452
#define MACADDRH 0x5634

/* Constants */
#define NTDRENTRIES 32
#define NRDRENTRIES 128
#define NMTAENTRIES 128
#define PACKETBUFSIZE 2048 

int pci_e1000_attach(struct pci_func *pcif);
void tx_init();
void rx_init();
int tx_send_packet(char *buf, size_t nbytes);
int rx_recv_packet(char *buf);

#endif  // SOL >= 6
