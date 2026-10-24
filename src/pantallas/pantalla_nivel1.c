/*---------------------------------------------------------------------------------
    pantalla_nivel1.c — Pantalla (7): el nivel 1, donde se juega.

    PROPÓSITO
      Orquesta un frame del nivel: lee el mando, actualiza la lógica pura del
      protagonista, calcula la cámara, escribe el scroll de las dos capas y
      dibuja. Es la única pantalla con _vblank(): ahí se disparan, de un
      tirón, las copias a la VRAM que el frame dejó apuntadas en la lista de
      DMA (juego/dma_lista.h): los tiles del cuadro de Tarou, las columnas del
      fondo y las filas del HUD.
      Capas (Mode 1 con BG3 en prioridad alta): fondo 0 = capa CERCANA (el
      tutorial de 2048 px dibujado en Tiled y vestido con el arte de Pixellab,
      con el seto y los carteles; scroll 1:1 con la cámara y dibujado por
      columnas: juego/fondo_nivel.c); fondo 1 = capa
      LEJANA (montañas sin logotipo, 512 px que se repiten, scroll a la mitad); fondo 2 = HUD
      de depuración (texto de 4 colores, apagado al inicio); cielo = degradado
      HDMA del título. START vuelve al título con fundido.
      Flujo por frame (docs/diseno-demo-jugable.md §11): dma_lista_vaciar →
      mando_leer → tarou_actualizar (lógica pura) → partida_actualizar →
      camara_para → tarou_sprite_dibujar (OAM y, si cambió el cuadro, sus
      copias a la lista) → fondo_nivel_actualizar (las columnas, a la lista) →
      hud_debug_dibujar (su fila, a la lista); y, tras el VBlank, el scroll de
      las dos capas (bgSetScroll) y dma_lista_ejecutar. Al entrar (_iniciar) y al
      reaparecer, con la pantalla apagada, la lista se dispara en el acto.
      Se mide cuántas líneas del VBlank usa todo eso (diag_vblank, fila LIN
      del HUD): el VBlank tiene 37 y el SDK gasta 5 antes de llegar aquí.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h,
                juego/nivel.h, juego/camara.h, juego/mando.h, juego/tarou.h,
                juego/tarou_sprite.h, juego/fondo_nivel.h, juego/colision.h,
                juego/partida.h, juego/dma_lista.h y nivel1_datos.h (generado:
                inicio, carteles, tronco, salida).
      Lee de la ROM: nivel1_lejos_*, nivel1_colision y hdmaGradcieloList (data.asm); la capa
      cercana la carga juego/fondo_nivel.c.
      Lo usan:  sistema/secuencia.c (_iniciar, _actualizar y _vblank).

    HARDWARE QUE TOCA
      PPU: modo (setMode), fondos por comun.c y fondo_nivel.c (VRAM, CGRAM),
      scroll de los fondos 0 y 1 (bgSetScroll), BG2 apagado (bgSetDisable),
      pantalla apagada al reaparecer (setScreenOff: con "forced blank" la VRAM
      se escribe cuando se quiera). HDMA: canal 6 con un color por línea
      (setModeHdmaColor / setModeHdmaReset). DMA: la lista (dma_ejecutar.asm) en
      el VBlank. OAM por tarou_sprite.c. Mando: por mando.c. Contadores del
      haz ($2137/$213D) para los medidores LIN y V del HUD.

    VARIABLES (globales y estáticas de este archivo)
      camara  u16    0..ANCHO_NIVEL-ANCHO_PANTALLA (0..1792) px desplazamiento del nivel
      tarou   Tarou  ver juego/tarou.h                          el protagonista
      mando   Mando  ver juego/mando.h                          intenciones del jugador este frame

    → glosario: Nivel, Cámara, Parallax, BG / capas, HDMA, VBlank
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "juego/nivel.h"
#include "juego/camara.h"
#include "juego/tarou.h"
#include "juego/tarou_sprite.h"
#include "juego/mando.h"
#include "juego/hud_debug.h"
#include "juego/fondo_nivel.h"
#include "juego/dma_lista.h"
#include "juego/colision.h"
#include "juego/partida.h"
#include "nivel1_datos.h"            /* Generado por scripts/preparar_nivel.py (objetos). */
#include "sistema/puntofijo.h"

extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern const unsigned char hdmaGradcieloList[];   /* Tabla HDMA del cielo (gfx4snes -n). */
extern u8 nivel1_colision;                         /* Rejilla de 128x14 (data.asm).      */

#define X_INICIAL (NIVEL1_INICIO_X - ANCHO_CELDA / 2) /* La celda, centrada en el objeto
                                        «inicio» del .tmj.                                */

