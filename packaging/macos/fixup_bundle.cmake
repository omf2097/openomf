include(BundleUtilities)

if(NOT DEFINED APP)
    message(FATAL_ERROR "APP is required")
endif()

if(NOT DEFINED SEARCH_DIRS)
    set(SEARCH_DIRS "")
endif()

fixup_bundle("${APP}" "" "${SEARCH_DIRS}")
