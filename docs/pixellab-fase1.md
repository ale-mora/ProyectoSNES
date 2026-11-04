# Pixellab: sprites y ambientación de la fase 1

**Proyecto:** _Tarou: A Dog's Journey_ · **Fecha:** 2026-10-26 · **Para qué sirve:**
es el encargo de arte para Pixellab de la fase 1 del beat 'em up (aldea → bosque
→ monasterio → dojo). Cada entrada es un trabajo de Pixellab: qué personaje o
pieza es, en qué celda cabe, qué altura debe tener, qué paleta usa, qué
animaciones pedir (con cuántos cuadros) y un prompt en inglés para cada una.

Escrito para: el autor, que genera el arte en Pixellab.

**Fuentes y prioridad.** `diseno-beat-em-up.md` (Rev. 3) manda en lo técnico;
`arte-beat-em-up.md` da las listas de animaciones y los trucos de hardware;
`guion-historia.md` manda donde choca con la spec (nombres, personajes, orden de
las escenas). Lo que sigue en duda está al final, en §9.

**Estado.** `[ ]` por pedir · `[~]` placeholder · `[x]` arte real · `♻` ya existe
(se reutiliza).

---

## 0. Reglas comunes (todo lo que sale de Pixellab)

- **Documentación oficial del MCP de Pixellab: <https://api.pixellab.ai/mcp/docs>.**
  Hay que tenerla en cuenta antes de generar con el MCP (configurado desde el
  2026-11-04 como servidor `pixellab`): qué herramienta usar para cada cosa
  (`create_character_state`, `animate_character` y sus modos `template`,
  `v3`, `skeleton-v3`, `pixminimax`), cuánto cuesta cada una y cómo se
  descarga el resultado. Manda sobre lo que diga este documento si chocan.
  Los trabajos tardan 2–5 minutos y no bloquean; la descarga es
  `/mcp/characters/{id}/download` (un ZIP con la misma estructura que la
  exportación a mano, que `scripts/preparar_personaje.py` ya lee).
- **Una paleta de 15 colores + transparente por personaje.** Todo mira al
  **este** (la derecha): el espejo del hardware hace el oeste. Sin sombra
  proyectada ni suelo; fondo transparente de verdad (alfa 0 o 255, sin
  semitransparencias).
- **Los pies en la última fila de la celda.** El script no escala: la figura se
  dibuja tal como sale de Pixellab. Por eso la **altura de la figura** de cada
  personaje se fija aquí (§0.1) y se pide así en el prompt.
- **Lienzo.** Pixellab exporta lienzos de 76 a 104 px (`diseno-demo-jugable.md`),
  así que un lienzo cuadrado de 96 px sirve para todo. La celda (96×64 o 64×96)
  la recorta el script. Comprobar en la interfaz de Pixellab el tamaño que
  admite antes de generar.
- **Tira:** como mucho **10 cuadros** por animación, y que la tira entera quepa
  en un banco de 32 KB (`arte-beat-em-up.md` §0). El tope real depende de las
  piezas de 16×16 de cada cuadro (§0.2).
- **Pedir los cuadros exactos.** Si la herramienta no deja fijar el número de
  cuadros, pedir más y recortar en el script. Las cifras de las tablas son las
  que se quieren.
- **Dos cajas por animación de ataque** (`<animación>.cajas.json`, arte §1.4):
  dónde golpea el arma, en qué cuadro, y la caja de daño. Se anota junto a la
  exportación.

### 0.1 Altura de la figura (escala común)

Medido sobre el arte real de Tarou: su figura de pie mide **57 px de alto** y
61–64 px de ancho en una celda de 96×64. La prueba de carga (`diseno-beat-em-up.md`
§9.2) usó un humano de **93 px** y se midió que cabe. Con eso, la escala es:

| Personaje | Celda | Altura de la figura (píxeles visibles) | Referencia |
|---|---|---|---|
| Tarou | 96×64 | **57** (♻) | su arte actual |
| Rex, Fenrir (perros) | 96×64 | **56–57**, como Tarou | ver §4 |
| Lucinda (gata) | 64×64 | **52** | ver §4 |
| Bárbara | **144×112** (2026-11-04) | **90–93** (el arte definitivo de «Bárbara 2», con katana) | con la katana mide hasta 133 px de ancho |
| Nikoraemon | 64×96 | **88–90** | prueba de carga (93) |
| Ninjas, kendokas | **96×96** (2026-11-04) | **74–81** (el arte definitivo de «Ninja 1») | tumbado mide hasta 88 px de ancho |
| Gomasuri-dono (jefe) | 96×96 | **90** (riesgo de presupuesto, §3.1) | ver §3 |

**Aviso:** `res/personajes/barbara_samurai.png` (el sprite de Bárbara de la
intro) **no sirve de escala**: es un sprite de pack de 64×64 con una figura de
**36 px** de alto. Para escalar, usar la figura de Tarou y la altura de la tabla.

### 0.2 Presupuesto de piezas (lo que cabe en la VRAM y en el VBlank)

El límite de un cuadro de personaje es **21 piezas de 16×16 no vacías** (unos
2,7 KB; `arte-beat-em-up.md` §0 y `diseno-beat-em-up.md` §9.1). Un cuadro de
**96×96** entero tiene 36 piezas y no cabe: hay que dejar el personaje en 21
como mucho, sin contar las vacías.

