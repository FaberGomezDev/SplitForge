# Ensayo de extremo a extremo con 7DTD real, sin manos (PowerShell 5.1, solo ASCII).
#
#   powershell -ExecutionPolicy Bypass -File tools\ensayo_7dtd.ps1
#
# Abre el paquete publicado (publish\SplitForge), pulsa JUGAR, espera a que J1 este dentro de su ultima partida
# (J2 se queda en el menu) y comprueba con el juego real:
#   - que los juegos tapan la barra de tareas y que el foco simulado externo no se envia a quien lo simula dentro;
#   - el estado del filtro en los dos clientes (foco, puntero propio, teclas soltadas);
#   - que RB cambia de objeto en la ventana SIN foco: sin teclas pegadas (fase A), tras perder el foco con Alt
#     pulsada con la correccion apagada (fase B: el fallo, no debe cambiar) y con ella encendida (fase C).
# Pulsa los botones con el "mando de prueba" del filtro (%LOCALAPPDATA%\SplitForge\xinput\prueba.txt).
#
# AVISOS: ocupa la pantalla unos 3 minutos y manda la tecla Alt al juego (como Alt+Tab). J1 entra en la ULTIMA
# PARTIDA del propietario y cambia el objeto que lleva en la mano; no pulsa nada mas. Solo captura la ventana de
# J1 (PrintWindow), nunca el escritorio, y guarda las capturas FUERA del repositorio. Al final cierra los juegos
# con el boton del lanzador (guardan al cerrar), cierra el lanzador y restaura los ajustes tal como estaban.
# No se ejecuta si hay un juego o un SplitForge abierto. Mientras corre, no tocar teclado, raton ni mandos, y no
# activar el bloqueo de entrada (Ctrl+Alt+B): con el, Alt no llega al juego y la fase B no reproduce el fallo.
param([string]$Repo = (Split-Path $PSScriptRoot -Parent), [string]$Out = (Join-Path $env:TEMP 'SplitForge-ensayo'))
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes, System.Drawing
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Text; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices; using System.Collections.Generic;
public static class T {
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern IntPtr GetTopWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetWindow(IntPtr h, uint c);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("kernel32.dll", CharSet = CharSet.Unicode)] public static extern bool MoveFileEx(string a, string b, uint flags);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public static uint Pid(IntPtr h) { uint p; GetWindowThreadProcessId(h, out p); return p; }
  public static string Class(IntPtr h) { var s = new StringBuilder(96); GetClassName(h, s, 96); return s.ToString(); }
  // Posicion en el orden Z (0 = arriba) de las ventanas visibles de esas clases.
  public static List<string> Z(string classes) {
    var res = new List<string>(); int n = 0;
    for (IntPtr h = GetTopWindow(IntPtr.Zero); h != IntPtr.Zero && n < 5000; h = GetWindow(h, 2)) {
      if (!IsWindowVisible(h)) continue;
      string c = Class(h);
      if (("|" + classes + "|").Contains("|" + c + "|")) { RECT r; GetWindowRect(h, out r); res.Add(n + " " + c + " pid=" + Pid(h) + " " + r.L + "," + r.T + " " + (r.R - r.L) + "x" + (r.B - r.T)); }
      n++;
    }
    return res;
  }
  // Da el foco a una ventana como lo haria Alt+Tab: Alt pulsada antes del cambio y soltada despues.
  public static bool Focus(IntPtr h) {
    keybd_event(0x12, 0x38, 0, UIntPtr.Zero); System.Threading.Thread.Sleep(120);
    SetForegroundWindow(h); System.Threading.Thread.Sleep(200);
    keybd_event(0x12, 0x38, 2, UIntPtr.Zero); System.Threading.Thread.Sleep(150);
    return GetForegroundWindow() == h;
  }
  // Captura SOLO esa ventana y la guarda; devuelve la fraccion de pixeles que no son negros.
  public static double Capture(IntPtr h, string path) {
    RECT r; GetWindowRect(h, out r); int w = r.R - r.L, hh = r.B - r.T;
    using (var bmp = new Bitmap(w, hh, PixelFormat.Format32bppArgb)) {
      using (var g = Graphics.FromImage(bmp)) { IntPtr dc = g.GetHdc(); PrintWindow(h, dc, 2); g.ReleaseHdc(dc); }
      bmp.Save(path, ImageFormat.Png);
      long lit = 0, total = 0;
      for (int y = 0; y < hh; y += 8) for (int x = 0; x < w; x += 8) { var c = bmp.GetPixel(x, y); total++; if (c.R + c.G + c.B > 24) lit++; }
      return total == 0 ? 0 : (double)lit / total;
    }
  }
  public static bool Replace(string temp, string path) { return MoveFileEx(temp, path, 1); }
}
'@
[void][T]::SetProcessDPIAware()
$AE = [Windows.Automation.AutomationElement]
function Find-Button($root, $name) {
    $cond = New-Object Windows.Automation.AndCondition((New-Object Windows.Automation.PropertyCondition($AE::ControlTypeProperty, [Windows.Automation.ControlType]::Button)), (New-Object Windows.Automation.PropertyCondition($AE::NameProperty, $name)))
    $root.FindFirst([Windows.Automation.TreeScope]::Descendants, $cond)
}
$xinput = Join-Path $env:LOCALAPPDATA 'SplitForge\xinput'
$testFile = Join-Path $xinput 'prueba.txt'
function Set-Pad([int]$procId, [string]$buttons, [string]$options) {
    $temp = "$testFile.tmp"
    [IO.File]::WriteAllText($temp, "$procId $buttons $options`n", [Text.Encoding]::ASCII)
    if (-not [T]::Replace($temp, $testFile)) { throw 'No se pudo escribir prueba.txt' }
}
function Tap([int]$procId, [string]$buttons, [string]$options) { Set-Pad $procId $buttons $options; Start-Sleep -Milliseconds 150; Set-Pad $procId '0000' $options; Start-Sleep -Milliseconds 1300 }
function Status([int]$procId) {
    $text = ''
    foreach ($f in (Get-ChildItem $xinput -Filter "estado-$procId-*.txt" -ErrorAction SilentlyContinue)) { $text += " [" + (((Get-Content $f.FullName) | Where-Object { $_ -match '^(dll|ranura|foco|puntero|puntero_propio|teclas_soltadas|intercepcion)=' }) -join ' ') + "]" }
    $text
}
function Z-Line { "    orden Z: " + (([T]::Z('UnityWndClass|Shell_TrayWnd|SplitForge.Fondo')) -join ' | ') }
function App-Log([int]$tail) { Get-Content (Join-Path $env:LOCALAPPDATA 'SplitForge\splitforge-net.log') -Tail $tail | ForEach-Object { "    " + ($_ -replace '^\S+ \S+ \[\w+\] ', '') } }

