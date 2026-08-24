/*---------------------------------------------------------------------------------
    Super Nintendo - Proyecto Plataformas
    Inicialización de Stack Tecnológico
---------------------------------------------------------------------------------*/
#include <snes.h>

int main(void)
{
    // Inicializar la consola de texto en el background 0
    consoleInitDefaultText(0);

    // Configurar punteros de gráficos y mapas para el background de texto
    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    // Poner la SNES en modo de video 1 y deshabilitar los backgrounds innecesarios
    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    // Dibujar textos informativos en pantalla
    consoleDrawText(8, 8, "PROYECTO SNES INICIADO");
    consoleDrawText(6, 12, "LENGUAJE: C (PVSNESLIB)");
    consoleDrawText(9, 14, "EMULADOR: MESEN");
    consoleDrawText(7, 18, "STACK TECNOLOGICO OK!");

    // Encender la pantalla
    setScreenOn();

    // Bucle principal
    while (1)
    {
        // Esperar al V-Blank para sincronizar a 60 FPS
        WaitForVBlank();
    }
    return 0;
}
