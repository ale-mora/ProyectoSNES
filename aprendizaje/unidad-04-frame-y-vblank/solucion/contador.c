/*---------------------------------------------------------------------------------
    contador.c — SOLUCIÓN del ejercicio 04: el contador de frames parpadea.

    (Es rom/src/contador.c con los TODO 1-4 de ejercicio_04.md resueltos.)

    PROPÓSITO
      Cuenta las vueltas del bucle principal (FRAMES), las convierte en
      segundos SIN dividir (un segundero que suma 1 cada SEGUNDOS(1) frames),
      y las pone al lado de los contadores del SDK: los VBlanks totales
      (snes_vblank_count) y los frames perdidos (lag_frame_counter). Si FRAMES
      sube igual que VBLANKS, el bucle no pierde ningún VBlank. La fila LINE
      dice por qué línea del haz iba la CPU al acabar el trabajo del frame,
      como la fila LIN del HUD del juego.
      Para no gastar de más, solo se reescribe UNA fila por frame, por turnos
      (la misma idea que el HUD de depuración del juego): cada número se
      refresca cada FILAS frames.
      Con -DNMI_PROPIA (README, «nmiSet»), además instala su propia rutina de
      VBlank con nmiSet, que cuenta interrupciones y llama a consoleVblank.
      Parte de tools/pvsneslib/snes-examples/systems/timer.

    SE CONECTA CON
      Incluye:  <snes.h> (PVSnesLib).
      Lo usan:  nadie: el arranque del SDK salta a main().
      Modelo:   src/sistema/main.c (el bucle), src/sistema/pantallas.h (SEGUNDOS),
                src/pantallas/pantalla_nivel1.c (leer_linea, los medidores LIN y V).

    HARDWARE QUE TOCA
      VRAM y CGRAM (fuente y texto, por la consola del SDK), PPU (BGMODE, TM,
      INIDISP), contadores del haz SLHV $2137 / OPVCT $213D (solo lectura).
      Con NMI_PROPIA: NMITIMEN $4200 (lo escribe nmiSet).

    VARIABLES (globales y estáticas de este archivo)
      frames              u16  0..65535      vueltas del bucle desde el arranque
      segundos            u16  0..65535      segundos enteros transcurridos
      frames_del_segundo  u8   0..59         frames dentro del segundo actual
      turno               u8   0..FILAS-1    fila que se reescribe en este frame
      linea               u16  0..261        línea del haz al acabar el frame anterior
      nmi_contadas        u16  0..65535      (solo NMI_PROPIA) veces que corrió mi_vblank
      frames_parpadeo     u8   0..29         frames desde el último cambio de visible
      visible             u8   0 / 1         1 = la fila FRAMES se muestra

    → glosario: Frame, VBlank / NMI, snes_vblank_count, Línea del haz, Consola de texto
---------------------------------------------------------------------------------*/
#include <snes.h>                    /* Toda la librería PVSnesLib.                        */

/*--- Tiempo en frames: las mismas macros que src/sistema/pantallas.h --------------*/
#define FPS 60                       /* NTSC: 60 frames por segundo.                       */
#define SEGUNDOS(s) ((u16)((s) * FPS)) /* Segundos → frames, calculado al compilar.        */
#define FILAS 5                      /* Filas de números que se reescriben por turnos.     */
#define COLUMNA 6                    /* Columna donde empiezan las filas de números.       */
#define FILA_FRAMES 8                /* Fila de pantalla de la primera de ellas.           */
/* TODO 1: medio segundo en frames. La división es entre constantes: la hace el
   compilador (30) y en la ROM no hay ninguna. */
#define MEDIO_SEGUNDO (SEGUNDOS(1) / 2)

/*--- Contadores del haz (no tienen nombre en PVSnesLib; igual que pantalla_nivel1.c):
    leer $2137 congela los contadores y $213D da la línea en dos lecturas. ---------*/
