# Arte de la Revisión 3: lista de trabajo y trucos de hardware

**Proyecto:** _Tarou: A Dog's Journey_ · **Revisión:** 3 · **Fecha:**
2026-10-23 · **Para qué sirve:** es la lista de todo el arte que necesita la
fase 1 del beat 'em up (`diseno-beat-em-up.md`): qué animaciones pedir a
Pixellab, qué escenografía dibujar por zona, qué objetos, efectos y HUD hacen
falta, y **qué recurso de la SNES usa cada pieza** para sacarle partido a la
máquina. Cada fila tiene una casilla de estado para ir marcándola.

Leyenda de estado: `[ ]` por pedir · `[~]` placeholder · `[x]` arte real en
el juego · `♻` se reutiliza lo que ya hay.

## 0. Reglas para todo el arte

- **Personajes**: una sola paleta de 15 colores + transparente; todo mirando
  al **este** (el espejo del hardware hace el oeste); los pies en la última
  fila de la celda; sin escalar. Se exporta en el formato nativo de Pixellab
  y entra por `scripts/preparar_personaje.py`, que necesita un parámetro de
  **tamaño de celda** (hoy 96×64 fijo; las celdas de 64×96 son nuevas).
- **Tira**: como mucho **10 cuadros de 3 KB** por animación (cada tira es una
  sección de `data.asm` y no puede cruzar un banco de 32 KB). Las
  animaciones más largas se parten en dos tiras.
- **Cajas de golpe y de daño**: cada animación de ataque necesita saber en qué
  cuadros golpea y con qué rectángulo. Se anotan en un `.json` junto a la
  exportación (§1.4); el script las pasa al `.h`.
- **Fondos**: 4bpp, 15 colores + transparente por paleta. BG1 y BG2
  **comparten 8 paletas en total**, y el HUD (BG3) ocupa la primera: cada
  zona tiene **7 paletas × 15 colores para sus dos capas juntas** (por
  ejemplo, 4 para la cercana y 3 para la lejana; `diseno-beat-em-up.md` §8).
  Tiles de 8×8 deduplicados con giro (`-F`). BG3 es 2bpp (4 colores): solo
  HUD y texto.
- **Sprites pequeños**: en pantalla solo hay dos tamaños (`OBSEL`), hoy 16×16
  y 32×32. **No hay 8×8**: una hoja o una luciérnaga se dibuja dentro de una
  pieza de 16×16 (y gasta 2 ranuras por línea).
- **Translúcido, no brillante**: las paletas de sprite 4–7 van con la **media
  suma** durante el combate (el modo de color es global). Lo que esté en
  ellas (sombras, chispas, objetos) sale mezclado con el fondo. El brillo
  fuerte (suma con el color fijo) solo existe en los momentos congelados:
  apoyos, pausa y escenas.
- **Ninjas: 64×96** (decidido tras la prueba de carga, `diseno-beat-em-up.md`
  §9.1). Un cuadro de enemigo tiene que quedarse en **≤ 21 piezas de 16×16 no
  vacías (≤ 2,7 KB)**: la espada de los ataques se dibuja dentro de la celda,
  sin salirse. Tarou, Bárbara y los enemigos se cortan en piezas de 16×16 sin las
  vacías (decisión 38).

## 1. Protagonistas

### 1.1 Tarou: 96×64, katana en la boca, rápido

