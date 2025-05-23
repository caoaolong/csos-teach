#include <logf.h>
#include <mio.h>
#include <pic.h>
#include <task.h>
#include <interrupt.h>
#include <pci/e1000.h>
#include <netx/eth.h>
#include <csos/string.h>
#include <csos/memory.h>

#define VENDORID        0x8086 // 供应商英特尔
#define DEVICEID_LOW    0x1000
#define DEVICEID_HIGH   0x1028

#define RX_DESC_NR      32 // 接收描述符数量
#define TX_DESC_NR      32 // 传输描述符数量

// 寄存器偏移
enum REGISTERS
{
    E1000_CTRL = 0x00,   // Device Control 设备控制
    E1000_STATUS = 0x08, // Device Status 设备状态
    E1000_EERD = 0x14,   // EEPROM Read EEPROM 读取

    E1000_ICR = 0xC0,   // Interrupt Cause Read 中断原因读
    E1000_ITR = 0xC4,   // Interrupt Throttling 中断节流
    E1000_ICS = 0xC8,   // Interrupt Cause Set 中断原因设置
    E1000_IMS = 0xD0,   // Interrupt Mask Set/Read 中断掩码设置/读
    E1000_IMC = 0xD8,   // Interrupt Mask Clear 中断掩码清除

    E1000_RCTL = 0x100,   // Receive Control 接收控制
    E1000_RDBAL = 0x2800, // Receive Descriptor Base Address LOW 接收描述符低地址
    E1000_RDBAH = 0x2804, // Receive Descriptor Base Address HIGH 64bit only 接收描述符高地址
    E1000_RDLEN = 0x2808, // Receive Descriptor Length 接收描述符长度
    E1000_RDH = 0x2810,   // Receive Descriptor Head 接收描述符头
    E1000_RDT = 0x2818,   // Receive Descriptor Tail 接收描述符尾

    E1000_TCTL = 0x400,   // Transmit Control 发送控制
    E1000_TDBAL = 0x3800, // Transmit Descriptor Base Low 传输描述符低地址
    E1000_TDBAH = 0x3804, // Transmit Descriptor Base High 传输描述符高地址
    E1000_TDLEN = 0x3808, // Transmit Descriptor Length 传输描述符长度
    E1000_TDH = 0x3810,   // TDH Transmit Descriptor Head 传输描述符头
    E1000_TDT = 0x3818,   // TDT Transmit Descriptor Tail 传输描述符尾

    E1000_MAT0 = 0x5200, // Multicast Table Array 05200h-053FCh 组播表数组
    E1000_MAT1 = 0x5400, // Multicast Table Array 05200h-053FCh 组播表数组
};

enum IMSBITS {
    IMS_TXDW    = 1 << 0,
    IMS_TXQE    = 1 << 1,
    IMS_LSC     = 1 << 2,
    IMS_RXSEQ   = 1 << 3,
    IMS_RXDMT0  = 1 << 4,
    IMS_RXO     = 1 << 6,
    IMS_RXT0    = 1 << 7,
    IMS_MDAC    = 1 << 9,
    IMS_RXCFG   = 1 << 10,
    IMS_PHYINT  = 1 << 12,
    IMS_GPI0    = 1 << 13,
    IMS_GPI1    = 1 << 14,
    IMS_TXD_LOW = 1 << 15,
    IMS_SRPD    = 1 << 16
};

enum CTRLBITS {
    CTRL_FD     = 1 << 0,
    CTRL_ASDE   = 1 << 5,
    CTRL_SLU    = 1 << 6
};

enum RCTLBITS {
    RCTL_EN = 1 << 1,               // Receiver Enable
    RCTL_SBP = 1 << 2,              // Store Bad Packets
    RCTL_UPE = 1 << 3,              // Unicast Promiscuous Enabled
    RCTL_MPE = 1 << 4,              // Multicast Promiscuous Enabled
    RCTL_LPE = 1 << 5,              // Long Packet Reception Enable
    RCTL_LBM_NONE = 0b00 << 6,      // No Loopback
    RCTL_LBM_PHY = 0b11 << 6,       // PHY or external SerDesc loopback
    RTCL_RDMTS_HALF = 0b00 << 8,    // Free Buffer Threshold is 1/2 of RDLEN
    RTCL_RDMTS_QUARTER = 0b01 << 8, // Free Buffer Threshold is 1/4 of RDLEN
    RTCL_RDMTS_EIGHTH = 0b10 << 8,  // Free Buffer Threshold is 1/8 of RDLEN

