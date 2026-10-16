# Pixellab → Tarou adulto (protagonista, Revisión 1)

Deja en **esta carpeta** las animaciones exportadas desde Pixellab. Git ignora
todo lo que pongas aquí salvo este README (son descargas, como el resto de
`res/descargas/`). Después ejecuta, desde la raíz del repositorio:

```
python scripts/preparar_personaje.py tarou
```

El script genera `res/personajes/tarou/tarou_<animación>.png` (tiras indexadas
con una sola paleta) y `res/personajes/tarou/tarou_anim.h` (cuántos cuadros
tiene cada animación). Si una animación falta, pone un **marcador** numerado en
su lugar, así el juego compila igual. Cuando la descargues, vuelve a ejecutar el
script y `make`: el arte entra sin tocar código.

## Nombres de archivo (exactos, en minúsculas)

| Animación | Nombre base | Qué muestra | Cuadros sugeridos |
|---|---|---|---|
| reposo | `reposo` | parado, respirando | 4 |
| caminar | `caminar` | ciclo de caminar | 8 (o 6) |
| correr | `correr` | ciclo de correr | 8 (o 6) |
| saltar | `saltar` | subida del salto | 2 |
| caer | `caer` | caída y aterrizaje | 2 |
| agachado | `agachado` | agachado, quieto | 2 |
| agachado_caminar | `agachado_caminar` | caminar agachado | 6 |
| desenvainar | `desenvainar` | sacar la espada | 4 |
| envainar | `envainar` | guardar la espada | 4 |
| atacar | `atacar` | tajo con la espada | 5 |
| echarse | `echarse` | tumbarse para dormir | 4 |
| dormido | `dormido` | dormido, en bucle | 4 |
| despertar | `despertar` | levantarse | 4 |

Máximo **15 cuadros** por animación (límite de un banco de ROM de 32 KB).

## Formatos aceptados (uno por animación)

- **Tira horizontal PNG**: `reposo.png` con los cuadros uno al lado del otro,
  todos de 64×64. Es el formato preferido («sprite sheet» de una fila).
- **Carpeta con cuadros sueltos**: `reposo/01.png`, `reposo/02.png`… (PNG de
  64×64, en orden alfabético).
- **GIF animado**: `reposo.gif` (se extraen sus cuadros).

## Cómo exportar desde Pixellab

En la herramienta *Characters*, cada animación se puede exportar como **sprite
sheet** o como **cuadros sueltos** (ver su documentación:
https://www.pixellab.ai/docs/ways-to-use-pixellab). Elige sprite sheet de una
sola fila si existe la opción; si solo exporta una cuadrícula de varias filas o
cuadros sueltos, descárgalos tal cual y usa el formato de carpeta.

## Reglas que impone la SNES (el script avisa si no se cumplen)

- Cada cuadro de **64×64 px**, el personaje mirando **a la derecha** (la
  izquierda se obtiene por espejo en el hardware).
- **Máximo 15 colores + transparente** y **una sola paleta** para todo el
  personaje. El script reduce colores si hace falta, pero cuantos menos use el
  arte, mejor se verá; evita degradados y tonos casi iguales.
- Fondo **transparente** (alfa 0 o 255, sin semitransparencias), sin sombra
  proyectada ni suelo.
- **Caminar en el sitio**: el cuerpo no avanza dentro del cuadro; el
  desplazamiento lo pone el código.
- Las patas tocan **la misma fila** en todas las animaciones; el script alinea
  todos los cuadros a los pies, pero necesita que el arte sea coherente.
- Bucle limpio: el último cuadro no repite el primero.
