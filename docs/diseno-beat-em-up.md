# Diseño — Revisión 3: el juego pasa a ser un beat 'em up

**Proyecto:** _Tarou: A Dog's Journey_ · **Etapa del plan:** 2–3 (Prototipo →
Vertical Slice) · **Revisión:** 3 · **Fecha de esta versión:** 2026-10-23 ·
**Estado:** diseño acordado con el autor, sin código. Lo primero que se
construye es la ROM de prueba de carga (§9).

El juego deja de ser un plataformas de scroll lateral y pasa a ser un **beat
'em up clásico de carril en profundidad** (*belt-scroll*: *Final Fight*,
*Streets of Rage*), con los dos protagonistas, Tarou y Bárbara, y un relevo
entre ellos al estilo de *Donkey Kong Country*. La Revisión 3 entrega la
**fase 1 completa**: aldea → bosque → santuario → dojo, con el jefe.

La Revisión 2 (el tutorial de plataformas) queda congelada en el tag
**`plataformas-r2`** (commit `480f373`). Sus documentos (`diseno-demo-jugable.md`,
`diseno-tutorial-movimiento.md`) pasan a ser históricos: siguen explicando el
código de ese tag, que es el que usan algunas unidades del curso.

La lista de todo el arte nuevo (animaciones, escenografía, objetos, HUD, intro)
y del truco de hardware que usa cada pieza está en
[`arte-beat-em-up.md`](arte-beat-em-up.md).

> **Vocabulario.** Los términos son los de `docs/glosario-snes.md`. Nuevos en
> este documento: *belt-scroll*, *carril*, *profundidad*, *zona*, *oleada*,
> *relevo*, *apoyo*, *barra de ataque*, *caja de golpe*, *caja de daño*,
> *derribo*, *agarre*, *parada de impacto*, *prueba de carga*.

## 0. Historial

| Versión | Fecha | Qué decía |
|---|---|---|
| v1 | 2026-10-23 | Cambio de género acordado con el autor en una sesión de preguntas. Tag `plataformas-r2`. |

## 1. Decisiones del autor (sesión del 2026-10-23)

