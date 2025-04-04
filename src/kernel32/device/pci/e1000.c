#include <pci/e1000.h>
#include <logf.h>
#include <mio.h>
#include <csos/string.h>
#include <csos/memory.h>

#define VENDORID        0x8086 // 供应商英特尔
#define DEVICEID_LOW    0x1000
#define DEVICEID_HIGH   0x1028

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

e1000_t e1000;

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

static void e1000_reset()
{
    e1000_read_mac();
    logf("MAC address: %2X-%2X-%2X-%2X-%2X-%2X",
        e1000.mac[0], e1000.mac[1], e1000.mac[2], e1000.mac[3], e1000.mac[4], e1000.mac[5]);
}

void e1000_init()
{
    // 查找PCI设备
    pci_device_t *dev = find_e1000_device();
    if (!dev) {
        logf("find e1000 pci device failed!");
        return;
    }
    e1000.dev = dev;
    kernel_strcpy(e1000.name, "e1000");
    // 启用总线主控
    pci_enable_busmastering(e1000.dev);
    // 获取 Base Address Register
    pci_set_bars(e1000.dev);
    pci_bar_t *bar = &e1000.dev->bar[0];
    if (bar->index < 0) {
        logf("set e1000 bar failed");
        return;
    }
    // 映射EEPROM
    if (map_area(bar->iobase, bar->size) < 0) {
        logf("mapping eeprom failed");
        return;
    }
    // 重置网卡
    e1000_reset();
}