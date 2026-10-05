/**
 * @file log.h
 * @brief Logging system with multiple outputs and severity levels.
 * @details Provides a flexible logging system that supports multiple output destinations
 *          (stderr, files) with independent log levels. Supports colored output for terminals.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Log message severity levels.
 */
typedef enum log_level
{
    LOG_DEBUG, ///< Debug messages for development
    LOG_INFO,  ///< Informational messages
    LOG_WARN,  ///< Warning messages
    LOG_ERROR  ///< Error messages
} log_level;

/**
 * @brief Channel filter bitmask for log output.
 * Unset means all channels enabled; otherwise only matching bits are shown.
 */
typedef uint32_t log_channel;
enum {
    LOG_CHANNEL_NONE = 0u,
    LOG_CHANNEL_AI_STATE = 1u << 0,    ///< AI state transitions and behavior flow.
    LOG_CHANNEL_AI_DECISION = 1u << 1, ///< AI tactical decisions and intent selection.
    LOG_CHANNEL_REC = 1u << 2,         ///< REC playback/recording controller events.
    LOG_CHANNEL_NETWORK = 1u << 3,     ///< Reserved placeholder for networking/logging during future migration.
    LOG_CHANNEL_GAMEPLAY = 1u << 4,    ///< Reserved placeholder for gameplay systems and world simulation logs.
    LOG_CHANNEL_VIDEO = 1u << 5,       ///< Reserved placeholder for rendering and video pipeline diagnostics.
    LOG_CHANNEL_AUDIO = 1u << 6,       ///< Reserved placeholder for audio system diagnostics.
};

/**
 * @brief Log a debug message.
 * @param ... Printf-style format string and arguments
 */
#define log_debug(...) log_msg(LOG_DEBUG, __VA_ARGS__)

/**
 * @brief Log an informational message.
 * @param ... Printf-style format string and arguments
 */
#define log_info(...) log_msg(LOG_INFO, __VA_ARGS__)

/**
 * @brief Log a warning message.
 * @param ... Printf-style format string and arguments
 */
#define log_warn(...) log_msg(LOG_WARN, __VA_ARGS__)

/**
 * @brief Log an error message.
 * @param ... Printf-style format string and arguments
 */
#define log_error(...) log_msg(LOG_ERROR, __VA_ARGS__)

/**
 * @brief Log a debug message tagged with a channel.
 * @param channel Channel bitmask to tag the message with.
 * @param ... Printf-style format string and arguments
 */
#define log_debug_channel(channel, ...) log_msg_channel((channel), LOG_DEBUG, __VA_ARGS__)

/**
 * @brief Log an informational message tagged with a channel.
 * @param channel Channel bitmask to tag the message with.
 * @param ... Printf-style format string and arguments
 */
#define log_info_channel(channel, ...) log_msg_channel((channel), LOG_INFO, __VA_ARGS__)

/**
 * @brief Log a warning message tagged with a channel.
 * @param channel Channel bitmask to tag the message with.
 * @param ... Printf-style format string and arguments
 */
#define log_warn_channel(channel, ...) log_msg_channel((channel), LOG_WARN, __VA_ARGS__)

/**
 * @brief Log an error message tagged with a channel.
 * @param channel Channel bitmask to tag the message with.
 * @param ... Printf-style format string and arguments
 */
#define log_error_channel(channel, ...) log_msg_channel((channel), LOG_ERROR, __VA_ARGS__)

/**
 * @brief Set the active channel filter.
 * @param channels Bitmask of channels to enable; unset means all channels.
 */
void log_set_channel_filter(log_channel channels);

/**
 * @brief Parse a channel list into a bitmask.
 * @param channels Comma-separated channel names, such as "ai-state,rec".
 * @return Bitmask of enabled channels; NULL/empty leaves all channels enabled.
 */
log_channel log_channels_from_string(const char *channels);

/**
 * @brief Log a message at the specified level, tagged with a channel.
 * @details Prefer using the log_debug_channel/info_channel/warn_channel/error_channel macros instead.
 * @param channel Channel bitmask to tag the message with.
 * @param level Severity level
 * @param fmt Printf-style format string
 * @param ... Format arguments
 */
void log_msg_channel(log_channel channel, log_level level, const char *fmt, ...) ATTR_FORMAT_PRINTF(3, 4);

/**
 * @brief Initialize the logging system.
 * @details Must be called before any logging functions.
 */
void log_init(void);

/**
 * @brief Close the logging system and release resources.
 * @details Closes all open log file handles.
 */
void log_close(void);

/**
 * @brief Set the minimum log level for all outputs.
 * @details Messages below this level will not be logged anywhere.
 * @param level Minimum log level
 */
void log_set_level(log_level level);

/**
 * @brief Enable or disable colored output globally.
 * @param toggle true to enable colors, false to disable
 */
void log_set_colors(bool toggle);

/**
 * @brief Add stderr as a log output destination.
 * @param level Minimum log level for this output
 * @param colors Whether to use ANSI colors for this output
 */
void log_add_stderr(log_level level, bool colors);

/**
 * @brief Add a file as a log output destination.
 * @param filename Path to the log file
 * @param level Minimum log level for this output
 */
void log_add_file(const char *filename, log_level level);

/**
 * @brief Convert a log level string to enum value.
 * @param level String representation (e.g., "DEBUG", "INFO")
 * @param default_value Value to return if string is not recognized
 * @return The corresponding log_level enum value
 */
log_level log_level_text_to_enum(const char *level, log_level default_value);

/**
 * @brief Check if a string is a valid log level name.
 * @param level String to check
 * @return true if valid log level, false otherwise
 */
bool is_log_level(const char *level);

/**
 * @brief Log a message at the specified level.
 * @details Prefer using the log_debug/info/warn/error macros instead.
 * @param level Severity level
 * @param fmt Printf-style format string
 * @param ... Format arguments
 */
void log_msg(log_level level, const char *fmt, ...) ATTR_FORMAT_PRINTF(2, 3);

/**
 * @brief Get the last logged error message.
 * @return The most recent error message, or NULL if none
 */
const char *log_last_error(void);

#endif // LOG_H
