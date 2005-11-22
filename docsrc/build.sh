#!/bin/bash

OUTPUT=header.html
PAGEDIR=stage

cat header_top.src > $OUTPUT

# start the list of derived models
echo "<ul class=menu1>" >> $OUTPUT

for PAGE in `ls $PAGEDIR/group__model__*.html` ; do

   PAGENAME=${PAGE#$PAGEDIR/}
   STRIPEND=${PAGE%.html}
   STRIPPED=${STRIPEND#$PAGEDIR/group__model__}
   
   echo "Adding link for " $STRIPPED
   echo "<li class=menu><a href=\""$PAGENAME"\">"$STRIPPED"</a></li>" >> $OUTPUT
done

# end the list of models
echo "</ul>" >> $OUTPUT

## start the list of player plugins
#echo "<hr><li class=menu>Player interfaces<ul>" >> $OUTPUT

#for PAGE in `ls $PAGEDIR/group__player*.html` ; do#
#
#   PAGENAME=${PAGE#$PAGEDIR/}
#   STRIPEND=${PAGE%.html}
#   STRIPPED=${STRIPEND#$PAGEDIR/group__player__}
#   
#   echo "Adding link for " $STRIPPED
#   echo "<li class=menu><a href=\""$PAGENAME"\">"$STRIPPED"</a></li>" >> $OUTPUT
#done
## end the list of plugins
#echo "</ul>" >> $OUTPUT
#echo "<hr><li class=menu><a href=group__driver__stage.html>Plugin</a></li>" >> $OUTPUT

cat header_bottom.src >> $OUTPUT

# <li><a href=" group__model__basic.html "> basic</a></li>
