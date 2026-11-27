"""Writes the desktop icons UI/Textures/ROOT_AER/Icons/<name>.png (128x128 RGBA, drawn at 512 and downscaled).
App and file icons are coloured; glyph icons (prefix g_) are white for tinting.
Usage: python tools/gen_icons.py"""
import math
import pathlib

from PIL import Image, ImageDraw, ImageFilter

S = 512
OUT_SIZE = 128
OUT = pathlib.Path(__file__).resolve().parent.parent / "UI/Textures/ROOT_AER/Icons"
ICONS = {}


def icon(name):
    def reg(fn):
        ICONS[name] = fn
        return fn
    return reg


def hexc(h, a=255):
    h = h.lstrip("#")
    return (int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16), a)


def blank():
    return Image.new("RGBA", (S, S), (0, 0, 0, 0))


def mask(fn):
    m = Image.new("L", (S, S), 0)
    fn(ImageDraw.Draw(m))
    return m


def paint(img, m, c1, c2=None, vertical=True):
    """Fills mask m with a colour or a linear gradient c1 -> c2."""
    c1 = hexc(c1) if isinstance(c1, str) else c1
    c2 = c1 if c2 is None else (hexc(c2) if isinstance(c2, str) else c2)
    strip = Image.new("RGBA", (1, S))
    strip.putdata([tuple(int(c1[k] + (c2[k] - c1[k]) * i / (S - 1)) for k in range(4)) for i in range(S)])
    g = strip.resize((S, S), Image.NEAREST)
    if not vertical:
        g = g.transpose(Image.TRANSPOSE)
    a = g.getchannel("A")
    a = Image.composite(a, Image.new("L", (S, S), 0), m)
    g.putalpha(a)
    img.alpha_composite(g)


def stroke(img, fn, color):
    layer = blank()
    fn(ImageDraw.Draw(layer), hexc(color) if isinstance(color, str) else color)
    img.alpha_composite(layer)


def shadow(img):
    a = img.getchannel("A").filter(ImageFilter.GaussianBlur(10))
    sh = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    sh.putalpha(a.point(lambda v: int(v * 0.35)))
    out = blank()
    out.alpha_composite(sh, (0, 10))
    out.alpha_composite(img)
    return out


def rr(box, r):
    return lambda d: d.rounded_rectangle(box, r, fill=255)


def ell(box):
    return lambda d: d.ellipse(box, fill=255)


def poly(pts):
    return lambda d: d.polygon(pts, fill=255)


def thick_line(d, pts, w, c):
    d.line(pts, fill=c, width=w, joint="curve")
    r = w // 2
    for x, y in (pts[0], pts[-1]):
        d.ellipse((x - r, y - r, x + r, y + r), fill=c)


# Shared shapes ----------------------------------------------------------------------------------

def page(img):
    """White document with folded top right corner."""
    m = mask(rr((104, 32, 408, 480), 26))
    d = ImageDraw.Draw(m)
    d.polygon([(318, 20), (420, 20), (420, 122)], fill=0)
    paint(img, m, "#FFFFFF", "#E8ECEF")
    edge = m.filter(ImageFilter.MaxFilter(9))
    ring = Image.composite(edge, Image.new("L", (S, S), 0), Image.eval(m, lambda v: 255 - v))
    paint(img, ring, "#9AA7B0")
    paint(img, mask(poly([(318, 32), (318, 122), (408, 122)])), "#C9D2D8")


def monitor(img, box, screen1, screen2):
    x0, y0, x1, y1 = box
    paint(img, mask(rr(box, 24)), "#2B3137", "#1C2024")
    paint(img, mask(rr((x0 + 20, y0 + 20, x1 - 20, y1 - 20), 8)), screen1, screen2)


def padlock(img, cx, cy, s):
    stroke(img, lambda d, c: d.arc((cx - s * 0.55, cy - s * 1.25, cx + s * 0.55, cy - s * 0.05), 180, 360, fill=c, width=int(s * 0.22)), "#6D4C00")
    stroke(img, lambda d, c: d.line((cx - s * 0.44, cy - s * 0.65, cx - s * 0.44, cy), fill=c, width=int(s * 0.22)), "#6D4C00")
    stroke(img, lambda d, c: d.line((cx + s * 0.44, cy - s * 0.65, cx + s * 0.44, cy), fill=c, width=int(s * 0.22)), "#6D4C00")
    paint(img, mask(rr((cx - s * 0.8, cy - s * 0.3, cx + s * 0.8, cy + s * 0.9), s * 0.18)), "#FFCA28", "#F9A825")
    paint(img, mask(ell((cx - s * 0.15, cy + s * 0.05, cx + s * 0.15, cy + s * 0.35))), "#6D4C00")


