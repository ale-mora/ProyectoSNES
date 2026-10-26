# Unidad 23 — Combate: estados que se afectan, cajas de golpe y daño

> **Código real, no previsto.** Esta unidad se escribió el 2026-10-26 contra
> el commit `641d7c0` de la rama `demo-secuencia-arranque` (560 pruebas de PC
> en verde). La ficha del plan (`PLAN.md` §3, «Unidad 23») se redactó antes de
> que existiera el código y prometía más de lo que hoy hay: **manda el código**.
> La sección «Lo que la ficha prometía y todavía no existe» lista, una por una,
> las diferencias; lo que está «por venir» no se enseña como hecho.

**Objetivo:** entender cómo se resuelve un golpe en el beat 'em up: el combo de
Tarou como máquina de estados (golpe, duración, cuadros activos, ventana de
encadenado), la caja de golpe en el suelo con tolerancia de profundidad, cómo el
daño de un luchador **cambia el estado de otro** (resuelto en un módulo aparte,
en los dos sentidos), y lo que da contundencia sin gastar el frame: la parada de
impacto, el buffer de entrada y la invulnerabilidad con parpadeo.

**Prerrequisitos:** 02 (máquinas de estado), 11 (animación: qué cuadro se ve en
cada estado), 20 (la AABB `se_tocan`, de la que la caja de golpe es pariente) —
las tres enseñan código del tag `plataformas-r2`; el concepto vale igual — y
22 (el cuerpo x/y/z de `mundo.c`; **la unidad 22 no estaba escrita cuando se
redactó esta**, así que la parte del cuerpo que necesitas se explica aquí). **Práctica:** PC (la
demo, compilada contra los `.c` reales) + juego real en un clon (TDD con
`tests/prueba_combate.c`). **Tiempo orientativo:** dos tardes. **No hay
mini-ROM**: toda la lógica de combate es pura (sin `snes.h`); lo que se ve en
pantalla (el parpadeo) es de las unidades de sprites.

## Conceptos

- **Luchador** — cualquier cosa que pega y recibe: Tarou (`Cuerpo`,
  `src/juego/mundo.h`) y el ninja (`Ninja`, `src/enemigos/ninja.h`). Cada uno
  tiene su máquina de estados; Bárbara será otro `Cuerpo` (por venir).
- **Tajo** — un golpe entero, de que empieza a que acaba. El golpe 1 de Tarou
  es un tajo; el golpe 2, otro. El ninja da un tajo de shinai por ataque.
- **Cuadro activo** — el cuadro en que la caja de golpe **existe**. Un tajo dura
  14 cuadros pero solo 4 (de `reloj` 4 a 7) pueden hacer daño; el resto es
  preparación y recuperación. → glosario: Caja de golpe.
- **Caja de golpe** (*hitbox*, `combate_caja`) — rectángulo **en el suelo**
  delante del luchador, no en la pantalla: de `COMBATE_ATRAS` px detrás de los
  pies a `alcance` px delante (hacia donde mira), y una tolerancia en y
  (`profundidad`): el golpe solo alcanza a quien esté en un carril cercano. Es
  la AABB de la unidad 20 reducida a un eje (x) más una comparación (y), y no
  hay altura (z): ver la parte B del ejercicio.
- **Combo** — golpes encadenados con A. En el código, un **campo** (`golpe`,
  1..`GOLPES`) dentro del estado `CUERPO_ATACANDO`, no tres estados distintos.
  → glosario: Combo.
- **Ventana de encadenado y cola** — a partir de `GOLPE_ENCADENAR_DESDE` (cuadro
  5) de un golpe, una A recién pulsada se **recuerda** en `c->cola`; cuando el
  golpe acaba, si hay cola, el combo pasa al siguiente; si no, se corta. La
  cola es una cola de **un** elemento: un `u8` que vale 0 o 1.
- **`id` del tajo** — un número (`Cuerpo.id`, 1..254) que cambia en cada golpe.
  Cada ninja guarda el último que lo golpeó (`Ninja.id_golpe`) y rechaza el
  mismo: los 4 cuadros activos de un tajo pegan **una** vez.
- **Derribo** — el golpe que deja al objetivo en el suelo (`NINJA_DERRIBADO`,
  `CUERPO_DERRIBADO`): cae sin poder actuar y, si no le queda vida, desaparece
  (ninja) o se queda en el suelo (Tarou, KO). El último del combo derriba.
