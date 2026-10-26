# Unidad 22 — Belt-scroll: x/y/z, el carril y el orden por profundidad

> **Unidad del juego actual.** A diferencia de las unidades 00–20, esta enseña
> sobre el código de la **rama principal** (el beat 'em up de la Revisión 3,
> [`docs/diseno-beat-em-up.md`](../../docs/diseno-beat-em-up.md) §3), no sobre
> el tag `plataformas-r2`. Las cifras se midieron el 2026-10-26 sobre el commit
> `641d7c0`. Las prácticas de PC se compilan desde esta carpeta con `../../src`;
> el ejercicio se hace en un clon (`git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\<nombre>`,
> rama actual), nunca en el árbol principal.
>
> **Esto es un esqueleto en marcha.** Lo que la ficha del plan y la spec
> prometen y todavía **no existe** está marcado como **(llegará)**: el carril
> por columnas dibujado en Tiled, el script `preparar_zona.py`, los
> `zonaN_datos.h`, el cambio de prioridad delante/detrás de un poste y el
> fondo propio de la fase 1 (hoy es el del tutorial del plataformas, provisional).
> Hay una lista de diferencias entre la ficha y el código al final.

**Objetivo:** entender cómo un mundo de 2D parece tener fondo: cómo se coloca
algo en el carril (x, y), cómo salta (z), por qué la sombra se queda en el
suelo, quién tapa a quién cuando dos personajes se pisan, cómo avanza la cámara
por zonas y por qué su scroll se escribe en el VBlank.

**Prerrequisitos:** 01 (punto fijo 12.4, `PF()`, por qué no hay `/` en el
frame), 07 (cámara y scroll), 08 (sprites y OAM), 10 (la gravedad, que aquí se
aplica a z) y 21 (el presupuesto de sprites: 128 entradas de OAM, 32 por línea;
si no la has leído, basta con saber eso). **Práctica:** PC + juego real (en un
clon, con pruebas). **Tiempo orientativo:** dos tardes.

## Conceptos

### Las tres coordenadas: x, y, z

Un beat 'em up (*Final Fight*, *Streets of Rage*) se mueve por un suelo que
tiene **profundidad**. El código lo guarda en la `struct Cuerpo`
(`src/juego/mundo.h`, lógica pura probada en el PC) con tres ejes
(→ glosario: Profundidad (y) y altura (z)):

| Eje | Campo | Tipo | Qué es |
|---|---|---|---|
| **x** | `c->x` | `u16`, píxeles | avance por la fase (centro del cuerpo) |
| **y** | `c->y` | `u16`, píxeles | **profundidad**: la línea de pantalla de los **pies** sobre el suelo; más abajo en la pantalla = más cerca de la cámara |
| **z** | `c->z`, `c->vz` | `s16`, **12.4** | **altura** sobre el suelo (16 = 1 px); 0 = en el suelo |

Dos detalles que no son los de la spec (§3 dice «x ... punto fijo 12.4 como
hoy»): **solo z y vz van en 12.4**. x e y son píxeles enteros que se mueven de
a `PASO_X = 2` y `PASO_Y = 1` por frame; una velocidad fraccionaria hace falta
en el salto (la gravedad cambia `vz` en pasos de media décima de píxel), no al
andar. Y `vz` mira hacia **arriba con signo negativo** (como la `vy` de la
unidad 10) mientras que `z` crece al subir: `z = z − vz`.

**La cuenta que lo une todo** es una resta (docs/diseno-beat-em-up.md §3):

```
pantalla_y del cuerpo = y − z/16        (la sombra: siempre y)
```

La x y la y de un personaje en el escenario son lo mismo que su x e y en el
carril; z solo mueve el dibujo hacia arriba. Un personaje que salta **no cambia
de profundidad**: sus pies siguen en la misma línea de suelo, y por eso la
sombra, que se dibuja en `y` y no en `y − z`, se queda en el suelo y enseña
dónde aterrizará. Por eso también el orden de dibujo (más abajo) se calcula
con `y`, nunca con `y − z`: un enemigo no pasa «por detrás» de otro porque
uno de los dos salte.

### El carril

El **carril** es la franja de suelo por la que se puede andar
(→ glosario: Carril). Hoy es un **rango fijo** de líneas de pantalla para los
pies, igual en toda la fase:

```c
#define CARRIL_Y_MIN 184      /* src/juego/mundo.h */
#define CARRIL_Y_MAX 216
```

