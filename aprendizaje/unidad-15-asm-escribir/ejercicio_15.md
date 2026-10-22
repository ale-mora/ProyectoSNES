# Ejercicio 15 — `numero` del HUD en ensamblador

Ejercicio **sobre el juego real**: reescribir en ensamblador la función que
escribe los números del HUD de depuración, comprobar que da exactamente los
mismos dígitos que la de C y medir cuántos ciclos ahorra. Antes: el `README.md`
de la unidad y la mini-ROM de `rom/` compilada y leída.

**Dónde se trabaja.** En un clon, nunca en `E:\ProyectoSNES`:

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mi_u15
cd E:\ProyectoSNES_trabajo\mi_u15
git apply E:\ProyectoSNES\aprendizaje\unidad-15-asm-escribir\andamio_hud_debug.c.patch
copy E:\ProyectoSNES\aprendizaje\unidad-15-asm-escribir\ejercicio_15_hud_numero.asm src\juego\hud_numero.asm
```

El **andamio** (`andamio_hud_debug.c.patch`) prepara `src/juego/hud_debug.c`
para el ejercicio, sin tocar nada si no se pide:

- todas las llamadas a `numero(...)` pasan por una macro `NUMERO`;
- compilando con `-DHUD_NUMERO_ASM`, `NUMERO` es `hud_numero` (la tuya, en
  `src/juego/hud_numero.asm`); sin la macro, la de C de siempre;
- con `-DHUD_NUMERO_ASM`, `comprobar_numero()` llama a las dos versiones al
  encender el nivel con 14 valores (0, 9, 10, 99, 100, 1234, 10000, 32767, −1,
  −9, −10, −1000, −32767, −32768) sobre búferes rellenos de basura igual, y
  cuenta cuántos salen idénticos (largo y palabras); la fila 5 del HUD lo
  muestra: `CAM 0  F 1234  ASM 14/14`.

---

## TODO 1–7 — Completar `src/juego/hud_numero.asm`

Los TODO están numerados dentro del archivo. La lógica es la de `numero` en
`src/juego/hud_debug.c` (léela antes); el reparto de registros está en la
cabecera del esqueleto. Reglas:

- Todo en 16 bits (`rep #$30` al principio); no hace falta `sep` en ningún
  momento: el mapa del HUD son palabras.
- `[tcc__r0],y` es «la dirección de 24 bits guardada en `tcc__r0`/`tcc__r0h`,
  más Y». Escribe con `sta [tcc__r0],y` y avanza Y de 2 en 2 (`iny` / `iny`).
- `pha` / `pla` para no perder `v` cuando necesites A para otra cosa.
- Cada línea con su comentario en español.

**Comprobación rápida en el PC, sin compilar la ROM** (un mini-simulador del
65816 que solo entiende las instrucciones de esta rutina, en 16 bits, y la
compara con la versión en C para los 65 536 valores de `s16`):

```
python E:\ProyectoSNES\aprendizaje\unidad-15-asm-escribir\comprobar_hud_numero.py src\juego\hud_numero.asm
```

Tiene que decir `fallos: 0`. Con el esqueleto sin completar dice
`fallos: 65536`. El simulador no comprueba que WLA-DX acepte la sintaxis ni el
enlazado: eso lo dice `make`.

## TODO 8 — Compilar las dos ROM de prueba

En el clon, desde PowerShell, con el entorno del SDK (CLAUDE.md):

```
make clean; $env:CFLAGS="-DGUION_PRUEBA"; make; $env:CFLAGS=$null
copy ProyectoSNES.sfc ..\hud_c.sfc; copy ProyectoSNES.sym ..\hud_c.sym
make clean; $env:CFLAGS="-DGUION_PRUEBA -DHUD_NUMERO_ASM"; make; $env:CFLAGS=$null
copy ProyectoSNES.sfc ..\hud_asm.sfc; copy ProyectoSNES.sym ..\hud_asm.sym
```

Las dos deben terminar en `No warnings or errors found.` Busca `hud_numero` en
`hud_asm.sym`: si no está, tu `.asm` no se ensambló (¿está en `src/juego/`?).

## TODO 9 — Los mismos dígitos (Mesen)

Abre `hud_asm.sfc` en Mesen. Es la ROM que se juega sola: arranca directamente
en el nivel. Pulsa SELECT (tecla E) para encender el HUD.

1. La fila 5 debe decir `ASM 14/14`.
2. Todas las filas con números (`X`, `Y`, `CAM`, `F`, `LIN`, `V`, `GOLPE`) se
   ven bien, con signo cuando toca (`VX -16` al frenar hacia la izquierda...).

