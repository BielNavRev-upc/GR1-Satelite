#!/usr/bin/env python3
import time
import serial
import numpy as np
import matplotlib.pyplot as plt

# ---------- CONFIG ----------
PORT      = '/dev/ttyACM0'          # <-- change to your port
BAUDRATE  = 9600
TIMEOUT   = 0.1

# ---------- OPEN PORT ----------
try:
    ser = serial.Serial(PORT, BAUDRATE, timeout=TIMEOUT)
    print(f"Opened {ser.port} @ {ser.baudrate} baud")
except serial.SerialException as e:
    print(f"Error opening {PORT}: {e}")
    exit(1)

ser.flushInput()

# ---------- SET UP PLOT ----------
plt.ion()
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})
dot, = ax.plot([], [], 'ro', ms=12)
ax.set_ylim(0, 1)
ax.set_yticks([])
ax.set_xticks([])
ax.set_title("Live Angle from Arduino", va='bottom')

# ---------- MAIN LOOP ----------
try:
    while True:
        raw = ser.readline()
        if not raw:
            continue

        try:
            angle_deg = float(raw.decode('utf-8').strip())
        except ValueError:
            print(f"Could not parse {raw!r}")
            continue

        theta = np.deg2rad(angle_deg)   # 0–π
        dot.set_data([theta], [1])
        fig.canvas.draw()
        fig.canvas.flush_events()
        time.sleep(0.01)

except KeyboardInterrupt:
    print("\nInterrupted – closing.")
finally:
    ser.close()
    plt.ioff()
    plt.show()