# Apps -------------------------------------------------------------------------------------------

@icon("folder")
def _folder(img):
    paint(img, mask(rr((40, 96, 236, 200), 26)), "#E0A100")
    paint(img, mask(rr((40, 140, 472, 420), 28)), "#F0B000", "#D99400")
    paint(img, mask(rr((40, 182, 472, 432), 28)), "#FFD966", "#FFBF1F")


@icon("computer")
def _computer(img):
    paint(img, mask(rr((216, 368, 296, 424), 6)), "#9EA9B1")
    paint(img, mask(rr((146, 412, 366, 446), 14)), "#C3CBD1", "#9EA9B1")
    monitor(img, (36, 64, 476, 380), "#5AB2F6", "#1565C0")
    paint(img, mask(poly([(56, 360), (456, 84), (456, 360)])), (255, 255, 255, 30))


@icon("notepad")
def _notepad(img):
    page(img)
    for y in (180, 232, 284, 336, 388):
        stroke(img, lambda d, c, y=y: thick_line(d, [(156, y), (356 if y != 388 else 280, y)], 18, c), "#5B8DB8")


@icon("photos")
def _photos(img):
    paint(img, mask(rr((40, 80, 472, 432), 30)), "#FFFFFF", "#E3E8EC")
    paint(img, mask(rr((64, 104, 448, 408), 14)), "#7FD0FF", "#3E9BE0")
    paint(img, mask(poly([(64, 408), (196, 236), (300, 408)])), "#3E8E41")
    paint(img, mask(poly([(196, 408), (330, 262), (448, 380), (448, 408)])), "#5DB761")
    paint(img, mask(ell((330, 136, 400, 206))), "#FFE082")


@icon("terminal")
def _terminal(img):
    paint(img, mask(rr((40, 64, 472, 448), 40)), "#2D2D30", "#151517")
    paint(img, mask(rr((40, 64, 472, 128), 40)), "#3C3C40")
    paint(img, mask(lambda d: d.rectangle((40, 100, 472, 128), fill=255)), "#3C3C40")
    for i, col in enumerate(("#FF5F57", "#FEBC2E", "#28C840")):
        paint(img, mask(ell((76 + i * 44, 82, 104 + i * 44, 110))), col)
    stroke(img, lambda d, c: thick_line(d, [(112, 196), (196, 264), (112, 332)], 34, c), "#4EE07A")
    stroke(img, lambda d, c: thick_line(d, [(232, 340), (352, 340)], 34, c), "#E8E8E8")


@icon("browser")
def _browser(img):
    paint(img, mask(ell((48, 48, 464, 464))), "#5CC8FF", "#0A6FC2")
    w = (255, 255, 255, 200)

    def grid(d, c):
        d.ellipse((168, 52, 344, 460), outline=c, width=16)
        d.line((256, 52, 256, 460), fill=c, width=16)
        d.line((52, 256, 460, 256), fill=c, width=16)
        for y in (150, 362):
            h = math.sqrt(208 ** 2 - (y - 256) ** 2)
            d.line((256 - h, y, 256 + h, y), fill=c, width=14)
    stroke(img, grid, w)


@icon("mail")
def _mail(img):
    paint(img, mask(rr((40, 104, 472, 408), 30)), "#4FA8F7", "#1767C8")
    stroke(img, lambda d, c: thick_line(d, [(68, 136), (256, 282), (444, 136)], 30, c), "#FFFFFF")
    stroke(img, lambda d, c: d.line((60, 392, 200, 262), fill=c, width=14), (255, 255, 255, 110))
    stroke(img, lambda d, c: d.line((452, 392, 312, 262), fill=c, width=14), (255, 255, 255, 110))


@icon("messenger")
def _messenger(img):
    paint(img, mask(ell((40, 60, 472, 396))), "#6DD36F", "#24963B")
    paint(img, mask(poly([(120, 330), (84, 460), (230, 372)])), "#2C9E42")
    for x in (162, 256, 350):
        paint(img, mask(ell((x - 30, 198, x + 30, 258))), "#FFFFFF")


