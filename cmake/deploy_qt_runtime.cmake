# Qt tools share an output directory. Concurrent windeployqt processes can both
# see a plugin as missing and then fail when copying it over the other's file.
# Serialize deployment per destination without serializing compilation/linking.
foreach(required QT_DEPLOY_EXECUTABLE QT_DEPLOY_MODE QT_DEPLOY_DIR QT_DEPLOY_BINARY)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing ${required} for Qt runtime deployment")
    endif()
endforeach()

file(LOCK "${QT_DEPLOY_DIR}/.w3d-qt-deploy.lock"
    GUARD PROCESS TIMEOUT 120 RESULT_VARIABLE lock_result)
if(NOT "${lock_result}" STREQUAL "0")
    message(FATAL_ERROR "Could not lock Qt deployment directory: ${lock_result}")
endif()

execute_process(
    COMMAND "${QT_DEPLOY_EXECUTABLE}" "--${QT_DEPLOY_MODE}" --no-translations
        --dir "${QT_DEPLOY_DIR}" "${QT_DEPLOY_BINARY}"
    RESULT_VARIABLE deploy_result)
if(NOT "${deploy_result}" STREQUAL "0")
    message(FATAL_ERROR "Qt runtime deployment failed for ${QT_DEPLOY_BINARY}: ${deploy_result}")
endif()
