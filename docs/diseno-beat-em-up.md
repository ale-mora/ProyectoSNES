# Diseño — Revisión 3: el juego pasa a ser un beat 'em up

**Proyecto:** _Tarou: A Dog's Journey_ · **Etapa del plan:** 2–3 (Prototipo →
Vertical Slice) · **Revisión:** 3 · **Fecha de esta versión:** 2026-10-23 ·
**Estado:** diseño acordado con el autor, sin código. Lo primero que se
construye es la ROM de prueba de carga (§9).

El juego deja de ser un plataformas de scroll lateral y pasa a ser un **beat
'em up clásico de carril en profundidad** (*belt-scroll*: *Final Fight*,
*Streets of Rage*), con los dos protagonistas, Tarou y Bárbara, y un relevo
entre ellos al estilo de *Donkey Kong Country*. La Revisión 3 entrega la
**fase 1 completa**: aldea → bosque → monasterio de Rex → dojo, con el jefe.

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
| v14 | 2026-10-24 | Pantalla de la fase: decisiones 56–58 (cuatro zonas encadenadas, escenas con el motor de escenas, la cámara no retrocede) y el director de la fase (`fase.c`) con sus placeholders. |
| v13 | 2026-10-24 | Fase 1 por zonas: decisiones 51–55 (barra llena al activarse, longitudes, oleadas, objetos, escenas) y §7 con los números; el «lugarteniente» pasa a ser Gomasuri-dono. |
| v12 | 2026-10-24 | Apoyos y barra de ataque: decisiones 47–50 (daño, jefes, barra de un uso, Rex cura) y cronología propuesta de cada apoyo (§5.4). |
| v11 | 2026-10-24 | Bárbara y relevo: decisión 33 revisada (Bárbara 96×96); decisiones 43–46 (entrada del relevo, relevo por KO, quién puede salir, Bárbara en combate). |
| v10 | 2026-10-24 | Decisiones 40–42 (referencia de cine de ninjas de Cannon Films): patada voladora, zoom al KO, entrada del jefe. |
| v9 | 2026-10-23 | §5.5: cifras de combate decididas (vida, daño, velocidades, tiempos, rangos, jefes, puntos); salen de §12. |
| v8 | 2026-10-23 | Decisión 5: la intro cuenta la apertura del guion (Rev. 3). |
| v7 | 2026-10-23 | Decisión 39: ataque corriendo para los dos protagonistas (sale de §12). |
| v6 | 2026-10-23 | Restos de «santuario» → monasterio (decisión 26, barra de ataque, objetos de Tiled, modos de color, archivos de §10: `pantalla_monasterio.c`); §12 sin los nombres ya decididos. |
| v5 | 2026-10-23 | Ajuste al guion (`guion-historia.md`): hermanos, Tarou pastor, nombres de villano y apoyos, santuario → monasterio, jefe Gomasuri-dono. Decisiones 14, 15, 23, 24. |
| v4 | 2026-10-23 | La capa de sprites (piezas de 16×16 `.p16`, plan de subidas por número de piezas, OAM en ensamblador con camino rápido) y la prueba de carga rehecha con ninjas de 64×96: 3 ninjas + Tarou + sombras caben (§9.2). |
| v3 | 2026-10-23 | Reparto de la VRAM de sprites en pares de filas y subidas en trozos de 512 B (§8); decisiones 35–38. |
| v2 | 2026-10-23 | Resultados de la prueba de carga (§9.1): ≤ 2 KB por cuadro de enemigo, el límite de ranuras borra primero al protagonista, la capa de sprites tiene que ir en ensamblador. Correcciones de hardware en §8 (matemática de color global, `OBSEL`, CGRAM de fondos). |
| v1 | 2026-10-23 | Cambio de género acordado con el autor en una sesión de preguntas. Tag `plataformas-r2`. |

## 1. Decisiones del autor (sesión del 2026-10-23)

