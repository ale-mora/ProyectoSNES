/*---------------------------------------------------------------------------------
    ejercicio_02_perro.c (SOLUCIÓN) — Ejercicio de la unidad 02: la máquina de
    estados de un perro (PARADO, CAMINANDO, SALTANDO) con un mando simulado.

    PROPÓSITO
      Un perro con tres estados y la misma forma que src/juego/tarou.c: una
      struct con un campo `estado`, un perro_iniciar (una vez) y un
      perro_actualizar (una vez por frame) con un switch que decide la
      transición según el estado y el mando; la física del aire va DESPUÉS del
      switch y depende de en_suelo, no del estado (como el paso 2 de
      tarou_actualizar). El mando sale de una cadena de teclas: cada carácter
      es un frame. Imprime cada frame y, al final, la secuencia de estados.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf, fgets), <string.h> (strlen), <stdint.h>.
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   tarou_actualizar, manejar_pie, saltar, aterrizar y
                cambiar_estado en src/juego/tarou.c; enum Estado y
                NOMBRES_ESTADO en src/juego/animaciones.h/.c.
      Compilar: gcc -std=c99 -Wall -Wextra -o ejercicio_02.exe ejercicio_02_perro.c
      Ejecutar: .\ejercicio_02.exe "..dd.b......ddxdxdddd."      (teclas como argumento)
                Get-Content ..\entrada_ejemplo.txt | .\ejercicio_02.exe  (desde un archivo)
                .\ejercicio_02.exe                             (escribirlas y pulsar Enter)

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      perro      Perro   -                  el protagonista de este ejercicio
      mando      Mando   derecha, saltar_pulsado (0/1)  lo que pide el jugador este frame
      teclas     char[]  hasta 80 teclas    una por frame: . d b x
      secuencia  char[]  texto              los estados por los que pasó

    → conceptos (README de la unidad 02): estado, transición, enum, switch,
      máquina de estados, patrón _iniciar/_actualizar
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* printf, fprintf, fgets.                           */
#include <string.h>                  /* strlen, strcat.                                   */
#include <stdint.h>                  /* uint8_t, int16_t.                                 */

/*
 * TECLAS (un carácter = un frame):
 *   .  nada pulsado          d  derecha mantenida
 *   b  pulsa saltar (B)      x  derecha + pulsa saltar
 *
 * SALIDA ESPERADA con la entrada de ../entrada_ejemplo.txt ("..dd.b......ddxdxdddd."):
 * capturas/salida_ejercicio_02_perro.txt. Su última línea es:
 *   Secuencia: PARADO > CAMINANDO > PARADO > SALTANDO > PARADO > CAMINANDO > SALTANDO > CAMINANDO > PARADO
 */

typedef uint8_t u8;                  /* Los mismos nombres de tipo que el juego.          */
typedef int16_t s16;

#define VEL_SALTO 3                  /* Al despegar sube 3 px el primer frame...          */
#define GRAVEDAD 1                   /* ...y cada frame sube 1 px menos.                  */
#define MAX_TECLAS 80                /* Frames como mucho.                                */

/* TODO 1: los estados. Uno solo a la vez; ESTADO_CANTIDAD no es un estado: cuenta. */
enum
{
    ESTADO_PARADO = 0,               /* Quieto en el suelo.                               */
    ESTADO_CAMINANDO,                /* Andando a la derecha, 1 px por frame.             */
    ESTADO_SALTANDO,                 /* En el aire.                                       */
    ESTADO_CANTIDAD                  /* Cuántos hay (3).                                  */
};

/* TODO 1 (sigue): un nombre por estado, en el MISMO orden que el enum. */
static const char *const NOMBRES[ESTADO_CANTIDAD] = {"PARADO", "CAMINANDO", "SALTANDO"};

typedef struct
{
    u8 derecha;                      /* 1 mientras se mantiene la derecha.                */
    u8 saltar_pulsado;               /* 1 solo en el frame en que se pulsa B.             */
} Mando;

typedef struct
{
    s16 x;                           /* Posición horizontal en píxeles.                   */
    s16 y;                           /* Altura sobre el suelo (0 = en el suelo).          */
    s16 vy;                          /* Velocidad vertical (positiva = sube).             */
    u8 estado;                       /* Un valor del enum: qué hace ahora.                */
    u8 en_suelo;                     /* 1 si está apoyado.                                */
} Perro;

/* Entra en un estado nuevo. En tarou.c también reinicia la animación. */
static void cambiar_estado(Perro *p, u8 nuevo)
{
    p->estado = nuevo;               /* Una sola variable dice qué hace.                  */
}

/* TODO 2: despegar: velocidad hacia arriba, fuera del suelo, estado SALTANDO. */
static void saltar(Perro *p)
{
    p->vy = VEL_SALTO;               /* Hacia arriba.                                     */
    p->en_suelo = 0;                 /* Ya no está apoyado: la física del aire actúa.     */
    cambiar_estado(p, ESTADO_SALTANDO);
}

/* TODO 6: al tocar el suelo, el estado depende del mando (como aterrizar en tarou.c). */
static void aterrizar(Perro *p, const Mando *m)
{
    if (m->derecha)
    {
        cambiar_estado(p, ESTADO_CAMINANDO);   /* Derecha mantenida: sigue andando.      */
    }
    else
    {
        cambiar_estado(p, ESTADO_PARADO);      /* Si no, se queda quieto.                */
    }
}

