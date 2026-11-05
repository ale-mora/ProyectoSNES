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
;     .rodata_aldea_*    aldea_cerca_til/_til_end/_pal, aldea_cerca_col, aldea_noren,
;                        aldea_banco, aldea_lejos_til/_til_end/_pal/_map/_map_end y
;                        hdmaGradatardecerList (la aldea de la fase 1); ambiente_til/_pal/
;                        _pieza/_banco (el humo y los pájaros)
;     .rodata_p16_*      tarou_<anim>_p16 (14 tiras en piezas de 16x16) y tarou_p16_bancos
;     .rodata_bp16_*     barbara_<anim>_p16 (20 tiras, celda 112x96), barbara_pal y barbara_p16_bancos
;     .rodata_np16_*     ninja1_<anim>_p16 (7 tiras de «Ninja 1», celda 96x96), ninja1_pal y ninja1_p16_bancos
;     .rodata_n2p16_*, .rodata_n3p16_*  ninja2_/ninja3_<anim>_p16 (mediano y grande, 128x112), sus _pal y _p16_bancos
;     .rodata_prueba_*   ninja64_andar, ninja64_ataque, ninja_pal, sombra_til, sombra_piezas,
;                        sombra_banco, ninja_bancos
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
; --- La aldea (zona 1 de la fase 1, 2026-11-05): scripts/preparar_aldea.py --------
; Capa cercana: 512 tiles (16 KB) y DOS paletas (la 4 y la 5); el mapa COLUMNA A
; COLUMNA (256 x 32 x 2 = 16 KB) con el bit de prioridad en el primer plano, y detrás
; los dos cuadros del noren (8 tiles cada uno), en la misma sección: un solo banco
; (aldea_banco) vale para los dos. Lejana: gfx4snes desde lejos.png (512 px, -e 2).
.section ".rodata_aldea_cerca" superfree
aldea_cerca_til:      .incbin "res/aldea/cerca_til.bin"
aldea_cerca_til_end:
aldea_cerca_pal:      .incbin "res/aldea/cerca_pal.bin"
.ends

.section ".rodata_aldea_col" superfree
aldea_cerca_col:      .incbin "res/aldea/cerca_col.bin"
aldea_noren:          .incbin "res/aldea/noren_til.bin"
.ends

.section ".rodata_aldea_banco" superfree
aldea_banco:          .db :aldea_cerca_col
.ends

.section ".rodata_aldea_lejos" superfree
aldea_lejos_til:      .incbin "res/aldea/lejos.pic"
aldea_lejos_til_end:
aldea_lejos_pal:      .incbin "res/aldea/lejos.pal"
aldea_lejos_map:      .incbin "res/aldea/lejos.map"
aldea_lejos_map_end:
.ends

; El humo y los pájaros (sprites, juego/ambiente_sprite.c): 4 piezas de 16x16 (fila de
; arriba de las 4 y luego la de abajo, 512 bytes), la paleta de sprites 7 y la tabla de
; piezas de una sola pieza (dx, dy = 0, 0), con su banco.
.section ".rodata_aldea_ambiente" superfree
ambiente_til:         .incbin "res/aldea/ambiente_til.bin"
ambiente_pal:         .incbin "res/aldea/ambiente_pal.bin"
ambiente_pieza:       .db 0, 0
ambiente_banco:       .db :ambiente_pieza
.ends

.section ".rodata_aldea_cielo" superfree
.include "res/aldea/atardecer_grad_data.as"                        ; el cielo del atardecer
.ends

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