@icon("calendar")
def _calendar(img):
    paint(img, mask(rr((56, 84, 456, 456), 34)), "#FFFFFF", "#E6EAED")
    paint(img, mask(rr((56, 84, 456, 196), 34)), "#F0524A", "#D32F2F")
    paint(img, mask(lambda d: d.rectangle((56, 160, 456, 196), fill=255)), "#D63A34")
    for x in (156, 356):
        paint(img, mask(rr((x - 16, 44, x + 16, 124), 14)), "#56636C")
    for r in range(3):
        for col in range(4):
            x = 104 + col * 84
            y = 226 + r * 72
            c = "#E53935" if (r, col) == (1, 2) else "#B3BEC6"
            paint(img, mask(rr((x, y, x + 52, y + 44), 10)), c)


@icon("network")
def _network(img):
    for r, a in ((236, 255), (166, 255), (96, 255)):
        stroke(img, lambda d, c, r=r: d.arc((256 - r, 410 - r, 256 + r, 410 + r), 225, 315, fill=c, width=46), "#1E88E5")
    paint(img, mask(ell((222, 376, 290, 444))), "#1E88E5")


@icon("remote")
def _remote(img):
    monitor(img, (36, 70, 330, 290), "#4DD0C4", "#00897B")
    monitor(img, (182, 200, 476, 420), "#5AB2F6", "#1565C0")
    stroke(img, lambda d, c: thick_line(d, [(96, 330), (96, 420), (150, 420)], 22, c), "#FF9800")
    stroke(img, lambda d, c: thick_line(d, [(416, 160), (416, 120)], 22, c), "#FF9800")


@icon("ping")
def _ping(img):
    paint(img, mask(ell((40, 40, 472, 472))), "#1F5E2A", "#0F3A18")
    for r in (180, 116, 52):
        stroke(img, lambda d, c, r=r: d.ellipse((256 - r, 256 - r, 256 + r, 256 + r), outline=c, width=10), (130, 230, 140, 200))
    paint(img, mask(lambda d: d.pieslice((40, 40, 472, 472), 280, 340, fill=255)), (120, 255, 140, 110))
    paint(img, mask(ell((340, 150, 376, 186))), "#B9FFC0")
    paint(img, mask(ell((236, 236, 276, 276))), "#B9FFC0")


@icon("map")
def _map(img):
    xs = [48, 184, 328, 464]
    tops = [104, 64, 104, 64]
    bots = [448, 408, 448, 408]
    cols = [("#8BD48E", "#5DB761"), ("#B5E4B6", "#86CC88"), ("#8BD48E", "#5DB761")]
    for i in range(3):
        paint(img, mask(poly([(xs[i], tops[i]), (xs[i + 1], tops[i + 1]), (xs[i + 1], bots[i + 1]), (xs[i], bots[i])])), cols[i][0], cols[i][1])
    stroke(img, lambda d, c: d.line((60, 300, 200, 250, 300, 330, 452, 270), fill=c, width=14, joint="curve"), "#3D7DCA")
    paint(img, mask(ell((262, 90, 390, 218))), "#F44336", "#C62828")
    paint(img, mask(poly([(272, 180), (380, 180), (326, 310)])), "#C62828")
    paint(img, mask(ell((300, 128, 352, 180))), "#FFFFFF")


@icon("cctv")
def _cctv(img):
    paint(img, mask(rr((380, 290, 460, 440), 12)), "#7C8A94")
    stroke(img, lambda d, c: thick_line(d, [(320, 250), (420, 360)], 30, c), "#9AA7B0")
    paint(img, mask(poly([(120, 150), (400, 96), (432, 214), (150, 284)])), "#F2F5F7", "#C3CDD4")
    paint(img, mask(ell((64, 140, 176, 300))), "#37474F")
    paint(img, mask(ell((92, 182, 148, 258))), "#4FC3F7", "#0277BD")
    paint(img, mask(ell((370, 116, 398, 144))), "#F44336")


