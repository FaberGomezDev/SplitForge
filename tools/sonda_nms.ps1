# Sonda de No Man's Sky con UNA instancia (la copia de Steam). Solo ASCII, PowerShell 5.1.
#
#   powershell -ExecutionPolicy Bypass -File tools\sonda_nms.ps1 -Paso abrir
#   ... -Paso estado | -Paso captura -Arg1 <nombre> | -Paso tocar -Arg1 <hex> [-Ms 1700] | -Paso foco-fuera
#   ... -Paso foco-juego | -Paso cerrar          (SIEMPRE terminar con "cerrar")
#
# "abrir" pone el filtro de SplitForge como xinput9_1_0.dll en Binaries de forma TEMPORAL, lanza el juego por
# Steam y asigna el mando 1 al proceso. "tocar" pulsa botones con el mando de prueba (mascara wButtons de XInput:
# A 1000, B 2000, cruceta 0001/0002/0004/0008). "cerrar" pide el cierre normal, retira el filtro (solo si es el
# nuestro) y restaura la asignacion de mandos. Solo captura la ventana del juego, fuera del repositorio.
# No entrar en ninguna partida con la sonda: es para el menu. Ver docs/plans/no-mans-sky.md.
param([string]$Paso, [string]$Arg1 = '', [int]$Ms = 160, [string]$Repo = '', [string]$Out = '',
      [string]$Juego = "C:\Program Files (x86)\Steam\steamapps\common\No Man's Sky")
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Text; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class N {
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern IntPtr FindWindow(string cls, string title);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
  [DllImport("kernel32.dll", CharSet = CharSet.Unicode)] public static extern bool MoveFileEx(string a, string b, uint flags);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public static uint Pid(IntPtr h) { uint p; GetWindowThreadProcessId(h, out p); return p; }
  public static string Class(IntPtr h) { var s = new StringBuilder(96); GetClassName(h, s, 96); return s.ToString(); }
  public static bool Focus(IntPtr h) {
    keybd_event(0x12, 0x38, 0, UIntPtr.Zero); System.Threading.Thread.Sleep(120);
    SetForegroundWindow(h); System.Threading.Thread.Sleep(200);
    keybd_event(0x12, 0x38, 2, UIntPtr.Zero); System.Threading.Thread.Sleep(150);
    return GetForegroundWindow() == h;
  }
  public static string Capture(IntPtr h, string path, int maxWidth) {
    RECT r; GetWindowRect(h, out r); int w = r.R - r.L, hh = r.B - r.T;
    using (var bmp = new Bitmap(w, hh, PixelFormat.Format32bppArgb)) {
      using (var g = Graphics.FromImage(bmp)) { IntPtr dc = g.GetHdc(); PrintWindow(h, dc, 2); g.ReleaseHdc(dc); }
      long lit = 0, total = 0;
      for (int y = 0; y < hh; y += 8) for (int x = 0; x < w; x += 8) { var c = bmp.GetPixel(x, y); total++; if (c.R + c.G + c.B > 24) lit++; }
      int ow = Math.Min(maxWidth, w), oh = (int)((long)hh * ow / w);
      using (var small = new Bitmap(bmp, ow, oh)) small.Save(path, ImageFormat.Png);
      return w + "x" + hh + " no negro " + (total == 0 ? 0 : 100 * lit / total) + "%";
    }
  }
}
'@
[void][N]::SetProcessDPIAware()
if (-not $Repo) { $Repo = Split-Path $PSScriptRoot -Parent }
if (-not $Out) { $Out = Join-Path $env:TEMP 'SplitForge-nms' }
New-Item -ItemType Directory -Force $Out | Out-Null
$bin = Join-Path $Juego 'Binaries'
$proxy = Join-Path $bin 'xinput9_1_0.dll'
$ours = Join-Path $Repo 'publish\SplitForge\native\SplitForge.XInputFilter.dll'
$xinput = Join-Path $env:LOCALAPPDATA 'SplitForge\xinput'
$assign = Join-Path $xinput 'asignacion.txt'
$testFile = Join-Path $xinput 'prueba.txt'
function Game { Get-Process NMS -ErrorAction SilentlyContinue | Select-Object -First 1 }
function Set-Pad([int]$procId, [string]$buttons) {
    $temp = "$testFile.tmp"
    [IO.File]::WriteAllText($temp, "$procId $buttons`n", [Text.Encoding]::ASCII)
    if (-not [N]::MoveFileEx($temp, $testFile, 1)) { throw 'No se pudo escribir prueba.txt' }
}
function Status([int]$procId) {
    foreach ($f in (Get-ChildItem $xinput -Filter "estado-$procId-*.txt" -ErrorAction SilentlyContinue)) { "  " + ((Get-Content $f.FullName) -join ' ') }
}