`mundo_actualizar` lo hace cumplir al final del frame (paso C: `if (c->y <
CARRIL_Y_MIN) ... else if (c->y > CARRIL_Y_MAX)`). El suelo del fondo empieza
en la 176 (`SUELO_Y`, `nivel.h`) y la pantalla acaba en la 224; el comentario de
`mundo.h` razona «176 + 4 .. 224 − 16» para que el cuerpo entre entero sin
cruzarse con la sombra de la fila de abajo, pero las constantes son 184 y 216
(no averigüé por qué difieren; son cifras «a ajustar jugando»).
**(Llegará)**: la spec (§3 y §7) quiere el carril **por columnas** dibujado en
Tiled (`carril_arriba[col]` y `carril_abajo[col]`, de modo que una calle se
estreche o una escalera se ensanche) y generado por `scripts/preparar_zona.py`
en un `zonaN_datos.h`. Hoy no hay ni los arrays ni el script; los dos extremos
son constantes y no dependen de x.

### El salto es la gravedad de la unidad 10, aplicada a z

La física del salto es la de la unidad 10 con otros números y otro eje. En el
frame del despegue, en el suelo y con B recién pulsada (`mundo.c`, paso B1):

```c
c->vz = (s16)(-SALTO_V0);       /* Hacia arriba: SALTO_V0 = 96 (6 px/frame en 12.4). */
c->z = 1;                       /* Ya no está en el suelo.                            */
```

y cada frame en el aire (paso B4):

```c
c->z = (s16)(c->z - c->vz);     /* vz < 0 → z crece (sube).        */
c->vz = (s16)(c->vz + GRAVEDAD);   /* GRAVEDAD = 8 (0,5 px/frame²) */
if (c->z <= 0) { c->z = 0; c->vz = 0; aire = 0; }   /* Tocó el suelo. */
```

Dos diferencias con la unidad 10, las dos medidas con la demo (parte 1):

1. **El orden:** aquí primero se mueve (`z -= vz`) y después actúa la
   gravedad; en `tarou.c` del plataformas era al revés. Por eso el primer paso
   sube los 96 enteros, no 92, y la altura es la suma `8·(12 + 11 + ... + 1)
   = 624`, más el 1 del despegue: 625/16 = **39 px** (`mundo_altura`, `z >> 4`).
2. **La gravedad:** 8 y no 4. Con el doble, el salto es más corto y rápido:
   apogeo en el cuadro 12, vuelve al suelo en el **26**, 39 px (la unidad 10:
   69 px y 47 frames). El comentario de `mundo.h` y `mundo.c` dice «36 px y 24
   frames»: es una estimación antigua, **manda la medida** (la prueba la acepta
   porque pide 30..40 px y 20..28 frames).

`mundo_actualizar` sabe que está en el aire con `aire = (z > 0 || vz < 0)`
(el `z = 1` del despegue va junto a `vz` en el mismo bloque; no probé si se
podría quitar). Aterrizar es clavar `z = 0` y `vz = 0` en el mismo frame, y
**`mundo_altura` devuelve `z >> 4` sin comprobar el signo** porque z nunca es
negativa (la misma regla que `PF_ENTERO`: solo valores ≥ 0).

En el aire, x **conserva el impulso** (no se cambia de dirección) pero y **sí
se puede mover**, como en *Final Fight* (pasos B2 y B3). Y el estado que sale
al final del frame (`CUERPO_SUBIENDO` si `vz < 0`, `CUERPO_BAJANDO` si no) es
lo que anima `protagonista_sprite.c`.

### La sombra siempre en el suelo

La sombra es otra entidad (otra entrada de la tabla de sprites) dibujada en
`(x, y)`, no en `(x, y − z)`. En `protagonista_sprite_actualizar`:

```c
s->y0 = (u16)(c->y - ALTO_CELDA + 2 - mundo_altura(c));  /* el cuerpo: sube con z */
...
s->y0 = (u16)(c->y - SOMBRA_ALTO);                        /* la sombra: no depende de z */
```

(`ALTO_CELDA` = 64: la celda de Tarou es de 96×64; el `+ 2` hunde los pies 2 px
en la hierba, como en el nivel del tutorial, unidad 20; `SOMBRA_ALTO` = 10: el
dibujo de la sombra, de 32×16, empieza 10 px sobre la línea de los pies.) Con
`y = 200`, la demo imprime cuerpo `y0 = 138` en el suelo y **99** en el apogeo,
y la sombra `y0 = 190` siempre.

