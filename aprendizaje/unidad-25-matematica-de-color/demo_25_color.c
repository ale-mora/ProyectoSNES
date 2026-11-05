/*---------------------------------------------------------------------------------
    demo_25_color.c — La matemática de color de la PPU, imitada en el PC con los
    colores reales del juego.

    PROPÓSITO
      La PPU no mezcla «con transparencia»: por cada píxel, si la capa que está
      delante (la pantalla principal) tiene encendida la matemática de color, le
      SUMA o le RESTA otro color (el de la pantalla secundaria o el color fijo) y, si
      se pide, divide el resultado entre 2. Todo en 5 bits por canal (0..31), con
      tope en 0 y en 31. Esta demo hace esa cuenta canal a canal para los cinco
      casos del juego:
        A. la sombra (paleta de sprites 4, negra) sobre la calle: media suma;
        B. el humo (paleta 7) sobre una montaña y sobre el cielo vacío;
        C. la media luz de la pausa en un fondo: restar el color fijo negro y dividir;
        D. la misma media luz en un sprite de paleta 0..3, que NO entra en la
           matemática: hay que oscurecer su paleta a mano (v >> 1 con la máscara
           0x3DEF), y qué pasa si se olvida la máscara;
        E. el destello (ejercicio): sumar un color fijo gris, con tope en 31.

    SE CONECTA CON
      Imita: src/juego/protagonista_sprite.c (protagonista_escena_cargar: TS, CGWSEL,
             CGADSUB de la escena) y src/pantallas/pantalla_fase1.c (pantalla_fase1_vblank:
             la media luz; oscurecer()).
      Los colores salen de scripts/preparar_aldea.py (PALETA_ARQ, PALETA_LEJOS,
      PALETA_OBJETOS) y de res/prueba_carga/ninja_pal.bin (la sombra, color 0).

    HARDWARE QUE TOCA
      ninguno (PC).

    VARIABLES: ninguna global.
    Compilar (desde esta carpeta):
      gcc -std=c99 -Wall -Wextra -o demo_25.exe demo_25_color.c
    → glosario: Matemática de color, CGRAM, Paleta
---------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

/* Un color de la SNES: 15 bits, 0bbbbbgg gggrrrrr (BGR555). */
static u8 rojo(u16 c) { return (u8)(c & 31); }
static u8 verde(u16 c) { return (u8)((c >> 5) & 31); }
static u8 azul(u16 c) { return (u8)((c >> 10) & 31); }
static u16 bgr(u8 r, u8 g, u8 b) { return (u16)(r | (g << 5) | (b << 10)); }

/* De "#rrggbb" (8 bits por canal) a BGR555: se quedan los 5 bits altos. */
static u16 desde_hex(const char *h)
{
    unsigned r, g, b;
    sscanf(h + 1, "%02x%02x%02x", &r, &g, &b);
    return bgr((u8)(r >> 3), (u8)(g >> 3), (u8)(b >> 3));
}

/* Un canal: principal ± operando, a la mitad si se pide, con tope 0..31. */
static u8 canal(u8 p, u8 o, u8 restar, u8 mitad)
{
    int v = restar ? (int)p - (int)o : (int)p + (int)o;
    if (v < 0) v = 0;                                /* Al restar, no baja de 0...           */
    if (mitad) v >>= 1;                              /* ...la mitad, si CGADSUB tiene el 6... */
    if (v > 31) v = 31;                              /* ...y al sumar, no pasa de 31.        */
    return (u8)v;
}

/* La cuenta de la PPU para un píxel. `secundaria_vacia`: el operando es la pantalla
   secundaria pero en ese píxel no hay nada (solo el fondo): entonces se usa el COLOR
   FIJO y NO se divide entre 2. (Así el humo sobre el cielo sale con su color.) */
static u16 mezclar(u16 principal, u16 operando, u8 restar, u8 mitad, u8 secundaria_vacia,
                   u16 fijo)
{
    if (secundaria_vacia)
    {
        operando = fijo;
        mitad = 0;
    }
    return bgr(canal(rojo(principal), rojo(operando), restar, mitad),
               canal(verde(principal), verde(operando), restar, mitad),
               canal(azul(principal), azul(operando), restar, mitad));
}

static void ver(const char *que, u16 c)
{
    printf("  %-34s $%04X  r=%2u g=%2u b=%2u\n", que, c, rojo(c), verde(c), azul(c));
}

int main(void)
{
    const u16 negro = 0;
    const u16 calle = desde_hex("#b88e62");      /* PALETA_ARQ: la tierra clara.          */
    const u16 montana = desde_hex("#3e2a52");    /* PALETA_LEJOS: una ladera.             */
    const u16 humo = desde_hex("#b8aab0");       /* PALETA_OBJETOS: el gris del humo.      */
    const u16 fachada = desde_hex("#d8a868");    /* PALETA_ARQ: el yeso al sol.           */
    u16 tarou, sin_mascara, con_mascara;

    printf("A. La sombra (paleta 4, color negro) sobre la calle: CGWSEL=$02 (operando:\n");
    printf("   la secundaria, que tiene BG1+BG2 por TS=$03), CGADSUB=$50 (media suma, sprites 4..7)\n");
    ver("calle", calle);
    ver("sombra encima: (negro + calle) / 2", mezclar(negro, calle, 0, 1, 0, negro));

    printf("\nB. El humo (paleta 7) con el mismo modo:\n");
    ver("montana", montana);
    ver("humo", humo);
    ver("humo sobre la montana: (h + m) / 2", mezclar(humo, montana, 0, 1, 0, negro));
    ver("humo sobre el cielo (secundaria vacia)", mezclar(humo, 0, 0, 1, 1, negro));

    printf("\nC. La media luz de la pausa en un fondo: COLDATA=$E0 (color fijo negro),\n");
    printf("   CGWSEL=$00 (operando: el color fijo), CGADSUB=$F3 (restar, mitad, BG1, BG2,\n");
    printf("   sprites 4..7 y el color de fondo)\n");
    ver("fachada", fachada);
    ver("fachada en pausa: (f - 0) / 2", mezclar(fachada, negro, 1, 1, 0, negro));

    printf("\nD. Un sprite de paleta 0..3 (Tarou) no entra en la matematica: se oscurece\n");
    printf("   su PALETA (oscurecer() de pantalla_fase1.c)\n");
    tarou = bgr(21, 13, 7);                      /* Un naranja (los tres canales, impares). */
    sin_mascara = (u16)(tarou >> 1);
    con_mascara = (u16)((tarou >> 1) & 0x3DEF);
    ver("color de Tarou", tarou);
    ver("v >> 1 (MAL: bits que se cuelan)", sin_mascara);
    ver("(v >> 1) & 0x3DEF (bien)", con_mascara);
    printf("  Sin mascara, el bit bajo del verde cae en el bit alto del rojo (r=%u en vez de %u)\n",
           rojo(sin_mascara), rojo(con_mascara));
    printf("  y el bajo del azul en el alto del verde (g=%u en vez de %u).\n",
           verde(sin_mascara), verde(con_mascara));

    printf("\nE. El destello del ejercicio: COLDATA=$E0|16 (gris 16), CGWSEL=$00, CGADSUB=$23\n");
    printf("   (sumar, sin mitad, BG1, BG2 y fondo)\n");
    ver("fachada con destello", mezclar(fachada, bgr(16, 16, 16), 0, 0, 0, negro));
    ver("calle con destello", mezclar(calle, bgr(16, 16, 16), 0, 0, 0, negro));
    printf("  (El rojo de la fachada llega al tope: 27 + 16 = 43 -> 31.)\n");
    return 0;
}