if (Get-Process 7DaysToDie, SplitForge -ErrorAction SilentlyContinue) { throw 'Hay un juego o un SplitForge abierto: no se toca nada.' }
New-Item -ItemType Directory -Force $Out | Out-Null
Get-ChildItem $Out -Filter 'cap-*.png' | Remove-Item -Force
$settings = Join-Path $env:LOCALAPPDATA 'SplitForge\settings.v2.json'
$backup = Join-Path $Out 'settings.v2.json.respaldo'
Copy-Item $settings $backup -Force
$raw = [IO.File]::ReadAllText($settings)
$changed = [regex]::Replace($raw, '"QuickContinue":\s*false', '"QuickContinue": true')
if ($changed -eq $raw) { throw 'No se encontro QuickContinue=false en los ajustes.' }
[IO.File]::WriteAllText($settings, $changed, (New-Object Text.UTF8Encoding($false)))
"ajuste temporal: J1 carga su ultima partida (se restaura al final)"
if (Test-Path $testFile) { Remove-Item $testFile -Force }

$app = $null; $root = $null; $started = Get-Date
try {
    $folder = Join-Path $Repo 'publish\SplitForge'
    $app = Start-Process (Join-Path $folder 'SplitForge.exe') -WorkingDirectory $folder -PassThru
    $deadline = (Get-Date).AddSeconds(40)
    do { Start-Sleep -Milliseconds 500; $app.Refresh() } while ($app.MainWindowHandle -eq 0 -and (Get-Date) -lt $deadline)
    if ($app.MainWindowHandle -eq 0) { throw 'La ventana de SplitForge no aparecio.' }
    Start-Sleep -Seconds 4
    $root = $AE::FromHandle($app.MainWindowHandle)
    $play = Find-Button $root 'Jugar'
    if (-not $play) { throw 'No se encontro el boton Jugar.' }
    if (-not $play.Current.IsEnabled) { throw 'El boton Jugar esta deshabilitado.' }
    $play.GetCurrentPattern([Windows.Automation.InvokePattern]::Pattern).Invoke()
    "pulsado JUGAR"

    # Dos clientes con ventana, sesion colocada y J1 dentro de la partida.
    $deadline = (Get-Date).AddSeconds(240); $j1Log = $null; $spawned = $false
    do {
        Start-Sleep -Seconds 3
        $games = @(Get-Process 7DaysToDie -ErrorAction SilentlyContinue)
        if (-not $j1Log) {
            $j1Log = Get-ChildItem "$env:APPDATA\7DaysToDie\BacktraceLogs\Player-*.log" | Where-Object { $_.LastWriteTime -ge $started } |
                Where-Object { Select-String -Path $_.FullName -Pattern 'Command line arguments:.*-quick-continue' -Quiet } | Select-Object -First 1
        }
        if ($j1Log) { $spawned = [bool](Select-String -Path $j1Log.FullName -Pattern 'PlayerSpawnedInWorld \(reason: LoadedGame' -Quiet) }
        $elapsed = ((Get-Date) - $started).TotalSeconds
        if ($j1Log -and $elapsed -gt 42 -and $elapsed -le 85 -and -not (Select-String -Path $j1Log.FullName -Pattern 'QuickContinue mode' -Quiet)) {
            # El juego a veces abre un cuadro modal "Inicio de sesion de Discord" que hay que aceptar: boton A del mando de prueba.
            $p1 = Get-CimInstance Win32_Process -Filter "Name='7DaysToDie.exe'" | Where-Object { $_.CommandLine -match '-quick-continue' } | Select-Object -First 1
            if ($p1) {
                Set-Pad ([int]$p1.ProcessId) '0000' ''; Start-Sleep -Milliseconds 1300
                Set-Pad ([int]$p1.ProcessId) '1000' ''; Start-Sleep -Milliseconds 150; Set-Pad ([int]$p1.ProcessId) '0000' ''
                "a los $([int]$elapsed) s el menu no habia abierto: pulsado A en el mando de prueba de J1 (cuadro de Discord)"
                Start-Sleep -Seconds 5
            }
        }
        if ($j1Log -and ((Get-Date) - $started).TotalSeconds -gt 95 -and -not (Select-String -Path $j1Log.FullName -Pattern 'QuickContinue mode' -Quiet)) {
            foreach ($g in $games) {
                $cmd = (Get-CimInstance Win32_Process -Filter "ProcessId=$($g.Id)").CommandLine
                $tag = if ($cmd -match '-quick-continue') { 'j1' } else { 'j2' }
                "atasco: {0} pid={1} captura valida: {2:p0} |{3}" -f $tag, $g.Id, [T]::Capture($g.MainWindowHandle, (Join-Path $Out "cap-atasco-$tag.png")), (Status $g.Id)
            }
            "atasco: primer plano pid=" + [T]::Pid([T]::GetForegroundWindow())
            throw 'El juego no llego a su menu principal en 85 s: se cierra para reintentar.'
        }
    } while (-not $spawned -and (Get-Date) -lt $deadline -and ($games.Count -gt 0 -or ((Get-Date) - $started).TotalSeconds -lt 40))
    "clientes: $($games.Count) | J1 en la partida: $spawned | $([int]((Get-Date) - $started).TotalSeconds) s"
    if (-not $spawned) { throw 'J1 no llego a entrar en su partida.' }
    Start-Sleep -Seconds 12

    # J1 = el proceso lanzado con -quick-continue.
    $pids = @{}
    foreach ($p in (Get-CimInstance Win32_Process -Filter "Name='7DaysToDie.exe'")) { $pids[[int]$p.ProcessId] = ($p.CommandLine -match '-quick-continue') }
    $j1 = [int]($pids.Keys | Where-Object { $pids[$_] } | Select-Object -First 1)
    $j2 = [int]($pids.Keys | Where-Object { -not $pids[$_] } | Select-Object -First 1)
    $h1 = (Get-Process -Id $j1).MainWindowHandle; $h2 = (Get-Process -Id $j2).MainWindowHandle
    "J1 pid=$j1 ventana=$($h1 -ne 0) | J2 pid=$j2 ventana=$($h2 -ne 0)"
    "--- barra de tareas (0 = arriba; los juegos deben estar por encima de Shell_TrayWnd)"
    Z-Line
    "--- foco simulado externo y barra de tareas en el registro"
    Get-Content (Join-Path $env:LOCALAPPDATA 'SplitForge\splitforge-net.log') -Tail 12 | Where-Object { $_ -match 'Foco simulado|arra de tareas' } | ForEach-Object { "    " + $_.Substring(37) }
    "--- estado del filtro"
    "    J1:" + (Status $j1); "    J2:" + (Status $j2)

    $fg = [T]::Pid([T]::GetForegroundWindow())
    "ventana activa al empezar: " + $(if ($fg -eq $j1) { 'J1' } elseif ($fg -eq $j2) { 'J2' } else { "otra (pid $fg)" })
    if ($fg -ne $j2) { "    foco en J2: " + [T]::Focus($h2); Start-Sleep -Milliseconds 1500 }

    "--- fase A (control): J1 SIN foco y sin teclas pegadas; RB debe cambiar un objeto a la derecha"
    Set-Pad $j1 '0000' ''; Start-Sleep -Milliseconds 1500
    "    captura valida: {0:p0}" -f [T]::Capture($h1, (Join-Path $Out 'cap-0a.png'))
    Tap $j1 '0200' ''
    [void][T]::Capture($h1, (Join-Path $Out 'cap-0b.png'))

    "--- fase B (fallo reproducido): correccion apagada; J1 pierde el foco con Alt pulsada; RB NO debe cambiar de objeto"
    Set-Pad $j1 '0000' 'sin_soltar'; Start-Sleep -Milliseconds 1500
    "    foco en J1: " + [T]::Focus($h1); Start-Sleep -Milliseconds 1200
    "    foco en J2 (Alt pulsada sobre J1): " + [T]::Focus($h2); Start-Sleep -Milliseconds 1500
    [void][T]::Capture($h1, (Join-Path $Out 'cap-1a.png'))
    Tap $j1 '0200' 'sin_soltar'
    [void][T]::Capture($h1, (Join-Path $Out 'cap-1b.png'))
    "    J1:" + (Status $j1)

    "--- fase C (correccion): J1 pierde el foco con Alt pulsada; RB debe cambiar un objeto a la derecha sin tener el foco"
    Set-Pad $j1 '0000' ''; Start-Sleep -Milliseconds 500
    "    foco en J1 (recibe el soltar de Alt): " + [T]::Focus($h1); Start-Sleep -Milliseconds 1200
    "    foco en J2 (Alt pulsada sobre J1): " + [T]::Focus($h2); Start-Sleep -Milliseconds 1500
    [void][T]::Capture($h1, (Join-Path $Out 'cap-2a.png'))
    Tap $j1 '0200' ''
    [void][T]::Capture($h1, (Join-Path $Out 'cap-2b.png'))
    Start-Sleep -Milliseconds 2200
    "    J1:" + (Status $j1); "    J2:" + (Status $j2)

    "--- barra de tareas con J1 (arriba) como ventana activa, dos muestras"
    [void][T]::Focus($h1); Start-Sleep -Milliseconds 900; Z-Line; Start-Sleep -Milliseconds 1200; Z-Line
    "--- barra de tareas al final"
    Z-Line
    "--- registro de SplitForge"
    App-Log 9
}
finally {
    if (Test-Path $testFile) { Remove-Item $testFile -Force -ErrorAction SilentlyContinue }
    try {
        if ($root -and (Get-Process 7DaysToDie -ErrorAction SilentlyContinue)) {
            $close = Find-Button $root 'Cerrar juegos'
            if ($close) { $close.GetCurrentPattern([Windows.Automation.InvokePattern]::Pattern).Invoke(); "pulsado Cerrar juegos" }
            $deadline = (Get-Date).AddSeconds(150)
            while ((Get-Process 7DaysToDie -ErrorAction SilentlyContinue) -and (Get-Date) -lt $deadline) { Start-Sleep -Seconds 2 }
            "juegos abiertos tras cerrar: " + @(Get-Process 7DaysToDie -ErrorAction SilentlyContinue).Count
        }
    } catch { "aviso al cerrar los juegos: $($_.Exception.Message)" }
    try {
        if ($app -and -not $app.HasExited) { [void]$app.CloseMainWindow(); "lanzador cerrado: " + $app.WaitForExit(30000) }
    } catch { "aviso al cerrar el lanzador: $($_.Exception.Message)" }
    Start-Sleep -Seconds 1
    Copy-Item $backup $settings -Force
    "ajustes restaurados: " + ((Get-FileHash $backup).Hash -eq (Get-FileHash $settings).Hash)
}

