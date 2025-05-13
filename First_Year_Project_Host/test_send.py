# sender.py
import socket
import sys
import time

# 默认目标（本机 loopback），可以改成 Arduino 的 IP
TARGET_IP = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
TARGET_PORT = 1234

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

count = 0
try:
    while True:
        message = f"hello {count}".encode('utf-8')
        sock.sendto(message, (TARGET_IP, TARGET_PORT))
        print(f"Sent {len(message)} bytes to {TARGET_IP}:{TARGET_PORT}: {message!r}")
        count += 1
        time.sleep(1.0)
except KeyboardInterrupt:
    print("\nStopped by user")
