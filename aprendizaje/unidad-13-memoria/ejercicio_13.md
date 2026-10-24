# Ejercicio 13 — Encontrar los bytes: en el `.sfc`, en el `.log` y en la WRAM

Tres partes. Las direcciones de los ejemplos son las de la compilación del
2026-10-22; las tuyas salen de **tu** `.sym` (README, «Antes de empezar»).

## Parte A — Del `.sym` al byte del `.sfc` (PC)

Archivo: `ejercicio_13_rom.c`. Compila tal como está (no hace nada). Complétalo:

- **TODO 1.** Abrir el `.sym`, leerlo línea a línea y encontrar la etiqueta
  pedida (`sscanf(linea, "%8lx %399s", &valor, nombre)` separa el número y el
  nombre; las líneas de comentario no encajan y se saltan solas).
- **TODO 2.** Separar el banco (bits 16..23) y la posición (bits 0..15).
- **TODO 3.** Si es WRAM o registros, decirlo y salir. Si es ROM, calcular el
  byte del `.sfc` con la fórmula LoROM del README.
- **TODO 4.** Leer esos bytes del `.sfc` y compararlos con el archivo de
  origen; sin archivo, imprimir el primer byte.

Compílalo y ejecútalo con tu ROM (desde esta carpeta; `C` es tu clon):

```
gcc -std=c99 -Wall -Wextra -o e13.exe ejercicio_13_rom.c
$C="E:\ProyectoSNES_trabajo\u13"
.\e13.exe $C\ProyectoSNES.sym $C\ProyectoSNES.sfc nivel1_colision $C\res\nivel1\colision.bin
.\e13.exe $C\ProyectoSNES.sym $C\ProyectoSNES.sfc nivel1_cerca_col_banco
.\e13.exe $C\ProyectoSNES.sym $C\ProyectoSNES.sfc nivel1_cerca_col $C\res\nivel1\fondo_cerca.col
.\e13.exe $C\ProyectoSNES.sym $C\ProyectoSNES.sfc ESCENA_NINTENDO
```

Salida esperada (salvo las rutas y, si tu código es otro, las direcciones):
`capturas/salida_ejercicio_13_rom.txt`.

```
nivel1_colision: $86:F900 -> byte 0x037900 del .sfc
nivel1_colision: los 1792 bytes coinciden con .../res/nivel1/colision.bin
nivel1_cerca_col_banco: $80:FFB1 -> byte 0x007FB1 del .sfc
nivel1_cerca_col_banco: primer byte = $81
nivel1_cerca_col: $81:B3BA -> byte 0x00B3BA del .sfc
nivel1_cerca_col: los 16640 bytes coinciden con .../res/nivel1/fondo_cerca.col
ESCENA_NINTENDO: $7F:01FA no esta en la ROM (WRAM o registros)
```

(`fondo_cerca.col` lo genera `make` en tu clon a partir del mapa; no está en
git.)

**Preguntas:**

- a) ¿Por qué el primer byte de `nivel1_cerca_col_banco` es justo el banco de
  `nivel1_cerca_col`? ¿Qué escribió eso, y cuándo?
- b) ¿Por qué `fondo_nivel.c` no puede sacar ese `$81` de `&nivel1_cerca_col`?
- c) `ESCENA_NINTENDO` es `const` y sale en la WRAM. ¿Dónde están entonces sus
  64 bytes de valores iniciales y quién los copia? (Pista: genera
  `src/escena/escena_nintendo.asm` en tu clon con `make` y busca `globram` y
  `glob.data`; después bórralo de `src/`.)

## Parte B — ¿Cuántos cuadros de 96×64 caben todavía? (`.log`)

Abre tu `ProyectoSNES.log` y busca el bloque `ROM bank 0 ... ROM bank 127` del
final.

1. ¿Cuántos bancos están vacíos del todo? ¿Qué bancos de la CPU son (`$80 + n`)?
2. Un cuadro de Tarou son 3 072 bytes y una animación es una sección superfree.
   ¿Cuántos cuadros caben por banco? ¿Cuántos en los bancos vacíos?
3. El banco a medias (`ROM bank 14`): ¿cuántos cuadros más caben en su hueco?
4. Compara tu total con la división ingenua `bytes libres / 3072`. ¿Por qué no
   coinciden?
5. ¿Por qué hace falta `nivel1_cerca_col_banco` aunque el código ya sepa la
   dirección de 16 bits de `nivel1_cerca_col`?

## Parte C — La `struct Tarou` en Memory Tools (Mesen)

ROM: la de prueba que se juega sola con el HUD encendido desde el principio
(`E:\ProyectoSNES_trabajo\roms\u13_memoria_guion.sfc`, con su `.sym` al lado;
o la tuya: en tu clon, `make clean; $env:CFLAGS="-DGUION_PRUEBA"; make;
$env:CFLAGS=$null` y SELECT para el HUD).

1. Busca en su `.sym` la variable `static Tarou tarou;` de
   `pantalla_nivel1.c`. Las `static` salen con nombre largo:
   `tccs_src/pantallas/pantalla_nivel1.asm_tarou`. En la ROM de prueba:
   `$7E:2631`.
2. Con `src/juego/tarou.h` (la `typedef struct ... Tarou`) apunta el
   desplazamiento de cada campo. Pista: los `s16` miden 2 bytes y van primero;
   tcc816 deja `frames_quieto` en +16 (un byte de relleno en +15: la struct mide
   18 bytes, lo dice el `.asm` de `pantalla_nivel1.c`: `..._tarou dsb 18`).
3. Abre la ROM en Mesen, deja que el guion empiece a caminar y abre Debug →
   Memory Tools. Elige la memoria de la CPU (o la WRAM: entonces la posición es
   `$2631`, sin el banco) y ve a la dirección (Ctrl+G).
4. Pausa (Game → Pause), haz una captura de la ventana de Memory Tools con los
   18 bytes y la del juego con el HUD. Avanza unos segundos y repite.

**Lo que tiene que verse:** `x_fp` (los 2 primeros bytes, el bajo primero) en
12.4: dividido entre 16 es la `X` del HUD de esa misma captura; entre las dos
capturas ha crecido. `estado` (+8) cambia según lo que haga Tarou (el número
del `enum Estado` de `src/juego/animaciones.h`; el HUD enseña su nombre en
`EST`), y `en_suelo` (+11) vale 1 en el suelo y 0 en el aire.

Respuestas: `solucion/respuestas_13.md`.