; --- Tarou en piezas de 16x16 (formato .p16) -----------------------------------------
; Las mismas 14 tiras que arriba, en el formato de la capa de sprites del beat 'em up
; (scripts/piezas16.py; docs/diseno-beat-em-up.md §8 v3): índice por cuadro, tablas
; (dx, dy) y tiles en orden de subida. Hoy solo las usa la ROM de prueba de carga.
.section ".rodata_p16_reposo" superfree
tarou_reposo_p16: .incbin "res/personajes/tarou/tarou_reposo.p16"
.ends
.section ".rodata_p16_caminar" superfree
tarou_caminar_p16: .incbin "res/personajes/tarou/tarou_caminar.p16"
.ends
.section ".rodata_p16_correr" superfree
tarou_correr_p16: .incbin "res/personajes/tarou/tarou_correr.p16"
.ends
.section ".rodata_p16_saltar" superfree
tarou_saltar_p16: .incbin "res/personajes/tarou/tarou_saltar.p16"
.ends
.section ".rodata_p16_caer" superfree
tarou_caer_p16: .incbin "res/personajes/tarou/tarou_caer.p16"
.ends
.section ".rodata_p16_agachado" superfree
tarou_agachado_p16: .incbin "res/personajes/tarou/tarou_agachado.p16"
.ends
.section ".rodata_p16_agachado_caminar" superfree
tarou_agachado_caminar_p16: .incbin "res/personajes/tarou/tarou_agachado_caminar.p16"
.ends
.section ".rodata_p16_desenvainar" superfree
tarou_desenvainar_p16: .incbin "res/personajes/tarou/tarou_desenvainar.p16"
.ends
.section ".rodata_p16_envainar" superfree
tarou_envainar_p16: .incbin "res/personajes/tarou/tarou_envainar.p16"
.ends
.section ".rodata_p16_atacar" superfree
tarou_atacar_p16: .incbin "res/personajes/tarou/tarou_atacar.p16"
.ends
.section ".rodata_p16_echarse" superfree
tarou_echarse_p16: .incbin "res/personajes/tarou/tarou_echarse.p16"
.ends
.section ".rodata_p16_dormido" superfree
tarou_dormido_p16: .incbin "res/personajes/tarou/tarou_dormido.p16"
.ends
.section ".rodata_p16_despertar" superfree
tarou_despertar_p16: .incbin "res/personajes/tarou/tarou_despertar.p16"
.ends
.section ".rodata_p16_guardia" superfree
tarou_guardia_p16: .incbin "res/personajes/tarou/tarou_guardia.p16"
.ends
; Las del beat 'em up (2026-11-04): guardia, golpes 1-3, recibir y KO.
.section ".rodata_p16_t_guardia_bu" superfree
tarou_guardia_bu_p16: .incbin "res/personajes/tarou/tarou_guardia_bu.p16"
.ends
.section ".rodata_p16_t_golpe_1" superfree
tarou_golpe_1_p16: .incbin "res/personajes/tarou/tarou_golpe_1.p16"
.ends
.section ".rodata_p16_t_golpe_2" superfree
tarou_golpe_2_p16: .incbin "res/personajes/tarou/tarou_golpe_2.p16"
.ends
.section ".rodata_p16_t_golpe_3" superfree
tarou_golpe_3_p16: .incbin "res/personajes/tarou/tarou_golpe_3.p16"
.ends
.section ".rodata_p16_t_recibir" superfree
tarou_recibir_p16: .incbin "res/personajes/tarou/tarou_recibir.p16"
.ends
.section ".rodata_p16_t_ko" superfree
tarou_ko_p16: .incbin "res/personajes/tarou/tarou_ko.p16"
.ends

; El banco de cada tira .p16, en el orden del enum Estado la guardia y las del beat 'em up (TAROU_TIRA_*, juego/personaje.h).
.section ".rodata_p16_bancos" superfree
tarou_p16_bancos: .db :tarou_reposo_p16, :tarou_caminar_p16, :tarou_correr_p16, :tarou_saltar_p16, :tarou_caer_p16, :tarou_agachado_p16, :tarou_agachado_caminar_p16
                  .db :tarou_desenvainar_p16, :tarou_envainar_p16, :tarou_atacar_p16, :tarou_echarse_p16, :tarou_dormido_p16, :tarou_despertar_p16, :tarou_guardia_p16
                  .db :tarou_guardia_bu_p16, :tarou_golpe_1_p16, :tarou_golpe_2_p16, :tarou_golpe_3_p16, :tarou_recibir_p16, :tarou_ko_p16
