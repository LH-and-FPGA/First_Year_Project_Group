import tkinter as tk
from tkinter import ttk
import numpy as np
import time
import threading
import socket
import msgpack


class RobotControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Robot Control")

        self.arduino_ip = "192.168.1.100"  # Replace with your Arduino's IP
        self.arduino_port = 1234
        self.sock = None

        self.active_keys = set()
        self.velocities = {
            'forward': 0.0,
            'left': 0.0,
            'angular': 0.0
        }
        self.VELOCITY_SCALE = 0.1

        self.setup_gui()
        self.setup_key_bindings()

        self.running = True
        threading.Thread(target=self.send_packets, daemon=True).start()
        self.update_velocities()

    def setup_gui(self):
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        # IP entry and connect button
        conn_frame = ttk.LabelFrame(main_frame, text="Connection", padding="5")
        conn_frame.grid(row=0, column=0, sticky=tk.W)

        ttk.Label(conn_frame, text="Arduino IP:").grid(row=0, column=0)
        self.ip_entry = ttk.Entry(conn_frame, width=15)
        self.ip_entry.insert(0, self.arduino_ip)
        self.ip_entry.grid(row=0, column=1)

        ttk.Button(conn_frame, text="Connect", command=self.connect).grid(row=0, column=2)

        self.status_label = ttk.Label(conn_frame, text="Disconnected", foreground="red")
        self.status_label.grid(row=1, column=0, columnspan=3, sticky=tk.W)

        # Velocity display
        vel_frame = ttk.LabelFrame(main_frame, text="Velocities", padding="5")
        vel_frame.grid(row=1, column=0, sticky=tk.W)

        self.vel_labels = {}
        for i, name in enumerate(self.velocities.keys()):
            ttk.Label(vel_frame, text=name.capitalize()).grid(row=0, column=2 * i)
            self.vel_labels[name] = ttk.Label(vel_frame, text="0.00")
            self.vel_labels[name].grid(row=0, column=2 * i + 1)

        # MsgPack output
        debug_frame = ttk.LabelFrame(main_frame, text="Sent MsgPack Packet", padding="5")
        debug_frame.grid(row=2, column=0, sticky=tk.W)

        self.msg_display = tk.Text(debug_frame, height=6, width=50, state='disabled')
        self.msg_display.grid(row=0, column=0, padx=5, pady=5)

        ttk.Label(main_frame, text="Use W/A/S/D/Q/E to control.").grid(row=3, column=0, sticky=tk.W)

    def connect(self):
        self.arduino_ip = self.ip_entry.get()
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(3)
            self.sock.connect((self.arduino_ip, self.arduino_port))
            self.status_label.config(text="Connected", foreground="green")
        except Exception as e:
            self.status_label.config(text=f"Connect failed: {e}", foreground="red")
            self.sock = None

    def setup_key_bindings(self):
        self.root.bind('<KeyPress>', self.on_key_press)
        self.root.bind('<KeyRelease>', self.on_key_release)

    def on_key_press(self, event):
        key = event.keysym.lower()
        if key in ['w', 'a', 's', 'd', 'q', 'e']:
            self.active_keys.add(key)

    def on_key_release(self, event):
        key = event.keysym.lower()
        if key in ['w', 'a', 's', 'd', 'q', 'e']:
            self.active_keys.discard(key)

    def update_velocities(self):
        self.velocities = {'forward': 0.0, 'left': 0.0, 'angular': 0.0}
        if 'w' in self.active_keys: self.velocities['forward'] += self.VELOCITY_SCALE
        if 's' in self.active_keys: self.velocities['forward'] -= self.VELOCITY_SCALE
        if 'a' in self.active_keys: self.velocities['left'] += self.VELOCITY_SCALE
        if 'd' in self.active_keys: self.velocities['left'] -= self.VELOCITY_SCALE
        if 'q' in self.active_keys: self.velocities['angular'] += self.VELOCITY_SCALE * 5
        if 'e' in self.active_keys: self.velocities['angular'] -= self.VELOCITY_SCALE * 5

        for name in self.velocities:
            self.vel_labels[name].config(text=f"{self.velocities[name]:.2f}")

        self.root.after(20, self.update_velocities)

    def create_msgpack_packet(self):
        fwd = np.clip(self.velocities['forward'], -1.0, 1.0)
        ang = np.clip(self.velocities['angular'], -1.0, 1.0)

        # Convert velocities to PWM
        pwm_left = int((fwd - ang) * 127 + 128)
        pwm_right = int((fwd + ang) * 127 + 128)
        pwm_left = max(0, min(255, pwm_left))
        pwm_right = max(0, min(255, pwm_right))

        dir_left = 1 if fwd >= 0 else 0
        dir_right = 1 if fwd >= 0 else 0

        # Build MsgPack packet as [int, int, int, int]
        payload = [pwm_left, pwm_right, dir_left, dir_right]

        # Use `msgpack.packb()` to write as array (same as `packer.to_array(...)`)
        packed = msgpack.packb(payload)

        # Return both for debug
        return packed, payload


    def send_packets(self):
        while self.running:
            if self.sock:
                try:
                    packed, debug_packet = self.create_msgpack_packet()
                    self.sock.sendall(packed)
                    self.display_msg(debug_packet, packed)
                except Exception as e:
                    print(f"Send failed: {e}")
                    self.sock = None
            time.sleep(0.05)

    def display_msg(self, data_arr, raw_bytes):
        self.msg_display.config(state='normal')
        self.msg_display.delete("1.0", tk.END)
        self.msg_display.insert(tk.END, f"Decoded: {data_arr}\n")
        self.msg_display.insert(tk.END, f"Raw Bytes: {list(raw_bytes)}\n")
        self.msg_display.config(state='disabled')

    def cleanup(self):
        self.running = False
        if self.sock:
            try:
                self.sock.close()
            except:
                pass


def main():
    root = tk.Tk()
    app = RobotControlGUI(root)

    def on_closing():
        app.cleanup()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()


if __name__ == "__main__":
    main()
