/*---------------------------------------------------------------------------------
    ninja_sprite.c — Los ninjas y sus sombras en pantalla (ver ninja_sprite.h).

    PROPÓSITO
      Cada frame, para cada ranura: (1) si no está viva, sus dos entidades se apagan (n = 0)
      y se olvida el cuadro que tenía en la VRAM; (2) si está viva y su cuadro no es el que
      hay en su hueco, se sube entero (turno: solo UN ninja por frame, en rotación);
      (3) cada ranura dibuja el tipo que la ocupa (2026-11-05: normal, mediano, grande,
      rápido, kendoka y jefe, con su arte, su celda y su paleta, que se carga al cambiar de
      tipo): ninja_dibujo() dice qué dibujo toca y tira_de() lo traduce a la tira de ese
      tipo (si no la tiene, otra parecida); un cuadro de más de TOPE_NINJA bytes (el jefe)
      se sube en dos VBlanks; (4) se coloca el cuerpo (x0, y0 en pantalla), la mirada (espejo y tabla de piezas
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

extern char ninja1_guardia_p16, ninja1_caminar_p16, ninja1_ataque_1_p16, ninja1_recibir_p16, ninja1_derribado_p16, ninja1_levantarse_p16, ninja1_ko_p16, ninja1_agarrado_p16, ninja1_bloqueo_p16, ninja1_ataque_2_p16, ninja1_agarre_p16, ninja1_lanzamiento_p16, ninja1_patada_p16;
extern char ninja2_guardia_p16, ninja2_caminar_p16, ninja2_ataque_1_p16, ninja2_recibir_p16, ninja2_derribado_p16, ninja2_levantarse_p16, ninja2_ko_p16;
extern char ninja3_guardia_p16, ninja3_caminar_p16, ninja3_ataque_1_p16, ninja3_recibir_p16, ninja3_derribado_p16, ninja3_levantarse_p16, ninja3_ko_p16;
extern char kendoka_guardia_p16, kendoka_caminar_p16, kendoka_ataque_p16, kendoka_bloqueo_p16, kendoka_agarre_p16, kendoka_lanzamiento_p16, kendoka_recibir_p16, kendoka_derribado_p16, kendoka_levantarse_p16, kendoka_ko_p16;
extern char jefe_guardia_p16, jefe_caminar_p16, jefe_carga_p16, jefe_tajo_p16, jefe_convocar_p16, jefe_recibir_p16, jefe_derribado_p16, jefe_levantarse_p16, jefe_ko_p16;
extern char ninja1_pal, ninja2_pal, ninja3_pal, ninjar_pal, kendoka_pal, jefe_pal, sombra_piezas;
extern u8 ninja1_p16_bancos[], ninja2_p16_bancos[], ninja3_p16_bancos[];   /* Banco de cada tira. */
extern u8 kendoka_p16_bancos[], jefe_p16_bancos[];
extern u8 sombra_banco;

static const u8 *tira_ninja1(u8 i)
{
    switch (i)
    {
        case 1: return (const u8 *)&ninja1_caminar_p16;
        case 2: return (const u8 *)&ninja1_ataque_1_p16;
        case 3: return (const u8 *)&ninja1_recibir_p16;
        case 4: return (const u8 *)&ninja1_derribado_p16;
        case 5: return (const u8 *)&ninja1_levantarse_p16;
        case 6: return (const u8 *)&ninja1_ko_p16;
        case 7: return (const u8 *)&ninja1_agarrado_p16;
        case 8: return (const u8 *)&ninja1_bloqueo_p16;
        case 9: return (const u8 *)&ninja1_ataque_2_p16;
        case 10: return (const u8 *)&ninja1_agarre_p16;
        case 11: return (const u8 *)&ninja1_lanzamiento_p16;
        case 12: return (const u8 *)&ninja1_patada_p16;
        default: return (const u8 *)&ninja1_guardia_p16;
    }
}