- **Aturdido** (`RECIBIR`) — el estado del que recibe un golpe normal: no actúa,
  retrocede un poco y se recupera solo.
- **Un luchador cambia el estado de otro** — el golpe que conecta no lo decide
  ni el atacante ni el golpeado: lo decide `combate.c`, que mira a los dos y
  llama a `ninja_recibir` o a `mundo_recibir`. Los dos módulos de luchador no se
  incluyen entre sí (`mundo.h` no sabe nada de `ninja.h`), solo `combate.h`
  incluye ambos.
- **Invulnerabilidad** (`Cuerpo.inv`) — cuadros durante los que `mundo_recibir`
  devuelve 0. Se dibuja **parpadeando**.
- **Parada de impacto** (*hit-stop*) — al conectar, el mundo se congela unos
  cuadros (se sigue dibujando). Da peso al golpe sin costar nada. → glosario:
  Parada de impacto.
- **Buffer de entrada** — recordar una pulsación hecha cuando no se podía
  atender, para atenderla en cuanto se pueda. Aquí: una A pulsada durante la
  parada (`a_guardada`).
- **KO** — Tarou sin vida: `CUERPO_DERRIBADO` para siempre; el juego vuelve al
  título 150 cuadros después de que acabe su caída, cuando `stun` llega a 0 (provisional).

## Cómo funciona

### 1. El combo de Tarou: una máquina de estados dentro de un estado

`mundo_actualizar` (`src/juego/mundo.c`) tiene tres ramas por cuadro, en este
orden:

1. **Aturdido o KO** (`stun != 0` o `vida == 0`): `paso_aturdido` desliza al
   cuerpo, gasta `stun` y no deja pasar el mando.
2. **Golpe en curso** (`golpe != 0`): solo corre `paso_golpe`; ni anda ni salta
   ni cambia de carril (la prueba «Quieto mientras golpea» lo fija).
3. **Libre**: en el suelo, `A` recién pulsada pone `golpe = 1`, `reloj = 0`,
   `cola = 0` y un `id` nuevo; si no, B salta, etc.

Las constantes del combo (`mundo.h`):

| Constante | Valor | Qué es |
|---|---|---|
| `GOLPES` | 3 | golpes del combo |
| `GOLPE_FRAMES` | 14 | duración de los golpes 1 y 2 |
| `GOLPE_ULTIMO_FRAMES` | 20 | el último tarda más en recuperarse |
| `GOLPE_ACTIVO_DESDE` / `_HASTA` | 4 / 8 | caja activa con `reloj` en 4..7 (4 cuadros) |
| `GOLPE_ENCADENAR_DESDE` | 5 | desde aquí, una A recién pulsada entra en la cola |

`paso_golpe` es corta y es toda la máquina:

```c
c->reloj++;
if (m->atacar_pulsado && c->reloj >= GOLPE_ENCADENAR_DESDE && c->golpe < GOLPES)
{
    c->cola = 1;                       /* A a tiempo: encadena al acabar. */
}
if (c->reloj >= duracion)              /* Acabó este golpe. */
{
    if (c->cola) { c->golpe++; c->id = nuevo_id(c->id); }   /* siguiente tajo */
    else         { c->golpe = 0; }                          /* el combo se corta */
    c->reloj = 0;
    c->cola = 0;
}
```

Léela con calma: (a) la A se mira **antes** de comprobar si el golpe ha
acabado, así que la pulsación del cuadro en que `reloj` llega a la duración
todavía cuenta (por el código; no hay una prueba que lo fije); (b) `c->golpe <
GOLPES` hace que en el último golpe no entre nada en la cola: **no hay quinto
golpe** aunque se machaque A; (c) el `id` solo cambia al **encadenar**, y
`nuevo_id` lo mantiene en 1..254 (el 0 es el de un ninja recién nacido y el 255
no se usa); (d) una A pulsada demasiado pronto (antes del cuadro 5) se pierde:
no se guarda en ningún sitio.

La ventana del jugador es, pues, de unos 9–10 cuadros de cada golpe de 14
(`reloj` 5 hasta el cambio de golpe). `mundo_golpe_activo` resume si la caja
existe ahora:

```c
return (u8)(c->golpe != 0 && c->reloj >= GOLPE_ACTIVO_DESDE && c->reloj < GOLPE_ACTIVO_HASTA);
```

Atención al orden del cuadro del primer golpe (la demo lo imprime): el cuadro en
que se pulsa A es el cuadro 0 (`reloj = 0`, ya con `golpe = 1`), de modo que la
caja se activa el 5.º cuadro, cuando `reloj` vale 4.

### 2. La caja de golpe: `combate_caja`

`src/juego/combate.c`, la función más corta y la que más se parece a la unidad
20:

```c
u8 combate_caja(u16 ax, u16 ay, u8 mira_izq, u16 alcance, u16 profundidad, u16 tx, u16 ty);
```

`(ax, ay)` son los pies del que ataca; `(tx, ty)`, los del objetivo. Primero
la profundidad (`|ty − ay| ≤ profundidad`, con dos comparaciones en vez de un
valor absoluto: nada de funciones en el frame), después x: «delante» es hacia
donde mira, y el objetivo cuenta desde `COMBATE_ATRAS` (8 px) detrás hasta
`alcance` delante. La demo la dibuja:

```
Tarou mirando a la derecha (alcance 52, profundidad 14):
      dx: -16 -12 -8 -4  0  4  8 12 16 20 24 28 32 36 40 44 48 52 56 60
  dy -16    .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
  dy -12    .   .   #   #   #   #   #   #   #   #   #   #   #   #   #   #   #   #   .   .
  dy  +0    .   .   #   #   @   #   #   #   #   #   #   #   #   #   #   #   #   #   .   .
  dy +16    .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
```

(El dibujo de 4 en 4 px no llega a la fila `dy = ±14`, que sí está dentro; la
prueba `combate_en_caja(&c, 440, 200 + COMBATE_PROFUNDIDAD)` fija ese borde.)
Las dos cajas del juego comparten función con distintas constantes:

| | alcance | profundidad | dónde |
|---|---|---|---|
| Tarou | `COMBATE_ALCANCE` 52 | `COMBATE_PROFUNDIDAD` 14 | `combate.h` |
| Ninja | `NINJA_ALCANCE` 48 | `NINJA_PROFUNDIDAD` 12 | `ninja.h` |

`combate_en_caja(c, tx, ty)` es la versión de Tarou (usa su `x`, `y`,
`mira_izq` y las constantes de `combate.h`).

### 3. Tarou → ninjas: `combate_resolver`

Se llama **una vez por cuadro**. Si Tarou no está en un cuadro activo
(`!mundo_golpe_activo`), no hace nada. Si lo está, recorre las `NINJAS_MAX`
ranuras **con un puntero que avanza** (`n++`, sin `n[k]`: `Ninja` mide 16 bytes,
potencia de 2, pero el patrón es el del proyecto) y a cada ninja vivo dentro de
la caja le llama a:

```c
ninja_recibir(n, combate_danio(c->golpe), derribo, c->mira_izq, c->id)
```

`ninja_recibir` (`src/enemigos/ninja.c`) es donde **cambia el estado del otro**:

- no hace nada (devuelve 0) si el ninja no está vivo, si ya lo golpeó **este**
  `id`, o si está `NINJA_DERRIBADO` (un caído no se vuelve a golpear: no hay
  «malabares» en el aire);
- guarda el `id`, le resta el daño a `vida` (sin bajar de 0), lo hace **mirar
  hacia quien le pegó** y lo empuja hacia donde iba el golpe;
- si `derribo` o se quedó sin vida: `NINJA_DERRIBADO`, 36 cuadros, devuelve 1;
- si no: `NINJA_RECIBIR`, 16 cuadros, devuelve 0. Un aturdido **sí** puede
  recibir un golpe nuevo (otro `id`) y entonces vuelve a empezar su aturdimiento
  — también un ninja en pleno tajo: el golpe de Tarou lo **cancela** (las
  pruebas «Un golpe recibido cancela el tajo»).

Daño y derribo salen de dos funciones de una línea (`combate.c`):

```c
combate_danio(golpe)   = golpe >= GOLPES ? 2 : 1      /* 1, 1, 2 */
combate_derriba(golpe) = golpe >= GOLPES              /* solo el último */
```