| Celda | Bytes por cuadro lleno | Cuadros por tira (≤ 32 KB) | Tope de trabajo |
|---|---|---|---|
| 96×64 (Tarou, Rex, Fenrir) | ≤ 3 KB | 10 | 10 |
| 64×96 (Nikoraemon) | ≤ 2,7 KB | 10 | 10 |
| 144×112 (Bárbara) | ≤ 3 KB: el tope son las **24 piezas** del búfer de la protagonista, no la celda | 10 | 10 |
| 96×96 (ninjas, kendokas) | ≤ 2,7 KB: siguen siendo ≤ 21 piezas, las vacías no se guardan | 10 | 10 |
| 96×96 (Gomasuri-dono) | ~4,6 KB si está lleno | ~6 | **6** por tira; si hace falta más, partirla en dos |
| 64×64 (Lucinda) | ≤ 2 KB | 10 | 10 |

### 0.3 Paletas de sprite (CGRAM, 8 paletas de 16 colores)

Cada paleta de sprite tiene un uso fijo, y las **4–7** son las que la matemática
de color afecta (media suma en combate: lo que esté ahí sale translúcido):

| Paleta | Uso | Qué se pide a Pixellab |
|---|---|---|
| 0 | protagonista activo (Tarou o Bárbara, se intercambian con L) | Tarou ♻, Bárbara (§1.2) |
| 1 | ninja, color 1 de la oleada (cada color es un traje distinto) | §2 |
| 2 | ninja, color 2 de la oleada | §2 |
| 3 | ninja, color 3 de la oleada; **kendokas** solo en la pelea del jefe (§2.5) | §2 y §3 |
| 4 | sombras (negras), polvo | a mano, y polvo (§5) |
| 5 | chispas, nube de relevo | §5, translúcidas: dibujarlas claras |
| 6 | apoyo activo (solo uno a la vez: se cambia al invocarlo); **Gomasuri-dono** en el dojo (no hay apoyo invocado allí) | §4 y §3 |
| 7 | objetos, armas arrojadizas, hojas, luciérnagas | §5 y §6, **todo en una sola paleta de 15 colores** |

**Ojo con tres cosas:**

1. **Cada color de traje es una paleta en pantalla.** Hay 3 paletas de ninja
   (1, 2 y 3), así que una oleada tiene como máximo **3 colores de ninja** a la
   vez. El color se fija por oleada (§2.4).
2. **La paleta 7 es de todos los objetos a la vez.** Los barriles, los
   shuriken, las hojas y las luciérnagas deben salir de **una misma sesión** de
   Pixellab, o se reducen juntos con un script, para que compartan los 15 colores.
3. **Cada zona tiene 7 paletas × 15 colores para BG1 y BG2 juntas** (`arte-beat-em-up.md` §0):
   por ejemplo 4 para la capa cercana y 3 para la lejana. Se pide cada zona
   como un solo juego de tiles y de imagen de fondo, nunca una zona con más de 7.

---

## 1. Protagonistas

### 1.1 Tarou (96×64, altura 57) ♻

Tiene ya 13 animaciones y el guardia (el arte de `res/descargas/graficos/pixellab/tarou/`).
Para la fase 1 faltan las de **combate**. Todo mira al este y se exporta en el
formato nativo de Pixellab (`MAPEOS_PIXELLAB`). Tarou lleva la katana **en la
boca**: cada prompt lo dice.

Base del prompt, común a todas las de combate:
`pixel art, full body side view facing right, grey and white dog with an orange neckerchief (the look of res/descargas/graficos/pixellab/tarou/Idle/rotations/east.png), holding a katana sideways in its mouth, same character as the reference, 15 colors max, transparent background`

| Animación | Cuadros | Prompt de acción (inglés) | Estado |
|---|---|---|---|
| reposo, caminar, correr, saltar, caer, desenvainar, envainar | 9, 8, 8, 3+3, 9+9 | ♻ tal como están | ♻ |
| guardia (respira) | 4 | `The dog holds a low guard stance with the katana in its mouth, breathing gently` | [ ] |
| caminar en guardia | 8 | `The dog walks forward slowly in a combat guard stance, katana in its mouth` | [ ] |
| golpe 1 (corte) | 4 | `The dog swings the katana in a quick horizontal slash from its mouth` | [ ] |
| golpe 2 (corte) | 4 | `The dog follows with a second quick slash, the katana sweeping back` | [ ] |
| golpe 3 (corte) | 5 | `The dog makes a stronger, wider slash with the katana, leaning in` | [ ] |
| golpe 4: mordisco (derriba) | 6 | `The dog lunges and bites forward with the katana in its jaws, knocking an enemy down` | [ ] |
| ataque aéreo | 4 | `The dog strikes downward with the katana while in mid air` | [ ] |
| agarre (sujetar) | 2 | `The dog grabs an enemy with its front paws and holds on` | [ ] |
| golpe de agarre (zarpazo) | 3 | `The dog swipes with its paw while holding an enemy` | [ ] |
| lanzamiento | 6 | `The dog throws an enemy forward with a strong shake of its head` | [ ] |
| agarrado (por el enemigo) | 2 | `The dog is held from behind, struggling` | [ ] |
| recibir golpe | 2 | `The dog recoils from a hit, head snapping back` | [ ] |
| derribado (vuela y cae) | 5 | `The dog is knocked back and flies backwards, then lands on its back` | [ ] |
| en el suelo | 1 | `The dog lies flat on the ground, dazed` (un cuadro) | [ ] |
| levantarse | 6 | `The dog gets back on its feet, shaking off the dust` (♻ provisional: sale de Stand_up) | [ ] |
| especial A+B (giro) | 8 | `The dog spins fully around with the katana in its mouth, a wide circular slash` | [ ] |
| lanzar arma arrojadiza | 4 | `The dog throws a shuriken with a sideways toss of its head` | [ ] |
| entrada de relevo (cae y golpea) | 6 | `The dog jumps in from above and slams down onto the ground` | [ ] |
| invocar apoyo | 4 | `The dog raises its head and howls to call a companion` | [ ] |
| KO (cae y queda) | 6 | `The dog collapses sideways, eyes closed` | [ ] |
| victoria | 6 | `The dog wags its tail and sits proudly with the katana held high` | [ ] |
| **Se retiran** (no pedir): agachado, agachado_caminar, echarse, dormido, despertar, atacar (pose fija) | — | — | — |