static const u8 *tira_ninja2(u8 i)
{
    switch (i)
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

static const u8 *tira_ninja3(u8 i)
{
    switch (i)
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

static const u8 *tira_kendoka(u8 i)
{
    switch (i)
    {
        case 1: return (const u8 *)&kendoka_caminar_p16;
        case 2: return (const u8 *)&kendoka_ataque_p16;
        case 3: return (const u8 *)&kendoka_bloqueo_p16;
        case 4: return (const u8 *)&kendoka_agarre_p16;
        case 5: return (const u8 *)&kendoka_lanzamiento_p16;
        case 6: return (const u8 *)&kendoka_recibir_p16;
        case 7: return (const u8 *)&kendoka_derribado_p16;
        case 8: return (const u8 *)&kendoka_levantarse_p16;
        case 9: return (const u8 *)&kendoka_ko_p16;
        default: return (const u8 *)&kendoka_guardia_p16;
    }
}

static const u8 *tira_jefe(u8 i)
{
    switch (i)
    {
        case 1: return (const u8 *)&jefe_caminar_p16;
        case 2: return (const u8 *)&jefe_carga_p16;
        case 3: return (const u8 *)&jefe_tajo_p16;
        case 4: return (const u8 *)&jefe_convocar_p16;
        case 5: return (const u8 *)&jefe_recibir_p16;
        case 6: return (const u8 *)&jefe_derribado_p16;
        case 7: return (const u8 *)&jefe_levantarse_p16;
        case 8: return (const u8 *)&jefe_ko_p16;
        default: return (const u8 *)&jefe_guardia_p16;
    }
}

/* Cuadros de cada tira, por personaje (de sus *_anim.h, generados). */
static const u8 CUADROS_NINJA1[13] = {8, 6, 3, 6, 7, 5, 7, 2, 2, 8, 2, 6, 7};
static const u8 CUADROS_NINJA2[13] = {8, 6, 3, 6, 7, 5, 7, 1, 1, 1, 1, 1, 1};
static const u8 CUADROS_NINJA3[13] = {8, 6, 3, 6, 7, 5, 7, 1, 1, 1, 1, 1, 1};
static const u8 CUADROS_KENDOKA[13] = {6, 6, 6, 2, 2, 5, 5, 6, 5, 6, 1, 1, 1};
static const u8 CUADROS_JEFE[13] = {6, 6, 6, 6, 4, 5, 6, 5, 6, 1, 1, 1, 1};

#define ENT_NINJA 2                  /* Entradas 2..4: los cuerpos.                       */
#define ENT_SOMBRA_NINJA 5           /* Entradas 5..7: las sombras.                       */
#define ENTIDADES 8                  /* Tarou (0), su sombra (1) y las seis de los ninjas. */
#define TILE_SOMBRA 480              /* Par 15: la sombra (la sube protagonista_sprite.c). */
#define PALETA_SOMBRA 4
#define PRIORIDAD 2                  /* Por delante de BG1 y BG2, salvo sus tiles con prioridad
                                        (el primer plano de la aldea los tapa). */
#define SOMBRA_ALTO 10
#define SIN_CUADRO 0xFF
#define SIN_TIPO 0xFF
#define TOPE_NINJA 1024              /* Bytes por VBlank: un cuadro más grande (el jefe, o
                                        uno de 24 piezas) se sube en dos VBlanks.          */
#define VRAM_TILE(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))   /* 16 palabras por tile.  */

static const u16 HUECOS[NINJAS_MAX] = {192, 256, 352};   /* Primer tile de cada hueco:   */
                                                         /* 16, 24 y 24 piezas (ver .h). */
static const u16 BLANCO[16] = {0, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
                               0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF};

/* Cada tipo (normal, mediano, grande, rápido, kendoka, jefe): su celda. */
static const u8 ANCHO_TIPO[NINJA_TIPOS] = {NINJA_ANCHO, 128, 128, NINJA_ANCHO, 128, 144};
static const u8 ALTO_TIPO[NINJA_TIPOS] = {NINJA_ALTO, 112, 112, NINJA_ALTO, 112, 112};

/*--- Los dibujos: de qué animación es cada NINJA_DIB_* y qué tira la dibuja por tipo ---
    Animaciones: 0 guardia, 1 caminar, 2 puño, 3 recibir, 4 derribado, 5 levantarse, 6 KO,
    7 agarrado, 8 bloqueo, 9 shuriken, 10 agarre, 11 lanzar, 12 patada, 13 carga,
    14 convocar, 15 pesado. Los tipos sin esa tira usan otra parecida. */
static const u8 BASE_ANIM[16] = {NINJA_DIB_GUARDIA, NINJA_DIB_CAMINAR, NINJA_DIB_ATAQUE,
                                 NINJA_DIB_RECIBIR, NINJA_DIB_DERRIBADO, NINJA_DIB_LEVANTARSE,
                                 NINJA_DIB_KO, NINJA_DIB_AGARRADO, NINJA_DIB_BLOQUEO,
                                 NINJA_DIB_SHURIKEN, NINJA_DIB_AGARRE, NINJA_DIB_LANZAR,
                                 NINJA_DIB_PATADA, NINJA_DIB_CARGA, NINJA_DIB_CONVOCAR,
                                 NINJA_DIB_PESADO};
/* 1 = la animación hace bucle (si el tipo tiene menos cuadros, vuelve a empezar). */
static const u8 BUCLE_ANIM[16] = {1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};
/* La tira de cada animación, por tipo (16 por fila: tabla[t][a] es t·16 + a, sin tcc__mul). */
static const u8 TIRA_ANIM[NINJA_TIPOS][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 2, 2, 2},      /* normal                    */
    {0, 1, 2, 3, 4, 5, 6, 3, 0, 2, 2, 2, 2, 2, 2, 2},         /* mediano                   */
    {0, 1, 2, 3, 4, 5, 6, 3, 0, 2, 2, 2, 2, 2, 2, 2},         /* grande                    */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 2, 2, 2},      /* rápido (el del normal)    */
    {0, 1, 2, 6, 7, 8, 9, 6, 3, 2, 4, 5, 2, 1, 0, 2},         /* kendoka                   */
    {0, 1, 3, 5, 6, 7, 8, 5, 0, 3, 0, 0, 3, 2, 4, 3},         /* jefe                      */
};

