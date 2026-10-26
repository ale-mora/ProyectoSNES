# Ejercicio 14 — Leer el ensamblador que genera tcc816

Ejercicio de **lectura** sobre el juego real, más un experimento (que se deshace)
y un punto de ruptura en Mesen. Antes: haber leído el `README.md` de la unidad y
`demo_14_camara_anotado.asm` entero.

**Dónde se trabaja.** Nunca en `E:\ProyectoSNES\src`: un `.asm` olvidado en
`src/` se ensamblaría junto a su `.c` (PLAN §6.4) y los experimentos ensucian el
árbol. Haz un clon de trabajo:

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mi_u14
git -C E:\ProyectoSNES_trabajo\mi_u14 checkout -q plataformas-r2
cd E:\ProyectoSNES_trabajo\mi_u14
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
make                                   # una vez entera: genera los gráficos que
                                       # algunos .c incluyen (personajes.c)
```

> **OJO con PowerShell.** Los comandos de PowerShell (`Get-Content`, `Set-Content`)
> respetan el `cd`, pero los métodos de .NET (`[IO.File]::ReadAllText("src/...")`)
> resuelven las rutas relativas contra la carpeta en la que ARRANCÓ PowerShell, no
> contra la del `cd`. Escribiendo una ruta relativa con .NET se puede acabar
> editando el `E:\ProyectoSNES` de verdad. Usa rutas absolutas o un editor.

Escribe tus respuestas en un `mis_respuestas_14.md` (o anota directamente las
copias de los `.asm`). La solución está en `solucion/solucion_14.md`.

---

## TODO 1 — Generar los `.asm`

En el clon:

```
make src/juego/camara.asm src/juego/nivel.asm src/pantallas/personajes.asm src/juego/guion.asm
```

Muévelos enseguida fuera de `src/` (a tu carpeta de respuestas). Compáralos con
los de `asm_generado/`: deben ser **idénticos** (son texto; las direcciones las
pone después el enlazador, así que no aparecen). Si no lo son, alguien cambió el
`.c` desde el 2026-10-22: manda el código.

## TODO 2 — Anotar `nivel.asm` (`nivel_columna_vram`)

El C (`src/juego/nivel.c`):

```c
return (u16)(((col & 32) << 5) | (col & 31));
```

Copia `nivel.asm` y escribe un comentario en **cada** línea de la función. Tienen
que quedar identificados:

1. dónde se lee el parámetro `col` (¿en qué desplazamiento de la pila y por qué
   ese número, si la función no tiene variables locales?);
2. el `& 32`;
3. el `<< 5`: ¿por qué sale como un **bucle** (`ldy.w #5` / `asl a` / `dey` /
   `bne -`) y no como cinco `asl` seguidos? ¿Cuántas instrucciones ejecuta?
4. el `& 31` y el `|`;
5. dónde queda el valor devuelto.

## TODO 3 — `tile_del_bloque64` frente a `tile_del_bloque32`

En `asm_generado/personajes.asm` busca las dos funciones (se llaman
`tccs_{WLA_FILENAME}_tile_del_bloque32` y `..._64`: el prefijo `tccs_` y el
nombre de archivo los pone tcc816 a las funciones `static`).

```c
static u16 tile_del_bloque32(u16 n) { return (u16)((n / 4) * 64 + (n % 4) * 4); }
static u16 tile_del_bloque64(u8 n)  { return (u16)(TILE_BASE_BARBARA + (n / 2) * 128 + (n % 2) * 8); }
```

1. Cuenta los `jsr.l tcc__div` de cada una.
2. ¿Cómo se ha traducido `n / 4` en la de 32? ¿Y `n % 4`?
3. Explica por qué la de 64, que divide por 2 (aún más fácil), llama dos veces a
   la rutina de dividir. Pista: ¿a qué tipo convierte C un `u8` antes de operar,
   y tiene ese tipo signo? ¿Cuánto da `-3 / 2` en C y cuánto `-3 >> 1`?
4. Compruébalo: aplica `solucion/personajes_u16.patch` (cambia `n` por `(u16)n`),
   genera otra vez `personajes.asm` y cuenta. Deshaz el cambio
   (`git checkout -- src/pantallas/personajes.c`).
5. Mira `asm_generado/divisiones.asm` (la sonda de `asm_generado/divisiones.c`) y
   completa la tabla: para `u8 / 2`, `(u16)u8 / 2`, `u16 / 2`, `s16 / 2` y
   `u16 / u16`, ¿qué instrucción o rutina sale?

## TODO 4 — El experimento de `guion[i]`

`mantenidos` (`src/juego/guion.c`) recorre la tabla del guion con un puntero que
avanza (`t++`). Vas a reescribirla «como en los libros», con un índice, y mirar
qué pasa.