@icon("games")
def _games(img):
    paint(img, mask(rr((40, 150, 472, 360), 100)), "#8C6BE0", "#5A35B8")
    paint(img, mask(ell((40, 250, 200, 440))), "#5A35B8")
    paint(img, mask(ell((312, 250, 472, 440))), "#5A35B8")
    stroke(img, lambda d, c: (d.line((110, 250, 200, 250), fill=c, width=30), d.line((155, 205, 155, 295), fill=c, width=30)), "#FFFFFF")
    for (x, y, col) in ((360, 210, "#FFD54F"), (404, 254, "#EF5350"), (316, 254, "#66BB6A"), (360, 298, "#42A5F5")):
        paint(img, mask(ell((x - 22, y - 22, x + 22, y + 22))), col)


@icon("snake")
def _snake(img):
    paint(img, mask(rr((40, 40, 472, 472), 48)), "#1E2B22", "#101812")
    pts = [(120, 380), (300, 380), (300, 260), (160, 260), (160, 150), (330, 150)]
    stroke(img, lambda d, c: thick_line(d, pts, 56, c), "#8BCF4A")
    paint(img, mask(ell((312, 112, 392, 192))), "#A5E06A")
    paint(img, mask(ell((352, 134, 370, 152))), "#102010")
    paint(img, mask(ell((382, 318, 438, 374))), "#F44336")


@icon("calculator")
def _calculator(img):
    paint(img, mask(rr((96, 36, 416, 476), 40)), "#56646E", "#37434B")
    paint(img, mask(rr((128, 70, 384, 160), 14)), "#C9F0E8", "#9ED9CC")
    for r in range(4):
        for c in range(4):
            x = 128 + c * 66
            y = 186 + r * 70
            col = "#FF9800" if c == 3 else "#EEF2F4"
            paint(img, mask(rr((x, y, x + 52, y + 54), 12)), col)


def _bin(img, full):
    if full:
        paint(img, mask(poly([(170, 160), (250, 70), (300, 150)])), "#FFFFFF", "#DCE3E8")
        paint(img, mask(poly([(250, 160), (330, 92), (360, 168)])), "#FFF3C4", "#F2DE8A")
        paint(img, mask(rr((150, 120, 250, 170), 20)), "#E3F2FD")
    else:
        paint(img, mask(rr((104, 96, 408, 148), 18)), "#AEBBC4", "#8E9DA8")
        stroke(img, lambda d, c: d.rounded_rectangle((206, 60, 306, 108), 18, outline=c, width=18), "#8E9DA8")
    paint(img, mask(poly([(128, 160), (384, 160), (354, 462), (158, 462)])), (205, 216, 224, 235), (150, 166, 178, 235))
    for x in (200, 256, 312):
        stroke(img, lambda d, c, x=x: thick_line(d, [(x, 204), (x + (x - 256) * 0.1, 420)], 16, c), (90, 105, 118, 200))


@icon("trash")
def _trash(img):
    _bin(img, False)


@icon("trash_full")
def _trash_full(img):
    _bin(img, True)


@icon("settings")
def _settings(img):
    def gear(d):
        d.ellipse((120, 120, 392, 392), fill=255)
        for k in range(8):
            a = k * math.pi / 4
            ux, uy = math.cos(a), math.sin(a)
            tx, ty = -uy, ux
            cx, cy = 256 + ux * 165, 256 + uy * 165
            d.polygon([(cx + ux * r + tx * t, cy + uy * r + ty * t) for r, t in ((-50, -40), (50, -36), (50, 36), (-50, 40))], fill=255)
        d.ellipse((196, 196, 316, 316), fill=0)
    paint(img, mask(gear), "#A3B1BB", "#5D6D78")


@icon("info")
def _info(img):
    paint(img, mask(ell((48, 48, 464, 464))), "#3FB6F2", "#0A6FC2")
    paint(img, mask(ell((226, 118, 286, 178))), "#FFFFFF")
    paint(img, mask(rr((226, 214, 286, 394), 24)), "#FFFFFF")


@icon("logo")
def _logo(img):
    """Radix emblem: a stem splitting into five roots on a green tile."""
    paint(img, mask(rr((32, 32, 480, 480), 104)), "#3FA672", "#185C3E")
    w = "#FFFFFF"
    stroke(img, lambda d, c: thick_line(d, [(256, 92), (256, 236)], 40, c), w)
    roots = ([(256, 236), (196, 290), (132, 330), (112, 404)],
             [(256, 236), (220, 320), (196, 420)],
             [(256, 236), (256, 432)],
             [(256, 236), (292, 320), (316, 420)],
             [(256, 236), (316, 290), (380, 330), (400, 404)])
    for r in roots:
        stroke(img, lambda d, c, r=r: thick_line(d, r, 22, c), w)