switch ($Paso) {
    'abrir' {
        if (Game) { throw 'NMS ya esta abierto.' }
        if (Test-Path $proxy) { throw 'Ya hay un xinput9_1_0.dll en la carpeta del juego: no se toca.' }
        Copy-Item $ours $proxy
        "filtro copiado como xinput9_1_0.dll (temporal): " + ((Get-FileHash $proxy).Hash -eq (Get-FileHash $ours).Hash)
        if (Test-Path $assign) { Copy-Item $assign (Join-Path $Out 'asignacion.respaldo') -Force }
        Start-Process 'steam://rungameid/275850'
        $deadline = (Get-Date).AddSeconds(240)
        do { Start-Sleep -Seconds 2; $g = Game } while (-not $g -and (Get-Date) -lt $deadline)
        if (-not $g) { throw 'NMS no arranco en 240 s (Steam cerrado o pidiendo algo).' }
        [IO.File]::WriteAllText($assign, "$($g.Id) 0`n", [Text.Encoding]::ASCII)
        Set-Pad $g.Id '0000'
        "NMS pid=$($g.Id); mando 1 asignado"
        $deadline = (Get-Date).AddSeconds(90)
        do { Start-Sleep -Seconds 2; $g.Refresh() } while ($g.MainWindowHandle -eq 0 -and -not $g.HasExited -and (Get-Date) -lt $deadline)
        "ventana: $($g.MainWindowHandle -ne 0) clase=" + [N]::Class($g.MainWindowHandle) + " titulo=" + $g.MainWindowTitle
    }
    'estado' {
        $g = Game; if (-not $g) { 'NMS no esta abierto'; break }
        $fg = [N]::GetForegroundWindow()
        "pid=$($g.Id) ventana activa es NMS: " + ([N]::Pid($fg) -eq $g.Id) + " (clase activa: " + [N]::Class($fg) + ")"
        Status $g.Id
        $mods = $g.Modules | Where-Object { $_.ModuleName -match 'xinput|gameinput|dinput|steamclient|gameoverlay|hid\.dll' } | ForEach-Object { $_.ModuleName + '@' + $(if ($_.FileName -like "$bin*") { 'juego' } elseif ($_.FileName -like "$env:WINDIR*") { 'windows' } else { 'otro' }) }
        "modulos de entrada: " + ($mods -join ', ')
    }
    'captura' {
        $g = Game; if (-not $g) { 'NMS no esta abierto'; break }
        $path = Join-Path $Out ("nms-" + $Arg1 + ".png")
        "captura " + [N]::Capture($g.MainWindowHandle, $path, 1280) + " -> $path"
    }
    'tocar' {
        $g = Game; if (-not $g) { 'NMS no esta abierto'; break }
        Set-Pad $g.Id $Arg1; Start-Sleep -Milliseconds $Ms; Set-Pad $g.Id '0000'; Start-Sleep -Milliseconds 900
        "pulsado $Arg1 durante $Ms ms"
    }
    'foco-fuera' {
        $tray = [N]::FindWindow('Shell_TrayWnd', $null)
        "foco en la barra de tareas (Alt pulsada sobre la ventana activa): " + [N]::Focus($tray)
    }
    'foco-juego' {
        $g = Game; if (-not $g) { 'NMS no esta abierto'; break }
        "foco en NMS: " + [N]::Focus($g.MainWindowHandle)
    }
    'cerrar' {
        if (Test-Path $testFile) { Remove-Item $testFile -Force }
        $g = Game
        if ($g) {
            [void][N]::PostMessage($g.MainWindowHandle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)
            "cierre normal pedido (WM_CLOSE): salio = " + $g.WaitForExit(60000)
        }
        if (Game) { 'AVISO: NMS sigue abierto; no se fuerza ni se quita el filtro.'; break }
        Start-Sleep -Seconds 2
        if (Test-Path $proxy) {
            if ((Get-FileHash $proxy).Hash -eq (Get-FileHash $ours).Hash) { Remove-Item $proxy -Force; "filtro retirado de la carpeta del juego: " + (-not (Test-Path $proxy)) }
            else { 'AVISO: el xinput9_1_0.dll de la carpeta no es el nuestro; no se toca.' }
        }
        $backup = Join-Path $Out 'asignacion.respaldo'
        if (Test-Path $backup) { Copy-Item $backup $assign -Force; 'asignacion de mandos restaurada' } elseif (Test-Path $assign) { Remove-Item $assign -Force }
    }
    default { 'paso desconocido' }
}
