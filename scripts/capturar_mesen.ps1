<#
---------------------------------------------------------------------------------
    capturar_mesen.ps1 — Lanza Mesen con la ROM y guarda capturas de su ventana
    cada N segundos, sin depender de la E/S de los scripts Lua (que Mesen
    bloquea por defecto).

    PROPÓSITO
      Verificar la ROM "sin mirar la pantalla": el agente o el autor lanzan este
      script, esperan, y revisan los PNG. Usa la API de Windows PrintWindow
      (captura el contenido de la ventana aunque esté tapada) y, si sale negra
      (Mesen dibuja con la GPU), cae a copiar la región de pantalla.

    SE CONECTA CON
      Lee:     ProyectoSNES.sfc (la ROM) y, opcionalmente, un script Lua que
               Mesen carga al arrancar (por ejemplo para simular el mando).
      Escribe: scripts/capturas/frame_<n>_t<seg>s.png
      Lo usan: docs/plan-implementacion-revision-1.md (pasos "Mesen" de cada
               tarea) y docs/mesen-y-depuracion.md.

    USO (desde PowerShell, en la raíz del repo)
      .\scripts\capturar_mesen.ps1                       # 40 s, una captura cada 4 s
      .\scripts\capturar_mesen.ps1 -Duracion 20 -Cada 2
      .\scripts\capturar_mesen.ps1 -Lua scripts\mando_nivel.lua   # con script Lua

    VARIABLES (parámetros)
      Rom        ruta de la ROM
      Lua        ruta de un script Lua que Mesen cargará (vacío = ninguno)
      Cada       segundos entre capturas
      Duracion   segundos totales antes de cerrar Mesen
      Salida     carpeta de los PNG (se crea si no existe)
      Espera     segundos de espera inicial para que aparezca la ventana
---------------------------------------------------------------------------------
#>
param(
    [string]$Rom = "E:\ProyectoSNES\ProyectoSNES.sfc",
    [string]$Lua = "",
    [int]$Cada = 4,
    [int]$Duracion = 40,
    [string]$Salida = "E:\ProyectoSNES\scripts\capturas",
    [int]$Espera = 3
)

# System.Drawing da mapas de bits y PNG; user32 da PrintWindow y el rectángulo de la ventana.
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class CapturaWin32 {
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdc, uint flags);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT r);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
"@

# Sin esto Windows "virtualiza" las coordenadas al 100 % de escala y la captura sale recortada
# cuando la pantalla usa 125 % o 150 % (GetWindowRect y PrintWindow no coinciden).
[CapturaWin32]::SetProcessDPIAware() | Out-Null

New-Item -ItemType Directory -Force $Salida | Out-Null

# Argumentos de Mesen: la ROM y, si se pidió, el script Lua (Mesen carga cualquier .lua de la línea de comandos).
$argumentos = @("`"$Rom`"")
if ($Lua -ne "") { $argumentos += "`"$Lua`"" }
$proceso = Start-Process -FilePath "E:\ProyectoSNES\tools\mesen\Mesen.exe" -ArgumentList $argumentos -PassThru
Start-Sleep -Seconds $Espera

# El identificador de la ventana principal puede tardar en existir: lo pedimos hasta tenerlo.
$ventana = [IntPtr]::Zero
for ($i = 0; $i -lt 20 -and $ventana -eq [IntPtr]::Zero; $i++) {
    $proceso.Refresh()
    $ventana = $proceso.MainWindowHandle
    if ($ventana -eq [IntPtr]::Zero) { Start-Sleep -Milliseconds 500 }
}
if ($ventana -eq [IntPtr]::Zero) { Write-Error "No encontré la ventana de Mesen"; Stop-Process -Id $proceso.Id -Force; exit 1 }
[CapturaWin32]::SetForegroundWindow($ventana) | Out-Null

function Capturar([IntPtr]$h, [string]$ruta) {
    $r = New-Object CapturaWin32+RECT
    [CapturaWin32]::GetWindowRect($h, [ref]$r) | Out-Null
    $ancho = $r.R - $r.L; $alto = $r.B - $r.T
    if ($ancho -le 0 -or $alto -le 0) { return "ventana sin tamaño" }
    $bmp = New-Object System.Drawing.Bitmap $ancho, $alto
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $g.GetHdc()
    # 2 = PW_RENDERFULLCONTENT: pide a DWM el contenido compuesto, incluido el lienzo de la GPU
    # con el que Mesen dibuja. No usamos CopyFromScreen: una pantalla negra (fundidos,
    # dedicatoria) es una captura válida, no un fallo.
    $ok = [CapturaWin32]::PrintWindow($h, $hdc, 2)
    $g.ReleaseHdc($hdc)
    $g.Dispose()
    if (-not $ok) { $bmp.Dispose(); return "PrintWindow falló" }
    $bmp.Save($ruta, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    return "ok ${ancho}x${alto}"
}

$inicio = Get-Date
$n = 0
while (((Get-Date) - $inicio).TotalSeconds -lt $Duracion) {
    $seg = [int](((Get-Date) - $inicio).TotalSeconds)
    $n++
    $ruta = Join-Path $Salida ("frame_{0:D3}_t{1:D2}s.png" -f $n, $seg)
    $res = Capturar $ventana $ruta
    Write-Output ("captura {0} t={1}s: {2}" -f $n, $seg, $res)
    Start-Sleep -Seconds $Cada
}

if (-not $proceso.HasExited) { Stop-Process -Id $proceso.Id -Force }
Write-Output "Mesen cerrado; $n capturas en $Salida"