Las **cajas** de golpe de los ataques (`.cajas.json`) se hacen sobre el arte
una vez salido, no antes.

### 1.2 Bárbara (144×112, altura 90–93) [x]

**Hecho (2026-11-04).** «Bárbara 2» con katana es la definitiva. Las 25
animaciones salen del estado `Fighting_pose_with_a` (21 generadas con el MCP,
skeleton-v3, solo el este), salvo `golpe_3` (estado `Attack_1`) y `especial`
(estado `Fighting_pose_with_a_2`, katana en llamas). Celda de **144×112**: la
katana llega a 133 px de ancho. Cada cuadro cabe en las **24 piezas** del búfer de
la protagonista con `piezas16.py --rejilla-optima`; para eso se quitaron 5
cuadros (golpe_3 6–7, especial 2 y 4, recoger 1) y el último del KO, que salió
deformado. El mapeo está en `MAPEOS_PIXELLAB["barbara"]`. Lo de abajo es el
encargo original.

Bárbara no tiene arte de juego. El `res/personajes/barbara_samurai.png` es un
sprite de pack de 64×64 (figura de 36 px): **no es referencia de escala**. Hay
que generarla entera. Su carpeta ya existe: `res/descargas/graficos/pixellab/barbara/`
(su README está **desactualizado**: habla de 64×64 y 15 cuadros; manda esta lista).

Base del prompt:
`pixel art, full body side view facing right, young Japanese samurai woman with a high ponytail, blue kendo gi with white hakama, holding a katana in one hand, 15 colors max, transparent background, no ground`

| Animación | Cuadros | Prompt de acción (inglés) | Estado |
|---|---|---|---|
| guardia | 4 | `The samurai woman holds a high guard with the katana, breathing` | [ ] |
| caminar | 8 | `The samurai woman walks forward with a steady stride, katana held low` | [ ] |
| correr | 8 | `The samurai woman runs forward with her katana held at the side` | [ ] |
| saltar | 3 | `The samurai woman jumps up with her katana tucked back` | [ ] |
| caer | 3 | `The samurai woman falls back down, knees bent, katana up` | [ ] |
| golpe 1 (corte, lento) | 5 | `The samurai woman makes a slow, wide horizontal katana cut` | [ ] |
| golpe 2 (corte, lento) | 5 | `The samurai woman follows with a second slow cut from the other side` | [ ] |
| golpe 3: tajo descendente (derriba) | 7 | `The samurai woman raises the katana high and brings it down in a strong diagonal strike` | [ ] |
| ataque aéreo (estocada hacia abajo) | 4 | `The samurai woman thrusts her katana downward in mid air` | [ ] |
| agarre | 2 | `The samurai woman grabs an enemy by the collar` | [ ] |
| golpe de empuñadura | 3 | `The samurai woman strikes with the katana's hilt, a short hit` | [ ] |
| lanzamiento de judo | 7 | `The samurai woman throws an enemy over her hip` | [ ] |
| agarrada | 2 | `The samurai woman is held from behind, struggling` | [ ] |
| recibir golpe | 2 | `The samurai woman staggers from a hit` | [ ] |
| derribada | 5 | `The samurai woman is knocked back and flies backwards` | [ ] |
| en el suelo | 1 | `The samurai woman lies on the ground, dazed` | [ ] |
| levantarse | 6 | `The samurai woman gets back up to her feet` | [ ] |
| especial A+B: corte circular | 8 | `The samurai woman spins with the katana in a full wide circle` | [ ] |
| lanzar arma | 4 | `The samurai woman throws a shuriken with a sharp overhand toss` | [ ] |
| recoger | 2 | `The samurai woman bends down and picks up an item` | [ ] |
| entrada de relevo | 6 | `The samurai woman drops in from above and lands hard` | [ ] |
| invocar apoyo | 4 | `The samurai woman raises her katana to call a companion` | [ ] |
| KO | 6 | `The samurai woman collapses, eyes closed` | [ ] |
| victoria | 6 | `The samurai woman sheathes the katana with a proud pose` | [ ] |
| intro: jura venganza (katana en alto) | 4 | `The samurai woman raises the katana high and vows revenge` | [ ] |

