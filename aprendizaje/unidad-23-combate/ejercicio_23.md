# Ejercicio 23 (juego real, con TDD) — Un cuarto golpe y un salto que esquiva

**Qué vas a hacer:** dos cambios al combate del juego, los dos **escribiendo
primero la prueba que falla** en `tests/prueba_combate.c`:

- **Parte A.** El combo de Tarou pasa de 3 golpes (daño 1 + 1 + 2) a **4**
  (daño 1 + 1 + 1 + 2): solo el cuarto derriba.
- **Parte B.** Un ninja no puede pegarle a un Tarou que **salta alto**: si
  tiene los pies a 12 px del suelo o más, el tajo pasa por debajo (y no se
  gasta: si Tarou aterriza mientras el tajo sigue activo, le da).

(La ficha del plan hablaba de «que un golpe de Tarou en el aire no cuente».
Hoy Tarou **no puede golpear en el aire** —`mundo_actualizar` solo empieza un
golpe con `!aire`, y el ataque aéreo está por venir—, así que la parte B es la
versión que sí tiene sentido: el aire contra los golpes que **recibe**.)

Trabaja en un **clon de la rama principal** (las unidades 21–26 no usan el tag
`plataformas-r2`), nunca en el árbol principal:

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio
```

Tiene que estar en el commit `641d7c0` o posterior de la rama
`demo-secuencia-arranque`. Las cifras de abajo son las de `641d7c0` (560
pruebas de partida). Antes de empezar, comprueba tu punto de partida (PowerShell):

```text
cd E:\ProyectoSNES_trabajo\mio
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

Debe terminar en `560 pruebas, 0 fallos` (`capturas/pruebas_base_560.txt`).
Cada parte se hace desde ese punto: haz `git switch -c parte-a` (y, para la B,
vuelve a la rama de partida con `git switch demo-secuencia-arranque` y
`git switch -c parte-b`).

**Antes de tocar nada, lee** `demo_23_combate.c` y su salida
(`capturas/salida_demo_23_combate.txt`) y haz la traza del salto
(`traza_23_salto.c`, `capturas/salida_traza_23_salto.txt`): la parte B usa sus
cifras.

---

## Parte A — Un combo de cuatro golpes

### TODO 1 — La prueba que falla (rojo)

Al final de `pruebas_combate` (antes de la llave que la cierra) escribe un
bloque nuevo que diga lo que quieres:

1. `GOLPES == 4`.
2. `combate_danio` devuelve 1, 1, 1 y 2 para los golpes 1..4, y solo
   `combate_derriba(4)` es verdadero.
3. Una partida entera contra un ninja (la de «El combo completo mata a un
   ninja» te sirve de modelo: el ninja se queda en x = 440 porque esa prueba
   **no llama a `ninja_actualizar`**, A cada 10 cuadros, 130 cuadros): conectan
   **4** golpes; tras el tercero el ninja sigue vivo con `NINJA_VIDA - 3` de
   vida y en `NINJA_RECIBIR`; al terminar, `parada == PARADA_DERRIBO` y
   `n[0].vida == 0`.

Compila y ejecuta. **Tiene que fallar** —si pasa, la prueba no prueba nada—
y por las razones esperadas: cinco comprobaciones rojas, las tres de arriba más
dos de la partida (`capturas/pruebas_parteA_1_falla.txt`):

```text
FALLO prueba_combate.c:390: GOLPES == 4
FALLO prueba_combate.c:391: combate_danio(1) == 1 && combate_danio(3) == 1 && combate_danio(4) == 2
FALLO prueba_combate.c:392: !combate_derriba(3) && combate_derriba(4)
FALLO prueba_combate.c:420: golpes_vistos == 4
FALLO prueba_combate.c:421: vida_tras_3 == NINJA_VIDA - 3 && estado_tras_3 == NINJA_RECIBIR
566 pruebas, 5 fallos
```

(Los números de línea dependen de dónde pongas tu bloque.)

### TODO 2 — Una sola constante (verde… a medias)

En `src/juego/mundo.h`: `#define GOLPES 4`. Nada más: `combate_danio` y
`combate_derriba` ya hablan de «el último» (`golpe >= GOLPES`), y `paso_golpe`
usa `c->golpe < GOLPES` para decidir si encadena. Vuelve a ejecutar.

