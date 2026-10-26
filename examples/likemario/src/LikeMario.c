/*---------------------------------------------------------------------------------
    Simple game with famous hero ;) and map and sprite engine demo
    -- alekmaul
---------------------------------------------------------------------------------*/
/*=====================================================================================
    GUIA PEDAGOGICA DE ESTE ARCHIVO (LikeMario.c)
    =====================================================================================

    Este archivo es el ejemplo "LikeMario" incluido en PVSnesLib: un mini plataformas
    al estilo Super Mario World que sirve para mostrar, en un solo programa, las tres
    piezas grandes de cualquier juego de SNES hecho con esta librería:

      1) El motor de MAPA (map.h / bgXxx de background.h): dibuja y desplaza (hace
         "scroll") un fondo (background/BG) mucho más grande que la pantalla, formado
         por "tiles" (mosaicos, normalmente de 8x8 u 16x16 pixeles) repetidos.
      2) El motor de OBJETOS (object.h): un sistema tipo "entidad + máquina de estados"
         genérico para manejar personajes/enemigos que se mueven, caen por gravedad y
         chocan contra el mapa.
      3) El motor de SPRITES dinámicos (sprite.h) y el motor de SONIDO (sound.h), que
         dibujan a Mario en pantalla (vía OAM, la memoria de sprites del hardware) y
         reproducen música/efectos a través del coprocesador de audio SPC700.

    Un dato clave para quien viene de programación "normal" (PC/web): la SNES es un
    ordenador de 1990 con muy poca RAM y sin sistema operativo. No hay "malloc" de
    objetos dinámicos con vida libre ni framebuffer de pixeles como en una GPU moderna.
    En vez de eso, el hardware de vídeo (la PPU) lee directamente de tres memorias muy
    pequeñas y especializadas:
       - VRAM  (Video RAM): guarda los tiles (dibujos de 8x8/16x16 px) y los mapas
         (qué tile va en cada casilla de la cuadricula del fondo).
       - CGRAM (Color Generator RAM): guarda las paletas de color (grupos de 16 o 256
         colores que los tiles/sprites usan como "libro de colores").
       - OAM   (Object Attribute Memory): guarda hasta 128 sprites con su posición,
         qué tile usan y sus "atributos" (flip, prioridad, paleta).
    PVSnesLib no reemplaza esas memorias: te da funciones en C que, por debajo, hacen
    las copias (DMA) y cálculos necesarios para rellenarlas correctamente. Casi todo
    este archivo consiste en "preparar datos en RAM normal" y luego "pedirle a la
    librería que los copie/actualice en la memoria de vídeo" en el momento adecuado
    del fotograma (frame).

    El "bucle de juego" (game loop) de la SNES corre a 60 fotogramas por segundo (NTSC).
    Cada fotograma se hace, en general: leer mando -> mover cosas -> dibujar -> esperar
    al VBlank (el breve instante en que el rayo del tubo de imagen "no está dibujando
    nada", y por tanto es seguro tocar VRAM/OAM/CGRAM sin producir parpadeos raros).
    Verás esa espera como la llamada WaitForVBlank() al final del bucle en main().
=====================================================================================*/

// snes.h es la cabecera "paraguas" de PVSnesLib: incluye a su vez background.h,
// object.h, map.h, sprite.h, sound.h, input.h, video.h, console.h, etc. Con solo
// este include tenemos disponibles todas las funciones/tipos que usa este archivo
// (bgXxx, objXxx, mapXxx, oamXxx, spcXxx, padsCurrent, KEY_xxx, setMode...).
#include <snes.h>

// soundbank.h es un archivo GENERADO automáticamente por las herramientas de
// conversión de audio de PVSnesLib a partir de los ficheros overworld.it (música,
// formato tracker Impulse Tracker) y mariojump.wav (efecto de sonido). Declara el
// símbolo MOD_OVERWORLD (el índice del módulo de música dentro del "banco de
// sonido" que se compila y enlaza en la ROM).
#include "soundbank.h"

// Estos tres ".inc" también son generados automáticamente (por el Makefile, a
// partir de los .bmp/.png de este mismo directorio) cuando compilas el proyecto.
// Cada uno declara punteros/etiquetas en ensamblador hacia los datos ya convertidos
// a formato SNES (tiles 4bpp, paletas de 15 colores + transparencia, mapas):
//   - mario_sprite.inc -> gráficos+paleta del spritesheet de Mario (mario_sprite.bmp)
//   - mariofont.inc    -> gráficos+paleta de la fuente de texto (mariofont.bmp)
//   - tiles.inc        -> gráficos+paleta del tileset del escenario (tiles.png)
// No son ficheros que edites a mano: si cambias el .bmp/.png de origen, el Makefile
// vuelve a generar el .inc correspondiente en el siguiente "make".
#include "mario_sprite.inc"
#include "mariofont.inc"
#include "tiles.inc"

//---------------------------------------------------------------------------------
// SOUNDBANK__ es la etiqueta (definida en soundbank.asm, también generado) que
// marca el INICIO del banco de sonido dentro de la ROM. Se pasa a spcSetBank() en
// main() para decirle al motor de audio dónde vive todo el contenido sonoro.
extern char SOUNDBANK__;
// jumpsnd / jumpsndend son las etiquetas de inicio y fin de los datos BRR (el
// formato de audio comprimido que usa el chip de sonido de la SNES, el S-DSP) del
// efecto de salto. Restando las direcciones (&jumpsndend - &jumpsnd) obtenemos su
// tamaño en bytes sin tener que conocerlo de antemano: es un truco habitual en C
// para "archivos incluidos como datos" (aquí, incluidos vía ensamblador).
extern char jumpsnd, jumpsndend;