La sombra no se pinta opaca: `protagonista_sprite_cargar` configura la
**matemática de color** (`REG_TS = 0x03`, `REG_CGWSEL = 0x02`,
`REG_CGADSUB = 0x50`: media suma de la pantalla secundaria, solo sobre los
sprites de las paletas 4..7, que es la de la sombra), así que oscurece lo que
hay debajo (→ glosario: Matemática de color, unidad 12).

### Quién tapa a quién: el orden de la OAM

Cuando dos sprites de la **misma prioridad** se pisan, la SNES dibuja encima el
de la **entrada más baja de la OAM** (la entrada 0 gana a la 1, la 1 a la 2...;
→ glosario: OAM). No hay un «z-buffer» de sprites: ese orden es el único árbitro
entre ellos. En un beat 'em up tapa el que tiene los **pies más abajo** (está
más cerca de la cámara). Así que cada frame hay que:

1. Ordenar las entidades por la `y` de sus pies, de **mayor a menor**.
2. Escribir sus piezas en la OAM en ese orden: la primera ordenada, a la entrada 0.

El juego lo hace con una tabla de entidades fija (`src/juego/sprites.c`:
`sprites_ent[SPRITES_MAX]`, 16 entradas de 16 bytes, hoy 8 en uso: Tarou, su
sombra, 3 ninjas y 3 sombras), un array de claves `sprites_y[]` y un array de
índices `sprites_orden[]`. **Se ordenan los índices, no las entidades** (una razón evidente: mover
un `u8` es más barato que mover una `SpriteEnt` de 16 bytes). Después
`sprites_dibujar()` (en ensamblador, `sprites_oam.asm`) recorre
`sprites_orden` y escribe todas las piezas de cada entidad en la OAM, una tras
otra, desde la entrada 0.

**Las sombras llevan `y = 0`** (`sprites_y[ENT_SOMBRA] = 0`): son siempre las
últimas, detrás de todo, aunque estén en el suelo junto a un cuerpo. Es un
truco con una sola línea: no hace falta un caso especial en la ordenación.

### Ordenación por inserción, estable

`profundidad_ordenar(u8 *orden, const u16 *y, u8 n)` (`src/juego/profundidad.c`)
es una **ordenación por inserción**: para cada entidad desde la segunda, la
guarda y corre un sitio a la derecha a las de delante que tengan la `y` menor,
hasta encontrar su hueco.

```c
while (j > 0 && y[orden[j - 1]] < y_actual) { orden[j] = orden[j - 1]; j--; }
```

Tres propiedades que importan en este juego:

- **Es casi gratis cuando ya está ordenada.** Cada frame la tabla viene como la
  dejó el anterior, y los personajes se mueven 1 o 2 px: el orden cambia pocas
  veces. Entonces cada entidad hace **una** comparación y no se mueve nada:
  `n − 1` comparaciones en total (7 con 8 entidades). Un quicksort no
  aprovecharía esa ventaja y exigiría recursión o pila.
- **Es estable:** la comparación es `<` **estricta**, así que con la misma `y`
  no se intercambian y queda el orden que traían. Con `<=` dos enemigos a la
  misma altura se cambiarían de sitio un frame sí y otro no, y parpadearían
  (`prueba_profundidad.c`, el caso de los empates).
- **No usa `int`, ni multiplicaciones, ni divisiones** y solo toca `u8`/`u16`.
  Es la regla de la lógica de frame del proyecto.

Se llama **dos veces por frame**: `protagonista_sprite_actualizar` ordena con la
`y` de Tarou nueva y las de los ninjas del frame anterior, y
`ninja_sprite_actualizar` actualiza las `y` de los ninjas y ordena de nuevo. La
segunda es la que vale; la primera es inocua porque cuesta casi nada.

### El bit de prioridad: contra los fondos, no contra otros sprites

Cada entrada de la OAM tiene 2 bits de **prioridad** (`SPRITE_PRIORIDAD(p)` en
`sprites.h`, que es `p << 4`: el byte de atributos es `vhoopppN`). Ese bit
decide el sprite frente a los **fondos**, no frente a otros sprites: entre
sprites manda el índice. En el Mode 1 (el del juego), de delante a atrás el
orden de las capas es, según la documentación de la SNES (no lo medí en
Mesen): `BG3` alto (si `BG3_MODE1_PRIORITY_HIGH`) · sprite 3 · `BG1` alto ·
`BG2` alto · sprite 2 · `BG1` bajo · `BG2` bajo · sprite 1 · `BG3` · sprite 0.

