# comprobar_hud_numero.py — Comprueba hud_numero.asm en el PC, sin SNES.
#
# Un mini-simulador del 65816 que solo conoce las instrucciones que usa la rutina
# y solo en modo de 16 bits (rep #$30): lee el .asm, lo ejecuta para los 65536
# valores de s16 y compara lo escrito con la versión en C de numero()
# (src/juego/hud_debug.c, traducida a Python en numero_c_exacto). NO prueba la
# codificación de las instrucciones ni el enlazado: eso lo prueba la ROM (la fila
# "ASM 14/14" del HUD). Sí prueba el algoritmo, los saltos y las banderas.
# Uso:  python comprobar_hud_numero.py hud_numero.asm
import re, sys
src = open(sys.argv[1], encoding='utf-8').read().splitlines()
defs = {}
prog = []; labels = {}; data = {}
for line in src:
    line = line.split(';')[0].strip()
    if not line: continue
    m = re.match(r'\.define\s+(\w+)\s+\$([0-9A-Fa-f]+)', line)
    if m: defs[m.group(1)] = int(m.group(2), 16); continue
    if line.startswith('.'):
        m = re.match(r'\.dw\s+(.*)', line)
        if m: data[last] = [int(x) for x in m.group(1).split(',')]
        continue
    if line.endswith(':'):
        last = line[:-1]; labels[last] = len(prog); continue
    prog.append(line)
def val(tok):
    tok = tok.strip()
    if tok.startswith('#'): tok = tok[1:]
    if tok in defs: return defs[tok]
    if tok.startswith('$'): return int(tok[1:], 16)
    return int(tok)
M = 0xFFFF
def run(p_lo, valor):
    mem = {}
    stack = {5: p_lo, 7: 0x7E, 9: valor & M}   # tras php
    dp = {'tcc__r0': 0, 'tcc__r0h': 0, 'tcc__r1': 0, 'tcc__r2': 0}; A = X = Y = 0; C = Z = N = 0; pc = labels['hud_numero']; spush = []
    def flags(v): 
        nonlocal Z, N
        Z = int((v & M) == 0); N = int((v & 0x8000) != 0)
    steps = 0
    while True:
        steps += 1
        if steps > 10000: raise Exception('demasiadas instrucciones (¿bucle sin fin?)')
        ins = prog[pc]; pc += 1
        op, _, arg = ins.partition(' '); op = op.split('.')[0]; arg = arg.strip()
        if op in ('php', 'rep', 'sec', 'clc') :
            if op == 'sec': C = 1
            if op == 'clc': C = 0
        elif op == 'plp': return mem, dp['tcc__r0'], steps
        elif op == 'rtl': raise Exception('rtl sin plp')
        elif op == 'lda':
            if arg.endswith(',s'): A = stack[int(arg[:-2])]
            elif arg.startswith('#'): A = val(arg)
            else: A = dp[arg]
            flags(A)
        elif op == 'sta':
            if arg.startswith('['): mem[(dp['tcc__r0'] + Y) & M] = A
            else: dp[arg] = A
        elif op == 'stz': dp[arg] = 0
        elif op == 'ldy': Y = val(arg); flags(Y)
        elif op == 'ldx': X = val(arg); flags(X)
        elif op == 'iny': Y = (Y + 1) & M; flags(Y)
        elif op == 'inx': X = (X + 1) & M; flags(X)
        elif op == 'inc':
            if arg == 'a': A = (A + 1) & M; flags(A)
            else: dp[arg] = (dp[arg] + 1) & M; flags(dp[arg])
        elif op == 'eor': A ^= val(arg); flags(A)
        elif op == 'ora': A |= dp[arg]; flags(A)
        elif op == 'pha': spush.append(A)
        elif op == 'pla': A = spush.pop(); flags(A)
        elif op == 'tya': A = Y; flags(A)
        elif op in ('cmp', 'sbc', 'adc', 'cpx'):
            if arg.endswith(',x'): o = data[arg[:-2]][X // 2]
            elif arg.startswith('#'): o = val(arg)
            else: o = dp[arg]
            if op == 'adc':
                r = A + o + C; C = int(r > M); A = r & M; flags(A)
            else:
                reg = X if op == 'cpx' else A
                r = reg - o - (0 if op != 'sbc' else (1 - C))
                C = int(r >= 0); flags(r & M)
                if op == 'sbc': A = r & M
        elif op in ('bpl', 'bcc', 'beq', 'bne', 'bra'):
            t = {'bpl': not N, 'bcc': not C, 'beq': Z, 'bne': not Z, 'bra': True}[op]
            if t: pc = labels[arg]
        else: raise Exception('no simulada: ' + ins)
def numero_c_exacto(valor):
    TILE = lambda c: ((c - 32) | 0x2000)
    out = []; empezado = 0
    v = valor
    if valor < 0: out.append(TILE(ord('-'))); v = (-valor) & M
    for pot in (10000, 1000, 100, 10):
        cifra = ord('0')
        while v >= pot: v -= pot; cifra += 1
        if cifra != ord('0') or empezado: out.append(TILE(cifra)); empezado = 1
    out.append(TILE(ord('0') + v))
    return out
fallos = 0; maxpasos = 0
for valor in range(-32768, 32768):
    base = 0x2000
    try:
        mem, fin, pasos = run(base, valor)
    except Exception as e:
        fallos += 1
        if fallos < 5: print('FALLA', valor, 'error:', e)
        continue
    maxpasos = max(maxpasos, pasos)
    esperado = numero_c_exacto(valor)
    obtenido = [mem.get(base + 2 * i) for i in range(len(esperado))]
    if obtenido != esperado or fin != base + 2 * len(esperado) or len(mem) != len(esperado):
        fallos += 1
        if fallos < 5: print('FALLA', valor, obtenido, esperado, hex(fin))
print('valores probados: 65536, fallos:', fallos, ', instrucciones max. por llamada:', maxpasos)
if fallos == 0:
    for v in (0, 7, 1234, -32768, 32767):
        print('valor', v, '->', run(0x2000, v)[2], 'instrucciones')