**Dos cosas que no se ven en la tabla:** la katana debe quedar **dentro de la
celda** de 64 px de ancho en el tajo (el golpe 3 es el más largo); y el
**retrato** de HUD (§1.3).

### 1.3 Retratos

| Pieza | Tamaño | Cuadros | Paleta | Estado |
|---|---|---|---|---|
| Tarou, normal y dolor (HUD) | 32×32 | 2 | sprites 16 colores (fila superior, fuera del carril) | [ ] |
| Bárbara, normal y dolor (HUD) | 32×32 | 2 | ídem | [ ] |
| Retratos de escena (cuadros de texto): Tarou, Bárbara, Rex, Lucinda, Nikoraemon, Gomasuri-dono | **64×64 (propuesta)** | 1 cada uno | sprites | [ ] |

Para los **retratos de escena** la medida es una propuesta: un busto de 64×64
(16 piezas de 16×16) con el mismo estilo de paleta. Confirmar en §9.

Prompt base de busto: `pixel art bust portrait, head and shoulders, facing right, [personaje], expressive face, 15 colors max, transparent background`

---

## 2. Enemigos: ninjas (celda 96×96, altura 74–81)

**Celda (2026-11-04): 96×96, no 64×96.** De pie, «Ninja 1» mide 74–81 px de alto
y 42–55 de ancho, pero derribado, en el suelo y en el KO llega a 73–88 px de
ancho y en 64 se recortaba. Las piezas vacías no se guardan, así que la celda
más ancha no cuesta VRAM; solo las poses anchas usan más sprites por línea.
Al integrar el arte, `NINJA_ANCHO` (`src/enemigos/ninja.h`) pasa de 64 a 96.

**Actualizado (2026-10-26):** los enemigos normales son **ninjas de varios colores
de traje**. Los **kendokas** (con bogu y shinai) aparecen **solo con el jefe**
(§2.5). El guion (§2) habla de kendokas de bogu negro para todos; esta lista sigue
la decisión nueva del autor.

### 2.1 Base del prompt (ninja, común a todos los colores)

`pixel art, full body side view facing right, ninja in a [COLOR] outfit with a face mask and headband, a flat-colored sash in one solid color used nowhere else on the body, clumsy comedic posture, limited palette of 15 colors, flat shading, no anti-aliasing, transparent background, no ground, no shadow`

Sustituye `[COLOR]` por el color del traje. Cada color que pidas será una paleta
distinta (§2.4), así que pide el mismo personaje con el resto del texto igual.

### 2.2 Tipos de ninja

**Decidido (2026-10-26): un solo personaje ninja base, con varios colores de traje por
paleta.** El color cambia la paleta, no la silueta. Todos los ninjas tienen el mismo
conjunto de ataques: **golpe, lanzamiento de shuriken, bloqueo y agarre**.

| Animación | Cuadros | Notas |
|---|---|---|
| guardia | 4 | |
| caminar | 8 | |
| ataque 1 (golpe con espada corta, propuesta) | 4 | |
| ataque 2 (lanzar shuriken) | 4 | el proyectil es el shuriken de §5 |
| bloqueo | 2 | |
| agarre + lanzamiento | 2 + 6 | el lanzamiento es el mismo que el del protagonista |
| recibir golpe | 2 | |
| derribado / en el suelo / levantarse | 5 / 1 / 5 | |
| agarrado | 2 | |
| KO (cae y parpadea) | 4 | |
| **Total** | **49** | |

**Prompts de acción:**
- guardia: `The ninja holds a short blade in a clumsy guard, swaying`
- caminar: `The ninja walks forward, blade held loosely`
- ataque 1: `The ninja swings the short blade in a sloppy overhead strike`
- ataque 2: `The ninja flicks a shuriken forward with a clumsy throw`
- bloqueo: `The ninja braces with a wide guard and raises its arms to block`
- agarre: `The ninja lunges forward and grabs its opponent`
- lanzamiento: `The ninja hurls the grabbed opponent forward`
- recibir: `The ninja flinches back from a hit`
- derribado: `The ninja is knocked over backwards`
- KO: `The ninja falls and lies still, the mask askew`

*Propuesta:* el ataque 1 es una espada corta, no un shinai (el shinai es de los
kendokas). Confirmar al ver el arte.

**Pendiente de medir:** el cuadro de agarre y el de lanzamiento pueden pasar de las
21 piezas de §2.3. Se miden en la prueba de carga antes de dar el arte por bueno.

### 2.3 Presupuesto de un cuadro

Un ninja de 88 px de alto cabe en **15–21 piezas** no vacías (la prueba
de carga, `diseno-beat-em-up.md` §9.2). **La espada del ataque se queda dentro de la
celda**: si sale del borde, el cuadro se pasa de piezas.

### 2.4 Colores de traje (variantes por paleta)

- **Hasta 3 colores por oleada** (paletas 1, 2 y 3). Cada color es un traje distinto
  del mismo personaje base, así que se pide una vez y los demás colores se derivan
  cambiando la paleta.
