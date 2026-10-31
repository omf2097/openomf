# Create the PSP XMB icon from the artwork distributed by omf2097.com.
# The generated PNG and downloaded archive stay in the build directory.
function(omf_prepare_psp_icon out_var)
    set(icon_dir "${CMAKE_BINARY_DIR}/psp-icon")
    set(icon "${icon_dir}/icon0.png")
    if(NOT EXISTS "${icon}")
        file(MAKE_DIRECTORY "${icon_dir}")
        set(archive "${icon_dir}/openomf-icons.zip")
        if(NOT EXISTS "${archive}")
            file(DOWNLOAD
                "https://www.omf2097.com/pub/files/omf/openomf-icons.zip"
                "${archive}" STATUS download_status TLS_VERIFY ON TIMEOUT 60)
            list(GET download_status 0 download_code)
            if(NOT download_code EQUAL 0)
                file(REMOVE "${archive}")
                message(FATAL_ERROR "Unable to download OpenOMF PSP icon: ${download_status}")
            endif()
        endif()

        find_program(OMF_PSP_UNZIP NAMES unzip)
        if(NOT OMF_PSP_UNZIP)
            message(FATAL_ERROR "PSP icon preparation requires unzip")
        endif()
        set(source_icon "${icon_dir}/omf-logo.png")
        execute_process(
            COMMAND "${OMF_PSP_UNZIP}" -p "${archive}" "omf-logo/omf-256x256.png"
            OUTPUT_FILE "${source_icon}" RESULT_VARIABLE extract_result
            ERROR_VARIABLE extract_error)
        if(NOT extract_result EQUAL 0)
            message(FATAL_ERROR "Unable to extract PSP icon: ${extract_error}")
        endif()

        find_program(OMF_PSP_MAGICK NAMES magick convert)
        find_program(OMF_PSP_SIPS NAMES sips)
        if(OMF_PSP_MAGICK)
            execute_process(
                COMMAND "${OMF_PSP_MAGICK}" "${source_icon}" -resize 80x80
                    -background none -gravity center -extent 144x80 "PNG32:${icon}"
                RESULT_VARIABLE resize_result ERROR_VARIABLE resize_error)
        elseif(OMF_PSP_SIPS)
            execute_process(COMMAND "${OMF_PSP_SIPS}" -z 80 80 "${source_icon}" --out "${icon}"
                RESULT_VARIABLE resize_result ERROR_VARIABLE resize_error OUTPUT_QUIET)
            if(resize_result EQUAL 0)
                execute_process(COMMAND "${OMF_PSP_SIPS}" -p 80 144 --padColor 000000 "${icon}"
                    RESULT_VARIABLE resize_result ERROR_VARIABLE resize_error OUTPUT_QUIET)
            endif()
        else()
            message(FATAL_ERROR "PSP icon preparation requires ImageMagick or macOS sips")
        endif()
        if(NOT resize_result EQUAL 0 OR NOT EXISTS "${icon}")
            file(REMOVE "${icon}")
            message(FATAL_ERROR "Unable to prepare PSP icon: ${resize_error}")
        endif()
    endif()
    set(${out_var} "${icon}" PARENT_SCOPE)
endfunction()
