#
# version.cmake has the job of setting five variables:
# - SHA1_HASH, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, AND VERSION_LABEL
#
# It calculates these by running git commands.
#
# Although it is used in CMakeLists.txt, version.cmake is also run from github actions
# as a standalone cmake script, so that our actions have access to the version information as well.
#

set(GIT_REMOTE origin)

# set version in case we error
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_LABEL)

# file-internal macro for things we want to do before finishing this script,
# whether we're on the success path or error path.
macro(version_finally)
    if(DEFINED SHA1_HASH_SHORT)
        string(APPEND VERSION_LABEL "-${SHA1_HASH_SHORT}")
    endif()

    if(DEFINED CMAKE_SCRIPT_MODE_FILE)
        # output exactly what the github actions wants
        message("${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_LABEL}")
    else()
        message(STATUS "Git SHA1 Hash: ${SHA1_HASH}")
        message(STATUS "OpenOMF Version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_LABEL}")
    endif()
endmacro()

# file-internal macro for reporting a fatal error.
#
# Although the error is fatal to version.cmake, it is only
# raised as a cmake fatal error if VERSION_REQUIRED is set.
# This lets the project be built even if version detection fails.
macro(version_error error)
    # put error into version label
    string(APPEND VERSION_LABEL "-${error}")

    if(NOT DEFINED SHA1_HASH)
        set(SHA1_HASH "${error}")
    endif()

    version_finally()

    # Enable VERSION_REQUIRED to treat any errors as fatal.
    if(VERSION_REQUIRED)
        message(FATAL_ERROR "Version did not succeed, ${error}.")
    endif()

    # exit the version.cmake script, we're done.
    return()
endmacro()


if(DEFINED CMAKE_SCRIPT_MODE_FILE)
    # suppress find_package's status message in script mode
    set(GIT_FIND_QUIETLY QUIET)
    # treat version errors as fatal
    set(VERSION_REQUIRED ON)
endif()

find_package(Git ${GIT_FIND_QUIETLY})
if(NOT Git_FOUND)
    message(WARNING "Git not found")
    version_error("GITNOTFOUND")
endif()