| # | Tema | Decisión |
|---|---|---|
| 1 | Jugadores | **1 jugador con relevo** entre Tarou y Bárbara; el cooperativo a dos mandos queda como meta futura, no como requisito. |
| 2 | Perspectiva | **Belt-scroll**: x (avance), y (profundidad en el carril), z (altura del salto); sombra bajo los pies; orden de dibujo por y. |
| 3 | Avance | **Zonas por oleadas**: la cámara se bloquea hasta vencer la oleada; después «GO →». |
| 4 | Código de plataformas | Tag `plataformas-r2`; se **reutilizan los motores** (lista de DMA, metasprite, fondo por columnas, cámara, motor de escenas, HUD de depuración); se **retiran** la física de plataformas, la rejilla de colisión, el tutorial y sus pruebas. Se sigue en la rama `demo-secuencia-arranque`. |
| 5 | Arranque | La secuencia se queda; la **intro se rehace** (3 escenas, con placeholders en la R3): es la apertura del guion (flashback, robo, juramento; `guion-historia.md` §4). |
| 6 | Combate | Estilo *Final Fight*: combo A-A-A (el último derriba), ataque aéreo, agarre caminando contra el enemigo (golpe o lanzamiento), especial A+B que cuesta vida. |
| 7 | Estilos | Distintos. **Tarou**: katana sujeta con la boca (rápida, de alcance corto) + mordisco, combos largos, se mueve rápido. **Bárbara**: katana lenta, mucho alcance y daño, buenos agarres. |
| 8 | Katana de Tarou | La conserva (es una katana, como la de Bárbara; él la lleva en la boca, como en la portada). **Envaina sola fuera de combate** (al acabar la oleada) y desenvaina al aparecer enemigos; sin botón. |
| 9 | Etapas de vida | Aparcadas: Tarou adulto todo el juego. |
| 10 | Vida y relevo | Barras **separadas**; quien espera fuera se cura poco a poco. Si cae el activo, entra el otro; si caen los dos, se pierde una vida. |
| 11 | Relevo | Botón **L**; quien entra da un **golpe de entrada** que empuja a los cercanos; enfriamiento de ~2 s. |
| 12 | Vidas | Arcade: 3 vidas, 3 continues, comida que cura, puntuación. |
| 13 | Enemigos en pantalla | **Se mide antes** con una ROM de prueba de carga (§9). |
| 14 | Historia | Japón feudal. Los **ninjas kendokas** de la federación «Autistas Marciales», liderada por **Nikoraemon**, roban el juguete favorito de Tarou, el **Señor Pimentón** (un pimiento rojo de goma con forma humana; en verdad una reliquia: un dios antiguo). Tarou y Bárbara son **hermanos** (Tarou, perro pastor; no husky). Bárbara quiere venganza; Tarou, su juguete. Guion completo, final incluido, en [`guion-historia.md`](guion-historia.md) (Rev. 3). |
| 15 | Apoyos | **3 apoyos** que lanzan un poder cuando la **barra de ataque** está llena: **Lucinda**, la gata samurái, muy rápida; **Rex**, el perro monje psíquico (levita, nunca golpea); **Fenrir**, el husky bruto, que embiste y derriba a todos. |
| 16 | Barra de ataque | Se llena **solo golpeando**; compartida por el dúo; el relevo no la vacía. |
| 17 | Elegir apoyo | **R** cambia el apoyo elegido (se ve en el HUD); **X** lo invoca con la barra llena. |
| 18 | Efecto de un apoyo | Ataque a toda la pantalla: entra, hace su golpe y se va. Gata: ráfaga de cortes; monje: **levanta a todos y los suelta**; bruto: embestida que derriba. |
| 19 | Especial A+B | Se mantiene junto a los apoyos: salida de emergencia cuando te rodean, cuesta vida. |
| 20 | Primera entrega | La **fase 1 completa** (con la prueba de carga antes, por la decisión 13). |
| 21 | Enemigos | 3 tipos + jefe: **ninja básico** (shinai), **ninja rápido** (lanza shuriken), **kendoka pesado** (agarra, bloquea). |
| 22 | Arte | Pixellab con el mismo pipeline que Tarou; placeholders numerados mientras tanto. Tarou reutiliza su arte de 96×64 y se amplía. |
| 23 | Fase 1 | Aldea → bosque (se une Lucinda; Fenrir cruza como gag) → **monasterio de Rex** (antes «santuario»; se une Rex y se activa la barra) → dojo, con un jefe. Fenrir se une del todo en la fase 2. |
| 24 | Jefe de la fase 1 | **Gomasuri-dono**, lugarteniente adulador, gordo y pomposo; al vencerlo, Nikoraemon huye con el juguete. La historia completa tiene 5 fases y cierre en la fase 5 (`guion-historia.md`). |
| 25 | Objetos | Comida que cura, **armas arrojadizas** (shuriken, kunai; las usan los dos) y vida extra escondida. |
| 26 | Monasterio | Una **escena con el motor de escenas** con Rex y cuadros de texto en inglés; al terminar, la barra queda activa. |
| 27 | Audio | Después de la R3. |
| 28 | Editor | **Tiled** para zonas y oleadas. |
| 29 | Documentos | Esta spec + `plan-desarrollo.md` reescrito; las specs de plataformas, históricas. |
| 30 | Correr | **Doble toque** de dirección. Y queda libre. |
| 31 | Dormir | Tarou ya no duerme (se retira la animación). |
| 32 | Start | **Pausa**; en pausa, Select vuelve al título. |
| 33 | Celdas | **Bárbara: 96×96** (revisada el 2026-10-24: con 64×96 su katana solo llegaba a 32 px de los pies y no superaba el alcance de Tarou, 36). Tarou sigue en 96×64. Los dos comparten la celda de VRAM, así que Bárbara debe caber en **24 piezas de 16×16 no vacías por cuadro (3 KB)**: a comprobar con `piezas16.py` en su primera exportación, y hay que **volver a medir las ranuras por línea** con su celda de 96 px (§8). |
| 34 | Especies | Los ninjas son **humanos** (celdas de 64×96); el lugarteniente y el villano, también (más grandes). |
| 35 | Tamaño de los ninjas (tras la prueba de carga) | Se mantiene **64×96**, con subida de tamaño exacto y **bandas de cine** si hace falta margen en el VBlank (§9.1). |
| 36 | Enemigos a la vez | **3 como máximo**, que la IA reparte en profundidad (nunca 3 a la misma altura) para no pasar de 34 ranuras por línea. |
| 37 | Siguiente paso técnico | La **capa de sprites en ensamblador** (orden, OAM y plan de subidas, una llamada por frame) con los planes de copia precalculados por script; después se repite la prueba de carga. |
| 38 | Piezas de Tarou | Tarou también se corta en **piezas de 16×16 sin las vacías**, como los enemigos y Bárbara. |
| 39 | Ataque corriendo | A mientras se corre. **Tarou**: tajo en carrera; **Bárbara**: embestida con el hombro. Los dos derriban. |
| 40 | Patada voladora | Ataque aéreo con giro de piernas (de referencia: cine de ninjas de los 80, Cannon Films). Más alcance que el ataque aéreo y derriba a todos los de la línea. Se suma al aéreo, no lo reemplaza. **Entrada:** A en el aire con la cruceta hacia donde mira (sin cruceta, ataque aéreo normal). **Cifras:** 7 Tarou / 10 Bárbara, caja de 56 px (el aéreo, 36 px). |
| 41 | Zoom al KO | Al vencer al último enemigo de una oleada: encuadre cercano de 30 frames (0,5 s) con la acción congelada y luego se corta. Solo se ve en el KO del último enemigo; los demás, sin zoom. |
| 42 | Entrada del jefe | Cada jefe entra de pie, con un plano fijo y su título en pantalla (formato de cartel, `arte-beat-em-up.md` §11). Durante la entrada, nada le pega. |
| 43 | Relevo: la entrada | El que sale desaparece en una nube. El que entra **cae desde 80 px** sobre el mismo sitio, mirando hacia donde miraba el otro, **invulnerable** durante la caída y el golpe de entrada (36 frames). El golpe de entrada se resuelve **por distancia**: a ≤ 48 px en x y a la misma profundidad, 4 de daño y retroceso, una sola vez al aterrizar. |
| 44 | Relevo por KO | Si el activo cae KO, entra el otro **al instante, sin enfriamiento** y con 60 frames de invulnerabilidad. Si el otro también está a 0, se pierde una vida. El que salió por KO se sigue curando fuera (+1 cada 120 frames, tope 48) y puede volver en cuanto tenga 1 de vida. |
| 45 | Relevo: quién puede salir | No puede salir quien está recibiendo un golpe, derribado, en el suelo, levantándose, agarrado, lanzado, KO, entrando, agarrando o lanzando (soltar a un enemigo a medias no tiene sentido). Atacando o saltando, sí. |
| 46 | Bárbara en combate | Combo de **3 golpes, 24/24/30 frames**, ventana de encadenado de 12; el tajo final (8) derriba y cierra el combo. Velocidad 1,25 andando y 2,5 corriendo. Mismos estados que Tarou, con sus tablas propias (`luchador.c`, `QUIEN_BARBARA`). |
| 47 | Daño de los apoyos | **Lucinda 24** (3 tajos de 8), **Rex 16** (al soltar a los enemigos levantados), **Fenrir 20** (la embestida). A toda la pantalla; derriban a los enemigos normales. |
| 48 | Apoyos contra jefes | **La mitad del daño** (Lucinda 4 por tajo, Rex 8, Fenrir 10) y **no derriban** al jefe. Un apoyo ayuda sin saltarse el combate. |
| 49 | Barra de ataque: un uso | Se llena con **48 golpes que conectan** y no suma mientras actúa un apoyo. Llena = **una** invocación; al invocar vuelve a 0. Bloqueada hasta el monasterio de Rex. Si caen los dos protagonistas, vuelven con la barra llena. |
| 50 | Rex cura | Además de levantar y soltar a los enemigos, **cura 16** al protagonista activo (sin pasar de 64). Es el apoyo defensivo del guion (§2). |
| 51 | La barra empieza llena | Al activarse en el monasterio la barra **empieza llena**: la oleada del monasterio (3 amarillos, ~20 golpes) no bastaría para llenar los 48, y la spec pedía «probar el apoyo». Activarla otra vez no la vuelve a llenar. |
| 52 | Longitud de las zonas | Aldea **1536 px**, bosque **1280**, monasterio **768**, dojo **1024**: 4608 px (18 pantallas). La cámara llega hasta `longitud − 256`. |
| 53 | Oleadas por zona | Aldea: 2 blancos; 3 blancos; 2 blancos + 1 rápido. Bosque: 2 rápidos + 1 amarillo; 3 amarillos; 1 pesado rojo + 2 amarillos. Monasterio: 3 amarillos. Dojo: puerta con 2 verdes + 1 rápido amarillo; jefe (Gomasuri-dono), solo. Los enemigos salen de uno en uno, 40 frames entre cada uno; la cámara se bloquea en la x de la oleada hasta vencerla y entonces sale «GO →» (1,5 s). |
| 54 | Rompibles y comida | Aldea 6 (2 onigiri, **1 vida extra escondida** en el último farol, 3 vacíos); bosque 4 (2 onigiri, 1 shuriken, 1 vacío); monasterio 3 (1 pescado, 2 vacíos); dojo 4 (1 onigiri, 1 pescado, 2 vacíos). |
| 55 | Escenas en mitad de la zona | Bosque: Lucinda se une al vencer la oleada 1; Fenrir cruza (gag) tras la 2. Monasterio: la escena de Rex **abre la zona**, antes de su oleada (se une Rex y se activa la barra). Dojo: la entrada del jefe va antes de su oleada y la huida de Nikoraemon, después. |
| 56 | La pantalla de la fase | **Las cuatro zonas encadenadas** (aldea → bosque → monasterio → dojo) en una sola pantalla, con fundido entre zonas y, tras el jefe, STAGE CLEAR (puntuación) y STAGE 2 / COMING SOON. Sin vidas: GAME OVER con la cuenta atrás del continue (9 s). Se prueba con `-DFASE1`. |
| 57 | Escenas con el motor de escenas | Al saltar una escena la acción se para: el brillo baja de 15 a 6 y el mosaico sube de 0 a 5 en 24 frames **solo en los fondos** (el texto, en BG3, queda nítido); salen los cuadros de texto del guion (≤ 26 caracteres; A los adelanta) y se vuelve a afilar. |
| 58 | La cámara | Sigue al protagonista hacia la derecha **sin retroceder**, queda a 120 px del borde izquierdo y se bloquea en la x de cada oleada. El protagonista no sale de la pantalla (16 px de margen). Salida de la zona: con la zona completa, 32 px antes del final; en el dojo no hay salida, viene el STAGE CLEAR. |

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
| Celda | 96×64 (la de hoy) | 96×96 |
| Arma | katana en la boca | katana en la mano |
| Velocidad (andar / correr) | alta / muy alta | media / alta |
| Combo | 4 golpes: corte, corte, corte, mordisco (derriba) | 3 golpes: corte, corte, tajo descendente (derriba) |
| Alcance | corto (caja de 28 px; llega a 36 de los pies) | largo (caja de 44 px, 52 el tajo; llega a 48 de los pies, el borde de su celda) |
| Daño por golpe | bajo | alto |
| Agarre | zarpazos y lanzamiento corto | golpes de empuñadura y lanzamiento de judo largo |
| Especial A+B | giro con la espada (360°), 96 px | corte circular amplio, 128 px, centrado en los pies |

