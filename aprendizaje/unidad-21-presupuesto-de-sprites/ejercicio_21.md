# Ejercicio de la unidad 21 — Presupuesto de sprites

Tres partes. La 1 es de PC; la 2 y la 3 se hacen en un **clon** del juego (nunca
en el árbol principal). Para las tres, prepara el clon y copia esta carpeta
dentro:

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio
Copy-Item -Recurse E:\ProyectoSNES\aprendizaje\unidad-21-presupuesto-de-sprites E:\ProyectoSNES_trabajo\mio\aprendizaje\
cd E:\ProyectoSNES_trabajo\mio\aprendizaje\unidad-21-presupuesto-de-sprites
```

(El clon sale de la rama `demo-secuencia-arranque`, commit `641d7c0`. Si usas
otro commit, las cifras de la unidad pueden cambiar.)

## Parte 1 (PC): `ejercicio_21_presupuesto.c`

Compila tal como está, sin avisos, y da todo ceros. Completa los cinco `TODO`
**sin mirar** `src/juego/subida.c` ni `solucion/`:

| TODO | Función | Qué calcula |
|---|---|---|
| 1 | `tile_pieza(k)` | el tile de la pieza `k` en su hueco, con desplazamientos (sin `/`, `*` ni `%`) |
| 2 | `plan_subida(n, plan)` | las copias de un cuadro de `n` piezas: pares completos (2 × 512 B) y el último par a medias (2 × `q`·64 B) |
| 3 | `vblanks(plan, n, tope)` | cuántos VBlanks tarda el plan con un tope de bytes por VBlank (siempre avanza: mínimo una copia) |
| 4 | `max_ranuras(lista, n)` | el máximo de ranuras de 8 px en una línea de una escena inventada (2 ranuras por pieza y línea) |
| 5 | `ninjas_que_caben(sombras)` | el mayor número de ninjas a la misma `y` que no pasa de 34 ranuras |

```text
gcc -std=c99 -Wall -Wextra -o e21.exe ejercicio_21_presupuesto.c
.\e21.exe > mia.txt
Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_21_presupuesto.txt) -SyncWindow 0
```

La salida esperada exacta está en la cabecera del archivo:

```text
1. tiles: 0 14 32 64 78
2. plan de 21 piezas: 6 copias
     copia 0: origen 0 vram 0 bytes 512
     ...
     copia 5: origen 2368 vram 1280 bytes 320
3. vblanks: tope1536=2 tope65535=1 tope1024=3 tope100=6
4. ranuras: Tarou=10 Tarou+2ninjas=22 Tarou+2ninjas+sombras=34 Tarou+3ninjas+sombras=44
5. caben: sin sombras=4 con sombras=2
```

Pistas: en el TODO 2 los tiles del origen están en el **orden de subida**, así
que el origen de la copia siguiente es el origen anterior más sus bytes; en la
VRAM, la fila de abajo de un par está 256 palabras más allá y el par siguiente,
512. En el TODO 5, ¿qué pasa en las líneas de los pies, donde se apilan cuerpos
y sombras?

## Parte 2 (clon): las pruebas de los cuadros reales

Antes de tocar nada, comprueba el punto de partida:

```text
cd E:\ProyectoSNES_trabajo\mio
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

Debe decir **560 pruebas, 0 fallos**. Ahora añade a `tests/prueba_subida.c`
(antes de la prueba del tile de la pieza) dos bloques que fijen lo que enseñó la
demo, **calculando antes a mano** el resultado esperado:

1. El peor cuadro del ninja: 21 piezas. ¿Cuántas copias? ¿De cuántos bytes cada
   una, y en qué dirección de la VRAM? ¿Cuántos VBlanks con el tope 1536, y qué
   toma cada uno? ¿Y sin tope?
2. El peor cuadro de Tarou andando: 16 piezas. Mismas preguntas.

Vuelve a compilar y ejecuta. Debe decir **570 pruebas, 0 fallos** (tu cifra
exacta depende de cuántos `COMPROBAR` escribas; la solución añade 10).

## Parte 3 (clon): una configuración nueva en la prueba de carga

La pregunta: *¿caben Tarou y 3 ninjas a la misma `y` si **no** hay sombras?*

1. **Predice** con la demo. Añade a `CONFIGS_PRED` de `demo_21_presupuesto.c` la
   fila `{"Tarou + 3 misma y, SIN sombras", 3, {0, 0, 0}, 0}` (y sube a 7 el
   tamaño del array y el límite del bucle `for (c = 0; c < ...`). Anota cuántas
   ranuras da andando y atacando, y si alguna línea pasa de 34.
2. **Añádela** al juego: una fila más en `CONFIGS` de `src/pantallas/prueba_carga.c`
   (`{ninjas, ataque, reparto, sombras, subidas}` = `{3, 1, 0, 0, 1}`) y
   `CONFIGS_N` a 9.
3. **Compila** la ROM de prueba de carga, con `make clean` antes y después:

```text
cd E:\ProyectoSNES_trabajo\mio
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
make clean; $env:CFLAGS="-DPRUEBA_CARGA"; make; $env:CFLAGS=$null; make clean
```

Debe terminar con `No warnings or errors found.`

4. (A mano, si quieres verla) abre el `.sfc` en Mesen (se copia aparte antes del
   segundo `make clean`) o usa
   `.\scripts\capturar_mesen.ps1 -Rom RUTA\prueba_carga.sfc -Duracion 66 -Cada 2 -Espera 2`
   y mira la configuración 8 (la última, hacia el segundo 48–54): con las
   sombras fuera no debería faltarle ningún trozo a Tarou en la línea de los pies.
   Compárala con la configuración 3 (misma `y`, con sombras), donde sí falta.

## Criterio de aprobación

1. La parte 1 compila con `gcc -std=c99 -Wall -Wextra` sin avisos y su salida es
   **idéntica** a la esperada (`Compare-Object` sin diferencias).
2. Antes: **560 pruebas, 0 fallos**; después de la parte 2: **570** (o las que
   correspondan a tus `COMPROBAR`), **0 fallos**.
3. La parte 3 compila sin avisos y tu predicción coincide con la de la demo
   (andando: 34 ranuras; atacando: 32; ninguna línea > 34).
4. Opcional, por el autor: la captura de la configuración 8 en Mesen. **No se
   hizo al escribir esta unidad.**

## Solución

En `solucion/`: `ejercicio_21_presupuesto.c` (la parte 1), `prueba_subida.diff` (la
parte 2), `demo_config8.diff` y `prueba_carga.diff` (la parte 3). Las salidas
reales de su ejecución están en `capturas/`: `salida_ejercicio_21_presupuesto.txt`,
`pruebas.txt`, `prediccion_config8.txt` y `compilacion_rom.txt`.
