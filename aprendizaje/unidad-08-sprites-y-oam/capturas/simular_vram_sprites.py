# Simula la VRAM de sprites de la mini-ROM u08 con TILE_LOBO=192 y pinta lo que leería la PPU.
import sys
from PIL import Image
R = r'E:\ProyectoSNES\aprendizaje\unidad-08-sprites-y-oam\rom\res'
bar = open(R + r'\barbara.pic', 'rb').read()
lobo = open(R + r'\lobo.pic', 'rb').read()
def pal(f):
    d = open(R + '\\' + f, 'rb').read()
    out = []
    for i in range(16):
        v = d[2*i] | d[2*i+1] << 8
        out.append(((v & 31) * 8, ((v >> 5) & 31) * 8, ((v >> 10) & 31) * 8))
    return out
pb, pl = pal('barbara.pal'), pal('lobo.pal')
TL = int(sys.argv[1]) if len(sys.argv) > 1 else 192
vram = bytearray(512 * 32)
vram[0:8192] = bar
vram[TL*32:TL*32+4096] = lobo[:4096]
def tile(t):
    b = vram[t*32:t*32+32]; px = []
    for y in range(8):
        row = []
        for x in range(8):
            bit = 7 - x
            v = ((b[2*y] >> bit) & 1) | (((b[2*y+1] >> bit) & 1) << 1) | (((b[16+2*y] >> bit) & 1) << 2) | (((b[17+2*y] >> bit) & 1) << 3)
            row.append(v)
        px.append(row)
    return px
def sprite(N, p, flip):
    im = Image.new('RGB', (64, 64), (48, 80, 112))
    for r in range(8):
        for c in range(8):
            t = (N & 0x100) | ((((N >> 4) + r) & 15) << 4) | (((N & 15) + c) & 15)
            px = tile(t)
            for y in range(8):
                for x in range(8):
                    v = px[y][x]
                    if v:
                        X = c*8+x
                        if flip: X = 63 - X
                        im.putpixel((X, r*8+y), p[v])
    return im
out = Image.new('RGB', (6*68, 64), (0, 0, 0))
cases = [(TL + 0, pl, 1), (TL + 8, pl, 1), (0, pb, 0), (8, pb, 0), (128, pb, 0), (136, pb, 0)]
for k, (N, p, f) in enumerate(cases):
    out.paste(sprite(N, p, f), (k*68, 0))
out = out.resize((out.width*3, out.height*3), Image.NEAREST)
out.save(sys.argv[2])