Las cifras están en §5.5; van como constantes en `src/juego/cifras.h`.

**Relevo (L):**
1. Solo si el enfriamiento ha terminado, el que sale no está en un estado
   bloqueado (recibiendo un golpe, derribado, agarrado) y el que entra tiene
   vida.
2. El que sale **desaparece en una nube** (sprite de efecto). El que entra
   cae desde arriba y da el **golpe de entrada**: daño pequeño y empuje a los
   enemigos a menos de ~48 px. **Nunca hay dos protagonistas en pantalla a la
   vez**: así los dos comparten la celda de VRAM y el presupuesto de sprites
   (§8).
3. El que espera fuera recupera 1 punto de vida cada 2 s (120 frames), hasta 48
   de 64 (no se cura del todo esperando).
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
  Bloqueada hasta el monasterio de Rex.

### 5.4 Apoyos

Al invocarlo (X con la barra llena): la acción se congela, el apoyo hace su
ataque a toda la pantalla y se va, y la barra vuelve a 0.

| Apoyo | Qué hace | Truco de hardware (detalle en `arte-beat-em-up.md` §6) |
|---|---|---|
| Gata samurái | cruza la pantalla en 3 tajos; todos reciben daño y derribo | ventanas 1+2 dibujadas por HDMA en diagonal + suma del color fijo = tajos de luz (modo especial: la acción está congelada y las sombras escondidas, §8) |
| Perro monje | levanta a todos los enemigos (z sube), los sostiene y los suelta | mosaico pulsante + ondulación por HDMA del scroll de BG2; enemigos en `LANZADO` con z alta |
| Perro bruto | embiste de lado a lado; todos derribados y lanzados | temblor de pantalla (scroll ±2 px), polvo en sprites |

