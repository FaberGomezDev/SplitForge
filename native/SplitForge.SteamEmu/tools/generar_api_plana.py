"""Genera la API plana de steam_api64.dll del emulador: SteamAPI_<Interfaz>_<Método>(self, ...).

Los juegos con Steamworks.NET (como 7 Days to Die) llaman SOLO a la API plana del SDK, cuyo primer
argumento es el puntero a la interfaz (`self`). Este script la genera a partir de las vtables del
emulador (src/interfaces.h) y de la lista de entradas que usa Steamworks.NET
(tools/steamworksnet_api.json, extraída con tools/extraer_api_steamworksnet.cs):

- reenvío tipado a la vtable para cada método implementado (sobrecargas con el sufijo del SDK:
  GetStatInt32/GetStatFloat, SetPropertyString…);
- entrada neutra (0, false, nullptr o "") para cada método que Steamworks.NET importa y el emulador
  no implementa, de modo que nunca falte una exportación;
- registro en steamemu.log de la primera llamada a cada entrada, para saber qué usa cada juego.

Salidas: src/flat_generated.cpp, src/flat_required_exports.h y el bloque generado de steamemu.def.
Uso: python tools/generar_api_plana.py [--comprobar]   (--comprobar: falla si las salidas no están al día)
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INTERFACES = ROOT / "src" / "interfaces.h"
API = ROOT / "tools" / "steamworksnet_api.json"
OUT_CPP = ROOT / "src" / "flat_generated.cpp"
OUT_LIST = ROOT / "src" / "flat_required_exports.h"
DEF = ROOT / "steamemu.def"
DEF_BEGIN = "    ; >>> API plana generada por tools/generar_api_plana.py (no editar a mano)"
DEF_END = "    ; <<< fin de la API plana generada"

# Interfaces que implementa el juego, no el emulador: no tienen API plana que exportar.
SKIP_CLASSES = {"ISteamMatchmakingServerListResponse"}

# Sufijo del SDK para distinguir sobrecargas, según el tipo del primer parámetro que las diferencia.
OVERLOAD_SUFFIX = {
    "int32_t*": "Int32", "int32_t": "Int32", "float*": "Float", "float": "Float",
    "int64_t*": "Int64", "int64_t": "Int64", "double*": "Double", "double": "Double",
    "const char*": "String", "bool": "Bool",
}
# Casos que la regla no resuelve: (clase, método, índice de la sobrecarga) -> nombre plano o None (no exportar).
OVERLOAD_OVERRIDES = {
    ("ISteamUGC", "CreateQueryAllUGCRequest", 0): "CreateQueryAllUGCRequestPage",
    # La segunda sobrecarga del emulador no coincide con la del SDK (cursor const char*): sin reenvío.
    ("ISteamUGC", "CreateQueryAllUGCRequest", 1): None,
}
# Estructuras que cruzan la ABI por valor (tamaños comprobados con static_assert en steamemu_types.h).
STRUCT_SIZES = {
    "InputDigitalActionData_t": 2, "InputAnalogActionData_t": 13,
    "InputMotionData_t": 40, "SteamIPAddress_t": 20,
}


class Method:
    def __init__(self, cls: str, ret: str, name: str, params: list[str]):
        self.cls, self.ret, self.name, self.params = cls, ret, name, params
        self.flat: str | None = None


def parse_interfaces(text: str) -> list[Method]:
    methods: list[Method] = []
    for match in re.finditer(r"^class (ISteam\w+) \{\npublic:\n(.*?)^\};", text, re.S | re.M):
        cls, body = match.group(1), match.group(2)
        if cls in SKIP_CLASSES:
            continue
        for line in body.splitlines():
            m = re.fullmatch(r"\s*virtual (.+?) (\w+)\((.*)\) = 0;", line)
            if not m:
                if line.strip():
                    raise SystemExit(f"interfaces.h: línea no reconocida en {cls}: {line.strip()}")
                continue
            ret = m.group(1).replace("class ", "").strip()
            params = [p.strip() for p in m.group(3).split(",")] if m.group(3).strip() else []
            methods.append(Method(cls, ret, m.group(2), params))
    return methods


def assign_flat_names(methods: list[Method]) -> None:
    groups: dict[tuple[str, str], list[Method]] = {}
    for method in methods:
        groups.setdefault((method.cls, method.name), []).append(method)
    for (cls, name), group in groups.items():
        if len(group) == 1:
            group[0].flat = f"SteamAPI_{cls}_{name}"
            continue
        for index, method in enumerate(group):
            key = (cls, name, index)
            if key in OVERLOAD_OVERRIDES:
                suffix_name = OVERLOAD_OVERRIDES[key]
                method.flat = f"SteamAPI_{cls}_{suffix_name}" if suffix_name else None
                continue
            position = next(i for i in range(len(method.params))
                            if len({tuple(g.params)[i] if i < len(g.params) else None for g in group}) > 1)
            suffix = OVERLOAD_SUFFIX.get(method.params[position])
            if suffix is None:
                raise SystemExit(f"Sobrecarga sin sufijo conocido: {cls}::{name}({', '.join(method.params)})")
            method.flat = f"SteamAPI_{cls}_{name}{suffix}"


def kind(cpp_type: str) -> str:
    t = cpp_type.strip()
    if t == "void":
        return "void"
    if "*" in t or "&" in t:
        return "ptr"
    base = t.replace("const ", "").strip()
    if base in ("float", "double"):
        return base
    size = STRUCT_SIZES.get(base)
    if size is not None and size not in (1, 2, 4, 8):
        return f"struct:{size}"
    return "int"


def register_class(k: str) -> str:
    """Banco de registros que usa cada argumento en la ABI x64 de Windows."""
    if k in ("float", "double"):
        return k
    return "gp"  # enteros, punteros y estructuras por valor (que viajan por puntero)


def check(method: Method, entry: dict) -> str | None:
    """Devuelve el motivo si el reenvío no es seguro para la firma que usa Steamworks.NET."""
    managed = entry["params"]
    mine = ["ptr"] + [kind(p) for p in method.params]
    if len(mine) > len(managed):
        return f"el emulador espera {len(mine) - 1} argumentos y Steamworks.NET pasa {len(managed) - 1}"
    for i, (a, b) in enumerate(zip(mine, managed)):
        if register_class(a) != register_class(b):
            return f"argumento {i}: {a} en el emulador, {b} en Steamworks.NET"
    mine_ret, managed_ret = kind(method.ret), entry["ret"]
    if managed_ret != "void":
        if mine_ret == "void":
            return "el emulador no devuelve valor"
        if register_class(mine_ret) != register_class(managed_ret) or (
                mine_ret.startswith("struct") and mine_ret != managed_ret):
            return f"retorno {mine_ret} en el emulador, {managed_ret} en Steamworks.NET"
    return None


HEADER = """// GENERADO por tools/generar_api_plana.py a partir de src/interfaces.h y tools/steamworksnet_api.json.
// No editar a mano: cambia las entradas y vuelve a ejecutar el generador.
//
// API plana del SDK: cada método de interfaz se exporta como SteamAPI_<Interfaz>_<Método> con el
// puntero a la interfaz (self) como primer argumento. Steamworks.NET solo usa esta vía.