con `NINJA_VIDA 4` el combo completo (1 + 1 + 2) mata al ninja **justo** (la
prueba «El combo completo mata a un ninja»). Fíjate en las dos constantes de
cuadros de parada que devuelve `combate_resolver`: 3 (`PARADA_GOLPE`) si el
golpe es nuevo y aturde, 5 (`PARADA_DERRIBO`) si derribó o remató. El `else if`
tiene una guarda (`parada == 0`) para que un golpe normal no rebaje una parada de
derribo ya decidida por otro ninja del mismo cuadro. Y `antes != c->id &&
n->id_golpe == c->id` es el truco para saber si **este** cuadro es el que
conectó (los otros tres cuadros activos del mismo tajo devuelven 0: sin
parada repetida).

### 4. Ninjas → Tarou: el mismo diseño al revés

El ataque del ninja es otra máquina de estados pequeña (`ninja.c`):

- `ninja_intentar_atacar` (después de `ninja_actualizar`): si hay turno libre
  (`turnos < NINJA_TURNOS` = 2 a la vez), el ninja ya está dentro de la pantalla,
  `NINJA_PARADO`, descansado (`espera == 0`) y cerca (`dx ≤ NINJA_DISTANCIA + 8`
  = 48, `dy ≤ 8`), pasa a `NINJA_ATACANDO` con `timer = 0` y `conecto = 0`.
- el tajo dura `NINJA_ATAQUE_FRAMES` = 28 cuadros; su caja está activa con
  `timer` en 12..19 (`NINJA_GOLPE_DESDE`..`HASTA`, 8 cuadros — el doble que la
  de Tarou) y mientras no haya acertado (`ninja_golpe_activo` mira `!conecto`);
  al acabar descansa `NINJA_ESPERA` = 50 cuadros.

`combate_ninjas_resolver(n, c)`: para cada ninja con el golpe activo y Tarou
dentro de **su** caja, `conecto = 1` y `mundo_recibir(c, NINJA_DANIO, n->mira_izq)`.
Dos detalles de diseño que las pruebas fijan:

- `conecto` se pone **aunque `mundo_recibir` devuelva 0** (Tarou invulnerable):
  el tajo se pierde, no «espera» a que termine la invulnerabilidad. Si no, un
  Tarou parpadeando recibiría el tajo en cuanto dejara de parpadear.
- el empuje va **hacia donde mira el ninja** (`n->mira_izq`), y una prueba
  comprueba que un ninja que ataca de espaldas no acierta.

`mundo_recibir` (`mundo.c`) es la pareja de `ninja_recibir` para Tarou:

| Condición | Efecto |
|---|---|
| `inv != 0` o `vida == 0` | devuelve 0, no pasa nada |
| si no | `vida` baja (sin pasar de 0); el golpe que diera se corta (`golpe = reloj = cola = 0`); cae al suelo (`z = vz = 0`); `empuje` 1 (izquierda) o 2 (derecha); `inv = INVULNERABLE_FRAMES` (60) |
| `vida == 0` | `CUERPO_DERRIBADO`, `stun = DERRIBO_FRAMES` (36) |
| `vida > 0` | `CUERPO_RECIBIR`, `stun = RECIBIR_FRAMES` (16) |

Devuelve 1, y `combate_ninjas_resolver` convierte ese 1 en 3 cuadros de parada o
5 si `vida == 0`. Con `vida == 0` Tarou queda `CUERPO_DERRIBADO` y la rama 1 de
`mundo_actualizar` ya no le deja hacer nada, por muy fuerte que se machaque A
(la prueba lo hace durante 200 cuadros).

### 5. Invulnerabilidad y parpadeo (por falta de arte)

`mundo_actualizar` gasta `inv` de uno en uno cada cuadro. Mientras queda,
`protagonista_sprite.c` oculta el cuerpo en los cuadros con el bit 1 puesto:

```c
if (c->inv != 0 && (c->inv & 2)) { s->y0 = 240; s->borde = 1; }   /* y >= 224: no se dibuja */
```

dos cuadros fuera, dos dentro, y no hay arte nuevo que dibujar. El ninja hace
lo mismo en `ninja_sprite.c` mientras está aturdido o derribado
(`(p->estado == NINJA_RECIBIR || p->estado == NINJA_DERRIBADO) && (p->reloj &
2)`; `aturdido` incrementa `reloj` para eso). Es la solución provisional del
«destello»: el aturdido de Tarou se dibuja con el cuadro de reposo
(`ESTADO_ANIM` de `protagonista_sprite.c`) y sus tres golpes comparten **un solo
cuadro de ataque**, así que el parpadeo es lo único que dice «te han dado».

