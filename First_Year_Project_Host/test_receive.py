# receiver.py
import socket

UDP_IP = "0.0.0.0"   # 监听本机所有网卡
UDP_PORT = 1234     # 与 Arduino 一致的端口

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
print(f"Listening on UDP {UDP_IP}:{UDP_PORT} ...")

while True:
    data, addr = sock.recvfrom(1024)  # buffer size 1024 bytes
    print(f"Received {len(data)} bytes from {addr}: {data!r}")
