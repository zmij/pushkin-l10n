# pushkin-l10n

Project contains cmake functions for localization maintenance and a library for working with foreign assets that require runtime localization (e.g. some data in csv or json formats).

## cmake script

Script for cmake is [cmake/scripts/l10n.cmake](https://github.com/zmij/pushkin-l10n/blob/develop/cmake/scripts/l10n.cmake). It doen't requre the C++ libraries to be build and can be used standalone. It can be used as follows:

```cmake
# In project's root CMakeLists.txt
if(NOT COMMAND parse_argn)
    # This command is required by l10n script.
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/argn.cmake)
endif()
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/l10n.cmake)

# Interpreter mode is needed to invoke message extraction
# programs/scripts. If there are no *.po files in the source
# tree, at least a one run in interpreter mode is needed
# to generate the *.pot and *.po files and add them to
# your repository.
# For those who is not working with the translations,
# regeneration of *.pot and *.po files may be annoying,
# as the files will be modified each time the sources 
# change, so the interpreter mode should be turned off,
# and only *.mo files will be compiled from *.po files
# that are in the source tree.
option(INTERPRETER_MODE "Interpreter mode (extract string, generate pot/po files)" OFF)
# In the interpreter mode tests can be added to the test
# suite that run simple checks on the translation files
# (msgcmp, actually, to check that all strings are tranlated)
option(CHECK_TRANSLATION "Generate tests to check translations" OFF)

l10n_project(
    # The project name. Will be passed to extraction program in option --package-name
    PACKAGE ${PROJECT_NAME}
    # The project version. Will be passed to extraction program in option --package-version
    PACKAGE_VERSION ${PROJECT_VERSION}
    # Copyright. Will be passed to extraction program in option --copyright-holder
    # If none specified, option --foreing-user will be passed
    COPYRIGHT "Some copyright"
    # Address to send msgid error. Will be passed to extraction program in option --msgid-bugs-address
    BUGS "someone@example.com"
    # Directory where *.pot files will be generated. Language *.po files will be 
    # generated in respective subdirectories.
    # In non-interpreter mode the *.po files will be used as sources to compile
    # *.mo files, therefore they need to exist.
    POT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/translations
    # Directory for compiled binary *.mo files. No use to store them in the source tree.
    MO_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/messages
    # Directory to install *.mo files
    INSTALL_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/messages
    # List of languages to generate *.po files and/or compile *.mo files.
    LANGUAGES en de fr
    # Interpreter mode switch
    INTERPRETER ${INTERPRETER_MODE}
)
```
The `l10n_project` function will set the following variables: `L10N_PACKAGE`, `L10N_PACKAGE_VERSION`, `L10N_COPYRIGHT`, `L10N_BUGS`, `L10N_POT_DIRECTORY`, `L10N_MO_DIRECTORY`, `L10N_INSTALL_DIRECTORY`, `L10N_LANGUAGES` and `L10N_INTERPRETER`. Those variables are used by message extraction functions. The functions can override most of them. A subdirectory of source tree can have nested `l10n_project`s, the `l10n_project` function call will override all variables for the respective subtree.

To extract messages from sources `localize` function can be used to generate targets that invoke `xgettext` to extract messages, `msginit` and `msgmerge` to generate or update \*.po files and `msgfmt` to compile \*.mo files. It can also generate test targets that invoke `msgcmp` to check that all strings are translated.
```cmake
localize(
    # List of sources to extract strings from. Mandatory parameter.
    SOURCES main.cpp other.cpp
    # Domain of strings. Actually it will be the file name (e.g. my_strings.pot, en/my_strings.po)
    # Default "messages"
    DOMAIN my_strings
    # Generate install targets for *.mo files. The installation directory is set by l10n_project function.
    # Default OFF
    INSTALL ON
    # Generate tests for translation files. Will be ignored in non-interpreter mode.
    # Default OFF
    TEST_TRANSLATION ${CHECK_TRANSLATION}
    # Add localization dependency to the target.
    # Optional, no default value
    TARGET my_program
    # Additional options for the xgettext program.
    # Default options are:
    #   --from-code=UTF-8
    #   --sort-by-file
    #   --keyword=translate:1,1t
    #   --keyword=translate:1c,2,2t
    #   --keyword=translate:1,2,3t
    #   --keyword=translate:1c,2,3,4t
    #   --keyword=gettext:1
    #   --keyword=pgettext:1c,2
    #   --keyword=ngettext:1,2
    #   --keyword=npgettext:1c,2,3
    # They are suitable to extract gettext and ::boost::locale translation calls.
    OPTIONS --extract-all --indent --omit-header
    # The rest of argument are intended to override variables set by l10n_project function 
    # and are optional.
    POT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/translations
    PACKAGE ${PROJECT_NAME}
    PACKAGE_VERSION ${PROJECT_VERSION}
    COPYRIGHT "Some copyright"
    BUGS "someone@example.com"
)
```
To invoke a custom message extraction command, `extract_l10n` function can be used. The generator command should accept the same parameters as `xgettext` does and produce a file that is passed via `--output` argument.
```cmake
extract_l10n(
    PROGRAM my_pot_generator
    # Rest of options are the same as that of the localize function
)
```

## C++ libraries

### messages

- [ ] TODO Describe the purpose and usage of `psst::l10n::message`

### po_generator

- [ ] TODO Describe the purpose and usage of `psst::l10n::po_generator`