### 6. Parada de impacto y buffer de entrada: el orden del frame

Todo esto lo orquesta `pantalla_fase1_actualizar` (`src/pantallas/pantalla_fase1.c`).
El orden, simplificado:

```
dma_lista_vaciar; barra de vida; mando_leer; START / SELECT
si parada > 0:
    parada--;  si A recién pulsada → a_guardada = 1          (mundo congelado)
si no:
    si a_guardada → mando.atacar_pulsado = 1; a_guardada = 0
    mundo_actualizar (Tarou) → límites → cámara y oleadas
    parada = combate_resolver(Tarou → ninjas)
    para cada ninja: ninja_actualizar → ninja_intentar_atacar
    golpe_ninja = combate_ninjas_resolver(ninjas → Tarou); parada = el mayor
    KO → temporizador; oleada vencida
sprites: Tarou, ninjas, fondo, OAM         (se dibuja SIEMPRE, también en la parada)
```

El `if (parada)` salta **toda** la lógica: Tarou no avanza (`reloj` del golpe
tampoco), los ninjas no andan, `inv` y `stun` no se gastan, y el KO no cuenta.
Lo único que corre es el mando, el HUD y el dibujo. Esa es la «contundencia»: 3
cuadros (1/20 de segundo) de pausa en cada golpe. Se paga con un `if`.

El buffer arregla un efecto secundario. El golpe 1 conecta en el cuadro 4; el
jugador pulsa A **durante** la parada, es decir, con `reloj` aún en 4, antes de
la ventana (`reloj ≥ 5`). Sin buffer esa A se perdería: la parada se la comió.
Con él, `a_guardada` la recuerda y se inyecta como `atacar_pulsado` en el primer
cuadro después de la parada, cuando `reloj` ya es 5. El buffer es de **un
solo bit y solo para A**: una B (salto) pulsada en la parada se pierde porque
`saltar_pulsado` es un «recién pulsado» de ese cuadro y no se guarda.

### Una observación (medida en la demo, no vista en Mesen)

> **Corregido después en el juego (2026-10-26).** Esta unidad y sus capturas describen el commit
> `641d7c0`. El autor decidió arreglar lo que sigue: `aturdido()` de `ninja.c` ahora retrocede
> 1 px durante 4 cuadros (4 px por golpe, antes 12) y `prueba_combate.c` tiene una prueba
> nueva (el combo completo contra un ninja que camina deja su vida en 0). Para repetir las
> medidas de abajo, haz el clon en `641d7c0`.


La demo (`capturas/salida_demo_23_combate.txt`) usa el código real con el ninja
**libre**, como en el juego: llega andando a 40 px de Tarou y se deja pegar.
Resultado: Tarou da los **tres** golpes, pero solo **dos** cuentan (`vida final
del ninja = 2 de 4`). El motivo está en las constantes: `RECIBIR` retrocede 2 px
por cuadro durante 6 cuadros (12 px); tras el primer golpe el ninja está a 52 px
(justo el `COMBATE_ALCANCE`, aún dentro), tras el segundo a 64, y el tercero
**falla**. La prueba `El combo completo mata a un ninja` no lo ve porque nunca
llama a `ninja_actualizar`. Puede ser lo que se quiere (el combo falla si el
enemigo se aleja: la spec dice «si el golpe falla, el combo se corta»), pero el
comentario de `NINJA_VIDA` («el combo da 1+1+2 = justo») supone que los tres
golpes conectan. Si hay que arreglarlo es decisión del autor; el ejercicio
«para ir más lejos» lo plantea. **No se ha comprobado en una ROM**: las capturas
de Mesen de esta unidad no existen.

## Lo que la ficha prometía y todavía no existe (por venir)