    RCTL_BAM = 1 << 15, // Broadcast Accept Mode
    RCTL_VFE = 1 << 18, // VLAN Filter Enable

    RCTL_CFIEN = 1 << 19, // Canonical Form Indicator Enable
    RCTL_CFI = 1 << 20,   // Canonical Form Indicator Bit Value
    RCTL_DPF = 1 << 22,   // Discard Pause Frames
    RCTL_PMCF = 1 << 23,  // Pass MAC Control Frames
    RCTL_SECRC = 1 << 26, // Strip Ethernet CRC

    RCTL_BSIZE_256 = 3 << 16,
    RCTL_BSIZE_512 = 2 << 16,
    RCTL_BSIZE_1024 = 1 << 16,
    RCTL_BSIZE_2048 = 0 << 16,
    RCTL_BSIZE_4096 = (3 << 16) | (1 << 25),
    RCTL_BSIZE_8192 = (2 << 16) | (1 << 25),
    RCTL_BSIZE_16384 = (1 << 16) | (1 << 25)
};

enum TCTLBITS {
    TCTL_EN = 1 << 1,      // Transmit Enable
    TCTL_PSP = 1 << 3,     // Pad Short Packets
    TCTL_CT = 4,           // Collision Threshold
    TCTL_COLD = 12,        // Collision Distance
    TCTL_SWXOFF = 1 << 22, // Software XOFF Transmission
    TCTL_RTLC = 1 << 24,   // Re-transmit on Late Collision
    TCTL_NRTU = 1 << 25,   // No Re-transmit on underrun
};

// 接收状态
enum RS
{
    RS_DD = 1 << 0,    // Descriptor done
    RS_EOP = 1 << 1,   // End of packet
    RS_VP = 1 << 3,    // Packet is 802.1q (matched VET);
                       // indicates strip VLAN in 802.1q packet
    RS_UDPCS = 1 << 4, // UDP checksum calculated on packet
    RS_L4CS = 1 << 5,  // L4 (UDP or TCP) checksum calculated on packet
    RS_IPCS = 1 << 6,  // Ipv4 checksum calculated on packet
    RS_PIF = 1 << 7,   // Passed in-exact filter
};

// 传输命令
enum TCMD
{
    TCMD_EOP = 1 << 0,  // End of Packet
    TCMD_IFCS = 1 << 1, // Insert FCS
    TCMD_IC = 1 << 2,   // Insert Checksum
    TCMD_RS = 1 << 3,   // Report Status
    TCMD_RPS = 1 << 4,  // Report Packet Sent
    TCMD_VLE = 1 << 6,  // VLAN Packet Enable
    TCMD_IDE = 1 << 7,  // Interrupt Delay Enable
};

// 发送状态
enum TS
{
    TS_DD = 1 << 0, // Descriptor Done
    TS_EC = 1 << 1, // Excess Collisions
    TS_LC = 1 << 2, // Late Collision
    TS_TU = 1 << 3, // Transmit Underrun
};

e1000_t e1000;
uint32_t IRQ_E1000;

// 查找网卡设备
static pci_device_t *find_e1000_device()
{
    pci_device_t *device = NULL;

    for (int i = DEVICEID_LOW; i <= DEVICEID_HIGH; i++)
    {
        device = pci_find_device(VENDORID, i);
        if (device) {
            return device;
        }
    }
    return NULL;
}

// 检测只读存储器
static void e1000_eeprom_detect()
{
    uint32_t membase = e1000.dev->bar->iobase;
    moutl(membase + E1000_EERD, 0x1);
    for (int i = 0; i < 1000 && !e1000.eeprom; i++)
    {
        uint32_t val = minl(membase + E1000_EERD);
        if (val & 0x10)
            e1000.eeprom = TRUE;
        else
            e1000.eeprom = FALSE;
    }
}

