/*---------------------------------------------------------------------------------
    demo_05_mascaras.c — Demo de PC de la unidad 05: el mando como 16 bits.

    PROPÓSITO
      Simula en el PC lo que hace la rutina del VBlank de PVSnesLib con el mando
      (pad_keys, pad_keysold, pad_keysdown) y lo que hace src/juego/mando.c con
      esos bits, frame a frame, con un mando "grabado" en una tabla. Enseña:
        1. cada botón es un bit (las mismas máscaras KEY_* de input.h);
        2. recién pulsado = (ahora ^ antes) & ahora;  recién soltado = antes & ~ahora;
        3. la trampa de (u8)(mantenidos & KEY_B): B es el bit 15 y el cast lo pierde;
        4. por qué el switch del ejemplo input/controller falla con dos botones.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint16_t, uint8_t).
      Lo usan:  nadie: programa independiente de PC.
      Modelo:   tools/pvsneslib/pvsneslib/include/snes/input.h (KEY_*, padsDown),
                src/juego/mando.c, src/juego/guion.c.
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_05.exe demo_05_mascaras.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      GRABACION  u16[8]  lo que "llega del mando" en cada frame
      antes      u16     pad_keysold: el valor del frame anterior

    → conceptos (README de la unidad 05): máscara, recién pulsado, padsDown
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* printf.                                           */
#include <stdint.h>                  /* Enteros de tamaño fijo.                           */

typedef uint16_t u16;                /* Como en el juego (src/sistema/tipos.h).           */
typedef uint8_t u8;

/* Las máscaras de input.h (KEYPAD_BITS): un bit por botón. */
#define KEY_B      0x8000            /* bit 15 */
#define KEY_Y      0x4000            /* bit 14 */
#define KEY_SELECT 0x2000            /* bit 13 */
#define KEY_START  0x1000            /* bit 12 */
#define KEY_UP     0x0800            /* bit 11 */
#define KEY_DOWN   0x0400            /* bit 10 */
#define KEY_LEFT   0x0200            /* bit 9  */
#define KEY_RIGHT  0x0100            /* bit 8  */
#define KEY_A      0x0080            /* bit 7  */
#define KEY_X      0x0040            /* bit 6  */
#define KEY_L      0x0020            /* bit 5  */
#define KEY_R      0x0010            /* bit 4  */

#define FRAMES 8                     /* Frames de la grabación.                           */

/* Lo que lee el hardware en cada frame: B se pulsa en el 1 y se mantiene hasta el 3;
   en el 4 se suelta; en el 5 se pulsan Derecha y A a la vez; en el 7, nada. */
static const u16 GRABACION[FRAMES] = {
    0,                               /* frame 0: nada                                     */
    KEY_B,                           /* frame 1: baja B                                   */
    KEY_B,                           /* frame 2: B mantenido                              */
    KEY_B | KEY_RIGHT,               /* frame 3: B mantenido y baja Derecha               */
    KEY_RIGHT,                       /* frame 4: se suelta B                              */
    KEY_RIGHT | KEY_A,               /* frame 5: baja A con Derecha mantenida             */
    KEY_RIGHT | KEY_A,               /* frame 6: los dos mantenidos                       */
    0,                               /* frame 7: se suelta todo                           */
};

/* Lo que haría el switch del ejemplo input/controller con este valor. */
static const char *como_el_ejemplo(u16 pad)
{
    switch (pad)                     /* Compara el valor ENTERO con cada máscara...       */
    {
        case KEY_B:     return "B PRESSED";
        case KEY_RIGHT: return "RIGHT PRESSED";
        case KEY_A:     return "A PRESSED";
        default:        return "(nada)";  /* ...así que con dos botones no coincide ninguno. */
    }
}

int main(void)
{
    u16 antes = 0;                   /* pad_keysold: el frame anterior.                   */
    u8 f;                            /* Número de frame.                                  */

    printf("frame  pad    down   up     B?  B?(u8)  switch del ejemplo\n");
    for (f = 0; f < FRAMES; f++)
    {
        u16 ahora = GRABACION[f];                    /* pad_keys: lo que leyó la NMI.     */
        u16 down = (u16)((ahora ^ antes) & ahora);   /* pad_keysdown (vblank.asm).        */
        u16 up = (u16)(antes & ~ahora);              /* padsUp (input.asm).               */
        u8 b_bien = (u8)((ahora & KEY_B) != 0);      /* Como mando.c: comparar con 0.     */
        u8 b_mal = (u8)(ahora & KEY_B);              /* TRAMPA: 0x8000 en 8 bits = 0.     */

        printf("%5u  %04x   %04x   %04x   %u   %u       %s\n",
               (unsigned)f, (unsigned)ahora, (unsigned)down, (unsigned)up,
               (unsigned)b_bien, (unsigned)b_mal, como_el_ejemplo(ahora));
        antes = ahora;               /* El frame siguiente, esto será "antes".            */
    }
    return 0;                        /* Terminó bien.                                     */
}
