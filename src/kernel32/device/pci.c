#include <pci.h>
#include <logf.h>
#include <csos/string.h>

pci_classname_t pci_classnames[] = {
    {0x000000, "Non-VGA unclassified device"},
    {0x000100, "VGA compatible unclassified device"},
    {0x010000, "SCSI storage controller"},
    {0x010100, "IDE interface"},
    {0x010200, "Floppy disk controller"},
    {0x010300, "IPI bus controller"},
    {0x010400, "RAID bus controller"},
    {0x018000, "Unknown mass storage controller"},
    {0x020000, "Ethernet controller"},
    {0x020100, "Token ring network controller"},
    {0x020200, "FDDI network controller"},
    {0x020300, "ATM network controller"},
    {0x020400, "ISDN controller"},
    {0x028000, "Network controller"},
    {0x030000, "VGA controller"},
    {0x030100, "XGA controller"},
    {0x030200, "3D controller"},
    {0x038000, "Display controller"},
    {0x040000, "Multimedia video controller"},
    {0x040100, "Multimedia audio controller"},
    {0x040200, "Computer telephony device"},
    {0x048000, "Multimedia controller"},
    {0x050000, "RAM memory"},
    {0x050100, "FLASH memory"},
    {0x058000, "Memory controller"},
    {0x060000, "Host bridge"},
    {0x060100, "ISA bridge"},
    {0x060200, "EISA bridge"},
    {0x060300, "MicroChannel bridge"},
    {0x060400, "PCI bridge"},
    {0x060500, "PCMCIA bridge"},
    {0x060600, "NuBus bridge"},
    {0x060700, "CardBus bridge"},
    {0x060800, "RACEway bridge"},
    {0x060900, "Semi-transparent PCI-to-PCI bridge"},
    {0x060A00, "InfiniBand to PCI host bridge"},
    {0x068000, "Bridge"},
    {0x070000, "Serial controller"},
    {0x070100, "Parallel controller"},
    {0x070200, "Multiport serial controller"},
    {0x070300, "Modem"},
    {0x078000, "Communication controller"},
    {0x080000, "PIC"},
    {0x080100, "DMA controller"},
    {0x080200, "Timer"},
    {0x080300, "RTC"},
    {0x080400, "PCI Hot-plug controller"},
    {0x088000, "System peripheral"},
    {0x090000, "Keyboard controller"},
    {0x090100, "Digitizer Pen"},
    {0x090200, "Mouse controller"},
    {0x090300, "Scanner controller"},
    {0x090400, "Gameport controller"},
    {0x098000, "Input device controller"},
    {0x0A0000, "Generic Docking Station"},
    {0x0A8000, "Docking Station"},
    {0x0B0000, "386"},
    {0x0B0100, "486"},
    {0x0B0200, "Pentium"},
    {0x0B1000, "Alpha"},
    {0x0B2000, "Power PC"},
    {0x0B3000, "MIPS"},
    {0x0B4000, "Co-processor"},
    {0x0C0000, "FireWire (IEEE 1394)"},
    {0x0C0100, "ACCESS Bus"},
    {0x0C0200, "SSA"},
    {0x0C0300, "USB Controller"},
    {0x0C0400, "Fiber Channel"},
    {0x0C0500, "SMBus"},
    {0x0C0600, "InfiniBand"},
    {0x0D0000, "IRDA controller"},
    {0x0D0100, "Consumer IR controller"},
    {0x0D1000, "RF controller"},
    {0x0D8000, "Wireless controller"},
    {0x0E0000, "I2O"},
    {0x0F0000, "Satellite TV controller"},
    {0x0F0100, "Satellite audio communication controller"},
    {0x0F0300, "Satellite voice communication controller"},
    {0x0F0400, "Satellite data communication controller"},
    {0x100000, "Network and computing encryption device"},
    {0x101000, "Entertainment encryption device"},
    {0x108000, "Encryption controller"},
    {0x110000, "DPIO module"},
    {0x110100, "Performance counters"},
    {0x111000, "Communication synchronizer"},
    {0x118000, "Signal processing controller"},
    {0x000000, NULL}
};

pci_device_t devices[PCI_DEVICE_SIZE];

static pci_device_t *pci_alloc_device()
{
    for (int i = 0; i < PCI_DEVICE_SIZE; i++) {
        pci_device_t *dev = &devices[i];
        if (dev) {
            return dev;
        }
    }
    return NULL;
}

const char *pci_classname(uint32_t classcode)
{
    for (int i = 0; pci_classnames[i].name != NULL; i++)
    {
        if (pci_classnames[i].classcode == classcode)
            return pci_classnames[i].name;
        if (pci_classnames[i].classcode == (classcode & 0xFFFF00))
            return pci_classnames[i].name;
    }
    return "Unknown device";
}

static inline uint32_t pci_addr(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr)
{
    return (uint32_t)(0x80000000) | ((bus & 0xff) << 16) | ((dev & 0x1f) << 11) | ((func & 0x7) << 8) | addr;
}

uint32_t pci_inl(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr)
{
    outl(PCI_CONF_ADDR, pci_addr(bus, dev, func, addr));
    return inl(PCI_CONF_DATA);
}

void pci_outl(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr, uint32_t value)
{
    outl(PCI_CONF_ADDR, pci_addr(bus, dev, func, addr));
    outl(PCI_CONF_DATA, value);
}

static void pci_check_device(uint8_t bus, uint8_t dev)
{
    uint32_t value = 0;
    for (uint8_t func = 0; func < 8; func++) {
        value = pci_inl(bus, dev, func, PCI_CONF_VENDOR);
        uint16_t vendorid = value & 0xFFFF;
        if (vendorid == 0 || vendorid == 0xFFFF)
            return;
        pci_device_t *device = pci_alloc_device();
        device->bus = bus;
        device->dev = dev;
        device->func = func;
        device->vendorid = vendorid;
        device->deviceid = value >> 16;
        value = pci_inl(bus, dev, func, PCI_CONF_REVISION);
        device->classcode = value >> 8;
        device->revision = value & 0xFF;
        logf("PCI %02x:%02x.%x %4x:%4x %s",
            device->bus, device->dev, device->func, device->vendorid, device->deviceid,
            pci_classname(device->classcode));
    }
}

// PCI 总线枚举
static void pci_enum_device()
{
    for (int bus = 0; bus < 256; bus++)
    {
        for (int dev = 0; dev < 32; dev++)
        {
            pci_check_device(bus, dev);
        }
    }
}

void pci_init()
{
    kernel_memset(devices, 0, sizeof(devices));
    pci_enum_device();
}