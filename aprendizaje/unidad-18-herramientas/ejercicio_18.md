# Ejercicio 18 — Tarou en guardia: una animación nueva de principio a fin

**Qué se quiere:** cuando Tarou está **quieto con la espada fuera**, que se
dibuje con la pose `Fight_Pose` de Pixellab (la espada en la boca) en vez del
reposo, en el que la espada no aparece. Es una tira nueva, `guardia`, de **un
cuadro**. Recorre la tubería entera: prueba → script → tira → `Makefile` →
`data.asm` → `tarou_sprite.c` → ROM → Mesen.

**Guardia no es un estado nuevo.** La lógica (`tarou.c`) sigue en
`ESTADO_PARADO` con `espada == ESPADA_FUERA`: solo cambia **lo que se
dibuja**. Por eso no se toca `animaciones.c` ni el `enum Estado` (pregunta 3).

Trabaja en un **clon** (los cambios no van a la rama principal; `PLAN.md` §2):

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mi_u18
cd E:\ProyectoSNES_trabajo\mi_u18
```

## Paso 1 — La prueba primero (TDD, en rojo)

En `scripts/pruebas/prueba_preparar_personaje.py`:

1. `test_marcador_cuando_falta_animacion`: ahora salen **15** PNG (14
   animaciones + la estática); añade
   `self.assertEqual(cuadros_del_header(header, "guardia"), 1)`; y
   `TAROU_ANIMACIONES` pasa a **14** (cámbialo también en
   `test_reposo_estatico_siempre_dos_cuadros`).
2. `PruebasPixellabNativo.setUp`: añade una pose fija falsa de 50×45 px:
   `rotacion_pixellab(self.entrada, "Fight_Pose", "east", ancho=50, alto=45)`.
3. `test_cuadros_del_mapeo`: añade `"guardia": 1` al diccionario `esperado`.
4. Una prueba nueva en `PruebasPixellabNativo`,
   `test_guardia_es_la_pose_fija`: abre `tarou_guardia.png` y comprueba que
   mide 96×64 (un cuadro), que su caja opaca (`caja_opaca`) mide 50×45 (no se
   escaló), que acaba en la fila 64 (pies abajo) y que está centrada
   (`(b[0] + b[2]) // 2 == 48`).

Ejecuta `python -m unittest scripts/pruebas/prueba_preparar_personaje.py`.
**Tiene que fallar**: 4 de 12 (3 `FAIL` y 1 `ERROR`, el de la prueba nueva,
porque `tarou_guardia.png` no existe). Si no falla nada, la prueba no mide el
cambio. Referencia: `capturas/pruebas_en_rojo.txt`.

## Paso 2 — El script (en verde)

En `scripts/preparar_personaje.py`:

- `ANIMACIONES`: añade `("guardia", 1)` al final (el 1 son los cuadros del
  marcador si falta el arte).
- `MAPEOS_PIXELLAB["tarou"]`: `"guardia": ("Fight_Pose", "east", None, None, False)`.
  `None` como primer cuadro = pose fija (`<estado>/rotations/<dirección>.png`).
- Actualiza los comentarios que dicen «13 animaciones».

Las cuatro baterías de pruebas (el comando de la sección «Pruebas» del README)
tienen que dar **`Ran 36 tests` ... `OK`**.

## Paso 3 — Ejecutar el script

```
python scripts/preparar_personaje.py tarou --entrada E:\ProyectoSNES\res\descargas\graficos\pixellab\tarou
git status --short
```

(`--entrada` porque la exportación de Pixellab no está en git y el clon no la
tiene.) Esperado: la lista termina en `tarou_guardia.png  1 cuadros
(pixellab)` y `git status` muestra **solo** `tarou_anim.h` modificado y
`tarou_guardia.png` nuevo (más los archivos que ya editaste). Salida de
referencia: `capturas/salida_preparar_personaje.txt`.

## Paso 4 — El `Makefile`

Añade `guardia` al final de `TAROU_ANIMS`. Nada más: la regla de patrón
convierte la tira y `TAROU_PICS` entra en `ASSETS` (y por tanto en
`data.obj: $(ASSETS)`) y en `tarou_sprite.obj: $(TAROU_PICS)`.

