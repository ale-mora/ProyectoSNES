# Mesen y depuración: ejecutar la ROM y mirarla por dentro

**Para quién:** quien tenga que probar un cambio, cazar un fallo gráfico o de
física, o verificar la ROM sin tocar el teclado.
**Qué es:** cómo se relaciona el proyecto con el emulador (MesenCE 2.2.1, en
`tools/mesen/Mesen.exe`), qué herramientas de Mesen sirven para qué, y las
dos herramientas propias de depuración: el HUD en pantalla y el script de
capturas con guion. Términos: `docs/glosario-snes.md`.

## 1. Cargar y ejecutar la ROM

| Forma | Cómo |
|---|---|
| Tras compilar todo | `.\run.bat` (hace `make clean` + `make` y abre Mesen) |
| Solo abrir la ROM ya compilada | `& "E:\ProyectoSNES\tools\mesen\Mesen.exe" "E:\ProyectoSNES\ProyectoSNES.sfc"` (mirar antes la fecha del `.sfc`: si el código cambió después, hay que recompilar) |
| Desde Mesen | File → Open, o arrastrar el `.sfc` a la ventana |
| Para verificar sin manos | `.\scripts\capturar_mesen.ps1 ...` (ver §5) |

Mesen guarda su configuración en `Documentos\MesenCE\settings.json`
(OneDrive en esta máquina). Tres ajustes que importan al proyecto:

- `Emulation.EmulationSpeed`: **estaba en 75** y todo parecía un 25 % más
  lento de lo que la ROM hace (45 FPS en vez de 60). El script de capturas
  fuerza `--Emulation.EmulationSpeed=100` por línea de comandos; para jugar a
  mano, ponerlo en 100 en Settings → Emulation.
- `Snes.Region = Ntsc`: 60 frames por segundo, como supone todo el código
  (`SEGUNDOS(n)` = n × 60). En PAL serían 50 y los tiempos se alargarían.
- `Debug.ScriptWindow.AllowIoOsAccess = false`: los scripts Lua no pueden
  escribir archivos (por eso `capturas_mesen.lua` no guarda PNG). Se puede
  activar en Debug → Script Window → Settings, pero el proyecto no lo necesita.

Opciones de línea de comandos útiles (Mesen 2): `<rom.sfc>` carga la ROM;
`--<Sección>.<Propiedad>=<valor>` cambia un ajuste solo para esa ejecución;
`-donotsavesettings` no toca `settings.json` al salir. **MesenCE 2.2.1 no
carga la ROM si además se le pasa un `.lua`** (verificado el 2026-10-16 en
ambos órdenes): los scripts Lua se abren a mano.

## 2. El mando en el teclado

Mesen lee el mando de la SNES del teclado según Settings → Input → Player 1.
La asignación guardada en esta máquina (`Mapping2`; `Mapping1` es un gamepad
XInput) y lo que hace cada botón en el nivel (`juego/mando.c`):

| Botón SNES | Tecla | En el nivel 1 |
|---|---|---|
| Cruceta | flechas | caminar; Abajo = agacharse; control aéreo |
| B | A | saltar (mantener alarga el salto) |
| Y | Z | correr (mantenido) |
| A | S | atacar (solo con la espada fuera y en suelo) |
| X | X | libre |
| L | Q | libre |
| R | W | envainar / desenvainar |
| Start | D | en el título entra al nivel; en el nivel vuelve al título |
| Select | E | HUD de depuración sí/no |

En la secuencia de arranque no se puede saltar nada (`PERMITE_SALTO = 0`);
el título aparece hacia los 50 s y, si nadie toca nada, vuelve a la intro a
los 15 s (attract).

## 3. El HUD de depuración (SELECT en el nivel)

Siete filas de texto sobre el juego (`juego/hud_debug.c`), que leen la struct
`Tarou` y los contadores del nivel. Es la herramienta principal para afinar
constantes y para **ver** el punto fijo:

```
X 123  VX -16          x en píxeles enteros; vx en punto fijo 12.4 (16 = 1 px/frame)
Y 112  VY 0            y de la celda (pies = Y + 64 = suelo base 176); vy (negativo = sube)
EST CAMINANDO          estado actual, con el nombre del enum
ESP FUERA  DIR DER     espada fuera/envainada; dirección derecha/izquierda
CAM 0  F 1234          cámara (0..1792 en el tutorial) y frames desde que empezó el nivel
GOLPE 167 168 28 16    caja de golpe de la espada (x, y, ancho, alto) o "-"
LIN 252 10 59  VB 3738 línea del haz tras la lógica, tras el sprite y al final
                       del frame; y VBlanks totales desde el arranque
```

