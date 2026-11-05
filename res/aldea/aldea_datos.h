/* aldea_datos.h — GENERADO por scripts/preparar_aldea.py: no editar a mano.
   La capa cercana de la aldea (res/aldea/cerca_*.bin): cuántos tiles tiene y dónde
   están los del noren, que el juego cambia cada 8 frames (noren_til.bin: el cuadro 1
   y luego el 2, ALDEA_NOREN_TILES tiles cada uno). */
#ifndef ALDEA_DATOS_H
#define ALDEA_DATOS_H
#define ALDEA_TILES 512
#define ALDEA_NOREN_PRIMERO 1
#define ALDEA_NOREN_TILES 8
#define ALDEA_PALETA 4          /* La primera de las dos (4 y 5). */
/* El humo: de qué punto de los tejados sale (x, y del mundo), ordenados por x. Las
   tablas, solo en el archivo que define ALDEA_HUMOS_TABLAS (juego/ambiente.c). */
#define ALDEA_HUMOS 7
#ifdef ALDEA_HUMOS_TABLAS
static const u16 aldea_humo_x[ALDEA_HUMOS] = {88, 359, 728, 999, 1368, 1639, 2008};
static const u16 aldea_humo_y[ALDEA_HUMOS] = {82, 82, 82, 82, 82, 82, 82};
#endif
#endif
