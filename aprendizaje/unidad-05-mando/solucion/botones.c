/*---------------------------------------------------------------------------------
    botones.c — SOLUCIÓN del ejercicio 05: el mando 1 en pantalla y un contador de B.

    (Es rom/src/botones.c con los TODO 1-4 de ejercicio_05.md resueltos.)

    PROPÓSITO
      Cada frame lee el mando 1 y muestra:
        HELD       los 12 botones; el nombre si está mantenido, puntos si no
        LAST DOWN  los últimos botones que BAJARON (recién pulsados) y en qué
                   frame: se "congela" en pantalla porque un recién pulsado dura
                   un solo frame y no se vería
        LAST UP    lo mismo con los que SUBIERON (recién soltados)
        ACTION     qué haría Tarou con lo pulsado, aplicando la prioridad del
                   juego (START > SELECT > R > A > B > Abajo > dirección); las
                   acciones de un solo frame (las de un botón recién pulsado) se
                   retienen RETENER frames en pantalla para que se vean
        PAD $xxxx  el valor crudo de 16 bits, para ver los bits
        B PRESSES  cuántas veces se ha pulsado B (ejercicio 05)
      Como el HUD del juego, reescribe una fila por frame, por turnos.
      Con -DGUION_MANDO no lee el mando: los botones salen de una tabla de
      tiempo (un GUION DE MANDO, como src/juego/guion.c) y la ROM se juega sola.
      Parte de tools/pvsneslib/snes-examples/input/controller.

    SE CONECTA CON
      Incluye:  <snes.h> (padsCurrent, padsDown, padsUp, KEY_*).
      Lo usan:  nadie: el arranque del SDK salta a main().
      Modelo:   src/juego/mando.c (bits → intenciones), src/juego/guion.c
                (mantenidos y recién pulsados desde una tabla).

    HARDWARE QUE TOCA
      El mando, a través de las variables que rellena la rutina de la NMI del
      SDK (pad_keys, pad_keysdown, pad_keysold; registros JOY1L/H $4218/$4219).
      VRAM/CGRAM/PPU para el texto, como en las unidades 03 y 04.

    VARIABLES (globales y estáticas de este archivo)
      frames            u16    0..65535       vueltas del bucle (el reloj del guion)
      mantenidos        u16    bits KEY_*     botones mantenidos este frame
      pulsados          u16    bits KEY_*     botones que bajaron este frame
      soltados          u16    bits KEY_*     botones que subieron este frame
      ultimo_pulsado    u16    bits KEY_*     los últimos pulsados (≠ 0) vistos
      frame_pulsado     u16    frames         cuándo
      ultimo_soltado    u16    bits KEY_*     los últimos soltados (≠ 0) vistos
      frame_soltado     u16    frames         cuándo
      turno             u8     0..FILAS-1     fila que se reescribe en este frame
      accion_vista      u8     ACCION_*       la acción que se muestra en ACTION
      pulsaciones_b     u16    0..65535       veces que B pasó de suelto a pulsado
      retener           u8     0..RETENER     frames que aún se retiene una acción de botón
      linea_texto       char[33]              la fila que se está componiendo
      BOTONES           Boton[12] (const)     máscara y nombre de cada botón

    → glosario: Mando (pad), Guion de mando, VBlank / NMI
---------------------------------------------------------------------------------*/
#include <snes.h>                    /* Toda la librería PVSnesLib.                        */

#define FILAS 6                      /* TODO 3: una fila más (B PRESSES).                 */
#define FILA_HELD 6                  /* Fila de pantalla de la primera.                   */
#define NUM_BOTONES 12               /* Botones del mando de la SNES.                     */
#define RETENER 30                   /* Frames que se ve una acción de un solo frame.     */

/* Las acciones, en orden de PRIORIDAD (la 0 gana a todas). Las cinco primeras salen
   de un botón recién pulsado (duran un frame); las otras, de botones mantenidos. */
enum { ACCION_TITULO, ACCION_HUD, ACCION_ESPADA, ACCION_ATAQUE, ACCION_SALTO,
       ACCION_AGACHARSE, ACCION_CAMINAR, ACCION_NADA };
#define ACCIONES_DE_PULSACION 5      /* Las de índice menor que esto duran un frame.      */
static const char *const NOMBRES_ACCION[8] = {   /* En pantalla, en inglés.               */
    "TITLE    ", "DEBUG HUD", "SWORD    ", "ATTACK   ",
    "JUMP     ", "CROUCH   ", "WALK     ", "IDLE     ",
};

