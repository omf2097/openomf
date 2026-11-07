include(CheckIncludeFile)

check_include_file(uchar.h OMF_HAVE_UCHAR_H)
if(OMF_HAVE_UCHAR_H)
    add_compile_definitions(OMF_HAVE_UCHAR_H)
endif()
