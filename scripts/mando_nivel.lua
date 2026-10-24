-- ---------------------------------------------------------------------------------
--   mando_nivel.lua — Simula el mando dentro de Mesen siguiendo un guion por frames,
--   para verificar el nivel sin manos (junto con scripts/capturar_mesen.ps1).
--
--   PROPÓSITO
--     Mesen bloquea la E/S de los scripts Lua, pero sí deja fijar el estado del
--     mando con emu.setInput() dentro del evento inputPolled (el momento en que
--     la consola lee el pad). Este script cuenta frames desde el arranque y, en
--     cada frame, pulsa los botones que el GUION indique. Así una misma
--     secuencia (entrar al nivel, caminar, saltar...) se repite exactamente igual
--     en cada verificación.
--
--   SE CONECTA CON
--     Se abre A MANO en Mesen: Debug → Script Window → File → Open, con la ROM
--     ya cargada. OJO: MesenCE 2.2.1 no carga la ROM si se le pasa un .lua por
--     línea de comandos (verificado el 2026-10-16), por eso las verificaciones
--     automáticas usan el guion de teclado de scripts/capturar_mesen.ps1 (-Guion)
--     y este script queda como alternativa manual y como ejemplo de la API de
--     entrada de Lua.
--     Lee: nada. Escribe: nada (solo emu.log, visible en la ventana de scripts).
--
--   HARDWARE QUE TOCA
--     El mando 1 emulado (puerto 0). Nombres de botones de Mesen para la SNES:
--     a, b, x, y, l, r, select, start, up, down, left, right.
--
--   VARIABLES
--     GUION   tabla   lista de tramos {desde, hasta, botones}: el tramo está activo
--                     mientras desde <= frame < hasta (frames desde el arranque)
--     frame   número  frames transcurridos desde que arrancó la ROM
--
--   Tiempos de referencia de la secuencia de arranque (60 frames = 1 s): el
--   título aparece hacia el frame 3600 (60 s). PERMITE_SALTO = 0, así que antes
--   no se puede acelerar nada.
-- ---------------------------------------------------------------------------------

local GUION = {
    { desde = 3700, hasta = 3701, botones = { start = true } },   -- Start en el título: entra al nivel
    { desde = 3760, hasta = 3960, botones = { right = true } },   -- 200 frames a la derecha
    { desde = 3960, hasta = 4140, botones = { left = true } },    -- 180 frames a la izquierda
    { desde = 4200, hasta = 4201, botones = { start = true } },   -- START en el nivel: vuelve al título
}

local frame = 0

-- Se ejecuta cada vez que la consola lee el mando: fijamos todos los botones
-- (en false los que el guion no nombra, para que el teclado no interfiera).
local function alLeerMando()
    local entrada = {
        a = false, b = false, x = false, y = false, l = false, r = false,
        select = false, start = false, up = false, down = false, left = false, right = false,
    }
    for _, tramo in ipairs(GUION) do
        if frame >= tramo.desde and frame < tramo.hasta then
            for boton, valor in pairs(tramo.botones) do
                entrada[boton] = valor
            end
        end
    end
    emu.setInput(entrada, 0)
end

local function alFinalDeFrame()
    frame = frame + 1
    if frame % 600 == 0 then
        emu.log("frame " .. frame .. " (" .. (frame // 60) .. " s)")
    end
end

emu.addEventCallback(alLeerMando, emu.eventType.inputPolled)
emu.addEventCallback(alFinalDeFrame, emu.eventType.endFrame)
emu.log("mando_nivel.lua: guion con " .. #GUION .. " tramos")