static uint16_t e1000_eeprom_read(uint8_t addr)
{
    uint32_t membase = e1000.dev->bar->iobase;
    uint32_t tmp;
    if (e1000.eeprom)
    {
        moutl(membase + E1000_EERD, 1 | (uint32_t)addr << 8);
        while (!((tmp = minl(membase + E1000_EERD)) & (1 << 4)));
    }
    else
    {
        moutl(membase + E1000_EERD, 1 | (uint32_t)addr << 2);
        while (!((tmp = minl(membase + E1000_EERD)) & (1 << 1)));
    }
    return (tmp >> 16) & 0xFFFF;
}

static void e1000_read_mac()
{
    uint16_t val;
    e1000_eeprom_detect();
    if (e1000.eeprom) {
        val = e1000_eeprom_read(0);
        e1000.mac[0] = val & 0xFF;
        e1000.mac[1] = val >> 8;

        val = e1000_eeprom_read(1);
        e1000.mac[2] = val & 0xFF;
        e1000.mac[3] = val >> 8;

        val = e1000_eeprom_read(2);
        e1000.mac[4] = val & 0xFF;
        e1000.mac[5] = val >> 8;
    } else {
        char *mac = (char *)e1000.dev->bar->iobase + 0x5400;
        for (int i = 5; i >= 0; i--)
        {
            e1000.mac[i] = mac[i];
        }
    }
}

static int e1000_rx_init()
{
    uint32_t membase = e1000.dev->bar[0].iobase;

    e1000.rx = (rx_desc_t *)alloc_page();
    e1000.rx_now = 0;
    e1000.rx_buff = (desc_buff_t **)&e1000.rx[RX_DESC_NR];

    moutl(membase + E1000_RDBAL, (uint32_t)e1000.rx);
    moutl(membase + E1000_RDBAH, 0);
    moutl(membase + E1000_RDLEN, sizeof(rx_desc_t) * RX_DESC_NR);

    moutl(membase + E1000_RDH, 0);
    moutl(membase + E1000_RDT, RX_DESC_NR - 1);

    for (int i = 0; i < RX_DESC_NR; i++)
    {
        desc_buff_t *buff = (desc_buff_t *)alloc_desc_buff();
        e1000.rx_buff[i] = buff;
        e1000.rx[i].address = (uint32_t)get_paddr(0, (uint32_t)&buff->payload);
        e1000.rx[i].status = 0;
    }

    uint32_t value = RCTL_EN | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_2048;
    moutl(membase + E1000_RCTL, value);
}

static int e1000_tx_init()
{
    uint32_t membase = e1000.dev->bar[0].iobase;

    e1000.tx = (tx_desc_t *)alloc_page();
    e1000.tx_now = 0;
    e1000.tx_buff = (desc_buff_t **)&e1000.tx[RX_DESC_NR];

    moutl(membase + E1000_TDBAL, (uint32_t)e1000.tx);
    moutl(membase + E1000_TDBAH, 0);
    moutl(membase + E1000_TDLEN, sizeof(tx_desc_t) * TX_DESC_NR);

    moutl(membase + E1000_TDH, 0);
    moutl(membase + E1000_TDT, 0);

    for (int i = 0; i < TX_DESC_NR; i++)
    {
        e1000.tx[i].address = 0;
        e1000.tx[i].sta = TS_DD;
    }

    uint32_t value = TCTL_EN | TCTL_PSP | TCTL_RTLC | (0x10 << TCTL_CT) | (0x40 << TCTL_COLD);
    moutl(membase + E1000_TCTL, value);
}

static void e1000_reset()
{
    uint32_t membase = e1000.dev->bar[0].iobase;
    // 读取MAC地址
    e1000_read_mac();
    logf("MAC address: %02X-%02X-%02X-%02X-%02X-%02X",
        e1000.mac[0], e1000.mac[1], e1000.mac[2], e1000.mac[3], e1000.mac[4], e1000.mac[5]);
    // Set Link Up
    moutl(membase + E1000_CTRL, (minl(membase + E1000_CTRL) | CTRL_SLU));
    // 初始化组播表数组
    for (int i = E1000_MAT0; i < E1000_MAT1; i += 4)
        moutl(membase + i, 0);
    // 禁用中断
    moutl(membase + E1000_IMS, 0);
    // 初始化缓冲区
    desc_buff_init();
    // 接收初始化
    e1000_rx_init();
    // 传输初始化
    e1000_tx_init();
    // 初始化中断
    int value = IMS_RXT0 | IMS_RXO | IMS_RXDMT0 | IMS_RXSEQ | IMS_LSC | IMS_TXQE | IMS_TXDW | IMS_TXD_LOW;
    moutl(membase + E1000_IMS, value);
}