.ends

; --- Bárbara en piezas de 16x16 (formato .p16, celda de 112x96) -----------------------
; Las tiras de los estados que ya tiene la fase 1 (Makefile: BARBARA_ANIMS). Cada una
; en su sección (un banco de 32 KB). barbara_p16_bancos va en el orden de las tiras de
; Bárbara de juego/personaje.h (BARBARA_TIRA_*).
.section ".rodata_bp16_guardia" superfree
barbara_guardia_p16: .incbin "res/personajes/barbara/barbara_guardia.p16"
.ends
.section ".rodata_bp16_caminar" superfree
barbara_caminar_p16: .incbin "res/personajes/barbara/barbara_caminar.p16"
.ends
.section ".rodata_bp16_saltar" superfree
barbara_saltar_p16: .incbin "res/personajes/barbara/barbara_saltar.p16"
.ends
.section ".rodata_bp16_caer" superfree
barbara_caer_p16: .incbin "res/personajes/barbara/barbara_caer.p16"
.ends
.section ".rodata_bp16_golpe_1" superfree
barbara_golpe_1_p16: .incbin "res/personajes/barbara/barbara_golpe_1.p16"
.ends
.section ".rodata_bp16_golpe_2" superfree
barbara_golpe_2_p16: .incbin "res/personajes/barbara/barbara_golpe_2.p16"
.ends
.section ".rodata_bp16_golpe_3" superfree
barbara_golpe_3_p16: .incbin "res/personajes/barbara/barbara_golpe_3.p16"
.ends
.section ".rodata_bp16_recibir" superfree
barbara_recibir_p16: .incbin "res/personajes/barbara/barbara_recibir.p16"
.ends
.section ".rodata_bp16_ko" superfree
barbara_ko_p16: .incbin "res/personajes/barbara/barbara_ko.p16"
.ends
.section ".rodata_bp16_correr" superfree
barbara_correr_p16: .incbin "res/personajes/barbara/barbara_correr.p16"
.ends
.section ".rodata_bp16_aereo" superfree
barbara_aereo_p16: .incbin "res/personajes/barbara/barbara_aereo.p16"
.ends
.section ".rodata_bp16_agarre" superfree
barbara_agarre_p16: .incbin "res/personajes/barbara/barbara_agarre.p16"
.ends
.section ".rodata_bp16_golpe_agarre" superfree
barbara_golpe_agarre_p16: .incbin "res/personajes/barbara/barbara_golpe_agarre.p16"
.ends
.section ".rodata_bp16_lanzamiento" superfree
barbara_lanzamiento_p16: .incbin "res/personajes/barbara/barbara_lanzamiento.p16"
.ends
.section ".rodata_bp16_especial" superfree
barbara_especial_p16: .incbin "res/personajes/barbara/barbara_especial.p16"
.ends
.section ".rodata_bp16_lanzar_arma" superfree
barbara_lanzar_arma_p16: .incbin "res/personajes/barbara/barbara_lanzar_arma.p16"
.ends
.section ".rodata_bp16_derribada" superfree
barbara_derribada_p16: .incbin "res/personajes/barbara/barbara_derribada.p16"
.ends
.section ".rodata_bp16_levantarse" superfree
barbara_levantarse_p16: .incbin "res/personajes/barbara/barbara_levantarse.p16"
.ends
.section ".rodata_bp16_agarrada" superfree
barbara_agarrada_p16: .incbin "res/personajes/barbara/barbara_agarrada.p16"
.ends
.section ".rodata_bp16_victoria" superfree
barbara_victoria_p16: .incbin "res/personajes/barbara/barbara_victoria.p16"
.ends
.section ".rodata_bp16_varios" superfree
barbara_pal:          .incbin "res/personajes/barbara/barbara_guardia.pal"
barbara_p16_bancos:   .db :barbara_guardia_p16, :barbara_caminar_p16, :barbara_saltar_p16, :barbara_caer_p16, :barbara_golpe_1_p16, :barbara_golpe_2_p16, :barbara_golpe_3_p16, :barbara_recibir_p16, :barbara_ko_p16, :barbara_correr_p16, :barbara_aereo_p16, :barbara_agarre_p16, :barbara_golpe_agarre_p16, :barbara_lanzamiento_p16, :barbara_especial_p16, :barbara_lanzar_arma_p16, :barbara_derribada_p16, :barbara_levantarse_p16, :barbara_agarrada_p16, :barbara_victoria_p16
.ends

