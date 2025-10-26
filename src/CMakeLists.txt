cmake_minimum_required(VERSION 3.13)

project(dcjam2025)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    main.cpp
    math.cpp
    cam.cpp
    player.cpp
    level.cpp
    primitives.cpp
    textures.cpp
    ui.cpp
    renderer.cpp
    audio.cpp
    settings.cpp
    prefabs.cpp
    )

set(EDITOR_SOURCES
    editor/editor.cpp
    editor/editor_ui.cpp
    editor/cam.cpp
    editor/player.cpp
    )

set(GAME_SOURCES
    game/game.cpp
    game/game_ui.cpp
    game/cam.cpp
    game/player.cpp
    game/spawn.cpp
    game/triggers.cpp
    )


if(MSVC)
  # https://github.com/raysan5/raylib/issues/857
  add_compile_definitions(NOVIRTUALKEYCODES NOWINMESSAGES NOWINSTYLES NOSYSMETRICS NOMENUS NOICONS NOKEYSTATES NOSYSCOMMANDS NORASTEROPS NOSHOWWINDOW OEMRESOURCE NOATOM NOCLIPBOARD NOCOLOR NOCTLMGR NODRAWTEXT NOGDI NOKERNEL NOUSER NOMB NOMEMMGR NOMETAFILE NOMINMAX NOMSG NOOPENFILE NOSCROLL NOSERVICE NOSOUND NOTEXTMETRIC NOWH NOWINOFFSETS NOCOMM NOKANJI NOHELP NOPROFILER NODEFERWINDOWPOS NOMCX)
endif()
if (EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -DGRAPHICS_API_OPENGL_ES3 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 -s STACK_SIZE=1mb -s ALLOW_MEMORY_GROWTH=1  --preload-file ${CMAKE_CURRENT_BINARY_DIR}/../../res@res/")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -DGRAPHICS_API_OPENGL_ES3 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 -s STACK_SIZE=1mb -s ALLOW_MEMORY_GROWTH=1  --preload-file ${CMAKE_CURRENT_BINARY_DIR}/../../res@res/")
  set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so taht you can directly open it.
endif ()
    
add_executable(editor ${SOURCES} ${EDITOR_SOURCES})
add_executable(game ${SOURCES} ${GAME_SOURCES})
#target_include_directories(dcjam2025 PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../src)
target_link_libraries(editor PUBLIC raylib glfw eecs edat)
target_link_libraries(game PUBLIC raylib glfw eecs edat)

