
Copyright Richard Vaughan and contributors 1998-2009.  
Part of the Player Project (http://playerstage.org)

License
-------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.
 
This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

A copy of the license is included with the sourcecode in the file
'COPYING". Copying and redistribution is permitted only under the
terms of the license.


Introduction
------------
Stage is a robot simulator. It provides a virtual world populated by
mobile robots and sensors, along with various objects for the robots
to sense and manipulate.

There are three ways to use Stage: 

1. The "stage" program: a standalone robot simulation program
that loads your robot control program from a library that you provide.

2. The Stage plugin for Player (libstageplugin) - provides a
population of virtual robots for the popular Player networked robot
interface system.

3. Write your own simulator: the "libstage" C++ library makes it
easy to create, run and customize a Stage simulation from inside your
own programs.


Models
------
Stage provides several sensor and actuator models, including sonar
or infrared rangers, scanning laser rangefinder, color-blob tracking,
fiducial tracking, bumpers, grippers and mobile robot bases with
odometric or global localization.


Design
------
Stage was designed with multi-agent systems in mind, so it provides
fairly simple, computationally cheap models of lots of devices rather
than attempting to emulate any device with great fidelity. This design
is intended to be useful compromise between conventional high-fidelity
robot simulations, the minimal simulations described by Jakobi [4], and
the grid-world simulations common in artificial life research [5]. We
intend Stage to be just realistic enough to enable users to move
controllers between Stage robots and real robots, while still being
fast enough to simulate large populations. We also intend Stage to be
comprehensible to undergraduate students, yet sophisticated enough for
professional reseachers.
  
Player also contains several useful 'virtual devices'; including
some sensor pre-processing or sensor-integration algorithms that help
you to rapidly build powerful robot controllers. These are easy to use
with Stage.


Citations
---------
If you use Stage in your work, we'd appreciate a citation. At the time 
of writing, the most suitable reference is either:

Richard Vaughan. "Massively Multiple Robot Simulations in Stage", Swarm
Intelligence 2(2-4):189-208, 2008. Springer.

Or, if you are using Player/Stage:

Brian Gerkey, Richard T. Vaughan and Andrew Howard. "The Player/Stage 
Project: Tools for Multi-Robot and Distributed Sensor Systems" 
Proceedings of the 11th International Conference on Advanced Robotics,
pages 317-323, Coimbra, Portugal, June 2003 (ICAR'03)  
http://www.isr.uc.pt/icar03/ .  

[gzipped postscript](http://robotics.stanford.edu/~gerkey/research/final_papers/icar03-player.ps.gz), 
[pdf](http://robotics.stanford.edu/~gerkey/research/final_papers/icar03-player.pdf)

Please help us keep track of what's being used out there by correctly
naming the Player/Stage components you use. Player used on its own is
called "Player". Player and Stage used together are referred to as
"the Player/Stage system" or just "Player/Stage". When Stage is used
without Player, it's just called "Stage". When the Stage library is
used to create your own custom simulator, it's called "libstage" or
"the Stage library". When Player is used with its 3D ODE-based
simulation backend, Gazebo, it's called Player/Gazebo. Gazebo without
Player is just "Gazebo". All this software is part of the "Player
Project".

Support
-------
Funding for Stage has been provided in part by:

- DARPA (USA)
- NASA (USA)
- NSERC (Canada)
- DRDC Suffield (Canada)
- NSF (USA)
- Simon Fraser University (Canada)

Names
-----
The names "Player" and "Stage" were inspired by the lines:

    "All the world's a stage,  
     And all the men and women merely players"  

from "As You Like It" by William Shakespeare.


References
----------
[4] Nick Jakobi (1997) "Evolutionary Robotics and the Radical Envelope
of Noise Hypothesis", Adaptive Behavior Volume 6, Issue 2. pp.325 -
368.
 
[5] Stuart Wilson (1985) "Knowledge Growth in an Artificial Animal",
Proceedings of the First International Conference on Genetic Agorithms
and Their Applications.  Hillsdale, New Jersey. pp.16-23.
