ADD_SUBDIRECTORY(benchmark)

FILE( GLOB cfgs "*.cfg" )
FILE( GLOB worlds "*.world" )
FILE( GLOB incs "*.inc" )
FILE( GLOB scripts "*.sh" )

INSTALL(FILES ${cfgs} ${worlds} ${incs} ${scripts}
DESTINATION share/stage/worlds)

#INSTALL (DIRECTORY bitmaps wifi DESTINATION share/stage/worlds)

ADD_SUBDIRECTORY(bitmaps)
ADD_SUBDIRECTORY(wifi)

#FILE(GLOB worlds "*.world")
#FILE(GLOB incs "*.inc")
#INSTALL(FILES ${worlds} ${incs}
#        DESTINATION share/stage/worlds
#)