# Veredicto: casilla resaltada del cinturon (1-10; 0 si no se distingue) en cada captura. Es la que tiene el
# borde superior blanco. Medidas tomadas con la ventana a 539 px de alto (V 3.3) y escaladas con la altura.
function Belt-Slot([string]$name) {
    $b = New-Object Drawing.Bitmap (Join-Path $Out "cap-$name.png")
    try {
        $s = $b.Height / 539.0; $slot = 39.0 * $s; $left0 = $b.Width / 2.0 - 5 * $slot; $top = $b.Height - 43.5 * $s
        $best = 0; $bestCount = 0
        for ($i = 0; $i -lt 10; $i++) {
            $count = 0
            for ($y = [int]($top - 1 * $s); $y -le [int]($top + 2 * $s); $y++) {
                $row = 0
                for ($x = [int]($left0 + $i * $slot + 5 * $s); $x -lt [int]($left0 + ($i + 1) * $slot - 5 * $s); $x++) { $p = $b.GetPixel($x, $y); if ($p.R -ge 215 -and $p.G -ge 215 -and $p.B -ge 215) { $row++ } }
                if ($row -gt $count) { $count = $row }
            }
            if ($count -gt $bestCount) { $bestCount = $count; $best = $i + 1 }
        }
        if ($bestCount -lt 0.6 * ($slot - 10 * $s)) { 0 } else { $best }
    } finally { $b.Dispose() }
}
if (Test-Path (Join-Path $Out 'cap-2b.png')) {
    $a0 = Belt-Slot '0a'; $a1 = Belt-Slot '0b'; $b0 = Belt-Slot '1a'; $b1 = Belt-Slot '1b'; $c0 = Belt-Slot '2a'; $c1 = Belt-Slot '2b'
    "--- veredicto (casilla del cinturon de J1 antes -> despues de RB; 0 = no se distingue)"
    "    fase A, sin foco y sin teclas pegadas:            $a0 -> $a1  " + $(if ($a0 -gt 0 -and $a1 -gt 0 -and $a1 -ne $a0) { 'cambia de objeto (correcto)' } else { 'NO cambia' })
    "    fase B, Alt pegada con la correccion apagada:     $b0 -> $b1  " + $(if ($b0 -gt 0 -and $b1 -eq $b0) { 'NO cambia (fallo reproducido)' } else { 'cambia (el fallo no se reprodujo)' })
    "    fase C, Alt pulsada con la correccion encendida:  $c0 -> $c1  " + $(if ($c0 -gt 0 -and $c1 -gt 0 -and $c1 -ne $c0) { 'cambia de objeto (CORREGIDO)' } else { 'NO cambia (SIGUE FALLANDO)' })
    if ($b0 -gt 0 -and $b1 -ne $b0) { "    (si la fase B cambia: Alt no llego al juego. Pasa con el bloqueo de entrada Ctrl+Alt+B activo o si alguien toca el teclado.)" }
    "    capturas de la ventana de J1 en: $Out"
}
if ($j1Log) {
    "--- registro de J1 (cierre y errores)"
    Select-String -Path $j1Log.FullName -Pattern 'SaveAndCleanupWorld|Saving \d+ of chunks|World\.Save|Persist|OnApplicationQuit|Terminating application|EXC |ERR ' | Select-Object -Last 8 |
        ForEach-Object { "    " + ($_.Line -replace '^\d{4}-\d\d-\d\dT', '').Substring(0, [Math]::Min(150, ($_.Line -replace '^\d{4}-\d\d-\d\dT', '').Length)) }
}
