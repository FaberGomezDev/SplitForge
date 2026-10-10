# Instalador/restaurador del emulador de Steam educativo (EXPERIMENTAL).
# Uso:
#   tools\SteamEmu_Instalar.ps1 -Juego "C:\...\7 Days To Die" -AppId 251570 -SteamId "76561198000000001" -Persona "SplitForgeJ1" -Accion Instalar
#   tools\SteamEmu_Instalar.ps1 -Juego "C:\...\7 Days To Die" -Accion Restaurar
# Solo para experimentación educativa con juegos propios y sin EAC. Restaurar antes de verificar
# archivos en Steam. Reversible: copia de seguridad + SHA-256 + archivo de estado.

param(
    [Parameter(Mandatory = $true)]
    [string]$Juego,
    [ValidateSet("Instalar", "Restaurar")]
    [string]$Accion = "Instalar",
    [int]$AppId = 480,
    [string]$SteamId = "76561198000000001",
    [string]$Persona = "SplitForgePlayer",
    [string]$Lenguaje = "spanish",
    [string]$Pais = "ES",
    [string]$Dll = ""
)

$ErrorActionPreference = "Stop"
$Raiz = Split-Path -Parent $PSScriptRoot
$CarpetaJuego = (Resolve-Path -LiteralPath $Juego).Path

function Buscar-DllSteam {
    param([string]$Nombre)
    $raiz = Join-Path $CarpetaJuego $Nombre
    if (Test-Path -LiteralPath $raiz) { return $raiz }
    return Get-ChildItem -LiteralPath $CarpetaJuego -Recurse -Filter $Nombre -File -ErrorAction SilentlyContinue |
        Select-Object -First 1 -ExpandProperty FullName
}

$DllJuego = Buscar-DllSteam "steam_api64.dll"
$CarpetaDll = if ($DllJuego) { Split-Path -Parent $DllJuego } else { $CarpetaJuego }
$CopiaOriginal = Join-Path $CarpetaDll "steam_api64_splitforge_original.dll"
$JsonConfig = Join-Path $CarpetaDll "SplitForge.SteamEmu.json"
$ArchivoEstado = Join-Path $CarpetaDll "SplitForge.SteamEmu.estado.txt"

function Buscar-CMake {
    $candidatos = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    )
    foreach ($c in $candidatos) { if (Test-Path $c) { return $c } }
    $dePath = Get-Command cmake -ErrorAction SilentlyContinue
    if ($dePath) { return $dePath.Source }
    return $null
}

if ($Accion -eq "Restaurar") {
    $copiaBuscada = Buscar-DllSteam "steam_api64_splitforge_original.dll"
    if (-not $copiaBuscada) {
        Write-Host "No hay copia de seguridad (la DLL original ya estaba en su sitio)." -ForegroundColor Yellow
        if (Test-Path -LiteralPath $ArchivoEstado) { Remove-Item -LiteralPath $ArchivoEstado -Force }
        if (Test-Path -LiteralPath $JsonConfig) { Remove-Item -LiteralPath $JsonConfig -Force }
        exit 0
    }
    $CarpetaDll = Split-Path -Parent $copiaBuscada
    $CopiaOriginal = $copiaBuscada
    $DllJuego = Join-Path $CarpetaDll "steam_api64.dll"
    $JsonConfig = Join-Path $CarpetaDll "SplitForge.SteamEmu.json"
    $ArchivoEstado = Join-Path $CarpetaDll "SplitForge.SteamEmu.estado.txt"
    if (Test-Path -LiteralPath $ArchivoEstado) {
        $guardado = (Get-Content -LiteralPath $ArchivoEstado -Raw).Trim()
        $actual = (Get-FileHash -LiteralPath $DllJuego -Algorithm SHA256).Hash
        if ($guardado -ne $actual) {
            Write-Host "La DLL instalada no coincide con la registrada; no se restaura para no pisar cambios." -ForegroundColor Red
            exit 1
        }
    }
    Copy-Item -LiteralPath $CopiaOriginal -Destination $DllJuego -Force
    Remove-Item -LiteralPath $CopiaOriginal -Force
    Remove-Item -LiteralPath $JsonConfig -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $ArchivoEstado -Force -ErrorAction SilentlyContinue
    Write-Host "Restaurada la DLL original de Steam ($CarpetaDll)." -ForegroundColor Green
    exit 0
}