| Animación | Cuadros | Origen | Estado |
|---|---|---|---|
| reposo (envainado) | 9 | Idle | ♻ |
| caminar (envainado) | 8 | Walk | ♻ |
| correr | 8 | Run (comprobar si se ve la espada; si no, pedir *correr en guardia*) | ♻ |
| saltar / caer | 3 + 3 | Jump | ♻ |
| desenvainar / envainar | 9 + 9 | Unsheathe (y al revés) | ♻ |
| recoger / comer | 1–3 | Sneak, cuadro 0 (agachado) | ♻ provisional |
| levantarse | 6–10 | Stand_up (se desenrosca: sirve de levantarse, provisional) | ♻ provisional |
| guardia (respira) | 4 | hoy es la pose fija Fight_Pose | [ ] |
| caminar en guardia | 8 | | [ ] |
| golpe 1 / 2 / 3 (cortes) | 4 / 4 / 5 | hoy solo hay poses fijas (Attack_1, Attack_2) | [ ] |
| golpe 4: mordisco (derriba) | 6 | | [ ] |
| ataque aéreo | 4 | | [ ] |
| agarre (sujetar) | 2 | | [ ] |
| golpe de agarre (zarpazo) | 3 | | [ ] |
| lanzamiento | 6 | | [ ] |
| agarrado (por el kendoka) | 2 | | [ ] |
| recibir golpe | 2 | | [ ] |
| derribado (vuela y cae) / en el suelo | 5 / 1 | también sirve para *lanzado* | [ ] |
| especial A+B: giro con la espada | 8 | | [ ] |
| lanzar arma arrojadiza | 4 | | [ ] |
| entrada de relevo (cae y golpea) | 6 | | [ ] |
| invocar apoyo (llamada) | 4 | | [ ] |
| KO | 6 | | [ ] |
| victoria | 6 | | [ ] |
| **Se retiran** | — | agachado, agachado_caminar, echarse, dormido, despertar, atacar (pose fija) | — |

### 1.2 Bárbara: 64×96, katana, alcance

Hoy solo tiene un sprite estático de 64×64 (`res/personajes/barbara_samurai.png`)
para la intro y el título. Necesita el **juego completo** en Pixellab
(carpeta `res/descargas/graficos/pixellab/barbara/`, ya creada con su README).

| Animación | Cuadros | Estado |
|---|---|---|
| guardia / caminar / correr | 4 / 8 / 8 | [ ] |
| saltar / caer | 3 / 3 | [ ] |
| golpe 1 / 2 (cortes de katana, lentos) | 5 / 5 | [ ] |
| golpe 3: tajo descendente (derriba) | 7 | [ ] |
| ataque aéreo (estocada hacia abajo) | 4 | [ ] |
| agarre / golpe de empuñadura / lanzamiento de judo | 2 / 3 / 7 | [ ] |
| agarrada / recibir / derribada / en el suelo / levantarse | 2 / 2 / 5 / 1 / 6 | [ ] |
| especial A+B: corte circular | 8 | [ ] |
| lanzar arma / recoger | 4 / 2 | [ ] |
| entrada de relevo / invocar apoyo | 6 / 4 | [ ] |
| KO / victoria | 6 / 6 | [ ] |
| intro: jura venganza (katana en alto) | 4 | [ ] |

### 1.3 Retratos del HUD

| Pieza | Tamaño | Cuadros | Estado |
|---|---|---|---|
| retrato de Tarou | 32×32 | 2 (normal, dolor) | [ ] |
| retrato de Bárbara | 32×32 | 2 | [ ] |

Van en **sprites** de 16 colores en las líneas de arriba (fuera del carril), no
en BG3, porque BG3 solo tiene 4 colores.

### 1.4 Formato de las cajas (`<animación>.cajas.json`)

```json
{ "cuadros": [ { "dano": [10, 8, 40, 52] },
               { "dano": [10, 8, 40, 52], "golpe": [52, 20, 36, 16], "efecto": "derribo", "valor": 12 } ] }
```
x, y, ancho, alto en píxeles dentro de la celda, mirando al este.

## 2. Enemigos: ninjas kendokas humanos (64×96 en principio)

| Animación | Básico (shinai) | Rápido (shuriken) | Pesado (agarra, bloquea) |
|---|---|---|---|
| guardia | 4 | 4 | 4 |
| caminar | 8 | 8 | 8 |
| salto lateral (cambia de carril) | — | 4 | — |
| ataque 1 | shinai 4 | tajo rápido 4 | golpe pesado 6 |
| ataque 2 | patada 4 | lanzar shuriken 4 | agarre 2 + lanzamiento 6 |
| bloqueo | — | — | 2 |
| recibir / derribado / en el suelo / levantarse | 2 / 5 / 1 / 5 | 2 / 5 / 1 / 5 | 2 / 5 / 1 / 6 |
| agarrado / lanzado | 2 / (= derribado) | 2 | 2 |
| KO (cae y parpadea) | 4 | 4 | 4 |
| **Total aprox.** | 39 cuadros | 43 | 47 |

