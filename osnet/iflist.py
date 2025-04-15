from scapy.all import get_if_list, get_if_addr, get_if_hwaddr

def show_if_list():
    interfaces = get_if_list()
    print("所有网卡信息：\n")
    for iface in interfaces:
        try:
            ip = get_if_addr(iface)
        except:
            ip = "无IP"
        try:
            mac = get_if_hwaddr(iface)
        except:
            mac = "无MAC"
        print(f"接口: {iface}")
        print(f"  IP地址:  {ip}")
        print(f"  MAC地址: {mac}")
        print("-----------")
