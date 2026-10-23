; ---------------------------------------------------------------------------------
;   data.asm — Datos binarios (gráficos, paletas, mapas) que van dentro de la ROM.
;
;   PROPÓSITO
;     Este archivo es ensamblador, no C. Su único trabajo es "pegar" archivos
;     binarios en la ROM con .incbin y darles un nombre (etiqueta) para que el
;     código C los encuentre con `extern char nombre;`. Cada bloque va en una
;     sección "superfree": el enlazador la coloca en cualquier banco de 32 KB
;     donde quepa, así que ninguna sección puede superar 32 KB. Por eso los
;     tiles grandes van en una sección y su paleta + mapa en otra.
;
;   SE CONECTA CON
;     Incluye:  hdr.asm (generado por el Makefile: cabecera y mapa de bancos).
;     Lee:      los .pic/.pal/.map/.as que gfx4snes genera en res/ (el Makefile
;               los produce antes de ensamblar: data.obj depende de ASSETS).
;     Lo usan:  sistema/comun.c (fuentes), pantallas/personajes.c (sprites) y
;               cada pantalla_xxx.c (sus fondos), siempre vía `extern char`.
;
;   HARDWARE QUE TOCA
;     ROM: ocupa bancos LoROM de 32 KB. La CPU no lee estos datos directamente:
;     el código C los copia a VRAM/CGRAM por DMA en cada carga de pantalla.
;
;   VARIABLES (etiquetas que exporta, por sección)
;     .rodata_fuente     fuente_consola_til/_til_end/_pal, fuente_2bpp_til/_til_end/_pal
;     .rodata_barbara    barbara_samurai_til/_til_end/_pal
;     .rodata_nintendo   nintendo_til/_til_end/_pal/_map/_map_end
;     .rodata_publisher  publisher_*            .rodata_devs  devs_*
;     .rodata_escena1_*  escena1_*              .rodata_escena2_*  escena2_*
;     .rodata_escena3    escena3_*
;     .rodata_titulo_*   cerca_*, lejos_*, hdmaGradcieloList (tabla HDMA)
;     .rodata_tarou_*    tarou_<anim>_til/_til_end (13 animaciones) y tarou_pal
;     .rodata_nivel1_lejos  nivel1_lejos_til/_til_end/_pal/_map/_map_end (capa lejana del nivel 1)
;     .rodata_nivel1_cerca  nivel1_cerca_til/_til_end/_pal/_col/_col_end (capa cercana, por columnas)
;     .rodata_nivel1_banco  nivel1_cerca_col_banco (1 byte: el banco del mapa por columnas)
;     .rodata_nivel1_colision  nivel1_colision (rejilla de 128x14 bytes)
;     .rodata_prueba_*   ninja3_*_til, ninja2_*_til, ninja_pal, sombra_til, ninja_bancos
;                        (solo la ROM de prueba de carga, -DPRUEBA_CARGA)
;
;   Extensiones que genera gfx4snes: .pic tiles, .pal paleta, .map mapa;
;   en Mode 7: .pc7 tiles y .mp7 mapa. Todo se regenera con `make`.
;   → glosario: Banco / LoROM, DMA / HDMA
; ---------------------------------------------------------------------------------
.include "hdr.asm"

; --- Fuentes de la consola (Ark Pixel 10 px, mayúsculas) ---------------------------
.section ".rodata_fuente" superfree
fuente_consola_til:  .incbin "res/fuente/ark_consola.pic"        ; 96 tiles, 16 colores
fuente_consola_til_end:
fuente_consola_pal:  .incbin "res/fuente/ark_consola.pal"
fuente_2bpp_til:     .incbin "res/fuente/ark_consola_2bpp.pic"   ; 96 tiles, 4 colores (BG3)
fuente_2bpp_til_end:
fuente_2bpp_pal:     .incbin "res/fuente/ark_consola_2bpp.pal"
.ends

;  --- Sprites de los personajes de la secuencia de arranque --------------------------
; El lobo samurái (res/personajes/lobo_samurai.png) ya no va en la ROM: Tarou usa su
; propio arte (sección .rodata_tarou_reposo, más abajo). El PNG y su regla del
; Makefile se conservan para cuando sea un enemigo (Etapa 3).
.section ".rodata_barbara" superfree
barbara_samurai_til: .incbin "res/personajes/barbara_samurai.pic"
barbara_samurai_til_end:
barbara_samurai_pal: .incbin "res/personajes/barbara_samurai.pal"
.ends