#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <windows.h>

#include <atomic>
#include <cstring>

namespace {

// Registra en steamemu.log la primera llamada a cada entrada (qué usa realmente cada juego).
void FirstCall(std::atomic<bool>& seen, const char* name, bool implemented) {
    if (!seen.exchange(true, std::memory_order_relaxed))
        sfemu::Emu().Log("API plana: %s%s (pid=%lu)", name, implemented ? "" : " [sin implementar]",
                         GetCurrentProcessId());
}

} // namespace

#define SF_FLAT(name, implemented)                       \\
    do {                                                 \\
        static std::atomic<bool> sf_seen{false};         \\
        FirstCall(sf_seen, name, implemented);           \\
    } while (0)

using namespace sfemu;

extern "C" {
"""


def render_forwarder(method: Method) -> str:
    params = [f"{method.cls}* self"] + [f"{p} a{i}" for i, p in enumerate(method.params)]
    args = ", ".join(f"a{i}" for i in range(len(method.params)))
    call = f"self->{method.name}({args})"
    if method.ret == "void":
        body = f"    if (self)\n        {call};\n"
    else:
        body = f"    if (!self)\n        return {{}};\n    return {call};\n"
    return (f"{method.ret} {method.flat}({', '.join(params)}) {{\n"
            f"    SF_FLAT(\"{method.flat}\", true);\n{body}}}\n")


def render_stub(entry: dict) -> str:
    name, ret = entry["name"], entry["ret"]
    log = f"    SF_FLAT(\"{name}\", false);\n"
    if ret == "void":
        return f"void {name}() {{\n{log}}}\n"
    if ret == "float":
        return f"float {name}() {{\n{log}    return 0.0f;\n}}\n"
    if ret == "double":
        return f"double {name}() {{\n{log}    return 0.0;\n}}\n"
    if ret.startswith("struct:"):
        size = int(ret.split(":")[1])
        # Estructura devuelta por puntero oculto (primer argumento): se rellena con ceros.
        return f"void* {name}(void* result) {{\n{log}    std::memset(result, 0, {size});\n    return result;\n}}\n"
    if ret == "ptr":
        value = '""' if entry["string"] else "nullptr"
        type_ = "const char*" if entry["string"] else "void*"
        return f"{type_} {name}() {{\n{log}    return {value};\n}}\n"
    return f"uint64_t {name}() {{\n{log}    return 0;\n}}\n"


def handwritten_exports(def_text: str) -> set[str]:
    names: set[str] = set()
    inside = False
    for line in def_text.splitlines():
        if line == DEF_BEGIN:
            inside = True
        elif line == DEF_END:
            inside = False
        elif not inside:
            token = line.strip().split()[0] if line.strip() else ""
            if token and token not in ("LIBRARY", "EXPORTS") and not token.startswith(";"):
                names.add(token)
    return names


def generate() -> tuple[dict[Path, str], list[str]]:
    methods = parse_interfaces(INTERFACES.read_text(encoding="utf-8"))
    assign_flat_names(methods)
    api = json.loads(API.read_text(encoding="utf-8"))
    entries = {e["name"]: e for e in api["entradas"]}
    def_text = DEF.read_text(encoding="utf-8")
    handwritten = handwritten_exports(def_text)
    report: list[str] = []

    forwarders: list[Method] = []
    for method in methods:
        if method.flat is None:
            continue
        entry = entries.get(method.flat)
        reason = check(method, entry) if entry else None
        if reason:
            report.append(f"sin reenvío {method.flat}: {reason}")
            continue
        forwarders.append(method)
    generated = {m.flat for m in forwarders}
    duplicated = generated & handwritten
    if duplicated:
        raise SystemExit(f"Exportaciones a mano que ahora se generan (quítalas de flat_api.cpp y del .def): {sorted(duplicated)}")

    stubs = [e for name, e in sorted(entries.items())
             if name.startswith("SteamAPI_ISteam") and name not in generated and name not in handwritten]
    missing = sorted(name for name in entries
                     if not name.startswith("SteamAPI_ISteam") and name not in handwritten)
    if missing:
        raise SystemExit(f"Entradas de Steamworks.NET sin exportar (añádelas a mano en flat_api.cpp y al .def): {missing}")

    cpp = [HEADER]
    cpp.append(f"// {len(forwarders)} reenvíos a las vtables del emulador.\n")
    cpp.extend("\n" + render_forwarder(m) for m in forwarders)
    cpp.append(f"\n// {len(stubs)} entradas que Steamworks.NET importa y el emulador no implementa: valor neutro.\n")
    cpp.extend("\n" + render_stub(e) for e in stubs)
    cpp.append("\n} // extern \"C\"\n")

    exported = sorted(generated | {e["name"] for e in stubs})
    block = [DEF_BEGIN] + [f"    {name}" for name in exported] + [DEF_END]
    if DEF_BEGIN in def_text:
        before = def_text.split(DEF_BEGIN)[0]
        after = def_text.split(DEF_END, 1)[1]
        new_def = before + "\n".join(block) + after
    else:
        new_def = def_text.rstrip("\n") + "\n" + "\n".join(block) + "\n"

    required = sorted(entries)
    header = ["// GENERADO por tools/generar_api_plana.py: entradas de steam_api64 que importa Steamworks.NET.",
              "// La prueba de humo comprueba que la DLL las exporta todas.",
              "#pragma once", "", "namespace sfemu_flat {", "",
              "inline constexpr const char* kSteamworksNetExports[] = {"]
    header += [f'    "{name}",' for name in required]
    header += ["};", "", "} // namespace sfemu_flat", ""]

    report.insert(0, f"{len(forwarders)} reenvíos, {len(stubs)} entradas neutras, "
                     f"{len(required)} entradas de Steamworks.NET cubiertas.")
    return {OUT_CPP: "".join(cpp), OUT_LIST: "\n".join(header), DEF: new_def}, report


def main() -> int:
    outputs, report = generate()
    stale = [path for path, content in outputs.items()
             if not path.exists() or path.read_text(encoding="utf-8") != content]
    for line in report:
        print(line)
    if "--comprobar" in sys.argv:
        if stale:
            print("Salidas desactualizadas: " + ", ".join(str(p.relative_to(ROOT)) for p in stale))
            return 1
        print("OK: API plana al día")
        return 0
    for path in stale:
        path.write_text(outputs[path], encoding="utf-8", newline="\n")
        print("Escrito: " + str(path.relative_to(ROOT)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
