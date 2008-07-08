
FILE( GLOB pngs "*.png" )

INSTALL(FILES ${pngs} rgb.txt 
        DESTINATION share/stage/assets
)