; --- Pantalla 1: logo Nintendo (Mode 1, 16 colores, paleta 1) ----------------------
.section ".rodata_nintendo" superfree
nintendo_til:        .incbin "res/nintendo/logo_nintendo.pic"
nintendo_til_end:
nintendo_pal:        .incbin "res/nintendo/logo_nintendo.pal"
nintendo_map:        .incbin "res/nintendo/logo_nintendo.map"
nintendo_map_end:
.ends

; --- Pantalla 2: logo del publisher (Capcom del SDK) --------------------------------
.section ".rodata_publisher" superfree
publisher_til:       .incbin "res/publisher/logo_publisher.pic"
publisher_til_end:
publisher_pal:       .incbin "res/publisher/logo_publisher.pal"
publisher_map:       .incbin "res/publisher/logo_publisher.map"
publisher_map_end:
.ends

; --- Pantalla 3: logo del estudio (Konami del SDK, Mode 1, 16 colores) -------------
.section ".rodata_devs" superfree
devs_til:            .incbin "res/devs/logo_devs.pic"
devs_til_end:
devs_pal:            .incbin "res/devs/logo_devs.pal"
devs_map:            .incbin "res/devs/logo_devs.map"
devs_map_end:
.ends

; --- Pantalla 5: intro. Escenas 1 y 2 en 256 colores (Mode 3), escena 3 en 16 ------
.section ".rodata_escena1_til" superfree
escena1_til:         .incbin "res/intro/escena_1.pic"
escena1_til_end:
.ends
.section ".rodata_escena1_map" superfree
escena1_pal:         .incbin "res/intro/escena_1.pal"
escena1_map:         .incbin "res/intro/escena_1.map"
escena1_map_end:
.ends

.section ".rodata_escena2_til" superfree
escena2_til:         .incbin "res/intro/escena_2.pic"
escena2_til_end:
.ends
.section ".rodata_escena2_map" superfree
escena2_pal:         .incbin "res/intro/escena_2.pal"
escena2_map:         .incbin "res/intro/escena_2.map"
escena2_map_end:
.ends

.section ".rodata_escena3" superfree
escena3_til:         .incbin "res/intro/escena_3.pic"
escena3_til_end:
escena3_pal:         .incbin "res/intro/escena_3.pal"
escena3_map:         .incbin "res/intro/escena_3.map"
escena3_map_end:
.ends

; --- Pantalla 6: título. Capa cercana (64x32, scroll), capa lejana, cielo HDMA -------
.section ".rodata_titulo_cerca" superfree
cerca_til:           .incbin "res/titulo/fondo_cerca.pic"
cerca_til_end:
cerca_pal:           .incbin "res/titulo/fondo_cerca.pal"
cerca_map:           .incbin "res/titulo/fondo_cerca.map"
cerca_map_end:
.ends

.section ".rodata_titulo_lejos" superfree
lejos_til:           .incbin "res/titulo/fondo_lejos.pic"
lejos_til_end:
lejos_pal:           .incbin "res/titulo/fondo_lejos.pal"
lejos_map:           .incbin "res/titulo/fondo_lejos.map"
lejos_map_end:
.ends

.section ".rodata_titulo_cielo" superfree
.include "res/titulo/cielo_grad_data.as"                          ; tabla HDMA: un color por línea
.ends

; --- Nivel 1: capa lejana de 512 px (64x32), sin el logotipo del título ----------
; Capa cercana del nivel 1 (el tutorial, 2048 px): la genera scripts/preparar_nivel.py
; desde Tiled. Tiles (~8 KB) + paleta + el mapa COLUMNA A COLUMNA (fondo_cerca.col:
; 256 columnas x 32 filas x 2 bytes = 16 KB), que src/juego/fondo_nivel.c copia a la
; VRAM de una en una con DMA. Todo en una sección: ~24 KB, cabe en un banco.
.section ".rodata_nivel1_cerca" superfree
nivel1_cerca_til:     .incbin "res/nivel1/fondo_cerca.pic"
nivel1_cerca_til_end:
nivel1_cerca_pal:     .incbin "res/nivel1/fondo_cerca.pal"
nivel1_cerca_col:     .incbin "res/nivel1/fondo_cerca.col"
nivel1_cerca_col_end:
.ends

