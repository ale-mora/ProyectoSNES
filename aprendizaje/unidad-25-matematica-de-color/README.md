# Unidad 25 — Matemática de color: la sombra, la media luz de la pausa y lo translúcido

> **Código real, rama principal.** Esta unidad enseña sobre el código del beat 'em up
> (commit `6845b5f`, 2026-11-05): `src/juego/protagonista_sprite.c`
> (`protagonista_escena_cargar`), `src/pantallas/pantalla_fase1.c` (`oscurecer`,
> `pantalla_fase1_vblank`), `src/juego/estado_fase.c`, `src/juego/ambiente_sprite.c` y
> la paleta de la sombra. El ejercicio se hace en un clon de la rama actual:
>
> ```text
> git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u25
> git -C E:\ProyectoSNES_trabajo\u25 checkout -q 6845b5f
> ```
>
> **La ficha de `PLAN.md` promete más de lo que hay.** Lo que **existe**: la matemática de
> color con la pantalla secundaria (la sombra, el humo y los pájaros) y con el color fijo
> (la media luz de la pausa y del GAME OVER), y las paletas de sprites oscurecidas a mano.
> Lo que **todavía no existe** y esta unidad no enseña como hecho: las **ventanas** dibujadas
> por HDMA (los tajos de luz de la gata samurái), el mosaico pulsante y la ondulación del
> perro monje, el temblor de pantalla del perro bruto, los apoyos (`apoyos.c`) y Mode 7.
> Están en «Por venir», rotulados.

**Objetivo:** entender cómo la PPU mezcla colores sin que la CPU toque un píxel: por qué
la sombra oscurece lo que tiene debajo, por qué el humo es translúcido sobre las montañas
pero no sobre el cielo, cómo la pausa deja la pantalla a media luz con tres registros, por
qué los cuerpos (paletas 0–3) no entran en esa cuenta y hay que oscurecer sus paletas a
mano, y cómo se hace un destello de pantalla sin estropear todo lo anterior.

**Prerrequisitos:** 06 (paletas y BGR555), 08 (sprites y la OAM: el byte de atributos y
su paleta), 12 (HDMA y el motor de escenas: la primera matemática de color del juego, el
brillo del logo), 21 (capa de sprites), 22 (la sombra), 18 (pruebas de PC).
**Práctica:** PC (gcc) + juego real en un clon, con capturas de Mesen.
**Tiempo orientativo:** una tarde (teoría y demo) y otra (ejercicio).

## Conceptos

### Pantalla principal, pantalla secundaria y color fijo

La PPU compone cada línea **dos veces**. La **pantalla principal** (*main screen*) es la
imagen normal: las capas que enciende el registro `TM` (`$212C`). La **pantalla
secundaria** (*sub screen*) es una segunda imagen, con las capas que enciende `TS`
(`$212D`), que **no se ve**: solo sirve de operando para mezclar. El **color fijo**
(*fixed color*, `COLDATA`, `$2132`) es un único color de 15 bits que también puede hacer
de operando. → glosario: Matemática de color

La **matemática de color** (*color math*) es esto: para cada píxel de la principal cuya
capa lo tenga permitido, la PPU **suma o resta** el operando, canal a canal, y si se pide
**divide entre 2**. Los canales son de 5 bits (0..31) y el resultado se corta: no baja de 0
ni pasa de 31. Lo que se ve es ese resultado. La cuenta es de hardware: cuesta **cero**
ciclos de CPU, la haga en un píxel o en toda la pantalla.

### Los tres registros

| Registro | Bits que usa el juego | Qué dice |
|---|---|---|
| `CGWSEL` `$2130` | bit 1 | **qué operando**: 0 = el color fijo; 1 = la pantalla secundaria (y donde la secundaria no tiene nada, el color fijo) |
| `CGADSUB` `$2131` | bit 7 restar · bit 6 mitad · bit 5 color de fondo · bit 4 sprites de paletas 4–7 · bits 3..0 BG4..BG1 | **qué capas** de la principal se mezclan y **cómo** |
| `COLDATA` `$2132` | bits 7/6/5 = escribir en azul/verde/rojo · bits 4..0 = intensidad | el **color fijo**: `$E0` = los tres canales a 0 (negro); `$E0 \| 16` = gris 16 |

`COLDATA` es raro: no se escribe el color de una vez, sino una **intensidad** y a qué canales
aplicarla. `$E0` pone 0 en los tres; `$20 | 31` pondría solo el rojo a 31 y dejaría los
otros dos como estuvieran.

