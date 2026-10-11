# Ensayo del lanzador con No Man's Sky, sin manos (PowerShell 5.1, solo ASCII).
#
#   powershell -ExecutionPolicy Bypass -File tools\ensayo_nms.ps1 -SinJuego    (solo la pantalla del lanzador)
#   powershell -ExecutionPolicy Bypass -File tools\ensayo_nms.ps1              (ademas abre y cierra el juego)
#
# Abre el paquete publicado (publish\SplitForge), elige No Man's Sky en la biblioteca y lee su preparacion.
# Sin -SinJuego, ademas pulsa JUGAR (abre la copia de Steam a traves de Steam), comprueba que el lanzador le
# asigna su mando y la cierra con "Cerrar juegos" (cierre normal, desde el menu del juego: no entra en partida).
# No pulsa "Preparar ahora": el filtro de mandos solo lo instala el propietario.
# Captura SOLO la ventana de SplitForge y la del juego. Restaura settings.v2.json al terminar.
# AVISAR al propietario antes de ejecutarlo sin -SinJuego: abre el juego y Steam.
param([string]$Repo = (Split-Path $PSScriptRoot -Parent), [string]$Out = (Join-Path $env:TEMP 'SplitForge-nms'), [switch]$SinJuego)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes, System.Drawing
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class L {
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public static string Capture(IntPtr h, string path, int maxWidth) {
    RECT r; GetWindowRect(h, out r); int w = r.R - r.L, hh = r.B - r.T;
    using (var bmp = new Bitmap(w, hh, PixelFormat.Format32bppArgb)) {
      using (var g = Graphics.FromImage(bmp)) { IntPtr dc = g.GetHdc(); PrintWindow(h, dc, 2); g.ReleaseHdc(dc); }
      int ow = Math.Min(maxWidth, w), oh = (int)((long)hh * ow / w);
      using (var small = new Bitmap(bmp, ow, oh)) small.Save(path, ImageFormat.Png);
      return w + "x" + hh;
    }
  }
}
'@
[void][L]::SetProcessDPIAware()
$AE = [Windows.Automation.AutomationElement]
$scope = [Windows.Automation.TreeScope]::Descendants
function Find-One($root, $type, $name) {
    $root.FindFirst($scope, (New-Object Windows.Automation.AndCondition((New-Object Windows.Automation.PropertyCondition($AE::ControlTypeProperty, $type)), (New-Object Windows.Automation.PropertyCondition($AE::NameProperty, $name)))))
}
function Texts($root, $pattern) {
    $root.FindAll($scope, (New-Object Windows.Automation.PropertyCondition($AE::ControlTypeProperty, [Windows.Automation.ControlType]::Text))) |
        ForEach-Object { $_.Current.Name } | Where-Object { $_ -match $pattern } | Select-Object -Unique
}
function App-Log([int]$tail, [string]$pattern) { Get-Content (Join-Path $env:LOCALAPPDATA 'SplitForge\splitforge-net.log') -Tail $tail | Where-Object { $_ -match $pattern } | ForEach-Object { "    " + $_.Substring(37) } }