## Paso 5 — `data.asm`

1. Una sección nueva detrás de `.rodata_tarou_despertar`, igual que las demás:
   `.section ".rodata_tarou_guardia" superfree`, la etiqueta
   `tarou_guardia_til:` con su `.incbin`, `tarou_guardia_til_end:` y `.ends`.
2. En `tarou_til_bancos`, **al final**, `.db :tarou_guardia_til`: será el
   índice 13, el siguiente al último estado.

## Paso 6 — `src/juego/tarou_sprite.c`

1. `#include "tarou_guardia_meta.inc"` (lo genera `gfx4snes -T`) y
   `extern char tarou_guardia_til;`.
2. `#define DIBUJO_GUARDIA ESTADO_CANTIDAD` (13): el número de la tira.
3. Un `case DIBUJO_GUARDIA:` en `tarou_sprite_tabla` (devuelve
   `tarou_guardia_metasprites[cuadro]`) y otro en `tarou_sprite_tiles`
   (devuelve `(u8 *)&tarou_guardia_til`).
4. Dos `static u8` nuevas, `dibujo_estado` y `dibujo_cuadro`, y una función
   `static void elegir_dibujo(const Tarou *t)`: si `t->estado ==
   ESTADO_PARADO && t->espada == ESPADA_FUERA`, guardia y cuadro 0; si no, el
   estado y el cuadro de `t`.
5. En `tarou_sprite_al_dia` y en el paso 1 de `tarou_sprite_dibujar`, llama a
   `elegir_dibujo(t)` y usa `dibujo_estado`/`dibujo_cuadro` donde ahora pone
   `t->estado`/`t->cuadro` (también para `tarou_til_bancos[...]`).
6. Actualiza la cabecera del archivo (las 14 tiras, la variable nueva).

Reglas de tcc816 (CLAUDE.md): `u8`, nada de `int`; las `static` se
inicializan antes de leerse (aquí `elegir_dibujo` las escribe siempre antes).

## Paso 7 — Compilar

Desde PowerShell, en el clon:

```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make clean; make
Select-String -Path ProyectoSNES.sym -Pattern tarou_guardia_til
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

Esperado: `No warnings or errors found.`, la etiqueta en el `.sym` y
`367 pruebas, 0 fallos`.

## Paso 8 — Verlo en Mesen

```
& "E:\ProyectoSNES\tools\mesen\Mesen.exe" "E:\ProyectoSNES_trabajo\mi_u18\ProyectoSNES.sfc"
```

Start (tecla D) en el título; en el nivel, quieto, R (tecla W): desenvaina
(unos 0,75 s) y se queda **en guardia**. Camina (flechas): animación de
caminar. Para: guardia otra vez. R: envaina y vuelve el reposo. Sin tocar nada
10 s se echa a dormir como siempre.

Sin manos: la ROM de prueba (`make clean; $env:CFLAGS="-DGUION_PRUEBA"; make;
$env:CFLAGS=$null`) pasa por la guardia sola: el guion desenvaina contra el
tronco en el cuadro 1772 y ataca en el 1872, así que entre los cuadros ~1817
y 1872 del nivel (unos 30,3–31,2 s) Tarou está en guardia.

## Paso 9 — Preguntas (respuestas en `solucion/respuestas.md`)

1. ¿Por qué hace falta la línea `.db :tarou_guardia_til` si la dirección de la
   tira ya está en `tarou_sprite_tiles`?
2. Tras el paso 4, ¿por qué un `make` sin `clean` ya recompilaría
   `tarou_sprite.c` y reensamblaría `data.asm`? ¿Y por qué se pide `make
   clean` de todos modos?
3. Si guardia fuera un **estado** de verdad (`ESTADO_GUARDIA`), ¿qué más
   habría que tocar? Nombra archivos.
4. La tira tiene 1 cuadro de 96×64 (6 piezas de 32×32 = 3 KB). ¿Por qué la
   sección ocupa 4 KB en la ROM (`$85:F000`..`$86:0000` en el `.sym` de
   referencia)?
5. ¿Qué pasaría si `elegir_dibujo` devolviera `t->cuadro` también para la
   guardia?
