LINK_DIRECTORIES(${PROJECT_BINARY_DIR})
SET(SRC_LIST ${MODULE_SRC} NutrientTransportSediment.cpp api.cpp)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
ADD_LIBRARY(NUTRSED SHARED ${SRC_LIST})
IF (MSVC)
    SET_PROPERTY(TARGET NUTRSED PROPERTY FOLDER "modules/nutrient")
ENDIF ()
IF (WIN32)
    TARGET_LINK_LIBRARIES(NUTRSED module_setting bmps data util)
ELSE ()
    TARGET_LINK_LIBRARIES(NUTRSED libmodule_setting.a bmps.a libdata.a libutil.a)
ENDIF ()