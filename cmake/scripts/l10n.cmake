if (NOT XGETTEXT)
    find_program(XGETTEXT xgettext)
    if (NOT XGETTEXT)
        message(FATAL_ERROR "Failed to find xgettext program")
    endif()
endif()
if (NOT MSGINIT)
    find_program(MSGINIT msginit)
    if (NOT MSGINIT)
        message(FATAL_ERROR "Failed to find msginit program")
    endif()
endif()
if (NOT MSGMERGE)
    find_program(MSGMERGE msgmerge)
    if (NOT MSGMERGE)
        message(FATAL_ERROR "Failed to find msgmerge program")
    endif()
endif()
if (NOT MSGFMT)
    find_program(MSGFMT msgfmt)
    if (NOT MSGFMT)
        message(FATAL_ERROR "Failed to find msgfmt program")
    endif()
endif()
if (NOT MSGCMP)
    find_program(MSGCMP msgcmp)
    if (NOT MSGCMP)
        message(FATAL_ERROR "Failed to find msgcmp program")
    endif()
endif()
if (NOT L10N_MSGMERGE_SCRIPT)
    find_file(
        L10N_MSGMERGE_SCRIPT msgmerge.cmake
        NO_DEFAULT_PATH
        HINTS
            ${CMAKE_SOURCE_DIR}/cmake/scripts
            ${CMAKE_SOURCE_DIR}/lib/l10n/cmake/scripts    # When in a subproject
        DOC "CMake script for merging gettext message files"
    )
    if (NOT L10N_MSGMERGE_SCRIPT)
        message(FATAL_ERROR "Failed to find CMake script for merging gettext message files")
    endif()
endif()