| # | Tema | Decisión |
|---|---|---|
| 1 | Jugadores | **1 jugador con relevo** entre Tarou y Bárbara; el cooperativo a dos mandos queda como meta futura, no como requisito. |
| 2 | Perspectiva | **Belt-scroll**: x (avance), y (profundidad en el carril), z (altura del salto); sombra bajo los pies; orden de dibujo por y. |
| 3 | Avance | **Zonas por oleadas**: la cámara se bloquea hasta vencer la oleada; después «GO →». |
| 4 | Código de plataformas | Tag `plataformas-r2`; se **reutilizan los motores** (lista de DMA, metasprite, fondo por columnas, cámara, motor de escenas, HUD de depuración); se **retiran** la física de plataformas, la rejilla de colisión, el tutorial y sus pruebas. Se sigue en la rama `demo-secuencia-arranque`. |
| 5 | Arranque | La secuencia se queda; la **intro se rehace** (3 escenas, con placeholders en la R3). |
| 6 | Combate | Estilo *Final Fight*: combo A-A-A (el último derriba), ataque aéreo, agarre caminando contra el enemigo (golpe o lanzamiento), especial A+B que cuesta vida. |
| 7 | Estilos | Distintos. **Tarou**: espada corta y rápida + mordisco, combos largos, se mueve rápido. **Bárbara**: katana lenta, mucho alcance y daño, buenos agarres. |
| 8 | Espada de Tarou | La conserva. **Envaina sola fuera de combate** (al acabar la oleada) y desenvaina al aparecer enemigos; sin botón. |
| 9 | Etapas de vida | Aparcadas: Tarou adulto todo el juego. |
| 10 | Vida y relevo | Barras **separadas**; quien espera fuera se cura poco a poco. Si cae el activo, entra el otro; si caen los dos, se pierde una vida. |
| 11 | Relevo | Botón **L**; quien entra da un **golpe de entrada** que empuja a los cercanos; enfriamiento de ~2 s. |
| 12 | Vidas | Arcade: 3 vidas, 3 continues, comida que cura, puntuación. |
| 13 | Enemigos en pantalla | **Se mide antes** con una ROM de prueba de carga (§9). |
| 14 | Historia | Japón feudal. Un clan de **ninjas kendokas** liderado por el villano roba el juguete favorito de Tarou, el **Señor Pimentón** (un pimiento rojo de goma con forma humana; el autor ya tiene su sprite). Bárbara jura venganza; Tarou quiere recuperar su juguete. |
| 15 | Apoyos | **3 apoyos** que lanzan un poder cuando la **barra de ataque** está llena: la **gata samurái**, muy rápida; el **perro monje** telepático; el **perro bruto**, que embiste y derriba a todos. |
| 16 | Barra de ataque | Se llena **solo golpeando**; compartida por el dúo; el relevo no la vacía. |
| 17 | Elegir apoyo | **R** cambia el apoyo elegido (se ve en el HUD); **X** lo invoca con la barra llena. |
| 18 | Efecto de un apoyo | Ataque a toda la pantalla: entra, hace su golpe y se va. Gata: ráfaga de cortes; monje: **levanta a todos y los suelta**; bruto: embestida que derriba. |
| 19 | Especial A+B | Se mantiene junto a los apoyos: salida de emergencia cuando te rodean, cuesta vida. |
| 20 | Primera entrega | La **fase 1 completa** (con la prueba de carga antes, por la decisión 13). |
| 21 | Enemigos | 3 tipos + jefe: **ninja básico** (shinai), **ninja rápido** (lanza shuriken), **kendoka pesado** (agarra, bloquea). |
| 22 | Arte | Pixellab con el mismo pipeline que Tarou; placeholders numerados mientras tanto. Tarou reutiliza su arte de 96×64 y se amplía. |
| 23 | Fase 1 | Aldea → bosque → **santuario** (allí se conoce a los 3 apoyos y se activa la barra) → dojo, con un jefe. |
| 24 | Jefe de la fase 1 | Un **lugarteniente** kendoka; al final el villano huye con el juguete (gancho para la fase 2). |
| 25 | Objetos | Comida que cura, **armas arrojadizas** (shuriken, kunai; las usan los dos) y vida extra escondida. |
| 26 | Santuario | Una **escena con el motor de escenas** y cuadros de texto en inglés; al terminar, la barra queda activa. |
| 27 | Audio | Después de la R3. |
| 28 | Editor | **Tiled** para zonas y oleadas. |
| 29 | Documentos | Esta spec + `plan-desarrollo.md` reescrito; las specs de plataformas, históricas. |
| 30 | Correr | **Doble toque** de dirección. Y queda libre. |
| 31 | Dormir | Tarou ya no duerme (se retira la animación). |
| 32 | Start | **Pausa**; en pausa, Select vuelve al título. |
| 33 | Celdas | **Bárbara: 64×96** (vertical). Tarou sigue en 96×64. Los dos son 6 piezas de 32×32: mismos 3 KB por cuadro, misma celda de VRAM. |
| 34 | Especies | Los ninjas son **humanos** (celdas de 64×96); el lugarteniente y el villano, también (más grandes). |

## 2. Mando

| Botón | En la fase | En pausa |
|---|---|---|
| Cruceta | moverse por el carril (x e y); **doble toque** a izquierda o derecha = correr | — |
| A | golpe (combo); con arma arrojadiza en la mano, la lanza; en el aire, ataque aéreo; contra un enemigo agarrado, golpe de agarre | — |
| B | saltar | — |
| A+B | especial de emergencia (cuesta vida; no mata al protagonista: deja 1 punto como mínimo) | — |
| Cruceta hacia atrás + A en un agarre | lanzamiento | — |
| L | relevo Tarou ↔ Bárbara | — |
| R | siguiente apoyo (solo con la barra activada) | — |
| X | invocar el apoyo elegido (barra llena) | — |
| Y | libre | — |
| Start | pausa | reanudar |
| Select | HUD de depuración | volver al título |

El **doble toque** es lógica pura (`mando` sigue siendo solo la lectura): se
guarda la última dirección horizontal pulsada y los frames desde que se soltó;
si la misma vuelve a pulsarse en menos de `VENTANA_DOBLE_TOQUE` (~12 frames),
el protagonista corre hasta que suelta la dirección. Se prueba en el PC con
`guion.c`.

## 3. El mundo en x/y/z

- **x**: posición en la fase (0..ancho de la zona), punto fijo 12.4 como hoy.
- **y**: profundidad en el **carril**, la franja de suelo transitable. Para el
  dibujo, y es la línea de pantalla de los pies. El carril lo fija Tiled por
  columnas: `carril_arriba[col]` y `carril_abajo[col]` (§7), de modo que
  una calle se puede estrechar o una escalera ensanchar.
