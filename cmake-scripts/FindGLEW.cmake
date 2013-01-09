IF(WIN32)
    FIND_PATH(GLEW_INCLUDE_PATH GL/glew.h
        $ENV{PROGRAMFILES}/GLEW/include
        /usr/local/include
        /usr/include
        /usr/include
    )
    FIND_LIBRARY(GLEW_LIBRARY
        NAMES glew GLEW glew32 glew32s
        PATHS
        /usr/local/lib
        /usr/lib64
        /usr/lib
        $ENV{PROGRAMFILES}/GLEW/lib
    )
ELSE(WIN32)
    FIND_PATH(GLEW_INCLUDE_PATH GL/glew.h
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
    )
    FIND_LIBRARY(GLEW_LIBRARY
        NAMES GLEW glew
        PATHS
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
    )
ENDIF(WIN32)

IF(GLEW_INCLUDE_PATH AND GLEW_LIBRARY)
   SET(GLEW_FOUND TRUE)
ENDIF (GLEW_INCLUDE_PATH AND GLEW_LIBRARY)

IF(GLEW_FOUND)
    MESSAGE(STATUS "Found GLEW: ${GLEW_LIBRARY}")
ELSE(GLEW_FOUND)
    MESSAGE(WARNING "Could not find GLEW!")
ENDIF(GLEW_FOUND)