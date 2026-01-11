/**
 * @file config.h
 * @brief Configuration file management.
 * @details Provides functions to read, write, and manipulate configuration values
 *          stored in an INI-style configuration file. Supports integer, boolean,
 *          float, and string value types.
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * @brief Register an integer configuration option.
 * @param name Option name (key)
 * @param default_val Default value if not found in config file
 */
void conf_addint(char *name, int default_val);

/**
 * @brief Register a boolean configuration option.
 * @param name Option name (key)
 * @param default_val Default value if not found in config file (0 or 1)
 */
void conf_addbool(char *name, int default_val);

/**
 * @brief Register a float configuration option.
 * @param name Option name (key)
 * @param default_val Default value if not found in config file
 */
void conf_addfloat(char *name, double default_val);

/**
 * @brief Register a string configuration option.
 * @param name Option name (key)
 * @param default_val Default value if not found in config file
 */
void conf_addstring(char *name, char *default_val);

/**
 * @brief Initialize the configuration system and load a config file.
 * @param filename Path to the configuration file to load
 * @return 0 on success, non-zero on error
 */
int conf_init(const char *filename);

/**
 * @brief Write current configuration values to a file.
 * @param filename Path to the configuration file to write
 * @return 0 on success, non-zero on error
 */
int conf_write_config(const char *filename);

/**
 * @brief Close the configuration system and free resources.
 */
void conf_close(void);

/**
 * @brief Get an integer configuration value.
 * @param name Option name (key)
 * @return The integer value, or 0 if not found
 */
int conf_int(const char *name);

/**
 * @brief Get a float configuration value.
 * @param name Option name (key)
 * @return The float value, or 0.0 if not found
 */
double conf_float(const char *name);

/**
 * @brief Get a boolean configuration value.
 * @param name Option name (key)
 * @return The boolean value (0 or 1), or 0 if not found
 */
int conf_bool(const char *name);

/**
 * @brief Get a string configuration value.
 * @param name Option name (key)
 * @return The string value, or NULL if not found. Do not free this pointer.
 */
const char *conf_string(const char *name);

/**
 * @brief Set an integer configuration value.
 * @param name Option name (key)
 * @param val New value
 */
void conf_setint(const char *name, int val);

/**
 * @brief Set a float configuration value.
 * @param name Option name (key)
 * @param val New value
 */
void conf_setfloat(const char *name, double val);

/**
 * @brief Set a boolean configuration value.
 * @param name Option name (key)
 * @param val New value (0 or 1)
 */
void conf_setbool(const char *name, int val);

/**
 * @brief Set a string configuration value.
 * @param name Option name (key)
 * @param val New value (will be copied)
 */
void conf_setstring(const char *name, const char *val);

#endif // CONFIG_H
