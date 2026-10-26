/*---------------------------------------------------------------------------------
    prueba_dialogos.c — Pruebas de juego/dialogos.c y escena/escena_dialogo.c.

    PROPÓSITO
      Fijar los textos de las escenas de la fase 1 (guion-historia.md §11): cada escena
      con su número de cuadros, todas las líneas dentro de 26 caracteres y en ASCII (la fuente
      de 8x8 no tiene acentos), quién habla en cada cuadro; y las dos tablas del motor de
      escenas para entrar y salir de un diálogo: sin saltos, de 15 a 6 de brillo y de 0 a 5 de
      mosaico y de vuelta, en 24 frames.

    SE CONECTA CON
      Incluye:  minitest.h, juego/dialogos.h, juego/fase1.h, escena/escena.h, escena/escena_dialogo.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_dialogos).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Diálogo, Motor de escenas
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/dialogos.h"
#include "juego/fase1.h"
#include "escena/escena.h"
#include "escena/escena_dialogo.h"

/* 1 si todos los caracteres son ASCII imprimible. */
static u8 es_ascii(const char *s)
{
    while (*s)
    {
        if ((unsigned char)*s < 32 || (unsigned char)*s > 126)
            return 0;
        s++;
    }
    return 1;
}

/* Corre una tabla entera y devuelve cuántas llamadas hicieron falta; anota el valor en cada frame. */
static u16 correr_tabla(const EventoEscena *tabla, u8 *brillo, u8 *mosaico)
{
    Escena e;
    u16 n;

    escena_empezar(&e, tabla);
    n = 0;
    while (!e.terminada && n < 200)
    {
        escena_avanzar(&e);
        brillo[n] = ESCENA_VALOR(&e, PISTA_BRILLO);
        mosaico[n] = ESCENA_VALOR(&e, PISTA_MOSAICO);
        n++;
    }
    return n;
}

void pruebas_dialogos(void)
{
    u8 escenas[5];
    u8 cuadros[5];
    u8 i, n;
    u8 brillo[200], mosaico[200];
    u16 llamadas;

    escenas[0] = ESC_LUCINDA;      cuadros[0] = 3;
    escenas[1] = ESC_FENRIR_GAG;   cuadros[1] = 1;
    escenas[2] = ESC_REX;          cuadros[2] = 4;
    escenas[3] = ESC_JEFE_ENTRA;   cuadros[3] = 1;
    escenas[4] = ESC_VILLANO_HUYE; cuadros[4] = 1;

    /* Cuántos cuadros tiene cada escena, y que las que no tienen texto devuelven 0 y cadenas vacías. */
    for (i = 0; i < 5; i++)
        COMPROBAR(dialogo_cuadros(escenas[i]) == cuadros[i]);
    COMPROBAR(dialogo_cuadros(ESC_NADA) == 0 && dialogo_cuadros(99) == 0);
    COMPROBAR(dialogo_orador(ESC_NADA, 0)[0] == 0 && dialogo_linea1(ESC_REX, 9)[0] == 0);

    /* Cada cuadro: orador y primera línea no vacíos, todo en ASCII y dentro de 26 caracteres. */
    for (i = 0; i < 5; i++)
    {
        for (n = 0; n < cuadros[i]; n++)
        {
            COMPROBAR(strlen(dialogo_orador(escenas[i], n)) > 0);
            COMPROBAR(strlen(dialogo_linea1(escenas[i], n)) > 0);
            COMPROBAR(strlen(dialogo_orador(escenas[i], n)) <= DIALOGO_LINEA_MAX);
            COMPROBAR(strlen(dialogo_linea1(escenas[i], n)) <= DIALOGO_LINEA_MAX);
            COMPROBAR(strlen(dialogo_linea2(escenas[i], n)) <= DIALOGO_LINEA_MAX);
            COMPROBAR(es_ascii(dialogo_orador(escenas[i], n)) && es_ascii(dialogo_linea1(escenas[i], n))
                      && es_ascii(dialogo_linea2(escenas[i], n)));
        }
    }

    /* Textos concretos del guion (§11.3 y §11.4). */
    COMPROBAR(strcmp(dialogo_orador(ESC_LUCINDA, 0), "LUCINDA") == 0);
    COMPROBAR(strcmp(dialogo_linea1(ESC_LUCINDA, 0), "You two. Fight me. Now.") == 0);
    COMPROBAR(strcmp(dialogo_orador(ESC_LUCINDA, 1), "BARBARA") == 0);
    COMPROBAR(strcmp(dialogo_linea2(ESC_LUCINDA, 2), "I'm coming.") == 0);
    COMPROBAR(strcmp(dialogo_linea1(ESC_REX, 0), "That is no toy, young one.") == 0);   /* 26: justo. */
    COMPROBAR(strcmp(dialogo_linea2(ESC_REX, 1), "Nikoraemon wants it.") == 0);
    COMPROBAR(strcmp(dialogo_orador(ESC_REX, 3), "BARBARA") == 0);
    COMPROBAR(strcmp(dialogo_orador(ESC_JEFE_ENTRA, 0), "GOMASURI-DONO") == 0);
    COMPROBAR(strcmp(dialogo_linea1(ESC_VILLANO_HUYE, 0), "Fools! The relic is mine!") == 0);
    COMPROBAR(strcmp(dialogo_linea1(ESC_FENRIR_GAG, 0), "WHEEE!") == 0);

    /* Entrar: 24 frames, brillo de 15 a 6 y mosaico de 0 a 5, sin saltos hacia atrás ni de más de 1 punto. */
    llamadas = correr_tabla(ESCENA_DIALOGO_ENTRA, brillo, mosaico);
    COMPROBAR(llamadas == ESCENA_DIALOGO_FRAMES + 1);          /* La última llamada ve el FIN.   */
    COMPROBAR(brillo[0] >= 14 && mosaico[0] <= 1);
    COMPROBAR(brillo[llamadas - 2] == ESCENA_DIALOGO_BRILLO);  /* El último frame con rampa.      */
    COMPROBAR(mosaico[llamadas - 2] == ESCENA_DIALOGO_MOSAICO);
    for (i = 1; i < ESCENA_DIALOGO_FRAMES; i++)
    {
        COMPROBAR(brillo[i] <= brillo[i - 1] && brillo[i - 1] - brillo[i] <= 1);
        COMPROBAR(mosaico[i] >= mosaico[i - 1] && mosaico[i] - mosaico[i - 1] <= 1);
    }

    /* Salir: lo contrario. */
    llamadas = correr_tabla(ESCENA_DIALOGO_SALE, brillo, mosaico);
    COMPROBAR(llamadas == ESCENA_DIALOGO_FRAMES + 1);
    COMPROBAR(brillo[0] <= ESCENA_DIALOGO_BRILLO + 1 && mosaico[0] >= ESCENA_DIALOGO_MOSAICO - 1);
    COMPROBAR(brillo[llamadas - 2] == 15 && mosaico[llamadas - 2] == 0);
    for (i = 1; i < ESCENA_DIALOGO_FRAMES; i++)
    {
        COMPROBAR(brillo[i] >= brillo[i - 1] && brillo[i] - brillo[i - 1] <= 1);
        COMPROBAR(mosaico[i] <= mosaico[i - 1] && mosaico[i - 1] - mosaico[i] <= 1);
    }
}
