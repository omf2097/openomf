/*! \file
 * \brief LibShadowDive error handling.
 * \details Functions for dealing with libShadowDive errors.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */ 

#ifndef _SD_ERROR_H
#define _SD_ERROR_H

#ifdef __cplusplus 
extern "C" {
#endif

#ifdef DEBUGMODE
    void debug_print(const char* fn, int line, const char *fmt, ...);
    #define DEBUGLOG(...) debug_print(__FUNCTION__, __LINE__, __VA_ARGS__)
#else
    #define DEBUGLOG(...)
#endif

/*! \brief Errorcode list.
 */
enum SD_ERRORCODE {
    SD_SUCCESS, ///< Success message
    SD_FILE_OPEN_ERROR, ///< File could not be opened
    SD_FILE_INVALID_TYPE, ///< File was of invalid type
    SD_FILE_PARSE_ERROR, ///< File had a syntax error
    SD_ANIM_INVALID_STRING, ///< Invalid animation string
    SD_OUT_OF_MEMORY, ///< Out of memory error
    SD_INVALID_INPUT, ///< Function encountered unexpected/invalid arguments
    SD_FORMAT_NOT_SUPPORTED, ///< File format is not supported
    SD_INVALID_TAG, ///< Invalid tag in animation string
};

/*! \brief Get text error for error ID
 *
 * Returns a clear text error message for the given error ID.
 *
 * \sa SD_ERRORCODE
 * \param errorcode Errorcode
 * \return Error message
 */
const char* sd_get_error(int errorcode);

#ifdef __cplusplus
}
#endif

#endif // _SD_ERROR_H
