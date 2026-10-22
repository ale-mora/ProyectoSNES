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

El guion: A a 1 s (raya en la columna 16 del nivel, la del centro: x = 128),
Derecha de 2 a 12 s (a 2 px por frame hasta los 6 s: cámara ~480; con Y, a
8 px: llega al tope de 1792) y A otra vez al final.
Mientras dura la captura, no uses el equipo (las teclas se pierden si el foco
de Mesen salta).

Lo que debe verse: en `t1s` la raya roja en el centro; de `t2s` a `t6s` los
arbustos avanzando y las montañas a la mitad de velocidad (comparad dos
capturas seguidas: los arbustos se mueven el doble); hacia `t5s` la raya
vuelve a entrar por la derecha (cámara ~400: x = 128 + 512 = 640 cae en el
mismo sitio del anillo); al final, `CAM 1792` quieto aunque se siga pulsando
Derecha, y una raya nueva en el centro.

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
