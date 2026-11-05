/*---------------------------------------------------------------------------------
    protagonista_sprite.c — Una protagonista y su sombra en pantalla (ver .h).

    PROPÓSITO
      Cada frame: (1) la animación (personaje_animar con la tabla del personaje). (2) La
      subida: si el cuadro que toca no es el que está en la VRAM y no hay otra subida en
      marcha, planifica sus piezas hacia el búfer OCULTO; cada frame toma hasta `tope`
      bytes y los apunta en la lista de DMA; al terminar, los búferes se intercambian y
      la entidad enseña el nuevo. (3) Las entidades: la posición cambia cada frame; las
      piezas, solo con la mirada o el cuadro. Es el mismo método que midió la prueba de
      carga (pantallas/prueba_carga.c §9.2).

    SE CONECTA CON
      Incluye:  <snes.h>, juego/protagonista_sprite.h, sistema/comun.h (VRAM_SPRITES),
                juego/animaciones.h (enum Estado: las tiras de Tarou), juego/sprites.h,
                juego/p16.h, juego/dma_lista.h.
      Lo usan:  pantallas/pantalla_fase1.c, pantallas/pantalla_selector.c.

    HARDWARE QUE TOCA
      Ver protagonista_sprite.h. La matemática de color queda así: TS = BG1+BG2, CGWSEL =
      segundo operando la pantalla secundaria, CGADSUB = media suma solo en los sprites de
      las paletas 4..7 (la sombra); los cuerpos opacos usan las paletas 0..3.

    VARIABLES: los campos de cada ProtaSprite (protagonista_sprite.h).
    → glosario: Doble búfer, Pieza de 16×16, OAM
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/protagonista_sprite.h"
#include "sistema/comun.h"
#include "juego/animaciones.h"
#include "juego/sprites.h"
#include "juego/p16.h"
#include "juego/dma_lista.h"

extern char tarou_reposo_p16, tarou_caminar_p16, tarou_saltar_p16, tarou_caer_p16, tarou_atacar_p16, tarou_correr_p16;
extern char tarou_guardia_bu_p16, tarou_golpe_1_p16, tarou_golpe_2_p16, tarou_golpe_3_p16;
extern char tarou_recibir_p16, tarou_ko_p16;
extern char tarou_golpe_4_p16, tarou_levantarse_p16, tarou_derribado_p16, tarou_victoria_p16;
extern char lucinda_entrada_p16, lucinda_ataque_p16, rex_entrada_p16, rex_ataque_p16;
extern char fenrir_entrada_p16, fenrir_ataque_p16, lucinda_pal, rex_pal, fenrir_pal;
extern u8 lucinda_p16_bancos[], rex_p16_bancos[], fenrir_p16_bancos[];
extern char barbara_guardia_p16, barbara_caminar_p16, barbara_saltar_p16, barbara_caer_p16;
extern char barbara_golpe_1_p16, barbara_golpe_2_p16, barbara_golpe_3_p16;
extern char barbara_recibir_p16, barbara_ko_p16;
extern char barbara_correr_p16, barbara_aereo_p16, barbara_agarre_p16, barbara_golpe_agarre_p16, barbara_lanzamiento_p16, barbara_especial_p16, barbara_lanzar_arma_p16, barbara_derribada_p16, barbara_levantarse_p16, barbara_agarrada_p16, barbara_victoria_p16;
extern char tarou_pal, barbara_pal, ninja_pal, sombra_til, sombra_piezas;
extern u8 tarou_p16_bancos[];        /* Banco de cada tira .p16 de Tarou (enum Estado).   */
extern u8 barbara_p16_bancos[];      /* Banco de cada tira de Bárbara (BarbaraTira).      */
extern u8 sombra_banco;

#define TILE_SOMBRA 480              /* Par 15: la sombra.                                */
#define PALETA_SOMBRA 4              /* 4..7: las que entran en la matemática de color.   */
#define PRIORIDAD 2                 /* Por delante de BG1 y BG2, salvo sus tiles con prioridad
                                        (el primer plano de la aldea los tapa). */
#define SOMBRA_ALTO 10               /* La sombra, 10 px por encima de los pies.          */
#define VRAM_TILE(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))   /* 16 palabras por tile.  */

