import tkinter as tk
from tkinter import ttk
import numpy as np
import time
import threading
import socket
import struct
import pygame

# UDP
SENSOR_UDP_PORT = 1235
SENSOR_BUFFER_SIZE = 256

class RobotControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Robot Control")
        self.root.geometry("900x900")
        self.root.resizable(True, True)

        # Configure root grid
        self.root.grid_columnconfigure(0, weight=0)
        self.root.grid_columnconfigure(1, weight=1)
        self.root.grid_rowconfigure(1, weight=1)

        self.arduino_ip = "172.20.10.3"
        self.arduino_port = 1234
        self.sock = None

        self.active_keys = set()
        self.velocities = {'forward': 0.0, 'angular': 0.0}
        self.VELOCITY_SCALE = 1.0

        self.style = ttk.Style()
        self.style.configure("Custom.Treeview", rowheight=40)

        # Store last sent and last sensor reading
        self.last_sent = ([], [])   # payload list, raw frame list
        self.last_sensor = ("", 0.0, 0.0, False)
        self.last_raw = []          # 新增：存放最后收到的原始数据字节列表

        # Current selection for sensor updates
        self.current_selected_row = None

        pygame.init()
        pygame.joystick.init()
        if pygame.joystick.get_count() > 0:
            self.joystick = pygame.joystick.Joystick(0)
            self.joystick.init()
        else:
            self.joystick = None

        # Build UI
        self.setup_gui()
        self.sensor_rows = []
        self.setup_sensor_table()

        # UDP listener
        self.udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp_sock.bind(("", SENSOR_UDP_PORT))
        self.sensor_running = True
        threading.Thread(target=self.udp_listener, daemon=True).start()

        self.setup_key_bindings()
        self.connect()
        self.running = True

        threading.Thread(target=self.send_packets, daemon=True).start()
        self.update_vel_labels()


    def setup_gui(self):
        frm = ttk.Frame(self.root, padding=10)
        frm.grid(row=0, column=0, columnspan=2, sticky="ew")
        frm.grid_columnconfigure(0, weight=1)

        # Connection
        conn = ttk.LabelFrame(frm, text="Connection", padding=5)
        conn.grid(row=0, column=0, pady=5, sticky="w")
        ttk.Label(conn, text="Arduino IP:").grid(row=0, column=0)
        self.ip_entry = ttk.Entry(conn, width=15)
        self.ip_entry.insert(0, self.arduino_ip)
        self.ip_entry.grid(row=0, column=1)
        ttk.Button(conn, text="Connect", command=self.connect).grid(row=0, column=2)
        self.status_label = ttk.Label(conn, text="Disconnected", foreground="red")
        self.status_label.grid(row=1, column=0, columnspan=3, sticky="w")

        # Velocities
        vel = ttk.LabelFrame(frm, text="Velocities", padding=5)
        vel.grid(row=1, column=0, pady=5, sticky="w")
        ttk.Label(vel, text="Forward").grid(row=0, column=0)
        self.lbl_fwd = ttk.Label(vel, text="0.00")
        self.lbl_fwd.grid(row=0, column=1)
        ttk.Label(vel, text="Angular").grid(row=0, column=2)
        self.lbl_ang = ttk.Label(vel, text="0.00")
        self.lbl_ang.grid(row=0, column=3)

        # Sent Packet
        console = ttk.LabelFrame(frm, text="Sent Packet", padding=5)
        console.grid(row=2, column=0, columnspan=2, pady=5, sticky="ew")
        console.grid_columnconfigure(0, weight=1)

        self.msg_display = tk.Text(console, height=6, wrap='none', state='disabled')
        self.msg_display.grid(row=0, column=0, sticky="ew")

        ttk.Label(frm, text="Use W/S to move forward/back, A/D to turn.").grid(row=3, column=0, columnspan=2, sticky="w")

    def setup_sensor_table(self):
        table_frame = ttk.Frame(self.root, padding=(10, 0, 10, 5))
        table_frame.grid(row=1, column=1, sticky="nsew")
        table_frame.grid_columnconfigure(0, weight=1)
        table_frame.grid_rowconfigure(0, weight=1)

        cols = ("index", "spice", "name", "rf", "ir", "mag")
        self.sensor_tree = ttk.Treeview(table_frame, columns=cols, show="headings", style="Custom.Treeview")
        self.sensor_tree.heading('index', text='#')
        self.sensor_tree.column('index', width=30, anchor='center')
        self.sensor_tree.heading('spice', text='Spice')
        self.sensor_tree.column('spice', width=150, anchor='center')
        headings = {'name': 'Name', 'rf': 'RF Frequency', 'ir': 'IR Frequency', 'mag': 'Mag Dir'}
        widths = {'name': 150, 'rf': 150, 'ir': 150, 'mag': 150}
        for c in ('name', 'rf', 'ir', 'mag'):
            self.sensor_tree.heading(c, text=headings[c])
            self.sensor_tree.column(c, width=widths[c], anchor='center')

        for i in range(8):
            row_id = self.sensor_tree.insert('', 'end', values=(i+1, '', '', '', '', ''))
            self.sensor_rows.append(row_id)

        self.sensor_tree.grid(row=0, column=0, sticky="nsew")
        h_scroll = ttk.Scrollbar(table_frame, orient='horizontal', command=self.sensor_tree.xview)
        h_scroll.grid(row=1, column=0, sticky='ew')
        self.sensor_tree.configure(xscrollcommand=h_scroll.set)

        self.sensor_tree.bind('<<TreeviewSelect>>', self.on_row_select)

        btn_frame = ttk.Frame(self.root)
        btn_frame.grid(row=2, column=1, pady=(0,10))
        self.send_btn = ttk.Button(btn_frame, text="Send")
        self.send_btn.grid(row=0, column=0, padx=5)
        self.record_btn = ttk.Button(btn_frame, text="Record", command=self.record)
        self.record_btn.grid(row=0, column=1, padx=5)
        self.delete_btn = ttk.Button(btn_frame, text="Delete", command=self.delete_row)
        self.delete_btn.grid(row=0, column=2, padx=5)

        self.update_send_button()

    def on_row_select(self, event):
        sel = self.sensor_tree.selection()
        if sel:
            self.current_selected_row = sel[0]
            self.record_btn.config(state='normal')
            self.delete_btn.config(state='normal')
        else:
            self.current_selected_row = None
            self.record_btn.config(state='disabled')
            self.delete_btn.config(state='disabled')

    def update_send_button(self):
        self.send_btn.config(state='normal')

    def record(self):
        if not self.current_selected_row:
            return
        name, rf, ir, mag = self.last_sensor
        idx = self.sensor_tree.item(self.current_selected_row, 'values')[0]
        self.sensor_tree.item(
            self.current_selected_row,
            values=(idx, '', name, f"{rf:.2f}", f"{ir:.2f}", str(mag))
        )

    def delete_row(self):
        if not self.current_selected_row:
            return
        idx = self.sensor_tree.item(self.current_selected_row, 'values')[0]
        self.sensor_tree.item(self.current_selected_row, values=(idx, '', '', '', '', ''))

    def udp_listener(self):
        while self.sensor_running:
            data, _ = self.udp_sock.recvfrom(SENSOR_BUFFER_SIZE)
            raw = list(data)
            try:
                if len(data) < 1+4+4+4+1 or data[0] != 0xA5:
                    raise ValueError("Invalid packet")
                offset = 1
                rf, = struct.unpack_from('<f', data, offset); offset += 4
                ir, = struct.unpack_from('<f', data, offset); offset += 4
                mag_int, = struct.unpack_from('<i', data, offset); offset += 4
                name_bytes = data[offset:].split(b'\x00', 1)[0]
                name = name_bytes.decode('utf-8', errors='ignore')
                mag = bool(mag_int)

                # 保存原始字节到 last_raw
                self.last_sensor = (name, rf, ir, mag)
                self.last_raw = raw

                self.root.after(0, self.refresh_console)
            except Exception:
                continue

    def send_packets(self):
        while self.running:
            time.sleep(0.1)
            if not self.sock:
                continue
            frame, payload = self.create_raw_packet()
            try:
                self.sock.sendto(frame, (self.arduino_ip, self.arduino_port))
                self.last_sent = (payload, list(frame))
                self.root.after(0, self.refresh_console)
            except Exception as e:
                print("UDP send exception:", e)

    def refresh_console(self):
        sent_payload, sent_raw = self.last_sent
        name, rf, ir, mag = self.last_sensor

        self.msg_display.config(state="normal")
        self.msg_display.delete("1.0", "end")

        # Sent info
        self.msg_display.insert("end", f"Sent Decoded: {sent_payload}\n")
        self.msg_display.insert("end", f"Sent Raw Bytes: {sent_raw}\n")

        # Received info (使用 last_raw)
        self.msg_display.insert("end", f"Received Raw Data: {self.last_raw}\n")
        self.msg_display.insert(
            "end",
            f"Parsed Sensor -> Name: {name}, RF: {rf:.2f}, IR: {ir:.2f}, Mag: {mag}\n"
        )

        self.msg_display.config(state="disabled")

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
        self.root.bind("<KeyPress>", self.on_key_press)
        self.root.bind("<KeyRelease>", self.on_key_release)

    def on_key_press(self, e):
        k = e.keysym.lower()
        if k in ('w','s','a','d'):
            self.active_keys.add(k)

    def on_key_release(self, e):
        k = e.keysym.lower()
        self.active_keys.discard(k)

    def update_vel_labels(self):
        f_kb, a_kb = 0.0, 0.0
        if 'w' in self.active_keys: f_kb += self.VELOCITY_SCALE
        if 's' in self.active_keys: f_kb -= self.VELOCITY_SCALE
        if 'd' in self.active_keys: a_kb += self.VELOCITY_SCALE
        if 'a' in self.active_keys: a_kb -= self.VELOCITY_SCALE

        f_xbox, a_xbox = 0.0, 0.0
        if self.joystick:
            pygame.event.pump()
            axis_y = self.joystick.get_axis(1)  # 上下
            if abs(axis_y) < 0.1: axis_y = 0.0  # deadzone
            axis_x = self.joystick.get_axis(0)
            if abs(axis_x) < 0.1: axis_x = 0.0
            f_xbox = -axis_y * self.VELOCITY_SCALE
            a_xbox = axis_x * self.VELOCITY_SCALE
        f = np.clip(f_kb + f_xbox, -1.0, 1.0)
        a = np.clip(a_kb + a_xbox, -1.0, 1.0)
        self.velocities['forward'], self.velocities['angular'] = f, a
        self.lbl_fwd.config(text=f"{f:.2f}")
        self.lbl_ang.config(text=f"{a:.2f}")
        self.root.after(20, self.update_vel_labels)

    def create_raw_packet(self):
        f = self.velocities['forward']
        a = self.velocities['angular']
        left_val  = np.clip(f - a, -1.0, 1.0)
        right_val = np.clip(f + a, -1.0, 1.0)
        dir_left = 1 if left_val >= 0 else 0
        dir_right = 0 if right_val >= 0 else 1
        def to_pwm(v):
            if abs(v) < 1e-6:
                return 0
            return int(np.clip(128 + abs(v) * (255 - 128), 128, 255))
        pwm_left  = to_pwm(left_val)
        pwm_right = to_pwm(right_val)
        payload_bytes = struct.pack('>4B', pwm_left, pwm_right, dir_left, dir_right)
        frame = b'\x0F' + payload_bytes
        return frame, [pwm_left, pwm_right, dir_left, dir_right]

    def cleanup(self):
        self.running = False
        self.sensor_running = False
        if self.sock:
            self.sock.close()

if __name__ == "__main__":
    root = tk.Tk()
    app = RobotControlGUI(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (app.cleanup(), root.destroy()))
    root.mainloop()