//---------------------------------------------------------------------------------
// GRAVITY es cuánto se acelera Mario hacia abajo cada fotograma mientras está en el
// aire. JUMPVALUE es la velocidad vertical inicial de un salto normal.
//
// Estos números NO están en pixeles enteros: el motor de objetos de PVSnesLib
// trabaja internamente en PUNTO FIJO (fixed point), una técnica clásica en
// hardware sin unidad de coma flotante (la SNES no tiene FPU). En vez de usar
// decimales, se reserva parte de un entero para representar la fracción. Aquí el
// formato es "8.8": el byte alto es la parte entera en pixeles y el byte bajo es
// la fracción de pixel (1/256 de pixel de precisión). Por eso GRAVITY=48 no son
// "48 pixeles", sino 48/256 ~= 0.19 pixeles de aceleración por fotograma; y
// JUMPVALUE = GRAVITY*20 es, en la práctica, "una velocidad iniciale calibrada a
// ojo para que el salto se sienta bien", típico del ajuste manual de física en
// juegos de plataformas retro.
#define GRAVITY 48
#define JUMPVALUE (GRAVITY * 20)

// Constantes de movimiento horizontal de Mario, también en el mismo punto fijo 8.8:
//   MARIO_MAXACCEL -> velocidad horizontal máxima (tope de velocidad al andar)
//   MARIO_ACCEL    -> cuánta velocidad se suma/resta cada fotograma que se pulsa
//                     izquierda/derecha (aceleración, no velocidad directa: por
//                     eso Mario tarda unos fotogramas en alcanzar la velocidad tope)
//   MARIO_JUMPING  -> velocidad vertical (hacia arriba, de ahí el signo negativo al
//                     usarla) de un salto normal
//   MARIO_HIJUMPING-> velocidad vertical de un salto "alto" (cuando se pulsa arriba
//                     a la vez que el botón de salto)
#define MARIO_MAXACCEL 0x0140
#define MARIO_ACCEL 0x0038
#define MARIO_JUMPING 0x0394
#define MARIO_HIJUMPING 0x0594

// Estos identificadores documentan qué "fotograma" (frame) del spritesheet de
// Mario corresponde a cada pose: agachado/cayendo (0), saltando (1), caminando
// (2, y su vecino 3 se usa alternando para animar el paso) y de pie/quieto (6).
// Es un simple "enum" de constantes con nombre (más legible que usar 0,1,2,6
// sueltos por el código); fíjate más abajo que oambuffer[0].oamframeid SÍ usa esos
// mismos números literales (6 al estar quieto, 1 al saltar...) — es la forma en
// que este ejemplo conecta "qué animación toca" con "qué recorte de la imagen
// mario_sprite.bmp hay que mostrar".
enum
{
    MARIODOWN = 0,
    MARIOJUMPING = 1,
    MARIOWALK = 2,
    MARIOSTAND = 6
}; // Mario state

//---------------------------------------------------------------------------------
// mapmario/objmario, tilesetdef/tilesetatt son etiquetas (direcciones de memoria)
// definidas en data.asm, que a su vez es GENERADO por el Makefile a partir de
// map_1_1.tmx (el mapa del nivel, editado con la herramienta Tiled) y de la
// definición de objetos del nivel:
//   - mapmario     -> la cuadrícula del nivel 1-1: qué "tile de mapa" va en cada
//                     casilla (para mapLoad).
//   - objmario     -> la lista de objetos a colocar en el nivel (posición inicial,
//                     tipo, límites) que consume objLoadObjects().
//   - tilesetdef   -> qué tile GRAFICO (de tiles.inc) corresponde a cada tile de
//                     mapa (una tabla de indirección: mapa -> gráfico).
//   - tilesetatt   -> las PROPIEDADES de cada tile (sólido, pincho, escalera...),
//                     usadas por el motor de colisiones (objCollidMap) para saber
//                     con qué está chocando Mario.
// Se declaran como "extern char" (un solo byte) porque en C solo nos interesa su
// DIRECCIÓN (por eso siempre se usan con el operador &), no su contenido tal cual:
// son, en la práctica, punteros/etiquetas hacia bloques de datos en la ROM.
extern char mapmario, objmario;     // for map & tileset of map
extern char tilesetdef, tilesetatt; // for map & tileset of map

//---------------------------------------------------------------------------------
// Jump guardará, tras spcSetSoundEntry() en main(), los parámetros ya calculados
// (tono, volumen, panorámica, dirección en RAM de sonido...) para poder reproducir
// el efecto de salto en cualquier momento con una simple llamada a spcPlaySound().
brrsamples Jump; // The sound for jumping

// pad0 guarda, cada fotograma, el estado actual del mando 1 (qué botones están
// pulsados AHORA MISMO, como un mapa de bits). Se recalcula en cada marioupdate().
u16 pad0; // pad variable

// marioobj apunta a la "ficha" (struct t_objs, ver object.h) que el motor de
// objetos reserva internamente para Mario: ahí vive su posición, velocidad,
// acción actual (andando/saltando/cayendo), qué tile pisa, etc.
t_objs *marioobj; // pointer to mario object
// marioox/mariooy (Object X/Object Y) y marioxv/marioyv (velocity X/Y) son
// ATAJOS: en vez de escribir "marioobj->xpos" cada vez, guardamos directamente la
// dirección de memoria de esos campos, para leerlos/escribirlos más rápido y con
// menos código. Se calculan una sola vez en marioinit() y se reutilizan siempre.
s16 *marioox, *mariooy; // basics x/y coordinates pointers with fixed point
s16 *marioxv, *marioyv; // basics x/y velocity pointers with fixed point
// mariox/marioy son la posición de Mario ya "aplanada" a un solo u16 (los valores
// enteros, sin la parte fraccionaria de punto fijo), expresada en coordenadas DEL
// MAPA COMPLETO (que es mucho más grande que la pantalla), no solo de lo visible.
u16 mariox, marioy; // x & y coordinates of mario with map depth (not only screen)
// mariofidx: índice de fotograma de animación (alterna 0/1 al caminar, para dar
//            la sensación de "paso izquierdo, paso derecho").
// marioflp:  frame BASE de la animación actual (offset dentro del spritesheet:
//            2 para caminar, y luego se le suma mariofidx).
// flip:      contador de fotogramas usado como "reloj" para saber cuándo toca
//            cambiar de pose (ver el truco "flip & 3" en mariowalk).
u8 mariofidx, marioflp, flip; // to manage sprite display

