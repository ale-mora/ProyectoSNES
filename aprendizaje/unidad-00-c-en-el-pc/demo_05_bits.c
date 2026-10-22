/*---------------------------------------------------------------------------------
    demo_05_bits.c — Demo 5 de la unidad 00: binario, hexadecimal y operadores
    de bits.

    PROPÓSITO
      Enseña, en cinco pasos numerados en la salida, cómo se guardan varios
      datos de sí/no dentro de un solo número: 1) el mismo valor en decimal,
      hexadecimal y binario; 2) los operadores & | ^ ~ << >>; 3) probar,
      encender, apagar y alternar un bit con una máscara; 4) los bits del
      mando de la SNES (KEY_A, KEY_B... copiados del SDK) y el truco de
      «recién pulsado» de src/juego/guion.c:44; 5) el byte de atributos de un
      sprite de la OAM (paleta, prioridad, volteo), como lo arma OAM_ATTR del
      SDK.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf, putchar), <stdint.h> (uint8_t, uint16_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   tools/pvsneslib/pvsneslib/include/snes/input.h:49-63 (KEY_*),
                snestypes.h:34 (BIT), sprite.h:228 (OAM_ATTR), src/juego/mando.c.
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_05.exe demo_05_bits.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC). Los valores imitan dos cosas de la SNES:
      la palabra de 16 bits del mando y el byte de atributos de la OAM.

    VARIABLES
      a, b        u8    0xCA, 0x0F         dos bytes para probar los operadores
      banderas    u8    0..255             8 interruptores de sí/no en un byte
      ahora       u16   bits KEY_*         botones mantenidos en este frame
      antes       u16   bits KEY_*         botones mantenidos en el frame anterior
      nuevos      u16   bits KEY_*         los que bajaron en este frame
      attr        u8    0..255             byte de atributos de un sprite

    → conceptos (README de la unidad 00): bit, binario, hexadecimal, máscara,
      operadores de bits
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf y putchar.                            */
#include <stdint.h>                  /* Trae los enteros de tamaño fijo.                  */

typedef uint8_t u8;                  /* Los nombres del juego (src/sistema/tipos.h).      */
typedef uint16_t u16;

/*--- Copiado del SDK (snestypes.h:34 e input.h:49-63): un bit por botón ------------*/
#define BIT(n) (1 << n)              /* El bit n encendido: BIT(3) = 1 << 3 = 8.          */
typedef enum KEYPAD_BITS             /* Un enum (parte 8): nombres para números.          */
{
    KEY_A = BIT(7),       //!< pad A button.
    KEY_B = BIT(15),      //!< pad B button.
    KEY_SELECT = BIT(13), //!< pad SELECT button.
    KEY_START = BIT(12),  //!< pad START button.
    KEY_RIGHT = BIT(8),   //!< pad RIGHT button.
    KEY_LEFT = BIT(9),    //!< pad LEFT button.
    KEY_DOWN = BIT(10),   //!< pad DOWN button.
    KEY_UP = BIT(11),     //!< pad UP button.
    KEY_R = BIT(4),       //!< Right shoulder button.
    KEY_L = BIT(5),       //!< Left shoulder button.
    KEY_X = BIT(6),       //!< pad X button.
    KEY_Y = BIT(14),      //!< pad Y button.
} KEYPAD_BITS;

/*--- Byte de atributos de un sprite (OAM), igual que OAM_ATTR de sprite.h:228 -------*/
#define ATTR_VFLIP 0x80              /* Bit 7: voltear en vertical.                       */
#define ATTR_HFLIP 0x40              /* Bit 6: voltear en horizontal (espejo).            */

/* Imprime los `bits` bits de menor peso de `valor`, del más alto al más bajo, con
   un espacio cada 4 (un dígito hexadecimal). La máscara empieza en el bit más
   alto y se desplaza 1 a la derecha en cada vuelta: así no hace falta
   desplazar por una cantidad variable (lento en tcc816, ver CLAUDE.md). */
static void imprimir_binario(u16 valor, u8 bits)
{
    u16 mascara = (u16)(1 << (bits - 1));    /* El bit más alto que se imprime.          */
    u8 contados = 0;                         /* Cuántos bits se han impreso ya.          */
    while (mascara != 0)                     /* Hasta pasar por el bit 0.                */
    {
        putchar((valor & mascara) ? '1' : '0');   /* ¿Está encendido este bit?           */
        contados++;
        if (mascara != 1 && (contados & 3) == 0)  /* Cada 4 bits (y no al final)...      */
        {
            putchar(' ');                    /* ...un espacio para leerlo mejor.         */
        }
        mascara = (u16)(mascara >> 1);       /* Pasa al bit siguiente, uno más bajo.     */
    }
}

/* Una línea "texto: 0xHH = bbbb bbbb" para un byte. */
static void linea8(const char *texto, u8 valor)
{
    printf("%s 0x%02X = ", texto, valor);    /* %02X: hexadecimal, 2 cifras, con ceros.  */
    imprimir_binario(valor, 8);
    putchar('\n');
}

