from scapy.all import *
from scapy.layers.inet import IP, ICMP

if __name__ == "__main__":
    # 创建一个 ICMP 请求包
    packet = IP(dst="192.168.10.100") / ICMP()
    # 发送并接收响应
    response = sr1(packet, timeout=2)
    # 显示响应内容
    if response:
        response.show()
    else:
        print("请求超时，无响应。")