- **Un color por paleta en pantalla**: los ninjas de una misma oleada comparten el
  color de su paleta. Para tener más colores a la vez, haría falta otra paleta (§0.3).
- **Gomasuri-dono usa la paleta 6** (decidido), y los kendokas de su pelea usan las
  paletas 1 a 3 (§2.5). Hay que comprobar que en el dojo no haya un apoyo activo
  en la paleta 6.

### 2.5 Kendokas (solo con el jefe)

Los kendokas tienen **varios colores de traje**, como los ninjas, y **aparecen solo
en la pelea del jefe**, donde no hay ninjas (decidido). Por eso usan las paletas 1
a 3 sin conflicto. Son humanos con **bogu** (armadura de kendo) y **shinai**; su
celda es 96×96, igual que la de los ninjas (tumbados tampoco caben en 64).

Prompt base: `pixel art, full body side view facing right, Japanese kendo fighter in [COLOR] bogu armor with a men mask, holding a bamboo shinai, a flat-colored belt in one solid color, limited palette of 15 colors, flat shading, no anti-aliasing, transparent background, no ground, no shadow`

Cuadros: los mismos que el ninja base (§2.2), con **ataque 2 = patada** en vez de
lanzar shuriken, y sin lanzamiento de shuriken. Prompts de acción: igual que §2.2,
cambiando `The ninja` por `The kendo fighter`, y `blade` por `shinai`.

---

## 3. Jefe y villano

### 3.1 Gomasuri-dono (lugarteniente, jefe de la fase 1) — celda 96×96, altura 90

Humano gordo y pomposo, armadura de samurái de colores chillones. Adulador: dice
que sí a todo (guion §2). Es el **jefe** de la fase 1 y usa la paleta 6 (decidido).

Base del prompt:
`pixel art, full body side view facing right, fat pompous Japanese samurai lord, loud bright armor in magenta, lime green and orange, tall lacquered helmet, a folding fan in one hand, 15 colors max, transparent background, no ground`

| Animación | Cuadros | Prompt de acción (inglés) | Estado |
|---|---|---|---|
| guardia | 4 | `The fat lord stands with a fan and an overconfident grin, swaying` | [ ] |
| caminar | **6** (la tira no admite más: ver §0.2) | `The fat lord waddles forward proudly` | [ ] |
| carga | 6 | `The fat lord charges forward with a loud shout, arms wide` | [ ] |
| tajo de alcance | 6 | `The fat lord swings a long halberd in a wide sweep` | [ ] |
| convocar (1 ninja básico) | 4 | `The fat lord claps his hands and calls for backup` | [ ] |
| recibir golpe | 2 | `The fat lord staggers, eyes wide with surprise` | [ ] |
| derribado | 5 | `The fat lord falls backwards onto the ground` | [ ] |
| levantarse | 6 | `The fat lord struggles to his feet, pompous even while standing up` | [ ] |
| derrota | 8 | `The fat lord collapses in a heap and cries out` | [ ] |
| entrada | 6 | `The fat lord arrives on the scene with a dramatic leap` | [ ] |

**Aviso de presupuesto:** un jefe lleno en 96×96 pasa de las **21 piezas** del
límite del enemigo. **Decidido (2026-10-26): medirlo en la prueba de carga antes
de pedir el arte final.** Si no cabe, el jefe se pide más estrecho (64×96 con el
cuerpo ancho) o con menos piezas en la cabeza.
Con 96×96 caben **solo 6 cuadros por tira**, por eso `caminar` va a 6 y no a 8.

### 3.2 Nikoraemon (el villano) — celda 64×96, altura 88, **solo en escenas**

Humano barbudo de gi azul. Jefe de la federación «Autistas Marciales». Aparece
en la noche del robo (intro) y al final del dojo, cuando huye con el juguete. No
combate en la fase 1.

Base: `pixel art, full body side view facing right, bearded Japanese man in a blue karate gi, arrogant pose, 15 colors max, transparent background, no ground`

| Animación | Cuadros | Prompt de acción (inglés) | Estado |
|---|---|---|---|
| aparece | 4 | `The bearded man steps into view with a grand pose` | [ ] |
| coge el juguete | 4 | `The bearded man reaches down and grabs a small toy` | [ ] |
| huye | 6 | `The bearded man runs away holding the toy close` | [ ] |
| retrato de escena | 1 | ver §1.3 | [ ] |

---

## 4. Apoyos de la fase 1

**Decidido (2026-10-26): Lucinda y Rex caminan con el grupo** (guion §5), así
que la spec cambia en esto: ya no solo entran a atacar. Un apoyo sigue
atacando con la barra llena (en la fase 1, la barra se activa en el
monasterio), pero además acompaña al grupo en el carril. Por eso Lucinda y Rex
necesitan **guardia y caminar** para el carril, además de sus ataques.
Fenrir sigue siendo solo el gag del bosque.

