# Synthesizes the Advanced Equipment Reforged UI sounds (16-bit mono 44.1 kHz WAV) into Sounds/ROOT_AER/.
# Run: python make_sounds.py
import math
import os
import struct
import wave

import numpy as np

RATE = 44100
PEAK = 0.08
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")


def tone(freq, dur, vol=0.5, attack=0.01, release=0.25, harmonics=(1.0, 0.35, 0.12)):
    t = np.arange(int(RATE * dur)) / RATE
    s = sum(a * np.sin(2 * math.pi * freq * (i + 1) * t) for i, a in enumerate(harmonics))
    env = np.minimum(1, t / attack) * np.exp(-t / max(release, 1e-3))
    return vol * s * env / sum(harmonics)


def silence(dur):
    return np.zeros(int(RATE * dur))


def mix(parts):
    """parts: (start seconds, samples)."""
    end = max(int(RATE * s) + len(x) for s, x in parts)
    out = np.zeros(end)
    for s, x in parts:
        i = int(RATE * s)
        out[i:i + len(x)] += x
    return out


def save(name, data):
    data = np.clip(data / max(1e-6, np.max(np.abs(data))) * PEAK, -1, 1)
    fade = min(len(data), int(RATE * 0.02))
    data[-fade:] *= np.linspace(1, 0, fade)
    path = os.path.join(OUT, name + ".wav")
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(v * 32767)) for v in data))
    print(path)


save("AER_Startup", mix([(0.0, tone(523.25, 1.2, release=0.5)), (0.18, tone(659.25, 1.1, release=0.5)),
                         (0.36, tone(783.99, 1.0, release=0.55)), (0.54, tone(1046.5, 1.4, release=0.7))]))
save("AER_Shutdown", mix([(0.0, tone(1046.5, 0.9, release=0.35)), (0.16, tone(783.99, 0.9, release=0.35)),
                          (0.32, tone(523.25, 1.2, release=0.5))]))
save("AER_Standby", mix([(0.0, tone(659.25, 0.6, release=0.2)), (0.14, tone(493.88, 0.8, release=0.3))]))
save("AER_Mail", mix([(0.0, tone(880.0, 0.6, release=0.25)), (0.13, tone(1318.5, 0.9, release=0.35))]))
save("AER_Message", mix([(0.0, tone(1174.7, 0.35, release=0.1, harmonics=(1.0, 0.2))),
                         (0.09, tone(1567.98, 0.45, release=0.14, harmonics=(1.0, 0.2)))]))
save("AER_Error", mix([(0.0, tone(220.0, 0.25, release=0.2, harmonics=(1.0, 0.6, 0.4, 0.3))),
                       (0.14, tone(196.0, 0.3, release=0.25, harmonics=(1.0, 0.6, 0.4, 0.3)))]))
save("AER_UsbIn", mix([(0.0, tone(587.33, 0.25, release=0.1)), (0.08, tone(880.0, 0.35, release=0.15))]))
save("AER_UsbOut", mix([(0.0, tone(880.0, 0.25, release=0.1)), (0.08, tone(587.33, 0.35, release=0.15))]))
save("AER_Notify", tone(987.77, 0.5, release=0.18))
save("AER_Click", tone(2000.0, 0.04, attack=0.001, release=0.01, harmonics=(1.0,)))
