# Generic packaging options
SET(VERSION 0.6.5)
SET(CPACK_PACKAGE_VERSION_MAJOR 0)
SET(CPACK_PACKAGE_VERSION_MINOR 6)
SET(CPACK_PACKAGE_VERSION_PATCH 5)
SET(CPACK_PACKAGE_FILE_NAME "openomf_${VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Complete engine remake for the title One Must Fall: 2097.")
SET(CPACK_PACKAGE_DESCRIPTION "Open Source remake of One Must Fall 2097 by Diversions Entertainment. Since the original DOS game from 1994 still uses IPX networking and is a pain to set up, the community needed a better solution to keep playing the game we love. Together with networking, we try to make it easier to play One Must Fall in original glory on multiple platforms (Linux, Mac OSX, Windows, BSD to name a few).")

# Add packaging subdirectories here
add_subdirectory(debian)

