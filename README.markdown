ABOUT
=====

**mkxpud** is an image generator for xPUD project (<http://www.xpud.org>).

It is a binary-level build system that reads project config 
(named **cookbook**), parses it with package settings (called **recipe**), 
strips directly from a working APT/dpkg-based Linux installation, 
extracts them into rootfs and finally generates xPUD image.


REQUIREMENT
===========

To generate a xPUD image, you will need:

1. a working Ubuntu 9.04 installation 
2. **sudo** permission to install packages on the host and testing
3. Internet access to download packages (if required)
4. at least 500MB of hard disk free space

We designed **mkxpud** to be distribution-independent, but at current 
development stage, only limited distribuations and released version
have been fully tested. If you find something wrong or want to improve
the function on different distribution, please 
post on our [forum](http://groups.google.com/group/pud-linux) or 
[bug tracker](http://code.google.com/p/xpud/issues/list).

BUILDING
========

Quick Start:
------------

1. Check out the latest source code:

        git clone git://github.com/penk/mkxpud.git

2. Build the rootfs and generate the image:

        ./tools/mkxpud all

3. Test your result:

        ./tools/mkxpud test

This will create an iso9669 image at `deploy/default.iso`.

Usage:
------

* Basic usage of **mkxpud**: 

        mkxpud <option> [<project name>] 

If you doesn't specify the project name, it will create a project called **default**, 
by using the configuration file `config/default.cookbook`.
	
* To generate specified project:

        ./tools/mkxpud all example
	
It will use `config/example.cookbook` as configuration instead.
	
* To test generated image:

        ./tools/mkxpud test 

This requires QEMU installed on host and at least 384MB of ram to run virtual machine.

* To re-generate image:

        ./tools/mkxpud image

More information please read the output of `./tools/mkxpud help`

CONFIGURATION
=============

* Project configuration, **cookbook**:

A project configuration of mkxpud is named with `.cookbook` as filename extension. 

Lines beginning with `#` are for comments, and there are sections 
which quoted by `[ ]`, rest of the rows are data:

	[config]
	# short project description
	MKXPUD_NAME=""
	# you can skip the apt-get install by setting this to "true"
	MKXPUD_SKIP_APT=""
	# set this to "true" if you want to use host /dev nodes
	MKXPUD_HOST_DEV=""
	
	# specified the Linux kernel version to be used
	MKXPUD_KERNEL="2.6.28"
	MKXPUD_KERNEL_IMAGE="deploy/vmlinuz-2.6.28"
	MKXPUD_MOD_PATH="/lib/modules/2.6.28"
	
	[module]
	# kernel modules to be included into rootfs
	# the dependencies will be handled recursively
	
	[recipe]
	# packages and files to be included into rootfs

	[action]
	# the command that will be executed after rootfs created
	
	[overwrite]
	# files to be overwritten at system-wide 
	# not yet implemented

	[obfuscate]
	# files to be compressed if `upx' is installed on the host
	
	[remove]
	# files to be removed after rootfs extraction
	# not yet implemented
	
	[image]
	# image format that will be generated. CPIO is default
	gz iso zip

More information please read the example file `config/default.cookbook`

* Package configuration, **recipe**:

**mkxpud** takes several **recipe** files to **cook** an image:

	[name]
	# name of this recipe
	
	[package]
	# packages to be installed via apt-get or some other package manager

	[action]
	# actions to be executed before install this recipe
	# for example, you can download or checkout codes here

	[binary]
	# binaries to be copied from your host to target
	# these data will be handle by ldd-helper 
	# and the *.so.* files are copied automatically 

	[data]
	# other necessary data to be used with applications
	# for example /usr/share/* and /usr/lib/*
	# these will be copied from your host to target directory

	[config]
	# configuration files to be overwritten 
	# these will be copied from the "package/config" directory

	[overwrite]
	# system-wide files to be overwritten
	# these will be copied from the "skeleton/overwrite" directory
	
Please read files under `package/recipe/` as examples.

STRUCTURE
=========

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
    |   |   `-- default-module-2.6.28.tgz 		pre-built kernel modules
    |   `-- src
    |-- package
    |   |-- config				config files to be copied to target
    |   `-- recipe				config files for package
    |-- skeleton	
	|   |-- archive
    |   |   `-- dev.tgz			pre-generated device nodes
    |   |-- boot				boot loader for images
    |   |-- overwrite			data to be copied to target 
	|   `-- rootfs.tgz				skeleton rootfs
    |-- tools
    |   `-- mkxpud				the main script 
    `-- working
        `-- default				working and temporary files 

REFERENCE
=========

* Project website: <http://www.xpud.org>
* Repository host: <http://github.com/penk/mkxpud>
* Developer forum: <http://groups.google.com/group/pud-linux>
* Bug tracker: <http://code.google.com/p/xpud/issues/list>

LICENSE
=======

Copyright (c) 2009, Ping-Hsun Chen.

Except the file that comes with its own license, the rest of **mkxpud** is free.
You are free to distribute and/or modify this software under the terms of
the GNU General Public License.

On Debian systems, the complete text of the GNU General Public License can
be found in `/usr/share/common-licenses/GPL` file.

AUTHOR
======

Ping-Hsun Chen (penk) <penkia@gmail.com>

This project is under heavy development, and we're looking forward 
to any kind of feedback. If you have suggestion or find something wrong, 
please contact me directly. :-)

* <penkia@gmail.com>
* [@penk](http://twitter.com/penk) on Twitter
* <http://penkia.blogspot.com>
