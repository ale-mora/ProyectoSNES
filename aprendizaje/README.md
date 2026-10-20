# `aprendizaje/` — aprender a programar la SNES con este juego

Esta carpeta es el curso para quien hereda el proyecto: unidades de estudio
que van de C en el PC hasta el ensamblador del 65816 y del SPC700, cada una
anclada en el código real de *Tarou: A Dog's Journey*.

- **El plan completo** (las 21 unidades, su formato, dependencias, recetas
  verificadas y estado): [`PLAN.md`](PLAN.md).
- **El vocabulario** común: [`../docs/glosario-snes.md`](../docs/glosario-snes.md).
- **La documentación técnica** del juego, que las unidades resumen y citan:
  [`../docs/README.md`](../docs/README.md).

## Qué hay hoy (2026-10-20, unidad 00 escrita)

El juego que enseñan las unidades es el de la **Revisión 2**: el nivel 1 es
un tutorial de movimiento de 2048 px con colisión contra el escenario,
dibujado en Tiled y verificado con un guion que lo juega solo
(`docs/diseno-tutorial-movimiento.md`). `PLAN.md` está al día con ese código.

| Carpeta | Contenido |
|---|---|
| `unidad-00-c-en-el-pc/` | Unidad 00: C en el PC (lógica, funciones, arrays, structs y punteros). Cuatro demos, cuatro ejercicios, sus soluciones y las salidas de referencia. Incluye lo que antes era `etapa0/`. |
| `unidad-NN-tema/` | Las demás, por escribir. Se crean una por sesión, en el orden de `PLAN.md` §4. |

## Para el alumno

1. Empieza por la unidad 00 y sigue el orden de `PLAN.md` §4.
2. En cada unidad: lee el `README.md`, ejecuta la demo, resuelve el
   ejercicio, compáralo con `solucion/` y cumple el criterio de aprobación.
3. Si un término no está definido en la unidad, búscalo en el glosario.

## Para la sesión que escribe las unidades

Una unidad por sesión (las 00, 01 y 02 pueden ir de dos en dos). Pega esto
al empezar, cambiando el número:

```
Escribe la unidad NN de aprendizaje/ siguiendo aprendizaje/PLAN.md: su ficha
en §3, el formato de §2 y el procedimiento de §6 (definición de «hecho»,
convenciones, receta de mini-ROM, receta del .asm, comandos y trampas
conocidas). Antes de escribir, relee los archivos de «Código del juego» de
la unidad; si algo cambió, manda el código y corrige el plan. Ejecuta el
criterio de aprobación de verdad con la solución, guarda las capturas en la
unidad, marca la tabla de §7 y haz un commit sin push.
```

Al terminar cada unidad, la tabla de estado de `PLAN.md` §7 dice qué falta.
