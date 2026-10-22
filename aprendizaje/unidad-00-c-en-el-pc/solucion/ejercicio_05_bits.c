/*---------------------------------------------------------------------------------
    solucion/ejercicio_05_bits.c — Solución del ejercicio 5 de la unidad 00.

    PROPÓSITO
      Las seis funciones de bits del ejercicio: cada una es una sola
      expresión con & | ^ ~ >>. main es idéntico al del ejercicio.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t, uint16_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/juego/mando.c (bit_probar es lo que hace con cada KEY_*),
                src/juego/guion.c:44 (recien_pulsados), src/juego/metasprite.c:40
                (el espejo con METASPRITE_HFLIP = 0x40).
      Compilar: gcc -std=c99 -Wall -Wextra -o solucion_05.exe solucion/ejercicio_05_bits.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      PAD[5]   const u16   bits KEY_*   lo que «pulsa» el jugador en 5 frames
      ahora    u16         bits KEY_*   mantenidos en el frame f
      antes    u16         bits KEY_*   mantenidos en el frame f - 1 (0 en el primero)
      pad      u16         bits KEY_*   palabra que se edita bit a bit en la parte 2
      attr     u8          0..255       byte de atributos de un sprite

    → conceptos (README de la unidad 00): máscara, operadores de bits, hexadecimal
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Trae uint8_t y uint16_t.                          */

/*
 * SALIDA ESPERADA (exacta):
 *   Frame 0: pad=0x0000 derecha=0 correr=0 saltar=0 saltar_pulsado=0
 *   Frame 1: pad=0x0100 derecha=1 correr=0 saltar=0 saltar_pulsado=0
 *   Frame 2: pad=0x8100 derecha=1 correr=0 saltar=1 saltar_pulsado=1
 *   Frame 3: pad=0x8100 derecha=1 correr=0 saltar=1 saltar_pulsado=0
 *   Frame 4: pad=0xC000 derecha=0 correr=1 saltar=1 saltar_pulsado=0
 *   Encender A y START: 0x1080
 *   Apagar A: 0x1000
 *   Apagar A otra vez: 0x1000
 *   Atributo 0x67: paleta=3 espejo=0x27 espejo del espejo=0x67
 *   Atributo 0x8E: paleta=7 espejo=0xCE espejo del espejo=0x8E
 *
 * Antes de escribir código, comprueba a mano dos cosas: por qué en el frame 3
 * saltar vale 1 pero saltar_pulsado vale 0 (B ya estaba en el frame 2), y por
 * qué 0x67 tiene la paleta 3 (escribe 0x67 en binario y mira los bits 3-1).
 */

typedef uint8_t u8;                  /* Los nombres del juego (src/sistema/tipos.h).      */
typedef uint16_t u16;

#define BIT(n) (1 << n)              /* Copiado de snestypes.h:34.                        */
enum                                 /* Copiado de input.h:49-63 (solo los que se usan).  */
{
    KEY_A = BIT(7),                  /* A.                                                */
    KEY_B = BIT(15),                 /* B: saltar en el juego.                            */
    KEY_START = BIT(12),             /* START.                                            */
    KEY_RIGHT = BIT(8),              /* Derecha de la cruceta.                            */
    KEY_Y = BIT(14),                 /* Y: correr en el juego.                            */
};

#define ATTR_HFLIP 0x40              /* Bit 6 del atributo: espejo horizontal.            */

/* TODO 1: devuelve 1 si ALGÚN bit de mascara está encendido en valor, y 0 si
           no. Pista: valor & mascara deja solo esos bits; compáralo con 0. */
u8 bit_probar(u16 valor, u16 mascara)
{
    return (u8)((valor & mascara) != 0);     /* != 0 da 1 o 0, sea cual sea el bit.      */
}

/* TODO 2: devuelve valor con los bits de mascara encendidos y los demás
           como estaban. Pista: OR. Recorta el resultado con (u16)(...). */
u16 bit_encender(u16 valor, u16 mascara)
{
    return (u16)(valor | mascara);           /* OR: los de la máscara pasan a 1.         */
}