- **z**: altura sobre el suelo (salto, levantamiento del monje, lanzamientos).
  `pantalla_y = y − z`. La **sombra** se dibuja siempre en y.
- Un golpe solo conecta si las cajas se cruzan en x **y** las profundidades
  están cerca: `|y_atacante − y_objetivo| ≤ TOLERANCIA_Y` (~8 px). Es la
  regla que hace que un beat 'em up se sienta «en 3D».
- **Orden de dibujo**: cada frame, las entidades vivas (≤ 8) se ordenan por y
  (inserción: casi siempre ya vienen ordenadas, coste mínimo) y se escriben en
  la OAM en ese orden. El que está más abajo en pantalla va delante. En la
  SNES, el orden de la OAM decide qué sprite tapa a cuál, y el bit de
  prioridad decide contra qué fondos se dibuja.
- **Delante del decorado**: los postes, faroles y columnas en primer plano son
  tiles de BG1 con el **bit de prioridad**. En Mode 1, BG1 alto queda por
  encima de los sprites de prioridad 2 y por debajo de los de prioridad 3. Un
  personaje que pasa **detrás** de un poste (su y es menor que la base del
  poste) se dibuja con prioridad 2, y uno que pasa **delante**, con prioridad 3.

## 4. Protagonistas y relevo

| | Tarou | Bárbara |
|---|---|---|
| Celda | 96×64 (la de hoy) | 64×96 |
| Arma | espada corta | katana |
| Velocidad (andar / correr) | alta / muy alta | media / alta |
| Combo | 4 golpes: corte, corte, corte, mordisco (derriba) | 3 golpes: corte, corte, tajo descendente (derriba) |
| Alcance | corto | largo |
| Daño por golpe | bajo | alto |
| Agarre | zarpazos y lanzamiento corto | golpes de empuñadura y lanzamiento de judo largo |
| Especial A+B | giro con la espada (360°) | corte circular amplio |

Las cifras exactas (velocidades, daño, frames de cada golpe) son constantes en
`tarou.h` / `barbara.h`, a ajustar jugando.

**Relevo (L):**
1. Solo si el enfriamiento ha terminado, el que sale no está en un estado
   bloqueado (recibiendo un golpe, derribado, agarrado) y el que entra tiene
   vida.
2. El que sale **desaparece en una nube** (sprite de efecto). El que entra
   cae desde arriba y da el **golpe de entrada**: daño pequeño y empuje a los
   enemigos a menos de ~48 px. **Nunca hay dos protagonistas en pantalla a la
   vez**: así los dos comparten la celda de VRAM y el presupuesto de sprites
   (§8).
3. El que espera fuera recupera 1 punto de vida cada ~2 s, hasta un tope (no
   se cura del todo esperando).
4. Si el activo cae (vida 0), entra el otro automáticamente. Si el otro
   también está a 0, se pierde una vida y vuelven los dos con la barra llena.

**Envainar de Tarou** (decisión 8): fuera de combate (sin oleada activa) usa
la tira `caminar` envainada y `envainar` al acabar la oleada. Cuando entra
una oleada, `desenvainar`, y desde ahí `guardia`/`caminar en guardia`. Es una
regla del estado del juego, no un botón.

## 5. Combate

### 5.1 Estados de un luchador (protagonista o enemigo)

`QUIETO · ANDAR · CORRER · SALTO · ATAQUE_AEREO · GOLPE_n (combo) ·
AGARRANDO · GOLPE_AGARRE · LANZANDO · AGARRADO · LANZADO · RECIBIR ·
DERRIBADO · EN_SUELO · LEVANTARSE · ESPECIAL · LANZAR_ARMA · RECOGER ·
ENTRADA_RELEVO · KO · VICTORIA` (+ `ENVAINAR/DESENVAINAR` en Tarou,
`BLOQUEO` en el kendoka pesado, `INVOCAR` en los protagonistas).

Es la misma máquina de estados de la Revisión 1 (un `switch` por estado, una
animación por estado). Lo nuevo es que **un luchador puede cambiar el estado
de otro**: el golpe que conecta pone al objetivo en `RECIBIR` o `DERRIBADO`, y
el agarre lo pone en `AGARRADO`. Eso lo resuelve un módulo nuevo,
`combate.c`, lógica pura probada en el PC.