if (Get-Process NMS, 7DaysToDie, SplitForge -ErrorAction SilentlyContinue) { throw 'Hay un juego o un SplitForge abierto: no se toca nada.' }
New-Item -ItemType Directory -Force $Out | Out-Null
$settings = Join-Path $env:LOCALAPPDATA 'SplitForge\settings.v2.json'
$backup = Join-Path $Out 'settings.v2.json.respaldo'
Copy-Item $settings $backup -Force
$app = $null; $root = $null
try {
    $folder = Join-Path $Repo 'publish\SplitForge'
    $app = Start-Process (Join-Path $folder 'SplitForge.exe') -WorkingDirectory $folder -PassThru
    $deadline = (Get-Date).AddSeconds(40)
    do { Start-Sleep -Milliseconds 500; $app.Refresh() } while ($app.MainWindowHandle -eq 0 -and (Get-Date) -lt $deadline)
    if ($app.MainWindowHandle -eq 0) { throw 'La ventana de SplitForge no aparecio.' }
    Start-Sleep -Seconds 5
    $root = $AE::FromHandle($app.MainWindowHandle)
    "biblioteca: " + (($root.FindAll($scope, (New-Object Windows.Automation.PropertyCondition($AE::ControlTypeProperty, [Windows.Automation.ControlType]::ListItem))) | ForEach-Object { $_.Current.Name } | Where-Object { $_ -match 'Days|Sky' }) -join ' | ')
    "--- con 7 Days to Die (elegido por defecto)"
    Texts $root 'PANTALLA DIVIDIDA|EN PREPARACI|Todo listo|Falta preparar|^Juego|Identidad|mando por ventana|^Mandos$|^Partida$|^Copias del juego$' | ForEach-Object { "    $_" }
    $card = Find-One $root ([Windows.Automation.ControlType]::ListItem) "No Man's Sky"
    if (-not $card) { throw "No aparece No Man's Sky en la biblioteca." }
    $card.GetCurrentPattern([Windows.Automation.SelectionItemPattern]::Pattern).Select()
    Start-Sleep -Seconds 3
    "--- con No Man's Sky"
    Texts $root 'PANTALLA DIVIDIDA|EN PREPARACI|Todo listo|Falta preparar|^Juego|Identidad|mando por ventana|^Mandos$|^Partida$|^Copias del juego$|^J1: |Con su cuenta|Con la cuenta|Steam Input|su propia copia|Falta instalar|mandos conectados' | ForEach-Object { "    " + $_.Substring(0, [Math]::Min(150, $_.Length)) }
    "    botones: " + (($root.FindAll($scope, (New-Object Windows.Automation.PropertyCondition($AE::ControlTypeProperty, [Windows.Automation.ControlType]::Button))) | ForEach-Object { $_.Current.Name } | Where-Object { $_ -match 'Jugar|Cerrar juegos|Preparar|Elegir copia|Quitar filtro' }) -join ' | ')
    "    captura del lanzador: " + [L]::Capture($app.MainWindowHandle, (Join-Path $Out 'lanzador-nms.png'), 1400)

    # AVANZADO > NO MAN'S SKY: las mismas herramientas que tiene 7 Days to Die en su pestana.
    function Select-Tab($name) {
        $tab = Find-One $root ([Windows.Automation.ControlType]::TabItem) $name
        if (-not $tab) { throw "No aparece la pestana $name." }
        $tab.GetCurrentPattern([Windows.Automation.SelectionItemPattern]::Pattern).Select()
        Start-Sleep -Seconds 1
    }
    Select-Tab 'AVANZADO'
    "--- AVANZADO: " + (($root.FindAll($scope, (New-Object Windows.Automation.PropertyCondition($AE::ControlTypeProperty, [Windows.Automation.ControlType]::TabItem))) | ForEach-Object { $_.Current.Name } | Where-Object { $_ -notmatch '^(JUGAR|MANDOS|AVANZADO)$' }) -join ' | ')
    Select-Tab "NO MAN'S SKY"
    Start-Sleep -Seconds 4
    Texts $root 'Emulador de Steam|^Copias del juego$|^Filtro de mandos$|^Abrir y cerrar$|^J1: |no instalado|Instalado en|Ranuras XInput|no detectado' | ForEach-Object { "    " + $_.Substring(0, [Math]::Min(150, $_.Length)) }
    "    botones: " + (($root.FindAll($scope, (New-Object Windows.Automation.PropertyCondition($AE::ControlTypeProperty, [Windows.Automation.ControlType]::Button))) | ForEach-Object { $_.Current.Name } | Where-Object { $_ -match 'Elegir copia|Instalar|Asignar mandos|Quitar filtro|Lanzar|Dividir pantalla|Cerrar juegos' }) -join ' | ')
    "    captura de AVANZADO: " + [L]::Capture($app.MainWindowHandle, (Join-Path $Out 'avanzado-nms.png'), 1400)
    Select-Tab 'JUGAR'
    if ($SinJuego) { return }

    $play = Find-One $root ([Windows.Automation.ControlType]::Button) 'Jugar'
    $play.GetCurrentPattern([Windows.Automation.InvokePattern]::Pattern).Invoke()
    "pulsado JUGAR"
    $deadline = (Get-Date).AddSeconds(240)
    do { Start-Sleep -Seconds 2; $g = Get-Process NMS -ErrorAction SilentlyContinue | Select-Object -First 1 } while (-not $g -and (Get-Date) -lt $deadline)
    if (-not $g) { throw 'NMS no arranco.' }
    "NMS pid=$($g.Id)"
    $deadline = (Get-Date).AddSeconds(120); $done = $false
    do { Start-Sleep -Seconds 3; $done = [bool](Get-Content (Join-Path $env:LOCALAPPDATA 'SplitForge\splitforge-net.log') -Tail 8 | Select-String -Pattern 'abierto J1 con su mando' -Quiet) } while (-not $done -and (Get-Date) -lt $deadline)
    "el lanzador dio por abierto a J1: $done"
    App-Log 10 "No Man|Foco simulado|barra de tareas"
    "    asignacion.txt: " + ((Get-Content (Join-Path $env:LOCALAPPDATA 'SplitForge\xinput\asignacion.txt') -ErrorAction SilentlyContinue) -join ' ; ')
    Start-Sleep -Seconds 12
    $g.Refresh()
    "    captura del juego: " + [L]::Capture($g.MainWindowHandle, (Join-Path $Out 'nms-lanzador.png'), 1280)

    # "Cerrar juegos" abre una confirmacion modal: un proceso aparte pulsa "Si" mientras esta llamada espera.
    $clicker = Start-Job -ArgumentList $app.Id -ScriptBlock {
        param($appPid)
        Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes
        $AE = [Windows.Automation.AutomationElement]
        $deadline = (Get-Date).AddSeconds(30)
        while ((Get-Date) -lt $deadline) {
            Start-Sleep -Milliseconds 500
            $cond = New-Object Windows.Automation.AndCondition((New-Object Windows.Automation.PropertyCondition($AE::ProcessIdProperty, [int]$appPid)), (New-Object Windows.Automation.PropertyCondition($AE::NameProperty, "Cerrar No Man's Sky")))
            $dialog = $AE::RootElement.FindFirst([Windows.Automation.TreeScope]::Descendants, $cond)
            if ($dialog) {
                $yes = $dialog.FindAll([Windows.Automation.TreeScope]::Descendants, (New-Object Windows.Automation.PropertyCondition($AE::ControlTypeProperty, [Windows.Automation.ControlType]::Button))) | Where-Object { $_.Current.Name -match '^S' } | Select-Object -First 1
                if ($yes) { $yes.GetCurrentPattern([Windows.Automation.InvokePattern]::Pattern).Invoke(); return 'confirmacion aceptada' }
            }
        }
        'no aparecio la confirmacion'
    }
    $close = Find-One $root ([Windows.Automation.ControlType]::Button) 'Cerrar juegos'
    $close.GetCurrentPattern([Windows.Automation.InvokePattern]::Pattern).Invoke()
    "pulsado Cerrar juegos: " + (Receive-Job $clicker -Wait -AutoRemoveJob)
    $deadline = (Get-Date).AddSeconds(90)
    while ((Get-Process NMS -ErrorAction SilentlyContinue) -and (Get-Date) -lt $deadline) { Start-Sleep -Seconds 2 }
    "NMS abierto tras cerrar: " + [bool](Get-Process NMS -ErrorAction SilentlyContinue)
    Start-Sleep -Seconds 2
    App-Log 6 "No Man|Sesi"
}
finally {
    try { if ($app -and -not $app.HasExited) { [void]$app.CloseMainWindow(); "lanzador cerrado: " + $app.WaitForExit(30000) } } catch { "aviso al cerrar el lanzador: $($_.Exception.Message)" }
    Start-Sleep -Seconds 1
    Copy-Item $backup $settings -Force
    "ajustes restaurados: " + ((Get-FileHash $backup).Hash -eq (Get-FileHash $settings).Hash)
    if (Get-Process NMS -ErrorAction SilentlyContinue) { "AVISO: NMS sigue abierto (no se fuerza)." }
}