; El BANCO (los 8 bits altos de la dirección de 24 bits) donde quedó el mapa por
; columnas: el DMA lo necesita aparte ($4304) y desde C no se puede sacar de un
; puntero. «:etiqueta» es el banco de la etiqueta en WLA-DX.
.section ".rodata_nivel1_banco" superfree
nivel1_cerca_col_banco: .db :nivel1_cerca_col
.ends

; Rejilla de colisión del nivel 1: 128 x 14 celdas de 16 px, 1 byte cada una
; (0 vacío, 1 sólido, 2 plataforma, 3 zarza), fila a fila. La lee src/juego/colision.c.
.section ".rodata_nivel1_colision" superfree
nivel1_colision:      .incbin "res/nivel1/colision.bin"
.ends
.section ".rodata_nivel1_lejos" superfree
nivel1_lejos_til:     .incbin "res/nivel1/fondo_lejos.pic"
nivel1_lejos_til_end:
nivel1_lejos_pal:     .incbin "res/nivel1/fondo_lejos.pal"
nivel1_lejos_map:     .incbin "res/nivel1/fondo_lejos.map"
nivel1_lejos_map_end:
.ends

; --- Tarou (Revisión 1): 13 animaciones como piezas de 32x32 para el motor de
;     metasprites dinámicos (src/juego/tarou_sprite.c). Una sección por animación:
;     como máximo 10 cuadros de 96x64 (6 piezas = 3 KB) = 30 KB, que caben en un banco. La paleta es
;     la misma en todas (preparar_personaje.py); solo se incrusta la de reposo. ----
;     Numeración de bloques (verificada el 2026-10-16 en tarou_reposo_meta.inc):
;     gfx4snes -T numera los bloques de 32x32 por filas de la imagen de origen
;     (una tira de N cuadros de 96 px tiene 3N bloques por fila), así que el
;     cuadro n usa los bloques 3n..3n+2 (arriba) y 3N+3n..3N+3n+2 (abajo). Con
;     2 cuadros: cuadro 0 = 0, 1, 2, 6, 7, 8 (verificado en tarou_*_meta.inc). Mientras ningún .c las referencie, wlalink descarta
;     estas secciones (se ve como "discarded" en el .log).
.section ".rodata_tarou_reposo" superfree
tarou_reposo_til:     .incbin "res/personajes/tarou/tarou_reposo.pic"
tarou_reposo_til_end:
tarou_pal:            .incbin "res/personajes/tarou/tarou_reposo.pal"
.ends
; Tira ESTÁTICA de reposo: SIEMPRE 2 cuadros (6 KB) elegidos del reposo real por
; preparar_personaje.py. La cargan de una vez intro y título (src/pantallas/
; personajes.c), sin motor dinámico; el nivel usa .rodata_tarou_reposo entera.
.section ".rodata_tarou_reposo_estatico" superfree
tarou_reposo_estatico_til:     .incbin "res/personajes/tarou/tarou_reposo_estatico.pic"
tarou_reposo_estatico_til_end:
.ends
.section ".rodata_tarou_caminar" superfree
tarou_caminar_til:    .incbin "res/personajes/tarou/tarou_caminar.pic"
tarou_caminar_til_end:
.ends
.section ".rodata_tarou_correr" superfree
tarou_correr_til:     .incbin "res/personajes/tarou/tarou_correr.pic"
tarou_correr_til_end:
.ends
.section ".rodata_tarou_saltar" superfree
tarou_saltar_til:     .incbin "res/personajes/tarou/tarou_saltar.pic"
tarou_saltar_til_end:
.ends
.section ".rodata_tarou_caer" superfree
tarou_caer_til:       .incbin "res/personajes/tarou/tarou_caer.pic"
tarou_caer_til_end:
.ends
.section ".rodata_tarou_agachado" superfree
tarou_agachado_til:   .incbin "res/personajes/tarou/tarou_agachado.pic"
tarou_agachado_til_end:
.ends
.section ".rodata_tarou_agachado_caminar" superfree
tarou_agachado_caminar_til:.incbin "res/personajes/tarou/tarou_agachado_caminar.pic"
tarou_agachado_caminar_til_end:
.ends
.section ".rodata_tarou_desenvainar" superfree
tarou_desenvainar_til:.incbin "res/personajes/tarou/tarou_desenvainar.pic"
tarou_desenvainar_til_end:
.ends
.section ".rodata_tarou_envainar" superfree
tarou_envainar_til:   .incbin "res/personajes/tarou/tarou_envainar.pic"
tarou_envainar_til_end:
.ends
.section ".rodata_tarou_atacar" superfree
tarou_atacar_til:     .incbin "res/personajes/tarou/tarou_atacar.pic"
tarou_atacar_til_end:
.ends
.section ".rodata_tarou_echarse" superfree
tarou_echarse_til:    .incbin "res/personajes/tarou/tarou_echarse.pic"
tarou_echarse_til_end:
.ends
.section ".rodata_tarou_dormido" superfree
tarou_dormido_til:    .incbin "res/personajes/tarou/tarou_dormido.pic"
tarou_dormido_til_end:
.ends
.section ".rodata_tarou_despertar" superfree
tarou_despertar_til:  .incbin "res/personajes/tarou/tarou_despertar.pic"
tarou_despertar_til_end:
.ends
; La pose de guardia (PARADO con la espada fuera), 1 cuadro = 3 KB (unidad 18 del curso).
.section ".rodata_tarou_guardia" superfree
tarou_guardia_til:    .incbin "res/personajes/tarou/tarou_guardia.pic"
tarou_guardia_til_end:
.ends