#define REG_SLHV (*(volatile u8 *)0x2137)   /* Leerlo "fotografía" la posición del haz.   */
#define REG_OPVCT (*(volatile u8 *)0x213D)  /* Línea: primero bits 0..7, luego el bit 8. */

static u16 frames;                   /* Vueltas del bucle principal.                       */
static u16 segundos;                 /* Segundos enteros.                                  */
static u8 frames_del_segundo;        /* Frames dentro del segundo actual (0..59).          */
static u8 turno;                     /* Qué fila se reescribe en este frame.               */
static u16 linea;                    /* Línea del haz al terminar el frame anterior.       */
/* TODO 2: el estado del parpadeo. */
static u8 frames_parpadeo;           /* Frames desde el último cambio (0..MEDIO_SEGUNDO-1). */
static u8 visible;                   /* 1 = la fila FRAMES se ve; 0 = está en blanco.      */

#ifdef NMI_PROPIA
/* consoleVblank está en la librería (libc.obj) pero ninguna cabecera de la 4.6.0 la
   declara: hay que anunciarla aquí. Es la rutina que consoleInit deja instalada y la
   que copia el texto de consoleDrawText a la VRAM. */
extern void consoleVblank(void);
static u16 nmi_contadas;             /* Veces que se ejecutó mi_vblank.                    */

/* Rutina propia de VBlank: el SDK la llama en CADA VBlank (también en los frames
   perdidos), entre la copia de la OAM y la lectura del mando. Tiene que ser corta. */
static void mi_vblank(void)
{
    nmi_contadas++;                  /* Contar la interrupción.                            */
    consoleVblank();                 /* Sin esto, el texto dejaría de copiarse a la VRAM.  */
}
#endif

/*--- leer_linea: la línea del haz en este instante (0..261 en NTSC) ---------------*/
static u16 leer_linea(void)
{
    u8 bajo, alto;                   /* Las dos mitades del número de línea.               */
    bajo = REG_SLHV;                 /* Congelar los contadores (el valor leído no importa). */
    bajo = REG_OPVCT;                /* Primera lectura: bits 0..7.                        */
    alto = (u8)(REG_OPVCT & 1);      /* Segunda lectura: el bit 8.                         */
    return (u16)(((u16)alto << 8) | bajo);  /* Juntar las dos mitades.                    */
}