if (-not (Test-Path -LiteralPath $DllJuego)) {
    Write-Host "No existe steam_api64.dll en la carpeta indicada ni en sus subcarpetas. ¿Es la carpeta del juego?" -ForegroundColor Red
    exit 1
}
if (Test-Path -LiteralPath $CopiaOriginal) {
    Write-Host "Ya hay una copia de seguridad: parece que el emulador ya está instalado. Usa -Accion Restaurar primero." -ForegroundColor Yellow
    exit 1
}

$cmake = Buscar-CMake
if (-not $cmake) {
    Write-Host "No se encontró CMake. Compila native\SplitForge.SteamEmu manualmente o pasa -Dll con la ruta de la DLL emulada." -ForegroundColor Red
    exit 1
}
$dllFuente = $Dll
if ($dllFuente -eq "") {
    $buildDir = Join-Path $Raiz "native\build-steamemu"
    if (-not (Test-Path (Join-Path $buildDir "Release\steam_api64.dll"))) {
        Write-Host "Compilando el emulador (CMake)..."
        & $cmake -S (Join-Path $Raiz "native\SplitForge.SteamEmu") -B $buildDir -A x64 | Out-Null
        if ($LASTEXITCODE -ne 0) { Write-Host "Fallo de configuración CMake." -ForegroundColor Red; exit 1 }
        & $cmake --build $buildDir --config Release | Out-Null
        if ($LASTEXITCODE -ne 0) { Write-Host "Fallo de compilación." -ForegroundColor Red; exit 1 }
        & "$($cmake.Substring(0, $cmake.Length - 8))ctest.exe" --test-dir $buildDir -C Release --output-on-failure | Out-Null
        if ($LASTEXITCODE -ne 0) { Write-Host "La prueba de humo del emulador falló; no se instala." -ForegroundColor Red; exit 1 }
    }
    $dllFuente = Join-Path $buildDir "Release\steam_api64.dll"
}
if (-not (Test-Path -LiteralPath $dllFuente)) {
    Write-Host "No se encontró la DLL emulada en $dllFuente" -ForegroundColor Red
    exit 1
}

$almacen = Join-Path $env:LOCALAPPDATA "SplitForge\steamemu\$AppId"
$config = @{
    app_id = $AppId
    steam_id = $SteamId
    persona_name = $Persona
    language = $Lenguaje
    ip_country = $Pais
    storage_dir = $almacen
    net_port = 27117
    net_discovery_seconds = 2
    net_multicast_address = "239.255.69.69"
} | ConvertTo-Json -Depth 3

Copy-Item -LiteralPath $DllJuego -Destination $CopiaOriginal
Copy-Item -LiteralPath $dllFuente -Destination $DllJuego -Force
Set-Content -LiteralPath $JsonConfig -Value $config -Encoding utf8
(Get-FileHash -LiteralPath $DllJuego -Algorithm SHA256).Hash | Set-Content -LiteralPath $ArchivoEstado -Encoding ascii

Write-Host "Emulador instalado (experimental):"
Write-Host "  Juego:     $CarpetaJuego"
Write-Host "  Original:  steam_api64_splitforge_original.dll (copia de seguridad)"
Write-Host "  Config:    SplitForge.SteamEmu.json (steam_id=$SteamId)"
Write-Host "  Guardados: $almacen"
Write-Host "Para una segunda instancia con otra identidad: arranca el juego con la variable de"
Write-Host "entorno SPLITFORGE_STEAMEMU_CONFIG apuntando a otro JSON (otro steam_id y storage_dir)."
Write-Host "Restaurar: tools\SteamEmu_Instalar.ps1 -Juego `"$CarpetaJuego`" -Accion Restaurar"