### 5.2 Cajas por cuadro

Cada cuadro de animación lleva, en la tabla generada (`<personaje>_anim.h`),
una **caja de daño** (dónde se le puede golpear) y, en los cuadros activos de
un golpe, una **caja de golpe** con su daño y su efecto (`RETROCESO` /
`DERRIBO`). Las cajas se escriben en Tiled o en un `.json` junto a la
exportación de Pixellab, y `preparar_personaje.py` las añade al `.h`.
Regla de tcc816: la tabla de cajas se rellena a **8 bytes** por entrada (x, y,
ancho, alto, daño, efecto, 2 de relleno) para que `a[i]` sea un
desplazamiento y no una llamada a `tcc__mul`.

### 5.3 Reglas

- **Combo**: A dentro de la ventana de encadenado del golpe anterior pasa al
  siguiente; el último derriba. Si el golpe falla, el combo se corta.
- **Agarre**: andar contra un enemigo que no está atacando, a profundidad
  cercana, durante ~10 frames. A = golpe de agarre (máximo 3); atrás + A =
  lanzamiento, y el lanzado golpea a los que estén en su trayectoria.
- **Ataque aéreo**: A en el aire; derriba.
- **Especial A+B**: invulnerable durante la animación; golpea alrededor;
  cuesta `COSTE_ESPECIAL` de vida solo si conecta (como *Final Fight*).
- **Invulnerabilidad**: ~60 frames al levantarse, con parpadeo.
- **Parada de impacto**: al conectar un golpe fuerte, atacante y objetivo
  se congelan 3–4 frames (el mundo sigue dibujándose). Es barato y da
  contundencia.
- **Destello**: el objetivo golpeado parpadea en blanco 2 frames
  (copia de una paleta blanca a su entrada de CGRAM por la lista de DMA).
- **Barra de ataque**: cada golpe que conecta suma; recibir daño no suma.
  Bloqueada hasta el santuario.

### 5.4 Apoyos

Al invocarlo (X con la barra llena): la acción se congela, el apoyo hace su
ataque a toda la pantalla y se va, y la barra vuelve a 0.

| Apoyo | Qué hace | Truco de hardware (detalle en `arte-beat-em-up.md` §6) |
|---|---|---|
| Gata samurái | cruza la pantalla en 3 tajos; todos reciben daño y derribo | ventanas 1+2 dibujadas por HDMA en diagonal + suma del color fijo = tajos de luz (modo especial: la acción está congelada y las sombras escondidas, §8) |
| Perro monje | levanta a todos los enemigos (z sube), los sostiene y los suelta | mosaico pulsante + ondulación por HDMA del scroll de BG2; enemigos en `LANZADO` con z alta |
| Perro bruto | embiste de lado a lado; todos derribados y lanzados | temblor de pantalla (scroll ±2 px), polvo en sprites |

Durante el apoyo, el protagonista queda en una pose fija (`INVOCAR`). Su
segunda celda de VRAM queda libre y se presta a los cuadros del apoyo (§8).

## 6. Enemigos

| Enemigo | Vida | Comportamiento | Ataques |
|---|---|---|---|
| Ninja básico | baja | se acerca, rodea, espera turno | golpe de shinai, patada |
| Ninja rápido | baja | mantiene distancia, cambia de carril | lanza shuriken (proyectil por y), tajo rápido |
| Kendoka pesado | alta | avanza despacio | bloquea golpes frontales (el combo rebota), agarra y lanza |
| Lugarteniente (jefe) | muy alta | patrones por fases de vida | carga, tajo de alcance, convoca 1 ninja básico |

**Turnos de ataque**: como mucho **2** enemigos atacando a la vez (un «testigo»
que se pasan). Los demás rodean o esperan. Es la regla clásica para que 3
enemigos no se sientan injustos. También abarata la CPU, porque solo 2
calculan cajas de golpe.

**Variantes por paleta**: un mismo sprite con otra paleta = otra variante
(más vida, más rápido) sin arte nuevo.

## 7. La fase 1 y Tiled