| Apoyo | Celda | Altura | En la fase 1 | Animaciones | Estado |
|---|---|---|---|---|---|
| **Lucinda**, gata samurái (muy rápida) | 64×64 | 52 | sí: se une en el bosque, camina con el grupo y ataca desde el monasterio | **caminar 8, correr 8**, guardia 4, entrada 3, tajo ×3 (6), salida 3; escena: saludo 4 | [ ] |
| **Rex**, perro monje psíquico (levita, nunca golpea) | 96×64 | 56 | sí: se une en el monasterio, flota con el grupo y activa la barra | **caminar 8 (flotando)**, guardia 4, entrada 4, concentración 6 (bucle), soltar 4, salida 3; escena: saludo 4 | [ ] |
| **Fenrir**, husky bruto (gag de la fase 1) | 96×64 | 57 | **cruza el bosque y el monasterio** (durante la oleada), corriendo y destruyendo todo a su paso. No frena | carrera 8 (bucle), choque 4 (contra objetos y ninjas); salida 3 y escena (guardia 4 + saludo 4) son de la fase 2 | [ ] |

**Prompts de acción:**
- Lucinda: `The cat samurai walks forward with a confident swagger, sword on her shoulder` (caminar); `The cat samurai runs forward fast, sword held low` (correr); `The cat samurai slashes three times in a quick combo, then leaps away` (tajo ×3); `The cat samurai dashes in from the side with her sword` (entrada); `The cat samurai sheathes her sword and jumps off screen` (salida).
- Rex: `The white dog monk floats forward slowly, robe drifting, hovering above the ground` (caminar flotando); `The white dog monk floats up with eyes closed, concentrating` (concentración, bucle); `The white dog monk releases a wave of energy and lets things fall` (soltar); `The white dog monk drifts in calmly, hovering` (entrada).
- Fenrir: `The husky runs forward with a huge smile, red scarf flying` (carrera, bucle); `The husky crashes into an obstacle and bounces off` (choque).

**Icono del apoyo en el HUD:** 16×16, 3 iconos (uno por apoyo) más el marco de
selección (arte §3). Se pueden hacer a mano a partir del busto.

**Lucinda y Rex en la escena:** su retrato de escena (§1.3), y la escena en el
motor de escenas (`escena.c`). Sin sprite en el carril.

---

## 5. Objetos, efectos y armas (piezas pequeñas)

Todo lo de esta sección va en una **sola sesión de Pixellab** o se reduce
junto con un script, porque comparte **la paleta 7** (15 colores). Son translúcidos
en combate: diseñar los colores claros.

| Pieza | Tamaño | Cuadros | Paleta | Pixellab | Estado |
|---|---|---|---|---|---|
| **Señor Pimentón** (el juguete) | el del autor | escenas | 7 | **no**: lo aporta el autor (`diseno-beat-em-up.md` §12) | [ ] |
| barril (intacto + rotura) | 32×32 | 1 + 3 | 7 | sí, objeto | [ ] |
| caja de madera (intacta + rotura) | 32×32 | 1 + 3 | 7 | sí, objeto | [ ] |
| jarra de sake (intacta + rotura) | 32×32 | 1 + 3 | 7 | sí, objeto | [ ] |
| farol de papel (aldea) (intacto + rotura) | 32×64 | 1 + 3 | 7 | sí, objeto | [ ] |
| puerta shoji (dojo) (intacta + rotura) | 32×64 | 1 + 3 | 7 | sí, objeto | [ ] |
| shuriken (en vuelo / en el suelo) | 16×16 | 2 + 1 | 7 | sí | [ ] |
| kunai (en vuelo / en el suelo) | 16×16 | 1 + 1 | 7 | sí | [ ] |
| onigiri (cura poco) | 16×16 | 1 | 7 | sí | [ ] |
| pescado asado (cura mucho) | 16×16 | 1 | 7 | sí | [ ] |
| vida extra (cabeza de Tarou, brilla) | 16×16 | 2 | 7 | sí | [ ] |
| chispa de golpe | 16×16 | 3 | 5 (translúcida) | sí, clara | [ ] |
| golpe fuerte | 32×32 | 4 | 5 (translúcida) | sí, clara | [ ] |
| nube de relevo | 32×32 | 5 | 5 | sí | [ ] |
| polvo (derribo) | 16×16 | 4 | 4 | sí, en tonos tierra | [ ] |
| hojas / pétalos / luciérnagas | 16×16 | 2–3 | 7 (translúcidas) | sí (ver §6) | [ ] |
| sombra (óvalo) | 16×16 ×2 | 1 | 4 (negra) | **no**: a mano | [ ] |
| «GO →» | 32×16 | 2 (parpadeo) | 5 | **no**: a mano (texto) | [ ] |

**Prompts de objeto (base):** `pixel art game object, side view, [objeto], 15 colors max, transparent background, light colors if translucent, no shadow`
- barril: `wooden sake barrel, iron bands`; caja: `wooden crate with rope`; jarra: `ceramic sake jug with a lid`; farol: `paper lantern on a wooden stand, warm glow`; shoji: `sliding paper door with a wooden frame`.
- Rotura (los 3 cuadros): `the same object breaking apart into planks, then splinters, then dust`.
- Shuriken: `a four-pointed steel throwing star, spinning`; kunai: `a short steel throwing knife with a ring handle`.

---

## 6. Ambientación de la fase 1 (zonas)