; --- «Ninja 1» en piezas de 16x16 (formato .p16, celda de 96x96) ----------------------
; Las tiras de los estados del ninja (Makefile: NINJA_ANIMS). ninja1_p16_bancos va en el
; orden de NinjaTira (enemigos/ninja.h). La sombra sigue siendo la de la prueba de carga.
.section ".rodata_np16_guardia" superfree
ninja1_guardia_p16: .incbin "res/personajes/ninja/ninja_guardia.p16"
.ends
.section ".rodata_np16_caminar" superfree
ninja1_caminar_p16: .incbin "res/personajes/ninja/ninja_caminar.p16"
.ends
.section ".rodata_np16_ataque_1" superfree
ninja1_ataque_1_p16: .incbin "res/personajes/ninja/ninja_ataque_1.p16"
.ends
.section ".rodata_np16_recibir" superfree
ninja1_recibir_p16: .incbin "res/personajes/ninja/ninja_recibir.p16"
.ends
.section ".rodata_np16_derribado" superfree
ninja1_derribado_p16: .incbin "res/personajes/ninja/ninja_derribado.p16"
.ends
.section ".rodata_np16_levantarse" superfree
ninja1_levantarse_p16: .incbin "res/personajes/ninja/ninja_levantarse.p16"
.ends
.section ".rodata_np16_ko" superfree
ninja1_ko_p16: .incbin "res/personajes/ninja/ninja_ko.p16"
.ends
.section ".rodata_np16_varios" superfree
ninja1_pal:           .incbin "res/personajes/ninja/ninja_guardia.pal"
ninja1_p16_bancos:    .db :ninja1_guardia_p16, :ninja1_caminar_p16, :ninja1_ataque_1_p16, :ninja1_recibir_p16, :ninja1_derribado_p16, :ninja1_levantarse_p16, :ninja1_ko_p16
.ends

; --- El ninja mediano en piezas de 16x16 (celda de 128x112): mismas tiras y orden que «Ninja 1».
.section ".rodata_n2p16_guardia" superfree
ninja2_guardia_p16: .incbin "res/personajes/ninja_mediano/ninja_mediano_guardia.p16"
.ends
.section ".rodata_n2p16_caminar" superfree
ninja2_caminar_p16: .incbin "res/personajes/ninja_mediano/ninja_mediano_caminar.p16"
.ends
.section ".rodata_n2p16_ataque_1" superfree
ninja2_ataque_1_p16: .incbin "res/personajes/ninja_mediano/ninja_mediano_ataque_1.p16"
.ends
.section ".rodata_n2p16_recibir" superfree
ninja2_recibir_p16: .incbin "res/personajes/ninja_mediano/ninja_mediano_recibir.p16"
.ends
.section ".rodata_n2p16_derribado" superfree
ninja2_derribado_p16: .incbin "res/personajes/ninja_mediano/ninja_mediano_derribado.p16"
.ends
.section ".rodata_n2p16_levantarse" superfree
ninja2_levantarse_p16: .incbin "res/personajes/ninja_mediano/ninja_mediano_levantarse.p16"
.ends
.section ".rodata_n2p16_ko" superfree
ninja2_ko_p16: .incbin "res/personajes/ninja_mediano/ninja_mediano_ko.p16"
.ends
.section ".rodata_n2p16_varios" superfree
ninja2_pal:           .incbin "res/personajes/ninja_mediano/ninja_mediano_guardia.pal"
ninja2_p16_bancos:    .db :ninja2_guardia_p16, :ninja2_caminar_p16, :ninja2_ataque_1_p16, :ninja2_recibir_p16, :ninja2_derribado_p16, :ninja2_levantarse_p16, :ninja2_ko_p16
.ends