**Variantes por paleta** (gratis en arte): cada tipo tiene 2–3 paletas; la
oscura tiene más vida y es más rápida. Solo cuesta copiar 32 bytes a la
CGRAM al aparecer.

### 2.1 Jefe y villano

| Personaje | Celda | Animaciones | Estado |
|---|---|---|---|
| Lugarteniente (jefe de la fase 1) | 96×96 a medir en la prueba de carga (sale con como mucho 1 ninja básico, que deja VRAM libre) | guardia 4, caminar 8, carga 6, tajo de alcance 6, convocar 4, recibir 2, derribado 5, levantarse 6, derrota 8, entrada 6 | [ ] |
| Villano | 64×96 | solo en escenas: aparece 4, coge el juguete 4, huye 6 | [ ] |

## 3. Apoyos

Entran, atacan y se van. Mientras actúan, la acción está congelada y la
segunda celda del protagonista (128 tiles) está libre para ellos.

| Apoyo | Celda | Animaciones | Estado |
|---|---|---|---|
| Gata samurái (muy rápida) | 64×64 | entrada 3, tajo ×3 (6), salida 3; escena: guardia 4 + saludo 4 | [ ] |
| Perro monje (telépata) | 96×64 | entrada 4, concentración 6 (bucle), soltar 4, salida 3; escena: guardia 4 + saludo 4 | [ ] |
| Perro bruto (embiste) | 96×64 | carrera 8 (bucle), choque 4, salida 3; escena: guardia 4 + saludo 4 | [ ] |
| Iconos de apoyo en el HUD | 16×16 | 3 + marco de selección | [ ] |

## 4. Objetos, efectos y armas (sprites pequeños)

| Pieza | Tamaño | Cuadros | Paleta | Estado |
|---|---|---|---|---|
| **Señor Pimentón** (el juguete) | el del autor (lo aporta el autor) | escenas | 7 | [ ] |
| sombra | óvalo en piezas de 16×16 (2 piezas = 32×16), 2 tamaños | 1 | 4 (media suma: oscurece) | [ ] |
| chispa de golpe / golpe fuerte | 16×16 / 32×32 | 3 / 4 | 5 (translúcida; dibujarla clara) | [ ] |
| nube de relevo | 32×32 | 5 | 5 | [ ] |
| polvo (derribo, bruto) | 16×16 | 4 | 4 | [ ] |
| shuriken (en vuelo, en el suelo) | 16×16 | 2 + 1 | 7 | [ ] |
| kunai (en vuelo, en el suelo) | 16×16 | 1 + 1 | 7 | [ ] |
| onigiri (cura poco) / pescado asado (cura mucho) | 16×16 | 1 / 1 | 7 | [ ] |
| vida extra (cabeza de Tarou) | 16×16 | 2 (brilla) | 7 | [ ] |
| barril, caja de madera, jarra de sake | 32×32 | intacto + rotura 3 | 7 | [ ] |
| farol de papel (aldea), puerta shoji (dojo) | 32×64 | intacto + rotura 3 | 7 | [ ] |
| «GO →» | 32×16 | 2 (parpadeo) | 5 | [ ] |
| hojas / pétalos / luciérnagas | dibujo pequeño en pieza de 16×16 | 2–3 | 7 (translúcidas) | [ ] |

## 5. Escenografía por zona (Mode 1)

En todas: **BG1** = capa cercana (carril, fachadas, primer plano con bit de
prioridad), **BG2** = capa lejana, **BG3** = HUD, color de fondo por HDMA.
Cada zona se carga con la pantalla apagada y puede tener su propio juego de
tiles (hasta 8 KB de VRAM por capa en el mapa actual).

### 5.1 Aldea (atardecer)