/* _iniciar: una vez, al empezar. Deja todo en un estado conocido. */
static void perro_iniciar(Perro *p)
{
    p->x = 0;
    p->y = 0;
    p->vy = 0;
    p->en_suelo = 1;
    cambiar_estado(p, ESTADO_PARADO);
}

/* _actualizar: una vez por frame. 1) transiciones según el estado; 2) física. */
static void perro_actualizar(Perro *p, const Mando *m)
{
    switch (p->estado)
    {
        case ESTADO_PARADO:          /* TODO 3: B salta (gana a la derecha); derecha camina. */
            if (m->saltar_pulsado)
            {
                saltar(p);
            }
            else if (m->derecha)
            {
                cambiar_estado(p, ESTADO_CAMINANDO);
                p->x++;              /* Ya anda en este mismo frame.                      */
            }
            break;
        case ESTADO_CAMINANDO:       /* TODO 4: B salta; sin derecha, para; si no, avanza. */
            if (m->saltar_pulsado)
            {
                saltar(p);
                p->x++;              /* Conserva el paso: salta hacia delante.            */
            }
            else if (!m->derecha)
            {
                cambiar_estado(p, ESTADO_PARADO);
            }
            else
            {
                p->x++;              /* Sigue andando.                                    */
            }
            break;
        case ESTADO_SALTANDO:        /* TODO 5: en el aire solo hay control horizontal.   */
            if (m->derecha)
            {
                p->x++;
            }
            break;
        default:
            break;                   /* Valor imposible: no hacer nada (como tarou.c).    */
    }

    /* Física del aire: depende de en_suelo, no del estado (ya escrita). */
    if (!p->en_suelo)
    {
        p->y = (s16)(p->y + p->vy);  /* Mover...                                          */
        p->vy = (s16)(p->vy - GRAVEDAD);  /* ...y la gravedad frena la subida.            */
        if (p->y <= 0)
        {
            p->y = 0;                /* En el suelo, ni un píxel por debajo.              */
            p->vy = 0;
            p->en_suelo = 1;
            aterrizar(p, m);         /* La transición de vuelta al suelo.                 */
        }
    }
}

int main(int argc, char *argv[])
{
    char teclas[MAX_TECLAS + 2];     /* +2: el salto de línea y el final de cadena.       */
    char secuencia[2048] = "";       /* Los estados por los que pasa, en texto.           */
    Perro perro;                     /* El único perro.                                   */
    Mando mando;                     /* Se rellena en cada frame a partir de una tecla.   */
    u8 anterior;                     /* Estado del frame anterior: para ver los cambios.  */
    size_t i;                        /* Posición en la cadena de teclas.                  */
    u8 frame = 0;                    /* Número de frame.                                  */

    if (argc > 1)                    /* Teclas como argumento...                          */
    {
        strncpy(teclas, argv[1], MAX_TECLAS);   /* Copia como mucho MAX_TECLAS.           */
        teclas[MAX_TECLAS] = '\0';              /* Por si eran más: cierra la cadena.     */
    }
    else                             /* ...o escritas en la consola (o desde un archivo). */
    {
        /* El aviso va a stderr: así no se mezcla con la salida que se compara. */
        fprintf(stderr, "Teclas (. d b x), una por frame, y Enter: ");
        if (fgets(teclas, sizeof(teclas), stdin) == NULL)
        {
            teclas[0] = '\0';        /* Nada que leer: cero frames.                       */
        }
    }

    perro_iniciar(&perro);           /* _iniciar: una vez.                                */
    strcat(secuencia, NOMBRES[perro.estado]);   /* El estado de partida.                  */
    printf("inicio:         %-9s x=%d y=%d\n", NOMBRES[perro.estado], perro.x, perro.y);

    for (i = 0; i < strlen(teclas); i++)        /* Un carácter = un frame.                */
    {
        char c = teclas[i];          /* La tecla de este frame.                           */
        if (c == '\r' || c == '\n')
        {
            continue;                /* El fin de línea (CR LF en Windows) no es un frame.*/
        }
        mando.derecha = (u8)(c == 'd' || c == 'x');          /* d o x: derecha.           */
        mando.saltar_pulsado = (u8)(c == 'b' || c == 'x');   /* b o x: pulsa B.           */
        anterior = perro.estado;     /* Para saber si este frame cambia de estado.        */
        frame++;                     /* Un frame más.                                     */

        perro_actualizar(&perro, &mando);       /* _actualizar: un frame de juego.        */

        printf("frame %2u [%c]:   %-9s x=%d y=%d", (unsigned)frame, c,
               NOMBRES[perro.estado], perro.x, perro.y);   /* El estado se lee por nombre. */
        if (perro.estado != anterior)            /* ¿Hubo transición?                     */
        {
            printf("  <- cambia");
            strcat(secuencia, " > ");            /* Se apunta en la secuencia.            */
            strcat(secuencia, NOMBRES[perro.estado]);
        }
        printf("\n");                /* Fin de la línea del frame.                        */
    }
    printf("Secuencia: %s\n", secuencia);
    return 0;                        /* Terminó bien.                                     */
}

