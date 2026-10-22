/*---------------------------------------------------------------------------------
    fragmento_prueba_tarou.c (SOLUCIÓN de la práctica sobre el juego) — La prueba
    que se añade a tests/prueba_tarou.c. NO es un programa: es un trozo para pegar.

    PROPÓSITO
      Comprobar en el PC dos reglas de la máquina de estados de Tarou:
      (1) DESENVAINANDO ignora el mando (ni B ni Derecha hacen nada mientras
      dura la animación) y el cambio a PARADO llega el frame DESPUÉS de que la
      animación termine; (2) la espada es una variable aparte del estado:
      sigue fuera al pasar por PARADO, SALTANDO y CAMINANDO.

    DÓNDE SE PEGA (dos sitios, en tests/prueba_tarou.c de TU CLON o rama)
      1. La función, justo ANTES de la línea
             /* Punto de atención 3: R y A el mismo frame → gana R. */
         (así FRAMES_DESENVAINAR, preparar() y frames() ya están definidos).
      2. La llamada, en pruebas_tarou(), justo DEBAJO de
             prueba_prioridad_espada();
         Si se olvida, gcc avisa ("defined but not used") y la cuenta no sube.
      Lo mismo en formato diff: solucion/prueba_tarou.patch
      (git apply aprendizaje\unidad-02-maquinas-de-estado\solucion\prueba_tarou.patch).

    CÓMO SE COMPRUEBA
      $env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
      Antes: "367 pruebas, 0 fallos". Después: "374 pruebas, 0 fallos" (7 más).

    HARDWARE QUE TOCA
      ninguno (prueba de PC).

    VARIABLES: t (Tarou) y m (Mando) locales, como el resto de pruebas.
    → glosario: Estado, Espada, Máquina de estados
---------------------------------------------------------------------------------*/

/* ===== 1. Pegar antes de "Punto de atención 3" ===================================== */

/* Unidad 02 de aprendizaje/: la espada es una variable APARTE del estado. Sale en
   DESENVAINANDO (que ignora el mando: ni B ni Derecha hacen nada), y sigue fuera
   al pasar por PARADO, SALTANDO y CAMINANDO. Las cuentas de frames usan la copia
   fija tests/tarou_anim.h (desenvainar: 4 cuadros x 5 frames = 20). */
static void prueba_espada_persiste_entre_estados(void)
{
    Tarou t; Mando m;
    preparar(&t, &m, 100);
    m.espada_pulsado = 1; m.actividad = 1;
    frames(&t, &m, 1);                                  /* Frame 1: R.                    */
    COMPROBAR(t.estado == ESTADO_DESENVAINANDO);
    m.espada_pulsado = 0;
    m.saltar_pulsado = 1; m.saltar = 1; m.derecha = 1;  /* B y Derecha mientras saca...   */
    frames(&t, &m, FRAMES_DESENVAINAR - 1);             /* ...hasta el frame 20.          */
    COMPROBAR(t.estado == ESTADO_DESENVAINANDO && t.en_suelo == 1 && t.x_fp == PF(100));
    frames(&t, &m, 1);                                  /* Frame 21: termina la animación:*/
    COMPROBAR(t.estado == ESTADO_PARADO && t.espada == ESPADA_FUERA); /* PARADO, sin saltar.*/
    frames(&t, &m, 1);                                  /* Frame 22: ahora sí lee B.      */
    COMPROBAR(t.estado == ESTADO_SALTANDO);
    COMPROBAR(t.espada == ESPADA_FUERA);                /* En el aire, sigue fuera.       */
    m.saltar_pulsado = 0;
    frames(&t, &m, 48);                                 /* Aterriza (47 frames de salto)  */
    COMPROBAR(t.en_suelo == 1 && t.estado == ESTADO_CAMINANDO);  /* con Derecha: camina...*/
    COMPROBAR(t.espada == ESPADA_FUERA);                /* ...y con la espada fuera.      */
}

/* ===== 2. Pegar en pruebas_tarou(), debajo de prueba_prioridad_espada(); =========== */

    prueba_espada_persiste_entre_estados();
