"""Writes the tiling device textures Assets/ROOT_AER/Data/Textures/AER_<Name>_BCR.png (1024x1024, seamless).
RGB is a near-white detail layer (the material Color tints it), alpha is roughness.
Usage: python tools/gen_textures.py"""
import pathlib

import numpy as np
from PIL import Image

N = 1024
OUT = pathlib.Path(__file__).resolve().parent.parent / "Assets/ROOT_AER/Data/Textures"
rng = np.random.default_rng(3)


def noise(cells):
    """Seamless value noise in 0..1 with `cells` lattice cells per side (smoothstep interpolation)."""
    g = rng.random((cells, cells))
    t = np.arange(N) * cells / N
    i0 = np.floor(t).astype(int)
    f = t - i0
    f = f * f * (3 - 2 * f)
    i1 = (i0 + 1) % cells
    a = g[i0][:, i0]
    b = g[i0][:, i1]
    c = g[i1][:, i0]
    d = g[i1][:, i1]
    fx = f[None, :]
    fy = f[:, None]
    return (a * (1 - fx) + b * fx) * (1 - fy) + (c * (1 - fx) + d * fx) * fy


def fbm(base, octaves):
    v = np.zeros((N, N))
    amp, tot = 1.0, 0.0
    for o in range(octaves):
        v += noise(base * 2 ** o) * amp
        tot += amp
        amp *= 0.5
    return v / tot


def scratches(count, length, val):
    """Thin straight scratches, wrapped at the edges."""
    m = np.zeros((N, N))
    for _ in range(count):
        x, y = rng.random(2) * N
        ang = rng.random() * np.pi
        ln = length * (0.4 + rng.random())
        for s in np.linspace(0, ln, int(ln * 2)):
            m[int(y + np.sin(ang) * s) % N, int(x + np.cos(ang) * s) % N] = val
    return m


def save(name, rgb, rough):
    rgb = np.clip(rgb, 0, 1)
    if rgb.ndim == 2:
        rgb = np.repeat(rgb[:, :, None], 3, axis=2)
    a = np.clip(rough, 0, 1)[:, :, None]
    img = np.concatenate([rgb, a], axis=2)
    OUT.mkdir(parents=True, exist_ok=True)
    Image.fromarray((img * 255 + 0.5).astype(np.uint8), "RGBA").save(OUT / ("AER_%s_BCR.png" % name))
    print("wrote", name)


# Plastic: fine grain, soft smudges, light edge scratches
grain = fbm(256, 2)
smudge = fbm(6, 4)
scr = scratches(45, 50, 0.6)
save("Plastic", 0.86 + 0.08 * grain + 0.06 * (smudge - 0.5) + 0.12 * scr, 0.62 + 0.12 * (smudge - 0.5) - 0.2 * scr)

# Metal: brushed streaks along X, wear
streak = np.tile(rng.random((N, 1)), (1, N))
streak = 0.5 * streak + 0.5 * np.roll(streak, 1, 0)
streak = streak * (0.7 + 0.3 * noise(32))
wear = fbm(8, 4)
save("Metal", 0.82 + 0.1 * streak + 0.08 * (wear - 0.5), 0.32 + 0.1 * streak + 0.1 * (wear - 0.5))

# Rubber: dense fine grain, matte
save("Rubber", 0.9 + 0.1 * fbm(300, 2) + 0.04 * (fbm(10, 3) - 0.5), 0.88 + 0.06 * fbm(120, 2))

# Keys: smooth with fingertip polish
polish = fbm(16, 3)
save("Keys", 0.9 + 0.06 * fbm(200, 2), 0.55 - 0.15 * polish)

# Solar cells (full colour, material Color white): 4x4 dark blue cells per tile, white gaps, silver busbars
y, x = np.mgrid[0:N, 0:N]
cell = N // 4
gap = ((x % cell) < 6) | ((y % cell) < 6)
bus = ((x % (cell // 4)) < 3) & ~gap
speck = fbm(128, 2)
rgb = np.stack([0.03 + 0.03 * speck, 0.07 + 0.04 * speck, 0.2 + 0.08 * speck], axis=2)
rgb[gap] = (0.8, 0.8, 0.82)
rgb[bus] = (0.62, 0.62, 0.64)
save("SolarCells", rgb, np.where(gap | bus, 0.4, 0.18 + 0.05 * speck))
