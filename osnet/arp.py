from scapy.layers.l2 import Ether, ARP
from scapy.sendrecv import sendp
from iflist import show_if_list

tap = "\\Device\\NPF_{5BBDD45B-479E-4A03-9152-341F8DD7D3C1}"

def send_arp():
    # 目标 IP 地址
    target_ip = "192.168.10.100"
    # 构造以太网帧（广播目的地址）
    eth = Ether(dst="ff:ff:ff:ff:ff:ff")
    # 构造 ARP 请求包（who-has 请求）
    arp = ARP(pdst=target_ip)
    # 组合以太网帧和 ARP 请求
    packet = eth / arp
    # 发送数据包（指定网卡可以加 iface="tap"）
    sendp(packet, iface=tap)

if __name__ == "__main__":
    show_if_list()
    # send_arp()