/*! \file
 * \brief Format handler error handling.
 * \details Functions for dealing with format handler errors.
 * \copyright MIT license.
 * \date 2013-2014
 * \author animehunter
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_ERROR_H
#define SD_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUGMODE
    void debug_print(const char* fn, int line, const char *fmt, ...);
    #define DEBUGLOG(...) debug_print(__func__, __LINE__, __VA_ARGS__)
#else
    #define DEBUGLOG(...) ///< Prints debug text if the debugging mode is on.
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
    SD_FILE_WRITE_ERROR, ///< File could not be written
    SD_FILE_READ_ERROR ///< File could not be read
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

#endif // SD_ERROR_H
