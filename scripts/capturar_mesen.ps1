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
      "pulsa" teclas enviando mensajes WM_KEYDOWN/WM_KEYUP a la ventana de Mesen
      con PostMessage. Al arrancar activa Mesen UNA vez (le pasa el primer plano
      un instante y lo devuelve) y, con Mesen en primer plano, le da UN toque
      real de Shift (keybd_event): sin él PrintWindow devuelve la pantalla de
      juegos recientes en vez del juego. Shift solo no escribe nada. Todo lo
      demás son mensajes a la ventana de Mesen, que después se aparta fuera de
      la pantalla: ninguna tecla del guion puede ir a parar a otra aplicación.

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
      .\scripts\capturar_mesen.ps1 -Duracion 58 -Cada 0.1 -Desde 55.4 -Guion "55,55.3,Start"
        → pulsa Start en el título y captura 10 veces por segundo solo la entrada al nivel.
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
      Desde      segundo de la primera captura (0): para mirar de cerca un
                 instante con -Cada 0.1 sin llenar la carpeta con el arranque
      EnPantalla deja la ventana visible en vez de apartarla fuera de la pantalla
---------------------------------------------------------------------------------
#>
param(
    [string]$Rom = "E:\ProyectoSNES\ProyectoSNES.sfc",
    [double]$Cada = 4,
    [int]$Duracion = 40,
    [string]$Salida = "E:\ProyectoSNES\scripts\capturas",
    [int]$Espera = 3,
    [string]$Guion = "",
    [double]$Desde = 0,
    [switch]$EnPantalla
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
    [DllImport("user32.dll")] public static extern void keybd_event(byte bVk, byte bScan, uint dwFlags, UIntPtr dwExtraInfo);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, UIntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint pid);
    [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint idAttach, uint idAttachTo, bool fAttach);
    [DllImport("user32.dll")] public static extern IntPtr SetFocus(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr hWnd);
    [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
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

$WM_KEYDOWN = 0x100
$WM_KEYUP = 0x101

# Envía una tecla virtual como mensaje WM_KEYDOWN/WM_KEYUP directo a la ventana de Mesen.
# PostMessage no depende de qué ventana tenga el foco ni de dónde esté la ventana, y no
# toca el teclado real: nunca escribe en el editor del autor (el único keybd_event del
# script es el Shift de la activación, con Mesen en primer plano; ver más abajo).
function MensajeTecla([int]$vk, [bool]$ext, [bool]$abajo) {
    # lParam de WM_KEYDOWN: bits 0-15 repetición (1), 16-23 código de barrido, 24 tecla
    # extendida, 30 estado anterior y 31 transición (los dos últimos a 1 en WM_KEYUP).
    $scan = [CapturaWin32]::MapVirtualKey([uint32]$vk, 0)
    $lparam = [int64]1 -bor ([int64]$scan -shl 16)
    if ($ext) { $lparam = $lparam -bor ([int64]1 -shl 24) }
    if ($abajo) {
        [CapturaWin32]::PostMessage($script:ventana, $WM_KEYDOWN, [UIntPtr][uint64]$vk, [IntPtr]$lparam) | Out-Null
    } else {
        $lparam = $lparam -bor ([int64]1 -shl 30) -bor ([int64]1 -shl 31)
        [CapturaWin32]::PostMessage($script:ventana, $WM_KEYUP, [UIntPtr][uint64]$vk, [IntPtr]$lparam) | Out-Null
    }
}

function Tecla([string]$boton, [bool]$abajo) {
    $t = $TECLAS[$boton]
    if (-not $t) { Write-Warning "botón desconocido en el guion: $boton"; return }
    MensajeTecla $t.vk $t.ext $abajo
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

# Opciones de línea de comandos de Mesen 2:
#   --Emulation.EmulationSpeed=100  fuerza velocidad real (la configuración guardada del
#                                   autor tenía 75 %, y todo —física, sueño, tiempos— se
#                                   medía un 25 % más lento sin que la ROM tuviera culpa);
#   --Preferences.AllowBackgroundInput=true
#                                   acepta el mando aunque Mesen no sea la ventana activa:
#                                   el script devuelve el primer plano al autor, y con la
#                                   opción en false (la del autor) las teclas del guion se
#                                   perdían si el autor estaba usando el equipo (2026-10-19);
#   -donotsavesettings              no guarda nada en settings.json al salir (ni la posición
#                                   de la ventana fuera de pantalla ni estas opciones).
$proceso = Start-Process -FilePath "E:\ProyectoSNES\tools\mesen\Mesen.exe" -ArgumentList @("`"$Rom`"", "--Emulation.EmulationSpeed=100", "--Preferences.AllowBackgroundInput=true", "-donotsavesettings") -PassThru
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

# Activar una ventana: Windows solo deja cambiar el primer plano al hilo que "tiene la
# entrada". El truco clásico es engancharse (AttachThreadInput) al hilo de la ventana que
# está en primer plano, cambiar el foco y desengancharse. Devuelve si lo consiguió.
# No simula ninguna tecla real: si la activación falla, el script lo dice ("Mesen
# activada: False") y la salida es repetir la tanda, no escribir en otra ventana.
function Activar([IntPtr]$h) {
    $pid0 = [uint32]0
    $hiloActual = [CapturaWin32]::GetWindowThreadProcessId([CapturaWin32]::GetForegroundWindow(), [ref]$pid0)
    $miHilo = [CapturaWin32]::GetCurrentThreadId()
    if ($hiloActual -ne 0 -and $hiloActual -ne $miHilo) { [CapturaWin32]::AttachThreadInput($miHilo, $hiloActual, $true) | Out-Null }
    [CapturaWin32]::BringWindowToTop($h) | Out-Null
    [CapturaWin32]::SetForegroundWindow($h) | Out-Null
    [CapturaWin32]::SetFocus($h) | Out-Null
    if ($hiloActual -ne 0 -and $hiloActual -ne $miHilo) { [CapturaWin32]::AttachThreadInput($miHilo, $hiloActual, $false) | Out-Null }
    Start-Sleep -Milliseconds 150
    return ([CapturaWin32]::GetForegroundWindow() -eq $h)
}

# Mesen tiene que activarse UNA vez: hasta entonces el foco de teclado dentro de su ventana
# está en la barra de menús y los mensajes de tecla navegan el menú en vez de llegar al
# mando emulado. Después se devuelve el foco a la ventana que tenía el autor; Mesen recuerda
# su elemento con foco (el visor del juego) y sigue aceptando los mensajes de PostMessage.
$anterior = [CapturaWin32]::GetForegroundWindow()
$activada = $false
for ($k = 0; $k -lt 5 -and -not $activada; $k++) { $activada = Activar $ventana; if (-not $activada) { Start-Sleep -Milliseconds 300 } }
Write-Output "Mesen activada: $activada"
Start-Sleep -Milliseconds 400

# UN toque real de Shift (keybd_event), SOLO si Mesen quedó en primer plano: es lo único
# que hace que PrintWindow pase a capturar el lienzo del juego; sin él devuelve la pantalla
# de juegos recientes de Mesen. Se probó sin éxito con Shift por mensaje (WM_KEYDOWN), con
# SetWindowPos(SWP_FRAMECHANGED) + RedrawWindow + WM_NCACTIVATE, y con minimizar y
# restaurar (lienzo negro). Shift solo no escribe nada, y con Mesen en primer plano no
# puede llegar a otra aplicación; si la activación falló, no se envía.
if ($activada -and [CapturaWin32]::GetForegroundWindow() -eq $ventana) {
    [CapturaWin32]::keybd_event(0xA0, 0, 0, [UIntPtr]::Zero)
    [CapturaWin32]::keybd_event(0xA0, 0, $KEYEVENTF_KEYUP, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 150
} else {
    Write-Warning "Mesen no está en primer plano: las capturas pueden mostrar la pantalla de inicio de Mesen"
}

# Con la ventana activa, un clic de ratón por mensaje (WM_LBUTTONDOWN/UP, 0x201/0x202, en
# coordenadas del área cliente) sobre el visor del juego mueve el foco de teclado de la
# barra de menús al visor, sin tocar el ratón real. Tiene que ir ANTES de devolver el
# primer plano al autor: Avalonia solo atiende el clic mientras la ventana está activa.
# lParam = (y << 16) | x; wParam = MK_LBUTTON (1) al pulsar.
$posClic = [IntPtr]((250 -shl 16) -bor 250)
[CapturaWin32]::PostMessage($ventana, 0x201, [UIntPtr][uint64]1, $posClic) | Out-Null
Start-Sleep -Milliseconds 80
[CapturaWin32]::PostMessage($ventana, 0x202, [UIntPtr]::Zero, $posClic) | Out-Null
Start-Sleep -Milliseconds 400
if ($anterior -ne [IntPtr]::Zero -and $anterior -ne $ventana) { Activar $anterior | Out-Null }

# La ventana se lleva fuera de la pantalla (a la izquierda del monitor): DWM la sigue
# componiendo y PrintWindow la captura igual, pero no tapa el editor del autor ni le roba
# el foco, y nadie la minimiza por estorbar. 0x1 = SWP_NOSIZE, 0x4 = SWP_NOZORDER,
# 0x10 = SWP_NOACTIVATE.
if (-not $EnPantalla) {
    [CapturaWin32]::SetWindowPos($ventana, [IntPtr]::Zero, -2600, 100, 0, 0, 0x15) | Out-Null
} else {
    # -EnPantalla: la ventana se queda visible (abajo a la derecha) para medir si DWM
    # frena la emulación cuando la ventana no está en pantalla.
    [CapturaWin32]::SetWindowPos($ventana, [IntPtr]::Zero, 1300, 400, 0, 0, 0x15) | Out-Null
}
Start-Sleep -Milliseconds 300

function Capturar([IntPtr]$h, [string]$ruta) {
    $r = New-Object CapturaWin32+RECT
    [CapturaWin32]::GetWindowRect($h, [ref]$r) | Out-Null
    $ancho = $r.R - $r.L; $alto = $r.B - $r.T
    # Una ventana minimizada devuelve un rectángulo diminuto (solo la barra de título):
    # se restaura y se vuelve a medir, hasta 5 veces.
    for ($k = 0; $k -lt 5 -and $alto -lt 200; $k++) {
        [CapturaWin32]::ShowWindow($h, 9) | Out-Null              # SW_RESTORE
        Start-Sleep -Milliseconds 200
        [CapturaWin32]::GetWindowRect($h, [ref]$r) | Out-Null
        $ancho = $r.R - $r.L; $alto = $r.B - $r.T
    }
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
$proximaCaptura = $Desde          # Antes de -Desde no se captura (el guion sí corre).
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
        $ruta = Join-Path $Salida ("frame_{0:D3}_t{1}s.png" -f $n, ([math]::Floor($seg * 10) / 10).ToString("00.0", [cultureinfo]::InvariantCulture))
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