Cómo usar las dos últimas filas para medir:

- **Velocidad real**: anota `F` y `VB` en dos capturas separadas N segundos.
  A 60 FPS, `VB` sube 60·N. Si `F` sube igual que `VB`, el juego no pierde
  frames; si sube menos, la lógica se pasa de tiempo. (Así se descubrió que
  era Mesen, y no la ROM, quien iba al 75 %.)
- **Coste del frame**: `LIN` es la línea del haz (0..261 en NTSC) cuando
  termina cada tramo. El VBlank empieza en la 225; si el último número es
  menor que 225, el frame ha usado `(37 + LIN) / 262` del tiempo. Con todo
  encendido, la lógica acaba hacia la línea 60: un 37 %. Un valor ≥ 225 (o que
  da la vuelta) significa frame pasado de tiempo. En el tutorial la lógica
  (con colisión) acaba hacia la línea 70-120 y el final del frame, con el
  HUD encendido, hacia 150-200: **el propio HUD cuesta 50-80 líneas**, y con
  él encendido el nivel baja a unos 50 FPS (sin él, 60).
- Las filas se escriben por turnos (una por frame): un dato puede tener hasta
  6 frames de retraso respecto a otro. Para capturar un instante exacto, usa
  el depurador (§4).

## 4. Las herramientas de Mesen (menú Debug)

Rutas de menú según Mesen 2; en MesenCE los nombres coinciden. Lo que cada
una enseña de este proyecto:

| Herramienta | Para qué sirve aquí |
|---|---|
| **Debugger** | Ver el código en ensamblador con las etiquetas de nuestro código C, poner puntos de ruptura (breakpoints) en una función (`tarou_actualizar`, `pantalla_nivel1_vblank`), avanzar paso a paso y mirar los registros del 65816 (A, X, Y, P, DB, S). Las etiquetas vienen de `ProyectoSNES.sym`, que `snes_rules` filtra "para el depurador de Mesen"; si no aparecen, File → Import Labels dentro del depurador y elegir el `.sym`. |
| **Memory Tools** | Ver y editar cualquier memoria: WRAM (las variables; `ANIMACIONES` está en `$7F00E4`, porque tcc816 copia las tablas `const` a RAM), VRAM (tiles y mapas: `VRAM_FUENTE` en `0x6000` palabras, el mapa de texto en `0x6800`...), CGRAM (paletas), OAM (los 128 sprites), ROM. Útil para comprobar que un DMA dejó los datos donde `comun.h` dice. |
| **Tilemap Viewer** | Dibuja cada capa de fondo tal como la ve la PPU, con el scroll aplicado: para ver el mapa de 64×32 de la capa cercana del nivel, dónde está la costura del espejo, o si el HUD escribe en la fila correcta del mapa de texto. |
| **Tile Viewer** | Los tiles crudos de la VRAM, por formato (2bpp, 4bpp, 8bpp). Para comprobar la numeración de bloques de gfx4snes (`(n/4)·64 + (n%4)·4`) o que el motor dinámico copió el cuadro correcto en `0x0000`. |
| **Sprite Viewer** | Las 128 entradas de la OAM con posición, tile, paleta, tamaño y prioridad, y una vista de dónde cae cada una. Para ver las 6 piezas del metasprite de Tarou, un sprite "fantasma" fuera de pantalla, o el bit de espejo. |
| **Palette Viewer** | Las 256 entradas de la CGRAM: fondos 0..127 y sprites 128..255. Para confirmar que `tarou_pal` está en la paleta de sprites 0 y Bárbara en la 1 (el fallo del "lobo azul" del título se vio aquí: el título cargaba otra paleta en la 0). |
| **Event Viewer** | Qué registro de la PPU se escribió en qué línea y ciclo del frame: muestra los DMA del VBlank, las escrituras de HDMA del cielo y el instante de `bgSetScroll`. |
| **Profiler** | Cuántos ciclos consume cada función (con las etiquetas del `.sym`). Complementa la fila `LIN` del HUD. |
| **Script Window** | Ejecuta scripts Lua (`scripts/mando_nivel.lua` para simular el mando con un guion por frames; `capturas_mesen.lua` para capturas, que necesita activar la E/S). Se abren con File → Open dentro de la ventana. |
| **Trace Logger** | Graba cada instrucción ejecutada: para seguir una interrupción NMI o una rutina del SDK. |