**Cronología propuesta** (no estaba en la spec; se ajusta jugando, `src/juego/apoyos.c`):
Lucinda, 3 tajos a los 12, 24 y 36 frames y acaba a los 48; Rex, levanta y cura a los 20,
suelta a los 60 y acaba a los 80; Fenrir, la embestida pega a los 30 y acaba a los 50. Los
apoyos se van uniendo (Lucinda en el bosque, Rex en el monasterio, Fenrir en la fase 2) y R
solo recorre los que ya están. El protagonista no puede invocar en el aire ni en un estado
bloqueado, ni hacer un relevo mientras invoca (`LQ_INVOCAR`, invulnerable).

Durante el apoyo, el protagonista queda en una pose fija (`INVOCAR`). Su
segunda celda de VRAM queda libre y se presta a los cuadros del apoyo (§8).

### 5.5 Cifras de combate (v9, decididas con el autor)

Valores iniciales: van como constantes en **`src/juego/cifras.h`** (todas juntas) y se
ajustan jugando, pero el punto de partida ya no se discute. Unidades: puntos de vida,
**frames** (60 por segundo) y **px por frame** en punto fijo 12.4. Todo cabe en `u8`
salvo la puntuación. Los multiplicadores por rango van en **tablas** de 4 entradas,
nunca como multiplicación en el frame.

**Protagonistas**

| | Tarou | Bárbara |
|---|---|---|
| Vida | 64 | 64 |
| Andar / correr (x) | 1,5 / 3 | 1,25 / 2,5 |
| Profundidad (y) | la mitad de x | la mitad de x |
| Combo | 3 + 3 + 3 + 5 = 14 | 5 + 5 + 8 = 18 |
| Ataque aéreo / corriendo | 5 / 5 | 7 / 7 |
| Golpe de agarre / lanzamiento | 2 / 6 | 4 / 10 |
| Arma arrojadiza | 6 | 6 |
| Especial A+B | 6 a todos los cercanos; cuesta 4 de vida si conecta | ídem |

El lanzamiento también hace su daño a quien choque con el lanzado.

**Reglas con cifra**

| Regla | Valor |
|---|---|
| Ventana de encadenado del combo | 12 frames |
| Invulnerable al levantarse | 60 frames, con parpadeo |
| Parada de impacto (golpe fuerte) | 4 frames |
| Destello del golpeado | 2 frames |
| Agarre | tras 10 frames andando contra el enemigo |
| `TOLERANCIA_Y` (profundidad para conectar) | 8 px |
| Relevo: enfriamiento | 120 frames |
| Relevo: golpe de entrada | 4 de daño, empuje a menos de 48 px |
| El que espera fuera | +1 de vida cada 120 frames, tope 48 (75 %) |
| Barra de ataque | +1 por golpe que conecta; llena con 48 |
| Comida (solo al activo) | onigiri +16, pescado asado +64 |
| Vidas / continues | 3 / 3 (decisión 12) |

