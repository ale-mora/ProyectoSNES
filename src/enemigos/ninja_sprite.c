/*---------------------------------------------------------------------------------
    ninja_sprite.c — Los ninjas y sus sombras en pantalla (ver ninja_sprite.h).

    PROPÓSITO
      Cada frame, para cada ranura: (1) si no está viva, sus dos entidades se apagan (n = 0)
      y se olvida el cuadro que tenía en la VRAM; (2) si está viva y su cuadro no es el que
      hay en su hueco, se sube entero (turno: solo UN ninja por frame, en rotación);
      (3) el arte es el de «Ninja 1» (celda de 96x96): ninja_dibujo() dice qué dibujo toca
      (guardia, caminar, puñetazo, recibir, derribado, levantarse, KO) y solo el KO tumbado
      parpadea antes de irse (ninja_parpadea); (4) se coloca el cuerpo (x0, y0 en pantalla), la mirada (espejo y tabla de piezas
      espejada) y la sombra. Al final, sprites_ordenar() deja todo en orden por profundidad.
      Es el mismo método que midió la prueba de carga (pantallas/prueba_carga.c).

    SE CONECTA CON
      Incluye:  <snes.h>, enemigos/ninja_sprite.h, sistema/comun.h, juego/sprites.h,
                juego/subida.h, juego/p16.h, juego/dma_lista.h.
      Lo usan:  pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      Ver ninja_sprite.h.

    VARIABLES (estáticas de este archivo)
      muestra   u8[NINJAS_MAX]  cuadro que hay entero en el hueco de cada ranura (SIN_CUADRO: nada)
      mirada    u8[NINJAS_MAX]  mirada con que se calcularon las piezas del cuerpo
      turno     u8              ranura que tiene la preferencia para subir este frame
      subida    Subida          el plan de copias de la subida de este frame
    → glosario: Doble búfer, Pieza de 16×16, OAM
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "enemigos/ninja_sprite.h"
#include "sistema/comun.h"
#include "juego/sprites.h"
#include "juego/subida.h"
#include "juego/p16.h"
#include "juego/dma_lista.h"

extern char ninja1_guardia_p16, ninja1_caminar_p16, ninja1_ataque_1_p16, ninja1_recibir_p16;
extern char ninja1_derribado_p16, ninja1_levantarse_p16, ninja1_ko_p16;
extern char ninja2_guardia_p16, ninja2_caminar_p16, ninja2_ataque_1_p16, ninja2_recibir_p16;
extern char ninja2_derribado_p16, ninja2_levantarse_p16, ninja2_ko_p16;
extern char ninja3_guardia_p16, ninja3_caminar_p16, ninja3_ataque_1_p16, ninja3_recibir_p16;
extern char ninja3_derribado_p16, ninja3_levantarse_p16, ninja3_ko_p16;
extern char ninja1_pal, ninja2_pal, ninja3_pal, sombra_piezas;
extern u8 ninja1_p16_bancos[], ninja2_p16_bancos[], ninja3_p16_bancos[];   /* Banco de cada tira. */
extern u8 sombra_banco;

#define ENT_NINJA 2                  /* Entradas 2..4: los cuerpos.                       */
#define ENT_SOMBRA_NINJA 5           /* Entradas 5..7: las sombras.                       */
#define ENTIDADES 8                  /* Tarou (0), su sombra (1) y las seis de los ninjas. */
#define TILE_SOMBRA 480              /* Par 15: la sombra (la sube protagonista_sprite.c). */
#define PALETA_SOMBRA 4
#define PRIORIDAD 3
#define SOMBRA_ALTO 10
#define SIN_CUADRO 0xFF
#define VRAM_TILE(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))   /* 16 palabras por tile.  */

static const u16 HUECOS[NINJAS_MAX] = {192, 288, 384};   /* Primer tile de cada hueco.   */

/* Cada tipo (normal, mediano, grande): su celda. */
static const u8 ANCHO_TIPO[NINJA_TIPOS] = {NINJA_ANCHO, 128, 128};
static const u8 ALTO_TIPO[NINJA_TIPOS] = {NINJA_ALTO, 112, 112};