//---------------------------------------------------------------------------------
/*
    marioinit(): FUNCIÓN DE INICIALIZACIÓN de un objeto de tipo "Mario".

    No la llamamos nosotros directamente: se la "enchufa" al motor de objetos con
    objInitFunctions(0, &marioinit, &marioupdate, NULL) en main(), y es el propio
    motor quien la invoca automáticamente, UNA VEZ por cada objeto de ese tipo,
    cuando objLoadObjects() procesa la tabla de objetos del nivel (objmario). Es el
    mismo patrón que un "constructor" en programación orientada a objetos, salvo
    que aquí no hay clases: el motor de PVSnesLib usa un array fijo de structs
    (objbuffers[]) y "engancha" comportamiento mediante punteros a función.

    Parámetros (los pone el motor, leyéndolos de la tabla objmario):
      xp, yp    -> posición inicial de Mario en el mapa.
      type      -> tipo de objeto (aquí siempre 0, el único tipo de este demo).
      minx,maxx -> límites opcionales de movimiento (no se usan en este ejemplo,
                   pero el motor genérico los admite para otros objetos, p.ej. un
                   enemigo que patrulla de un punto a otro).
*/
void marioinit(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx)
{
    // consoleNocashMessage() envía texto a la ventana de depuración del emulador
    // no$sns (y, en Mesen, aparece igualmente en su consola de logs). NO hace
    // nada útil en una consola SNES real -ni siquiera existe ese "canal" en el
    // hardware físico-, por eso el propio comentario original advierte que es
    // solo para depurar mientras desarrollas, nunca para lógica de un juego final.
    consoleNocashMessage("marioinit %d %d\n", (u16)xp, (u16)yp);

    // objNew() reserva una "ficha" libre dentro de objbuffers[] (el array interno
    // de hasta OB_MAX objetos) y la marca como activa con este tipo y posición.
    // Guarda el identificador del objeto recién creado en la variable global
    // objgetid. Si no hay huecos libres, devuelve 0 y no hay nada más que hacer.
    if (objNew(type, xp, yp) == 0)
        // no more space, get out
        return;

    // objGetPointer(objgetid) traduce ese identificador en un puntero real dentro
    // de objbuffers[] y lo dejará disponible en la variable global objptr (que
    // apunta a la POSICIÓN+1 dentro del array, de ahí el "-1" de la siguiente
    // línea para obtener el índice real). Guardamos ese puntero en marioobj para
    // no tener que repetir esta traducción cada vez que necesitemos tocar los
    // datos de Mario.
    objGetPointer(objgetid);
    marioobj = &objbuffers[objptr - 1];
    // Le decimos al motor de colisiones el tamaño (en pixeles) de la "caja" de
    // Mario: 16x16, que coincide con el tamaño de sprite grande configurado más
    // abajo en oamInitDynamicSprite() (OBJ_SIZE8_L16 -> sprites grandes de 16x16).
    marioobj->width = 16;
    marioobj->height = 16;

    // Calculamos y guardamos los punteros "atajo" descritos más arriba. xpos/ypos
    // son arrays de 3 bytes en punto fijo (ver object.h): sumar 1 al puntero base
    // y reinterpretarlo como (u16*) nos da acceso directo a los 16 bits más
    // significativos (la parte "entera en pixeles" que realmente necesitamos para
    // dibujar), saltándonos el byte menos significativo (la fracción más fina).
    marioox = (u16 *)&(marioobj->xpos + 1);
    mariooy = (u16 *)&(marioobj->ypos + 1);
    // xvel/yvel sí son campos u16 normales (velocidad con signo, en el mismo
    // punto fijo 8.8 descrito junto a las constantes MARIO_xxx de más arriba).
    marioxv = (short *)&(marioobj->xvel);
    marioyv = (short *)&(marioobj->yvel);

    // Estado inicial de animación: fotograma 0, sin desplazamiento de pose, y el
    // motor de objetos arranca a Mario en el estado "de pie" (ACT_STAND, definido
    // en object.h). marioupdate() irá cambiando esta "action" según lo que ocurra.
    mariofidx = 0;
    marioflp = 0;
    marioobj->action = ACT_STAND;

    // Configuramos la "ficha" de sprite dinámico (struct t_sprites, ver
    // sprite.h) que se usará para dibujar a Mario cada fotograma:
    //   oamframeid  -> qué recorte (frame) del spritesheet mostrar al empezar (6
    //                  = de pie, según el enum de más arriba).
    //   oamrefresh  -> 1 le dice al motor de sprites dinámicos que, este
    //                  fotograma, debe (re)cargar el gráfico correspondiente en
    //                  VRAM (los sprites dinámicos comparten un hueco pequeño de
    //                  VRAM que se va reescribiendo según haga falta, en vez de
    //                  tener sitio para TODOS los fotogramas de animación a la vez).
    //   oamattribute-> byte de atributos OAM: 0x60 = prioridad 2 (bits 5-4) más
    //                  paleta de sprite 0 (bits 3-1, aquí "(0 << 1)" hace explícito
    //                  que se usa la paleta 0, aunque numéricamente no sume nada).
    //                  El bit 6 (flip horizontal) se activa/desactiva más adelante
    //                  en marioupdate() según hacia dónde mire Mario.
    //   oamgraphics -> puntero a los datos gráficos de origen (mario_sprite_til,
    //                  definido en mario_sprite.inc) de donde el motor copiará
    //                  los tiles a VRAM cuando oamrefresh esté a 1.
    oambuffer[0].oamframeid = 6;
    oambuffer[0].oamrefresh = 1;
    oambuffer[0].oamattribute = 0x60 | (0 << 1); // palette 0 of sprite and sprite 16x16 and priority 2 and flip sprite
    oambuffer[0].oamgraphics = &mario_sprite_til;

    // setPalette() (macro que en background.h/video.h termina llamando a
    // dmaCopyCGram) copia la paleta de colores de Mario a la CGRAM, en el hueco
    // reservado a partir de la entrada 128 (las paletas de SPRITES ocupan las
    // entradas 128-255 de la CGRAM; las de fondos, 0-127). "16*2" son 32 bytes:
    // 16 colores de 2 bytes cada uno (formato de color SNES: 15 bits RGB en un
    // valor de 16 bits).
    setPalette(&mario_sprite_pal, 128 + 0 * 16, 16 * 2);
}

