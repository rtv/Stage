
Stage build using CMake instructions
author: Richard Vaughan, 2008.4.11 $Id$

* Purpose *

As an alternative to the GNU autotools build chain, Stage can also be
buily with CMake. This has two main advantages: (i) it is much faster;
(ii) CMake can create native build files for Windows and Mac OS X,
which will help Stage become more portable.


* Configuring the build *

Unpack the distribution or check it out from SVN. Change directory to
the top level of the Stage source tree.

First, decide where you want to install Stage. The default
installation directory varies by system, but is often /usr/local on
Unix variants. This is easy and is often a good choice, but has the
disadvantage that installation needs root/sudo priviliges. To install
in the default location, do:

  $ cmake .

If you wish to install Stage elsewhere, define the CMAKE_INSTALL_PATH
path variable when invoking cmake. To do this, use this command,
substituting <prefix> with your chosen installation
directory.
	
  $ cmake -DCMAKE_INSTALL_PREFIX=PATH:<prefix>

For example to install in $HOME/stage, do:

  $ cmake -DCMAKE_INSTALL_PREFIX=PATH:$HOME/stage

CMake will generate makefiles specifically for your machine. When this
is done, you can inspect and edit the build settings using ccmake, or
by editing the file CMakeCache.txt. 

* Building *

In the top level directory of the source tree, do:

 $ make

* Installing *

In the top level directory of the source tree, do:

 $ make install

(You may need to run this command as root or sudo, depending on the
install location).

Stage will install its components in various directories, for example:

<prefix>/bin    (executables, including the 'stage' program)
<prefix>/lib    (libraries, including libstage)
<prefix>/share  (contains data resources, such as images)

* Setup *

You must ensure that the dynamic library libstage.so (or
libstage.dylib, or libstage.dll, depending on your platform) can be
found by your system's library loader. The method for doing this
varies by platform.

On Linux, using BASH:

  $ export LD_LIBRARY_PATH=<prefix>/lib

On OS X, using BASH:

  $ export DYLD_LIBRARY_PATH=<prefix>/lib

If you plan to use Stage plugins, you also need to set the STAGEPATH
environment variable to include the directory that contains your
plugins. E.g. in BASH, do:

  $ export STAGEPATH=/usr/local/lib

If you installed Stage somewhere other than /usr/local, substitute
your install prefix:

  $ export STAGEPATH=<stage install prefix>/lib

If you are using Stage with Player, you probably don't need to set the
STAGEPATH. However, you may need to set the PLAYERPATH to include
Stage's installed lib directory instead.

* Testing *

To test your Stage installation, do:

<prefix>/bin/stage worlds/simple.world

You should see a window appear, showing some robots. You can change
the camera point of view by holding down the 'ctrl' key and moving the
mouse pointer. If this works, you are ready to write your own robot
simulations using libstage.

If you plan to use Stage plugins, you can test that plugins are
working:

<prefix>/bin/stage worlds/fasr.world

You should see a window appear, showing some robots. Try pressing the
'p' key to pause and un-pause the simulation, to check that the robots
are working. If this works, you are ready to write Stage plugins.

* Next steps *

- read the Stage manual, available from the Player Project website
  (also buildable from the docsrc directory in the source tree
  (requires doxygen)).

- look at the examples provided in the worlds and examples
  directories.


Enjoy using Stage -- rtv

"All the world's a stage, and all the women merely players"
  Wm. Shakespeare - "As you like it"