Hoy **todas** las entidades usan `PRIORIDAD 3` (`protagonista_sprite.c`,
`ninja_sprite.c`), así que van por delante de BG1 y BG2 y solo se tapan entre
ellas por el orden de la OAM; el texto del HUD está en BG3 con prioridad alta y
queda por encima de todo. **(Llegará)** el uso de la 2: un poste o un farol
dibujado en BG1 con su bit de prioridad queda **entre** los sprites de
prioridad 2 y 3 (spec §3), así que un personaje que pasa **detrás** del poste
(su `y` es menor que la base del poste) se dibujaría con prioridad 2 y uno que
pasa **delante**, con 3. El fondo actual no tiene ningún objeto así.

### La cámara que solo avanza, con tope por zona

La cámara es un solo número, el scroll horizontal (`u16 camara`, unidad 07).
La del plataformas centraba al protagonista en los dos sentidos
(`camara_para`); la del beat 'em up **solo avanza**, como *Final Fight*
(`src/juego/camara.c`):

```c
u16 camara_avanzar(u16 camara, u16 x_centro, u16 limite);
```

`meta` es donde estaría la cámara si el protagonista quedara en el centro de la
pantalla (`x_centro − 128`, o 0 si x < 128), y se recorta primero a
`CAMARA_MAX = ANCHO_NIVEL − ANCHO_PANTALLA = 2048 − 256 = 1792` y luego a
`limite`. Si `meta <= camara`, no se mueve: **nunca retrocede**. Si no, avanza
**a lo sumo `CAMARA_PASO = 2`** px por frame (el mismo `PASO_X` que Tarou:
`fondo_nivel_actualizar` sube una columna de 8 px cada vez que la cámara cruza
una, y a más de 2 px por frame se saltaría alguna, y la cámara no puede
adelantarse al protagonista). Las nueve comprobaciones de `camara_avanzar` en `prueba_camara.c`
(`camara_avanzar(0, 129, ...) == 1`, `(255, 1000, 256) == 256`...) son la
tabla completa de la regla.

El **tope** (`limite`) lo pone la zona en curso (`src/juego/oleadas.c`, lógica
pura): una máquina de dos estados por zona, **libre** y **bloqueada**.

```c
static const u16 ZONA_CAMARA[OLEADAS_ZONAS] = {256, 640, 1024, 1536};   /* provisional */
u16 oleadas_limite(const Oleadas *o);          /* la cámara de la zona en curso, o 0xFFFF */
u8  oleadas_actualizar(Oleadas *o, u16 camara);/* ¿llegó? bloquea y devuelve 1 (una vez)  */
void oleadas_vencer(Oleadas *o);               /* libre otra vez y pasa a la siguiente    */
```

Mientras la oleada está viva, `oleadas_limite` devuelve la cámara de esa zona y
`camara_avanzar` la deja ahí aunque Tarou camine más allá. Al vencerla
(`oleadas_vencer`), el límite pasa a la zona siguiente (o a `0xFFFF` tras la
cuarta: «sin límite, solo manda el final del nivel»). Los cuatro puntos son
**provisionales** (se moverán al dibujar la fase en Tiled: objetos `oleada`,
spec §7).

Para que Tarou no se salga de la pantalla ni vuelva atrás de lo recorrido,
`mundo_limitar_x(c, x_min, x_max)` recorta su x a `[camara + 24, camara + 232]`
(`MARGEN_PANTALLA = 24`). Va **antes** de `camara_avanzar` y usa la cámara del
frame anterior; con cámara bloqueada Tarou puede llegar al borde derecho y no
pasarlo, que es el «no puedes seguir» del juego.

### El scroll se escribe en el VBlank

`pantalla_fase1_actualizar` solo **calcula** `camara`. La PPU no ve ese número
hasta que `pantalla_fase1_vblank` lo escribe, justo después del VBlank:

```c
bgSetScroll(0, camara, 0);        /* la capa cercana */
bgSetScroll(1, camara >> 1, 0);   /* la lejana, a la mitad (parallax) */
dma_lista_ejecutar();             /* las columnas nuevas, la OAM... */
```

