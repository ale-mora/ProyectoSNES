# Pixellab → Bárbara (segunda protagonista, Revisión 2)

Misma mecánica que `../tarou/README.md`: deja aquí las animaciones exportadas
desde Pixellab (git las ignora) y ejecuta, desde la raíz del repositorio:

```
python scripts/preparar_personaje.py barbara
```

Salida: `res/personajes/barbara/barbara_<animación>.png` y
`res/personajes/barbara/barbara_anim.h`. Hasta la Revisión 2 el juego no usa
estos archivos: en la secuencia de arranque Bárbara sigue siendo el sprite de
la samurái del pack (`res/personajes/barbara_samurai.png`).

## Nombres de archivo

Usa los mismos nombres base que Tarou (`reposo`, `caminar`, `correr`, `saltar`,
`caer`, `agachado`, `agachado_caminar`, `desenvainar`, `envainar`, `atacar`,
`echarse`, `dormido`, `despertar`). Si las mecánicas de Bárbara terminan siendo
distintas, la lista se amplía en el diseño de la Revisión 2; el script acepta
cualquier nombre que esté en su tabla de animaciones.

## Formatos y reglas

Los mismos del README de Tarou: tira PNG, carpeta de cuadros o GIF; 64×64 por
cuadro, mirando a la derecha, máximo 15 colores + transparente con una sola
paleta, máximo 15 cuadros por animación.
