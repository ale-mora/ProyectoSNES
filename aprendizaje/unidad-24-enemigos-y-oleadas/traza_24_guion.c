/*---------------------------------------------------------------------------------
    traza_24_guion.c — Repite en el PC la ROM autojugable de la fase 1 (-DGUION_FASE1).

    PROPÓSITO
      La ROM de prueba -DGUION_FASE1 lee el mando de la tabla GUION_F1 de src/juego/mando.c.
      Esta herramienta carga ESA MISMA tabla (extraída con sed; ver «Compilar») y repite el
      bucle de pantalla_fase1_actualizar con la lógica pura real (mundo.c, combate.c, ninja.c,
      oleadas.c, camara.c): sin dibujo, sin parada de impacto de A guardada (sí la parada).
      Imprime los SUCESOS con su cuadro: cuándo se bloquea cada zona, cuándo nace cada
      oleada, cuándo se vence, y la vida de Tarou. Es la forma de saber, sin abrir Mesen, si
      la tabla de teclas (que no mira lo que pasa: va por tiempo) sigue venciendo las
      oleadas cuando se cambia una cifra de la IA o de las zonas.

    SE CONECTA CON
      Incluye:  <stdio.h>, juego/mundo.h, juego/combate.h, juego/camara.h, juego/oleadas.h,
                enemigos/ninja.h, juego/nivel.h; y guion_f1.inc (la tabla, generada).
      Compilar (desde aprendizaje/unidad-24-enemigos-y-oleadas/; el sed saca la tabla de
      mando.c; en PowerShell, usa el sed de MSYS2 o copia las líneas a mano):
        sed -n "/^static const TramoFase1 GUION_F1/,/^};/p" ../../src/juego/mando.c > guion_f1.inc
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o traza_24.exe traza_24_guion.c
            ../../src/juego/mundo.c ../../src/juego/combate.c ../../src/enemigos/ninja.c
            ../../src/juego/oleadas.c ../../src/juego/camara.c
      (guion_f1.inc es generado: no se versiona.)

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      cuerpo, mando, oleadas, ninjas, camara, parada  los mismos que en pantalla_fase1.c
      cuadro_guion                                    cuadros desde el arranque (mando.c)

    → glosario: Guion de mando, Oleada, Zona
---------------------------------------------------------------------------------*/
#include <stdio.h>                       /* printf.                                     */
#include "juego/mundo.h"                 /* Cuerpo, mundo_*.                            */
#include "juego/combate.h"               /* combate_resolver, combate_ninjas_resolver.  */
#include "juego/camara.h"                /* camara_avanzar.                             */
#include "juego/oleadas.h"               /* Oleadas.                                    */
#include "enemigos/ninja.h"              /* Ninja.                                      */
#include "juego/nivel.h"                 /* ANCHO_PANTALLA.                             */

/* Las mismas definiciones que mando.c (aquí solo para leer la tabla). */
#define G_IZQ 1
#define G_DER 2
#define G_ARR 4
#define G_ABA 8
#define G_SAL 16
#define G_ATK 32
typedef struct
{
    u16 hasta;
    u8 teclas;
} TramoFase1;
#include "guion_f1.inc"                  /* static const TramoFase1 GUION_F1[] = {...}; */
#define TRAMOS (sizeof(GUION_F1) / sizeof(GUION_F1[0]))

#define MARGEN_NINJA 32                  /* Como pantalla_fase1.c.                      */