/* Lo mismo para una palabra de 16 bits (el mando). */
static void linea16(const char *texto, u16 valor)
{
    printf("%s 0x%04X = ", texto, valor);    /* %04X: 4 cifras hexadecimales.            */
    imprimir_binario(valor, 16);
    putchar('\n');
}

int main(void)
{
    /* 1. Un número, tres maneras de escribirlo. 0x delante = hexadecimal. */
    u8 a = 202;                              /* Decimal: 202.                            */
    printf("1. 202 en decimal = 0x%02X en hexadecimal = ", a);
    imprimir_binario(a, 8);                  /* 1100 1010: C = 1100, A = 1010.           */
    putchar('\n');
    printf("1. 0x2000 = %d: solo el bit 13 encendido\n", 0x2000);

    /* 2. Los seis operadores de bits, sobre dos bytes. */
    u8 b = 0x0F;                             /* 0000 1111: los 4 bits bajos.             */
    linea8("2. a          =", a);
    linea8("2. b          =", b);
    linea8("2. a & b      =", (u8)(a & b));  /* Y: 1 solo donde los dos tienen 1.        */
    linea8("2. a | b      =", (u8)(a | b));  /* O: 1 donde alguno tiene 1.               */
    linea8("2. a ^ b      =", (u8)(a ^ b));  /* O exclusivo: 1 donde son distintos.      */
    linea8("2. ~a         =", (u8)~a);       /* NO: da la vuelta a cada bit.             */
    linea8("2. 0x05 << 2  =", (u8)(0x05 << 2));  /* Corre los bits 2 sitios a la izquierda. */
    linea8("2. 0x50 >> 4  =", (u8)(0x50 >> 4));  /* Corre los bits 4 sitios a la derecha.  */

    /* 3. Una máscara elige bits. Un byte = 8 interruptores de sí/no. */
    u8 banderas = 0;                         /* Todos apagados.                          */
    banderas = (u8)(banderas | BIT(3));      /* Encender el bit 3: OR con la máscara.    */
    linea8("3. encender bit 3:", banderas);
    banderas = (u8)(banderas | BIT(0));      /* Encender el bit 0 sin tocar el 3.        */
    linea8("3. encender bit 0:", banderas);
    printf("3. bit 3 encendido? %d\n", (banderas & BIT(3)) != 0);   /* Probar: AND, != 0. */
    printf("3. bit 2 encendido? %d\n", (banderas & BIT(2)) != 0);
    banderas = (u8)(banderas & ~BIT(3));     /* Apagar el bit 3: AND con la máscara al revés. */
    linea8("3. apagar bit 3:  ", banderas);
    banderas = (u8)(banderas ^ BIT(0));      /* Alternar el bit 0: XOR (1 -> 0 -> 1...). */
    linea8("3. alternar bit 0:", banderas);

    /* 4. El mando: 12 botones en una palabra de 16 bits (padsCurrent(0) en la SNES). */
    u16 antes = KEY_RIGHT;                   /* Frame anterior: solo la derecha.         */
    u16 ahora = KEY_RIGHT | KEY_B;           /* Este frame: derecha y B a la vez.        */
    linea16("4. KEY_B           =", KEY_B);
    linea16("4. KEY_RIGHT       =", KEY_RIGHT);
    linea16("4. ahora           =", ahora);
    printf("4. B mantenido? %d  A mantenido? %d\n",
           (ahora & KEY_B) != 0, (ahora & KEY_A) != 0);   /* Lo que hace mando.c.        */
    u16 nuevos = (u16)(ahora & ~antes);      /* Encendidos ahora Y apagados antes.       */
    linea16("4. ahora & ~antes  =", nuevos); /* Solo B: acaba de bajar (padsDown).       */

    /* 5. El byte de atributos de un sprite: vflip, hflip, prioridad, paleta, tile. */
    u8 prioridad = 2, paleta = 3;            /* Prioridad 0..3 (2 bits), paleta 0..7 (3). */
    u16 tile = 0x105;                        /* Tile 261: necesita 9 bits.               */
    u8 attr = (u8)(ATTR_HFLIP                /* Bit 6: mirando a la izquierda.           */
                   | (prioridad << 4)        /* Bits 5-4.                                */
                   | (paleta << 1)           /* Bits 3-1.                                */
                   | ((tile >> 8) & 1));     /* Bit 0: el noveno bit del tile.           */
    linea8("5. attr =", attr);
    printf("5. paleta=%d prioridad=%d hflip=%d tile_alto=%d\n",
           (attr >> 1) & 7,                  /* Bajar los bits 3-1 a la derecha y quedarse con 3. */
           (attr >> 4) & 3,                  /* Bits 5-4: quedarse con 2.                */
           (attr & ATTR_HFLIP) != 0,         /* Un solo bit: probarlo.                   */
           attr & 1);
    linea8("5. attr ^ ATTR_HFLIP =", (u8)(attr ^ ATTR_HFLIP));   /* Darle la vuelta: mira a la derecha. */

    return 0;                                /* Terminó bien.                            */
}