/* Medidor de coste del frame ("barra de raster" en números): leer $2137 congela los
   contadores del haz y $213D devuelve la línea actual (0..261 en NTSC, en dos lecturas:
   byte bajo y luego el bit 8). Si al terminar la lógica el haz va por la línea L, el
   frame ha usado (37 + L) de 262 líneas: L >= 225 significa que se pasó del frame y
   el juego baja de 60 FPS. PVSnesLib no da nombre a estos registros. */
#define REG_SLHV (*(volatile u8 *)0x2137)
#define REG_OPVCT (*(volatile u8 *)0x213D)
u16 diag_linea[3];                 /* Línea del haz tras: lógica, sprite, fin del frame.  */
u16 diag_vblank[2];                /* Líneas desde el INICIO del VBlank (225): al entrar
                                      en _vblank y al salir. El VBlank acaba en la 37 (la
                                      línea 0 del frame siguiente aún no se dibuja);
                                      pasar de ahí es escribir la VRAM con la PPU
                                      dibujando: los bytes se pierden.                  */

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u16 camara;                 /* Desplazamiento horizontal del nivel en pantalla.    */
static Tarou tarou;                /* El protagonista (lógica pura, juego/tarou.h).       */
static Mando mando;                /* Lo que el jugador quiere hacer este frame.          */
static Partida partida;            /* Reglas: puntos de control, tronco, salida.          */
static const u16 controles[] = NIVEL1_CONTROLES_X;  /* x de los carteles (generado).    */

/*--- leer_linea: línea del haz en este instante (ver REG_SLHV arriba) -------------*/
static u16 leer_linea(void)
{
    u8 bajo, alto;
    bajo = REG_SLHV;                                 /* Leer $2137 congela los contadores.  */
    bajo = REG_OPVCT;                                /* Primera lectura: bits 0..7.         */
    alto = (u8)(REG_OPVCT & 1);                      /* Segunda: bit 8.                     */
    return (u16)(((u16)alto << 8) | bajo);
}

/*--- desde_vblank: una línea del haz, contada desde el inicio del VBlank -------------
    El VBlank va de la línea 225 a la 261 y sigue en la 0 del frame siguiente (la PPU
    empieza a dibujar en la 1). Así, 225 → 0, 261 → 36, 0 → 37, 9 → 46. */
static u16 desde_vblank(u16 linea)
{
    if (linea >= 225)
    {
        return (u16)(linea - 225);
    }
    return (u16)(linea + 37);
}

/*--- salir: apaga lo que solo usa esta pantalla antes de cambiar -----------------*/
static void salir(Pantalla siguiente)
{
    fade_out(FRAMES_FADE);
    setModeHdmaReset(0);                             /* Corta el HDMA del cielo.            */
    tarou_sprite_ocultar();
    secuencia_cambiar(siguiente);
}

/*--- reaparecer: fundido a negro, Tarou al último punto de control, fundido ------
    Con la pantalla a oscuras se puede reescribir el anillo entero (la cámara salta). */
static void reaparecer(void)
{
    fade_out(FRAMES_FADE);
    setScreenOff();                                  /* "Forced blank": la VRAM se puede
                                                        escribir en cualquier momento, no
                                                        solo en el VBlank (son 4 KB).      */
    tarou_iniciar(&tarou, partida.reaparicion - ANCHO_CELDA / 2, SUELO_Y); /* En el cartel. */
    camara = camara_para(PF_ENTERO(tarou.x_fp));
    fondo_nivel_rellenar(camara);                    /* Las columnas de la nueva cámara.    */
    bgSetScroll(0, camara, 0);
    bgSetScroll(1, camara >> 1, 0);
    tarou_sprite_refrescar();                        /* Sus tiles, otra vez a la VRAM ya   */
    do                                               /* (pantalla apagada): media celda por */
    {                                                /* vuelta hasta que esté entero.       */
        dma_lista_vaciar();
        tarou_sprite_dibujar(&tarou, camara);
        dma_lista_ejecutar();
    } while (!tarou_sprite_al_dia(&tarou));
    fade_in(FRAMES_FADE);
}