### La regla de los sprites: solo las paletas 4–7

En `CGADSUB`, el bit 4 enciende la matemática para los **sprites**, pero **solo para los
que usan las paletas 4 a 7** (las 8 paletas de sprites están en la CGRAM desde el color 128).
Un sprite de paleta 0–3 **nunca** se mezcla, se ponga lo que se ponga. El juego lo usa a
propósito:

| Paleta de sprites | Qué la usa | ¿Se mezcla? |
|---|---|---|
| 0 | la protagonista (Tarou o Bárbara) | no: cuerpo opaco |
| 1, 2, 3 | los tres ninjas (rojo, amarillo, morado) | no |
| 4 | la sombra (16 colores negros, `ninja_pal.bin` + 96) | sí: oscurece lo de debajo |
| 7 | el humo y los pájaros (`ambiente_sprite.c`, `PALETA 7`) | sí: translúcidos |

Y el modo es **global**: `CGADSUB` vale lo mismo para toda la pantalla (salvo que un canal
de HDMA lo cambie a mitad de imagen, unidad 12). No hay «este sprite suma y aquel resta».

### La escena del combate: media suma con la secundaria

`protagonista_escena_cargar` (`src/juego/protagonista_sprite.c:97`) deja la mezcla de todo
el combate:

```c
REG_TS = 0x03;          /* BG1 y BG2 también en la pantalla secundaria.          */
REG_CGWSEL = 0x02;      /* Operando: la secundaria.                               */
REG_CGADSUB = 0x50;     /* 0x40 mitad + 0x10 sprites 4..7: (sprite + fondo) / 2.   */
```

- **La sombra** es negra. En cada píxel de la sombra: (0 + fondo) / 2 = **el fondo a media
  luz**. Por eso parece una sombra de verdad (se ve la calle por debajo, más oscura) y no
  una mancha negra. Con los colores reales, la tierra `r=23 g=17 b=12` pasa a `11 8 6`.
- **El humo** sobre una ladera: (humo + ladera) / 2, una mezcla a partes iguales: se ve
  translúcido.
- **El humo sobre el cielo.** Ahí la secundaria no tiene nada (el cielo es el **color de
  fondo**, el color 0 de la CGRAM pintado por el HDMA del atardecer: no es BG1 ni BG2).
  Cuando el operando es la secundaria y en ese píxel está vacía, la PPU usa el **color fijo**
  y **no divide entre 2**. El color fijo es negro, así que el humo sale **con su color** sobre
  el cielo. Esto está medido, no supuesto: en una captura de Mesen de la aldea aparecen los
  grises del humo **exactos** (`#b8aab0` → `189,173,181` en la captura), cosa imposible si
  se hubieran dividido. Los cuervos, negros, se ven negros sobre el cielo por lo mismo.

### La media luz de la pausa: restar el color fijo

Al pausar (y en el GAME OVER), `estado_fase_atenuada` (`src/juego/estado_fase.c`) devuelve
1. La pantalla no escribe los registros enseguida: marca `cambiar_luz`
(`pantalla_fase1.c:322`) y los escribe en `pantalla_fase1_vblank` (`:421`), dentro del
VBlank, como todo lo que toca la PPU:

```c
REG_COLDATA = 0xE0;     /* El color fijo: negro.                                  */
REG_CGWSEL = 0x00;      /* Operando: el color fijo (no la secundaria).             */
REG_CGADSUB = 0xF3;     /* restar + mitad + fondo + sprites 4..7 + BG2 + BG1.      */
```

(color − 0) / 2 = **la mitad del brillo**, en BG1, BG2, el cielo y los sprites 4–7. Restar
negro no cambia nada (sumarlo daría lo mismo); lo que oscurece es la **mitad**. Con otro
color fijo, el mismo modo restaría un tinte (menos rojo y verde: una noche azulada). Fíjate en lo que **no** está: el bit 2, BG3. El texto (PAUSE, la barra de
vida) va en BG3 y se queda con todo su brillo.

### Los cuerpos: oscurecer la paleta a mano

La protagonista y los ninjas (paletas 0–3) no entran en la cuenta: en pausa saldrían a
plena luz sobre un fondo oscuro. El juego hace la mitad **en la CPU, una vez**: al entrar en
la fase calcula copias oscuras de esas 4 paletas (`oscurecer`, `pantalla_fase1.c:186`;
`oscuras[64]`) y, al pausar, las copia a la CGRAM por DMA (128 bytes). Al reanudar copia
las originales.

