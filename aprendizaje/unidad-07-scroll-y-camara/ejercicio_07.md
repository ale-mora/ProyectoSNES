# Ejercicio 07 — Parallax en la mini-ROM y una cámara para cualquier nivel

Dos partes: A en la mini-ROM (se aprueba con una captura de Mesen) y B en el
juego real, en el PC (se aprueba con las pruebas en verde).

## Parte A — La mini-ROM (TODO 1..3 de `rom/src/parallax.c`)

Tal como está, la mini-ROM enseña los arbustos del título delante de las
montañas del nivel, un cielo azul liso y cuatro líneas de texto. Derecha e
Izquierda mueven la capa cercana; las montañas no se mueven y la cámara no
tiene límites.

1. **TODO 1 — el recorte.** En `mover_camara`, recorta `c` a
   `0..CAMARA_MAX` como hace `camara_para` (`src/juego/camara.c`). Antes de
   escribirlo, pulsa Izquierda nada más arrancar con el esqueleto y mira qué
   pone `CAM`: es el `65534` de la trampa del u16.
2. **TODO 2 — el parallax.** Después de `WaitForVBlank()`, da a la capa
   lejana (fondo 1) la mitad del scroll de la cámara con `bgSetScroll`. Sin
   división: `>> 1`. Cuando funcione, prueba también `>> 2` (montañas más
   lejos) y `(camara >> 1) + (camara >> 2)` (3/4, más cerca) y apunta qué
   sensación da cada uno. Deja `>> 1`, que es lo que hace el juego.
3. **TODO 3 — una columna del anillo.** Completa `pintar_columna` con los
   registros de `video.h`: `REG_VMAIN = VMAIN_COLUMNA`, la dirección de la fila
   0 de la columna en `REG_VMADDLH` y 32 escrituras de `ENTRADA_RAYA` en
   `REG_VMDATALH`; al final, `REG_VMAIN = VMAIN_NORMAL`. Pulsa A: aparece una
   raya roja vertical en el centro de la pantalla. Ahora avanza con Y
   mantenido: **la raya vuelve a aparecer cada 512 px de cámara**. Explica por
   qué con la tabla del paso 3 de `demo_07_anillo.c`.

Preguntas (en tu cuaderno):

- ¿Qué valores de `NEAR` y `FAR` (la parte del scroll que usa la PPU: `&
  511`) se ven cuando `CAM` vale 1792? Compáralos con la última fila de la
  demo de PC.
- Si en `pintar_columna` olvidas devolver `REG_VMAIN` a 0x80, ¿qué función
  del SDK que use la mini-ROM después escribiría mal? (Pista: el texto.)

Compilar y probar (PowerShell, dentro de `rom/`):

```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
cd E:\ProyectoSNES
.\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES\aprendizaje\unidad-07-scroll-y-camara\rom\u07_parallax.sfc -Duracion 14 -Cada 1 -Espera 3 -Guion "1,1.3,A;2,12,Right;6,12,Y;12.5,12.8,A"
```

El guion (sus segundos son los de las capturas, que empiezan unos 3 s
después de arrancar la ROM): A a 1 s (raya en la columna 16 del nivel, la del
centro: x = 128), Derecha de 2 a 12 s (a 2 px por frame = 120 px/s hasta los
6 s: cámara 480; desde ahí con Y, a 8 px = 480 px/s: llega al tope de 1792
hacia los 8,7 s) y A otra vez al final.
Mientras dura la captura, no uses el equipo (las teclas se pierden si el foco
de Mesen salta).

Lo que debe verse (con la solución):