; --- El ninja grande en piezas de 16x16 (celda de 128x112): mismas tiras y orden que «Ninja 1».
.section ".rodata_n3p16_guardia" superfree
ninja3_guardia_p16: .incbin "res/personajes/ninja_grande/ninja_grande_guardia.p16"
.ends
.section ".rodata_n3p16_caminar" superfree
ninja3_caminar_p16: .incbin "res/personajes/ninja_grande/ninja_grande_caminar.p16"
.ends
.section ".rodata_n3p16_ataque_1" superfree
ninja3_ataque_1_p16: .incbin "res/personajes/ninja_grande/ninja_grande_ataque_1.p16"
.ends
.section ".rodata_n3p16_recibir" superfree
ninja3_recibir_p16: .incbin "res/personajes/ninja_grande/ninja_grande_recibir.p16"
.ends
.section ".rodata_n3p16_derribado" superfree
ninja3_derribado_p16: .incbin "res/personajes/ninja_grande/ninja_grande_derribado.p16"
.ends
.section ".rodata_n3p16_levantarse" superfree
ninja3_levantarse_p16: .incbin "res/personajes/ninja_grande/ninja_grande_levantarse.p16"
.ends
.section ".rodata_n3p16_ko" superfree
ninja3_ko_p16: .incbin "res/personajes/ninja_grande/ninja_grande_ko.p16"
.ends
.section ".rodata_n3p16_varios" superfree
ninja3_pal:           .incbin "res/personajes/ninja_grande/ninja_grande_guardia.pal"
ninja3_p16_bancos:    .db :ninja3_guardia_p16, :ninja3_caminar_p16, :ninja3_ataque_1_p16, :ninja3_recibir_p16, :ninja3_derribado_p16, :ninja3_levantarse_p16, :ninja3_ko_p16
.ends

; --- ROM de prueba de carga (docs/diseno-beat-em-up.md §9) ---------------------------
; El ninja provisional (el samurái del pack ×3, recortado a la celda de 64x96) en .p16,
; su paleta (3 variantes + la sombra) y la sombra (2 piezas). Los genera
; scripts/preparar_prueba_carga.py (.bin versionados: el pack de origen no se
; versiona). Solo los usa src/pantallas/prueba_carga.c (-DPRUEBA_CARGA): en la ROM
; normal nadie los nombra y wlalink -d los descarta.
.section ".rodata_prueba_n64_andar" superfree
ninja64_andar:        .incbin "res/prueba_carga/ninja64_andar.bin"
.ends
.section ".rodata_prueba_n64_ataque" superfree
ninja64_ataque:       .incbin "res/prueba_carga/ninja64_ataque.bin"
.ends
.section ".rodata_prueba_varios" superfree
ninja_pal:            .incbin "res/prueba_carga/ninja_pal.bin"
sombra_til:           .incbin "res/prueba_carga/sombra.bin"
sombra_piezas:        .db 0, 0, 16, 0      ; sus 2 piezas (dx, dy): izquierda y derecha
sombra_banco:         .db :sombra_piezas
ninja_bancos:         .db :ninja64_andar, :ninja64_ataque
.ends
