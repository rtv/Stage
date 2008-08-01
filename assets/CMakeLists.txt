
FILE( GLOB pngs "*.png" )

INSTALL(FILES ${pngs} rgb.txt 
        DESTINATION share/stage/assets
)

INSTALL(FILES rgb.txt 
        DESTINATION share/stage
)