/* main devuelve int porque C lo exige: es el único int permitido en el código de la SNES. */
int main(void)
{
    /*--- 1. Consola de texto y Mode 1, como en la unidad 03 -----------------------*/
    consoleSetTextMapPtr(0x6800);    /* Mapa de texto en la VRAM 0x6800.                   */
    consoleSetTextGfxPtr(0x3000);    /* Tiles de la fuente en 0x3000.                      */
    consoleInitDefaultText(0);       /* Fuente y paleta de la librería.                    */
    bgSetGfxPtr(0, 0x3000);          /* El fondo 0 usa esos tiles...                       */
    bgSetMapPtr(0, 0x6800, SC_32x32);/* ...y ese mapa.                                     */
    setMode(BG_MODE1, 0);            /* Mode 1.                                            */
    bgSetDisable(1);                 /* Solo el fondo 0.                                   */
    bgSetDisable(2);                 /* (y el fondo 2, también).                          */

    /*--- 2. Los textos fijos (en inglés: es texto en pantalla) --------------------*/
    consoleDrawText(7, 3, "UNIT 04: THE FRAME");  /* Título.                               */
    consoleDrawText(3, 20, "FRAMES = LOOPS OF MAIN"); /* Leyenda.                          */
    consoleDrawText(3, 21, "VBLANKS = NMI COUNT (SDK)");  /* Segunda línea de leyenda. */

    /*--- 3. Variables: regla del proyecto, inicializar siempre (CLAUDE.md; unidad 13) */
    frames = 0;                      /* Regla del proyecto: inicializar siempre.          */
    segundos = 0;                    /* Ningún segundo todavía.                           */
    frames_del_segundo = 0;          /* Al principio de un segundo.                       */
    turno = 0;                       /* Empezar por la fila FRAMES.                       */
    linea = 0;                       /* Aún no se ha medido nada.                         */
    frames_parpadeo = 0;             /* TODO 2: también estas, a mano.                     */
    visible = 1;                     /* Empieza visible.                                   */
#ifdef NMI_PROPIA
    nmi_contadas = 0;                /* Ninguna NMI propia todavía.                       */
#endif

    setScreenOn();                   /* Espera un VBlank y enciende a brillo 15.          */
    lag_frame_counter = 0;           /* Olvidar los VBlanks "perdidos" del arranque: el SDK
                                        cuenta como perdido todo VBlank que llega sin el
                                        bucle esperando (interrupt.h permite cambiarlo).  */
#ifdef NMI_PROPIA
    nmiSet(mi_vblank);               /* Desde aquí, el SDK llama a mi_vblank en cada VBlank. */
#endif

    /*--- 4. El bucle: una vuelta = un frame (si no se pasa de tiempo) --------------*/
    while (1)
    {
        frames++;                    /* Una vuelta más.                                   */
        frames_del_segundo++;        /* Y un frame más en este segundo.                   */
        if (frames_del_segundo >= SEGUNDOS(1))   /* 60 frames = 1 segundo, sin dividir.  */
        {
            frames_del_segundo = 0;  /* Empieza otro segundo...                           */
            segundos++;              /* ...y el segundero avanza.                         */
        }

        /* TODO 3: cada MEDIO_SEGUNDO frames, visible cambia de 1 a 0 o de 0 a 1. */
        frames_parpadeo++;           /* Un frame más desde el último cambio.              */
        if (frames_parpadeo >= MEDIO_SEGUNDO)
        {
            frames_parpadeo = 0;     /* Empezar a contar otro medio segundo...            */
            visible = (u8)(visible ^ 1);  /* ...y alternar: 1 ^ 1 = 0, 0 ^ 1 = 1.         */
        }

        /* Una sola fila por frame (consoleDrawText con %u formatea el número: es caro). */
        switch (turno)               /* ¿Qué fila toca?                                   */
        {
            case 0:                  /* TODO 4: el número o espacios, según visible.      */
                if (visible)
                {
                    consoleDrawText(COLUMNA, FILA_FRAMES, "FRAMES  %5u", frames);
                }
                else
                {
                    consoleDrawText(COLUMNA, FILA_FRAMES, "             ");  /* 13 espacios. */
                }
                break;
            case 1:
                consoleDrawText(COLUMNA, FILA_FRAMES + 2, "SECONDS %5u", segundos);
                break;
            case 2:
                consoleDrawText(COLUMNA, FILA_FRAMES + 4, "VBLANKS %5u", snes_vblank_count);
                break;
            case 3:
                consoleDrawText(COLUMNA, FILA_FRAMES + 6, "LAG     %5u", lag_frame_counter);
                break;
            default:
#ifdef NMI_PROPIA
                consoleDrawText(COLUMNA, FILA_FRAMES + 8, "LINE %3u NMI %5u", linea, nmi_contadas);
#else
                consoleDrawText(COLUMNA, FILA_FRAMES + 8, "LINE    %5u", linea);
#endif
                break;
        }
        turno++;                     /* La fila siguiente, en el frame siguiente.         */
        if (turno >= FILAS)          /* ¿Se pasó de la última?                            */
        {
            turno = 0;               /* Vuelta a la primera.                              */
        }

        linea = leer_linea();        /* ¿Por qué línea va el haz al acabar el trabajo?    */
        WaitForVBlank();             /* Dormir hasta el VBlank: esto fija los 60 por segundo. */
    }
    return 0;                        /* Nunca se llega aquí.                              */
}
