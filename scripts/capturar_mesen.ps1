<#
---------------------------------------------------------------------------------
    capturar_mesen.ps1 — Lanza Mesen con la ROM, simula el mando con el teclado
    siguiendo un guion por segundos y guarda capturas de la ventana cada N
    segundos. Todo sin manos y sin depender de la E/S de los scripts Lua (que
    Mesen bloquea) ni de cargar un .lua por línea de comandos (MesenCE 2.2.1 no
    carga la ROM si se le pasa un .lua).

    PROPÓSITO
      Verificar la ROM "sin mirar la pantalla": el agente o el autor lanzan este
      script, esperan, y revisan los PNG. Captura con la API de Windows
      PrintWindow (contenido compuesto por DWM, incluido el lienzo de la GPU) y
      pulsa teclas con keybd_event sobre la ventana de Mesen en primer plano.

    SE CONECTA CON
      Lee:     ProyectoSNES.sfc (la ROM).
      Escribe: scripts/capturas/frame_<n>_t<seg>s.png
      Usa:     el mapeo de teclado de MesenCE (Settings → Input, Mapping2):
               flechas = cruceta, S = A, A = B, X = X, Z = Y, Q = L, W = R,
               D = Start, E = Select.
      Lo usan: docs/plan-implementacion-revision-1.md (pasos "Mesen" de cada
               tarea) y docs/mesen-y-depuracion.md.

    USO (desde PowerShell, en la raíz del repo)
      .\scripts\capturar_mesen.ps1                       # 40 s, una captura cada 4 s
      .\scripts\capturar_mesen.ps1 -Duracion 20 -Cada 2
      .\scripts\capturar_mesen.ps1 -Duracion 90 -Cada 2 -Guion "76,76.2,Start;78,81,Right;81,84,Left"
        → a los 76 s pulsa Start 0,2 s; de 78 a 81 mantiene Derecha; de 81 a 84, Izquierda.
      Los segundos del guion y de las capturas cuentan desde el fin de la espera
      inicial (-Espera), no desde el arranque del proceso.

    VARIABLES (parámetros)
      Rom        ruta de la ROM
      Cada       segundos entre capturas
      Duracion   segundos totales antes de cerrar Mesen
      Salida     carpeta de los PNG (se crea si no existe)
      Espera     segundos de espera inicial para que aparezca la ventana
      Guion      tramos "desde,hasta,tecla" separados por ";" (tecla: Up, Down,
                 Left, Right, A, B, X, Y, L, R, Start, Select, en nombres de la SNES)
---------------------------------------------------------------------------------
#>
param(
    [string]$Rom = "E:\ProyectoSNES\ProyectoSNES.sfc",
    [int]$Cada = 4,
    [int]$Duracion = 40,
    [string]$Salida = "E:\ProyectoSNES\scripts\capturas",
    [int]$Espera = 3,
    [string]$Guion = ""
)

# System.Drawing da mapas de bits y PNG; user32 da PrintWindow, el rectángulo de la
# ventana, el primer plano y la inyección de teclas.
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class CapturaWin32 {
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdc, uint flags);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT r);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern void keybd_event(byte bVk, byte bScan, uint dwFlags, UIntPtr dwExtraInfo);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, UIntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint uCode, uint uMapType);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
"@

# Sin esto Windows "virtualiza" las coordenadas al 100 % de escala y la captura sale recortada
# cuando la pantalla usa 125 % o 150 % (GetWindowRect y PrintWindow no coinciden).
[CapturaWin32]::SetProcessDPIAware() | Out-Null

