#!/bin/bash

OUTPUT=header.html
PAGEDIR=stage_user

cat header_top.src > $OUTPUT

for PAGE in `ls $PAGEDIR/group__model*.html` ; do

   PAGENAME=${PAGE#$PAGEDIR/}
   STRIPEND=${PAGE%.html}
   STRIPPED=${STRIPEND#$PAGEDIR/group__model__}
   
   echo "Adding link for " $STRIPPED
   echo "<li><a href=\""$PAGENAME"\">"$STRIPPED"</a></li>" >> $OUTPUT
done

cat header_bottom.src >> $OUTPUT



# <li><a href=" group__model__basic.html "> basic</a></li>