Cada zona tiene **una capa cercana (BG1, la de la calle y los objetos que tapan)**
y **una capa lejana (BG2, el fondo con scroll por franjas)**. Por zona hay que
pedir: **tiles** para la cercana, **una imagen** para la lejana y **piezas** que
se repiten. Las piezas de la lejana se hacen **en bucle horizontal**: el mapa de
BG2 es de 32×32 celdas (256×256 px) y se repite; el borde izquierdo y el
derecho deben encajar.

Lo que **no** es Pixellab, en todas las zonas: el degradado del cielo (HDMA de
color por línea, `arte-beat-em-up.md` §5), las ondulaciones y los planos por
franjas (HDMA sobre el scroll), los rayos de luz (ventana diagonal), el halo de
los faroles (ventana circular, solo en la escena) y la luna como color (sale del
tileset, ver zona 3). Todo eso lo hace el código.

**Límite de tiles por capa:** `arte-beat-em-up.md` §5 dice **8 KB por capa (256
tiles de 8×8)**; `src/sistema/comun.h` reserva **512 tiles** por capa (0x2000
palabras). Mientras no se decida (§9), **trabajar con 256 tiles únicos por capa**.

### 6.1 Zona 1: aldea (atardecer)

| Pieza | Capa | Pixellab | Paleta | Estado |
|---|---|---|---|---|
| calle de tierra (el carril: suelo, borde, tierra) | BG1 | tileset (rejilla doble, como el suelo del nivel 1), 16×16 | 4 (cercana) | [ ] |
| fachadas *machiya* (módulos de 32 px de ancho) | BG1 | piezas modulares, en bucle horizontal | 4 | [ ] |
| puestos con *noren* (64×48) | BG1 | 1 objeto | 4 | [ ] |
| postes de farol y barriles apilados (primer plano) | BG1 alto (prioridad) | piezas en 16×16 | 4 | [ ] |
| *noren* ondeando (2 cuadros de 4 tiles) | BG1 | 2 tiles | 4 | [ ] |
| montañas, castillo lejano, sol bajo | BG2 | **1 imagen** ancha, en bucle | 3 (lejana) | [ ] |
| humo y pájaros | sprites 16×16 | 2 cuadros cada uno (prioridad baja) | 7 | [ ] |
| **aldeanos decorativos** (sin movimiento, sin interacción) | BG1, dentro del tileset | **propuesta:** una pose por aldeano, dibujada en el fondo y no como sprite, así no gasta ranuras ni VRAM de sprites. Altura de la figura: ~56 px (como Tarou) | 4 | [ ] |

**Prompt de la lejana:** `pixel art landscape background, feudal Japanese village at sunset, distant mountains, a castle silhouette, low sun, seamless horizontal tiling, 15 colors max, no sky gradient`.

**Prompt de la calle:** `pixel art tileset, dirt road with pebbles and a wooden edge, seen slightly from above, sunset light, 16x16 tiles, dual-grid, 15 colors max`.

### 6.2 Zona 2: bosque de bambú

| Pieza | Capa | Pixellab | Paleta | Estado |
|---|---|---|---|---|
| bambú del fondo (tallos en la distancia) | BG2 | **1 imagen** ancha y en bucle (la ondulación por HDMA se hace con código) | 3 | [ ] |
| sendero estrecho, rocas | BG1 | tileset, 16×16 | 4 | [ ] |
| linterna de piedra (*tōrō*) | BG1 | 1 objeto de 16×32 | 4 | [ ] |
| tallos de bambú en primer plano | BG1 alto (prioridad) | piezas en 16×16 | 4 | [ ] |
| hojas que caen | sprites 16×16 | 2–3 cuadros, pocas a la vez (cada una gasta 2 ranuras) | 7 | [ ] |

**Prompt de la lejana:** `pixel art background, dense bamboo forest in the distance, misty green, seamless horizontal tiling, 15 colors max`.

### 6.3 Zona 3: monasterio de Rex (noche)

| Pieza | Capa | Pixellab | Paleta | Estado |
|---|---|---|---|---|
| siluetas de cedros | BG2 | **1 imagen** ancha, en bucle | 3 | [ ] |
| explanada de piedra | BG1 | tileset, 16×16 | 4 | [ ] |
| torii rojo (en alto) | BG1 alto (prioridad) | 1 objeto | 4 | [ ] |
| faroles de piedra, *shimenawa*, árbol sagrado | BG1 | objetos | 4 | [ ] |
| luna | en el tileset del cielo, con colores claros | dentro de la imagen de BG2 | 3 | [ ] |
| luciérnagas | sprites 16×16 | 2 cuadros, translúcidas | 7 | [ ] |

**Prompt de la lejana:** `pixel art night landscape, cedar tree silhouettes against a deep blue sky, a full moon with light colors, seamless horizontal tiling, 15 colors max`.

La luz de los faroles (el halo) se hace con ventana circular por código: solo
en la escena del monasterio. En el combate, el halo queda pintado en el tileset.

### 6.4 Zona 4: dojo

| Pieza | Capa | Pixellab | Paleta | Estado |
|---|---|---|---|---|
| muro exterior, gran puerta, cerezos | BG1 / BG2 | piezas en 16×16 y 1 imagen de cerezos | 4 / 3 | [ ] |
| suelo de madera del interior | BG1 | tileset, 16×16 | 4 | [ ] |
| paredes *shoji*, pergaminos, armero de shinai | BG1 | piezas; la puerta shoji es el objeto de §5 | 4 | [ ] |
| vigas del techo | BG2 | **1 imagen**, cada franja con su propio scroll (perspectiva por HDMA) | 3 | [ ] |