/*--- tira_tipo: la tira número s (0 guardia .. 6 KO) de un tipo, y su banco ----------*/
static const u8 *tira_tipo(u8 tipo, u8 s, u8 *banco)
{
    if (tipo == NINJA_GRANDE)
    {
        *banco = ninja3_p16_bancos[s];
        switch (s)
        {
            case 1: return (const u8 *)&ninja3_caminar_p16;
            case 2: return (const u8 *)&ninja3_ataque_1_p16;
            case 3: return (const u8 *)&ninja3_recibir_p16;
            case 4: return (const u8 *)&ninja3_derribado_p16;
            case 5: return (const u8 *)&ninja3_levantarse_p16;
            case 6: return (const u8 *)&ninja3_ko_p16;
            default: return (const u8 *)&ninja3_guardia_p16;
        }
    }
    if (tipo == NINJA_MEDIANO)
    {
        *banco = ninja2_p16_bancos[s];
        switch (s)
        {
            case 1: return (const u8 *)&ninja2_caminar_p16;
            case 2: return (const u8 *)&ninja2_ataque_1_p16;
            case 3: return (const u8 *)&ninja2_recibir_p16;
            case 4: return (const u8 *)&ninja2_derribado_p16;
            case 5: return (const u8 *)&ninja2_levantarse_p16;
            case 6: return (const u8 *)&ninja2_ko_p16;
            default: return (const u8 *)&ninja2_guardia_p16;
        }
    }
    *banco = ninja1_p16_bancos[s];
    switch (s)
    {
        case 1: return (const u8 *)&ninja1_caminar_p16;
        case 2: return (const u8 *)&ninja1_ataque_1_p16;
        case 3: return (const u8 *)&ninja1_recibir_p16;
        case 4: return (const u8 *)&ninja1_derribado_p16;
        case 5: return (const u8 *)&ninja1_levantarse_p16;
        case 6: return (const u8 *)&ninja1_ko_p16;
        default: return (const u8 *)&ninja1_guardia_p16;
    }
}

/*--- tira_de: de qué tira es un dibujo (NINJA_DIB_*) de un tipo, qué cuadro y su banco ---*/
static const u8 *tira_de(u8 tipo, u8 dibujo, u8 *f, u8 *banco)
{
    u8 s;

    if (dibujo >= NINJA_DIB_KO)              { s = 6; *f = (u8)(dibujo - NINJA_DIB_KO); }
    else if (dibujo >= NINJA_DIB_LEVANTARSE) { s = 5; *f = (u8)(dibujo - NINJA_DIB_LEVANTARSE); }
    else if (dibujo >= NINJA_DIB_DERRIBADO)  { s = 4; *f = (u8)(dibujo - NINJA_DIB_DERRIBADO); }
    else if (dibujo >= NINJA_DIB_RECIBIR)    { s = 3; *f = (u8)(dibujo - NINJA_DIB_RECIBIR); }
    else if (dibujo >= NINJA_DIB_ATAQUE)     { s = 2; *f = (u8)(dibujo - NINJA_DIB_ATAQUE); }
    else if (dibujo >= NINJA_DIB_CAMINAR)    { s = 1; *f = (u8)(dibujo - NINJA_DIB_CAMINAR); }
    else                                     { s = 0; *f = dibujo; }
    return tira_tipo(tipo, s, banco);
}

static u8 muestra[NINJAS_MAX];
static u8 mirada[NINJAS_MAX];
static u8 turno;
static Subida subida;

void ninja_sprite_cargar(void)
{
    SpriteEnt *s;
    u8 k;

    /* Paletas de sprites 1..3: la ranura k es el tipo k, con su traje (rojo, amarillo, morado). */
    dmaCopyCGram((u8 *)&ninja1_pal, 128 + 16, 32);
    dmaCopyCGram((u8 *)&ninja2_pal, 128 + 32, 32);
    dmaCopyCGram((u8 *)&ninja3_pal, 128 + 48, 32);
    sprites_usar(ENTIDADES);                         /* Ahora la tabla tiene 8 entradas.    */
    for (k = 0; k < NINJAS_MAX; k++)
    {
        s = sprites_ent + ENT_NINJA + k;             /* El cuerpo: sin piezas hasta subir.  */
        s->n = 0;
        s->tile = HUECOS[k];
        s->banco = ninja1_p16_bancos[0];          /* (Se corrige al subir su primer cuadro.) */
        s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(k + 1));
        s = sprites_ent + ENT_SOMBRA_NINJA + k;      /* La sombra: la misma que la de Tarou. */
        s->piezas = (u16)&sombra_piezas;
        s->banco = sombra_banco;
        s->n = 0;                                    /* Apagada hasta que haya ninja.       */
        s->tile = TILE_SOMBRA;
        s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(PALETA_SOMBRA));
        sprites_y[ENT_SOMBRA_NINJA + k] = 0;         /* y = 0: detrás de todo.              */
        sprites_y[ENT_NINJA + k] = 0;
        muestra[k] = SIN_CUADRO;
        mirada[k] = 0;
    }
    turno = 0;
}