/* Un botón: su máscara (un bit de KEY_*) y su nombre. 2 + 6 = 8 bytes: potencia de 2,
   así tcc816 no necesita multiplicar para llegar a BOTONES[i] (aunque aquí se
   recorre con un puntero, que es lo más barato). */
typedef struct
{
    u16 mascara;                     /* El bit del botón, p. ej. KEY_B = 0x8000.           */
    char nombre[6];                  /* Su nombre en pantalla (inglés), hasta 5 letras.   */
} Boton;

/* En el orden en que los manda el mando (bit 15 primero), salvo los cuatro de abajo. */
static const Boton BOTONES[NUM_BOTONES] = {
    {KEY_B, "B"},       {KEY_Y, "Y"},       {KEY_SELECT, "SEL"}, {KEY_START, "STA"},
    {KEY_UP, "UP"},     {KEY_DOWN, "DN"},   {KEY_LEFT, "LT"},    {KEY_RIGHT, "RT"},
    {KEY_A, "A"},       {KEY_X, "X"},       {KEY_L, "L"},        {KEY_R, "R"},
};

static u16 frames;                   /* Vueltas del bucle desde el arranque.              */
static u16 mantenidos;               /* Mantenidos este frame.                            */
static u16 pulsados;                 /* Recién pulsados (0 → 1) este frame.               */
static u16 soltados;                 /* Recién soltados (1 → 0) este frame.               */
static u16 ultimo_pulsado;           /* Últimos recién pulsados distintos de 0.           */
static u16 frame_pulsado;            /* El frame en que pasó.                             */
static u16 ultimo_soltado;           /* Últimos recién soltados distintos de 0.           */
static u16 frame_soltado;            /* El frame en que pasó.                             */
static u8 turno;                     /* Fila que toca reescribir.                         */
static u8 accion_vista;              /* La acción que se muestra.                         */
static u8 retener;                   /* Frames que le quedan a una acción de botón.       */
static u16 pulsaciones_b;            /* TODO 1: veces que se pulsó B.                     */
static char linea_texto[33];         /* 32 caracteres y el 0 final.                       */

#ifdef GUION_MANDO
/*--- Guion de mando: "del frame desde al frame hasta (sin incluirlo), mantener estos
    botones". 8 bytes por tramo (con relleno): potencia de 2. ---------------------*/
typedef struct
{
    u16 desde;                       /* Primer frame con los botones mantenidos.          */
    u16 hasta;                       /* Primer frame SIN ellos.                           */
    u16 botones;                     /* Bits KEY_* (se juntan con |).                     */
    u16 relleno;                     /* Sin uso: lleva el tramo a 8 bytes.                */
} Tramo;

#define TRAMOS 7                     /* Cuántos tramos tiene el guion.                    */
static const Tramo GUION[TRAMOS] = {
    {60, 120, KEY_B, 0},             /* 1 s: B mantenido un segundo (un salto largo).     */
    {150, 160, KEY_B, 0},            /* 2,5 s: un toque de B.                             */
    {200, 260, KEY_RIGHT, 0},        /* 3,3 s: Derecha mantenida (caminar)...             */
    {220, 230, KEY_A, 0},            /* ...y A a la vez: gana A (prioridad).              */
    {300, 310, KEY_B, 0},            /* 5 s: otro toque de B (el tercero).                */
    {330, 400, KEY_DOWN | KEY_LEFT, 0}, /* 5,5 s: Abajo + Izquierda: gana Abajo.          */
    {350, 356, KEY_R, 0},            /* ...y R por en medio: gana R.                      */
};

/* Los botones que el guion mantiene en el frame `cuadro`: se recorre la tabla con un
   puntero que avanza (t++), como mantenidos() en src/juego/guion.c. */
static u16 botones_guion(u16 cuadro)
{
    const Tramo *t = GUION;          /* Primer tramo.                                     */
    u16 b = 0;                       /* Ningún botón todavía.                             */
    u8 n;                            /* Tramos que quedan por mirar.                      */
    for (n = TRAMOS; n != 0; n--, t++)
    {
        if (cuadro >= t->desde && cuadro < t->hasta)
        {
            b |= t->botones;         /* Este tramo está activo: sus botones, dentro.      */
        }
    }
    return b;                        /* Los botones de este frame.                        */
}
#endif

/*--- leer_mando: rellena mantenidos, pulsados y soltados ----------------------------*/
static void leer_mando(void)
{
#ifdef GUION_MANDO
    u16 antes = mantenidos;          /* Lo del frame anterior.                            */
    mantenidos = botones_guion(frames);          /* Lo de ahora, de la tabla.            */
    pulsados = (u16)(mantenidos & ~antes);       /* Ahora sí y antes no: recién pulsados. */
    soltados = (u16)(antes & ~mantenidos);       /* Antes sí y ahora no: recién soltados. */
#else
    mantenidos = padsCurrent(0);     /* Macro: pad_keys[0], lo que dejó la NMI.           */
    pulsados = padsDown(0);          /* Macro: pad_keysdown[0].                           */
    soltados = padsUp(0);            /* Función: pad_keysold[0] & ~pad_keys[0].           */
#endif
}

