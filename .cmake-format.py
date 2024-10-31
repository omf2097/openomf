# A python-style configuration file for cmake-format
# See their documentation for more information:
# https://cmake-format.readthedocs.io/en/latest/configuration.html
#
# For info on how to run cmake-format, see CONTRIBUTING.md

# ------------------------------------------------
# Options affecting comment reflow and formatting.
# ------------------------------------------------
with section("markup"):

  # If comment markup is enabled, don't reflow any comment block which matches
  # this (regex) pattern. Default is `None` (disabled).
  #
  # An empty regex is used to match all comments, and disable comment reflow
  literal_comment_pattern = ''