# Set SHA1_HASH
execute_process(
    COMMAND "${GIT_EXECUTABLE}" "rev-parse" "HEAD"
    OUTPUT_VARIABLE "SHA1_HASH"
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set SHA1_HASH_SHORT
execute_process(
    COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--short" "HEAD"
    OUTPUT_VARIABLE "SHA1_HASH_SHORT"
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Check if tag 0.4 (the first tagged openomf release) is known.
# If 0.4 is known, the tags are assumed to be up-to-date and complete.
execute_process(
    COMMAND "${GIT_EXECUTABLE}" cat-file -e "0.4"
    OUTPUT_QUIET ERROR_QUIET
)
if(GIT_ERRORCODE)
    # tag 0.4 wasn't found, so fetch all tags
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" fetch "${GIT_REMOTE}" "+refs/tags/*:refs/tags/*" "--depth=1"
        RESULT_VARIABLE "GIT_ERRORCODE"
        OUTPUT_VARIABLE "GIT_LOG"
        ERROR_VARIABLE "GIT_LOG"
    )

    if(GIT_ERRORCODE)
        message(WARNING "Failed to fetch tags.\n${GIT_LOG}")
        version_error("TAGFETCHFAILED")
    endif()
endif()

# Set versionsort.suffix to -, so `0.8.0-rc1` is considered an older version than `0.8.0`.
execute_process(
    COMMAND "${GIT_EXECUTABLE}" config "--comment" "set by version.cmake, to sort prerelease versions as older than releases." "versionsort.suffix" "-"
    OUTPUT_QUIET ERROR_QUIET
)

# List all tags, newest to oldest
execute_process(
    COMMAND "${GIT_EXECUTABLE}" tag "--sort=-version:refname"
    ERROR_QUIET
    OUTPUT_VARIABLE "VERSION_TAGS"
)

# example of what could be in VERSION_TAGS after the previous command:
#
# test-build
# latest
# 0.8.1-rc1
# 0.8.0
#
#
# the first two are not releases.
# although 0.8.1-rc1 is a prerelease, we want to extract it and consider it the latest release.


# grab the date of the commit we're currently building, formatted as a Unix Timestamp.
execute_process(
    COMMAND "${GIT_EXECUTABLE}" show -s "--format=%ct" "HEAD"
    ERROR_QUIET
    OUTPUT_VARIABLE "HEAD_TIMESTAMP"
    RESULT_VARIABLE "GIT_ERRORCODE"
)

# strip all but the last line
string(REGEX MATCH "([^\n]+)\n*$" outvar_dontcare "${HEAD_TIMESTAMP}")
set(HEAD_TIMESTAMP "${CMAKE_MATCH_1}")

while(1) # loop until we found a release that is not newer than HEAD

    if("${VERSION_TAGS}" STREQUAL "")
        # if this version error is tripped, I suspect someone's RTC battery
        # has died and their computer time has reset to the 1970's.
        version_error("CLOCKERROR")
    endif()

    # use regex to extract the first release tag from the list
    string(REGEX MATCH "(^|\n)([0-9]+[.][0-9]+[.][0-9]+(-)[A-Za-z0-9.]*)(\n.*|$)" outvar_dontcare "${VERSION_TAGS}")
    set(LATEST_RELEASE_TAG "${CMAKE_MATCH_2}")
    set(VERSION_TAGS "${CMAKE_MATCH_4}") # set VERSION_TAGS to the older ones we haven't look at yet.

    # grab the date of the LATEST_RELEASE_TAG commit (as a unix timestamp)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" show -s "--format=%ct" "${LATEST_RELEASE_TAG}"
        ERROR_QUIET
        OUTPUT_VARIABLE "RELEASE_TIMESTAMP"
        RESULT_VARIABLE "GIT_ERRORCODE"
    )

    # strip all but the last line
    string(REGEX MATCH "([^\n]+)\n*$" outvar_dontcare "${RELEASE_TIMESTAMP}")
    set(RELEASE_TIMESTAMP "${CMAKE_MATCH_1}")

    if("${RELEASE_TIMESTAMP}" LESS_EQUAL "${HEAD_TIMESTAMP}")
        # Good, we found a release that isn't from the future.
        break()
    endif()
endwhile()

# parse latest release into VERSION_ vars
string(REGEX MATCH "^([0-9]+)[.]([0-9]+)[.]([0-9]+)(-.*)?" outvar_dontcare "${LATEST_RELEASE_TAG}")
set(VERSION_MAJOR "${CMAKE_MATCH_1}")
set(VERSION_MINOR "${CMAKE_MATCH_2}")
set(VERSION_PATCH "${CMAKE_MATCH_3}")
set(VERSION_LABEL "${CMAKE_MATCH_4}")

# Check if we're on exactly LATEST_RELEASE_TAG
execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe HEAD --exact-match --match "${LATEST_RELEASE_TAG}"
    OUTPUT_QUIET ERROR_QUIET
    RESULT_VARIABLE GIT_ERRORCODE
)
if(NOT GIT_ERRORCODE)
    # a perfect match!
    version_finally()
    return()
endif()

# count commits
execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-list "${LATEST_RELEASE_TAG}..HEAD" --count
    OUTPUT_VARIABLE "VERSION_COUNT"
    ERROR_QUIET
    RESULT_VARIABLE "GIT_ERRORCODE"
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(GIT_ERRORCODE)
    # git rev-list RELEASE..HEAD failed, so we're probably missing some of the commits inbetween.
    # Let's deepen our shallow checkout using `git fetch --shallow-since <DATE OF COUNTFROM>`

    # grab the date of the LATEST_RELEASE_TAG commit (in a human readable ISO 8601-like form)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" show -s "--format=%ci" "${LATEST_RELEASE_TAG}"
        ERROR_QUIET
        OUTPUT_VARIABLE "LATEST_RELEASE_DATE"
        RESULT_VARIABLE "GIT_ERRORCODE"
    )

    # strip all but the last line
    string(REGEX MATCH "([^\n]+)\n*$" outvar_dontcare "${LATEST_RELEASE_DATE}")
    set(LATEST_RELEASE_DATE "${CMAKE_MATCH_1}")

    # Fetch commits dating back to LATEST_RELEASE_DATE.
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" fetch "${GIT_REMOTE}" "--shallow-since=${LATEST_RELEASE_DATE}"
        RESULT_VARIABLE "GIT_ERRORCODE"
        OUTPUT_VARIABLE "GIT_LOG"
        ERROR_VARIABLE "GIT_LOG"
    )

    if(GIT_ERRORCODE)
        message(WARNING "Failed to deepen fetch\n${GIT_LOG}")
        version_error("DEEPENFETCHFAILED")
    endif()

    # try to count the commits again
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-list "${LATEST_RELEASE_TAG}..HEAD" --count
        OUTPUT_VARIABLE "VERSION_COUNT"
        RESULT_VARIABLE "GIT_ERRORCODE"
        ERROR_VARIABLE "GIT_LOG"
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(GIT_ERRORCODE)
        message(WARNING "After fetching commits since ${LATEST_RELEASE_DATE}, we still can't count the commits from ${LATEST_RELEASE_TAG} to HEAD.\n${GIT_LOG}")
        version_error("INEFFECTUALFETCH")
    endif()
endif()

if("${VERSION_LABEL}" STREQUAL "")
    # increment VERSION_PATCH, because we're newer than the latest release.
    math(EXPR VERSION_PATCH "${VERSION_PATCH} + 1")
else()
    # VERSION_LABEL wasn't empty, so we're on a prerelease.
    # don't bump VERSION_PATCH.
endif()

# append VERSION_COUNT to VERSION_LABEL
string(APPEND VERSION_LABEL "-${VERSION_COUNT}")

version_finally()