Dos consejos de uso:

- Para pausar en el frame exacto en que Tarou aterriza, pon un breakpoint
  de escritura en la dirección de `tarou.en_suelo` (Memory Tools muestra la
  dirección de `tarou` por su etiqueta en el `.sym`).
- Para ver un glitch de un solo frame, usa Game → Pause y avanza frame a
  frame (Game → Step Frame o la tecla asignada).

## 5. Verificar sin manos: `scripts/capturar_mesen.ps1`

El agente (y el autor, si quiere) verifica la ROM con capturas en vez de
mirando la pantalla. Uso desde PowerShell, en la raíz:

```
.\scripts\capturar_mesen.ps1 -Duracion 92 -Cada 0.5 -Espera 4 `
    -Guion "55,55.3,Start;57,57.3,Select;58,59.5,Down;59.5,61,Down;59.5,61,Right;61.6,61.9,B;63.5,63.7,R;65,65.2,A"
```

- `-Guion`: tramos `desde,hasta,tecla` en segundos (desde el fin de la espera
  inicial); varias teclas a la vez son varios tramos solapados. Nombres de
  botón de la SNES: `Up Down Left Right A B X Y L R Start Select`.
- `-Cada`: segundos entre capturas (admite decimales); `-Duracion`: total;
  `-Desde`: segundo de la primera captura (el guion corre desde 0), para
  mirar de cerca un instante con `-Cada 0.1` sin llenar la carpeta.
- Salida: `scripts/capturas/frame_NNN_tSSs.png` (ignorados por git).
- `-EnPantalla`: deja la ventana visible (por defecto se aparta fuera de la
  pantalla para no estorbar).

Qué hace por dentro, y por qué (cada punto costó una tanda fallida):

1. Lanza `Mesen.exe <rom> --Emulation.EmulationSpeed=100
   --Preferences.AllowBackgroundInput=true -donotsavesettings`. La segunda
   opción pide a Mesen que acepte el mando aunque no sea la ventana activa.
   Se añadió el 2026-10-19 porque las teclas del guion se perdían mientras
   el autor usaba el equipo (el foco de Mesen saltaba a la barra de menús),
   pero **su efecto no está demostrado**: hubo una tanda buena sin ella y una
   mala con ella. Lo fiable es **no usar el equipo durante la tanda**. Sin
   comprobar: si con ella Mesen lee también el teclado físico, lo que teclees
   en ese momento llegaría al mando emulado. Ninguna de las dos opciones se
   guarda en tu configuración.
2. Restaura la ventana si estaba minimizada y la **activa una vez** con el
   truco de `AttachThreadInput` (Windows no cede el primer plano a quien no
   tiene la entrada). Sin esta activación, el foco de teclado dentro de Mesen
   está en la barra de menús y las teclas navegan el menú.
3. Con Mesen ya en primer plano le da **un único toque real de Shift**
   (`keybd_event`): es lo único que hace que `PrintWindow` capture el lienzo
   del juego; sin él devuelve la pantalla de juegos recientes de Mesen. Se
   probó sin éxito con Shift por mensaje, con `SWP_FRAMECHANGED` +
   `RedrawWindow` + `WM_NCACTIVATE`, y con minimizar y restaurar (lienzo
   negro). Shift solo no escribe nada y, con Mesen en primer plano, no puede
   llegar a otra aplicación; si la activación falló, no se envía y el script
   avisa.
4. Mientras está activa, envía un **clic por mensaje** (`WM_LBUTTONDOWN/UP`)
   sobre el visor para fijar el foco ahí, devuelve el primer plano a la
   ventana del autor y aparta Mesen fuera de la pantalla (`SetWindowPos` a
   x = −2600): DWM la sigue dibujando y `PrintWindow` la captura igual.
5. Las teclas del guion se envían con `PostMessage(WM_KEYDOWN/WM_KEYUP)`
   directamente a la ventana: no dependen del foco y nunca acaban en otra
   aplicación (un `keybd_event` iría a la ventana activa, que ya es la tuya).
6. Captura con `PrintWindow(PW_RENDERFULLCONTENT)` tras declarar el proceso
   DPI-aware (si no, con la pantalla al 125 % la captura salía recortada).

Si una tanda sale mal: comprobar en la salida `Mesen activada: True`; mirar
si las capturas son diminutas (ventana minimizada: el script la restaura) o
muestran el menú resaltado (el foco no llegó al visor: repetir).

### La ROM de prueba: verificar sin teclas

El guion de teclas de arriba falla en cuanto Mesen pierde el foco. Para el
nivel hay algo más fiable: una ROM compilada con `-DGUION_PRUEBA`
(`docs/tuberia-de-compilacion.md` §5) arranca directa en el nivel y su mando
sale de `src/juego/guion_tutorial.c`, el mismo guion que juega
`tests/prueba_recorrido.c` en el PC. No hace falta pulsar nada:

```
.\scripts\capturar_mesen.ps1 -Rom RUTA\tutorial_guion.sfc -Duracion 46 -Cada 2 -Espera 3 -Guion "1,1.3,Select"
```

(El `Select` enciende el HUD, si llega; del mando real esa ROM solo escucha
SELECT y START.) Si la ROM y el PC no hacen lo mismo con el mismo guion, el
compilador de la SNES hace algo distinto: compara las x del HUD con la traza
del PC. El 2026-10-20 coincidieron y la ROM llegó a la salida.

**Medir los FPS sin HUD:** el recorrido son 2238 cuadros; a 60 FPS dura
37,3 s. Capturar con `-Cada 0.5` cerca del final y ver cuándo se oscurece.

## 6. Fallos conocidos y cómo se vieron

| Síntoma | Causa | Dónde está la solución |
|---|---|---|
| Con el HUD encendido F avanza ~84 % de VB en el nivel tutorial | el HUD cuesta 50–80 líneas y la colisión se comió el margen; sin HUD va a 60 FPS | pendiente (abaratar el HUD) |
| La ROM de prueba iba mucho más lenta que la normal | `guion[i]` con structs de 5 bytes: `tcc__mul` en cada acceso | `guion.c` recorre la tabla con un puntero |
| Tarou reaparecía en lo alto de un salto | `PF_ENTERO` de una y negativa da un número enorme | comparar en punto fijo (`partida.c`) |
| El juego va "lento" (salto de 0,8 s tarda 1,5 s) | `EmulationSpeed = 75` en los ajustes de Mesen | §1; el HUD (`F`/`VB`) lo delata |
| El lobo del título salía azul al volver del nivel | el título cargaba su paleta en la 0 y el nivel otra; hoy intro, título y nivel comparten `tarou_pal` | `personajes.c` |
| Capturas con la pantalla de inicio de Mesen aunque la ROM corre | capa antigua en `PrintWindow` | §5, punto 4 |
| Las teclas del guion abren el menú | foco en la barra de menús | §5, puntos 2 y 3 (y nunca usar Alt: abre el menú) |
| `wla-65816`: `Label "SECTIONSTART_.bss" was defined more than once` | ruido de la información de depuración (`-d`) | no es un error |
| `816-tcc: error: ICE 2` | construcción de C que tcc816 no soporta (`?:` de cadenas en argumentos variádicos) | reescribir con `if/else` |
| Dos de los cuatro cuadros de Bárbara salían con trozos de Tarou | un sprite de 64×64 empezaba en el tile 192 y cruzaba a la segunda tabla de 256 tiles: la PPU da la vuelta dentro de la tabla | Bárbara en el tile 256 (`personajes.c`, bloque «LÍMITE») |
| El guion pierde teclas a mitad de tanda (el menú File aparece resaltado) | el autor usaba el equipo y el foco de Mesen saltó a su barra de menús | no usar el equipo durante la tanda; `--Preferences.AllowBackgroundInput=true` se fuerza pero su efecto no está demostrado (§5, punto 1) |
| Tarou parece flotar sobre una raya de hierba | el arte es una vista lateral un poco elevada (patas lejanas 3 px más arriba) y la hierba empezaba justo bajo los pies | entonces `scripts/preparar_suelo.py --hundir 4`; desde el tutorial, la banda empieza 2 px sobre el borde de la celda y el sprite se dibuja 2 px más abajo (`HUNDIR_PIES`) |
| El texto del HUD queda detrás del techo del túnel | los tiles de BG3 sin bit de prioridad van detrás de BG0 aunque el modo sea `BG3_MODE1_PRIORITY_HIGH` | `TILE(c)` con `0x2000` en `hud_debug.c` |
| Un `.asm` generado para estudiar rompe la compilación | `snes_rules` compila como fuente todo `src/*/*.asm`: el `.asm` y su `.c` dan símbolos duplicados | borrar el `.asm` de `src/` tras copiarlo (`aprendizaje/PLAN.md` §6.4) |