La razón es que `bgSetScroll` escribe **en el acto** en los registros de la
PPU (unidad 07): hecha en mitad del dibujo partiría la imagen en dos
desplazamientos. Escribir el scroll, las columnas del fondo y la OAM en el
mismo VBlank hace que lo que se ve en un cuadro sea coherente. `camara >> 1`
es un desplazamiento, no una división.

### El movimiento con el combo bloqueando el cuerpo

Mientras hay un golpe en curso (`c->golpe != 0`) el cuerpo **no anda, no
cambia de carril y no salta**: `mundo_actualizar` solo avanza el golpe
(`paso_golpe`) y todo el bloque B2..B4 va dentro de `if (c->golpe == 0)`. Los
golpes empiezan solo en el suelo (`!aire && m->atacar_pulsado`); no hay ataque
aéreo todavía. Si un golpe recibido corta el combo (`mundo_recibir`), pone
`z = 0` y `vz = 0`: cae al suelo. Y aturdido o KO (`stun != 0 || vida == 0`)
`mundo_actualizar` sale antes de nada: ni mando, ni salto. Esas reglas son de
la unidad 23; aquí basta ver que **el orden de las ramas de `mundo_actualizar`
es la prioridad entre acciones**: aturdido > golpe en curso > empezar golpe o
saltar > moverse.

## El orden de un frame

Lo que hace `pantalla_fase1_actualizar` (`src/pantallas/pantalla_fase1.c`), en
orden, y lo que cambia en cada paso:

```
 1. dma_lista_vaciar()               la lista de copias a VRAM de este frame
 2. mando_leer(&mando)               qué pide el jugador
 3. mundo_actualizar(&cuerpo, &mando)    x, y, z, estado (lógica pura)
 4. mundo_limitar_x(...)             sin salir de [camara+24, camara+232]
 5. camara_avanzar(camara, cuerpo.x, oleadas_limite(&oleadas))
 6. oleadas_actualizar(...)          ¿llegó al punto?  → sacar los ninjas
 7. combate, IA de los ninjas, vencer la oleada si no queda ninguno
 8. protagonista_sprite_actualizar(&cuerpo, camara)   x0,y0 de cuerpo y sombra; sprites_ordenar()
 9. ninja_sprite_actualizar(ninjas, camara)           y de los ninjas; sprites_ordenar() otra vez
10. fondo_nivel_actualizar(camara)   las columnas que entran, a la lista
11. sprites_dibujar()                UNA llamada: toda la OAM, en orden
(NMI) pantalla_fase1_vblank:         bgSetScroll ×2 + dma_lista_ejecutar()
```

Los pasos 3..7 son lógica pura (las pruebas de PC los ejercitan); 8..11
preparan lo que ve la PPU y el VBlank se lo entrega. Durante la **parada de
impacto** (unidad 23) los pasos 3..7 se saltan y 8..11 siguen: el mundo se
congela pero se sigue dibujando. Que x e y sean
píxeles de mundo y la x de pantalla sea `x − camara` permite que la misma
`camara` sirva a la lógica, al sprite y al scroll.

## La demo: `demo_22_belt.c`

Compila el código real (`mundo.c`, `profundidad.c`, `camara.c`, `oleadas.c`) con
`-DPRUEBAS_PC` y lo usa en cuatro partes:

```
cd E:\ProyectoSNES\aprendizaje\unidad-22-belt-scroll
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_22.exe demo_22_belt.c ../../src/juego/mundo.c ../../src/juego/profundidad.c ../../src/juego/camara.c ../../src/juego/oleadas.c
.\demo_22.exe
```

Compila sin avisos; la salida completa está en `capturas/salida_demo_22_belt.txt`.

**Parte 1 — el salto en z.** Un `Cuerpo` real en (120, 200) salta. Resumen de
la tabla (`z` en 12.4):

```
frame      z altura     vz    y    cuerpo y0    sombra y0  estado
    1     97      6    -88  200          132          190  SUBIENDO
   12    625     39      0  200           99          190  BAJANDO     <- apogeo
   25      1      0    104  200          138          190  BAJANDO
   26      0      0      0  200          138          190  PARADO
Apogeo en el frame 12 (z = 625 = 39 px), aterriza en el frame 26.
```

Mira la columna `y` (no cambia nunca), `cuerpo y0` (sube 39 px y baja) y
`sombra y0` (fija en 190 = 200 − 10). El cuerpo vuelve a 138 al aterrizar. `z`
pasa por 625 dos veces (cuadros 12 y 13): es el tope de la parábola.

