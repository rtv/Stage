# $Id: utils.m4,v 1.2 2002-01-29 00:43:25 gerkey Exp $
# some useful m4 utilities 

# this is a simple for loop. use it like:
#   forloop(`i',1,10,`i ')
define(`forloop',
        `pushdef(`$1', `$2')_forloop(`$1', `$2', `$3', `$4')popdef(`$1')')
define(`_forloop',
        `$4`'ifelse($1, `$3', , 
        `define(`$1', incr($1))_forloop(`$1', `$2', `$3', `$4')')')