| La ficha decía | Hoy en el código |
|---|---|
| Cajas de daño y de golpe **por cuadro** en una tabla generada por `preparar_personaje.py`, entrada rellena a 8 bytes | **No existe la tabla.** Las cajas son constantes (`COMBATE_ALCANCE`…, `NINJA_ALCANCE`…) en `combate.h` y `ninja.h`, iguales en todos los cuadros. El arte de Tarou tiene **un** cuadro de ataque para los tres golpes. (Lo que sí cumple la regla de tcc816: `Ninja` mide 16 bytes y se recorre con puntero.) |
| La máquina de estados con `GOLPE_n` como estados | `golpe` es un **campo** (1..`GOLPES`) dentro de `CUERPO_ATACANDO`; el cuadro de dibujo no cambia |
| Agarre por contacto sostenido, golpes de agarre, lanzamiento que golpea a otros | **No existen.** Tampoco los estados `AGARRADO` / `LANZADO` |
| Ataque aéreo | **No existe**: `mundo_actualizar` solo empieza un golpe en el suelo |
| Especial A+B que cuesta vida si conecta | **No existe** |
| Destello: paleta blanca copiada a CGRAM por la lista de DMA | **No existe**; se usa el parpadeo (`inv & 2`) |
| Parada de impacto de 3–4 cuadros | **3 y 5** (`PARADA_GOLPE`, `PARADA_DERRIBO`) |
| Invulnerabilidad ~60 cuadros «al levantarse» | 60 cuadros **desde el golpe** (`INVULNERABLE_FRAMES`): incluye los 16 de aturdimiento |
| `RECIBIR`, `DERRIBADO` y `AGARRADO` para ambos luchadores | `RECIBIR` y `DERRIBADO` en Tarou y el ninja; el ninja no tiene `EN_SUELO` ni `LEVANTARSE` (se levanta directo a `PARADO`) |
| Barra de ataque, apoyos, Bárbara | No existen (unidades 25–26) |

Lo que **sí** existe y la ficha no pedía: el `id` por tajo, la cola de un
elemento, el buffer de A, el KO con regreso al título, los turnos de ataque de
los ninjas (el detalle de la IA es de la unidad 24) y la barra de vida
(`src/hud/hud_juego.c`, fuera de esta unidad).

## Dónde está en el juego

| Concepto | Dónde |
|---|---|
| Constantes del combo, vida, invulnerabilidad | `src/juego/mundo.h` |
| El combo: `paso_golpe`, `mundo_golpe_activo`, `nuevo_id` | `src/juego/mundo.c` |
| Tarou recibe: `mundo_recibir`, `paso_aturdido` | `src/juego/mundo.c` |
| La caja, el daño, el derribo, la parada | `src/juego/combate.c`: `combate_caja`, `combate_en_caja`, `combate_danio`, `combate_derriba`, `combate_resolver`, `combate_ninjas_resolver` |
| El ninja: estados, `ninja_recibir`, `ninja_intentar_atacar`, `ninja_golpe_activo`, `ninja_atacando` | `src/enemigos/ninja.c`, `ninja.h` |
| El orden del frame, la parada, `a_guardada`, el KO | `src/pantallas/pantalla_fase1.c`: `pantalla_fase1_actualizar` |
| El parpadeo | `src/juego/protagonista_sprite.c` (`inv & 2`), `src/enemigos/ninja_sprite.c` (`reloj & 2`) |
| Las pruebas | `tests/prueba_combate.c` (la que lo fija todo), `prueba_mundo.c`, `prueba_ninja.c` |

## La librería: PVSnesLib

Esta unidad **no usa funciones nuevas de PVSnesLib**: la lógica del combate es
C puro (`mundo.c`, `combate.c`, `ninja.c` no incluyen `snes.h`), por eso se
prueba en el PC. Lo único de la librería que aparece en el archivo que orquesta
el frame, `src/pantallas/pantalla_fase1.c`, es contexto de otras unidades:

- `bgSetScroll(u8 bgNumber, u16 x, u16 y)` — escribe el scroll del fondo `bg`
  (`REG_BGnHOFS`/`VOFS`); el juego lo llama en `pantalla_fase1_vblank` y no a
  mitad del frame (unidades 04 y 07).
- `setModeHdmaReset(u8 channels)` — corta el HDMA de los canales indicados (máscara de bits); `salir()` lo usa al
  volver al título tras el KO o con START (unidad 12).

Páginas: <https://alekmaul.github.io/pvsneslib/> (cabecera `background.h` para
`bgSetScroll`, `dma.h` para `setModeHdmaReset`). Primero la librería y luego el
registro: en esta unidad no se baja a ningún registro.

## Trampas