**Parte 2 — el orden de la OAM, cuadro a cuadro.** Las 8 entidades del juego
(Tarou, su sombra, tres ninjas con y = 190, 204 y 204 y sus sombras) y Tarou
bajando un píxel por frame desde y = 184. Se ve cómo Tarou **entra en el orden**
entre los ninjas, y cuánto cuesta:

```
frame  yTar  orden de la OAM (delante -> detras)                  comp. mov.
    0   184  Nin2 Nin3 Nin1 Tarou sTarou sNin1 sNin2 sNin3    13    8   <- cambia el orden
    4   188  Nin2 Nin3 Nin1 Tarou sTarou sNin1 sNin2 sNin3     7    0
    7   191  Nin2 Nin3 Tarou Nin1 sTarou sNin1 sNin2 sNin3     8    1   <- cambia el orden
   21   205  Tarou Nin2 Nin3 Nin1 sTarou sNin1 sNin2 sNin3     8    2   <- cambia el orden
En 35 cuadros: 253 comparaciones y 11 movidas en total
```

El cuadro 0 es el caro (13 comparaciones, 8 movidas) porque parte del orden
`0..7` de `sprites_usar`; después casi todos los cuadros hacen las 7
comparaciones mínimas y ninguna movida. Mira también el empate: `Nin2` y `Nin3`
(204 los dos) **siempre** salen en el mismo orden relativo (estable), y Tarou
pasa a delante de los dos a la vez cuando llega a y = 205. La demo comprueba que
una copia instrumentada del algoritmo (la que cuenta) da el mismo orden que la
real. Los tamaños de pieza no aparecen: la demo solo ordena entidades.

**Parte 3 — quién tapa a quién.** Un dibujo de caracteres (T Tarou, N ninja, s
sombra) donde gana, en cada punto, la **entrada más baja** de la tabla:

```
b) Tarou detrás (y 180 < 190), SIN ordenar     c) el mismo caso, ORDENADO
    ..NNNNTTTTTTTT..                               ..NNNNNNNNTTTT..
    ..NNssTTTTTTTTss                               ..NNNNNNNNTTTTss
    ..NNNNNNNN......                               ..NNNNNNNN......
```

En **b)** la tabla sin ordenar mantiene a Tarou en la entrada 0: tapa al ninja
aunque esté más arriba en el carril (más lejos), que es el error visible que
evita la ordenación. En **c)** el ninja (y 190 > 180) va primero y tapa a Tarou.
En todos los casos la sombra `s` queda detrás de los dos cuerpos.

**Parte 4 — la cámara con tope.** Tarou camina a la derecha con el orden de
`pantalla_fase1.c` (pasos 3 a 6) y en el cuadro 260 «se vence la oleada»:

```
frame  Tar x camara  zona  bloq  limite de la camara
    1     66      0     0     0  0x0100
  160    384    256     0     1  0x0100      <- bloqueada en 256
  240    488    256     0     1  0x0100      <- Tarou al borde (256 + 232)
  261    488    258     1     0  0x0280   <- oleada vencida, la camara se libera
  452    870    640     1     1  0x0280      <- bloqueada otra vez, zona 1
```

La cámara no se mueve los primeros 33 frames (Tarou arranca en x = 64 y la
cámara espera a que pase de 128), le sigue exactamente a 2 px/frame (la misma
velocidad), se para en 256 y Tarou se queda con x = 488 = 256 + 256 − 24. Al
vencerla, `zona` pasa a 1, el límite a 640 (0x0280) y la cámara reanuda.

## La librería: PVSnesLib