function(l10n_project)
    set(argnames
        PACKAGE PACKAGE_VERSION COPYRIGHT BUGS LANGUAGES
        POT_DIRECTORY MO_DIRECTORY INSTALL_DIRECTORY
        INTERPRETER
    )
    parse_argn("" argnames ${ARGN})
    if (NOT PACKAGE)
        message(FATAL_ERROR "Package name is not set for localization project")
    endif()
    if (NOT POT_DIRECTORY)
        set(POT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if (NOT MO_DIRECTORY)
        set(MO_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale)
    endif()
    if (NOT INSTALL_DIRECTORY)
        set(INSTALL_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/${PACKAGE}/locale)
    endif()
    message(STATUS "POT directory for ${PACKAGE} is ${POT_DIRECTORY}")
    message(STATUS "MO directory for ${PACKAGE} is ${MO_DIRECTORY}")
    if (NOT EXISTS POT_DIRECTORY)
        file(MAKE_DIRECTORY ${POT_DIRECTORY})
    endif()
    if (NOT EXISTS MO_DIRECTORY)
        file(MAKE_DIRECTORY ${MO_DIRECTORY})
    endif()

    foreach(arg_name ${argnames})
        if (${arg_name})
            set(L10N_${arg_name} ${${arg_name}} PARENT_SCOPE)
        endif()
    endforeach()

    set(L10N_DOMAINS NOTFOUND PARENT_SCOPE)
    add_custom_target(
        ${PACKAGE}.i18n ALL
        COMMENT "Build l10n for package ${PACKAGE}"
    )
endfunction()

function(compile_mo DOMAIN POT_FILE)
    set(argnames INSTALL)
    parse_argn("" ${argnames} ${ARGN})
    foreach(lang ${L10N_LANGUAGES})
        set(po_dir ${L10N_POT_DIRECTORY}/${lang})
        set(mo_dir ${L10N_MO_DIRECTORY}/${lang}/LC_MESSAGES)
        set(install_dir ${L10N_INSTALL_DIRECTORY}/${lang}/LC_MESSAGES)
        if (NOT EXISTS mo_dir)
            file(MAKE_DIRECTORY ${mo_dir})
        endif()
        set(po_file ${po_dir}/${DOMAIN}.po)
        set(mo_file ${mo_dir}/${DOMAIN}.mo)
        add_custom_command(
            OUTPUT ${mo_file}
            COMMENT "Compile translation file domain ${DOMAIN} language ${lang} to binary format"
            DEPENDS ${po_file}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${mo_dir}
            COMMAND ${MSGFMT} --use-fuzzy --output-file=${mo_file} ${po_file}
        )
        add_custom_target(
            ${DOMAIN}.${lang}.l10n
            DEPENDS ${mo_file}
        )
        add_dependencies(${DOMAIN}.l10n ${DOMAIN}.${lang}.l10n)
        if (INSTALL)
            message(STATUS "Adding install target for ${mo_file}")
            install(
                FILES ${mo_file}
                DESTINATION ${install_dir}
            )
        endif()
    endforeach()
endfunction()

function(msgmerge DOMAIN POT_FILE)
    set(argnames TEST_TRANSLATION INSTALL)
    parse_argn("" ${argnames} ${ARGN})
    foreach(lang ${L10N_LANGUAGES})
        set(po_dir ${L10N_POT_DIRECTORY}/${lang})
        set(mo_dir ${L10N_MO_DIRECTORY}/${lang}/LC_MESSAGES)
        set(install_dir ${L10N_INSTALL_DIRECTORY}/${lang}/LC_MESSAGES)
        if (NOT EXISTS po_dir)
            file(MAKE_DIRECTORY ${po_dir})
        endif()
        if (NOT EXISTS mo_dir)
            file(MAKE_DIRECTORY ${mo_dir})
        endif()
        set(po_file ${po_dir}/${DOMAIN}.po)
        set(mo_file ${mo_dir}/${DOMAIN}.mo)
        add_custom_command(
            OUTPUT ${po_file}
            COMMENT "Merge translation file domain ${DOMAIN} language ${lang}"
            DEPENDS ${POT_FILE}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${po_dir}
            COMMAND ${CMAKE_COMMAND} -Dlang_file=${po_file} -DPOT_FILE=${POT_FILE}
                    -Dlang=${lang} -DMSGINIT=${MSGINIT} -DMSGMERGE=${MSGMERGE}
                    -P ${L10N_MSGMERGE_SCRIPT}
        )
        add_custom_command(
            OUTPUT ${mo_file}
            COMMENT "Compile translation file domain ${DOMAIN} language ${lang} to binary format"
            DEPENDS ${po_file}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${mo_dir}
            COMMAND ${MSGFMT} --use-fuzzy --output-file=${mo_file} ${po_file}
        )
        add_custom_target(
            ${DOMAIN}.${lang}.l10n
            DEPENDS ${po_file} ${mo_file}
        )
        add_dependencies(${DOMAIN}.l10n ${DOMAIN}.${lang}.l10n)
        if (TEST_TRANSLATION)
            message(STATUS "Add a test to check translation of ${DOMAIN} to ${lang}")
            add_test(
                NAME translate-${PROJECT_NAME}-${DOMAIN}-${lang}
                COMMAND ${MSGCMP} ${po_file} ${POT_FILE}
            )
        else()
            message(STATUS "Not testing translation of ${DOMAIN} to ${lang}")
        endif()
        if (INSTALL)
            message(STATUS "Adding install target for ${mo_file}")
            install(
                FILES ${mo_file}
                DESTINATION ${install_dir}
            )
        endif()
    endforeach()
endfunction()

function(extract_l10n)
    set(argnames
        PROGRAM OPTIONS
        TARGET DOMAIN POT_DIRECTORY SOURCES
        WORKING_DIRECTORY
        PACKAGE PACKAGE_VERSION COPYRIGHT BUGS
        INSTALL TEST_TRANSLATION
    )
    parse_argn("" argnames ${ARGN})

    if(NOT DOMAIN)
        set(DOMAIN "messages")
    endif()
    if(NOT SOURCES)
        message(FATAL_ERROR "No sources specified for POT extraction")
    endif()
    if(NOT WORKING_DIRECTORY)
        set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    set(out_file_name "${DOMAIN}.pot")
    foreach(arg_name ${argnames})
        if (NOT ${arg_name})
            if (L10N_${arg_name})
                set(${arg_name} ${L10N_${arg_name}})
            endif()
        endif()
    endforeach(arg_name ${argnames})
    set(out_file_name "${POT_DIRECTORY}/${out_file_name}")

    if (NOT L10N_INTERPRETER)
        # Non-interpreter mode, compile mo files from po files in source tree
        message(STATUS "Non-interpreter mode - compile mo files")
        set(l10n_target "${DOMAIN}.l10n")
        add_custom_target(
            ${l10n_target} ALL
            DEPENDS ${out_file_name})
        if(TARGET)
            add_dependencies(${TARGET} ${l10n_target})
        endif()
        compile_mo(${DOMAIN} ${out_file_name} INSTALL ${INSTALL})
        add_dependencies(${PACKAGE}.i18n ${l10n_target})
    else()
        # Interpreter mode, extract messages to pot file, generate or
        # merge po files for translation, compile mo files
        message(STATUS "Interpreter mode - extract strings, generate po files")
        set(XGETTEXT_OPTIONS ${OPTIONS})

        list(APPEND     XGETTEXT_OPTIONS --add-location=never)
        if (PACKAGE)
            list(APPEND XGETTEXT_OPTIONS --package-name=${PACKAGE})
        endif()
        if (PACKAGE_VERSION)
            list(APPEND XGETTEXT_OPTIONS --package-version=${PACKAGE_VERSION})
        endif()
        if (COPYRIGHT)
            list(APPEND XGETTEXT_OPTIONS --copyright-holder=${COPYRIGHT})
        else()
            list(APPEND XGETTEXT_OPTIONS --foreign-user)
        endif()
        if (BUGS)
            list(APPEND XGETTEXT_OPTIONS --msgid-bugs-address=${BUGS})
        endif()

        # TODO Generate a list of sources with paths relative to pot file
        # for correct references in the
        # TODO Check if file exists and merge existing
        add_custom_command(
            OUTPUT ${out_file_name}
            DEPENDS ${PROGRAM} ${SOURCES}
            COMMENT "Extract strings for localization in domain ${DOMAIN}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${PROGRAM} ${XGETTEXT_OPTIONS} -o ${out_file_name} ${SOURCES}
        )
        set(l10n_target "${DOMAIN}.l10n")
            add_custom_target(
                ${l10n_target} ALL
                DEPENDS ${out_file_name})
        if(TARGET)
            add_dependencies(${TARGET} ${l10n_target})
        endif()
        if (TEST_TRANSLATION)
            enable_testing()
        endif()
        msgmerge(${DOMAIN} ${out_file_name} INSTALL ${INSTALL} TEST_TRANSLATION ${TEST_TRANSLATION})
        add_dependencies(${PACKAGE}.i18n ${l10n_target})
    endif()

    if (NOT L10N_DOMAINS)
        set(L10N_DOMAINS ${DOMAIN})
    else()
        list(APPEND L10N_DOMAINS ${DOMAIN})
    endif()
    set(L10N_DOMAINS ${L10N_DOMAINS} PARENT_SCOPE)
endfunction()

function(localize)
    set(
        XGETTEXT_OPTIONS
        --from-code=UTF-8
        --sort-by-file
        --keyword=translate:1,1t
        --keyword=translate:1c,2,2t
        --keyword=translate:1,2,3t
        --keyword=translate:1c,2,3,4t
        --keyword=gettext:1
        --keyword=pgettext:1c,2
        --keyword=ngettext:1,2
        --keyword=npgettext:1c,2,3
        ${L10N_XGETTEXT_KEYWORDS}
    )
    extract_l10n(PROGRAM ${XGETTEXT} OPTIONS ${XGETTEXT_OPTIONS} ${ARGN})
    set(L10N_DOMAINS ${L10N_DOMAINS} PARENT_SCOPE)
endfunction()

function(l10n_reset_sources VAR_NAME)
    set(${VAR_NAME} CACHE INTERNAL "L10N source list ${VAR_NAME}")
endfunction()

function(l10n_collect_sources VAR_NAME)
    set(l10n_options POT_DIRECTORY)
    foreach(arg_name ${l10n_options})
        if (NOT ${arg_name})
            if (L10N_${arg_name})
                set(${arg_name} ${L10N_${arg_name}})
            endif()
        endif()
    endforeach(arg_name ${argnames})

    set(src_list ${${VAR_NAME}})
    foreach(src ${ARGN})
        set(src_path ${CMAKE_CURRENT_SOURCE_DIR}/${src})
        list(APPEND src_list ${src_path})
    endforeach()
    set(${VAR_NAME} ${src_list}
        CACHE INTERNAL "L10N source list ${VAR_NAME}")
endfunction()
