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

set numrobots(fnord) 150
set numrobots(pantera) 150
set numrobots(phat) 75
set numrobots(servalan) 75
set numrobots(sal218) 50

set baseport(fnord) 40000
set baseport(pantera) 40150
set baseport(phat) 40300
set baseport(servalan) 40375
set baseport(sal218) 40450
  
proc killplayers {} {
  global hosts username stagepids dirname
  puts "Caught SIGINT: killing remote stages"
  foreach host $hosts {
    if {[catch {exec ssh -1 -l $username $host "kill -INT `cat ${dirname}/stage.pid`"} err]} {
      puts "Got error for $host: $err"
    }
  }
  catch {exec killall manager}
  sleep 5
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
  set multirandombinary "${playerstageroot}/player/examples/c++/multirandom"
  
  exec cp $playerbinary .
  exec cp $stagebinary .
  exec cp $xsbinary .
  exec cp $multirandombinary .
  #exec cp $managerbinary .
  exec strip player
  exec strip stage
  exec strip xs
  exec strip multirandom

  puts "Sending binaries to:"

  foreach host $hosts {
    puts "* $host"
    exec ssh -1 -l $username $host "mkdir -p $dirname"
    exec scp -oProtocol=1 multirandom player stage xs ${username}@${host}:$dirname >@ stdout
  }

  puts "Sending $worldfile and $bitmap to:"
  foreach host $hosts {
    puts "* $host"
    exec scp -oProtocol=1 $worldfile $bitmap ${username}@${host}:$dirname >@ stdout
  }
  exec rm player
  exec rm stage
  exec rm xs
  exec rm multirandom
}

puts "\nStarting stage on:"
foreach host $hosts {
  puts "* $host"
  exec ssh -n -1 -l $username $host "cd $dirname; export PATH=$dirname; ./stage -xs -l log $worldfile" > /dev/null &
  #sleep 2
}

puts "Waiting for Stages to start..."
sleep 20
puts "\nStarting clients on:"
foreach host $hosts {
  puts "* $host"
  exec ssh -n -1 -l $username $host "cd $dirname; export PATH=$dirname; ./multirandom localhost $baseport($host) $numrobots($host) >! occ_grid.0 2>! poslog.0" > /dev/null &
  #sleep 2
}

puts "\nStarting Stage manager"
eval exec $managerbinary $hosts >@ stdout &

signal unblock SIGINT
signal trap SIGINT killplayers

while {1} {
  sleep 5
}