/*--- subir: UN ninja sube su cuadro entero, por turnos -----------------------------*/
static void subir(const Ninja *n)
{
    P16Cuadro cu;
    const Copia *copias;
    const Ninja *p;
    const u8 *tira;
    u8 i, k, cuantas, f, banco, dibujo;

    k = turno;
    for (i = 0; i < NINJAS_MAX; i++)
    {
        p = n + k;                                   /* 16 bytes por ranura: sin tcc__mul.  */
        dibujo = p->vivo ? ninja_dibujo(p) : SIN_CUADRO;
        if (p->vivo && dibujo != muestra[k])
        {
            tira = tira_de(p->tipo, dibujo, &f, &banco);
            p16_cuadro(tira, f, &cu);
            subida_empezar(&subida, cu.n, (u16)((u16)tira + cu.tiles),
                           VRAM_TILE(HUECOS[k]), banco);
            subida_tomar(&subida, 0xFFFF, &copias, &cuantas);    /* Todo, en este VBlank. */
            dma_lista_copias(copias, cuantas, banco);
            muestra[k] = dibujo;
            mirada[k] = 0xFF;                        /* Fuerza recalcular las piezas.       */
            turno = (u8)(k + 1 >= NINJAS_MAX ? 0 : k + 1);   /* El siguiente tiene la preferencia. */
            return;                                  /* Solo uno por frame.                 */
        }
        k++;
        if (k >= NINJAS_MAX)
        {
            k = 0;
        }
    }
}

void ninja_sprite_actualizar(const Ninja *n, u16 camara)
{
    P16Cuadro cu;
    SpriteEnt *s, *sombra;
    const Ninja *p;
    const u8 *tira;
    u8 k, f, banco, ancho, alto;

    subir(n);
    for (k = 0; k < NINJAS_MAX; k++)
    {
        p = n + k;
        s = sprites_ent + ENT_NINJA + k;
        sombra = sprites_ent + ENT_SOMBRA_NINJA + k;
        if (!p->vivo)                                /* Ranura libre: todo apagado.         */
        {
            s->n = 0;
            sombra->n = 0;
            muestra[k] = SIN_CUADRO;
            continue;
        }
        ancho = ANCHO_TIPO[p->tipo];                 /* La celda de su tipo.                */
        alto = ALTO_TIPO[p->tipo];
        s->x0 = (u16)(p->x - camara - (ancho >> 1)); /* x de pantalla del borde izquierdo.  */
        s->y0 = (u16)(p->y - alto);                  /* Los pies, en la y del carril.       */
        if (ninja_parpadea(p))
        {
            s->y0 = 240;                             /* KO tumbado: parpadea antes de irse. */
        }
        s->borde = sprites_borde(s->x0, s->y0, ancho, alto);
        s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(k + 1) |
                        (p->mira_izq ? SPRITE_HFLIP : 0));
        if (muestra[k] != SIN_CUADRO)                /* Hay un cuadro entero que enseñar.   */
        {
            if (mirada[k] != p->mira_izq)            /* Cambió el cuadro o la mirada.       */
            {
                tira = tira_de(p->tipo, muestra[k], &f, &banco);
                p16_cuadro(tira, f, &cu);
                s->piezas = (u16)((u16)tira + (p->mira_izq ? cu.espejo : cu.piezas));
                s->banco = banco;                    /* La tabla de piezas está en el banco de su tira. */
                s->n = cu.n;
                mirada[k] = p->mira_izq;
            }
        }
        sprites_y[ENT_NINJA + k] = p->y;             /* Los pies: la clave del orden.       */

        sombra->x0 = (u16)(p->x - camara - 16);      /* La sombra, bajo el centro.          */
        sombra->y0 = (u16)(p->y - SOMBRA_ALTO);
        sombra->borde = sprites_borde(sombra->x0, sombra->y0, 32, 16);
        sombra->n = 2;
    }
    sprites_ordenar();                               /* Por la y de los pies, otra vez.     */
}
