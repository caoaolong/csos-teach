from scapy.layers.l2 import Ether, ARP
from scapy.sendrecv import sendp

if __name__ == "__main__":
    # 目标 IP 地址
    target_ip = "192.168.10.1"
    # 构造以太网帧（广播目的地址）
    eth = Ether(dst="ff:ff:ff:ff:ff:ff")
    # 构造 ARP 请求包（who-has 请求）
    arp = ARP(pdst=target_ip)
    # 组合以太网帧和 ARP 请求
    packet = eth / arp
    # 发送数据包（指定网卡可以加 iface="eth0"）
    sendp(packet)