Tus pruebas nuevas pasan, **pero fallan cinco de las viejas**
(`capturas/pruebas_parteA_2_solo_la_constante.txt`): las que describían el
combo de tres golpes. Es lo que tiene que pasar: la prueba es la especificación,
y cambiaste la especificación.

```text
FALLO prueba_combate.c:104: c.golpe == GOLPES
FALLO prueba_combate.c:114: c.golpe == 0 && c.cola == 0
FALLO prueba_combate.c:135: combate_danio(1) == 1 && combate_danio(2) == 1 && combate_danio(3) == 2
FALLO prueba_combate.c:136: !combate_derriba(1) && !combate_derriba(2) && combate_derriba(3)
FALLO prueba_combate.c:205: golpes_vistos == 3
566 pruebas, 5 fallos
```

### TODO 3 — Poner al día las pruebas viejas y los comentarios

Para cada una, decide si el **código** o la **prueba** están mal (aquí, la
prueba: el combo cambió a propósito):

- línea 104 y 114 («Combo: A a tiempo encadena»): ahora el golpe 3 no es el
  último. Añade un tercer encadenado y comprueba que el golpe 4 es `GOLPES`, y
  que **no hay quinto golpe**.
- 135 y 136: `combate_danio` 1, 1, 1, 2; `combate_derriba` solo el 4.
- 205: `golpes_vistos == 4`.

Actualiza también los comentarios que dicen «1 + 1 + 2» o «el tercero derriba»
(`combate.h`, `combate.c`, `ninja.h`: `NINJA_VIDA`, `mundo.h`). Una constante
con un comentario que miente es peor que sin comentario.

Resultado: `567 pruebas, 0 fallos` (`capturas/pruebas_parteA_3_solucion.txt`).

### TODO 4 — Que compile la ROM

Desde PowerShell, en el clon (entorno de la unidad 03; la limpieza es obligatoria
porque `snes_rules` no sigue las cabeceras):

```text
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make clean; make
```

Debe acabar en `No warnings or errors found.`
(`capturas/compilacion_rom_parteA.txt`). **No se abrió en Mesen**: solo se
comprobó que compila. Si la abres, ten en cuenta lo que dice la sección «Una
observación…» del README: contra un ninja que camina, el empujón de 12 px del
golpe aturdido puede sacarlo de la caja antes de que llegue el tercer golpe (no se midió con
cuatro golpes).

### Preguntas

1. Con `NINJA_VIDA 4` el ninja ya no muere «justo» con el combo: ¿con qué
   golpe se muere ahora y cuánto daño sobra? ¿Cambia eso la parada de impacto
   del tercer golpe?
2. ¿Qué otra constante tendrías que mirar si subieras `GOLPES` a 6? (Pista:
   el `id` de cada tajo es un `u8` que vive en 1..254 y se reinicia.)
3. ¿Por qué `paso_golpe` pregunta `c->golpe < GOLPES` antes de poner la cola?

---

## Parte B — El salto esquiva el tajo

### TODO 5 — Elegir el umbral con la traza

Compila y ejecuta `traza_23_salto.c` (comando en su cabecera). Anota: cuántos
cuadros está Tarou en el aire (25), su altura máxima (39 px) y cuántos cuadros
supera 12 px (20). El tajo del ninja tiene la caja activa 8 cuadros
(`NINJA_GOLPE_DESDE`..`NINJA_GOLPE_HASTA`), y Tarou pasa 20 cuadros por encima
de 12 px: saltar a tiempo esquiva, saltar tarde no. Ese es el margen de
destreza que buscas; con 4 px el salto casi siempre esquivaría, con 30 casi
nunca.

### TODO 6 — La prueba que falla

Desde la rama de partida (`git switch demo-secuencia-arranque`,
`git switch -c parte-b`), al final de `pruebas_combate` añade un bloque de
**tres casos** para un ninja que da un tajo completo a un Tarou al que **pones
a mano** `c.z` (en 12.4: píxeles × 16):