# Botón de la SNES → tecla virtual de Windows (según el mapeo de teclado de MesenCE) y si
# es "tecla extendida" (las flechas lo son: hay que avisarlo en el flag).
$TECLAS = @{
    Up     = @{ vk = 0x26; ext = $true  }; Down  = @{ vk = 0x28; ext = $true  }
    Left   = @{ vk = 0x25; ext = $true  }; Right = @{ vk = 0x27; ext = $true  }
    A      = @{ vk = 0x53; ext = $false }; B     = @{ vk = 0x41; ext = $false }   # S, A
    X      = @{ vk = 0x58; ext = $false }; Y     = @{ vk = 0x5A; ext = $false }   # X, Z
    L      = @{ vk = 0x51; ext = $false }; R     = @{ vk = 0x57; ext = $false }   # Q, W
    Start  = @{ vk = 0x44; ext = $false }; Select = @{ vk = 0x45; ext = $false }  # D, E
}
$KEYEVENTF_EXTENDEDKEY = 0x1
$KEYEVENTF_KEYUP = 0x2

# Windows solo deja cambiar la ventana activa si el proceso "acaba de recibir entrada":
# un toque de tecla antes de SetForegroundWindow lo consigue. Se usa Shift, que Mesen no
# tiene mapeado (Alt abriría su barra de menús). Se llama antes de cada tecla, porque otra
# ventana (el editor, una notificación) puede robar el foco en cualquier momento.
function Enfocar([IntPtr]$h) {
    [CapturaWin32]::keybd_event(0xA0, 0, 0, [UIntPtr]::Zero)                  # Shift abajo
    [CapturaWin32]::keybd_event(0xA0, 0, $KEYEVENTF_KEYUP, [UIntPtr]::Zero)   # Shift arriba
    [CapturaWin32]::SetForegroundWindow($h) | Out-Null
    Start-Sleep -Milliseconds 60
}

$WM_KEYDOWN = 0x100
$WM_KEYUP = 0x101

# Envía la tecla de dos formas: como mensaje WM_KEYDOWN/WM_KEYUP directo a la ventana de
# Mesen (PostMessage: no depende de qué ventana tenga el foco) y, por si acaso, como
# pulsación global (keybd_event) tras poner Mesen en primer plano.
function Tecla([string]$boton, [bool]$abajo) {
    $t = $TECLAS[$boton]
    if (-not $t) { Write-Warning "botón desconocido en el guion: $boton"; return }
    if ($abajo) { Enfocar $script:ventana }
    # lParam de WM_KEYDOWN: bits 0-15 repetición (1), 16-23 código de barrido, 24 tecla
    # extendida, 30 estado anterior y 31 transición (los dos últimos a 1 en WM_KEYUP).
    $scan = [CapturaWin32]::MapVirtualKey([uint32]$t.vk, 0)
    $lparam = [int64]1 -bor ([int64]$scan -shl 16)
    if ($t.ext) { $lparam = $lparam -bor ([int64]1 -shl 24) }
    if ($abajo) {
        [CapturaWin32]::PostMessage($script:ventana, $WM_KEYDOWN, [UIntPtr][uint64]$t.vk, [IntPtr]$lparam) | Out-Null
    } else {
        $lparam = $lparam -bor ([int64]1 -shl 30) -bor ([int64]1 -shl 31)
        [CapturaWin32]::PostMessage($script:ventana, $WM_KEYUP, [UIntPtr][uint64]$t.vk, [IntPtr]$lparam) | Out-Null
    }
    $flags = 0
    if ($t.ext) { $flags = $flags -bor $KEYEVENTF_EXTENDEDKEY }
    if (-not $abajo) { $flags = $flags -bor $KEYEVENTF_KEYUP }
    [CapturaWin32]::keybd_event([byte]$t.vk, 0, [uint32]$flags, [UIntPtr]::Zero)
}

# Guion: lista de tramos con su estado (pulsada / soltada).
$tramos = @()
if ($Guion -ne "") {
    foreach ($trozo in $Guion.Split(";")) {
        $partes = $trozo.Trim().Split(",")
        if ($partes.Count -ne 3) { Write-Error "tramo mal formado: '$trozo' (esperaba desde,hasta,tecla)"; exit 1 }
        $tramos += [pscustomobject]@{ desde = [double]$partes[0]; hasta = [double]$partes[1]; boton = $partes[2].Trim(); pulsada = $false; soltada = $false }
    }
}

New-Item -ItemType Directory -Force $Salida | Out-Null