La lógica de esta unidad (`mundo.c`, `profundidad.c`, `camara.c`, `oleadas.c`)
es C puro y casi no llama a la librería. Las funciones y macros que sí aparecen
en el código que se lee (cabeceras en
`tools/pvsneslib/pvsneslib/include/snes/`; documentación en
<https://alekmaul.github.io/pvsneslib/>):

| Función / macro (firma exacta) | Qué hace | Cabecera | Dónde la usa el juego |
|---|---|---|---|
| `void bgSetScroll(u8 bgNumber, u16 x, u16 y);` | escribe el scroll de un fondo **en el acto** (BGnHOFS / BGnVOFS) | [`background.h`](https://alekmaul.github.io/pvsneslib/background_8h.html) | `pantalla_fase1.c`, `pantalla_fase1_vblank` |
| `void oamSetXY(u16 id, u16 xspr, u16 yspr);` | solo la posición de una entrada de la OAM | [`sprite.h`](https://alekmaul.github.io/pvsneslib/sprite_8h.html) | `sprites.c`, `sprites_iniciar` (las esconde en y = 224) |
| `void oamSetEx(u16 id, u8 size, u8 hide);` | tamaño (`OBJ_SMALL`/`OBJ_LARGE`) y visibilidad (`OBJ_SHOW`/`OBJ_HIDE`) | `sprite.h` | `sprites.c`, `sprites_iniciar` |
| `void oamInitGfxAttr(u16 address, u8 oamsize);` | base de los tiles de sprite y tamaños (`OBJ_SIZE16_L32` = pequeño 16×16, grande 32×32) | `sprite.h` | `protagonista_sprite.c`, `protagonista_sprite_cargar` |
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` | DMA a la CGRAM desde el color `address` (paleta de la sombra: `128 + 64`) | [`dma.h`](https://alekmaul.github.io/pvsneslib/dma_8h.html) | `protagonista_sprite.c` |
| `void dmaCopyVram(u8 *source, u16 address, u16 size);` | DMA a la VRAM (los tiles de la sombra) | `dma.h` | `protagonista_sprite.c` |
| `#define REG_TS (*(vuint8 *)0x212D)` | pantalla secundaria (qué capas) | `background.h` | `protagonista_sprite.c` |
| `#define REG_CGWSEL (*(vuint8 *)0x2130)`, `#define REG_CGADSUB (*(vuint8 *)0x2131)` | operando de la matemática de color y su operación (la media suma de la sombra) | [`video.h`](https://alekmaul.github.io/pvsneslib/video_8h.html) | `protagonista_sprite.c` |

Regla del proyecto: **primero la librería, después el registro**. Aquí se baja
al registro (`REG_TS`, `REG_CGWSEL`, `REG_CGADSUB`) porque la librería solo trae
`setColorEffect` (que escribe `CGWSEL`/`CGADSUB` pero no `TS`); lo mismo hace
`src/escena/efectos.c` (unidad 12). Las dos funciones de la OAM, y
`sprites_dibujar` en ensamblador, son la ruta rápida de la capa de sprites de la
unidad 21; la forma de escribir una entrada «a mano» con `oamSet`, con todo el
coste, es la de la unidad 08. El ejemplo del SDK equivalente de la unidad 08
es `snes-examples/graphics/Sprites/SimpleSprite`.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| `struct Cuerpo`, `CARRIL_Y_MIN/MAX`, `PASO_X/Y`, `SALTO_V0`, `GRAVEDAD`, `MARGEN_PANTALLA` | `src/juego/mundo.h` |
| `mundo_actualizar`: pasos B1..B4 (salto, x, y, gravedad) y C (límites y estado) | `src/juego/mundo.c` |
| `mundo_altura` (`z >> 4`), `mundo_limitar_x` | `src/juego/mundo.c` |
| `profundidad_ordenar` | `src/juego/profundidad.c` |
| `sprites_y`, `sprites_orden`, `sprites_ordenar`, `SPRITE_PRIORIDAD` | `src/juego/sprites.c` / `.h` |
| `sprites_dibujar` (la OAM en orden, en ensamblador) | `src/juego/sprites_oam.asm` |
| El cuerpo en `y − z`, la sombra con `y = 0` en el orden, la media suma de color | `src/juego/protagonista_sprite.c`, `_actualizar` y `_cargar` |
| Los ninjas: `sprites_y[ENT_NINJA + k] = p->y` y la segunda ordenación | `src/enemigos/ninja_sprite.c` |
| `camara_avanzar`, `CAMARA_PASO`, `CAMARA_MAX` | `src/juego/camara.c` / `.h` |
| La zona: `ZONA_CAMARA`, `oleadas_limite`, `_actualizar`, `_vencer` | `src/juego/oleadas.c` / `.h` |
| El orden del frame y el scroll en el VBlank | `src/pantallas/pantalla_fase1.c`: `_actualizar` y `_vblank` |
| Las cuentas, en pruebas | `tests/prueba_mundo.c`, `prueba_profundidad.c`, `prueba_camara.c`, `prueba_oleadas.c` |
| La spec | `docs/diseno-beat-em-up.md` §3 |

## Trampas

- **La profundidad se ordena con `y`, no con `y − z`.** Un personaje en el aire
  no pasa por detrás de otro ni lo tapa distinto: lo que sube es el dibujo.
- **Las sombras llevan `y = 0`, no «la y de su dueño».** Con su dueño, la
  sombra podría quedar por delante de otro cuerpo; con 0 está siempre al fondo.
- **`<` y no `<=`** en `profundidad_ordenar`: con `<=` se pierde la estabilidad
  y los empates parpadean.
- **El bit de prioridad no ordena sprites entre sí.** Para eso es el índice de
  la OAM; la prioridad es solo contra los fondos.
- **`mundo_altura` no vale si `z` fuese negativa.** Hoy no lo es (aterriza en
  `z <= 0 → 0`); un salto que se pasara por debajo daría un número enorme.
- **Las pruebas `COMPROBAR(c.vz == vz_antes + GRAVEDAD)`** (`prueba_mundo.c`,
  «en el aire no se salta otra vez») dependen de que **`m.saltar` esté a 0** en
  esa prueba. Cualquier regla nueva que lea B mantenida debe mantener esa prueba
  en verde (es justo lo que enseña el ejercicio).
- **`camara_avanzar` recibe el centro** (`cuerpo.x`), no el borde de la celda.
  La del plataformas (`camara_para`) recibía `x_pix` del borde y le sumaba 48.

## Ejercicio y criterio de aprobación

`ejercicio_22.md`: **salto de altura variable**. Con TDD, en un clon: una
prueba nueva en `tests/prueba_mundo.c` que exige que mantener B mientras sube
llegue a 48..56 px sin tocar el salto corto; verla fallar (2 fallos); cambiar
una línea de `mundo.c` y una constante de `mundo.h`; verla pasar; medir; y
comprobar que la ROM compila. **Aprobación:** `563 pruebas, 0 fallos` (las 560
de hoy más tres nuevas), y la medida `mantener=1 altura=51 cuadros=31`.

El resultado de referencia está en `solucion/ejercicio_22.patch` y en
`capturas/` (`pruebas_rojo.txt`, `pruebas_solucion.txt`,
`medida_salto_original.txt`, `medida_salto_solucion.txt`,
`compilacion_rom_clon.txt`).

## Diferencias entre la ficha del plan y el código

La ficha de la unidad 22 (`PLAN.md` §3) se escribió antes del código; lo que
manda es el código:

| La ficha / la spec dice | El código es |
|---|---|
| carril por columnas, `carril_arriba[col]`, `carril_abajo[col]`, de Tiled | un rango fijo `CARRIL_Y_MIN 184` .. `CARRIL_Y_MAX 216` (**llegará** lo de Tiled) |
| `scripts/preparar_zona.py` y `zonaN_datos.h` | no existen; las zonas están en `oleadas.c` (`ZONA_CAMARA`, provisional) |
| «x ... punto fijo 12.4 como hoy» (spec §3) | x e y en píxeles enteros; solo `z` y `vz` en 12.4 |
| regla de golpe `\|y_a − y_b\| ≤ TOLERANCIA_Y` (~8) | `COMBATE_PROFUNDIDAD` = 14 en `combate.c` (se estudia en la unidad 23) |
| ordenación de «≤ 8 entidades» | `SPRITES_MAX` = 16; hoy se usan 8 |
| prioridad 2 / 3 para pasar detrás/delante de un poste | todos usan prioridad 3; **llegará** con el fondo propio |
| «`camara.c` ... se le añade el bloqueo de zona» | el bloqueo no está en `camara.c`: `camara_avanzar` recibe un `limite` y lo calcula `oleadas.c` |
| `protagonista_sprite.c` sucesor de `tarou_sprite.c` | sí existe; el cuerpo usa la animación de `ANIMACIONES` de Tarou |
| fondo de la fase | el del tutorial del plataformas (2048 px), provisional |
| salto «de 36 px y 24 frames» (comentario de `mundo.h`) | medido: 39 px, apogeo en el cuadro 12, 26 cuadros hasta aterrizar |

## Lo que viene

La unidad 23 usa esta `y` para decidir si un golpe conecta (`COMBATE_PROFUNDIDAD`)
y esta `Cuerpo` para el combo, la parada de impacto y el daño. La 24 pone a los
ninjas en las zonas de `oleadas.c`.
