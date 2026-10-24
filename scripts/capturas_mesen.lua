-- capturas_mesen.lua — Guarda capturas del frame de la SNES cada N frames, sin
-- depender de la ventana de Mesen (sirve para verificar la secuencia de
-- arranque sin mirar la pantalla).
--
-- Uso:  Mesen.exe ProyectoSNES.sfc scripts/capturas_mesen.lua
-- Salida: scripts/capturas/frame_<n>.png (la carpeta la crea PowerShell antes)
-- Mesen 2 carga cualquier .lua pasado en la línea de comandos como script.

local CADA_FRAMES = 240          -- 240 frames = 4 s en NTSC
local MAX_FRAMES = 60 * 75       -- 75 s y se detiene
local carpeta = "E:/ProyectoSNES/scripts/capturas/"
local contador = 0

local function alFinalDeFrame()
    contador = contador + 1
    if contador % CADA_FRAMES == 0 then
        local png = emu.takeScreenshot()
        local seg = contador // 60
        local f = io.open(string.format("%sframe_%03d_t%02ds.png", carpeta, contador // CADA_FRAMES, seg), "wb")
        if f then
            f:write(png)
            f:close()
            emu.log("captura t=" .. seg .. "s")
        else
            emu.log("no pude escribir la captura (io bloqueado?)")
        end
    end
    if contador >= MAX_FRAMES then
        emu.log("fin de capturas")
        emu.stop()
    end
end

emu.addEventCallback(alFinalDeFrame, emu.eventType.endFrame)
emu.log("capturas_mesen.lua activo")