**Tiempos propuestos en el código** (no estaban en la spec; se ajustan jugando, en
`src/juego/luchador.c` y `luchador.h`): recibir un golpe 12 frames; caer derribado a
3 px/frame hacia arriba; estar en el suelo 30 frames; levantarse 36; KO 40 frames
antes de salir de la pelea; el especial es invulnerable toda su animación (24 frames); un
golpe en el aire derriba.

**Enemigos por rango** (paleta del cinturón, `guion-historia.md` §2). La vida y el daño
salen de la tabla del rango; el pesado es siempre rojo.

| Rango | Vida | Bonus de daño | Puntos al vencerlo |
|---|---|---|---|
| blanco | 16 | +0 | 100 |
| amarillo | 24 | +1 | 150 |
| verde | 32 | +2 | 200 |
| rojo | 48 | +3 | 300 |

| Ataque enemigo | Daño base |
|---|---|
| shinai / patada (básico) | 4 |
| shuriken / tajo rápido (rápido) | 3 |
| golpe pesado (pesado) | 8 |
| lanzamiento (pesado) | 10 |

Velocidad base de los ninjas: 1 px por frame (inicial; el rápido y el pesado se ajustan
jugando).

**Jefes**

| Jefe | Vida | Notas |
|---|---|---|
| Gomasuri-dono (fase 1) | 160 | cambia de patrón al 50 %; carga 10, tajo de alcance 8 |
| Nigeashi-dono / Hankozuki-dono / Nebō-dono | 180 / 200 / 220 | fases 2–4 |
| Nikoraemon | 200 + 160 | etapa 1 con relevo; etapa 2 solo Tarou (`guion-historia.md` §6) |

**Puntuación**: golpe que conecta 10 · ninja según rango (arriba) · jefe 5000 · rompible
50 · comida 200. **Vida extra** a 50 000 y a 150 000 puntos, además de la escondida
(decisión 25). Sin `long` en tcc816: la puntuación se guarda en **dos `u16`** (o en BCD)
y se suma con acarreo a mano.

## 6. Enemigos

| Enemigo | Vida | Comportamiento | Ataques |
|---|---|---|---|
| Ninja básico | baja | se acerca, rodea, espera turno | golpe de shinai, patada |
| Ninja rápido | baja | mantiene distancia, cambia de carril | lanza shuriken (proyectil por y), tajo rápido |
| Kendoka pesado | alta | avanza despacio | bloquea golpes frontales (el combo rebota), agarra y lanza |
| Gomasuri-dono (jefe) | muy alta (160) | patrones por fases de vida | carga, tajo de alcance, convoca 1 ninja básico |

**Turnos de ataque**: como mucho **2** enemigos atacando a la vez (un «testigo»
que se pasan). Los demás rodean o esperan. Es la regla clásica para que 3
enemigos no se sientan injustos. También abarata la CPU, porque solo 2
calculan cajas de golpe.

**Variantes por paleta**: un mismo sprite con otra paleta = otra variante
(más vida, más rápido) sin arte nuevo.

## 7. La fase 1 y Tiled

| Zona | Qué pasa | Oleadas |
|---|---|---|
| 1. Aldea (1536 px) | calle con casas y puestos al atardecer; cajas y barriles con comida | 3 (2 blancos; 3 blancos; 2 blancos + 1 rápido) |
| 2. Bosque de bambú (1280 px) | sendero estrecho (carril más fino), viento; **escena de Lucinda** tras la oleada 1 y gag de Fenrir tras la 2 | 3 (2 rápidos + 1 amarillo; 3 amarillos; pesado rojo + 2 amarillos) |
| 3. Monasterio de Rex (768 px) | torii, faroles, de noche; **escena** con Rex al entrar (reconoce la reliquia); la barra se activa, **llena**; una oleada corta para probar el apoyo | 1 (3 amarillos) |
| 4. Dojo (1024 px) | puerta exterior (1 oleada) e interior: **Gomasuri-dono**, con su entrada con título (decisión 42); al vencerlo, escena: Nikoraemon huye con el Señor Pimentón | 1 (2 verdes + 1 rápido amarillo) + jefe |

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
  `vida`), `escena` (el monasterio), `jefe`, `salida`.
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
sombras a 30 Hz (frames alternos). **Medido (§9.1): cuando una línea pasa de
34 ranuras, la PPU descarta primero los sprites de índice MÁS BAJO de la OAM,
es decir, los de delante, y el protagonista el primero.** Rotar el orden no
sirve: chocaría con el orden por profundidad, que usa ese mismo orden de la
OAM para decidir quién tapa a quién. Así que **34 ranuras por línea es un
límite duro**, que se respeta con el diseño (cuántos enemigos, su ancho y su
reparto en profundidad). Las sombras, al final de la OAM, sobreviven igual.

**VRAM de sprites (512 tiles).** El protagonista activo usa **256** (doble
búfer de 2 celdas de 128). Quedan 256 para enemigos, sombras, efectos y
objetos. Un ninja de 64×96 a 6 piezas son **96 tiles por cuadro**: 3 ninjas
con un cuadro cada uno = 288, **no caben**. Opciones que mide la prueba:
- **A**: piezas de 16×16 recortadas al contorno (un humano de 64×96 ocupa
  ~60–70 % de su caja: ~64 tiles) → 3 × 64 = 192 + 64 para lo demás.
