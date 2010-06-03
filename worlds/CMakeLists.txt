ADD_SUBDIRECTORY(benchmark)
ADD_SUBDIRECTORY(bitmaps)

FILE(GLOB worlds "*.world")
FILE(GLOB incs "*.inc")
INSTALL(FILES ${worlds} ${incs}
        DESTINATION share/stage/worlds
)

