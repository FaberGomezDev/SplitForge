# Versionado, compilación y entregas

## Versión y naturaleza del paquete

- Código **0.7.0** (`Directory.Build.props` → `<Version>`; también en `MainViewModel.Version` y `app.manifest`).
- `0.x` = experimental. `MAJOR.MINOR.PATCH` estable desde el primer adaptador validado en hardware.
- El paquete de usuario es una **publicación .NET autónoma win-x64** (incluye runtime, WPF y `SDL3.dll`): no requiere instalar nada.

## Integración continua

- `.github/workflows/build.yml` (cada push a `main`, PR y manual):
  - módulos nativos C++ (filtro XInput y emulador de Steam) con CTest;
  - pruebas .NET + Python y `check_docs`;
  - publicación autónoma con `native\SplitForge.XInputFilter.dll` y `native\SplitForge.SteamEmu\steam_api64.dll`;
  - prueba de humo de `SplitForge.Diagnostico.exe` y prueba de arranque de la interfaz (7 pestañas; capturas 7 días);
  - artefacto **SplitForge-win-x64** (30 días).
- `.github/workflows/release.yml` (etiquetas `v*`):
  - módulos nativos con CTest y pruebas .NET;
  - publicación con las DLL nativas;
  - `SplitForge-vX.Y.Z-win-x64.zip` + `.sha256` y release de GitHub (prerelease para `v0.*`).

### Publicar una versión

```powershell
git tag v0.7.0
git push origin v0.7.0
```

## Checklist antes de etiquetar

1. `dotnet test SplitForge.slnx -c Release` y `python -m unittest discover -s tests -v` en verde; anotar conteos.
2. `python tools/check_docs.py` sin errores.
3. En Windows 11 con dos PS4: W01–W07 de `07-PRUEBAS.md` como mínimo.
4. Informes Steam/7DTD revisados: sin rutas, cuentas ni títulos.
5. Actualizar `01-MEMORIA.md`, `15-HISTORIAL.md` y, si aplica, `05-INTEGRACION-STEAM.md`.
6. No declarar un adaptador validado sin build real, hardware real y evidencias.

## Compatibilidad con instalaciones existentes

- El prototipo Python y su `.venv` siguen funcionando en la misma carpeta del repositorio (`Iniciar_SplitForge.bat`).
- La 0.7 no modifica `settings.json`; usa `settings.v2.json` e importa orientación, separación y modo de lectura la primera vez.
- Los perfiles de botones deben recalibrarse una vez (los GUID de SDL3 pueden diferir de SDL2).

## Cómo actualizar la memoria de la próxima sesión

Aplicar [`templates/ENTREGA.md`](templates/ENTREGA.md): qué cambió, por qué, pruebas, restricciones nuevas y siguiente paso. No permitir que la documentación prometa funciones futuras como actuales.
