# Documentación de _Tarou: A Dog's Journey_

Índice de todo lo que hay en `docs/`, con una frase por documento y el orden
en que conviene leerlos si se llega nuevo al proyecto. El vocabulario está en
el glosario: cuando un documento dice «→ glosario: X», la definición está ahí.

## Para empezar (en este orden)

1. **`glosario-snes.md`** — El glosario único del proyecto: vocabulario
   acordado (Etapa, Revisión, Nivel, Protagonista, Estado...), hardware de la
   SNES, gráficos, mecánicas y lo que aparece de C. Todo lo demás lo cita.
2. **`mapa-de-archivos.md`** — Una ficha por archivo: qué hace, con qué se
   conecta y qué hardware toca; más el diagrama de inclusión de `src/`.
3. **`tuberia-de-compilacion.md`** — De un `.c` y un `.png` a la ROM: cada
   herramienta del SDK, el `Makefile` regla por regla, cómo leer el `.log`,
   y la tubería paralela de las pruebas en PC.
4. **`mesen-y-depuracion.md`** — Cómo ejecutar y mirar por dentro la ROM con
   Mesen: ajustes que importan, mando, el HUD de depuración (medir velocidad
   y coste del frame), las herramientas del menú Debug y el script de
   capturas con guion.
5. **`hardware-snes-por-modulo.md`** — Qué chip y qué registros hay detrás de
   cada módulo, y los mapas de VRAM, CGRAM, OAM, ROM y WRAM.

## Diseño (qué hace el juego y por qué)

- **`diseno-beat-em-up.md`** — La especificación de la **Revisión 3** (vigente):
  el juego pasa a ser un beat 'em up de carril con Tarou y Bárbara, relevo,
  combate estilo *Final Fight*, 3 apoyos, la fase 1 (aldea, bosque, santuario,
  dojo) y el presupuesto de hardware que manda (ranuras por línea, VRAM de
  sprites, VBlank), con la prueba de carga que va primero.
- **`arte-beat-em-up.md`** — La lista de trabajo del arte de la Revisión 3:
  animaciones por personaje, enemigos, apoyos, objetos, escenografía por zona,
  HUD e intro, cada pieza con el truco de la SNES que usa y su estado.
- *Históricos (plataformas, tag `plataformas-r2`):* los dos siguientes.
- **`diseno-demo-jugable.md`** — La especificación de la **Revisión 1**:
  Tarou adulto en el nivel 1 con ocho mecánicas, mando, estados, física en
  punto fijo, cámara, metasprites dinámicos, fondos, HUD, pipeline de assets,
  arquitectura, cabecera estándar y verificación. Documento vivo.
- **`diseno-tutorial-movimiento.md`** — La **Revisión 2**: el nivel 1 como
  tutorial de movimiento de 8 pantallas (escalones, muro, túnel, foso, zarzas,
  tronco, salida), dibujado en Tiled y vestido por script, con colisión,
  reaparición y un guion de mando que lo juega solo. Resultados y trampas.
- **`diseno-secuencia-arranque.md`** — La especificación de las seis
  pantallas de arranque (logos, dedicatoria, intro, título) con los hallazgos
  técnicos verificados en el SDK (Etapa 1.5).
- **`diseno-motor-escenas.md`** — El motor de escenas (`src/escena/`): una
  tabla de eventos mueve brillo, mosaico y un destello por ventana + HDMA;
  lógica pura probada en el PC. Estrenado en el logo Nintendo (2026-10-21).

## Planes

- **`plan-desarrollo.md`** — La hoja de ruta completa por Etapas (0 a 6):
  objetivos, entregables, criterios de salida e ítems técnicos a estudiar.
- **`plan-implementacion-revision-1.md`** — El plan de tareas con el que se
  construyó la Revisión 1 (8 fases, pruebas por tarea); sirve de plantilla
  para las revisiones siguientes.
- **`../aprendizaje/README.md`** — Entrada de la carpeta de aprendizaje: qué
  hay, cómo se usa y el texto para arrancar la sesión que escribe cada unidad.
- **`../aprendizaje/PLAN.md`** — El plan de la carpeta de aprendizaje: las
  unidades, de lo simple a lo complejo, para entender todo este código.

## Material de apoyo

- **`screenshots/`** — Capturas de referencia: la secuencia de arranque
  (2026-09-24) y `revision-1/` (título y todas las mecánicas con el HUD, con el
  arte real de Pixellab, 2026-10-19).
- **`../examples/likemario/LikeMario.md`** — Estudio del ejemplo del SDK que
  sirve de referencia para el motor de mapas, objetos y sprites dinámicos.
- **`../res/descargas/README.md`** y
  **`../res/descargas/graficos/pixellab/*/README.md`** — Qué material de
  terceros hace falta, dónde dejarlo y cómo exportar desde Pixellab (el
  personaje en `pixellab/tarou/`, el tileset del suelo en `pixellab/suelo/`).

## Reglas de la casa que todos los documentos asumen

- Cada línea de código lleva su comentario en español; el texto en pantalla
  del juego va en inglés (salvo el HUD de depuración, que muestra los nombres
  del código).
- Todo archivo fuente abre con la cabecera estándar (propósito, conexiones,
  hardware, variables, glosario): `diseno-demo-jugable.md` §12.
- Tras tocar cualquier `.h`: `make clean` antes de `make`.
- La lógica pura (`src/juego/tarou.c`, `camara.c`, `nivel.c`,
  `animaciones.c`) no incluye `<snes.h>` y se prueba en PC
  (`make -C tests; .\tests\pruebas.exe`) antes de cada commit.
