# Motor de escenas: logos, cinemáticas y efectos con tabla de eventos

**Estado:** implementado y verificado en Mesen el 2026-10-21 (commit `3c93dfe`).
Primera aplicación: la pantalla del logo Nintendo.

## 0. Historial

| Versión | Fecha | Cambio |
|---|---|---|
| v1.0 | 2026-10-21 | Motor (`src/escena/`), efectos de brillo, mosaico y destello; logo Nintendo. |

## 1. Problema y decisión

Las pantallas de arranque se animaban con llamadas del SDK que **paran el
programa** (`setFadeEffectEx`, `setMosaicEffect`) y con `if (frames_en_pantalla
== N)` sueltos. Eso no escala a cinemáticas (varias cosas a la vez, cada una con
su ritmo) ni se puede probar en el PC.

Decisión: separar el **qué pasa y cuándo** del **cómo se dibuja**.

- **Secuenciador** (`escena/escena.c`, lógica pura): una tabla de eventos
  ordenada por frame mueve *pistas* (números) con rampas en punto fijo 8.8.
  Se compila en el PC y tiene pruebas (`tests/prueba_escena.c`).
- **Efectos** (`escena/efectos.c`, hardware): en cada VBlank lleva el valor
  de cada pista a los registros de la PPU.
- **Una tabla por escena** (`escena/escena_nintendo.c`): se lee como un guion.

## 2. El secuenciador

```c
const EventoEscena ESCENA_NINTENDO[] = {
    FIJAR(0, PISTA_MOSAICO, 15),            /* bloques de 16x16 desde el frame 0 */
    RAMPA(0, 48, PISTA_MOSAICO, 15, 0),     /* ...que se enfocan en 48 frames   */
    RAMPA(0, 32, PISTA_BRILLO, 0, 15),      /* fundido de entrada               */
    RAMPA(70, 56, PISTA_DESTELLO, 0, 255),  /* la banda cruza el logo           */
    RAMPA(70, 14, PISTA_LUZ, 0, 14),
    RAMPA(112, 14, PISTA_LUZ, 14, 0),
    RAMPA(212, 32, PISTA_BRILLO, 15, 0),    /* fundido de salida                */
    FIN_ESCENA(244),
};
```

- **Evento** (`EventoEscena`, 8 bytes justos): frame de inicio, duración,
  pista, valor final y **paso** en 8.8. El paso lo calcula el compilador en la
  macro `RAMPA` (son constantes): en la SNES no hay ninguna división. Se usa
  `* 128 / frames * 2` y no `* 256 / frames` porque 255 × 256 no cabe en el
  `int` de 16 bits de tcc816.
- **Pista** (`Pista`, 8 bytes: `pistas[i]` no multiplica): valor 8.8, paso,
  frames que quedan, destino. En el último frame el valor se pone **exacto**.
- `escena_avanzar()` por frame: arranca los eventos cuyo frame llegó (`<=`,
  no `==`: un evento fuera de orden arranca tarde en vez de congelar la
  pantalla), mueve las pistas, avanza el reloj. Al llegar a `FIN_ESCENA` pone
  `terminada` y no toca nada más.
- `ESCENA_VALOR(e, pista)` es una macro: con la pista constante, el
  compilador resuelve el índice sin cuentas.
- Cuatro pistas hoy: `PISTA_BRILLO` (0..15), `PISTA_MOSAICO` (0..15),
  `PISTA_DESTELLO` (0..255, posición de la banda), `PISTA_LUZ` (0..31).
  Añadir una pista es añadir una constante y su caso en `efectos_aplicar`.

## 3. Los efectos

| Pista | Registro | Cómo |
|---|---|---|
| Brillo | `INIDISP` | `setBrightness(v)`; ojo: `setBrightness(0)` escribe `$80` = **forced blank** (pantalla apagada), mientras que el final de `fade_out` sí deja `$00`, negro **con la pantalla encendida** |
| Mosaico | `MOSAIC` | `(tamaño << 4) \| fondos` |
| Destello | `WH0`/`WH1` por HDMA | ventana 1 cuyos bordes cambian línea a línea |
| Luz | `COLDATA` | suma de color fijo, solo dentro de la ventana (`CGWSEL = 0x10`, `CGADSUB = fondos`) |