/*--- tira_de: dónde empieza en la ROM una tira .p16 y en qué banco --------------------*/
static const u8 *tira_de(u8 personaje, u8 tira)
{
    if (personaje == PERSONAJE_LUCINDA)              /* Los apoyos: entrada o ataque.       */
    {
        return tira ? (const u8 *)&lucinda_ataque_p16 : (const u8 *)&lucinda_entrada_p16;
    }
    if (personaje == PERSONAJE_REX)
    {
        return tira ? (const u8 *)&rex_ataque_p16 : (const u8 *)&rex_entrada_p16;
    }
    if (personaje == PERSONAJE_FENRIR)
    {
        return tira ? (const u8 *)&fenrir_ataque_p16 : (const u8 *)&fenrir_entrada_p16;
    }
    if (personaje == PERSONAJE_BARBARA)
    {
        switch (tira)
        {
            case BARBARA_TIRA_CAMINAR: return (const u8 *)&barbara_caminar_p16;
            case BARBARA_TIRA_SALTAR:  return (const u8 *)&barbara_saltar_p16;
            case BARBARA_TIRA_CAER:    return (const u8 *)&barbara_caer_p16;
            case BARBARA_TIRA_GOLPE_1: return (const u8 *)&barbara_golpe_1_p16;
            case BARBARA_TIRA_GOLPE_2: return (const u8 *)&barbara_golpe_2_p16;
            case BARBARA_TIRA_GOLPE_3: return (const u8 *)&barbara_golpe_3_p16;
            case BARBARA_TIRA_RECIBIR: return (const u8 *)&barbara_recibir_p16;
            case BARBARA_TIRA_KO:      return (const u8 *)&barbara_ko_p16;
            case BARBARA_TIRA_CORRER: return (const u8 *)&barbara_correr_p16;
            case BARBARA_TIRA_AEREO: return (const u8 *)&barbara_aereo_p16;
            case BARBARA_TIRA_AGARRE: return (const u8 *)&barbara_agarre_p16;
            case BARBARA_TIRA_GOLPE_AGARRE: return (const u8 *)&barbara_golpe_agarre_p16;
            case BARBARA_TIRA_LANZAMIENTO: return (const u8 *)&barbara_lanzamiento_p16;
            case BARBARA_TIRA_ESPECIAL: return (const u8 *)&barbara_especial_p16;
            case BARBARA_TIRA_LANZAR_ARMA: return (const u8 *)&barbara_lanzar_arma_p16;
            case BARBARA_TIRA_DERRIBADA: return (const u8 *)&barbara_derribada_p16;
            case BARBARA_TIRA_LEVANTARSE: return (const u8 *)&barbara_levantarse_p16;
            case BARBARA_TIRA_AGARRADA: return (const u8 *)&barbara_agarrada_p16;
            case BARBARA_TIRA_VICTORIA: return (const u8 *)&barbara_victoria_p16;
            default:                   return (const u8 *)&barbara_guardia_p16;
        }
    }
    switch (tira)                                    /* Tarou: su enum Estado o TAROU_TIRA_*. */
    {
        case TAROU_TIRA_GUARDIA_BU: return (const u8 *)&tarou_guardia_bu_p16;
        case TAROU_TIRA_GOLPE_1: return (const u8 *)&tarou_golpe_1_p16;
        case TAROU_TIRA_GOLPE_2: return (const u8 *)&tarou_golpe_2_p16;
        case TAROU_TIRA_GOLPE_3: return (const u8 *)&tarou_golpe_3_p16;
        case TAROU_TIRA_RECIBIR: return (const u8 *)&tarou_recibir_p16;
        case TAROU_TIRA_KO:      return (const u8 *)&tarou_ko_p16;
        case TAROU_TIRA_GOLPE_4: return (const u8 *)&tarou_golpe_4_p16;
        case TAROU_TIRA_LEVANTARSE: return (const u8 *)&tarou_levantarse_p16;
        case TAROU_TIRA_DERRIBADO: return (const u8 *)&tarou_derribado_p16;
        case TAROU_TIRA_VICTORIA: return (const u8 *)&tarou_victoria_p16;
        case ESTADO_CAMINANDO: return (const u8 *)&tarou_caminar_p16;
        case ESTADO_CORRIENDO: return (const u8 *)&tarou_correr_p16;
        case ESTADO_SALTANDO:  return (const u8 *)&tarou_saltar_p16;
        case ESTADO_CAYENDO:   return (const u8 *)&tarou_caer_p16;
        case ESTADO_ATACANDO:  return (const u8 *)&tarou_atacar_p16;
        default:               return (const u8 *)&tarou_reposo_p16;
    }
}