/*--- tira_tipo: la tira `i` del arte de un tipo, su banco y sus cuadros ----------------*/
static const u8 *tira_tipo(u8 tipo, u8 i, u8 *banco, u8 *cuadros)
{
    switch (tipo)
    {
        case NINJA_MEDIANO:
            *banco = ninja2_p16_bancos[i];
            *cuadros = CUADROS_NINJA2[i];
            return tira_ninja2(i);
        case NINJA_GRANDE:
            *banco = ninja3_p16_bancos[i];
            *cuadros = CUADROS_NINJA3[i];
            return tira_ninja3(i);
        case NINJA_KENDOKA:
            *banco = kendoka_p16_bancos[i];
            *cuadros = CUADROS_KENDOKA[i];
            return tira_kendoka(i);
        case NINJA_JEFE:
            *banco = jefe_p16_bancos[i];
            *cuadros = CUADROS_JEFE[i];
            return tira_jefe(i);
        default:                                     /* El normal y el rápido.              */
            *banco = ninja1_p16_bancos[i];
            *cuadros = CUADROS_NINJA1[i];
            return tira_ninja1(i);
    }
}

/*--- tira_de: la tira y el cuadro de un dibujo (NINJA_DIB_*) de un tipo -----------------*/
static const u8 *tira_de(u8 tipo, u8 dibujo, u8 *f, u8 *banco)
{
    const u8 *tira;
    u8 a, cuadros;

    a = 15;
    while (a > 0 && dibujo < BASE_ANIM[a])           /* La animación: la base más alta que no */
    {                                                /* pasa del dibujo.                    */
        a--;
    }
    *f = (u8)(dibujo - BASE_ANIM[a]);
    tira = tira_tipo(tipo, TIRA_ANIM[tipo][a], banco, &cuadros);
    if (*f >= cuadros)                               /* Ese tipo tiene menos cuadros:       */
    {
        *f = (u8)(BUCLE_ANIM[a] ? *f - cuadros : cuadros - 1);   /* vuelve a empezar o se queda. */
        if (*f >= cuadros)
        {
            *f = 0;
        }
    }
    return tira;
}

/*--- paleta_tipo: la paleta del arte de cada tipo -----------------------------------*/
static const u8 *paleta_tipo(u8 tipo)
{
    switch (tipo)
    {
        case NINJA_MEDIANO: return (const u8 *)&ninja2_pal;
        case NINJA_GRANDE:  return (const u8 *)&ninja3_pal;
        case NINJA_RAPIDO:  return (const u8 *)&ninjar_pal;
        case NINJA_KENDOKA: return (const u8 *)&kendoka_pal;
        case NINJA_JEFE:    return (const u8 *)&jefe_pal;
        default:            return (const u8 *)&ninja1_pal;
    }
}

static u8 muestra[NINJAS_MAX];
static u8 mirada[NINJAS_MAX];
static u8 blanca[NINJAS_MAX];        /* 1 si la paleta de la ranura está ahora en blanco. */
static u8 pal_tipo[NINJAS_MAX];      /* El tipo cuya paleta tiene la ranura (SIN_TIPO: ninguno). */
static u8 tipo_vram[NINJAS_MAX];     /* El tipo del cuadro que hay en su hueco.          */
static u8 turno;
static u8 subiendo;                  /* Ranura con una subida a medias (SIN_TIPO: ninguna). */
static u8 dib_subiendo;              /* El dibujo que se está subiendo.                   */
static Subida subida;

