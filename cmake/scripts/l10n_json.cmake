if(NOT XJSONGETTEXT)
    find_file(
        XJSONGETTEXT xjsongettext
        NO_DEFAULT_PATH
        HINTS
            ${CMAKE_SOURCE_DIR}/scripts
            ${CMAKE_SOURCE_DIR}/lib/l10n/scripts
        DOC "Script for extracting l10n messages from JSON files"
    )
    if (NOT XJSONGETTEXT)
        message(FATAL_ERROR "Failed to find script for extracting messages from json files")
    endif()
endif()

function(localize_json)
    if (XJSONGETTEXT)
        xgettext(PROGRAM ${XJSONGETTEXT} ${ARGN})
        set(L10N_DOMAINS ${L10N_DOMAINS} PARENT_SCOPE)    
    endif()
endfunction()