$proceso = Start-Process -FilePath "E:\ProyectoSNES\tools\mesen\Mesen.exe" -ArgumentList @("`"$Rom`"") -PassThru
Start-Sleep -Seconds $Espera

# El identificador de la ventana principal puede tardar en existir: lo pedimos hasta tenerlo.
$ventana = [IntPtr]::Zero
for ($i = 0; $i -lt 20 -and $ventana -eq [IntPtr]::Zero; $i++) {
    $proceso.Refresh()
    $ventana = $proceso.MainWindowHandle
    if ($ventana -eq [IntPtr]::Zero) { Start-Sleep -Milliseconds 500 }
}
if ($ventana -eq [IntPtr]::Zero) { Write-Error "No encontré la ventana de Mesen"; Stop-Process -Id $proceso.Id -Force; exit 1 }
$script:ventana = $ventana
[CapturaWin32]::ShowWindow($ventana, 9) | Out-Null          # 9 = SW_RESTORE: si Mesen arrancó minimizado, lo restaura.
Start-Sleep -Milliseconds 300
Enfocar $ventana                                            # Las teclas van a la ventana activa.
# Hasta que Mesen recibe una primera tecla, PrintWindow devuelve una capa antigua de la
# ventana (su pantalla de juegos recientes) en vez del juego. Un toque de Shift, que no
# está mapeado a nada, lo "despierta" sin tocar el mando emulado.
Start-Sleep -Milliseconds 300
[CapturaWin32]::keybd_event(0xA0, 0, 0, [UIntPtr]::Zero)                  # Shift izquierdo abajo
[CapturaWin32]::keybd_event(0xA0, 0, $KEYEVENTF_KEYUP, [UIntPtr]::Zero)   # y arriba

function Capturar([IntPtr]$h, [string]$ruta) {
    $r = New-Object CapturaWin32+RECT
    [CapturaWin32]::GetWindowRect($h, [ref]$r) | Out-Null
    $ancho = $r.R - $r.L; $alto = $r.B - $r.T
    if ($ancho -le 0 -or $alto -le 0) { return "ventana sin tamaño" }
    $bmp = New-Object System.Drawing.Bitmap $ancho, $alto
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $g.GetHdc()
    # 2 = PW_RENDERFULLCONTENT: pide a DWM el contenido compuesto, incluido el lienzo de la GPU
    # con el que Mesen dibuja. Una pantalla negra (fundidos) es una captura válida, no un fallo.
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
$proximaCaptura = 0.0
while ($true) {
    $seg = ((Get-Date) - $inicio).TotalSeconds
    if ($seg -ge $Duracion) { break }

    # Guion: pulsar al llegar a "desde", soltar al llegar a "hasta".
    foreach ($tr in $tramos) {
        if (-not $tr.pulsada -and $seg -ge $tr.desde) { Tecla $tr.boton $true; $tr.pulsada = $true; Write-Output ("tecla {0} abajo t={1:N1}s" -f $tr.boton, $seg) }
        if ($tr.pulsada -and -not $tr.soltada -and $seg -ge $tr.hasta) { Tecla $tr.boton $false; $tr.soltada = $true; Write-Output ("tecla {0} arriba t={1:N1}s" -f $tr.boton, $seg) }
    }

    # Capturas a intervalos de $Cada segundos.
    if ($seg -ge $proximaCaptura) {
        $n++
        $ruta = Join-Path $Salida ("frame_{0:D3}_t{1:D2}s.png" -f $n, [int]$seg)
        $res = Capturar $ventana $ruta
        Write-Output ("captura {0} t={1}s: {2}" -f $n, [int]$seg, $res)
        $proximaCaptura += $Cada
    }
    Start-Sleep -Milliseconds 40
}

# Soltar cualquier tecla que haya quedado pulsada (por si el guion termina después de la duración).
foreach ($tr in $tramos) { if ($tr.pulsada -and -not $tr.soltada) { Tecla $tr.boton $false } }

if (-not $proceso.HasExited) { Stop-Process -Id $proceso.Id -Force }
Write-Output "Mesen cerrado; $n capturas en $Salida"