| Pieza | Capa | Truco | Estado |
|---|---|---|---|
| cielo naranja → violeta | color de fondo | **HDMA** de color por línea (♻ el del título) | [ ] |
| montañas, castillo lejano, sol bajo | BG2 | **scroll por franjas con HDMA** sobre `BG2HOFS`: montañas a 1/4, tejados lejanos a 1/2 con una sola capa. Tres planos de parallax con dos fondos | [ ] |
| calle de tierra (el carril), fachadas *machiya*, puestos con *noren* | BG1 | anillo de columnas (♻ `fondo_nivel.c`) | [ ] |
| postes de farol, barriles apilados en primer plano | BG1 alto | **bit de prioridad**: tapan al personaje que pasa por detrás (prioridad 2) | [ ] |
| *noren* ondeando | BG1 | **animación de tiles**: cambiar 4 tiles (128 B) por DMA cada 8 frames | [ ] |
| humo, pájaros | sprites | prioridad baja en la OAM (son los primeros que se pierden si una línea se llena) | [ ] |

### 5.2 Bosque de bambú

| Pieza | Capa | Truco | Estado |
|---|---|---|---|
| verde claro → niebla | color de fondo | HDMA | [ ] |
| bambú del fondo | BG2 | **ondulación por HDMA**: una tabla de seno de `BG2HOFS` por línea; se anima moviendo el puntero de la tabla (♻ la técnica del destello del logo: reescribir 2 bytes) | [ ] |
| sendero estrecho, rocas, linterna de piedra (*tōrō*) | BG1 | carril más fino (`carril_arriba/abajo` de Tiled) | [ ] |
| tallos de bambú en primer plano | BG1 alto | prioridad: el combate pasa entre los tallos | [ ] |
| rayos de luz | ventana + color | **ventana 1 por HDMA en diagonal** sobre BG1/BG2. El modo del combate es media suma con la secundaria; que convivan con las sombras **se mide en la prueba de carga**. Si no, se pintan en el propio tileset | [ ] |
| hojas cayendo | sprites de 16×16 | paleta 7, pocas a la vez (cada una gasta 2 ranuras) | [ ] |

> **Nota (guion, 2026-10-23):** Tarou es un perro pastor (pañuelo naranja), no un husky; el
> husky es Fenrir. El «santuario» es ahora el **monasterio de Rex**. El jefe de la fase 1
> es Gomasuri-dono (humano gordo, armadura de colores chillones) y los ninjas comunes son
> kendokas con bogu negro, con paleta distinta por rango. Ver `guion-historia.md`.

### 5.3 Monasterio de Rex (noche)

| Pieza | Capa | Truco | Estado |
|---|---|---|---|
| cielo azul profundo, luna | color de fondo + BG2 | HDMA de color; la luna con colores claros en el tileset | [ ] |
| siluetas de cedros | BG2 | scroll por franjas | [ ] |
| explanada, *torii* rojo, faroles de piedra, *shimenawa*, árbol sagrado | BG1 (torii en alto) | prioridad | [ ] |
| halo de los faroles | ventana + color | **ventana circular**: una tabla HDMA da el borde izquierdo y derecho de cada línea (un círculo) y suma el color fijo dentro. **Solo en la escena** (momento congelado, sin sombras); en el combate, el halo va pintado | [ ] |
| luciérnagas | sprites de 16×16 | paleta 7, translúcidas | [ ] |
| **escena de los apoyos** | Mode 1 + motor de escenas | retratos de los 3 en sprites, cuadros de texto en BG3, fundidos y mosaico (♻ `escena.c`) | [ ] |
| **activación de la barra** (propuesta, no decidida) | **Mode 7** | un sello (*mandala*) de 128×128 a 256 colores que **gira y se acerca** (matriz de Mode 7 cambiada cada frame); después mosaico y vuelta al Mode 1. Pantalla aparte, cargada con la pantalla apagada | [ ] |

### 5.4 Dojo