**Prompt del suelo del dojo:** `pixel art tileset, polished wooden floorboards, warm brown, 16x16 tiles, seamless, 15 colors max`.

La cortinilla de la entrada del jefe (ventana que cierra) y el temblor de pantalla
los hace el código.

---

## 7. Lo que no es arte de Pixellab (para no pedirlo dos veces)

- **Sombras**, **chispas de golpe** (si no salen de Pixellab), **«GO →»**, el marco
  del HUD, los textos del HUD (BG3 tiene 4 colores).
- **Los degradados del cielo** y todo el HDMA del decorado (§6).
- **El mandala de Mode 7** de la activación de la barra en el monasterio: **no
  decidido** (`arte-beat-em-up.md` §5.3). No se pide hasta que se decida.
- **La intro** (3 escenas): hoy son placeholders. La escena 1 (noche del robo, §4 del
  guion) necesita a Tarou dormido y despertando (sale de `Sleep` y `Stand_up`, ♻),
  a Bárbara oyendo el ruido, al ninja que roba y al juguete. La escena 2 y la 3 los
  hace el motor de escenas con los sprites de arriba.
- **El villano en la intro**: §3.2.

---

## 8. Orden sugerido de encargos

1. **Tarou**: los cuadros de combate que faltan (§1.1). Son lo que más se usa.
2. **Bárbara** completa (§1.2), con la escala de §0.1 (88 px).
3. **Ninjas básico** (§2): se prueba primero en la prueba de carga.
4. **Objetos de la paleta 7** en una sola sesión (§5).
5. **Zona 1** (aldea): la calle y la lejana (§6.1).
6. **Lucinda, Rex y Fenrir**: sus ataques para el monasterio (§4).
7. **Zonas 2, 3 y 4**, por ese orden.
8. **Gomasuri-dono** y **Nikoraemon**, después de medir el presupuesto (§3.1).
9. **Retratos de escena** y **escena de la intro** (§1.3 y §7).

---

## 9. Decisiones y preguntas abiertas (para el autor)

Respuestas del autor (2026-10-26), ya aplicadas arriba:

1. **NPC de aldea:** sí, **aldeanos decorativos** (sin movimiento ni interacción).
   Propuesta en §6.1: dibujados en el tileset de BG1, sin sprite. Pendiente:
   cuántos y de qué tamaño.
2. **Lucinda y Rex:** **caminan con el grupo** (guion). Se añaden caminar y
   guardia a §4. Esto cambia la spec (`diseno-beat-em-up.md` §5.4): habrá que
   actualizarla.
3. **Ninjas (2026-10-26, segunda ronda):** un **personaje base de ninja** con **varios
   colores de traje** (hasta 3 por oleada, paletas 1 a 3), **lanzamiento de shuriken
   con el ataque 2**, y **bloqueo y agarre para todos** (§2.2, §2.4).
   **Kendokas** solo con el jefe, con su propio personaje y varios colores (§2.5).
   En la pelea del jefe no hay ninjas (§2.5).
4. **Gomasuri-dono:** **medir antes en la prueba de carga** (recomendado).
5. **Referencias:** las carpetas `res/descargas/graficos/pixellab/rex/`,
   `lucinda/` y `nikoraemon/` ya están creadas. Faltan las imágenes; el Señor
   Pimentón va en `res/descargas/graficos/` como indica `diseno-beat-em-up.md` §12.

**Fenrir (2026-10-26):** cruza el **bosque y el monasterio**, durante la oleada
   del monasterio. Solo **corre y destruye**: no frena, rompe los objetos
   rompibles (§5) y los ninjas que atraviesa **mueren** al primer choque. Arte:
   carrera 8 en bucle y choque 4. Para el ninja que muere se usa el KO del base
   (4 cuadros); para los objetos, sus cuadros de rotura. Si el choque contra
   ninjas necesita arte aparte (un frame de Fenrir distinto para cuando mata),
   se decide al medir.

Resueltas el mismo día (respuestas del autor):

- **Límite de tiles:** 256 por capa (8 KB), como dice `arte-beat-em-up.md` §5.
- **Retratos de escena:** 64×64.
- **Longitud de cada zona:** 1024 px (cuatro pantallas). La lejana se repite
  cuatro veces por zona en el bucle de BG2.
- **README de Bárbara:** actualizado (`res/descargas/graficos/pixellab/barbara/README.md`).
Pendiente que no bloquea: **el Señor Pimentón** lo aporta el autor cuando toque
(`diseno-beat-em-up.md` §12).

## 10. Errores encontrados al leer los documentos

- `arte-beat-em-up.md` §2 pone **47** cuadros para el ninja pesado; la suma da **48**.
- `arte-beat-em-up.md` §5 dice **8 KB por capa**; `comun.h` reserva **512 tiles**.
  Decidido: 256 por capa (§9).
- `barbara_samurai.png` no es la escala de Bárbara (figura de 36 px).
- El README de Bárbara estaba desactualizado; ya está actualizado (§9).