| Zona | Qué pasa | Oleadas |
|---|---|---|
| 1. Aldea | calle con casas y puestos al atardecer; cajas y barriles con comida | 3 (básicos; luego básicos + rápido) |
| 2. Bosque de bambú | sendero estrecho (carril más fino), viento | 3 (rápidos; un pesado) |
| 3. Santuario | torii, faroles, de noche; **escena** con los 3 apoyos; la barra se activa; una oleada corta para probar el apoyo | 1 |
| 4. Dojo | puerta exterior (1 oleada) e interior: el **lugarteniente**; al vencerlo, escena: el villano huye con el Señor Pimentón | 1 + jefe |

Entre zonas: fundido con mosaico y carga con la pantalla apagada (cada zona
puede tener su propio juego de tiles). Al vencer al jefe:
«STAGE CLEAR», puntuación y después «STAGE 2 / COMING SOON».

**Tiled** (como `res/nivel1/tutorial.tmj`, pero para el carril):
- Capa de tiles **lógica** con celdas de 16 px: *carril* (transitable),
  *pared* (no transitable) y *borde* (el fondo del carril). El script deduce
  `carril_arriba/abajo` por columna.
- Objetos: `inicio`, `limite_zona` (x donde se bloquea la cámara), `oleada`
  (propiedades: `tipos`, `cantidad`, `lado`, `retardo`, `zona`), `rompible`
  (propiedad `contiene`: `comida_chica`, `comida_grande`, `shuriken`, `kunai`,
  `vida`), `escena` (el santuario), `jefe`, `salida`.
- `scripts/preparar_zona.py` (sucesor de `preparar_nivel.py`) viste la lógica
  con el tileset de Pixellab y genera `fondo_cerca.png`, `zonaN_datos.h`
  (carril, oleadas, objetos) y las pruebas en el PC recorren los datos
  reales, como hoy con el tutorial.

## 8. Presupuesto de hardware (lo que manda en este género)

Cifras de `docs/hardware-snes-por-modulo.md`: 128 sprites, **32 sprites y 34
ranuras de 8 px por línea**, **512 tiles de sprite** direccionables (16 KB),
8 paletas de sprite (las **4–7** son las que entran en la matemática de
color), VBlank de 37 líneas (el SDK usa 5; hoy el nivel acaba en la 7 y en la
17–20 los frames que suben medio cuadro de Tarou).

**Ranuras por línea.** En belt-scroll todos comparten la misma franja de
líneas (el carril), así que este es el límite que manda:

| Quién | Ancho | Ranuras en sus líneas |
|---|---|---|
| Tarou (96×64) | 3 piezas de 32 | 12 |
| Bárbara (64×96) | 2 piezas | 8 |
| Ninja (64×96) | 2 piezas | 8 |
| Sombra (32×8 aprox., pieza de 16) | 2 piezas de 16 | 4 |

Tarou + 3 ninjas a la misma altura = 12 + 24 = **36 > 34**: se pierde un trozo.
Bárbara + 3 = 32: cabe justo. Las sombras van en las líneas de los pies, que
son las mismas. Medidas posibles, a decidir con la prueba de carga:
recortar piezas vacías (el metasprite ya se las salta), sombras de 16 px,
sombras a 30 Hz (frames alternos) y, si se pasa, **rotar el orden** de las
entidades cada frame para que el corte vaya cambiando (parpadeo en lugar de
un agujero fijo).

**VRAM de sprites (512 tiles).** El protagonista activo usa **256** (doble
búfer de 2 celdas de 128). Quedan 256 para enemigos, sombras, efectos y
objetos. Un ninja de 64×96 a 6 piezas son **96 tiles por cuadro**: 3 ninjas
con un cuadro cada uno = 288, **no caben**. Opciones que mide la prueba:
- **A**: piezas de 16×16 recortadas al contorno (un humano de 64×96 ocupa
  ~60–70 % de su caja: ~64 tiles) → 3 × 64 = 192 + 64 para lo demás.
- **B**: celda de enemigo más estrecha (48×96).
- **C**: 2 ninjas «grandes» + 1 en una pose compartida residente.

**Ancho de banda del VBlank.** Hoy medio cuadro de Tarou (1,5 KB) lleva el
VBlank de la línea 7 a la 17–20: ~125 bytes por línea. Quedan ~17 líneas
≈ 2 KB por frame. Un cuadro de enemigo recortado (~2 KB) cabe **uno por
frame**. Con 3 enemigos animando a ~12 cuadros por segundo (un cambio cada
5 frames) y un turno rotatorio (cada frame sube como mucho un enemigo),
funciona. El truco para ganar más: **alargar el VBlank** apagando la
pantalla (*forced blank*) en las últimas ~16 líneas, que quedan negras como
bandas de cine. Eso da ~16 líneas más de DMA (~2 KB). Lo hacían muchos juegos
comerciales y se activa con una IRQ de línea (`HTIMEN/VTIME`) o con HDMA
sobre `INIDISP`.