/*--- poner_botones: escribe en p los 12 nombres; los que no están en `bits`, como
    puntos del mismo ancho (así las columnas no bailan). Devuelve dónde sigue. ------*/
static char *poner_botones(char *p, u16 bits)
{
    const Boton *b = BOTONES;        /* Recorrer la tabla con un puntero.                 */
    const char *n;                   /* El nombre del botón actual.                       */
    u8 k;                            /* Botones que quedan.                               */
    for (k = NUM_BOTONES; k != 0; k--, b++)
    {
        n = b->nombre;               /* Empezar por la primera letra del nombre.          */
        while (*n != 0)              /* Letra a letra.                                    */
        {
            if ((bits & b->mascara) != 0)   /* != 0: B es el bit 15; un (u8) lo perdería. */
            {
                *p++ = *n;           /* Pulsado: su letra.                                */
            }
            else
            {
                *p++ = '.';          /* No pulsado: un punto.                             */
            }
            n++;                     /* Siguiente letra.                                  */
        }
        *p++ = ' ';                  /* Separador.                                        */
    }
    return p;                        /* Dónde seguir escribiendo.                         */
}

/*--- poner_texto: copia una cadena en p y devuelve dónde sigue ---------------------*/
static char *poner_texto(char *p, const char *s)
{
    while (*s != 0)
    {
        *p++ = *s++;                 /* Carácter a carácter.                              */
    }
    return p;
}

/*--- accion: lo que haría Tarou este frame, con la prioridad del juego -------------
    docs/diseno-demo-jugable.md §4, «Prioridad el mismo frame»:
    START > SELECT > R > A > B > Abajo > dirección. Una cadena de if/else: el primero
    que se cumple gana y los demás ni se miran. */
static u8 accion(void)
{
    if ((pulsados & KEY_START) != 0)  { return ACCION_TITULO; }    /* Volver al título.   */
    if ((pulsados & KEY_SELECT) != 0) { return ACCION_HUD; }       /* HUD sí/no.          */
    if ((pulsados & KEY_R) != 0)      { return ACCION_ESPADA; }    /* Envainar/desenvainar.*/
    if ((pulsados & KEY_A) != 0)      { return ACCION_ATAQUE; }    /* Atacar.             */
    if ((pulsados & KEY_B) != 0)      { return ACCION_SALTO; }     /* Saltar.             */
    if ((mantenidos & KEY_DOWN) != 0) { return ACCION_AGACHARSE; } /* Agacharse.          */
    if ((mantenidos & (KEY_LEFT | KEY_RIGHT)) != 0) { return ACCION_CAMINAR; } /* Caminar.*/
    return ACCION_NADA;                                            /* Nada.               */
}

