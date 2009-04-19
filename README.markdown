ABOUT
=====================================================================

mkxpud is an image generator for xPUD project (http://xpud.org).

It's a binary-level build system that reads project setting 
(named "cookbook"), parse it into package settings (called "recipe"), 
strips directly from a working APT/dpkg based Linux installation,
extracts them into rootfs and finally generates xPUD image.


REQUIREMENT
=====================================================================

To generate a xPUD image, you will need:

* a working Debian/Ubuntu GNU/Linux system
* "sudo" permission to install and test 
* Internet connection to download packages
* 500MB of hard disk free space



BUILDING
=====================================================================

* Usage:
  mkxpud <option> [<project name>] 

You can simply build the image with:
	$ ./tools/mkxpud all
This will create a project called "default", with 
the configuration file "config/default.cookbook"
	
To generate specified project with:
	$ ./tools/mkxpud all example
It will use "config/example.cookbook" as configuration file
	
To test generated image with:
	$ ./tools/mkxpud test 
This requires QEMU to run the image 

And re-generate image is simple, too:
	$ ./tools/mkxpud image

More information please read the output of "./tools/mkxpud help"

CONFIGURATION
=====================================================================

* Project configuration, "cookbook":

A project configuration for mkxpud is named with ".cookbook" 
in the end of filename extension. 
Lines beginning with `#' are for comments, and five sections,
which are quoted by `[]', take every raws as its input data:
	
	[config]
	# short project description
	MKXPUD_NAME=""
	# you can skip the apt-get install by setting this to "true"
	MKXPUD_SKIP_APT=""
	# specified the Linux kernel version to be used
	KERNEL=""

	[kernel]
	# kernel modules to be included into rootfs
	# not yet implemented

	[recipe]
	# packages and files to be included into rootfs

	[overwrite]
	# files to be overwritten
	# not yet implemented

	[obfuscate]
	# files to be compressed with `upx' or `shc'
	# not yet implemented
	
	[remove]
	# files to be removed after rootfs extraction
	# not yet implemented

More information please read the example file "config/default.cookbook"

* Package configuration, "recipe":

mkxpud reads several "recipe" files to generate an image:

	[name]
	# name of this recipe
	
	[package]
	# packages to be installed via apt-get 

	[action]
	# actions to be executed before install this recipe
	# for example, you can download or checkout codes here

	[binary]
	# binaries to be copied from your host to target
	# these data will be handle by ldd helper
	# and the *.so.* files are copied automatically

	[data]
	# other necessary data to be used with package
	# for example /usr/share/* and /usr/lib/*
	# these will be copied from your host to target directory

	[config]
	# configuration files to be overwritten 
	# these will be copied from the "package/config" directory

	[overwrite]
	# system-wide files to be overwritten
	# these will be copied from the "skeleton/overwrite" directory
	
Please read files under "package/recipe/" as examples.

STRUCTURE
=====================================================================

This is the internal structure of mkxpud:

.
|-- README				the file you're reading
|-- config
|   |-- config-2.6.28			kernel .config file
|   `-- default.cookbook		default config file for project
|-- deploy
|   `-- bzImage				pre-built kernel image
|-- kernel
|   |-- module
|   |   `-- 2.6.28-modules.tgz		pre-built kernel modules
|   `-- src
|-- package
|   |-- config				config files to be copied to target
|   `-- recipe				config files for package
|-- skeleton	
|   |-- archive
|   |   `-- dev.tgz			pre-generated device nodes
|   |-- boot				boot loader for images
|   |-- overwrite			data to be copied to target
|   `-- rootfs				skeleton rootfs 
|-- tools
|   `-- mkxpud				the main script 
`-- working
    `-- default				working and temporary files 

LICENSE
=====================================================================

Copyright (c) 2009, Ping-Hsun Chen.

Except the file that comes with its own license, the rest of mkxpud is free.
You are free to distribute and/or modify this software under the terms of
the GNU General Public License.

On Debian systems, the complete text of the GNU General Public License can
be found in /usr/share/common-licenses/GPL file.

AUTHOR
=====================================================================

Ping-Hsun Chen (penk) <penkia@gmail.com>

This project is under heavy development, and we're looking forward 
to any kind of feedback. If you have suggestion or find something wrong, 
please contact me directly. :-)

Last updated: 2009-04-01
