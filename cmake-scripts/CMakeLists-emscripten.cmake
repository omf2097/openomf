# Build instructions 
#
# mkdir build
# cd build
# emconfigure cmake.exe -G "Unix Makefiles" -DCMAKE_MAKE_PROGRAM=/path/to/make.exe -DEMSCRIPTEN_ROOT_PATH=/path/to/emscripten -DCMAKE_BUILD_TYPE=Release ..
# emmake make.exe

if(EMSCRIPTEN)
    message(STATUS "Emscripten toolchain selected")
    if(NOT "${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
        if(EXISTS "${EMSCRIPTEN_ROOT_PATH}" AND IS_DIRECTORY "${EMSCRIPTEN_ROOT_PATH}")
            SET(WIN32)
            SET(APPLE)
            set(CMAKE_C_SIZEOF_DATA_PTR 4)
            set(CMAKE_CXX_SIZEOF_DATA_PTR 4)

            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-warn-absolute-paths")
            
            get_filename_component(EMSCRIPTEN_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}" ABSOLUTE)
            set(CMAKE_C_COMPILER ${EMSCRIPTEN_ROOT_PATH}/emcc)
            set(CMAKE_LINKER ${EMSCRIPTEN_ROOT_PATH}/emcc)
            set(CMAKE_AR ${EMSCRIPTEN_ROOT_PATH}/emar)
            set(CMAKE_RANLIB ${EMSCRIPTEN_ROOT_PATH}/emranlib)
            set(CMAKE_C_LINK_EXECUTABLE "${EMSCRIPTEN_ROOT_PATH}/emcc -o <TARGET>.html -Wno-warn-absolute-paths <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES>")
            set(CMAKE_C_CREATE_STATIC_LIBRARY  "${EMSCRIPTEN_ROOT_PATH}/emar rc <TARGET> <LINK_FLAGS> <OBJECTS> "
                                               "<CMAKE_RANLIB> <TARGET> ")

            message(STATUS "Found Emscripten at '${EMSCRIPTEN_ROOT_PATH}'")
        else()
            message(FATAL_ERROR "Failed to find Emscripten at '${EMSCRIPTEN_ROOT_PATH}'")
        endif()
    else()
        message(FATAL_ERROR "The variable EMSCRIPTEN_ROOT_PATH is not set! Please set it and try again")
    endif()
endif(EMSCRIPTEN)