1. En `asm_generado/guion.asm`, busca `mantenidos`. ¿Cuánto suma `t++` al
   puntero (busca el `adc.w #`)? Eso es `sizeof(TramoGuion)` para tcc816.
   Compáralo con lo que dice el comentario de `guion.c` y razona por qué
   (`TramoGuion` son `u16`, `u16`, `u8`).
2. Aplica `solucion/guion_indice.patch` (o escríbelo tú: `u8 i;` y `guion[i].desde`,
   `guion[i].hasta`, `guion[i].botones`) y genera `src/juego/guion.asm`.
3. Cuenta los `jsr.l tcc__mul` dentro de `mantenidos`. ¿Qué número se carga en
   `tcc__r9` justo antes de cada uno? ¿Qué cuenta está haciendo?
4. El guion del tutorial tiene 13 tramos (`GUION_TUTORIAL_TRAMOS`) y
   `guion_mando` llama a `mantenidos` dos veces por frame. ¿Entre cuántas y
   cuántas multiplicaciones por frame costaría la versión con índice? (Ojo con
   `&&`: si la primera comparación falla, la segunda no se evalúa.)
5. Explica por qué la versión con puntero no tiene ninguna.
6. **Deshaz el cambio:** `git checkout -- src/juego/guion.c`, y borra el `.asm`
   de `src/`.

## TODO 5 — Un `switch` y un `>>` con signo

En `hud_debug_dibujar` (genera `src/juego/hud_debug.asm`) está el
`switch (turno)` de las 8 filas del HUD.

1. ¿Sale como una tabla de saltos o como una cadena de comparaciones? Copia las
   cuatro primeras líneas de cada caso (`cmp #0`, `cmp #1`...).
2. En el caso 0 aparece cuatro veces seguidas `cmp #$8000` / `ror a`. ¿Qué
   expresión de C es? (Pista: `PF_ENTERO(t->x_fp)`, `src/sistema/puntofijo.h`.)
   ¿Por qué no es un simple `lsr a`?

## TODO 6 — Un punto de ruptura en `camara_para` (Mesen)

Con la ROM normal del juego (`ProyectoSNES.sfc` del clon, con su `.sym` al lado):

1. Abre la ROM en `tools\mesen\Mesen.exe`. Debug → Debugger. Si las etiquetas no
   aparecen, File → Import Labels (o «Workspace → Import labels») y elige el `.sym`.
2. En la lista de etiquetas, `camara_para` → clic derecho → *Toggle breakpoint*
   (o un breakpoint de ejecución en su dirección: la del `.sym`, cambia en cada
   compilación).
3. Juega hasta el nivel (Start en el título). El juego se para al entrar:
   `pantalla_nivel1_iniciar` llama a `camara_para(X_INICIAL)`.
4. Anota y **explica** cada valor del panel de registros: `A`, `X`, `Y`, `S`,
   `D`, `DB` (o `DBR`), `PB` (o `K`), y las banderas `m`, `x`, `e`, `c`, `z`, `n`.
   ¿Por qué `DB` vale `$7E`? ¿Por qué `PB` es `$8x` y no `$0x`?
5. Mira la pila en el visor de memoria (a partir de `S + 1`): localiza la
   dirección de vuelta (3 bytes) y el parámetro `x_pix` (2 bytes, byte bajo
   primero). ¿Cuánto vale? Compruébalo con `X_INICIAL` de
   `src/pantallas/pantalla_nivel1.c` y `NIVEL1_INICIO_X` de
   `res/nivel1/nivel1_datos.h`.
6. Avanza instrucción a instrucción (F11, *Step Into*) hasta el `rtl`, mirando
   `tcc__r0` (dirección `$0000` de la página directa; en el visor, `$7E0000`).
   ¿Qué valor intermedio toma antes del primer `if` y con cuál sale? Haz la
   cuenta en decimal y en complemento a dos.
7. Quita el breakpoint, camina un poco a la derecha, vuelve a ponerlo y repite
   el 5 y el 6 con el nuevo `x_pix`.

---

## Criterio de aprobación

- Los TODO 2–5 respondidos y el `nivel.asm` anotado línea a línea, con las cinco
  piezas del TODO 2 identificadas.
- La cuenta del TODO 4 hecha con el `.asm` que generaste tú (no con el de
  `solucion/`), y el clon sin cambios al terminar (`git status` limpio salvo el
  `.asm` que hayas sacado de `src/`).
- Una **captura del debugger de Mesen** parado en `camara_para` con los registros
  explicados (TODO 6), y el valor de `tcc__r0` a la salida que coincide con tu
  cuenta.
