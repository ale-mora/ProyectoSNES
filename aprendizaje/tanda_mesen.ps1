# Tanda de capturas de Mesen para las unidades de aprendizaje (no usar el PC mientras corre).
# Uso: powershell -File aprendizaje\tanda_mesen.ps1   (~20 min, 37 ROM)
# Las ROM las dejaron las sesiones que escribieron las unidades en E:\ProyectoSNES_trabajo\roms\
# (fuera del repo); si faltan, se rehacen con la solución de cada unidad. Cada ROM deja sus PNG
# en aprendizaje\unidad-NN-*\capturas\mesen_<rom>\. Lo que solo se ve en las ventanas del
# depurador (breakpoints, Memory Tools, profiler) está descrito en el ejercicio de cada unidad.
# -Solo u06_fondo_solucion,u09_metasprite   repite solo esas ROM (por nombre, sin .sfc).
param([string[]]$Solo = @())
$ErrorActionPreference = "Continue"
$R = "E:\ProyectoSNES_trabajo\roms"
$A = "E:\ProyectoSNES\aprendizaje"
$cap = "E:\ProyectoSNES\scripts\capturar_mesen.ps1"
$tanda = @(
  @{u="unidad-03-tuberia-de-compilacion"; rom="u03_hola_mundo"; a=@("-Duracion",4,"-Cada",2,"-Espera",3)},
  @{u="unidad-04-frame-y-vblank"; rom="u04_contador"; a=@("-Duracion",6,"-Cada",1,"-Espera",3)},
  @{u="unidad-04-frame-y-vblank"; rom="u04_contador_parpadeo"; a=@("-Duracion",3,"-Cada",0.25,"-Espera",3)},
  @{u="unidad-04-frame-y-vblank"; rom="u04_contador_nmi_propia"; a=@("-Duracion",4,"-Cada",1,"-Espera",3)},
  @{u="unidad-04-frame-y-vblank"; rom="u04_tutorial_hud"; a=@("-Duracion",46,"-Cada",2,"-Espera",3)},
  @{u="unidad-05-mando"; rom="u05_contador_b_guion"; a=@("-Duracion",9,"-Cada",0.25,"-Espera",2)},
  @{u="unidad-05-mando"; rom="u05_contador_b"; a=@("-Duracion",6,"-Cada",0.5,"-Espera",3,"-Guion","1,2.5,B;1.5,2.5,Right;3,3.2,B;4,4.2,B")},
  @{u="unidad-06-tiles-y-fondos"; rom="u06_fondo_solucion"; a=@("-Duracion",8,"-Cada",1,"-Espera",3)},
  @{u="unidad-07-scroll-y-camara"; rom="u07_parallax_solucion"; a=@("-Duracion",14,"-Cada",1,"-Espera",3,"-Guion","1,1.3,A;2,12,Right;6,12,Y;12.5,12.8,A")},
  @{u="unidad-08-sprites-y-oam"; rom="u08_sprites_solucion"; a=@("-Duracion",6,"-Cada",1,"-Espera",3,"-Guion","2,4,Left")},
  @{u="unidad-08-sprites-y-oam"; rom="u08_sprites_cruce192"; a=@("-Duracion",4,"-Cada",0.5,"-Espera",3)},
  @{u="unidad-08-sprites-y-oam"; rom="u08_sprites_y240"; a=@("-Duracion",3,"-Cada",1,"-Espera",3)},
  @{u="unidad-09-metasprites"; rom="u09_metasprite"; a=@("-Duracion",16,"-Cada",1,"-Espera",3,"-Guion","4,7,Right;8,11,Left;12,16,Down")},
  @{u="unidad-09-metasprites"; rom="u09_metasprite_solucion"; a=@("-Duracion",16,"-Cada",1,"-Espera",3,"-Guion","4,7,Right;8,11,Left;12,16,Down")},
  @{u="unidad-10-fisica"; rom="u10_salto_g6_v120_guion"; a=@("-Duracion",46,"-Cada",2,"-Espera",3,"-Guion","1,1.3,Select")},
  @{u="unidad-11-animacion"; rom="u11_ritmo_espada_guion"; a=@("-Duracion",29,"-Cada",0.1,"-Desde",25,"-Espera",3,"-Guion","1,1.3,Select")},
  @{u="unidad-11-animacion"; rom="u11_ritmo_espada_normal"; a=@("-Duracion",73,"-Cada",0.1,"-Desde",69.5,"-Espera",4,"-Guion","55,55.3,Start;70,70.3,Right")},
  @{u="unidad-12-hdma-y-efectos"; rom="u12_cielo_solucion"; a=@("-Duracion",22,"-Cada",0.5,"-Espera",3)},
  @{u="unidad-12-hdma-y-efectos"; rom="u12_publisher_escena"; a=@("-Duracion",50,"-Cada",2,"-Espera",4)},
  @{u="unidad-13-memoria"; rom="u13_memoria_guion"; a=@("-Duracion",12,"-Cada",2,"-Espera",3)},
  @{u="unidad-16-mesen"; rom="u16_caso1_barbara192"; a=@("-Duracion",56,"-Cada",1,"-Desde",34,"-Espera",4)},
  @{u="unidad-16-mesen"; rom="u16_normal"; a=@("-Duracion",56,"-Cada",1,"-Desde",34,"-Espera",4)},
  @{u="unidad-16-mesen"; rom="u16_caso2_pies_flotan"; a=@("-Duracion",8,"-Cada",1,"-Espera",3)},
  @{u="unidad-16-mesen"; rom="u16_caso3_hud_sin_prioridad"; a=@("-Duracion",26,"-Cada",1,"-Desde",12,"-Espera",3)},
  @{u="unidad-16-mesen"; rom="u16_guion_hud"; a=@("-Duracion",46,"-Cada",2,"-Espera",3)},
  @{u="unidad-17-audio"; rom="u17_audio"; a=@("-Duracion",10,"-Cada",1,"-Espera",3,"-Guion","4,4.2,Up;5,5.2,Up;6,6.2,Down;7,7.2,Start")},
  @{u="unidad-18-herramientas"; rom="u18_guardia"; a=@("-Duracion",64,"-Cada",1,"-Desde",56,"-Espera",4,"-Guion","55,55.3,Start;58,58.3,R")},
  @{u="unidad-18-herramientas"; rom="u18_guardia_guion"; a=@("-Duracion",30,"-Cada",0.25,"-Desde",25,"-Espera",3)},
  @{u="unidad-19-del-arte-al-juego"; rom="u19_hundir0_guion"; a=@("-Duracion",29,"-Cada",1,"-Desde",25,"-Espera",3)},
  @{u="unidad-19-del-arte-al-juego"; rom="u19_hundir2_guion"; a=@("-Duracion",29,"-Cada",1,"-Desde",25,"-Espera",3)},
  @{u="unidad-19-del-arte-al-juego"; rom="u19_hundir4_guion"; a=@("-Duracion",29,"-Cada",1,"-Desde",25,"-Espera",3)},
  @{u="unidad-19-del-arte-al-juego"; rom="u19_zarza_guion"; a=@("-Duracion",29,"-Cada",1,"-Desde",12,"-Espera",3)},
  @{u="unidad-20-colision"; rom="u20_foso2_guion"; a=@("-Duracion",46,"-Cada",1,"-Desde",30,"-Espera",3)},
  @{u="unidad-12-hdma-y-efectos"; rom="juego_linea223"; a=@("-Duracion",52,"-Cada",2,"-Espera",4)},
  @{u="unidad-15-asm-escribir"; rom="u15_relleno"; a=@("-Duracion",6,"-Cada",2,"-Espera",3)},
  @{u="unidad-15-asm-escribir"; rom="u15_guion_hud_c"; a=@("-Duracion",46,"-Cada",2,"-Espera",3,"-Guion","1,1.3,Select")},
  @{u="unidad-15-asm-escribir"; rom="u15_guion_hud_asm"; a=@("-Duracion",46,"-Cada",2,"-Espera",3,"-Guion","1,1.3,Select")},
  @{u="unidad-15-asm-escribir"; rom="u15_guion_asm_completo"; a=@("-Duracion",46,"-Cada",2,"-Espera",3,"-Guion","1,1.3,Select")}
)
$Solo = @($Solo | ForEach-Object { $_ -split "," } | Where-Object { $_ })   # Con -File llega como una sola cadena.
if ($Solo.Count -gt 0) { $tanda = @($tanda | Where-Object { $Solo -contains $_.rom }) }
$i = 0
foreach ($t in $tanda) {
  $i++
  $sal = Join-Path $A "$($t.u)\capturas\mesen_$($t.rom)"
  Remove-Item -Recurse -Force $sal -ErrorAction SilentlyContinue
  Write-Host "[$i/$($tanda.Count)] $($t.rom)"
  # Los parámetros van por NOMBRE (un hashtable): con una lista, PowerShell los toma por posición.
  $p = @{ Rom = "$R\$($t.rom).sfc"; Salida = $sal }
  for ($k = 0; $k -lt $t.a.Count; $k += 2) { $p[$t.a[$k].TrimStart("-")] = $t.a[$k + 1] }
  & $cap @p *> "$env:TEMP\tanda_$($t.rom).log"
  Get-Process Mesen -ErrorAction SilentlyContinue | Stop-Process -Force
  Start-Sleep -Seconds 1
  $n = (Get-ChildItem $sal -Filter *.png -ErrorAction SilentlyContinue).Count
  Write-Host "    $n capturas"
}