def hexagon(cx, cy, r, rot=90):
    return [(cx + r * math.cos(math.radians(rot + 60 * k)), cy + r * math.sin(math.radians(rot + 60 * k))) for k in range(6)]


@icon("start")
def _start(img):
    """Start button, civilian style: armaOS node mark (hexagon, hub and three spokes) on a blue tile."""
    paint(img, mask(rr((32, 32, 480, 480), 104)), "#4FA3FF", "#1B5FD1")
    hexa = hexagon(256, 256, 168)
    stroke(img, lambda d, c: d.line(hexa + [hexa[0]], fill=c, width=34, joint="curve"), "#FFFFFF")
    for k in (0, 2, 4):
        x, y = hexa[k]
        stroke(img, lambda d, c, x=x, y=y: thick_line(d, [(256, 256), (x, y)], 26, c), "#FFFFFF")
        stroke(img, lambda d, c, x=x, y=y: d.ellipse((x - 30, y - 30, x + 30, y + 30), fill=c), "#FFFFFF")
    stroke(img, lambda d, c: d.ellipse((206, 206, 306, 306), fill=c), "#FFFFFF")
    paint(img, mask(ell((236, 236, 276, 276))), "#1B5FD1")


@icon("user")
def _user(img):
    circle = mask(ell((24, 24, 488, 488)))
    paint(img, circle, "#B7C4CE", "#7F909C")
    body = mask(ell((106, 300, 406, 600)))
    body = Image.composite(body, Image.new("L", (S, S), 0), circle)
    paint(img, body, "#F4F7F9", "#DCE3E8")
    paint(img, mask(ell((176, 110, 336, 270))), "#F4F7F9", "#E6EBEF")


@icon("drive")
def _drive(img):
    paint(img, mask(rr((176, 36, 336, 160), 10)), "#D9DEE2", "#A9B3BA")
    for x in (214, 276):
        paint(img, mask(lambda d, x=x: d.rectangle((x, 70, x + 24, 100), fill=255)), "#56636C")
    paint(img, mask(rr((136, 140, 376, 476), 34)), "#3D8BFD", "#1450B8")
    paint(img, mask(rr((206, 360, 306, 420), 12)), (255, 255, 255, 120))


@icon("games_board")
def _board(img):
    paint(img, mask(rr((48, 48, 464, 464), 30)), "#8D6E63")
    for r in range(4):
        for c in range(4):
            if (r + c) % 2 == 0:
                paint(img, mask(lambda d, r=r, c=c: d.rectangle((72 + c * 92, 72 + r * 92, 164 + c * 92, 164 + r * 92), fill=255)), "#F5E6CF")
            else:
                paint(img, mask(lambda d, r=r, c=c: d.rectangle((72 + c * 92, 72 + r * 92, 164 + c * 92, 164 + r * 92), fill=255)), "#5D4037")


# File types -------------------------------------------------------------------------------------

@icon("file")
def _file(img):
    page(img)


@icon("file_text")
def _file_text(img):
    page(img)
    for y in (190, 240, 290, 340):
        stroke(img, lambda d, c, y=y: thick_line(d, [(160, y), (352, y)], 16, c), "#8796A1")


@icon("file_image")
def _file_image(img):
    page(img)
    paint(img, mask(rr((150, 190, 362, 390), 10)), "#7FD0FF", "#3E9BE0")
    paint(img, mask(poly([(150, 390), (230, 280), (300, 390)])), "#3E8E41")
    paint(img, mask(poly([(230, 390), (310, 300), (362, 360), (362, 390)])), "#5DB761")
    paint(img, mask(ell((300, 214, 340, 254))), "#FFE082")


@icon("file_program")
def _file_program(img):
    page(img)
    paint(img, mask(rr((150, 196, 362, 392), 22)), "#3D8BFD", "#1450B8")
    stroke(img, lambda d, c: thick_line(d, [(196, 250), (246, 294), (196, 338)], 22, c), "#FFFFFF")
    stroke(img, lambda d, c: thick_line(d, [(266, 340), (318, 340)], 22, c), "#FFFFFF")


