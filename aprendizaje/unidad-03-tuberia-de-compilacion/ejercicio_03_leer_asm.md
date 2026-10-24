# Ejercicio 03 — Leer el ensamblador de `camara_para`

**Qué se practica:** pedirle a `make` un archivo intermedio, encontrar en él
cada sentencia de C y reconocer cómo llama y devuelve una función de `tcc816`.
No hace falta saber ensamblador del 65816: el README de la unidad y las pistas
de abajo bastan. La unidad 14 lo explica a fondo.

## El C de partida

`src/juego/camara.c` (la cámara se ve en la unidad 07; aquí solo importa la
aritmética):

```c
#define CAMARA_MAX (ANCHO_NIVEL - ANCHO_PANTALLA)   /* 2048 - 256 = 1792 */

u16 camara_para(u16 x_pix)
{
    s16 camara = (s16)x_pix + (ANCHO_CELDA / 2) - (ANCHO_PANTALLA / 2);
    if (camara < 0)
    {
        camara = 0;
    }
    if (camara > CAMARA_MAX)
    {
        camara = CAMARA_MAX;
    }
    return (u16)camara;
}
```

con `ANCHO_CELDA = 96` y `ANCHO_PANTALLA = 256` (`src/juego/nivel.h`).

## TODO 1 — Generar el `.asm` en tu clon

No trabajes en `E:\ProyectoSNES`: haz un clon tuyo (los cambios y los archivos
generados no deben ir a parar al repositorio):

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio
git -C E:\ProyectoSNES_trabajo\mio checkout -q plataformas-r2
cd E:\ProyectoSNES_trabajo\mio
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
make src/juego/camara.asm
```

Verás la salida de `816-opt` («15 optimizations performed in total») y al
final `rm src/juego/camara.ps`: `make` borra el `.ps` pero deja el `.asm`,
porque lo pediste como objetivo.

## TODO 2 — Sacarlo de `src/` enseguida

```
Copy-Item src\juego\camara.asm E:\ProyectoSNES_trabajo\mi_camara.asm
Remove-Item src\juego\camara.asm
```

Explica en una frase, en tu archivo, **por qué** hay que borrarlo de `src/`
(pista: qué hace `snes_rules` con `src/*/*.asm`).

Si tu `.asm` no es idéntico al de referencia de esta carpeta (`camara.asm`, 85
líneas, generado el 2026-10-22 con FastROM), anota la diferencia: el código
del juego puede haber cambiado desde entonces, y manda el tuyo.

## TODO 3 — Anotar línea a línea

Pon un comentario (`;` hasta el final de la línea) en **cada** línea de tu
copia diciendo qué hace y a qué parte del C corresponde. Pistas para lo que
todavía no se ha enseñado:

| Instrucción | Significa |
|---|---|
| `lda X` / `sta X` | cargar X en el registro A / guardar A en X |
| `clc` + `adc #n` | A = A + n (el 65816 siempre suma con acarreo: `clc` lo pone a 0) |
| `sec` + `sbc #n` | A = A − n (`sec` pone el acarreo a 1, que en la resta significa «sin préstamo») |
| `stz X` | guardar un 0 en X |
| `n,s` | la dirección «puntero de pila + n»: así se leen parámetros y variables locales |
| `tsa` / `tas` | copiar el puntero de pila a A / A al puntero de pila |
| `ldx #1` / `dex` / `txa` / `tay` | X = 1 / X = X − 1 / A = X / Y = A |
| `bmi` / `bpl` | saltar si el resultado fue negativo / positivo (bandera N) |
| `bvc` | saltar si no hubo desbordamiento con signo (bandera V) |
| `beq` / `bne` | saltar si el resultado fue 0 / distinto de 0 (bandera Z) |
| `brl` | salto incondicional (largo) |
| `+`, `++`, `+++` | etiquetas anónimas: `bmi +` salta al siguiente `+` de abajo |
| `tcc__r0`, `tcc__r5` | casillas de 16 bits que `tcc816` usa como registros extra |
| `rtl` | volver de una función llamada con `jsl` (guarda 3 bytes: banco y dirección) |
| `.SECTION`, `.ENDS`, `.BASE`, `.define`, `.ifgr` | órdenes para el ensamblador, no instrucciones de la CPU |

## TODO 4 — Señalar las cinco piezas

Marca en tu archivo, con un comentario en mayúsculas, las cinco líneas o
grupos de líneas que pide el criterio de aprobación:

1. **CARGA DEL PARÁMETRO** `x_pix` (y explica de dónde sale el número de
   bytes por encima del puntero de pila: cuántos son de la dirección de
   vuelta y cuántos de la variable local).
2. **MEDIA CELDA** (`+ 48`).
3. **MEDIA PANTALLA** (`− 128`).
4. **RECORTE POR ABAJO** (`camara < 0`) y **RECORTE POR ARRIBA**
   (`camara > 1792`).
5. **RETORNO**: dónde queda el valor devuelto y qué instrucción vuelve.

## TODO 5 — Tres preguntas

Respóndelas al final de tu archivo:

1. En el C pone `ANCHO_CELDA / 2`. ¿Hay alguna división en el `.asm`? ¿Por
   qué no? (Pista: unidad 00, preprocesador; ¿quién hace la cuenta?)
2. `camara_para` acaba en `rtl` y no en `rts`. ¿Qué dice eso de cómo la
   llaman las otras funciones y de en qué banco puede estar? Busca su
   dirección en el `.sym` del juego (`ProyectoSNES.sym`, tras un `make`
   completo en tu clon) y di en qué banco ha caído.
3. Para comparar `camara > 1792` el compilador usa nueve instrucciones
   (de `ldx #1` a `dex`) y luego mira X. Para `camara < 0`, cinco (de `sec` a `bmi`). ¿Qué
   diferencia hay entre los dos casos que lo explique? (Pista: el caso
   `camara == 1792`.)

## Comprobación

Compara tus anotaciones con `solucion/camara_anotado.asm`. No tienen que
decir lo mismo con las mismas palabras: tienen que identificar las mismas
cinco piezas. Al terminar, borra tu clon si ya no lo necesitas.

**Respuestas de referencia** (léelas después):

1. No: `ANCHO_CELDA / 2` son dos constantes, y el compilador hace la cuenta
   al compilar (`adc.w #48`). Solo cuesta dividir en la SNES cuando uno de
   los operandos es una variable.
2. `rtl` saca 3 bytes de la pila (banco + dirección): la función se llama con
   `jsl` («salto largo»), que puede cruzar de banco. `tcc816` lo hace con
   todas, porque el enlazador puede poner cada función en cualquier banco
   (sección `SUPERFREE`). En el juego compilado el 2026-10-22,
   `008ecbc7 camara_para`: banco `$8E`.
3. «Mayor que» es «distinto **y** no menor»: hay que descartar el caso igual
   (`beq ++` → falso) antes de mirar el signo de la resta; «menor que 0» es
   solo «el signo de `camara − 0` es negativo».
