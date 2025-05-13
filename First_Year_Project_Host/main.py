import tkinter as tk
from tkinter import ttk
import numpy as np
import time
import threading
import socket
import struct

class RobotControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Robot Control")

        # 默认 Arduino IP/端口
        self.arduino_ip = "172.20.10.3"
        self.arduino_port = 1234
        self.sock = None

        self.active_keys = set()
        self.velocities = {'forward': 0.0, 'angular': 0.0}
        self.VELOCITY_SCALE = 1.0

        self.setup_gui()
        self.setup_key_bindings()
        self.connect()
        self.running = True

        threading.Thread(target=self.send_packets, daemon=True).start()
        self.update_vel_labels()

    def setup_gui(self):
        frm = ttk.Frame(self.root, padding=10)
        frm.grid()

        # 连接设置
        conn = ttk.LabelFrame(frm, text="Connection", padding=5)
        conn.grid(row=0, pady=5)
        ttk.Label(conn, text="Arduino IP:").grid(row=0, column=0)
        self.ip_entry = ttk.Entry(conn, width=15)
        self.ip_entry.insert(0, self.arduino_ip)
        self.ip_entry.grid(row=0, column=1)
        ttk.Button(conn, text="Connect", command=self.connect).grid(row=0, column=2)
        self.status_label = ttk.Label(conn, text="Disconnected", foreground="red")
        self.status_label.grid(row=1, column=0, columnspan=3, sticky="w")

        # 速度显示
        vel = ttk.LabelFrame(frm, text="Velocities", padding=5)
        vel.grid(row=1, pady=5, sticky="w")
        ttk.Label(vel, text="Forward").grid(row=0, column=0)
        self.lbl_fwd = ttk.Label(vel, text="0.00"); self.lbl_fwd.grid(row=0, column=1)
        ttk.Label(vel, text="Angular").grid(row=0, column=2)
        self.lbl_ang = ttk.Label(vel, text="0.00"); self.lbl_ang.grid(row=0, column=3)

        # 调试输出
        dbg = ttk.LabelFrame(frm, text="Sent Packet", padding=5)
        dbg.grid(row=2, pady=5)
        self.msg_display = tk.Text(dbg, width=50, height=6, state="disabled")
        self.msg_display.grid()

        ttk.Label(frm, text="Use W/S to move forward/back, A/D to turn.").grid(row=3, sticky="w")

    def connect(self):
        self.arduino_ip = self.ip_entry.get()
        try:
            if self.sock:
                self.sock.close()
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.status_label.config(text="UDP Ready", foreground="green")
        except Exception as e:
            self.status_label.config(text=f"Init failed: {e}", foreground="red")
            self.sock = None

    def setup_key_bindings(self):
        self.root.bind("<KeyPress>",   self.on_key_press)
        self.root.bind("<KeyRelease>", self.on_key_release)

    def on_key_press(self, e):
        k = e.keysym.lower()
        if k in ('w','s','a','d'):
            self.active_keys.add(k)

    def on_key_release(self, e):
        k = e.keysym.lower()
        self.active_keys.discard(k)

    def update_vel_labels(self):
        f = 0.0; a = 0.0
        if 'w' in self.active_keys: f += self.VELOCITY_SCALE
        if 's' in self.active_keys: f -= self.VELOCITY_SCALE
        if 'd' in self.active_keys: a += self.VELOCITY_SCALE
        if 'a' in self.active_keys: a -= self.VELOCITY_SCALE

        f = np.clip(f, -1.0, 1.0)
        a = np.clip(a, -1.0, 1.0)
        self.velocities['forward'] = f
        self.velocities['angular'] = a

        self.lbl_fwd.config(text=f"{f:.2f}")
        self.lbl_ang.config(text=f"{a:.2f}")

        self.root.after(20, self.update_vel_labels)

    def create_raw_packet(self):
        f = self.velocities['forward']
        a = self.velocities['angular']
        # 差速
        left_val  = f - a
        right_val = f + a
        # 方向位
        dir_left  = 1 if left_val  >= 0 else 0
        dir_right = 0 if right_val >= 0 else 1
        # PWM 映射
        pwm_left  = int(np.clip(abs(left_val)  * 255, 0, 255))
        pwm_right = int(np.clip(abs(right_val) * 255, 0, 255))

        # 1 字节起始码 + 4 字节固定 payload
        payload_bytes = struct.pack('>4B', pwm_left, pwm_right, dir_left, dir_right)
        frame = b'\x0F' + payload_bytes
        return frame, [pwm_left, pwm_right, dir_left, dir_right]

    def send_packets(self):
        while self.running:
            if not self.sock:
                time.sleep(0.1)
                continue
            frame, payload = self.create_raw_packet()
            try:
                self.sock.sendto(frame, (self.arduino_ip, self.arduino_port))
                # 把调试信息传回主线程更新 UI
                self.root.after(0, lambda p=payload, f=list(frame): self.display_msg(p, f))
            except Exception as e:
                print("UDP send exception:", e)
            time.sleep(0.1)

    def display_msg(self, data_arr, raw_bytes):
        self.msg_display.config(state="normal")
        self.msg_display.delete("1.0", "end")
        self.msg_display.insert("end", f"Decoded: {data_arr}\n")
        self.msg_display.insert("end", f"Raw Bytes: {raw_bytes}\n")
        self.msg_display.config(state="disabled")

    def cleanup(self):
        self.running = False
        if self.sock:
            self.sock.close()

def main():
    root = tk.Tk()
    app = RobotControlGUI(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (app.cleanup(), root.destroy()))
    root.mainloop()

if __name__ == "__main__":
    main()