/* TODO 3: devuelve valor con los bits de mascara apagados y los demás como
           estaban. Pista: AND con la máscara dada la vuelta (~). Cuidado: ~
           convierte a int y enciende también los bits 16..31; el (u16)(...)
           final los recorta. Apagar un bit ya apagado lo deja apagado. */
u16 bit_apagar(u16 valor, u16 mascara)
{
    return (u16)(valor & ~mascara);          /* AND con 0 donde la máscara tenía 1.      */
}

/* TODO 4: los botones que están en `ahora` y NO estaban en `antes`: los que
           acaban de bajar en este frame. Una sola expresión con & y ~. */
u16 recien_pulsados(u16 ahora, u16 antes)
{
    return (u16)(ahora & ~antes);            /* Encendido ahora Y apagado antes.         */
}

/* TODO 5: la paleta de un byte de atributos de la OAM está en los bits 3-1.
           Bájalos a la derecha (>>) y quédate solo con 3 bits (& 7). */
u8 attr_paleta(u8 attr)
{
    return (u8)((attr >> 1) & 7);            /* Bits 3-1 bajan a 2-0; & 7 = 0000 0111.   */
}

/* TODO 6: devuelve attr con el bit de espejo (ATTR_HFLIP) dado la vuelta: si
           estaba encendido, apagado, y al revés. Un solo operador. */
u8 attr_espejo(u8 attr)
{
    return (u8)(attr ^ ATTR_HFLIP);          /* XOR: alterna solo el bit 6.              */
}

/*--- main: igual en el ejercicio y en la solución -----------------------------------*/
static const u16 PAD[5] = {          /* Lo que mantiene el jugador en cada frame.         */
    0,                               /* Frame 0: nada.                                    */
    KEY_RIGHT,                       /* Frame 1: derecha.                                 */
    KEY_RIGHT | KEY_B,               /* Frame 2: derecha y B (acaba de saltar).           */
    KEY_RIGHT | KEY_B,               /* Frame 3: sigue con los dos.                       */
    KEY_B | KEY_Y,                   /* Frame 4: suelta la derecha, pulsa Y, sigue con B. */
};

int main(void)
{
    /* Parte 1: el mando frame a frame, como lo traduce mando_leer. */
    for (u8 f = 0; f < 5; f++)
    {
        u16 ahora = PAD[f];                          /* Lo de este frame.                 */
        u16 antes = (f > 0) ? PAD[f - 1] : 0;        /* Lo del anterior (nada antes del 0). */
        u16 nuevos = recien_pulsados(ahora, antes);  /* Los que acaban de bajar.          */
        printf("Frame %d: pad=0x%04X derecha=%d correr=%d saltar=%d saltar_pulsado=%d\n",
               f, ahora, bit_probar(ahora, KEY_RIGHT), bit_probar(ahora, KEY_Y),
               bit_probar(ahora, KEY_B), bit_probar(nuevos, KEY_B));
    }

    /* Parte 2: editar una palabra bit a bit. */
    u16 pad = 0;                                     /* Todo suelto.                      */
    pad = bit_encender(pad, KEY_A);
    pad = bit_encender(pad, KEY_START);
    printf("Encender A y START: 0x%04X\n", pad);
    pad = bit_apagar(pad, KEY_A);
    printf("Apagar A: 0x%04X\n", pad);
    pad = bit_apagar(pad, KEY_A);                    /* Ya estaba apagado: no cambia.     */
    printf("Apagar A otra vez: 0x%04X\n", pad);

    /* Parte 3: dos bytes de atributos de la OAM. */
    u8 attr = 0x67;                                  /* El de la demo_05.                 */
    printf("Atributo 0x%02X: paleta=%d espejo=0x%02X espejo del espejo=0x%02X\n",
           attr, attr_paleta(attr), attr_espejo(attr), attr_espejo(attr_espejo(attr)));
    attr = 0x8E;                                     /* Volteado en vertical, paleta 7.   */
    printf("Atributo 0x%02X: paleta=%d espejo=0x%02X espejo del espejo=0x%02X\n",
           attr, attr_paleta(attr), attr_espejo(attr), attr_espejo(attr_espejo(attr)));

    return 0;                                        /* Terminó bien.                     */
}
