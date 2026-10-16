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
.section ".rodata_nivel1_lejos" superfree
nivel1_lejos_til:     .incbin "res/nivel1/fondo_lejos.pic"
nivel1_lejos_til_end:
nivel1_lejos_pal:     .incbin "res/nivel1/fondo_lejos.pal"
nivel1_lejos_map:     .incbin "res/nivel1/fondo_lejos.map"
nivel1_lejos_map_end:
.ends

; --- Tarou (Revisión 1): 13 animaciones como piezas de 32x32 para el motor de
;     metasprites dinámicos (src/juego/tarou_sprite.c). Una sección por animación:
;     como máximo 15 cuadros de 2 KB = 30 KB, que caben en un banco. La paleta es
;     la misma en todas (preparar_personaje.py); solo se incrusta la de reposo. ----
;     Numeración de bloques (verificada el 2026-10-16 en tarou_reposo_meta.inc):
;     gfx4snes -T numera los bloques de 32x32 por filas de la imagen de origen
;     (una tira de N cuadros tiene 2N bloques por fila), así que el cuadro n usa
;     los bloques 2n, 2n+1 (arriba) y 2N+2n, 2N+2n+1 (abajo). Con 4 cuadros:
;     cuadro 0 = 0, 1, 8, 9. Mientras ningún .c las referencie, wlalink descarta
;     estas secciones (se ve como "discarded" en el .log).
.section ".rodata_tarou_reposo" superfree
tarou_reposo_til:     .incbin "res/personajes/tarou/tarou_reposo.pic"
tarou_reposo_til_end:
tarou_pal:            .incbin "res/personajes/tarou/tarou_reposo.pal"
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
