#!/usr/bin/tclsh

package require Tclx

set USAGE "start.tcl \[-n\] <username> <worldfile>"

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
  global hosts username stagepids dirname
  puts "Caught SIGINT: killing remote stages"
  foreach host $hosts {
    if {[catch {exec ssh -1 -l $username $host "kill -INT `cat ${dirname}/stage.pid`"} err]} {
      puts "Got error for $host: $err"
    }
  }
  sleep 3
  puts "Exiting\n\n"
  exit
}
signal block SIGINT


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

set playerstageroot "$env(HOME)/PS"
set managerbinary "${playerstageroot}/stage/src/manager"
if {$send} {
  set playerbinary "${playerstageroot}/player/src/player"
  set stagebinary "${playerstageroot}/stage/src/stage_objs/stage"
  set xsbinary "${playerstageroot}/stage/src/xs"
  
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
    exec ssh -n -1 -l $username $host "cd $dirname; export PATH=$dirname; ./stage -l log $worldfile" &
  } else {
    exec ssh -n -1 -l $username $host "cd $dirname; export PATH=$dirname; ./stage -xs -l log $worldfile" &
  }
  #sleep 2
}

puts "Waiting for Stages to start..."
sleep 5
puts "\nStarting Stage manager"
eval exec $managerbinary $hosts >@ stdout &

signal unblock SIGINT
signal trap SIGINT killplayers

while {1} {
  sleep 5
}

