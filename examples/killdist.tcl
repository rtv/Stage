#!/usr/bin/tclsh

package require Tclx

set USAGE "kill.tcl <username> <worldfile>"

if {$argc < 2} {
  puts $USAGE
  exit
}

set username [lindex $argv 0]
set worldfile [lindex $argv 1]
set fd [open $worldfile r]

set hosts {}
while {![eof $fd]} {
  set line [gets $fd]
  if {[string first \{ $line] >= 0} {continue}


  if {[llength $line] == 4 &&
      ![string compare [lindex $line 0] "set"] &&
      ![string compare [lindex $line 1] "host"] &&
      [string compare  [lindex $line 3] "none"]} {
    lappend hosts [lindex $line 3]
  }
}

puts "Found the following hosts: $hosts\n"

foreach host $hosts {
  catch {exec ssh -1 -l $username $host "killall player xs stage"}
}
