#!/bin/bash

OUTPUT=header.html
PAGEDIR=user/html

cat header_top.src > $OUTPUT

for PAGE in `ls $PAGEDIR/group__model*.html` ; do

   PAGENAME=${PAGE#$PAGEDIR/}
   STRIPEND=${PAGE%.html}
   STRIPPED=${STRIPEND#$PAGEDIR/group__model__}
   
   echo "Adding link for " $STRIPPED
   echo "<a href=\"" $PAGENAME "\">" $STRIPPED "</a><br>" >> $OUTPUT
done

cat header_bottom.src >> $OUTPUT