- **Cambiar un `.h` y no hacer `make clean`.** `snes_rules` no sigue las
  cabeceras: tras tocar `mundo.h` (`GOLPES`) o `combate.h`, la ROM conserva los
  valores viejos. Las pruebas de PC sí se recompilan (`tests/Makefile` depende
  de las cabeceras).
- **Las pruebas fijan reglas, no el código.** Cambiar `GOLPES` rompe cinco
  pruebas buenas (ejercicio, parte A). Arreglarlas es parte del trabajo.
- **`PARADA_*` son la salida de la resolución, no un estado.** El juego guarda
  `parada` en una variable de `pantalla_fase1.c`; si metes lógica nueva fuera
  del `else` de la parada, correrá **durante** la parada.
- **Una A en la parada vale; una B no.** Limitación conocida del buffer (es de
  un solo bit y solo de A).
- **`conecto` se gasta aunque Tarou sea invulnerable.** Es lo que se quiere; pero
  es fácil «corregirlo» sin querer.
- **Un `u8` en `printf`.** La demo castea a `(unsigned)`; en la SNES, con
  `consoleDrawText("%d", ...)`, un `u8` sin castear a `(u16)` imprime basura
  (CLAUDE.md, unidades 09 y 17).
- **`mundo.h` comenta «salto de 36 px y 24 frames»; medido son 39 px y 25
  cuadros** (`traza_23_salto.c`, `capturas/salida_traza_23_salto.txt`: contando
  desde el cuadro del despegue). El comentario está algo desactualizado; el
  umbral de la parte B se calcula con lo medido.

## La demo

`demo_23_combate.c` incluye `mundo.h`, `combate.h` y `ninja.h` y se enlaza con
los `.c` **reales** (no con una copia): lo que imprime es lo que haría el juego.
Lo único propio es el «jugador» (un guion de A) y el bucle del cuadro, que
repite el orden de `pantalla_fase1_actualizar`. Compilar y ejecutar, desde esta
carpeta, en PowerShell (con el `gcc` de MSYS2 en el `PATH`):

```text
$env:PATH="C:\msys64\usr\bin;"+$env:PATH
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_23.exe demo_23_combate.c ../../src/juego/mundo.c ../../src/juego/combate.c ../../src/enemigos/ninja.c
.\demo_23.exe
```

Compila sin avisos (`capturas/compilacion_demo.txt`); la salida entera, 170
líneas, está en `capturas/salida_demo_23_combate.txt`. Cinco partes:

1. **Cajas dibujadas** (Tarou a derecha e izquierda; el ninja).
2. **El combo con buffer, cuadro a cuadro.** Fragmento real:

```
  cuadro  A  parada | Tarou: golpe reloj cola  id activo | ninja: estado   vida id_golpe x
      3  -       0 |     1     3    0   1     no | ANDANDO      4        0 440
      4  -       3 |     1     4    0   1     SI | RECIBIR      3        1 442  <- GOLPE CUENTA
      5  A       2 |     1     4    0   1     SI | RECIBIR      3        1 442  <- congelado
      7  -       0 |     1     4    0   1     SI | RECIBIR      3        1 442  <- congelado
      8  -       0 |     1     5    1   1     SI | RECIBIR      3        1 444
     11  -       0 |     1     8    1   1     no | RECIBIR      3        1 450
     17  -       0 |     2     0    0   2     no | RECIBIR      3        1 452
     21  -       3 |     2     4    0   2     SI | RECIBIR      2        2 454  <- GOLPE CUENTA
     38  -       0 |     3     4    0   3     SI | RECIBIR      2        2 464
```

   Mira: cuadro 4, el golpe conecta y la parada vale 3; cuadros 5–7, todo
   congelado (`reloj` fijo en 4) **salvo** la A del cuadro 5, que se guarda; en
   el cuadro 8 la cola ya vale 1 (la A guardada, con `reloj` = 5) y el golpe 2
   empieza en el 17, con otro `id`. Los cuatro cuadros activos del golpe 1
   (`reloj` 4, 5, 6 y 7: los cuadros 4, 8, 9 y 10; en los tres congelados, 5 a
   7, la columna sigue en «SI» pero nada se evalúa) pegan **una** vez: `id_golpe` pasa de
   0 a 1 y la vida de 4 a 3, y no vuelve a bajar. El tercer golpe (cuadro 38)
   tiene la caja activa y **no cuenta**: el ninja ya está a 64 px.