**El destello sin CPU.** Los pares (izquierda, derecha) de la ventana para
cada línea se calculan **una vez** (`efectos_destello`): el par *j* vale
`izq = 255 + x_min − ancho − j`. El canal 6 de HDMA en modo **indirecto** y
*repetir* lee un par nuevo por línea avanzando por esa lista; la línea *y*
lee el par `inicio + y`, así que cada línea queda 1 px a la izquierda de la
anterior (banda diagonal `/`). Mover la banda es cambiar **2 bytes**: el
puntero `inicio` en la tabla HDMA (`efectos_aplicar`). Fuera del rectángulo
del logo la ventana queda vacía (izquierda 255 > derecha 0) para no aclarar el
negro, que en el logo es un color opaco.

`efectos_terminar()` apaga HDMA, matemática de color, ventana y mosaico antes
de cambiar de pantalla: el título usa el mismo canal 6 para su cielo.

## 4. Uso en una pantalla

```c
_iniciar:     cargar fondos; efectos_preparar(FONDO_LOGO);
              efectos_destello(64, 59, 40, 215, 12); escena_empezar(&escena, TABLA);
_actualizar:  escena_avanzar(&escena); if (escena.terminada) { efectos_terminar(); secuencia_cambiar(...); }
_vblank:      efectos_aplicar(&escena);
```

`secuencia_vblank` despacha `pantalla_nintendo_vblank`. Las pistas empiezan
en 0, así que la pantalla arranca en negro: la escena hace el fundido.

## 5. Verificación

- `tests/prueba_escena.c` (59 comprobaciones): rampas exactas, `FIJAR` en el
  mismo frame, fin en el frame indicado, evento fuera de orden; y sobre la
  tabla real: orden, duración **244 = 32 + 180 + 32** (la de antes del motor,
  para no correr el arranque: el título sigue a los ~47 s), ningún salto
  mayor que el paso de cada pista, brillo 15 entre fundidos, todo apagado al
  final.
- Mesen (`capturar_mesen.ps1 -Cada 0.1`): mosaico que se enfoca, banda
  diagonal recortada al rectángulo rojo, CAPCOM sin tinte después.

## 6. Trampas encontradas

| Síntoma | Causa | Arreglo |
|---|---|---|
| Raya gris de 1 px bajo el logo durante el destello | el HDMA copia en el HBlank del **final** de cada línea: lo copiado "en la línea n" se ve en la n+1 | la primera entrada de la tabla cubre `arriba − 1` líneas |
| `255 * 256` puede dar un paso absurdo | `int` de 16 bits en tcc816: con todo constante (las tablas de `RAMPA`) el compilador pliega la cuenta con más bits y sale bien, pero si un operando es una variable el producto se hace a 16 bits y desborda (`aprendizaje/unidad-01-enteros-y-punto-fijo`, medido 2026-10-22) | `* 128 ... * 2` en `RAMPA`: la opción segura, no depende de lo que el compilador pliegue |
| Variables `static` sin inicializar con basura | la WRAM no se pone a cero al arrancar y tcc816 no copia `.bss` | inicializar siempre en `_preparar`/`_empezar` |

## 7. Pendiente / ideas

- Más pistas: desplazamiento de capas, posición de un sprite, paleta
  (ciclo de colores), tabla HDMA de color.
- Pasar el resto del arranque (publisher, devs, dedicatoria, intro) al motor:
  quitaría los `setFadeEffectEx` bloqueantes y permitiría solapar efectos.
- Un script que compruebe en el PC que cada `RAMPA` parte del valor en que
  quedó la pista (hoy lo detecta la prueba de saltos).