**CGRAM de sprites (8 paletas de 16):**

| Paleta | Uso |
|---|---|
| 0 | protagonista activo (al hacer relevo se copia la otra paleta: 32 bytes) |
| 1 | ninja básico (y sus variantes por paleta) |
| 2 | ninja rápido |
| 3 | kendoka pesado; en la zona del jefe, el lugarteniente |
| 4 | sombras (negras: con la media suma oscurecen al 50 % lo que hay debajo) |
| 5 | chispas de golpe, nube de relevo, «GO →» (translúcidas) |
| 6 | apoyo activo |
| 7 | objetos, armas arrojadizas, Señor Pimentón, efectos del decorado (hojas, luciérnagas) |

Las paletas 4–7 son las únicas que la matemática de color afecta. **Ojo: el
modo es global, no por paleta.** `CGWSEL` (qué es el segundo operando: la
pantalla secundaria o el color fijo) y `CGADSUB` (sumar o restar, a la mitad o
no, y sobre qué capas) valen para todos los sprites de las paletas 4–7 a la
vez; como mucho se cambian **por línea** con HDMA, nunca por sprite. Por eso:

- **Modo del combate** (todo el tiempo que se juega): **media suma con la
  pantalla secundaria** (BG1 y BG2 en la secundaria). Las sombras negras
  oscurecen; las chispas y objetos de las paletas 4–7 salen **translúcidos**
  (luz mezclada), no brillantes. Los objetos que deban verse opacos van en
  0–3 cuando haya hueco o se dibujan con colores claros.
- **Modos especiales**, solo en los momentos congelados (apoyos, pausa,
  escenas, el halo del santuario): suma con el **color fijo** (destellos,
  tajos de luz) o resta (pausa). En esos momentos las sombras se esconden.
- Si una zona necesita luz fija en el decorado (rayos del bosque, halo de los
  faroles), se aplica por **ventana** sobre las capas BG, no sobre los sprites,
  y la prueba de carga comprueba que convive con las sombras.

**Fondos (Mode 1):** BG1 (4bpp) la capa cercana (calle, suelo del carril,
fachadas), BG2 (4bpp) la lejana (montañas, cielo, bambú del fondo), BG3
(2bpp) el **HUD del juego** y el HUD de depuración, en filas distintas del
mismo mapa de texto; el color de fondo, por HDMA (el degradado de hoy). El
HUD del juego necesita prioridad alta (`BG3_MODE1_PRIORITY_HIGH`, como el de
depuración), y los retratos de los protagonistas van en sprites de 16 colores
en las líneas de arriba, donde el carril no llega.

**CGRAM de fondos (entradas 0..127):** BG1 y BG2 comparten **8 paletas de 16
colores en total** (no 8 por capa), y las paletas de 4 colores de BG3 se
solapan con las primeras entradas:

| Entradas | Paleta 4bpp | Uso |
|---|---|---|
| 0..15 | 0 | BG3: las paletas 2bpp 0..3 del HUD (4 × 4 colores); el color 0 es el color de fondo (HDMA) |
| 16..31 | 1 | BG3: paletas 2bpp 4..7 si el HUD necesita más; si no, libre para BG1/BG2 |
| 32..127 | 2..7 | **las zonas**: BG1 y BG2 juntas |

Con el HUD en las paletas 2bpp 0..3, cada zona tiene **7 paletas × 15
colores para sus dos capas** (6 si el HUD usa también las 4..7). Es la cifra
que hay que dar a Pixellab al pedir los tilesets: por ejemplo, 4 paletas para
la capa cercana y 3 para la lejana.

**Tamaños de sprite (`OBSEL`).** En pantalla solo hay **dos tamaños** a la vez.
El nivel usa hoy `OBJ_SIZE16_L32` (16×16 y 32×32): no hay sprites de 8×8, y
cualquier pieza pequeña ocupa 16×16 (2 ranuras por línea). Cambiar a 8/32
rompería las sombras y los objetos de 16×16 y la opción A. La prueba de carga
lo confirma.

