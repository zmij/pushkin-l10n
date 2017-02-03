if (EXISTS ${lang_file})
    execute_process(
        COMMAND ${MSGMERGE} --no-fuzzy-matching --no-wrap --update ${lang_file} ${POT_FILE}
        OUTPUT_QUIET ERROR_QUIET
    )
else()
    execute_process(
        COMMAND ${MSGINIT} --locale=${lang} --no-wrap --no-translator --input=${POT_FILE} --output=${lang_file}
        OUTPUT_QUIET ERROR_QUIET
    )
endif()
