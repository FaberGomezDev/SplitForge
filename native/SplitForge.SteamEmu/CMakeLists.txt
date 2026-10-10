cmake_minimum_required(VERSION 3.20)
project(SplitForgeSteamEmu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # Sin dependencias del runtime de VC++.

# La salida se llama steam_api64.dll a propósito: es la DLL de Steam que el juego carga.
add_library(SplitForge.SteamEmu SHARED
  src/flat_api.cpp
  src/flat_generated.cpp
  src/flat_networking.cpp
  src/interfaces_core.cpp
  src/interfaces_stats.cpp
  src/interfaces_multiplayer.cpp
  src/interfaces_content.cpp
  src/interfaces_extra.cpp
  src/interfaces_misc.cpp
  src/minijson.cpp
  src/net.cpp
  src/state.cpp
  steamemu.def)
set_target_properties(SplitForge.SteamEmu PROPERTIES OUTPUT_NAME "steam_api64" PREFIX "")

if(MSVC)
  target_compile_options(SplitForge.SteamEmu PRIVATE /W4 /WX /permissive- /O2 /GS /guard:cf /utf-8 /D_CRT_SECURE_NO_WARNINGS)
  target_link_options(SplitForge.SteamEmu PRIVATE /guard:cf /DYNAMICBASE /NXCOMPAT)
else()
  target_compile_options(SplitForge.SteamEmu PRIVATE -Wall -Wextra -Werror -O2)
  target_link_options(SplitForge.SteamEmu PRIVATE -static -static-libgcc -static-libstdc++)
endif()
target_link_libraries(SplitForge.SteamEmu PRIVATE ws2_32)

# Prueba de humo: carga steam_api64.dll, comprueba identidad/config, stats, almacenamiento
# y un intercambio P2P entre dos procesos del mismo PC (padre e hijo con configs distintas).
add_executable(SplitForge.SteamEmu.Smoke smoke_test.cpp)
target_include_directories(SplitForge.SteamEmu.Smoke PRIVATE src)
if(MSVC)
  target_compile_options(SplitForge.SteamEmu.Smoke PRIVATE /W4 /WX /permissive- /utf-8 /D_CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(SplitForge.SteamEmu.Smoke PRIVATE -Wall -Wextra -Werror -municode)
  target_link_options(SplitForge.SteamEmu.Smoke PRIVATE -municode -static)
endif()
add_dependencies(SplitForge.SteamEmu.Smoke SplitForge.SteamEmu)
add_custom_command(TARGET SplitForge.SteamEmu.Smoke POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SplitForge.SteamEmu> $<TARGET_FILE_DIR:SplitForge.SteamEmu.Smoke>/steam_api64.dll)

enable_testing()
add_test(NAME steamemu_humo COMMAND SplitForge.SteamEmu.Smoke $<TARGET_FILE_DIR:SplitForge.SteamEmu.Smoke>)
set_tests_properties(steamemu_humo PROPERTIES TIMEOUT 90)