3. **Variantes** (resumen):

```
  Resultado CON buffer, ninja libre: golpe más alto del combo = 3, golpes que contaron = 2,
  cuadros de juego con la caja activa encima del ninja = 4, vida final del ninja = 2 de 4.
  Resultado SIN buffer, ninja libre: golpe más alto del combo = 1, golpes que contaron = 1,
  cuadros de juego con la caja activa encima del ninja = 4, vida final del ninja = 3 de 4.
  Resultado CON buffer, ninja clavado: golpe más alto del combo = 3, golpes que contaron = 3,
  cuadros de juego con la caja activa encima del ninja = 12, vida final del ninja = 0 de 4.
```

   Sin buffer, el mismo jugador no pasa del primer golpe: su A cayó en la
   parada. Con el ninja clavado (la situación de la prueba) los tres golpes
   conectan, 12 cuadros activos en total, **tres** daños y no doce.
4. **Dos ninjas contra Tarou:** el primer tajo conecta en el cuadro 12 (`timer`
   12 = primer cuadro activo): `vida 24 → 22`, `inv 60`, `stun 16`, `RECIBIR`,
   y 3 cuadros de parada (el `inv` se queda en 60 cuatro cuadros seguidos).
   Entre los cuadros 32 y 39 el **segundo** ninja da su tajo con Tarou todavía
   parpadeando: su caja pega, `conecto` se pone a 1, `vida` sigue en 22 (el tajo
   se pierde). Cuando `inv` llega a 0, el primer ninja, ya descansado, golpea de
   nuevo en el cuadro 93 y Tarou baja a 20. La columna `dibujo` muestra el patrón
   `visible / (oculto)` de `inv & 2`.
5. **KO:** `mundo_recibir` con `vida = 2` y daño 2 devuelve 1, deja `vida 0`,
   estado `DERRIBADO`, `stun 36`, `inv 60`; tras 200 cuadros machacando A sigue
   igual y otro golpe devuelve 0.

La traza del salto (`traza_23_salto.c`) se compila igual con solo
`mundo.c`: 25 cuadros en el aire, 39 px de altura máxima, 20 cuadros con
altura ≥ 12 (`capturas/salida_traza_23_salto.txt`).

## Cómo se prueba

```text
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

(`560 pruebas, 0 fallos` en `641d7c0`.) `tests/prueba_combate.c` es el
contrato de esta unidad: el golpe y sus cuadros, el combo y su ventana, la caja
(los bordes exactos: 52 en, 53 fuera; ±14 en y, ±15 fuera; 8 px detrás),
ninja que recibe, el combo que mata, Tarou que recibe (vida, aturdido,
invulnerable, KO) y el ninja que ataca (turnos, tajo, caja, alcance, espaldas,
invulnerabilidad).

## Ejercicio y criterio de aprobación

Enunciado en `ejercicio_23.md`; soluciones en `solucion/` (se miran después).

- **Parte A (TDD):** un combo de cuatro golpes (daño 1 + 1 + 1 + 2).
- **Parte B (TDD):** un Tarou que salta alto esquiva el tajo del ninja.

**Aprobación:** las salidas de `pruebas.exe` en rojo (con tu prueba nueva), en
rojo distinto (solo la constante) y en verde, con `0 fallos`; la ROM del clon
compilando con `No warnings or errors found.`; y las tres preguntas
respondidas. Las salidas de referencia están en `capturas/`: A `566 pruebas, 5
fallos` → `566 pruebas, 5 fallos` (otros cinco) → `567 pruebas, 0 fallos`; B
`563 pruebas, 1 fallos` → `563 pruebas, 0 fallos`. Ambas ROM compilaron en el
clon; **no se abrieron en Mesen** (el PC es del autor).

## Lo que viene

- **Unidad 24:** la IA del ninja (acercarse, esperar el turno, atacar) y las
  oleadas; aquí solo se usó su tajo y su caja.
- **Unidad 25–26:** el destello de CGRAM, los apoyos y el relevo con Bárbara,
  que pegará con otro alcance (`COMBATE_ALCANCE` ya avisa de que Tarou tiene el
  corto).
- Cuando existan: la tabla de cajas por cuadro (con la regla de 8 bytes), el
  agarre, el lanzamiento, el ataque aéreo y el especial. Esta unidad se revisará
  entonces.
