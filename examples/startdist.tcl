#!/usr/bin/tclsh

package require Tclx

set USAGE "start.tcl \[-n\] <username> <worldfile> <bitmap>"

set idx 0
set send 1
if {![string compare [lindex $argv $idx] "-n"]} {
  set send 0
  incr idx
}
  
if {$argc < 2} {
  puts $USAGE
  exit
}

set username [lindex $argv $idx]
incr idx
set worldfile [lindex $argv $idx]

  
proc killplayers {} {
  global hosts username
  puts "Caught SIGINT: killing remote players"
  foreach host $hosts {
    catch {exec ssh -1 -l $username $host "killall player xs stage"}
  }
  exit
}
signal trap SIGINT killplayers

set dirname "/tmp/distributed_stage.$username"

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
  } elseif {[llength $line] == 4 &&
      ![string compare [lindex $line 0] "set"] &&
      ![string compare [lindex $line 1] "environment_file"]} {
    set bitmap "[lindex $line 3].gz"
  }
}

puts "Found the following hosts: $hosts\n"
puts "Using the bitmap: $bitmap\n"

if {$send} {
  set playerbinary "/home/vaughan/PS/player/src/player"
  set stagebinary "/home/vaughan/PS/stage/src/stage_objs/stage"
  set xsbinary "/home/vaughan/PS/stage/src/xs"
  set managerbinary "/home/vaughan/PS/stage/src/manager"
  
  exec cp $playerbinary .
  exec cp $stagebinary .
  exec cp $xsbinary .
  #exec cp $managerbinary .
  exec strip player
  exec strip stage
  exec strip xs

  puts "Sending binaries to:"

  foreach host $hosts {
    puts "* $host"
    exec ssh -1 -l $username $host "mkdir -p $dirname"
    exec scp -oProtocol=1 player stage xs ${username}@${host}:$dirname
  }

  puts "Sending $worldfile and $bitmap to:"
  foreach host $hosts {
    puts "* $host"
    exec scp -oProtocol=1 $worldfile $bitmap ${username}@${host}:$dirname
  }
  exec rm player
  exec rm stage
  exec rm xs
}

puts "\nStarting stage on:"
foreach host $hosts {
  puts "* $host"
  if {![string compare $host [lindex $hosts 0]]} {
    exec xterm -T "stage@$host" -e ssh -1 -l $username $host "cd $dirname; export PATH=$dirname; ./stage $worldfile" &
  } else {
    exec xterm -T "stage@$host" -e ssh -1 -l $username $host "cd $dirname; export PATH=$dirname; ./stage -xs $worldfile" &
  }
  #sleep 2
}

puts "Waiting for Stages to start..."
sleep 3
puts "\nStarting Stage manager"
eval exec $managerbinary $hosts >@ stdout &

while {1} {
  sleep 5
}