@icon("file_locked")
def _file_locked(img):
    page(img)
    for y in (190, 240):
        stroke(img, lambda d, c, y=y: thick_line(d, [(160, y), (352, y)], 16, c), "#B3BEC6")
    padlock(img, 300, 360, 100)


@icon("file_link")
def _file_link(img):
    page(img)
    paint(img, mask(rr((104, 330, 254, 480), 18)), "#FFFFFF")
    stroke(img, lambda d, c: d.rounded_rectangle((104, 330, 254, 480), 18, outline=c, width=8), "#9AA7B0")
    stroke(img, lambda d, c: d.arc((130, 372, 250, 492), 200, 290, fill=c, width=20), "#1E88E5")
    paint(img, mask(poly([(196, 354), (240, 378), (200, 410)])), "#1E88E5")


# Glyphs (white) ---------------------------------------------------------------------------------

W = "#FFFFFF"
GW = 40


def glyph(name, fn):
    ICONS["g_" + name] = lambda img: stroke(img, fn, W)


glyph("back", lambda d, c: (thick_line(d, [(400, 256), (120, 256)], GW, c), thick_line(d, [(240, 136), (120, 256), (240, 376)], GW, c)))
glyph("forward", lambda d, c: (thick_line(d, [(112, 256), (392, 256)], GW, c), thick_line(d, [(272, 136), (392, 256), (272, 376)], GW, c)))
glyph("up", lambda d, c: (thick_line(d, [(256, 400), (256, 120)], GW, c), thick_line(d, [(136, 240), (256, 120), (376, 240)], GW, c)))
glyph("chevron", lambda d, c: thick_line(d, [(200, 136), (320, 256), (200, 376)], GW, c))
glyph("chevron_down", lambda d, c: thick_line(d, [(136, 200), (256, 320), (376, 200)], GW, c))
glyph("close", lambda d, c: (thick_line(d, [(140, 140), (372, 372)], GW, c), thick_line(d, [(372, 140), (140, 372)], GW, c)))
glyph("min", lambda d, c: thick_line(d, [(130, 256), (382, 256)], 32, c))
glyph("max", lambda d, c: d.rectangle((136, 136, 376, 376), outline=c, width=32))
glyph("restore", lambda d, c: (d.rectangle((120, 180, 332, 392), outline=c, width=30), d.line((180, 150, 362, 150, 362, 332), fill=c, width=30)))
glyph("plus", lambda d, c: (thick_line(d, [(256, 112), (256, 400)], GW, c), thick_line(d, [(112, 256), (400, 256)], GW, c)))
glyph("search", lambda d, c: (d.ellipse((96, 96, 336, 336), outline=c, width=GW), thick_line(d, [(312, 312), (412, 412)], 48, c)))
glyph("refresh", lambda d, c: (d.arc((104, 104, 408, 408), 40, 330, fill=c, width=GW), d.polygon([(330, 70), (430, 200), (300, 210)], fill=c)))
glyph("home", lambda d, c: (thick_line(d, [(84, 260), (256, 104), (428, 260)], GW, c), d.line((140, 230, 140, 410, 372, 410, 372, 230), fill=c, width=GW, joint="curve"), d.rectangle((224, 300, 288, 410), fill=c)))
glyph("cut", lambda d, c: (d.ellipse((96, 300, 216, 420), outline=c, width=34), d.ellipse((296, 300, 416, 420), outline=c, width=34), thick_line(d, [(186, 320), (360, 92)], 34, c), thick_line(d, [(326, 320), (152, 92)], 34, c)))
glyph("copy", lambda d, c: (d.rounded_rectangle((176, 72, 424, 360), 30, outline=c, width=34), d.line((120, 152, 88, 152, 88, 440, 336, 440, 336, 410), fill=c, width=34, joint="curve")))
glyph("paste", lambda d, c: (d.rounded_rectangle((104, 112, 408, 452), 30, outline=c, width=34), d.rounded_rectangle((186, 70, 326, 150), 20, fill=c)))
glyph("rename", lambda d, c: (d.polygon([(330, 80), (432, 182), (204, 410), (102, 308)], outline=c, width=34), d.polygon([(102, 308), (204, 410), (72, 440)], fill=c)))
glyph("delete", lambda d, c: (thick_line(d, [(96, 140), (416, 140)], 34, c), d.line((140, 140, 168, 440, 344, 440, 372, 140), fill=c, width=34, joint="curve"), d.rounded_rectangle((200, 70, 312, 140), 16, outline=c, width=30), thick_line(d, [(222, 210), (226, 380)], 26, c), thick_line(d, [(290, 210), (286, 380)], 26, c)))
glyph("props", lambda d, c: (d.ellipse((72, 72, 440, 440), outline=c, width=34), d.ellipse((232, 140, 280, 188), fill=c), thick_line(d, [(256, 236), (256, 370)], 40, c)))
glyph("grid", lambda d, c: [d.rounded_rectangle((x, y, x + 140, y + 140), 20, fill=c) for x in (96, 276) for y in (96, 276)])
glyph("list", lambda d, c: [(d.ellipse((90, y - 22, 134, y + 22), fill=c), thick_line(d, [(190, y), (420, y)], 36, c)) for y in (136, 256, 376)])
glyph("power", lambda d, c: (d.arc((96, 112, 416, 432), 300, 240, fill=c, width=GW), thick_line(d, [(256, 72), (256, 260)], GW, c)))
glyph("wifi", lambda d, c: ([d.arc((256 - r, 410 - r, 256 + r, 410 + r), 225, 315, fill=c, width=40) for r in (236, 166, 96)], d.ellipse((226, 380, 286, 440), fill=c)))
glyph("battery", lambda d, c: (d.rounded_rectangle((60, 150, 420, 362), 30, outline=c, width=30), d.rounded_rectangle((420, 214, 460, 298), 10, fill=c), d.rectangle((100, 190, 380, 322), fill=c)))
glyph("plug", lambda d, c: (thick_line(d, [(200, 80), (200, 180)], 36, c), thick_line(d, [(312, 80), (312, 180)], 36, c), d.rounded_rectangle((140, 170, 372, 330), 40, fill=c), thick_line(d, [(256, 330), (256, 440)], 40, c)))
glyph("check", lambda d, c: thick_line(d, [(110, 270), (210, 370), (410, 150)], 48, c))
glyph("open", lambda d, c: (d.rounded_rectangle((80, 120, 392, 432), 30, outline=c, width=34), thick_line(d, [(260, 252), (430, 82)], 34, c), thick_line(d, [(300, 82), (430, 82), (430, 212)], 34, c)))
glyph("link", lambda d, c: (d.rounded_rectangle((64, 192, 292, 320), 64, outline=c, width=34), d.rounded_rectangle((220, 192, 448, 320), 64, outline=c, width=34)))
glyph("newfolder", lambda d, c: (d.line((72, 132, 200, 132, 236, 176, 440, 176, 440, 420, 72, 420, 72, 132), fill=c, width=32, joint="curve"), thick_line(d, [(256, 236), (256, 360)], 30, c), thick_line(d, [(194, 298), (318, 298)], 30, c)))
glyph("newfile", lambda d, c: (d.line((120, 72, 300, 72, 392, 164, 392, 440, 120, 440, 120, 72), fill=c, width=32, joint="curve"), thick_line(d, [(256, 220), (256, 360)], 30, c), thick_line(d, [(186, 290), (326, 290)], 30, c)))
glyph("dot", lambda d, c: d.ellipse((176, 176, 336, 336), fill=c))
# Start button, military style: hexagon frame with a reticle.
glyph("start_tac", lambda d, c: (d.line(hexagon(256, 256, 216) + [hexagon(256, 256, 216)[0]], fill=c, width=30, joint="curve"), d.ellipse((136, 136, 376, 376), outline=c, width=26), [thick_line(d, p, 26, c) for p in ([(256, 64), (256, 176)], [(256, 336), (256, 448)], [(64, 256), (176, 256)], [(336, 256), (448, 256)])], d.ellipse((232, 232, 280, 280), fill=c)))


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for name, fn in ICONS.items():
        img = blank()
        fn(img)
        if not name.startswith("g_"):
            img = shadow(img)
        img.resize((OUT_SIZE, OUT_SIZE), Image.LANCZOS).save(OUT / (name + ".png"))
    print(len(ICONS), "icons")


if __name__ == "__main__":
    main()
