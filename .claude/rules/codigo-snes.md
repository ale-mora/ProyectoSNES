---
paths:
  - "src/**"
  - "data.asm"
  - "Makefile"
  - "hdr.asm"
---

# Código de la SNES: trampas de tcc816, VBlank y enlazado

> Movido verbatim desde `CLAUDE.md` el 2026-11-05 (ordenar-memoria-repo). Rutas relativas a la raíz del repo. El resto de lo que estaba en `CLAUDE.md` sigue allí; lo movido quedó así: la crónica del juego y de las ROMs en `docs/estado/fase1-y-roms.md`, las trampas del código en `.claude/rules/codigo-snes.md`, los assets en `.claude/rules/assets.md` y el detalle de `docs/` y `aprendizaje/` en `.claude/rules/docs-y-curso.md`.

- Every line of game code carries a Spanish comment (project rule: the owner is learning C and SNES from this code). Keep the pure logic free of `int`, `long`, `float`, `bool`, divisions in per-frame code (tcc816 calls a slow routine), indexing arrays of structs whose size is not a power of two in per-frame loops (`a[i]` calls `tcc__mul` for 5- or 6-byte structs, while 3 bytes compiles to `asl`+`adc`, so check the `.asm`; pad the struct to 8 bytes or walk it with a pointer; this bit twice), shifts by a variable amount inside hot loops (tcc816 loops per bit; accumulate instead), `?:` with strings as variadic arguments (tcc816 ICE), a `u8` passed to `%d` in a variadic call (tcc816 pushes 1 byte, so the high byte is garbage: cast to `(u16)`), and byte-filling loops in C (`*p++ = 0` costs ~40 instructions per byte, ~60 inside the HUD; use the library `memset`, which is asm and takes 3). `static` variables are NOT zero at boot: crt0 clears only 10 bytes of `.bss`, so initialize them. Global `const` tables live in WRAM (`$7F`, copied at boot); only `static const` stays in ROM. `setBrightness(0)` writes `$80`, which is forced blank. A handwritten `.asm` with a `RAMSECTION` needs `.BASE $00` before it and `.BASE $80` after (FastROM; WLA adds the base to RAM banks too). `PF_ENTERO` is only valid for values >= 0: compare signed positions in fixed point. **VBlank budget:** 37 lines, the SDK takes 5; the debug HUD's `V a b` shows what the level uses (today, FastROM: `V5 7`, `V5 17..20` on sprite-upload frames). Anything written to VRAM after line 37 is silently dropped — that was the "Tarou in pieces" bug of 2026-10-20.

Note: wlalink runs with `-d` (discard unreferenced sections), so the ROM grows when game code starts referencing assets that only a test ROM used before (the 14 `.p16` strips of Tarou: 88 % → 83 % free). Since 2026-11-04 Tarou also has 6 beat 'em up strips (`guardia_bu`, `golpe_1..3`, `recibir`, `ko`; `.p16` only, `TAROU_BU_ANIMS` in the Makefile) that `juego/personaje.c` uses in the phase; he still walks, jumps and falls with the platformer strips.

## Al día (2026-11-05; añadido después de mover lo de arriba)

- Las copias a la CGRAM del VBlank (paletas) van ANTES de `dma_lista_ejecutar`: una copia a la CGRAM que se sale del VBlank choca con el HDMA del cielo (canal 6) y corrompe colores (el «triángulo amarillo»).
- Subidas de tiles de sprites por VBlank en la fase 1 (`PRESUPUESTO_SUBIDA` 2048 B en `subida.h`, desde el 2026-11-05; medido: ~7 líneas por KB sobre una base de ~18 con el HUD de depuración): `pantalla_fase1.c` lo reparte a turnos (un frame la protagonista primero, el siguiente los ninjas) y `ninja_sprite_reservar` elige UN ninja. Las ranuras 1 y 2 (24 piezas) usan los dos huecos a turnos como DOBLE BÚFER cuando la otra está vacía (también el jefe, salvo sus cuadros de >24 piezas); si un cuadro tiene que pisar el hueco que se enseña, va ENTERO en un VBlank o espera (solo los de más de 16 piezas con las dos ranuras ocupadas van en trozos, y mientras tanto el ninja no se dibuja: un parpadeo de 1-2 frames en vez de un cuadro a medias). Antes, los trozos de 1024 B sobre el cuadro visible hacían titilar a los enemigos. El noren espera si la lista pasa de 2560. La fila 5 del HUD de depuración mide el VBlank («V a MAX b»; hoy hasta 34 de 37).
- HDMA de la fase 1: canal 6 el cielo, canal 5 las ventanas de color de los objetos (`objetos_sprite.c`, modo 4, tabla doble cambiada en `objetos_sprite_vblank`); `REG_HDMAEN` se escribe con los dos; al salir, `setModeHdmaReset(0)` y `REG_WOBJSEL = 0`. La DMA general usa el canal 0.
- Paletas de sprites: 0..3 opacas (protagonista y las 3 ranuras de ninja, llenas), 4..7 entran en la media suma de la escena (sombra 4, humo/pájaros/objetos 7). `CGWSEL = OBJETOS_CGWSEL` (0x22) en la fase; la pausa pone 0x00.
- Las ROMs de prueba se definen por `CFLAGS` (`GUION_FASE1`, `GUION_BARBARA`, `ESCENARIO_INICIAL`, `GUION_APOYO`, `GUION_PRUEBA`, `PRUEBA_CARGA`): `make clean` antes y después.