| Captura | Pantalla |
|---|---|
| `t1s`–`t2s` | la raya roja en el centro (`CAM 0`; según el instante exacto de la pulsación, desde `t1s` o desde `t2s`) |
| `t3s`–`t4s` | arbustos y montañas avanzando, las montañas a la mitad (`CAM` ~120 y ~240, `FAR` = la mitad); la raya ya salió por la izquierda (sale con `CAM` > 128, hacia 3,1 s) |
| `t5s`–`t6s` | la raya **vuelve a entrar por la derecha** hacia 5,2 s: con `CAM` = 384 la columna 16 + 64 del anillo (x = 128 + 512 = 640 en el nivel) queda en x = 256 de la pantalla; en `t6s` (`CAM` 480) está en x = 160 |
| `t7s`–`t8s` | con Y, 8 px por frame: la cámara salta ~480 px entre capturas y la raya pasa una vez por cada 512 px |
| `t9s`–`t12s` | `CAM 1792` quieto aunque se siga pulsando Derecha; `NEAR 256  FAR 384`; `COL 224  RING 32  +1024`. La raya vieja no se ve (cae en x = 384, fuera) |
| `t13s` | una raya nueva en el centro (A a 12,5 s: columna 240 del nivel, 48 del anillo, palabra +1040) |

Las tres filas que cambian se reescriben una por frame (abajo, «Por qué el
texto no usa `consoleDrawText`»), así que en una captura con la cámara en
marcha pueden diferir en 1–2 frames (2–16 px); paradas, coinciden.

**Por qué el texto no usa `consoleDrawText`.** La primera versión de la
mini-ROM escribía esas tres filas con `consoleDrawText` y `%d` en cada
frame. En Mesen la cámara iba a ~40 px/s sin Y (y 130–160 con Y) en vez de
120 y 480: cada vuelta del bucle costaba unos 3 frames, casi todo en formatear
los cinco números (`%d` divide por 10, y el 65816 no tiene división). A los
12 s la cámara iba por 1068 y la raya volvía a entrar a los ~11 s
(2 + 384 / 40). Además la NMI del SDK copiaba el mapa de texto en mitad del
formateo: filas de frames distintos en la misma imagen. La solución es la del
HUD del juego (`src/juego/hud_debug.c`): números por restas, una fila por
frame en un tilemap en la WRAM y un DMA de 64 bytes en el VBlank.

## Parte B — Una cámara para cualquier ancho de nivel (juego real, PC)

`camara_para` recorta con `ANCHO_NIVEL - ANCHO_PANTALLA`, una constante:
con ella no se puede probar qué haría la cámara en un nivel de otro ancho
(el nivel 2 no tiene por qué medir 2048). Trabaja en un clon o en una rama
propia (nunca en la rama del autor):

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio
cd E:\ProyectoSNES_trabajo\mio
```

1. En `src/juego/camara.h` declara
   `u16 camara_para_ancho(u16 x_pix, u16 ancho_nivel);` con su comentario.
2. En `src/juego/camara.c` escribe `camara_para_ancho` con la misma regla,
   pero con el tope `ancho_nivel - ANCHO_PANTALLA` calculado en `s16`, y deja
   `camara_para(x)` como una línea que la llama con `ANCHO_NIVEL` (así nada
   más del juego cambia).
3. En `tests/prueba_camara.c`, **antes** de escribir el paso 2 (para verla
   fallar), añade al menos estas comprobaciones: un nivel de 1024 px (`500` →
   420, `848` → 768, `900` → 768, `10` → 0), uno de una sola pantalla (256 px:
   la cámara siempre en 0) y uno de 512 px (`400` → 256). Calcula cada valor
   a mano antes de escribirlo.
4. Compila y pasa las pruebas:

```
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

5. Como tocaste un `.h`, compila la ROM desde cero (`make clean` y `make`,
   con el entorno de la parte A) y comprueba `No warnings or errors found.`

Pregunta: ¿qué devuelve tu función con `ancho_nivel = 200` (un nivel más
estrecho que la pantalla) y por qué? ¿Merece una comprobación o un
comentario en el `.h`?

## Aprobación

- Captura de la mini-ROM con tus TODO: las dos capas desplazadas (a distinta
  distancia entre dos capturas), la raya roja y `CAM 1792` en el tope.
- `tests/pruebas.exe` en verde con tus comprobaciones nuevas (con las 8 de la
  solución: `375 pruebas, 0 fallos`; antes eran 367) y la ROM compilando sin
  avisos.

Soluciones: `solucion/parallax.c` (la mini-ROM), `solucion/camara.c`,
`camara.h`, `prueba_camara.c` y el parche `solucion/camara_ancho.patch`
(`git apply` en tu clon).
