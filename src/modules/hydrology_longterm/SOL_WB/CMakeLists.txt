LINK_DIRECTORIES(${PROJECT_BINARY_DIR})
SET(SRC_LIST ${MODULE_SRC} SOL_WB.cpp api.cpp)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
ADD_LIBRARY(SOL_WB SHARED ${SRC_LIST})
IF (MSVC)
    SET_PROPERTY(TARGET SOL_WB PROPERTY FOLDER "modules/hydrology_longterm")
ENDIF ()
IF (WIN32)
    TARGET_LINK_LIBRARIES(SOL_WB module_setting bmps data util)
ELSE ()
    TARGET_LINK_LIBRARIES(SOL_WB libmodule_setting.a bmps.a libdata.a libutil.a)
ENDIF ()