//---------------------------------------------------------------------------------
/*
    mariowalk(): manejador de animación para cuando Mario está ANDANDO por el
    suelo (marioobj->action == ACT_WALK). Se llama una vez por fotograma desde
    marioupdate(), y es responsable únicamente de la parte visual (qué fotograma
    del spritesheet toca mostrar) y de detectar cuándo hay que cambiar de "acción"
    (a caer o a quedarse quieto), dejando el movimiento en sí a objUpdateXY().

    El parámetro "idx" es el identificador del objeto dentro del motor (aquí no se
    usa en el cuerpo de la función porque solo hay un Mario, pero la FIRMA debe
    tener ese parámetro u16 porque así es como el motor de objetos invoca a
    cualquier función de actualización/refresco, sea cual sea el tipo de objeto).
*/
void mariowalk(u8 idx)
{
    // "flip" actúa como un contador de fotogramas de pantalla (0,1,2,3,0,1,2,3...
    // se desborda solo al superar 255 y seguir sumando, sin que nos importe).
    // "flip & 3" vale 3 justo un fotograma de cada cuatro: así conseguimos que la
    // animación de andar cambie de pose cada 4 fotogramas (a 60 FPS, unas 15
    // veces por segundo) en lugar de cada fotograma, que se vería demasiado
    // rápido/tembloroso.
    flip++;
    if ((flip & 3) == 3)
    {
        // Alterna el índice de fotograma entre 0 y 1 con la máscara "& 1" (un
        // patrón muy típico en C para hardware: en vez de "if/else" para
        // alternar, se usa aritmética de bits, más barata en un procesador de
        // 8/16 bits como el 65816 de la SNES).
        mariofidx++;
        mariofidx = mariofidx & 1;
        // marioflp ya trae el offset de "caminando" (2, puesto en marioupdate);
        // sumándole mariofidx (0 o 1) obtenemos el fotograma 2 o 3 del
        // spritesheet: los dos "pasos" de la animación de caminar.
        oambuffer[0].oamframeid = marioflp + mariofidx;
        // Avisamos de nuevo al motor de sprites dinámicos que debe refrescar el
        // gráfico en VRAM, porque hemos cambiado a qué fotograma apunta oamframeid.
        oambuffer[0].oamrefresh = 1;
    }

    // Comprobación de transición de estado: objCollidMap() (llamado antes, desde
    // marioupdate) ya habrá actualizado marioyv/marioxv según la física y las
    // colisiones de este fotograma. Si hay velocidad vertical, es que Mario ha
    // dejado de estar apoyado en un tile sólido (se ha caído de un borde, por
    // ejemplo) y pasamos a la acción de caída. Si ambas velocidades son cero,
    // es que se ha detenido del todo y pasamos a la acción de estar de pie.
    if (*marioyv != 0)
        marioobj->action = ACT_FALL;
    else if ((*marioxv == 0) && (*marioyv == 0))
        marioobj->action = ACT_STAND;
}

//---------------------------------------------------------------------------------
/*
    mariofall(): manejador para cuando Mario está CAYENDO (marioobj->action ==
    ACT_FALL). En este ejemplo tan simple no hay una animación de caída propia
    (se mantiene el último fotograma que tuviera el sprite mientras cae); la
    única responsabilidad de esta función es detectar el ATERRIZAJE.
*/
void mariofall(u8 idx)
{
    // objCollidMap() pone a 0 la velocidad vertical (yvel) en el instante en que
    // detecta que Mario ha topado con un tile sólido por debajo (ha "aterrizado").
    // Cuando eso ocurre, volvemos al estado "de pie" y reponemos el fotograma 6
    // (la pose neutral, ver el enum MARIOSTAND al principio del archivo).
    if (*marioyv == 0)
    {
        marioobj->action = ACT_STAND;
        oambuffer[0].oamframeid = 6;
        oambuffer[0].oamrefresh = 1;
    }
}

