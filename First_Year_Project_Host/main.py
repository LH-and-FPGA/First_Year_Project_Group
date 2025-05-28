import tkinter as tk
from tkinter import ttk
import numpy as np
import time
import threading
import socket
import struct

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
        conn.grid(row=0, column=0, pady=5)
        ttk.Label(conn, text="Arduino IP:").grid(row=0, column=0)
        self.ip_entry = ttk.Entry(conn, width=15)
        self.ip_entry.insert(0, self.arduino_ip)
        self.ip_entry.grid(row=0, column=1)
        ttk.Button(conn, text="Connect", command=self.connect).grid(row=0, column=2)
        self.status_label = ttk.Label(conn, text="Disconnected", foreground="red")
        self.status_label.grid(row=1, column=0, columnspan=3, sticky="w")

        # Velocities
        vel = ttk.LabelFrame(frm, text="Velocities", padding=5)
        vel.grid(row=1, column=0, pady=5)
        ttk.Label(vel, text="Forward").grid(row=0, column=0)
        self.lbl_fwd = ttk.Label(vel, text="0.00")
        self.lbl_fwd.grid(row=0, column=1)
        ttk.Label(vel, text="Angular").grid(row=0, column=2)
        self.lbl_ang = ttk.Label(vel, text="0.00")
        self.lbl_ang.grid(row=0, column=3)

        # Sent Packet
        dbg = ttk.LabelFrame(frm, text="Sent Packet", padding=5)
        dbg.grid(row=2, column=0, columnspan=2, pady=5, sticky="ew")
        dbg.grid_columnconfigure(0, weight=1)
        dbg.grid_rowconfigure(0, weight=1)

        self.msg_display = tk.Text(dbg, height=6, wrap='none')
        self.msg_display.grid(row=0, column=0, sticky="ew")
        # remove vertical scrollbar

        ttk.Label(frm, text="Use W/S to move forward/back, A/D to turn.").grid(row=3, column=0, columnspan=2, sticky="w")

    def setup_sensor_table(self):
        # One Send button at bottom
        # Table frame
        table_frame = ttk.Frame(self.root, padding=(10, 0, 10, 5))
        table_frame.grid(row=1, column=1, sticky="nsew")
        table_frame.grid_columnconfigure(0, weight=1)
        table_frame.grid_rowconfigure(0, weight=1)

        cols = ("index", "spice", "name", "rf", "ir", "mag")
        self.sensor_tree = ttk.Treeview(table_frame, columns=cols, show="headings",  style="Custom.Treeview", xscrollcommand=20)
        # table_frame.bind("<Configure>", self.on_table_resize)
        # Index column
        self.sensor_tree.heading('index', text='#')
        self.sensor_tree.column('index', width=30, anchor='center')
        # Spice placeholder
        self.sensor_tree.heading('spice', text='Spice')
        self.sensor_tree.column('spice', width=150, anchor='center')
        # Other columns
        headings = {'name': 'Name', 'rf': 'RF Frequency', 'ir': 'IR Frequency', 'mag': 'Mag Dir'}
        widths = {'name': 150, 'rf': 150, 'ir': 150, 'mag': 150}
        for c in ('name', 'rf', 'ir', 'mag'):
            self.sensor_tree.heading(c, text=headings[c])
            self.sensor_tree.column(c, width=widths[c], anchor='center')

        # Pre-fill 8 rows with index
        for i in range(8):
            row_id = self.sensor_tree.insert('', 'end', values=(i+1, '', '', '', '', ''))
            self.sensor_rows.append(row_id)

        self.sensor_tree.grid(row=0, column=0, sticky="nsew")
        # Horizontal scrollbar only
        h_scroll = ttk.Scrollbar(table_frame, orient='horizontal', command=self.sensor_tree.xview)
        h_scroll.grid(row=1, column=0, sticky='ew')
        self.sensor_tree.configure(xscrollcommand=h_scroll.set)

        # Send button below
        send_btn = ttk.Button(self.root, text="Send")
        send_btn.grid(row=2, column=1, pady=(0,10), sticky="e")

    def on_table_resize(self, event):
        # event.height 是 table_frame 的可用高度
        total_h = event.height
        # 预留一下表头高度（大约 25px 左右，视平台而定）
        header_h = 20
        # 8 行，向下取整
        row_h = max((total_h - header_h) // 8, 5)  # 最少 5px 避免太小
        self.style.configure("Custom.Treeview", rowheight=row_h)
        def on_shift_wheel(ev):
            multiplier = 30
            if hasattr(ev, 'delta') and ev.delta:
                # Windows/macOS
                units = int(-ev.delta/120 * multiplier)
                self.sensor_tree.xview_scroll(units, "units")
            else:
                # Linux 下用 Button-4/Button-5
                if ev.num == 4:
                    self.sensor_tree.xview_scroll(-multiplier, "units")
                elif ev.num == 5:
                    self.sensor_tree.xview_scroll(multiplier, "units")
            return "break"

        # 不同平台的 shift+轮 事件
        self.sensor_tree.bind("<Shift-MouseWheel>", on_shift_wheel)   # Win/mac
        self.sensor_tree.bind("<Shift-Button-4>",       on_shift_wheel)  # Linux up
        self.sensor_tree.bind("<Shift-Button-5>",       on_shift_wheel)  # Linux down
    def udp_listener(self):
        while self.sensor_running:
            data, _ = self.udp_sock.recvfrom(SENSOR_BUFFER_SIZE)
            if len(data) < 1+4+4+4+1 or data[0] != 0xA5:
                continue
            offset = 1
            rf, = struct.unpack_from('<f', data, offset); offset += 4
            ir, = struct.unpack_from('<f', data, offset); offset += 4
            mag_int, = struct.unpack_from('<i', data, offset); offset += 4
            name_bytes = data[offset:].split(b'\x00', 1)[0]
            try:
                name = name_bytes.decode('utf-8')
            except:
                name = repr(name_bytes)
            mag = bool(mag_int)
            self.root.after(0, self.update_sensor_row, name, rf, ir, mag)

    def update_sensor_row(self, name, rf, ir, mag):
        # overwrite first empty row matching spice blank
        for row_id in self.sensor_rows:
            vals = self.sensor_tree.item(row_id, 'values')
            if vals[1] == '':
                self.sensor_tree.item(row_id, values=(vals[0], '', name, f"{rf:.2f}", f"{ir:.2f}", str(mag)))
                return
        # if none empty, append to last
        row_id = self.sensor_tree.insert('', 'end', values=('', '', name, f"{rf:.2f}", f"{ir:.2f}", str(mag)))
        self.sensor_rows.append(row_id)

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
        f, a = 0.0, 0.0
        if 'w' in self.active_keys: f += self.VELOCITY_SCALE
        if 's' in self.active_keys: f -= self.VELOCITY_SCALE
        if 'd' in self.active_keys: a += self.VELOCITY_SCALE
        if 'a' in self.active_keys: a -= self.VELOCITY_SCALE
        f = np.clip(f, -1.0, 1.0)
        a = np.clip(a, -1.0, 1.0)
        self.velocities['forward'], self.velocities['angular'] = f, a
        self.lbl_fwd.config(text=f"{f:.2f}")
        self.lbl_ang.config(text=f"{a:.2f}")
        self.root.after(20, self.update_vel_labels)

    def create_raw_packet(self):
        f = self.velocities['forward']
        a = self.velocities['angular']
        left_val = f - a
        right_val = f + a
        dir_left = 1 if left_val >= 0 else 0
        dir_right = 0 if right_val >= 0 else 1
        pwm_left = int(np.clip(abs(left_val) * 255, 0, 255))
        pwm_right = int(np.clip(abs(right_val) * 255, 0, 255))
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
        self.sensor_running = False
        if self.sock:
            self.sock.close()

if __name__ == "__main__":
    root = tk.Tk()
    app = RobotControlGUI(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (app.cleanup(), root.destroy()))
    root.mainloop()