- **B**: celda de enemigo más estrecha (48×96).
- **C**: 2 ninjas «grandes» + 1 en una pose compartida residente.

**Reparto decidido tras la prueba (v3):** opción A, todo en piezas de 16×16
sin las vacías, empaquetadas en **pares de filas** de 1 KB (32 tiles: 8 piezas;
la pieza k de un cuadro va en el tile base + (k/8)·32 + (k%8)·2). Los 512 tiles
son 16 pares:

| Pares | Tiles | Quién | Cuánto cabe |
|---|---|---|---|
| 0–2 | 0–95 | protagonista, búfer A | 24 piezas (Tarou: 18 como mucho; Bárbara 64×96: hasta 24) |
| 3–5 | 96–191 | protagonista, búfer B | ídem: el cuadro nuevo sube aquí mientras se enseña el otro |
| 6–8 | 192–287 | enemigo 1 (un búfer) | 24 piezas (un ninja de 64×96: 15–21) |
| 9–11 | 288–383 | enemigo 2 | ídem |
| 12–14 | 384–479 | enemigo 3 | ídem |
| 15 | 480–511 | **compartido**: sombras (2 piezas), chispas, shuriken y kunai, objetos, «GO →» | 8 piezas residentes |

Un hueco puede cruzar la frontera entre las dos tablas de nombres (tile 256):
el bit 8 del tile va en el atributo de **cada pieza**, no del sprite entero. Las
columnas 12–15 libres de la celda vieja de Tarou desaparecen con este formato:
la sombra pasa al par 15. Retratos del HUD, apoyos y jefe no caben a la vez que
3 enemigos: los apoyos se dibujan con la acción congelada (los enemigos dejan
de subir cuadros y su par se presta) y el jefe sale con como mucho 1 ninja.