//---------------------------------------------------------------------------------
/*
    mariojump(): manejador para cuando Mario está SALTANDO (marioobj->action ==
    ACT_JUMP). Se activa un único fotograma de "salto" y, en cuanto la velocidad
    vertical deja de ser negativa (es decir, en cuanto Mario alcanza el punto más
    alto del salto y empieza a bajar por efecto de la gravedad), se cede el
    control a la acción de caída (mariofall se encargará entonces de detectar el
    aterrizaje).
*/
void mariojump(u8 idx)
{
    // Cambia al fotograma de salto (1) solo si no estaba ya puesto, para no pedir
    // un refresco de VRAM innecesario en cada fotograma mientras se sigue en el aire.
    if (oambuffer[0].oamframeid != 1)
    {
        oambuffer[0].oamframeid = 1;
        oambuffer[0].oamrefresh = 1;
    }

    // *marioyv es un short CON SIGNO: negativo significa "moviéndose hacia
    // arriba" (recuerda que en pantalla el eje Y crece hacia ABAJO, al revés que
    // en matemáticas). La gravedad (aplicada internamente por el motor de
    // colisiones en cada objCollidMap) va sumando GRAVITY a yvel cada fotograma,
    // así que yvel empieza muy negativo (el impulso del salto) y va subiendo
    // hasta cruzar 0: ese es el instante exacto en que Mario "toca techo" y
    // empieza a caer.
    if (*marioyv >= 0)
        marioobj->action = ACT_FALL;
}

//---------------------------------------------------------------------------------
/*
    marioupdate(): FUNCIÓN DE ACTUALIZACIÓN de Mario, el "cerebro" que se ejecuta
    UNA VEZ POR FOTOGRAMA (60 veces por segundo). Igual que marioinit(), no la
    llamamos nosotros: queda registrada en objInitFunctions() y el motor la invoca
    automáticamente desde objUpdateAll() (ver el bucle principal en main()) para
    cada objeto activo que esté dentro de la "pantalla virtual".

    Aquí se concentra toda la lógica de "un fotograma de juego" para Mario:
      1. Leer el mando.
      2. Traducir botones pulsados en cambios de velocidad/estado.
      3. Delegar en el motor de colisiones contra el mapa.
      4. Delegar la animación al manejador del estado actual (walk/fall/jump).
      5. Mover a Mario según su velocidad (física).
      6. Recolocar su sprite en pantalla y mover la cámara para seguirlo.
*/
void marioupdate(u8 idx)
{
    // padsCurrent(0) devuelve, como un mapa de bits de 16 bits, qué botones del
    // mando 1 están pulsados EN ESTE INSTANTE (no distingue "recién pulsado" de
    // "mantenido"; para eso PVSnesLib ofrece otras variables/funciones, pero este
    // ejemplo solo necesita saber "está pulsado ahora sí o no").
    pad0 = padsCurrent(0);

    // Comprobación rápida: si no hay NINGUNA de las teclas que nos interesan
    // pulsada, nos ahorramos evaluar los tres "if" de dentro. "&" es un AND a
    // nivel de bits: KEY_RIGHT|KEY_LEFT|KEY_A combina sus tres máscaras de bit en
    // una sola, y "pad0 & (...)" da distinto de cero si CUALQUIERA de esos tres
    // bits está activo en pad0.
    if (pad0 & (KEY_RIGHT | KEY_LEFT | KEY_A))
    {
        // --- Movimiento hacia la izquierda ---
        if (pad0 & KEY_LEFT)
        {
            // Si el fotograma de animación actual no pertenece ya al rango de
            // "caminar" (2 o 3), lo reiniciamos a 2 (el primer paso). Esto evita
            // que, al cambiar de dirección o de una pose distinta (salto, quieto)
            // a caminar, se empiece la animación de andar "a medias".
            if ((marioflp > 3) || (marioflp < 2))
            {
                marioflp = 2;
            }
            // El bit 0x40 del byte de atributos OAM es el flip horizontal
            // (ver OAM_ATTR/t_sprites en sprite.h). "&= ~0x40" lo apaga: como el
            // spritesheet dibuja a Mario mirando hacia la derecha de forma
            // "nativa", para mirar a la izquierda basta con NO voltear el sprite.
            oambuffer[0].oamattribute &= ~0x40; // do not flip sprite

            // Pasamos a la acción de caminar y aplicamos aceleración negativa
            // (hacia la izquierda). Fíjate que esto es ACELERACIÓN, no velocidad
            // directa: cada fotograma que se mantenga pulsado izquierda, xvel se
            // vuelve un poco más negativo, hasta el tope -MARIO_MAXACCEL (así se
            // consigue esa sensación de "arrancar" progresivamente, típica de
            // Mario, en vez de moverse a velocidad constante desde el primer
            // instante).
            marioobj->action = ACT_WALK;
            *marioxv -= (MARIO_ACCEL);
            if (*marioxv <= (-MARIO_MAXACCEL))
                *marioxv = (-MARIO_MAXACCEL);
        }
        // --- Movimiento hacia la derecha (simétrico al de la izquierda) ---
        if (pad0 & KEY_RIGHT)
        {
            if ((marioflp > 3) || (marioflp < 2))
            {
                marioflp = 2;
            }
            // Aquí sí activamos el flip horizontal (0x40) para que el sprite,
            // dibujado "de fábrica" mirando a la derecha, se voltee... espera:
            // en realidad, según el comentario del propio autor, esta rama es la
            // que ENCIENDE el flip. Compáralo con la rama de la izquierda (que lo
            // apaga): son las dos caras de la misma decisión "hacia dónde miro".
            oambuffer[0].oamattribute |= 0x40; // flip sprite

            marioobj->action = ACT_WALK;
            *marioxv += (MARIO_ACCEL);
            if (*marioxv >= (MARIO_MAXACCEL))
                *marioxv = (MARIO_MAXACCEL);
        }
        // --- Salto ---
        if (pad0 & KEY_A)
        {
            // marioobj->tilestand es un campo que objCollidMap() rellena cada
            // fotograma con el tipo de tile que Mario tiene justo debajo de los
            // pies (0 si no hay nada sólido ahí, es decir, si está en el aire).
            // Solo dejamos saltar si tilestand != 0: así se evita el clásico
            // "salto infinito en el aire" de los juegos de plataformas mal hechos.
            if ((marioobj->tilestand != 0))
            {
                marioobj->action = ACT_JUMP;
                // Si además se mantiene pulsado arriba, el salto es más alto
                // (velocidad inicial MARIO_HIJUMPING en vez de MARIO_JUMPING).
                // El signo negativo es, otra vez, "hacia arriba en pantalla".
                if (pad0 & KEY_UP)
                    *marioyv = -(MARIO_HIJUMPING);
                else
                    *marioyv = -(MARIO_JUMPING);
                // spcPlaySound(0) pide al motor de audio que reproduzca, ahora
                // mismo, el ÚLTIMO efecto de sonido cargado en la tabla de sonido
                // (índice 0 = el más reciente; aquí es "Jump", el único efecto
                // cargado por spcSetSoundEntry() en main()).
                spcPlaySound(0);
            }
        }
    }

    // objCollidMap(idx) es el corazón físico del motor de objetos genérico:
    // mira qué tiles del mapa hay alrededor de Mario (según su posición y su
    // tamaño width/height fijados en marioinit), actualiza tilestand/tileabove
    // (qué pisa/qué tiene encima) y AJUSTA xvel/yvel según haya colisiones
    // (por ejemplo, poniendo yvel a 0 al aterrizar, o aplicando la gravedad
    // configurada por defecto si Mario está en el aire). Se llama SIEMPRE, cada
    // fotograma, independientemente de qué botones se hayan pulsado.
    objCollidMap(idx);

    // Según en qué "acción" haya quedado Mario (ya sea porque el jugador acaba de
    // pulsar un botón, ya sea porque objCollidMap ha detectado que está en el
    // aire o ha aterrizado), delegamos la animación/transición de estado en el
    // manejador correspondiente. Es un "switch" simplificado (una máquina de
    // estados hecha a mano con if/else): cada manejador puede, a su vez, cambiar
    // marioobj->action para el SIGUIENTE fotograma.
    if (marioobj->action == ACT_WALK)
        mariowalk(idx);
    else if (marioobj->action == ACT_FALL)
        mariofall(idx);
    else if (marioobj->action == ACT_JUMP)
        mariojump(idx);

    // objUpdateXY(idx) aplica físicamente la velocidad actual (xvel/yvel, ya
    // ajustada arriba por el jugador y por la colisión) a la posición (xpos/ypos)
    // del objeto. Es, literalmente, "posición += velocidad", pero hecho en
    // aritmética de punto fijo dentro del motor.
    objUpdateXY(idx);

    // Salvaguarda de límites: evita que Mario obtenga coordenadas "negativas" (en
    // realidad, como marioox/mariooy son u16 sin signo, un valor que se saliera
    // por abajo de 0 daría la vuelta a un número enorme; esta comprobación evita
    // ese problema si el jugador empuja contra el borde izquierdo/superior del
    // mapa).
    if (*marioox <= 0)
        *marioox = 0;
    if (*mariooy <= 0)
        *mariooy = 0;

    // Volcamos la posición de punto fijo (ya leída como enteros de 16 bits, ver
    // la explicación de marioox/mariooy más arriba) a mariox/marioy, que son la
    // posición de Mario dentro del MAPA COMPLETO (no de la pantalla).
    mariox = (*marioox);
    marioy = (*mariooy);
    // La pantalla solo muestra una "ventana" de 256x224 pixeles del mapa; x_pos e
    // y_pos (variables globales del motor de mapa, ver map.h) son la esquina
    // superior izquierda de esa ventana (la "cámara"). Restando la cámara a la
    // posición de Mario en el mapa obtenemos su posición EN PANTALLA, que es lo
    // que hay que escribir en el sprite (oamx/oamy son coordenadas de pantalla,
    // no de mapa).
    oambuffer[0].oamx = mariox - x_pos;
    oambuffer[0].oamy = marioy - y_pos;
    // oamDynamic16Draw(0) es quien realmente "publica" el contenido de
    // oambuffer[0] hacia el sistema de sprites dinámicos: coloca el sprite en la
    // cola de dibujo de este fotograma (tamaño 16x16, de ahí el "16" del nombre)
    // y, si oamrefresh estaba a 1, programa también la copia del gráfico a VRAM.
    oamDynamic16Draw(0);

    // mapUpdateCamera() le pide al motor de mapa que centre (o siga, según su
    // lógica interna de límites de mapa) la cámara sobre la posición de Mario.
    // Esto es lo que hace que el escenario se desplace (scroll) mientras caminas.
    mapUpdateCamera(mariox, marioy);
}