static u8 banco_de(u8 personaje, u8 tira)
{
    switch (personaje)
    {
        case PERSONAJE_BARBARA: return barbara_p16_bancos[tira];
        case PERSONAJE_LUCINDA: return lucinda_p16_bancos[tira];
        case PERSONAJE_REX:     return rex_p16_bancos[tira];
        case PERSONAJE_FENRIR:  return fenrir_p16_bancos[tira];
        default:                return tarou_p16_bancos[tira];
    }
}

const u8 *protagonista_paleta(u8 personaje)
{
    switch (personaje)
    {
        case PERSONAJE_BARBARA: return (const u8 *)&barbara_pal;
        case PERSONAJE_LUCINDA: return (const u8 *)&lucinda_pal;
        case PERSONAJE_REX:     return (const u8 *)&rex_pal;
        case PERSONAJE_FENRIR:  return (const u8 *)&fenrir_pal;
        default:                return (const u8 *)&tarou_pal;
    }
}

void protagonista_sprite_cambiar(ProtaSprite *p, u8 personaje)
{
    p->personaje = personaje;                        /* Mismas entidades, búferes y tope;   */
    p->ancho = personaje_ancho(personaje);           /* otra celda y otras tiras.           */
    p->alto = personaje_alto(personaje);
    p->anims = personaje_anims(personaje);
    p->a.anim = ANIM_GUARDIA;
    p->a.cuadro = 0;
    p->a.reloj = 0;
    p->m_anim = SIN_CUADRO;                          /* Lo de la VRAM ya no vale: se oculta */
    p->m_cuadro = SIN_CUADRO;                        /* hasta que suba su primer cuadro.    */
    p->s_anim = SIN_CUADRO;
    p->s_cuadro = SIN_CUADRO;
    sprites_ent[p->ent].n = 0;
}

void protagonista_escena_cargar(void)
{
    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE16_L32);    /* Piezas pequeñas de 16x16.           */
    sprites_iniciar();                               /* Las 128 entradas, escondidas.       */
    dmaCopyCGram((u8 *)&ninja_pal + 96, 128 + 64, 32);   /* Paleta 4: la sombra.            */
    dmaCopyVram((u8 *)&sombra_til, VRAM_TILE(TILE_SOMBRA), 128);            /* Fila de arriba. */
    dmaCopyVram((u8 *)&sombra_til + 128, VRAM_TILE(TILE_SOMBRA + 16), 128); /* Y la de abajo.  */

    /* Sombra: media suma con la pantalla secundaria (BG1 + BG2), solo sprites 4..7. */
    REG_TS = 0x03;                                   /* BG1 y BG2 también en la secundaria. */
    REG_CGWSEL = 0x02;                               /* Operando: la pantalla secundaria.   */
    REG_CGADSUB = 0x50;                              /* Sumar a la mitad (0x40) los sprites (0x10). */
}

void protagonista_sprite_cargar(ProtaSprite *p, u8 personaje, u8 ent, u16 tile_a,
                                u16 tile_b, u8 paleta, u16 tope)
{
    SpriteEnt *s;

    p->personaje = personaje;
    p->ent = ent;
    p->paleta = paleta;
    p->tile_a = tile_a;
    p->tile_b = tile_b;
    p->tope = tope;
    p->ancho = personaje_ancho(personaje);
    p->alto = personaje_alto(personaje);
    p->anims = personaje_anims(personaje);
    p->a.anim = ANIM_GUARDIA;
    p->a.cuadro = 0;
    p->a.reloj = 0;
    p->m_anim = SIN_CUADRO;                          /* Nada en la VRAM todavía.            */
    p->m_cuadro = SIN_CUADRO;
    p->s_anim = SIN_CUADRO;
    p->s_cuadro = SIN_CUADRO;
    p->bufer = 1;                                    /* El primer cuadro va al búfer A.     */

    dmaCopyCGram((u8 *)protagonista_paleta(personaje), (u16)(128 + ((u16)paleta << 4)), 32);

    s = sprites_ent + ent;
    s->n = 0;                                        /* Sin piezas hasta que suba su cuadro. */
    s->tile = tile_a;
    s->banco = banco_de(personaje, p->anims[ANIM_GUARDIA].tira);
    s = sprites_ent + ent + 1;                       /* La sombra no cambia de forma.       */
    s->piezas = (u16)&sombra_piezas;
    s->banco = sombra_banco;
    s->n = 2;
    s->tile = TILE_SOMBRA;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(PALETA_SOMBRA));
    sprites_y[ent + 1] = 0;                          /* y = 0: detrás de todo.              */
}