void ninja_sprite_cargar(void)
{
    SpriteEnt *s;
    u8 k;

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
        blanca[k] = 0;
        pal_tipo[k] = SIN_TIPO;                      /* La pone ninja_sprite_vblank.        */
        tipo_vram[k] = SIN_TIPO;
        dmaCopyCGram((u8 *)paleta_tipo(k), (u16)(128 + 16 + ((u16)k << 4)), 32);   /* Al entrar. */
        pal_tipo[k] = k;
    }
    turno = 0;
    subiendo = SIN_TIPO;
}

const u8 *ninja_sprite_paleta(const Ninja *n, u8 k)
{
    return paleta_tipo(n[k].tipo);
}

void ninja_sprite_vblank(const Ninja *n)
{
    u8 k, quiere;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        quiere = (u8)(n->vivo && n->destello != 0);  /* ¿Toca blanco este frame?            */
        if (quiere != blanca[k] || (n->vivo && !quiere && pal_tipo[k] != n->tipo))
        {
            dmaCopyCGram(quiere ? (u8 *)BLANCO : (u8 *)paleta_tipo(n->tipo),
                         (u16)(128 + 16 + ((u16)k << 4)), 32);   /* Paletas 1..3.          */
            blanca[k] = quiere;
            pal_tipo[k] = (u8)(quiere ? SIN_TIPO : n->tipo);
        }
        n++;
    }
}

void ninja_sprite_paletas_repuestas(void)
{
    u8 k;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        blanca[k] = 0;
        pal_tipo[k] = SIN_TIPO;                      /* La siguiente vblank pone la de su tipo. */
    }
}

/*--- seguir: toma lo que cabe este VBlank de la subida en marcha -----------------------*/
static void seguir(void)
{
    const Copia *copias;
    u8 cuantas;

    subida_tomar(&subida, TOPE_NINJA, &copias, &cuantas);
    dma_lista_copias(copias, cuantas, subida.banco);
    if (subida_terminada(&subida))
    {
        muestra[subiendo] = dib_subiendo;            /* Ya está entero: la OAM pasa a él.   */
        mirada[subiendo] = 0xFF;                     /* Fuerza recalcular las piezas.       */
        subiendo = SIN_TIPO;
    }
}

/*--- subir: UN ninja sube su cuadro, por turnos (en uno o dos VBlanks) ---------------*/
static void subir(const Ninja *n)
{
    P16Cuadro cu;
    const Ninja *p;
    const u8 *tira;
    u8 i, k, f, banco, dibujo;

    if (subiendo != SIN_TIPO)
    {
        seguir();                                    /* Primero, acabar la que va a medias. */
        return;
    }
    k = turno;
    for (i = 0; i < NINJAS_MAX; i++)
    {
        p = n + k;                                   /* 32 bytes por ranura: sin tcc__mul.  */
        dibujo = p->vivo ? ninja_dibujo(p) : SIN_CUADRO;
        if (p->vivo && (dibujo != muestra[k] || tipo_vram[k] != p->tipo))
        {
            tira = tira_de(p->tipo, dibujo, &f, &banco);
            p16_cuadro(tira, f, &cu);
            subida_empezar(&subida, cu.n, (u16)((u16)tira + cu.tiles),
                           VRAM_TILE(HUECOS[k]), banco);
            subiendo = k;
            dib_subiendo = dibujo;
            tipo_vram[k] = p->tipo;
            turno = (u8)(k + 1 >= NINJAS_MAX ? 0 : k + 1);   /* El siguiente tiene la preferencia. */
            seguir();
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
            s->y0 = 240;                             /* Parpadea (KO tumbado o recién levantado). */
        }
        s->borde = sprites_borde(s->x0, s->y0, ancho, alto);
        s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(k + 1) |
                        (p->mira_izq ? SPRITE_HFLIP : 0));
        if (muestra[k] != SIN_CUADRO && tipo_vram[k] == p->tipo)   /* Hay un cuadro entero. */
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
        else
        {
            s->n = 0;                                /* Aún sin su primer cuadro.           */
        }
        sprites_y[ENT_NINJA + k] = p->y;             /* Los pies: la clave del orden.       */

        sombra->x0 = (u16)(p->x - camara - 16);      /* La sombra, bajo el centro.          */
        sombra->y0 = (u16)(p->y - SOMBRA_ALTO);
        sombra->borde = sprites_borde(sombra->x0, sombra->y0, 32, 16);
        sombra->n = 2;
    }
    sprites_ordenar();                               /* Por la y de los pies, otra vez.     */
}
