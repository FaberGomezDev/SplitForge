cmake_minimum_required(VERSION 3.20)
project(SplitForgeXInputFilter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # Sin dependencias del runtime de VC++.

add_library(SplitForge.XInputFilter SHARED xinput_filter.cpp xinput_filter.def)
set_target_properties(SplitForge.XInputFilter PROPERTIES OUTPUT_NAME "SplitForge.XInputFilter" PREFIX "")

if(MSVC)
  target_compile_options(SplitForge.XInputFilter PRIVATE /W4 /WX /permissive- /O2 /GS /guard:cf)
  target_link_options(SplitForge.XInputFilter PRIVATE /guard:cf /DYNAMICBASE /NXCOMPAT)
else()
  target_compile_options(SplitForge.XInputFilter PRIVATE -Wall -Wextra -Werror -O2)
  target_link_options(SplitForge.XInputFilter PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Prueba de humo: carga el filtro renombrado como xinput1_4.dll y comprueba el filtrado y el estado.
add_executable(SplitForge.XInputFilter.Smoke smoke_test.cpp)
if(MSVC)
  target_compile_options(SplitForge.XInputFilter.Smoke PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(SplitForge.XInputFilter.Smoke PRIVATE -Wall -Wextra -Werror -municode)
  target_link_options(SplitForge.XInputFilter.Smoke PRIVATE -municode -static)
endif()
add_dependencies(SplitForge.XInputFilter.Smoke SplitForge.XInputFilter)
add_custom_command(TARGET SplitForge.XInputFilter.Smoke POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SplitForge.XInputFilter> $<TARGET_FILE_DIR:SplitForge.XInputFilter.Smoke>/xinput1_4.dll)
enable_testing()
add_test(NAME filtro_xinput COMMAND SplitForge.XInputFilter.Smoke $<TARGET_FILE_DIR:SplitForge.XInputFilter.Smoke>/xinput1_4.dll)
