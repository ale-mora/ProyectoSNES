# Ejercicio 28 (juego real, con TDD) — «READY?» al empezar la fase

**Qué vas a hacer:** al entrar en la fase, la pantalla dice **READY?** 2 s con el mundo
quieto; nada lo corta (Start no pausa) y luego se juega. Es un estado nuevo de la máquina de
la partida (`estado_fase.c`), con la prueba escrita **primero**. Verás también qué pasa con
las pruebas que suponían la regla vieja, y por qué la ROM autojugable necesita un trato aparte.

Trabaja en un **clon** de la rama actual (nunca en el árbol principal):

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u28
git -C E:\ProyectoSNES_trabajo\u28 checkout -q 6845b5f
```

El entorno de PowerShell es el de las unidades 24 y 25 (`C:\msys64\usr\bin` para las pruebas;
`PVSNESLIB_HOME` del repo principal para la ROM).

## Punto de partida

```text
make -C tests ; .\tests\pruebas.exe        →   2477 pruebas, 0 fallos
```

## La regla

1. `estado_fase_iniciar` deja la partida en un estado nuevo, `FASE_PREPARADOS`.
2. Dura `FASE_PREPARADOS_FRAMES` = 120 frames (2 s) y pasa a `FASE_JUGANDO`.
3. Durante él: el mundo **no** avanza, **no** hay media luz, y Start, Select, un KO o una
   fase vencida **no** hacen nada (devuelve siempre `ACCION_NADA`).

## Pasos

1. **TODO 1 — La prueba primero.** Al final de `pruebas_estado_fase` (`tests/prueba_estado_fase.c`)
   escribe un bloque que, tras `estado_fase_iniciar(&e)`, compruebe: el estado es
   `FASE_PREPARADOS`; el mundo no avanza y no hay media luz; durante 119 frames **con todo
   pulsado** (`estado_fase_actualizar(&e, 1, 1, 1, 1)`) la acción es siempre `ACCION_NADA` y
   sigue en `FASE_PREPARADOS` (acumula las acciones con `|`: si alguna no es 0, se nota); y
   en el frame 120 pasa a `FASE_JUGANDO` con el mundo avanzando.

2. **TODO 2 — Solo los nombres.** No compila: faltan `FASE_PREPARADOS` y
   `FASE_PREPARADOS_FRAMES`. Añádelos a `src/juego/estado_fase.h`: el estado **al final** del
   `enum` (después de `FASE_STAGE_CLEAR`: así no cambia el número de los otros) y la constante
   junto a las demás. **Nada más.** Ejecuta: deben salir **4 fallos**, todos de tu bloque.
   ¿Por qué ninguno de las pruebas viejas? (Todavía no has cambiado ninguna regla.)

3. **TODO 3 — La regla.** En `estado_fase.c`: `estado_fase_iniciar` pone `FASE_PREPARADOS`,
   y un `case FASE_PREPARADOS` nuevo cuenta `reloj` y al llegar a 120 llama a
   `pasar(e, FASE_JUGANDO)`. Ponlo **antes** del `default` (que es el STAGE CLEAR: si no,
   ¿qué haría tu estado?). `estado_fase_mundo_avanza` y `estado_fase_atenuada` no se tocan:
   ¿por qué ya hacen lo que pide la regla? Ejecuta: tu bloque pasa, pero ahora fallan **513**
   de las **viejas** (una de ellas es un bucle de 500 frames y falla en cada uno).

4. **TODO 4 — Las pruebas viejas.** Léelas: todas empiezan con `estado_fase_iniciar(&e)` y
   suponen que ya se está jugando. Esa regla **era** verdad; ya no. No se borran: escribe una
   ayuda `static void jugando(EstadoFase *e)` que inicie y pase los 120 frames sin botones, y
   úsala en vez de `estado_fase_iniciar` en los bloques viejos (cuatro sitios; tu bloque nuevo
   sigue con `estado_fase_iniciar`). Debe quedar **2481 pruebas, 0 fallos** (2477 + tus 4).

5. **TODO 5 — El texto.** En `aviso()` de `src/pantallas/pantalla_fase1.c` hay un `if` por
   estado. Sin cambiar nada, ¿qué texto saldría durante `FASE_PREPARADOS`? (El del último
   `else`: «STAGE CLEAR». Un estado nuevo cae en el caso «por defecto» de todo el código que
   no lo conoce: búscalos.) Añade el caso con `"             READY?"`.

6. **TODO 6 — La ROM autojugable.** `-DGUION_FASE1` lee el mando de una **tabla por tiempo**
   (unidad 24). Con 2 s de READY? quieta, cada pulsación de la tabla llegaría 120 frames
   antes de lo que el guion esperaba. Tras `estado_fase_iniciar(&partida)` en
   `pantalla_fase1_iniciar`, añade bajo `#ifdef GUION_FASE1` un `partida.estado = FASE_JUGANDO;`
   con su comentario. (Pregunta: ¿qué otra solución habría? Por ejemplo, que la tabla empiece
   con 120 frames sin teclas. ¿Cuál cuesta menos mantener?)

7. **TODO 7 — Compilar y verlo.** Has tocado un `.h`: `make clean` delante.

   ```text
   make clean ; make
   copy ProyectoSNES.sfc E:\ProyectoSNES_trabajo\u28_ready.sfc
   make clean ; $env:CFLAGS="-DGUION_FASE1" ; make ; $env:CFLAGS=$null
   make clean
   ```

   Y desde el repo principal, sin tocar el PC (el guion pulsa Start en el título, en el
   selector y **otra vez a los 55 s, durante el READY?**):

   ```text
   .\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\u28_ready.sfc -Duracion 59 -Desde 54 -Cada 1 -Espera 3 -Guion "50,50.3,Start;53,53.3,Start;55,55.3,Start"
   ```

   Debe verse READY? en las capturas de los 55 y 56 s y, en las siguientes, la fase **sin**
   PAUSE: el tercer Start no la pausó.

## Criterio de aprobación

- Las tres salidas de las pruebas: 4 fallos (TODO 2), 513 (TODO 3) y 2481 / 0 (TODO 4).
- Las dos ROM con `No warnings or errors found.`
- Las capturas: READY? y después la fase sin pausar.

## Solución

`solucion/ready.diff` (el cambio entero) y los archivos completos `estado_fase.h`,
`estado_fase.c` y `prueba_estado_fase.c`. Salidas reales en `capturas/`
(`pruebas_1_rojo.txt`, `pruebas_2_viejas_rojas.txt` —resumida—, `pruebas_3_verde.txt`,
`compilacion.txt`, `mesen_ready.png`, `mesen_despues_del_ready.png`, `mesen_secuencia.png`).