; El BANCO de la ROM de cada tira, en el orden del enum Estado
; (src/juego/animaciones.h): el DMA que sube un cuadro a la VRAM lo necesita aparte
; (src/juego/tarou_sprite.c). «:etiqueta» es el banco de la etiqueta en WLA-DX.
.section ".rodata_tarou_bancos" superfree
tarou_til_bancos: .db :tarou_reposo_til, :tarou_caminar_til, :tarou_correr_til
                  .db :tarou_saltar_til, :tarou_caer_til, :tarou_agachado_til
                  .db :tarou_agachado_caminar_til, :tarou_desenvainar_til, :tarou_envainar_til
                  .db :tarou_atacar_til, :tarou_echarse_til, :tarou_dormido_til
                  .db :tarou_despertar_til
                  .db :tarou_guardia_til     ; índice 13 = DIBUJO_GUARDIA (no es un estado)
.ends

; --- ROM de prueba de carga (docs/diseno-beat-em-up.md §9) ---------------------------
; El ninja provisional en dos escalas (×3: 93 px de alto; ×2: 62 px), cortado en piezas
; de 16x16 empaquetadas en pares de filas de 1 KB, su paleta (3 variantes + la sombra)
; y la sombra (2 piezas). Los genera scripts/preparar_prueba_carga.py (.bin versionados:
; el pack de origen no se versiona). Solo los usa src/pantallas/prueba_carga.c, que se
; compila con -DPRUEBA_CARGA: en la ROM normal nadie los nombra y wlalink -d los descarta.
.section ".rodata_prueba_n3_andar" superfree
ninja3_andar_til:     .incbin "res/prueba_carga/ninja3_andar.bin"
.ends
.section ".rodata_prueba_n3_ataque" superfree
ninja3_ataque_til:    .incbin "res/prueba_carga/ninja3_ataque.bin"
.ends
.section ".rodata_prueba_n2_andar" superfree
ninja2_andar_til:     .incbin "res/prueba_carga/ninja2_andar.bin"
.ends
.section ".rodata_prueba_n2_ataque" superfree
ninja2_ataque_til:    .incbin "res/prueba_carga/ninja2_ataque.bin"
.ends
.section ".rodata_prueba_varios" superfree
ninja_pal:            .incbin "res/prueba_carga/ninja_pal.bin"
sombra_til:           .incbin "res/prueba_carga/sombra.bin"
; El banco de cada tira, en este orden: ×3 andar, ×3 ataque, ×2 andar, ×2 ataque.
ninja_bancos:         .db :ninja3_andar_til, :ninja3_ataque_til, :ninja2_andar_til, :ninja2_ataque_til
.ends