/*--- subir: empieza la subida del cuadro que toca y sigue la que está en marcha -------*/
static void subir(ProtaSprite *p)
{
    P16Cuadro cu;
    const Copia *copias;
    const u8 *tira;
    u8 n, t;

    if (p->s_cuadro == SIN_CUADRO && (p->a.cuadro != p->m_cuadro || p->a.anim != p->m_anim))
    {
        t = p->anims[p->a.anim].tira;
        tira = tira_de(p->personaje, t);
        p16_cuadro(tira, p->a.cuadro, &cu);
        subida_empezar(&p->subida, cu.n, (u16)((u16)tira + cu.tiles),
                       VRAM_TILE(p->bufer ? p->tile_a : p->tile_b),   /* El búfer oculto.   */
                       banco_de(p->personaje, t));
        p->s_cuadro = p->a.cuadro;
        p->s_anim = p->a.anim;
    }
    if (p->s_cuadro != SIN_CUADRO)
    {
        subida_tomar(&p->subida, p->tope, &copias, &n);
        dma_lista_copias(copias, n, p->subida.banco);    /* En ensamblador: dma_copias.asm. */
        if (subida_terminada(&p->subida))
        {
            p->m_cuadro = p->s_cuadro;               /* Tras este VBlank está entero.       */
            p->m_anim = p->s_anim;
            p->bufer ^= 1;                           /* El búfer nuevo pasa a ser el visible. */
            p->s_cuadro = SIN_CUADRO;
        }
    }
}

void protagonista_sprite_actualizar(ProtaSprite *p, const Cuerpo *c, u16 camara)
{
    SpriteEnt *s;
    P16Cuadro cu;
    const u8 *tira;
    u8 t;

    personaje_animar(&p->a, p->anims, personaje_anim_de(c));
    subir(p);

    /*--- El cuerpo: x0, y0 son el borde izquierdo y el de arriba de la celda ---------*/
    s = sprites_ent + p->ent;
    s->x0 = (u16)(c->x - camara - (p->ancho >> 1));  /* x de pantalla, centrado (puede ser < 0). */
    s->y0 = (u16)(c->y - p->alto + 2 - mundo_altura(c));   /* 2 px hundido, como en el nivel. */
    s->borde = sprites_borde(s->x0, s->y0, p->ancho, p->alto);
    if (c->inv != 0 && (c->inv & 2))                 /* Invulnerable: parpadea (cada 2 cuadros fuera). */
    {
        s->y0 = 240;                                 /* y >= 224: sus piezas no se dibujan. */
        s->borde = 1;
    }
    s->tile = p->bufer ? p->tile_b : p->tile_a;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(p->paleta) |
                    (c->mira_izq ? SPRITE_HFLIP : 0));
    if (p->m_anim != SIN_CUADRO)                     /* Ya hay un cuadro entero que enseñar. */
    {
        t = p->anims[p->m_anim].tira;
        tira = tira_de(p->personaje, t);
        p16_cuadro(tira, p->m_cuadro, &cu);
        s->piezas = (u16)((u16)tira + (c->mira_izq ? cu.espejo : cu.piezas));
        s->banco = banco_de(p->personaje, t);
        s->n = cu.n;
    }
    sprites_y[p->ent] = c->y;                        /* Los pies: la clave del orden.       */

    /*--- La sombra: en el suelo (no sube con el salto), bajo el centro -----------------*/
    s = sprites_ent + p->ent + 1;
    s->x0 = (u16)(c->x - camara - 16);
    s->y0 = (u16)(c->y - SOMBRA_ALTO);
    s->borde = sprites_borde(s->x0, s->y0, 32, 16);
}

u8 protagonista_sprite_al_dia(const ProtaSprite *p)
{
    return (u8)(p->m_anim == p->a.anim && p->m_cuadro == p->a.cuadro);
}

void protagonista_sprite_ocultar(void)
{
    sprites_iniciar();                               /* Las 128 entradas, escondidas.       */
    REG_TS = 0;                                      /* Sin matemática de color.            */
    REG_CGWSEL = 0;
    REG_CGADSUB = 0;
}