| Pieza | Capa | Truco | Estado |
|---|---|---|---|
| exterior: muro, gran puerta, cerezos | BG1/BG2 | pétalos en sprites | [ ] |
| interior: suelo de madera, paredes *shoji*, pergaminos, armero de shinai | BG1 | puertas shoji rompibles (§4) | [ ] |
| vigas del techo | BG2 | **perspectiva por HDMA**: cada línea del techo con su propio `BG2HOFS` (las cercanas se mueven más) y las vigas convergen al hacer scroll. Solo fuera del carril, para que los pies no «patinen» | [ ] |
| entrada del jefe | ventanas | **cortinilla** de ventana que cierra, nombre del jefe y banda de cine | [ ] |
| impactos del jefe | scroll | **temblor de pantalla** (±2 px en el scroll del VBlank) | [ ] |

## 6. Efectos de pantalla (sin arte de personaje)

| Efecto | Recurso | Dónde |
|---|---|---|
| sombras translúcidas | sprites de paleta 4 negros + **media suma** con la pantalla secundaria (BG1 y BG2 en la secundaria): oscurecen el 50 % de lo que hay debajo. Es el **modo de color del combate**, global para las paletas 4–7. **Verificar en la prueba de carga** | todo el juego |
| destello del golpeado | copiar una paleta blanca a su entrada de la CGRAM 2 frames (32 B por la lista de DMA) | combate |
| parada de impacto | congelar a atacante y objetivo 3–4 frames | combate |
| tajos de la gata | **ventanas 1 y 2** con tablas HDMA en diagonal + suma del color fijo = 3 rayas de luz; destello blanco con `COLDATA`. Modo especial: la acción está congelada y las sombras escondidas | apoyo |
| poder del monje | **mosaico** pulsante + ondulación HDMA de BG1/BG2 | apoyo |
| embestida del bruto | temblor de pantalla + polvo | apoyo |
| pausa | **resta de color** a la mitad (pantalla oscurecida) + «PAUSE» en BG3 | pausa |
| paso de zona | fundido + mosaico (♻ motor de escenas) | entre zonas |
| bandas de cine | *forced blank* en las últimas ~16 líneas (IRQ de línea): más tiempo de DMA y aspecto de cine en las escenas | a decidir en la prueba de carga |

## 7. HUD del juego (BG3, 2bpp)

| Pieza | Estado |
|---|---|
| marco superior, barras de vida de Tarou y Bárbara (la activa resaltada), barra de ataque (con candado hasta el monasterio) | [ ] |
| icono del apoyo elegido (sprite 16×16, §3), vidas, continues, puntuación (la fuente actual ♻) | [ ] |
| nombre y barra del enemigo golpeado (como *Final Fight*) | [ ] |
| textos: «PAUSE», «GO», «STAGE CLEAR», «CONTINUE?», «GAME OVER», «STAGE 2 / COMING SOON» (en inglés) | [ ] |

El HUD de depuración comparte el mapa de BG3: usa otras filas.

## 8. Intro (3 escenas, Mode 3 de 256 colores, ♻ `pantalla_intro.c`)

Placeholders en la Revisión 3 y arte final después.

| Escena | Qué se ve | Estado |
|---|---|---|
| 1 | Jardín de casa: Tarou juega con el **Señor Pimentón**; Bárbara lo mira | [~] |
| 2 | Los ninjas kendokas irrumpen; el villano se lleva al Señor Pimentón | [~] |
| 3 | Bárbara jura venganza con la katana en alto; Tarou, decidido. Fundido al título | [~] |

## 9. Cuánta ROM ocupa (estimación)

| Bloque | Cuadros | Por cuadro | Total |
|---|---|---|---|
| Tarou | ~150 | 3 KB | ~450 KB |
| Bárbara | ~150 | 3 KB | ~450 KB |
| 3 ninjas | ~130 | ≤ 3 KB | ~390 KB |
| Lugarteniente + villano | ~70 | ≤ 4,5 KB | ~280 KB |
| Apoyos | ~70 | 2–3 KB | ~180 KB |
| 4 zonas (tiles + mapas) | — | — | ~250 KB |
| Intro (3 imágenes de 256 colores) + Mode 7 del monasterio | — | — | ~220 KB |
| **Total** | | | **~2,2 MB** de los ~3,7 MB libres |

Cabe en la ROM de 4 MB. Lo que aprieta no es la ROM sino la VRAM y el VBlank
de cada frame (`diseno-beat-em-up.md` §8).
