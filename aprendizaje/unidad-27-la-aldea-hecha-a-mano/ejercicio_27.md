# Ejercicio 27 — Tiles con volteos (PC, TDD) y un farol más en la aldea (clon)

## Parte A — Un juego de tiles con volteos, con las pruebas primero

**Qué vas a hacer:** la pieza que más ahorra en la conversión de la aldea es «este tile ya lo
tengo, quizá volteado». La vas a escribir en Python puro (sin numpy), **empezando por las
pruebas**, en esta misma carpeta.

1. **TODO 1 — Las pruebas.** Crea `prueba_27_tiles.py` junto a `ejercicio_27_tiles.py`, con
   `unittest`. Importa el ejercicio así, para que la misma prueba sirva después contra la
   solución:

   ```python
   import importlib, os, sys
   sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
   et = importlib.import_module("ejercicio_27_tiles")
   ```

   Escribe una función de ayuda que convierta 8 cadenas de 8 dígitos hexadecimales en la tupla
   de 64 índices (más fácil de leer que 64 números). Prueba, como mínimo:
   - `voltear_h` y `voltear_v` con un tile de un solo píxel en una esquina (¿adónde va?);
   - que voltear dos veces deja el tile como estaba;
   - que `buscar` del tile vacío da `(0, 0, 0)` y no añade nada;
   - que un tile repetido da el mismo número y no se añade otra vez;
   - que su espejo horizontal da `(n, 1, 0)`, el vertical `(n, 0, 1)` y los dos `(n, 1, 1)`;
   - que si lo primero que se guardó fue el **espejado**, el original sale con `h = 1`;
   - que la misma imagen con otra paleta es otro tile;
   - que un tile **simétrico** (igual a su espejo) sale sin volteo.

2. **TODO 2 — Verlo fallar.**

   ```text
   python -m unittest prueba_27_tiles
   ```

   Con el esqueleto, la mayoría falla (con las 9 pruebas de la solución, **7**). ¿Cuáles
   pasan sin haber escrito nada y por qué? (El esqueleto devuelve el tile sin voltear: voltear
   dos veces «funciona»; y como añade siempre uno nuevo, dos paletas dan números distintos.)

3. **TODO 3–7 — Completarlo** siguiendo los TODO de `ejercicio_27_tiles.py`. Todas en verde.
   Compara después con `Juego.buscar` de `scripts/preparar_aldea.py`: ¿qué hace igual y qué
   usa allí en vez de tuplas? (Arrays de numpy de 8×8: el volteo es `m[:, ::-1]`.)

## Parte B — Un farol más (juego real, en un clon)

**Qué vas a hacer:** añadir un farol al primer plano, regenerar la aldea, leer cuánto ha
costado en tiles y ver en Mesen que tapa a Tarou.

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u27
git -C E:\ProyectoSNES_trabajo\u27 checkout -q 6845b5f
```

El arte de Pixellab **no está en git** (pesa y es de origen): cópialo al clon, o el script
se para con «faltan piezas»:

```text
xcopy /E /I E:\ProyectoSNES\res\descargas\graficos\pixellab\aldea E:\ProyectoSNES_trabajo\u27\res\descargas\graficos\pixellab\aldea
```

4. **TODO 8 — Regenerar sin cambios.** En el clon: `python scripts/preparar_aldea.py` (unos
   20 s). Apunta la línea que escribe (tiles, **juntados**, puestos, humo). Luego `git status`:
   no debe cambiar **ningún** archivo. ¿Por qué es importante que el script sea determinista?
   (Los `.bin` están versionados: si cada ejecución diera bytes distintos, cada commit
   llevaría 32 KB de cambios falsos.)

5. **TODO 9 — El farol.** En `scripts/preparar_aldea.py`, añade `300` a la lista `FAROLES`
   (déjala ordenada). ¿Por qué 300 y no 296 o 304? (Los cuatro faroles que ya hay están en x
   ≡ 4 módulo 8: el mismo farol en la misma posición dentro de sus tiles da los mismos tiles
   del poste.) Vuelve a ejecutar el script. ¿Cuántos tiles se juntaron ahora? Abre
   `res/aldea/cerca_vista.png` en x ≈ 300: ¿se nota algo raro donde el script juntó tiles?

6. **TODO 10 — Las cuentas.** El farol nuevo cae sobre una fachada en una posición donde no
   había farol: los tiles de esas celdas (poste + fachada) son **nuevos**. Explica por qué
   «juntados» sube en vez de subir «tiles» (que sigue en 512). ¿Qué pasaría si lo pones en
   x = 196 + 640 = 836 (la misma fachada, 640 px después)? Pruébalo si quieres. Pistas: la
   `SECUENCIA` se repite cada 640 px, así que la parte de fachada sale gratis; pero la calle
   de debajo se repite cada 256 px (128 y su espejo) y 640 no es múltiplo de 256: las celdas
   del pie del poste sí serán nuevas.

7. **TODO 11 — Compilar y verlo.**

   ```text
   make clean ; $env:CFLAGS="-DGUION_FASE1" ; make ; $env:CFLAGS=$null
   copy ProyectoSNES.sfc E:\ProyectoSNES_trabajo\u27_farol.sfc
   make clean
   ```

   Y desde el repo principal, sin tocar el PC:

   ```text
   .\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\u27_farol.sfc -Duracion 6 -Cada 1 -Espera 1
   ```

   Hacia los 3–4 s Tarou pasa por x = 300. Busca la captura en la que el farol nuevo lo tapa
   y mira la **franja**: la parte de fachada que cae en las celdas del poste también tapa a
   Tarou. ¿Por qué? (La prioridad es por celda de 8×8.)

## Criterio de aprobación

- Parte A: la salida en rojo (7 fallos con 9 pruebas, o los tuyos) y la salida en verde.
- Parte B: `git status` limpio tras regenerar sin cambios; la línea del script con el farol
  (juntados **113**, antes 89); la ROM con `No warnings or errors found.`; una captura con el
  farol nuevo tapando a Tarou.

## Solución

- Parte A: `solucion/ejercicio_27_tiles.py` y `solucion/prueba_27_tiles.py`
  (`python -m unittest solucion.prueba_27_tiles` desde la carpeta de la unidad).
- Parte B: `solucion/farol.diff` (una línea). Salidas reales: `capturas/preparar_antes.txt`,
  `preparar_con_farol.txt`, `compilacion.txt`, `mesen_farol_nuevo.png` y
  `mesen_farol_tapa_a_tarou.png` (ampliada).
