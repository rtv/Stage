FILE(GLOB worlds "*.world")
FILE(GLOB incs "*.inc")
INSTALL(FILES ${worlds} ${incs}
        DESTINATION share/stage/worlds/wifi
)