**Por qué no se comparan capturas de la ROM en C y la de asm:** el HUD solo
refresca una fila si el haz va por encima de `HUD_LINEA_MAX` (unidad 04), y al
ser más rápida la versión en asm cambia QUÉ frames refrescan; dos capturas del
mismo segundo pueden mostrar números distintos y estar las dos bien. Por eso la
comparación la hace la propia ROM (`comprobar_numero`) con valores fijos.

## TODO 10 — Medir ciclos (profiler de Mesen)

En cada una de las dos ROM: Debug → **Performance Profiler** (con el `.sym`
cargado), deja correr el nivel con el HUD encendido unos 20 s y anota de la fila
de la función:

| ROM | función | llamadas | ciclos medios por llamada | % del total |
|---|---|---|---|---|
| `hud_c.sfc` | `tccs_src/juego/hud_debug.asm_numero` | | | |
| `hud_asm.sfc` | `hud_numero` | | | |

Y, en las dos, `hud_debug_dibujar` (la función entera que dibuja una fila).
Explica la diferencia con lo que has visto en el `.asm` generado de `numero`
(genera `src/juego/hud_debug.asm` con la receta de la unidad 14 y cuenta las
instrucciones de la vuelta de la resta).

## TODO 11 — De paso: `*p++ = 0` frente a `memset`

En la mini-ROM (`rom/`), genera el ensamblador de `relleno_c.c`
(`make src/relleno_c.asm` dentro de `rom/`, y sácalo de `src/` enseguida):

1. Cuenta las instrucciones que ejecuta **una vuelta** de `limpiar_c` (sigue el
   camino de una vuelta que no termina: no cuentes el `dex` ni el `brl` que se
   saltan).
2. Lee el bucle de `memset` en `../unidad-14-asm-leer/capturas/rutinas_libreria.txt`
   (sacado de los bytes de la ROM). ¿Cuántas instrucciones por byte?
3. Con el profiler de Mesen sobre `rom/unidad15.sfc`: ciclos de `limpiar_c` y de
   `memset` (las dos vacían los mismos 256 bytes, una vez).

## Ampliación — `colision_apoyan` en ensamblador

El plan pedía `colision_en`, «la consulta más repetida de la física». **Hoy el
juego ya no la llama** (solo las pruebas): `tarou.c` usa las consultas de
columna y fila de `colision.c`. La más repetida es `colision_apoyan`: la llama
`apoyado` en `mover_y` en cada frame que Tarou está en el suelo y en cada borde
de celda que cruza al caer.

1. Escríbela en `src/juego/colision_rapida.asm` (no puede llamarse
   `colision.asm`). Pista: una subrutina local con `jsr`/`rts` para «¿sostiene
   este pie?» evita escribir dos veces lo mismo.
2. Problema 1: la rejilla es `static const u8 *rejilla;` en `colision.c`, y un
   `static` de tcc816 se llama `tccs_<archivo>_rejilla`: tu `.asm` no la ve.
   Hazla global con otro nombre (`colision_rejilla`).
3. Problema 2: si `colision.c` sigue definiendo `colision_apoyan`, el enlazador
   encuentra dos. Pero las pruebas del PC la necesitan en C. Usa `#ifdef
   PRUEBAS_PC` (lo define `tests/Makefile`).
4. Comprueba: `make -C tests` y `.\tests\pruebas.exe` → `367 pruebas, 0 fallos`
   (prueban la versión en C); la ROM con `-DGUION_PRUEBA` compila; en Mesen el
   recorrido entero (sin manos) llega a «LEVEL 2 / COMING SOON» exactamente igual
   que la ROM sin el cambio (la reaparición, el muro, el foso...). Y en el
   profiler, ciclos de `colision_apoyan` antes y después, **inclusivos** (los
   que cuentan también lo que la función llama): si la tuya llama a una
   subrutina con `jsr`, sus ciclos propios (exclusivos) no los incluyen.

---

## Criterio de aprobación

- `comprobar_hud_numero.py` da `fallos: 0` con tu `hud_numero.asm`.
- Las dos ROM compilan sin avisos; en Mesen, la de asm muestra `ASM 14/14` y el
  HUD con números correctos (captura).
- La tabla del TODO 10 rellena: menos ciclos por llamada en la versión en asm.
- (Ampliación) 367/367 en el PC y el recorrido de la ROM de prueba completo.
- Al terminar, el clon se borra o se deja aparte: nada de esto va a la rama
  principal.