static void receive_packet()
{
    uint32_t membase = e1000.dev->bar[0].iobase;
    while (TRUE) {
        rx_desc_t *rx = &e1000.rx[e1000.rx_now];
        if (!(rx->status & RS_DD)) return;
        if (rx->errors)
        {
        }
        desc_buff_t *buff = e1000.rx_buff[e1000.rx_now];
        buff->length = rx->length;

        netif_input(buff);
        
        buff = alloc_desc_buff();
        e1000.rx_buff[e1000.rx_now] = buff;
        rx->address = (uint32_t)get_paddr(0, (uint32_t)&buff->payload);
        rx->status = 0;
        moutl(membase + E1000_RDT, e1000.rx_now);
        e1000.rx_now = (e1000.rx_now + 1) % RX_DESC_NR;
    }
}

static void send_packet(eth_t *eth, uint16_t length)
{
    uint32_t membase = e1000.dev->bar[0].iobase;
    tx_desc_t *tx = &e1000.tx[e1000.tx_now];
    tx->address = (uint64_t)(void *)eth;
    tx->length = length;
    tx->cmd = TCMD_EOP | TCMD_RPS | TCMD_RS | TCMD_IFCS;
    tx->sta = 0;
    uint16_t tx_old = e1000.tx_now;
    e1000.tx_now = (e1000.tx_now + 1) % TX_DESC_NR;
    moutl(membase + E1000_TDT, e1000.tx_now);
    while(!(e1000.tx[tx_old].sta & 0xF));
}

void handler_e1000(interrupt_frame_t* frame)
{
    uint32_t pde = read_cr3();
    reset_pde();
    uint32_t membase = e1000.dev->bar[0].iobase;
    uint32_t status = minl(membase + E1000_ICR);
    // 传输描述符写回，表示有一个数据包发送完毕
    if (status & IMS_TXDW)
    {
    }
    // 传输队列为空，并且传输进程阻塞
    if (status & IMS_TXQE)
    {
    }
    // 连接状态改变
    if (status & IMS_LSC)
    {
    }
    // Overrun
    if (status & IMS_RXO)
    {
    }
    if (status & IMS_RXDMT0)
    {
    }
    if (status & IMS_RXT0)
    {
        receive_packet();
    }
    write_cr3(pde);
    send_eoi(IRQ_E1000);
}

e1000_t *e1000_init()
{
    // 查找PCI设备
    pci_device_t *dev = find_e1000_device();
    if (!dev) {
        logf("find e1000 pci device failed!");
        return NULL;
    }
    e1000.dev = dev;
    kernel_strcpy(e1000.name, "e1000");
    // 获取 Base Address Register
    pci_set_bars(e1000.dev);
    pci_bar_t *bar = &e1000.dev->bar[0];
    if (bar->index < 0) {
        logf("set e1000 bar failed");
        return NULL;
    }
    task_t *task = get_running_task();
    // 映射EEPROM
    if (map_area(bar->iobase, bar->size) < 0) {
        logf("mapping eeprom failed");
        return NULL;
    }
    // 启用总线主控
    pci_enable_busmastering(e1000.dev);
    // 重置网卡
    e1000_reset();
    // 初始化ARP缓存
    arp_map_init();
    // 安装IRQ
    IRQ_E1000 = pci_interrupt(e1000.dev) + 0x20;
    if (IRQ_E1000 != IRQ1_NIC) {
        logf("pci interrupt get failed");
        return NULL;
    }
    install_interrupt_handler(IRQ_E1000, (uint32_t)interrupt_handler_e1000);
    irq_enable(IRQ_E1000);
    irq_enable(IRQ0_CASCADE);
    return &e1000;
}

void e1000_send_packet(desc_buff_t *buff)
{
    uint32_t pde = read_cr3();
    reset_pde();
    send_packet((eth_t *)buff->payload, buff->length);
    write_cr3(pde);
}