**Subidas:** el plan de copias de cada cuadro lo da el número de piezas, sin
tablas: los pares completos son filas de 512 B seguidas y el último par a medias
son 2 copias de q·64 B (fila de arriba y de abajo). Se reparte en trozos de
**512 B** (no de 1 KB): así el tope de 1,5 KB por VBlank del protagonista se
llena entero y un cuadro de 3 KB sigue tardando 2 VBlanks. Un enemigo sube su
cuadro entero en un solo VBlank (un búfer), uno por VBlank y por turnos.

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
  escenas, el halo del monasterio): suma con el **color fijo** (destellos,
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

### 9.1 Resultados (2026-10-23, Mesen, ROM FastROM)

Cómo se hizo: `src/pantallas/prueba_carga.c` (ROM con `-DPRUEBA_CARGA`) recorre
10 configuraciones de 6 s sin tocar el mando. El enemigo provisional es el
samurái humano del pack *FULL_Samurai* ampliado ×3 (93 px de alto) o ×2 (62 px),
en piezas de 16×16 recortadas (`scripts/preparar_prueba_carga.py`, que además
imprime la predicción de ranuras por línea). Tarou cambia de cuadro cada 2
frames, así que su medio cuadro (1,5 KB) sube en **todos** los VBlanks: es el
peor caso. Las cifras CPU son líneas contadas desde el VBlank en que empieza el
frame (262 = el frame no cupo); el HUD de depuración cuesta además ~25–60 líneas
por frame, que no entran en CPU pero sí en los frames perdidos.

**Tamaño de un cuadro de enemigo (piezas de 16×16 no vacías):**

| Escala | Andar | Atacar (espada fuera) | Bytes por cuadro |
|---|---|---|---|
| ×3 (93 px; ~75 px de ancho andando) | 21–25 piezas | 21–38 piezas | 3–5 KB |
| ×2 (62 px; ~50 px de ancho) | 11–13 piezas | 12–23 piezas | 2–3 KB |
| **×3 recortado a una celda de 64×96** (solo cálculo del script) | 18–21 piezas | 15–19 piezas | **1,9–2,7 KB exactos** |

Las dos primeras filas usan un lienzo de 128 px de ancho, más ancho que la
celda de 64×96 decidida (34), y en la ROM cada subida se redondea a pares de
filas de 1 KB. La tercera es lo que importa para la decisión: un humano en
64×96, con la espada cortada en el borde de la celda, como saldría pedido a
Pixellab. Bytes exactos = piezas × 128. Pide 84 tiles por hueco, así que caben
**3 huecos** en los 256 tiles libres.

**Medidas** (VMAX: última línea del VBlank usada, el límite es 37):

| # | Configuración | VMAX | CPU típica / máx | Resultado |
|---|---|---|---|---|
| 0 | Tarou solo | 19 | 110 / 189 | cabe |
| 1 | ×3, 1 ninja andando y atacando (5 KB) | **51** | — | se sale del VBlank: trozos con tiles viejos |
| 2 | ×3, 2 ninjas andando (3–4 KB) | **45** | — | se sale del VBlank |
| 3 | ×2, 2 ninjas andando y atacando (≤ 3 KB) | **39** | 178 / 325 | se sale en los cuadros de 3 KB |
| 4 | ×2, 3 ninjas, misma y, sombras | 32 | 215 / 326 | VBlank bien; CPU justa |
| 5 | ×2, 3 repartidos | 32 | 218 / 328 | ídem |
| 6 | ×2, 4 repartidos | 32 | 242 / 361 | CPU no cabe |
| 8, 9 | 2 subidas de enemigo por VBlank | **44** | — | se sale: el techo es 1 subida de ≤ 2 KB |

**Lo que se aprendió:**

1. **VBlank:** junto al medio cuadro de Tarou cabe **una subida de enemigo por
   VBlank**. Medido: 2 KB acaba en la línea 32 y 3 KB en la 39, unas 7 líneas
   por KB, así que el techo real está en **~2,7 KB** (interpolado, no medido).
   Un humano de 64×96 (hasta 2,7 KB exactos) queda **justo en el borde**: hace
   falta subir el tamaño exacto (2 entradas de DMA para el último par de filas a
   medias) y probablemente las **bandas de cine** (§8: ~16 líneas más de DMA).
   Con eso, 2–3 enemigos de 64×96 son viables por VBlank. Dos subidas de enemigo
   en el mismo VBlank no caben nunca (44).
2. **Ranuras por línea:** lo que se ve es coherente con la predicción del
   script: el corte cae dentro de la franja predicha (128–175), aunque es más
   estrecho (~160–176), porque los ninjas van en cuadros distintos y la
   predicción usa el peor cuadro para todos. Con Tarou
   (12 ranuras) y 3 ninjas ×2 a la misma altura se pasa de 34 en la franja de
   los pies, y lo que desaparece es **Tarou**. El SNESdev Wiki lo confirma:
   cuando hay demasiadas ranuras, la PPU recorre los sprites al revés y descarta
   primero los de **índice más bajo** de la OAM, que son los que están delante.
   Cuando hay demasiados sprites (32), en cambio, descarta los de índice alto.
   Las sombras, que están al final de la OAM, se ven aunque el protagonista
   pierda trozos. La configuración 7 (orden rotando) no demostró nada: Tarou
   está fijo en las entradas 0..5 y siempre es el de índice más bajo. En la
   celda de 64×96 (cálculo): Tarou + 2 enemigos caben (32); 3 a la misma altura
   no (40); 3 repartidos en profundidad se pasan solo en 12 líneas. Tarou, con
   sus 12 ranuras en piezas de 32×32, es lo que más gasta por línea: cortado en
   piezas de 16×16 sin las vacías bajaría.
3. **CPU (lo que de verdad manda):**
   - Dibujar a Tarou desde C (planificar su cuadro con `metasprite_copias` y
     escribir su OAM) cuesta **~88 líneas** por frame.
   - Llenar la OAM con `oamSet` + `oamSetEx` del SDK cuesta **~5,4 líneas por
     pieza**. Con 3 ninjas (37 piezas) el frame no cabe (CPU 360–440).
   - Con la rutina en ensamblador `juego/oam_piezas.asm` (sustituida por
     `sprites_oam.asm`, §9.2; queda en el commit `4b0811c`) baja a ~2,2 líneas por
     pieza, pero esa cifra es sobre todo la sobrecarga de las 8–9 llamadas desde C
     (8 parámetros cada una), no el bucle de ensamblador. Son lecturas del HUD de
     un solo frame, no promedios. Con 3 ninjas ×2, CPU
     215 típico, pero los picos (326) y el HUD siguen perdiendo frames.
   - Conclusión: la capa de sprites (orden, OAM y plan de subidas) tiene que
     ir entera en ensamblador, con una llamada por frame, y el plan de copias
     de cada cuadro precalculado por script en la ROM, no en tiempo de
     ejecución.
4. **VRAM:** con cuadros ≤ 2 KB caben 4 huecos de 64 tiles (andar). Los de
   ataque de 3 KB solo dejan 2.
5. **Sombras con media suma:** se ven bien sobre la tierra y la hierba (oscurecen
   al 50 %), con `OBSEL` 16/32 y piezas de 16×16. No hacen falta sprites de 8×8.
6. **Orden por profundidad** (`juego/profundidad.c`, probado en el PC): funciona
   (el ninja de más abajo tapa a los de arriba).

### 9.2 Resultados con la capa de sprites (v4, 2026-10-23)

La misma prueba, rehecha con lo decidido (35–38) y con la capa de sprites nueva:
ninjas de **64×96** (el samurái ×3 recortado a la celda: 15–21 piezas, 1,9–2,7 KB),
Tarou también en piezas de 16×16 (14–18 piezas) y **dentro** del orden por
profundidad (se pasea por delante y por detrás), reparto de la VRAM de §8 v3.

**Lo que se construyó** (todo con pruebas en el PC o en Python):

| Pieza | Qué hace |
|---|---|
| `scripts/piezas16.py` → `.p16` | tira PNG → piezas de 16×16 sin las vacías: índice de 8 bytes por cuadro, tabla (dx, dy), la **misma tabla espejada** y los tiles en orden de subida (n·128 bytes). El Makefile convierte así las 14 tiras de Tarou |
| `juego/p16.c` | lee el índice de un cuadro (lógica pura) |
| `juego/subida.c` | el plan de copias sale del número de piezas (pares completos: 2 × 512 B; el último a medias: 2 × q·64 B) y se reparte con un tope por VBlank en trozos de 512 B (lógica pura) |
| `juego/dma_copias.asm` | pasa las copias de una subida a la lista de DMA en un bucle (sin una llamada desde C por copia) |
| `juego/sprites.c` + `sprites_oam.asm` | tabla de entidades **fijas** (16 bytes cada una; cada frame solo se toca lo que cambia), orden por la y de los pies, y UNA llamada por frame que escribe toda la OAM. **Camino rápido**: si la celda entera está en la pantalla, cada pieza es una suma de 16 bits ((dx \| dy << 8) + (x0 \| y0 << 8)) y dos escrituras; si no, se comprueba cada pieza (x negativa con su bit 8, fuera de pantalla con y = 224) |

**Medidas** (CPU en líneas desde el VBlank en que empieza el frame; 262 = no
cupo. «Típica» es la lectura del HUD en un frame cualquiera; «máx», el máximo de
toda la configuración. El HUD de depuración suma 40–60 líneas más, que no van en
CPU pero sí en los frames perdidos; en el juego va apagado):

| Configuración | CPU v2 (oamSet)¹ | CPU v4 típica / máx | VMAX | Perdidos (con HUD) |
|---|---|---|---|---|
| Tarou solo | 110 / 189 | **52 / 76** | 17 | 0 |
| 1 ninja andando y atacando | — | 115 / 162 | 34 | 0 |
| 2 ninjas | — | 154 / 204 | 36 | 0 |
| 3 ninjas, misma y, sombras | 360–440 | **171 / 232** | 36 | 12 % |
| 3 ninjas repartidos, sombras | — | **134 / 240** | 36 | 4 % |
| 3 repartidos, solo andando | — | 133 / 241 | 36 | 9 % |
| 3 repartidos, sin sombras | — | 111 / 199 | 36 | 0 |
| 2 subidas de ninja por VBlank | — | 154 / 296 | **50** | — |

¹ No es una comparación exacta: la v2 tenía ninjas de 62 px (11–13 piezas); la
v4, de 64×96 (15–21). La mejora real es mayor de lo que dice la tabla.

Desglose con 3 ninjas repartidos (un frame típico): las subidas ~30–40 líneas
(la marca incluye el VBlank, que acaba en la 24–36), la tabla de entidades en C
~26, la OAM en ensamblador ~44 (unas 0,5 líneas por pieza; con `oamSet` eran 5,4).
Verificado a la vista en las capturas: los ninjas miran a Tarou, Tarou se espeja
según hacia dónde anda, queda detrás del ninja que está más abajo y delante del
que está más arriba, y cada uno tiene su sombra. (Una pasada anterior salió con
los ninjas a trozos arriba de la pantalla: la entrada fija no guardaba el banco
de su tabla de piezas. El HUD no lo veía: la escena hay que mirarla.)

**Conclusiones:**
1. Con 3 ninjas de 64×96, Tarou y sombras, el frame **cabe** (máx. 240 de 262)
   y el VBlank también (36 de 37: una subida de ninja por VBlank). Queda un
   margen corto para la lógica del juego en los peores frames: lo siguiente
   que se puede abaratar es planificar las subidas (en C).
2. Dos subidas de ninja en el mismo VBlank siguen sin caber (43): se mantiene
   «una por VBlank, por turnos» (espera máxima medida: 2 frames).
3. Tarou en piezas de 16×16 gasta 10 ranuras en su peor fila de andar (antes,
   12): según el script, 3 ninjas repartidos caben en 34 andando, y atacando
   solo se pasan las líneas de los pies, por las sombras.
4. El nivel de plataformas (`tarou_sprite.c`) se queda con su motor de antes
   hasta que se retire entero (§10); la capa nueva se estrena directamente en
   la fase del beat 'em up (decisión del autor, 2026-10-23).

## 10. Arquitectura (qué cambia en `src/`)

| Se queda | Cambia | Sale | Nuevo |
|---|---|---|---|
| `sistema/*`, `escena/*`, `dma_lista` + `dma_ejecutar.asm`, `metasprite`, `fondo_nivel` (columnas al anillo), `camara` (+ bloqueo de zona), `hud_debug`, `mando`, `guion` | `tarou.c` (estados de combate), `animaciones.c` (cajas), `tarou_sprite.c` → `protagonista_sprite.c` (celda compartida, dos paletas), `pantalla_nivel1.c` → `pantalla_fase1.c`, `pantalla_intro.c` (3 escenas nuevas), `barbara.c` y `cambio_protagonista.c` (de stub a código) | `colision.c` (rejilla de plataformas), `partida.c` (reglas del tutorial), `guion_tutorial.c`, `res/nivel1/*` del tutorial y sus pruebas (quedan en el tag) | `mundo.c` (x/y/z, carril, orden por y), `combate.c` (cajas, daño, agarre, turnos), `enemigos/ninja.c` y `enemigos/jefe.c`, `oleadas.c` (zonas y bloqueo de cámara), `objetos/*` (rompibles, comida, armas), `apoyos.c` + efectos de pantalla en `efectos.c`, `hud/hud_juego.c`, `pantallas/pantalla_monasterio.c` |

Las reglas de la casa siguen igual: lógica pura sin `snes.h` y probada en el
PC (ahora también `mundo`, `combate`, `oleadas`, IA, apoyos y doble toque),
tablas de structs a potencia de 2, sin divisiones en el frame, `static`
inicializadas, toda escritura a la VRAM por la lista de DMA y los registros
de scroll en el VBlank.

## 11. Fuera de alcance de la Revisión 3

Audio · cooperativo a 2 mandos · fases 2+ · etapas de vida de Tarou · el arte
final de la intro (placeholders) · un mapa de viaje entre fases.

## 12. Pendientes que no bloquean el diseño

- Arte de los jefes de las fases 2 a 4 (nombres ya fijados) y el resto de pendientes del
  guion (`guion-historia.md` §10).
- El sprite del **Señor Pimentón**: el autor lo tiene y lo aporta cuando toque; falta dejarlo en
  `res/descargas/graficos/` y decir su tamaño.
