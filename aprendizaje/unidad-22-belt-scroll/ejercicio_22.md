# Ejercicio 22 (juego real) — Salto de altura variable

**Qué vas a hacer:** hoy el salto de Tarou mide siempre lo mismo (39 px). Vas a
hacer que **mantener B mientras sube** lo alargue hasta ~51 px, y que tocar B
siga dando el salto de siempre. Con **TDD**: primero la prueba (que falla),
luego el código (que la hace pasar), luego medir.

La gravedad ya es una resta en `mundo.c` (unidad 10 aplicada a z); la regla es
«mientras `vz < 0` (subiendo) y B está mantenida, tira de Tarou hacia abajo con
menos fuerza».

## Preparación (en un clon, nunca en el árbol principal)

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio
git -C E:\ProyectoSNES_trabajo\mio checkout -q 641d7c0
```

(Rama actual; esta unidad enseña el beat 'em up, no el tag `plataformas-r2`.)
Los comandos van en **PowerShell**, desde la raíz del clon. Antes de tocar nada,
comprueba que el punto de partida está en verde:

```
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

Debe decir `560 pruebas, 0 fallos`.

## Pasos

1. **TODO 1 — La prueba que falla.** En `tests/prueba_mundo.c`, sobre
   `preparar(...)`, añade un ayudante que salte una vez y devuelva la altura
   máxima y los cuadros que duró: con un parámetro `mantener` (0 o 1) que se
   copia a `m.saltar` (B **mantenida**; el despegue sigue usando
   `m.saltar_pulsado = 1` en el primer cuadro y 0 en los demás). Al final de
   `pruebas_mundo`, tres `COMPROBAR`:
   - tocando B, la altura máxima sigue entre 30 y 40 px;
   - manteniéndola, entre 48 y 56;
   - y aterriza más tarde (`cuadros_largo > cuadros_corto`).

   Compila y ejecuta. Deben fallar **2** (los del salto largo), y el total ser
   **563 pruebas, 2 fallos**. Guarda esa salida.

2. **TODO 2 — La constante.** En `src/juego/mundo.h`, junto a `GRAVEDAD`, una
   `GRAVEDAD_SUBIDA_B` menor. Prueba con 6. Antes de ejecutar nada, **predice**
   la altura: en la parte 1 de la demo, la altura es la suma de `−vz` en cada
   cuadro, con `vz = −96, −96 + g, ...` hasta llegar a 0 (`g` es la gravedad de
   cada cuadro), más el 1 del despegue, dividida entre 16. Con `g = 8` sale
   `8·(12+11+...+1) = 624`, +1, /16 = 39. ¿Y con `g = 6`?

3. **TODO 3 — La regla.** En `mundo_actualizar` (paso B4), la línea que suma la
   gravedad pasa a elegir cuál. Una sola línea: si **B está mantenida
   (`m->saltar`)** y **todavía sube (`c->vz < 0`)** usa `GRAVEDAD_SUBIDA_B`; si
   no, `GRAVEDAD`. Con comentario en español (regla del proyecto: sin `?:` con
   cadenas, pero con números no hay problema).

4. **TODO 4 — En verde.** Vuelve a compilar y ejecutar las pruebas. Deben ser
   **563 pruebas, 0 fallos**. Fíjate en que la prueba antigua «En el aire no se
   salta otra vez» (`c.vz == vz_antes + GRAVEDAD`) **sigue pasando**: ¿por qué?
   (Pista: mira qué campo del `Mando` pone esa prueba y cuál lee tu regla.)

5. **TODO 5 — Medirlo.** Escribe un programa corto (en una carpeta aparte, no
   en el clon) que haga el salto con `mantener = 0` y con `mantener = 1` y
   escriba la altura, los cuadros hasta aterrizar y el primer cuadro en que
   `vz >= 0`. Compílalo contra el `mundo.c` del clon y contra el del árbol
   principal (la solución de referencia lo hizo así). Esperado: sin mantener,
   39 px y 26 cuadros en las dos versiones; manteniendo, 39 px con el
   original y **51 px, 31 cuadros** con el cambio.

6. **TODO 6 — Que el juego compile.** Compila la ROM del clon con el entorno de
   la unidad 03:

   ```
   $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
   ```

   Debe decir `No warnings or errors found.` (Un cambio de `mundo.h` no se
   propaga solo a los `.c` que lo incluyen: si el clon ya tenía objetos
   compilados, haz antes `make clean`.) Si quieres verlo jugado, abre el
   `ProyectoSNES.sfc` del clon en Mesen: el HUD (SELECT) enseña `Z` y `VZ`.
   **La solución de referencia NO se probó en Mesen**, solo se compiló.

7. **Pregunta de diseño (sin código).** Con tu regla, quien mantiene B todo el
   salto salta siempre ~51 px y quien la suelta al instante, 39. En la unidad 10
   el corte era continuo (soltar B recortaba `vy`). ¿Qué haría falta para
   que soltar B a mitad de subida dé una altura intermedia? (Pista: no basta
   `m->saltar`; hay que acordarse de si ya se soltó.) Y, ya que el combate no
   mira z (`combate.c` no usa `z`): ¿conviene que Tarou pueda golpear a un
   ninja mientras está a 51 px del suelo? Es una pregunta para la unidad 23.

## Criterio de aprobación

- La salida roja del paso 1: `563 pruebas, 2 fallos` con los dos `FALLO` de
  `prueba_mundo.c` sobre `largo` y `aire_largo`.
- La salida verde del paso 4: `563 pruebas, 0 fallos`.
- La medida del paso 5: `mantener=1 altura=51 cuadros=31`.
- La ROM del clon compila sin avisos.

## Solución

`solucion/ejercicio_22.patch` (un `git diff` de `src/` y `tests/` sobre el
commit `641d7c0`; se aplica con `git apply`). La salida de referencia está en
`capturas/` (`pruebas_rojo.txt`, `pruebas_solucion.txt`,
`medida_salto_original.txt`, `medida_salto_solucion.txt` y
`compilacion_rom_clon.txt`). Mírala **después** de intentarlo.