void pantalla_nivel1_iniciar(void)
{
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);      /* Como el título: BG3 por delante.    */
    camara = camara_para(X_INICIAL);                 /* 0: cerca del borde izquierdo.       */
    fondo_nivel_parche(NIVEL1_TRONCO_COL, NIVEL1_PARCHE_COL); /* El tronco, entero.       */
    fondo_nivel_cargar(camara);                      /* Capa cercana: las columnas a la vista. */
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    fuente_2bpp(2);                                  /* Texto del HUD en el fondo 2...      */
    hud_debug_iniciar();                             /* ...apagado hasta que lo pida SELECT.*/
    mando_iniciar();                                 /* (Solo hace algo en la ROM de prueba.) */

    tarou_sprite_cargar();                           /* OAM lista + paleta de Tarou.        */
    colision_usar(&nivel1_colision);                 /* La física pregunta a esta rejilla.  */
    partida_iniciar(&partida, NIVEL1_INICIO_X, controles, NIVEL1_CONTROLES,
                    NIVEL1_TRONCO_X, NIVEL1_SALIDA_X); /* Y el tronco, sólido.              */
    tarou_iniciar(&tarou, X_INICIAL, SUELO_Y);       /* De pie sobre el suelo base.         */
    bgSetScroll(0, camara, 0);
    bgSetScroll(1, camara >> 1, 0);
    do                                               /* Primer cuadro antes del fade in, y  */
    {                                                /* sus tiles a la VRAM YA (la pantalla */
        dma_lista_vaciar();                          /* sigue apagada tras el fade out del  */
        tarou_sprite_dibujar(&tarou, camara);        /* título): media celda por vuelta.    */
        dma_lista_ejecutar();                        /* Sin esto, los 32 frames del fade in */
    } while (!tarou_sprite_al_dia(&tarou));          /* mostraban lo que hubiera en VRAM.   */
    diag_vblank[0] = diag_vblank[1] = 0;

    setModeHdmaColor(HDMA_CHANNEL6, (u8 *)&hdmaGradcieloList);   /* Cielo por línea.  */
    fade_in(FRAMES_FADE);
}

void pantalla_nivel1_actualizar(void)
{
    /*--- 0. La lista de copias a la VRAM de este frame empieza vacía ----------------*/
    dma_lista_vaciar();

    /*--- 1. Mando: de bits del pad a intenciones con nombre -------------------------*/
    mando_leer(&mando);
    if (mando.salir_pulsado)                         /* START: volver al título.            */
    {
        salir(PANTALLA_TITULO);
        return;
    }
    if (mando.hud_pulsado)                           /* SELECT: HUD de depuración sí/no.    */
    {
        hud_debug_alternar();
    }

    /*--- 2. Lógica pura: un frame de física y máquina de estados --------------------*/
    tarou_actualizar(&tarou, &mando);                /* Paredes, suelos y techos: colision.c. */
    switch (partida_actualizar(&partida, &tarou))    /* Las reglas del tutorial.            */
    {
        case PARTIDA_REAPARECER:                     /* Foso o zarza: al último cartel.     */
            reaparecer();
            return;
        case PARTIDA_TRONCO_ROTO:                    /* La espada rompió el tronco: sus     */
            fondo_nivel_quitar_parche();             /* columnas vuelven a ser las de debajo. */
            break;
        case PARTIDA_SALIDA:                         /* Fin del tutorial: al nivel 2.       */
            salir(PANTALLA_PROXIMAMENTE);
            return;
        default:
            break;
    }
    diag_linea[0] = leer_linea();                    /* Medida: coste de la lógica pura.    */

    /*--- 3. Cámara: solo se calcula aquí; el scroll se escribe en el VBlank --------
           (pantalla_nivel1_vblank). bgSetScroll escribe los registros en el acto: si
           se llamara aquí, a mitad del dibujo, la parte de abajo de la pantalla ya
           usaría la cámara nueva mientras las columnas que entran y la OAM de Tarou
           no llegan hasta el VBlank (unidad 07 de aprendizaje/). ---------------------*/
    camara = camara_para(PF_ENTERO(tarou.x_fp));

    /*--- 4. Tarou: entradas de la OAM del cuadro actual y, si cambió el cuadro, sus
           copias de tiles a la lista (primero: son lo más importante del VBlank);
           después las columnas del fondo que entran por los bordes ---------------*/
    tarou_sprite_dibujar(&tarou, camara);
    fondo_nivel_actualizar(camara);
    diag_linea[1] = leer_linea();                    /* Medida: + dibujo y lista de DMA.    */

    /*--- 5. HUD de depuración (solo escribe si está encendido) ----------------------*/
    hud_debug_dibujar(&tarou, camara);
    diag_linea[2] = leer_linea();                    /* Medida: fin del frame.              */
}

void pantalla_nivel1_vblank(void)
{
    diag_vblank[0] = desde_vblank(leer_linea());     /* Medida: dónde empieza (unos 5).     */
    bgSetScroll(0, camara, 0);                       /* Scroll de las dos capas, a la vez   */
    bgSetScroll(1, camara >> 1, 0);                  /* que sus columnas y que la OAM: la   */
                                                     /* cercana 1:1, la lejana a la mitad.  */
    dma_lista_ejecutar();                            /* Tiles de Tarou, columnas y HUD, de
                                                        un tirón.                           */
    diag_vblank[1] = desde_vblank(leer_linea());     /* Medida: dónde acaba (< 37 o se pierde). */
}