| Caso | `c.z` | Qué pasa | Qué compruebas |
|---|---|---|---|
| 0 | 20 px durante todo el tajo | pasa por debajo | `vida == VIDA_TAROU` **y** `n[0].conecto == 0` |
| 1 | 4 px durante todo el tajo | le da | `vida == VIDA_TAROU - NINJA_DANIO` |
| 2 | 20 px y `c.z = 0` en el cuadro `NINJA_GOLPE_DESDE + 3` | aterriza con el tajo activo y le da | `vida == VIDA_TAROU - NINJA_DANIO` |

(En el caso 2, al esquivar el tajo **no se gasta**: eso es lo que distingue
«pasa por debajo» de «falla».) Usa números literales en la prueba: la constante
nueva todavía no existe y un error de compilación no te dice por qué debe
fallar.

Rojo esperado: solo el caso 0, `563 pruebas, 1 fallos`
(`capturas/pruebas_parteB_1_falla.txt`):

```text
FALLO prueba_combate.c:413: c.vida == VIDA_TAROU && n[0].conecto == 0
```

### TODO 7 — El código

En `combate.h`: `#define COMBATE_ESQUIVE 12` (px). En
`combate_ninjas_resolver` (`combate.c`) calcula **una vez, antes del bucle**
`esquiva = mundo_altura(c) >= COMBATE_ESQUIVE` y añade `!esquiva &&` a la
condición del `if` del bucle, delante de `ninja_golpe_activo(n)`. Dos razones
de oficio: `mundo_altura` es una llamada y no debe repetirse tres veces por
frame, y como `conecto` **solo** se pone dentro del `if`, el tajo no se gasta
mientras Tarou está arriba.

Verde: `563 pruebas, 0 fallos` (`capturas/pruebas_parteB_2_solucion.txt`);
compila la ROM como en el TODO 4 (`capturas/compilacion_rom_parteB.txt`).

---

## Criterio de aprobación

1. **Parte A:** las tres salidas de `pruebas.exe` (rojo con tu prueba nueva,
   rojo distinto con solo `GOLPES 4`, verde con todo al día: `567 pruebas, 0
   fallos` si escribes las mismas comprobaciones; el recuento exacto depende de
   las tuyas, pero **0 fallos** no), y `No warnings or errors found.` de la ROM.
2. **Parte B:** la salida roja (un fallo, el del caso 0), la verde (`563
   pruebas, 0 fallos`) y la ROM compilada.
3. Responde por escrito las tres preguntas de la parte A.

Al terminar, deja los cambios en tu rama del clon (no en el árbol principal).
Las dos soluciones están en `solucion/` como parches de `git diff`
(`ejercicio_23_parteA.patch`, `ejercicio_23_parteB.patch`); **míralas después**.
Para aplicarlos sobre un clon limpio en Windows (los archivos del clon tienen
saltos de línea CRLF y el parche LF):

```text
git apply --ignore-whitespace E:\ProyectoSNES\aprendizaje\unidad-23-combate\solucion\ejercicio_23_parteA.patch
```

Verificado el 2026-10-26 en un segundo clon limpio: la A da `567 pruebas, 0
fallos`, la B `563 pruebas, 0 fallos`, y sin parche `560 pruebas, 0 fallos`.

## Para ir más lejos (sin solución)

- **El empujón que saca al ninja de la caja** (README, «Una observación»): ¿cómo
  lo arreglarías sin tocar la parada de impacto? Opciones a comparar:
  `Tarou` da un paso hacia delante en cada golpe (pero está «quieto mientras
  golpea», y los pasos los rompe la prueba de «Quieto mientras golpea»), el
  empujón sobre el ninja es más corto en el golpe 1 y 2 y largo solo en el
  derribo (`ninja_recibir` ya recibe `derribo`), o el ninja no retrocede cuando
  hay otro golpe en cola. Escribe primero la prueba: una partida con
  `ninja_actualizar` **sí llamado** y el ninja libre, que exija que los 3 (o 4)
  golpes conecten.
- Un ataque aéreo (spec §5.3 «A en el aire; derriba»): `mundo_actualizar` no
  empieza golpe en el aire; ¿qué cambia en `paso_golpe` si el golpe se da con
  `z > 0` y la gravedad debe seguir actuando?