Dividir un color BGR555 entre 2 parece `v >> 1`, pero **no lo es**: los tres canales van
pegados en 15 bits, y al desplazar, el bit bajo del verde cae en el bit alto del rojo, y el
bajo del azul en el alto del verde. La máscara `0x3DEF` (`0011 1101 1110 1111`) borra
justo esos tres bits altos (el 4, el 9 y el 14) después de desplazar:

```c
destino[i] = (u16)((origen[i] >> 1) & 0x3DEF);
```

La demo lo enseña con números: un naranja `r=21 g=13 b=7` con `v >> 1` da `r=26 g=22`
(¡más claro que el original!) y con la máscara, `r=10 g=6 b=3`.

### Un efecto a la vez

Como el modo es global, **dos efectos no se pueden mezclar** con unos registros: la pausa
**sustituye** la media suma de la sombra (al reanudar, `pantalla_fase1_vblank` vuelve a
`0x02` / `0x50`). Mientras dura la pausa, la sombra (paleta 4) también queda a media luz,
que es lo que se quiere. El ejercicio de esta unidad añade un tercer modo, el destello, y
tiene que convivir con los otros dos.

## La librería: PVSnesLib

| Función (firma exacta) | Qué hace / registros | Dónde |
|---|---|---|
| `void setColorEffect(u8 colorMathA, u8 colorMathB);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | escribe `CGWSEL` (`colorMathA`) y `CGADSUB` (`colorMathB`) | el juego no la usa: escribe `REG_CGWSEL` / `REG_CGADSUB` en el VBlank |
| `void setColorIntensity(u8 colorApply, u8 intensity);` | escribe `COLDATA`: `colorApply` con `CM_APPLY_RED/GREEN/BLUE` (bits 5/6/7) y la intensidad | el juego escribe `REG_COLDATA` |
| `REG_TS`, `REG_CGWSEL`, `REG_CGADSUB`, `REG_COLDATA` (macros de `background.h` y `video.h`) | los registros mismos, como variables | `protagonista_sprite.c`, `pantalla_fase1.c` |
| `CM_SUBBGOBJ_ENABLE` (bit 1 de `CGWSEL`), `CM_SUB_MAINSUB`, `CM_DIV2_DIVIDE`, `CM_MSCR_BACK`, `CM_MSCR_PAL47`, `CM_MSCR_BG1..BG4` | los nombres de los bits (`video.h`) | el juego escribe los números (`0x50`, `0xF3`) con el comentario al lado |
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | copia paletas a la CGRAM (las oscuras, al pausar) | `pantalla_fase1_vblank` |

El ejemplo del SDK es `snes-examples/graphics/Effects/Transparency/Transparency.c`:
`setColorEffect(CM_SUBBGOBJ_ENABLE, CM_MSCR_BACK | CM_MSCR_BG1);` (BG1 y el fondo se mezclan
con la secundaria). **Primero la librería, después el registro:** `setColorEffect(0x02, 0x50)`
haría lo mismo que las dos líneas del juego. El juego baja al registro porque estas
escrituras van **dentro del VBlank** y no hace falta más.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| El modo del combate (`TS`, `CGWSEL`, `CGADSUB`) | `src/juego/protagonista_sprite.c:97` (`protagonista_escena_cargar`); se apaga en `protagonista_sprite_ocultar` (`:231`) |
| La paleta 4 de la sombra | `PALETA_SOMBRA` en `protagonista_sprite.c:47` y `ninja_sprite.c` |
| La paleta 7 del humo y los pájaros | `PALETA` en `src/juego/ambiente_sprite.c:31`; su paleta, `res/aldea/ambiente_pal.bin` |
| Cuándo hay media luz | `estado_fase_atenuada` en `src/juego/estado_fase.c` |
| Las paletas oscuras | `oscurecer` (`pantalla_fase1.c:186`) y su uso al iniciar (`:273-276`) |
| Las escrituras en el VBlank | `pantalla_fase1_vblank` (`pantalla_fase1.c:421`) |
| La otra matemática de color del juego | `src/escena/efectos.c` (el brillo del logo Nintendo, unidad 12: suma con el color fijo dentro de una ventana) |

## Trampas

| Trampa | Qué pasa | Cómo se evita |
|---|---|---|
| Poner un cuerpo en la paleta 4–7 | sale translúcido (se ve el fondo a través del ninja) | cuerpos en 0–3, efectos en 4–7 |
| Oscurecer con `v >> 1` sin máscara | colores más claros o de otro tono (bits que se cuelan) | `& 0x3DEF` |
| Escribir `CGADSUB` fuera del VBlank | la mezcla cambia a mitad de la imagen (una franja) | en `_vblank`, como el scroll |
| Dejar `COLDATA` con un color tras un efecto | el humo y los pájaros sobre el cielo suman ese color (la secundaria vacía usa el color fijo) | volver a `$E0` al terminar |
| Olvidar el bit 4 (`0x10`) en un modo nuevo | la sombra pasa a negro opaco mientras dura (se ve en la captura del ejercicio) | decidirlo a propósito: en un destello de 4 frames no se nota |
| Encender el bit 2 (BG3) en la pausa | el texto «PAUSE» también se oscurece | BG3 fuera de `CGADSUB` |

## La demo

`demo_25_color.c` hace en el PC la cuenta de la PPU canal a canal, con los colores reales de
las paletas del juego. Desde esta carpeta:

```text
gcc -std=c99 -Wall -Wextra -o demo_25.exe demo_25_color.c
.\demo_25.exe
```

Salida real en `capturas/salida_demo_25_color.txt`. Qué mirar: A, la sombra sobre la calle
(la mitad de cada canal); B, el humo sobre la montaña (la media) y sobre el cielo (el humo
tal cual: la regla de la secundaria vacía); C, la fachada en pausa; D, el naranja de Tarou
oscurecido bien y mal; E, el destello del ejercicio, con el rojo de la fachada cortado en 31.

Antes de ejecutarla, predice: ¿qué color sale al poner la sombra sobre un píxel de cielo?
(Respuesta: la secundaria está vacía, así que el operando es el color fijo, negro, y no se
divide: negro + negro = **negro opaco**, una mancha y no una sombra. No pasa en el juego
porque la sombra siempre cae sobre la calle, que es BG1.)

## Ejercicio y criterio de aprobación

**Ejercicio (juego real, en un clon, con TDD):** `ejercicio_25.md`. Al recibir un golpe, la
pantalla se aclara 4 frames sumando un gris que baja de 16 a 4. La regla (cuándo empieza,
cuánto dura, qué pasa con otro golpe a mitad, que reiniciar la fase no cuenta) va en un
módulo puro, `src/juego/destello.c`, con su prueba escrita **antes**; la pantalla escribe los
registros en el VBlank sin pisar la pausa. Esqueletos: `ejercicio_25_destello.h/.c`.
Solución: `solucion/`.

**Aprobación:**

1. la demo compila sin avisos y su salida coincide con `capturas/salida_demo_25_color.txt`;
2. con la prueba escrita y el esqueleto sin completar: **6 fallos**, todos de
   `prueba_destello.c` (`capturas/pruebas_rojo.txt`);
3. completado: **2497 pruebas, 0 fallos** (`capturas/pruebas_verde.txt`; antes eran 2477);
4. la ROM normal y la de prueba (`-DGUION_FASE1 -DDESTELLO_FRAMES=63`) compilan con «No
   warnings or errors found» (`capturas/compilacion.txt`);
5. en Mesen, con la ROM de prueba, el golpe aclara los fondos y el cielo pero **no** a los
   personajes ni el texto, y la pantalla vuelve a ser la de antes
   (`capturas/mesen_antes_del_golpe.png`, `mesen_destello.png`, `mesen_despues.png`).

## Por venir (no enseñar como hecho)

> Al día (2026-11-05): después del commit de esta unidad ya existen las ventanas de color por HDMA que dejan opacos los objetos (`src/objetos/objetos_sprite.c`) y los efectos de los apoyos (mosaico de Rex, temblor de Fenrir). Siguen sin unidad: ver «Sin unidad todavía» en `aprendizaje/README.md`.

| Lo que dice la ficha / la spec | Estado hoy |
|---|---|
| Ventanas 1 y 2 por HDMA (tajos de luz en diagonal) | no existe en el combate (la ventana por HDMA sí, en el brillo del logo: unidad 12) |
| Destello blanco del apoyo con el color fijo | no existe (el del ejercicio es el primero) |
| Mosaico pulsante y ondulación del scroll de BG2 | no existe |
| Temblor de pantalla (scroll ±2 px en el VBlank) | no existe |
| Congelar la acción durante el apoyo | existe algo parecido: la pausa y la parada de impacto congelan el mundo |
| `apoyos.c`, ampliar `efectos.c` a estos efectos | no existe |
| Mode 7 en una escena (ampliación opcional) | no existe |