**CPU.** ~59 000 ciclos por frame. Con 1 protagonista + 3 enemigos + 2
proyectiles, las cajas de golpe se comprueban solo para los que atacan (como
mucho 2 enemigos + el protagonista), por pares, sin multiplicaciones. El HUD
de depuración (`LIN`) mide si cabe.

## 9. La prueba de carga (lo primero que se construye)

Una ROM `-DPRUEBA_CARGA` sin lógica de combate: Tarou o Bárbara y N ninjas
placeholder de 64×96 (rectángulos numerados de colores) que se pasean por el
carril con sombras, animando y subiendo cuadros por la lista de DMA.

| Mide | Cómo | Para decidir |
|---|---|---|
| Ranuras por línea | colocar 3 y 4 enemigos en la misma y; capturas en Mesen | cuántos enemigos, sombras sí/no, rotación |
| VRAM | opciones A/B/C de §8 | tamaño y forma de la celda del ninja |
| VBlank | HUD `V a b` con 1–4 enemigos subiendo cuadros | turno de subida, cuadros por segundo, bandas de cine sí/no |
| CPU | HUD `LIN` con orden por y y cajas | margen para la IA |
| Llenar la OAM | `LIN` con la opción A (cada ninja hasta ~24 piezas de 16×16: más de 70 `oamSet` por frame por tcc816) | si las piezas pequeñas compensan, o hace falta escribir el búfer de la OAM en asm |
| Modo de color | sombras con media suma sobre BG1+BG2, junto a una ventana de luz del decorado | que el modo del combate (§8) se ve bien y convive con los efectos de zona |
| `OBSEL` | 16/32 con sombras, objetos y piezas de 16×16 | que no hacen falta sprites de 8×8 |

**Criterio de salida:** la configuración elegida corre a 60 FPS (HUD), sin
trozos perdidos en el caso típico (2–3 enemigos), con el VBlank por debajo de
la línea 37 en el peor frame. El resultado se escribe en esta spec (v2) y fija
el tamaño de los ninjas para pedirlos a Pixellab.

## 10. Arquitectura (qué cambia en `src/`)

| Se queda | Cambia | Sale | Nuevo |
|---|---|---|---|
| `sistema/*`, `escena/*`, `dma_lista` + `dma_ejecutar.asm`, `metasprite`, `fondo_nivel` (columnas al anillo), `camara` (+ bloqueo de zona), `hud_debug`, `mando`, `guion` | `tarou.c` (estados de combate), `animaciones.c` (cajas), `tarou_sprite.c` → `protagonista_sprite.c` (celda compartida, dos paletas), `pantalla_nivel1.c` → `pantalla_fase1.c`, `pantalla_intro.c` (3 escenas nuevas), `barbara.c` y `cambio_protagonista.c` (de stub a código) | `colision.c` (rejilla de plataformas), `partida.c` (reglas del tutorial), `guion_tutorial.c`, `res/nivel1/*` del tutorial y sus pruebas (quedan en el tag) | `mundo.c` (x/y/z, carril, orden por y), `combate.c` (cajas, daño, agarre, turnos), `enemigos/ninja.c` y `enemigos/jefe.c`, `oleadas.c` (zonas y bloqueo de cámara), `objetos/*` (rompibles, comida, armas), `apoyos.c` + efectos de pantalla en `efectos.c`, `hud/hud_juego.c`, `pantallas/pantalla_santuario.c` |

Las reglas de la casa siguen igual: lógica pura sin `snes.h` y probada en el
PC (ahora también `mundo`, `combate`, `oleadas`, IA, apoyos y doble toque),
tablas de structs a potencia de 2, sin divisiones en el frame, `static`
inicializadas, toda escritura a la VRAM por la lista de DMA y los registros
de scroll en el VBlank.

## 11. Fuera de alcance de la Revisión 3

Audio · cooperativo a 2 mandos · fases 2+ · etapas de vida de Tarou · el arte
final de la intro (placeholders) · un mapa de viaje entre fases.

## 12. Pendientes que no bloquean el diseño

- Nombre del villano, del lugarteniente y de los 3 apoyos.
- El sprite del **Señor Pimentón**: el autor lo tiene y lo aporta cuando toque; falta dejarlo en
  `res/descargas/graficos/` y decir su tamaño.
- **Ataque corriendo** (A mientras se corre): propuesta, no decidida.
- Cifras de combate (daño, velocidades, ventanas): se ajustan jugando.