/* main devuelve int porque C lo exige: es el único int permitido en el código de la SNES. */
int main(void)
{
    char *p;                         /* Por dónde va la fila que se compone.              */

    /*--- 1. Consola de texto y Mode 1, como en las unidades 03 y 04 ----------------*/
    consoleSetTextMapPtr(0x6800);    /* Mapa de texto en la VRAM 0x6800.                   */
    consoleSetTextGfxPtr(0x3000);    /* Tiles de la fuente en 0x3000.                      */
    consoleInitDefaultText(0);       /* Fuente y paleta de la librería.                    */
    bgSetGfxPtr(0, 0x3000);          /* El fondo 0 usa esos tiles...                       */
    bgSetMapPtr(0, 0x6800, SC_32x32);/* ...y ese mapa.                                     */
    setMode(BG_MODE1, 0);            /* Mode 1.                                            */
    bgSetDisable(1);                 /* Solo el fondo 0...                                 */
    bgSetDisable(2);                 /* ...encendido.                                      */

    consoleDrawText(8, 2, "UNIT 05: THE PAD");           /* Título.                       */
#ifdef GUION_MANDO
    consoleDrawText(7, 3, "(SCRIPTED: NO HANDS)");       /* Aviso: se juega sola.         */
#endif

    /*--- 2. Variables: regla del proyecto, inicializar siempre (CLAUDE.md; unidad 13) */
    frames = 0;                      /* El reloj empieza en 0.                             */
    mantenidos = 0;                  /* Nada pulsado todavía.                              */
    pulsados = 0;                    /* Nada recién pulsado.                               */
    soltados = 0;                    /* Nada recién soltado.                               */
    ultimo_pulsado = 0;              /* Sin recuerdos.                                     */
    frame_pulsado = 0;               /* (frame 0).                                         */
    ultimo_soltado = 0;              /* Sin recuerdos.                                     */
    frame_soltado = 0;               /* (frame 0).                                         */
    turno = 0;                       /* Empezar por la fila HELD.                          */
    accion_vista = ACCION_NADA;      /* IDLE.                                              */
    retener = 0;                     /* Nada que retener.                                  */
    pulsaciones_b = 0;               /* TODO 1: empieza en 0 (a mano).                    */

    setScreenOn();                   /* Espera un VBlank y enciende.                      */

    /*--- 3. El bucle: leer, recordar, dibujar una fila, esperar ---------------------*/
    while (1)
    {
        leer_mando();                /* mantenidos, pulsados, soltados de este frame.     */
        if (pulsados != 0)           /* Algo bajó: recordarlo para que se vea.            */
        {
            ultimo_pulsado = pulsados;   /* Qué bajó...                                   */
            frame_pulsado = frames;      /* ...y cuándo.                                  */
        }
        if ((pulsados & KEY_B) != 0) /* TODO 2: B acaba de bajar (padsDown, no padsCurrent:
                                        mantener B no debe sumar 60 por segundo).         */
        {
            pulsaciones_b++;         /* Una pulsación más.                                */
        }
        if (soltados != 0)           /* Algo subió: ídem.                                 */
        {
            ultimo_soltado = soltados;   /* Qué subió...                                  */
            frame_soltado = frames;      /* ...y cuándo.                                  */
        }
        {
            u8 a = accion();         /* La acción de este frame, por prioridad.           */
            if (a < ACCIONES_DE_PULSACION)
            {
                accion_vista = a;    /* De un botón recién pulsado: mostrarla...          */
                retener = RETENER;   /* ...y retenerla medio segundo.                     */
            }
            else if (retener != 0)
            {
                retener--;           /* Todavía se ve la última acción de botón.          */
            }
            else
            {
                accion_vista = a;    /* Si no, la de los botones mantenidos.              */
            }
        }

        switch (turno)               /* ¿Qué fila toca?                                   */
        {
            case 0:                  /* HELD  B Y SEL STA UP DN LT RT A X L R              */
                consoleDrawText(1, FILA_HELD, "HELD:");
                p = poner_botones(linea_texto, mantenidos);
                *p = 0;              /* Fin de la cadena.                                 */
                consoleDrawText(0, FILA_HELD + 1, linea_texto);
                break;
            case 1:                  /* LAST DOWN: los últimos pulsados y su frame        */
                consoleDrawText(1, FILA_HELD + 3, "LAST DOWN AT FRAME %5u", frame_pulsado);
                p = poner_botones(linea_texto, ultimo_pulsado);
                *p = 0;              /* Fin de la cadena.                                 */
                consoleDrawText(0, FILA_HELD + 4, linea_texto);
                break;
            case 2:                  /* LAST UP: los últimos soltados y su frame          */
                consoleDrawText(1, FILA_HELD + 6, "LAST UP   AT FRAME %5u", frame_soltado);
                p = poner_botones(linea_texto, ultimo_soltado);
                *p = 0;              /* Fin de la cadena.                                 */
                consoleDrawText(0, FILA_HELD + 7, linea_texto);
                break;
            case 3:                  /* ACTION: la prioridad del juego                    */
                p = poner_texto(linea_texto, "ACTION ");
                p = poner_texto(p, NOMBRES_ACCION[accion_vista]);
                *p = 0;              /* Fin de la cadena.                                 */
                consoleDrawText(1, FILA_HELD + 9, linea_texto);
                break;
            case 5:                  /* TODO 4: B PRESSES n                               */
                consoleDrawText(1, FILA_HELD + 13, "B PRESSES %5u", pulsaciones_b);
                break;
            default:                 /* PAD $8000  FRAME 123                              */
                consoleDrawText(1, FILA_HELD + 11, "PAD $%04x  FRAME %5u", mantenidos, frames);
                break;
        }
        turno++;                     /* La fila siguiente, en el frame siguiente.         */
        if (turno >= FILAS)          /* ¿Se pasó de la última?                            */
        {
            turno = 0;               /* Vuelta a la primera.                              */
        }

        frames++;                    /* Un frame más (el reloj del guion).                */
        WaitForVBlank();             /* La NMI lee el mando para la vuelta siguiente.     */
    }
    return 0;                        /* Nunca se llega aquí.                              */
}