int main(void)
{
    Cuerpo cuerpo;
    Mando mando;
    Oleadas oleadas;
    Ninja ninjas[NINJAS_MAX];
    u16 camara = 0, cuadro_guion = 0, f;
    u8 parada = 0, teclas, teclas_antes = 0, k, turnos, golpe_ninja, i;
    u8 vida_antes = VIDA_TAROU;

    mundo_iniciar(&cuerpo, 64, 200);
    oleadas_iniciar(&oleadas);
    ninja_iniciar_todos(ninjas);
    printf("tramos de la tabla GUION_F1: %u\n", (unsigned)TRAMOS);
    for (f = 0; f < 1400; f++)
    {
        /*--- mando_leer, rama GUION_FASE1 (mando.c) ---------------------------------*/
        teclas = 0;
        for (i = 0; i < TRAMOS; i++)
        {
            if (cuadro_guion < GUION_F1[i].hasta)
            {
                teclas = GUION_F1[i].teclas;
                break;
            }
        }
        cuadro_guion++;
        mando.izquierda = (u8)((teclas & G_IZQ) != 0);
        mando.derecha = (u8)((teclas & G_DER) != 0);
        mando.arriba = (u8)((teclas & G_ARR) != 0);
        mando.abajo = (u8)((teclas & G_ABA) != 0);
        mando.saltar = (u8)((teclas & G_SAL) != 0);
        mando.saltar_pulsado = (u8)(mando.saltar && !(teclas_antes & G_SAL));
        mando.atacar_pulsado = (u8)((teclas & G_ATK) != 0 && !(teclas_antes & G_ATK));
        mando.correr = mando.espada_pulsado = 0;
        mando.hud_pulsado = mando.salir_pulsado = 0;
        mando.actividad = (u8)(teclas != 0);
        teclas_antes = teclas;

        /*--- pantalla_fase1_actualizar (sin dibujo ni parada guardada) -------------------*/
        if (parada)
        {
            parada--;
        }
        else
        {
            mundo_actualizar(&cuerpo, &mando);
            mundo_limitar_x(&cuerpo, (u16)(camara + MARGEN_PANTALLA),
                            (u16)(camara + ANCHO_PANTALLA - MARGEN_PANTALLA));
            camara = camara_avanzar(camara, cuerpo.x, oleadas_limite(&oleadas));
            if (oleadas_actualizar(&oleadas, camara))
            {
                u8 nacidos = ninja_oleada(ninjas, camara, oleadas.actual);

                printf("[f=%4u] zona %u bloqueada en camara %u: nacen %u ninja(s); Tarou en x=%u vida=%u\n",
                       f, oleadas.actual, camara, nacidos, cuerpo.x, cuerpo.vida);
            }
            parada = combate_resolver(&cuerpo, ninjas);
            turnos = ninja_atacando(ninjas);
            for (k = 0; k < NINJAS_MAX; k++)
            {
                if (ninjas[k].vivo)
                {
                    ninja_actualizar(ninjas + k, cuerpo.x, cuerpo.y);
                    ninja_limitar_x(ninjas + k, (u16)(camara + MARGEN_NINJA),
                                    (u16)(camara + ANCHO_PANTALLA - MARGEN_NINJA));
                    if (ninja_intentar_atacar(ninjas + k, cuerpo.x, cuerpo.y, (u8)(turnos < NINJA_TURNOS)))
                    {
                        turnos++;
                    }
                }
            }
            golpe_ninja = combate_ninjas_resolver(ninjas, &cuerpo);
            if (golpe_ninja > parada)
            {
                parada = golpe_ninja;
            }
            if (oleadas.bloqueada && ninja_vivos(ninjas) == 0)
            {
                oleadas_vencer(&oleadas);
                printf("[f=%4u] oleada VENCIDA; siguiente zona %u; Tarou vida=%u\n", f,
                       oleadas.actual, cuerpo.vida);
            }
        }
        if (cuerpo.vida != vida_antes)
        {
            printf("[f=%4u] Tarou recibe un tajo: vida %u -> %u\n", f, vida_antes, cuerpo.vida);
            vida_antes = cuerpo.vida;
        }
    }
    printf("fin de la tabla (cuadro %u): zona en curso %u, camara %u, Tarou x=%u vida=%u, ninjas vivos %u\n",
           f, oleadas.actual, camara, cuerpo.x, cuerpo.vida, ninja_vivos(ninjas));
    return 0;
}
