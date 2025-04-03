#include <pci.h>
#include <csos/string.h>

pci_device_t devices[PCI_DEVICE_SIZE];

void pci_init()
{
    kernel_memset(devices, 0, sizeof(devices));
}