//---------------------------------------------------------------------------------
/*
    main(): PUNTO DE ENTRADA del programa. En una SNES no existe "el sistema
    operativo que llama a main()" como en un PC: es el propio arranque de la
    consola (el vector de reset del 65816, definido en hdr.asm) el que acaba
    saltando aquí a través del runtime de PVSnesLib. A partir de aquí, TODO el
    juego (inicialización + bucle infinito) vive dentro de esta función.

    El orden de las llamadas de inicialización importa: cada subsistema (sonido,
    texto, fondos, sprites, objetos, mapa) necesita que ciertos pasos previos ya
    se hayan hecho antes de poder usarse con seguridad.
*/
int main(void)
{
    // spcBoot() arranca el SPC700 (el procesador de sonido secundario de la
    // SNES, un chip aparte del 65816 que ejecuta el propio "main") cargándole su
    // pequeño programa controlador (driver) por el puerto de comunicación entre
    // ambos procesadores. Tarda un tiempo perceptible, por eso se hace lo primero
    // de todo y con la pantalla aún apagada (nadie la ve tardar).
    spcBoot();

    // --- Preparar la consola de texto (para el HUD: "MARIO x00  WORLD TIME") ---
    // consoleSetTextMapPtr/consoleSetTextGfxPtr fijan en qué direcciones de VRAM
    // vivirán, respectivamente, el MAPA de texto (qué caracter va en cada celda
    // de la rejilla) y los GRÁFICOS de la fuente (el dibujo de cada letra).
    // 0x6000 y 0x3000 son direcciones de VRAM elegidas a mano para que no se
    // solapen con el resto de gráficos que se cargan más abajo (tiles del mapa,
    // sprites...): en SNES, organizar manualmente el mapa de VRAM es habitual.
    consoleSetTextMapPtr(0x6000);
    consoleSetTextGfxPtr(0x3000);
    // consoleInitText() vuelca en VRAM la fuente tipográfica (mariofont_til +
    // mariofont_pal, generados a partir de mariofont.bmp) y prepara el sistema de
    // texto para poder usar consoleDrawText() más adelante. "1" es el número de
    // paleta a usar para el texto y "16*2" el tamaño en bytes de esa paleta.
    consoleInitText(1, 16 * 2, &mariofont_til, &mariofont_pal);

    // --- Preparar y cargar audio ---
    // spcSetBank() indica dónde, dentro de la ROM, empieza el banco de sonido
    // (todo el contenido convertido de overworld.it y mariojump.wav).
    spcSetBank(&SOUNDBANK__);

    // Reserva ~10 KB de la RAM de sonido (que vive DENTRO del chip de audio, es
    // memoria totalmente aparte de la RAM principal de la SNES) para poder cargar
    // muestras BRR reproducibles como efectos de sonido. El comentario original
    // ya explica las cuentas: 39 bloques de 256 bytes.
    spcAllocateSoundRegion(39);

    // Carga el módulo de música de fondo (el "overworld" convertido desde
    // overworld.it) en la RAM de sonido, dejándolo listo para reproducir.
    spcLoad(MOD_OVERWORLD);

    // Registra el efecto de sonido de salto en la tabla de sonidos del motor de
    // audio: volumen 15/15, panorámica central-ish (8), tono/pitch 6, calculando
    // su longitud como la diferencia entre las etiquetas de fin e inicio (técnica
    // ya explicada junto a la declaración de jumpsnd/jumpsndend), y guarda los
    // parámetros resultantes en la variable global "Jump" declarada arriba.
    spcSetSoundEntry(15, 8, 6, &jumpsndend - &jumpsnd, &jumpsnd, &Jump);

    // --- Preparar el fondo (BG) que usará el motor de mapa ---
    // El motor de mapa de PVSnesLib (map.h) exige, por diseño, trabajar sobre el
    // Background #1 en modo 1, con un layout de VRAM muy concreto:
    //   bgSetGfxPtr(1, 0x3000)         -> los TILES de este BG viven en 0x3000
    //   bgSetMapPtr(1, 0x6000, ...)    -> reserva un mapa "auxiliar" de trabajo en
    //                                     0x6000 (el propio comentario del header
    //                                     de map.h insiste en que 0x6800 es la
    //                                     dirección obligatoria para el mapa
    //                                     "real" que se ve, fijada dos líneas
    //                                     más abajo).
    //   bgInitTileSet(0, ...)          -> carga los tiles de tiles.inc (gráficos +
    //                                     paleta) en VRAM para el BG 0, en modo
    //                                     16 colores, en la dirección 0x2000.
    //   bgSetMapPtr(0, 0x6800, ...)    -> fija dónde vive el mapa de 64x32 tiles
    //                                     que verá el jugador (el que mapLoad()
    //                                     rellenará dinámicamente según la cámara).
    bgSetGfxPtr(1, 0x3000);
    bgSetMapPtr(1, 0x6000, SC_32x32);
    bgInitTileSet(0, &tiles_til, &tiles_pal, 0, (&tiles_tilend - &tiles_til), 16 * 2, BG_16COLORS, 0x2000);
    bgSetMapPtr(0, 0x6800, SC_64x32);

    // --- Preparar el motor de sprites dinámicos ---
    // oamInitDynamicSprite() prepara el sistema que, fotograma a fotograma, va
    // copiando a VRAM solo los gráficos de sprite que realmente hacen falta (en
    // vez de tener sitio en VRAM para TODAS las poses de TODOS los personajes a
    // la vez, que no cabría). 0x0000/0x1000 son las direcciones de VRAM para
    // sprites "grandes" y "pequeños" respectivamente; OBJ_SIZE8_L16 configura el
    // hardware para sprites pequeños de 8x8 y grandes de 16x16 (Mario usa el
    // tamaño grande, ver marioinit: width=height=16).
    oamInitDynamicSprite(0x0000, 0x1000, 0, 0, OBJ_SIZE8_L16);

    // --- Preparar el motor de objetos (entidades con física/colisión) ---
    // objInitEngine() inicializa sus estructuras internas (objbuffers[], listas
    // enlazadas de objetos activos, etc.). Se llama una única vez, antes de crear
    // ningún objeto.
    objInitEngine();

    // objInitFunctions() "registra" el trío de funciones que este archivo define
    // para el tipo de objeto 0 (Mario, el único tipo que existe en este demo):
    // marioinit (constructor), marioupdate (lógica por fotograma) y NULL para la
    // función de refresco (no la necesitamos: dibujamos el sprite directamente
    // dentro de marioupdate con oamDynamic16Draw, en vez de separarlo en una
    // tercera función como permite el motor).
    objInitFunctions(0, &marioinit, &marioupdate, NULL);

    // objLoadObjects() lee la tabla de objetos del nivel (objmario, generada
    // desde el editor de mapas) y, por cada entrada, llama automáticamente a
    // marioinit() con los parámetros (posición, tipo, límites) de esa entrada.
    // Aquí es donde, en la práctica, "nace" el Mario que verás en pantalla.
    objLoadObjects((char *)&objmario);

    // mapLoad() vuelca en VRAM la primera "ventana" visible del mapa del nivel
    // (mapmario), usando tilesetdef (qué gráfico corresponde a cada tile de mapa)
    // y tilesetatt (qué PROPIEDADES de colisión tiene cada tile: sólido, pincho,
    // escalera...) para que el motor de colisiones (objCollidMap) pueda consultarlas
    // más tarde.
    mapLoad((u8 *)&mapmario, (u8 *)&tilesetdef, (u8 *)&tilesetatt);

    // setMode(BG_MODE1, 0) activa el Modo de vídeo 1 de la PPU (16 colores en BG1
    // y BG2, 4 colores en BG3): es el modo que usan el mapa (BG1, en realidad BG0
    // a nivel de API de PVSnesLib, ver arriba) y el texto. bgSetDisable(2) apaga
    // el BG3 porque este demo no lo usa (evita dibujar basura ahí).
    setMode(BG_MODE1, 0);
    bgSetDisable(2);

    // Dibuja el HUD de texto fijo en las filas 16 y 17 de la rejilla de texto,
    // columna 6: un remedo del clásico marcador "MARIO / WORLD / TIME" de Super
    // Mario Bros (aquí con valores de relleno "00000", no conectados a lógica de
    // puntuación real: es solo decorativo en este ejemplo).
    consoleDrawText(6, 16, "MARIOx00  WORLD TIME");
    consoleDrawText(6, 17, " 00000 ox00 1-1  000");

    // Hasta este punto la pantalla ha estado APAGADA a propósito (es la práctica
    // recomendada en SNES: preparar todo el contenido de VRAM/CGRAM/OAM primero,
    // y solo entonces "encender" para que el jugador no vea ningún fotograma a
    // medio construir). setScreenOn() la enciende.
    setScreenOn();

    // Arranca la reproducción de música desde el principio del módulo cargado
    // (posición 0) y fija el volumen general de la música al 100%.
    spcPlay(0);
    spcSetModuleVolume(100);

    // Una espera a VBlank suelta antes de entrar al bucle principal, para
    // sincronizar el arranque del bucle con el ritmo de 60 Hz de la pantalla.
    WaitForVBlank();

    // ============================ BUCLE PRINCIPAL ============================
    // Este "while (1)" es el auténtico corazón del juego: se repite EXACTAMENTE
    // una vez por fotograma, para siempre (hasta apagar la consola/emulador). El
    // orden de las llamadas dentro del bucle es importante y sigue el patrón
    // típico de PVSnesLib para juegos con mapa+objetos+sonido:
    while (1)
    {
        // 1) mapUpdate(): recalcula, en RAM normal (bg_L1[], el buffer descrito
        //    en map.h), qué filas/columnas nuevas del mapa hay que mostrar según
        //    hacia dónde se ha movido la cámara (mapUpdateCamera, llamada dentro
        //    de marioupdate). Todavía NO toca VRAM aquí: solo prepara los datos.
        mapUpdate();

        // 2) objUpdateAll(): recorre todos los objetos activos (aquí, solo
        //    Mario) que estén dentro de la "pantalla virtual" y llama a su
        //    función de actualización registrada (marioupdate). Aquí es donde
        //    realmente se ejecuta, cada fotograma, toda la lógica descrita arriba
        //    (leer mando, mover, colisionar, animar, posicionar sprite, mover
        //    cámara).
        objUpdateAll();

        // 3) oamInitDynamicSpriteEndFrame(): cierra la "recolección" de sprites
        //    de este fotograma y deja listo el sistema de sprites dinámicos para
        //    poder empezar a acumular los del SIGUIENTE fotograma.
        oamInitDynamicSpriteEndFrame();

        // 4) spcProcess(): "alimenta" al SPC700 con los datos de audio que
        //    necesite en este instante (streaming de música). PVSnesLib exige
        //    llamarla una vez por fotograma si se está reproduciendo música o
        //    efectos, para que el sonido no se corte ni se desincronice.
        spcProcess();

        // 5) WaitForVBlank(): PAUSA la ejecución hasta el instante exacto en que
        //    empieza el borrado vertical (VBlank), el único momento del
        //    fotograma en que es seguro escribir en VRAM/CGRAM/OAM sin generar
        //    artefactos visuales ("snow"/parpadeos) en pantalla. Esta llamada es
        //    también la que fija el ritmo de 60 fotogramas por segundo de todo
        //    el juego: si el resto del bucle tarda menos de un fotograma (que es
        //    lo normal en un juego tan simple como este), aquí es donde se
        //    "espera" el tiempo restante.
        WaitForVBlank();

        // 6) mapVblank(): ahora que estamos dentro del VBlank, vuelca a VRAM (vía
        //    DMA) los cambios de mapa calculados por mapUpdate() en el paso 1.
        mapVblank();

        // 7) oamVramQueueUpdate(): igualmente ya dentro del VBlank, copia a VRAM
        //    los gráficos de sprite pendientes (los que se marcaron con
        //    oamrefresh=1 en algún oamDynamic*Draw de este fotograma, por
        //    ejemplo cuando Mario cambia de pose). El propio hardware, mediante
        //    otro mecanismo automático de PVSnesLib, se encarga de volcar
        //    oamMemory a la OAM real de la PPU en cada VBlank sin que este
        //    código tenga que pedirlo explícitamente.
        oamVramQueueUpdate();
    }
    // (Este "return 0" nunca se alcanza en la práctica: el bucle de arriba es
    // infinito, como corresponde a un programa que debe seguir "vivo" mientras la
    // consola esté encendida. Se deja por convención de C, ya que main() debe
    // devolver un int.)
    return 0;
}
