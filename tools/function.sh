function usage {

	echo "Usage: 
 mkxpud <option> [<project codename>] - Image Generator for xPUD

Options:
	all		Execute the whole process of creating a project image
	clean		Removes most generated files
	image		Genreate image from the working directory
	test		Invoke 'QEMU' to test image with bundled kernel
	mkopt		Create opt package using recipe or directory
	help		Display this help

For further informations please refer to README file."
}

function clean {
	sudo rm -rf working/$MKXPUD_CODENAME deploy/$MKXPUD_CODENAME
}

function setup {

	echo "[mkxpud] Setup Project: $1"
	MKXPUD_CODENAME=$1
	mkdir -p working/$MKXPUD_CODENAME
	mkdir -p deploy/$MKXPUD_CODENAME
	export MKXPUD_CONFIG=config/$MKXPUD_CODENAME.cookbook
	eval export `./tools/parser $MKXPUD_CONFIG config`

	# copy initramfs skeleton
	cp -rfp --remove-destination skeleton/initramfs/ working/$MKXPUD_CODENAME/initramfs

	# untar rootfs skeleton
	tar zxf skeleton/rootfs.tgz -C working/$MKXPUD_CODENAME/
	export MKXPUD_TARGET=working/$MKXPUD_CODENAME/rootfs

	# copy /dev nodes
	if [ "$MKXPUD_HOST_DEV" == 'true' ]; then
		sudo tar zcf skeleton/archive/dev-host.tgz /dev/*
		sudo tar zxf skeleton/archive/dev-host.tgz -C $MKXPUD_TARGET/
	else 
		sudo tar zxf skeleton/archive/dev.tgz -C $MKXPUD_TARGET/
	fi
	
	# untar default kernel modules if exists
	if [ -e kernel/module/default-module-$MKXPUD_KERNEL.tgz ]; then
		sudo tar zxf kernel/module/default-module-$MKXPUD_KERNEL.tgz -C $MKXPUD_TARGET/
	fi

	echo "[mkxpud] Project Target: $MKXPUD_TARGET"
	
	echo "[mkxpud] Executing pre-build scripts:"
	eval `./tools/parser $MKXPUD_CONFIG prepare`
}

function install {

	echo "    Preparing recipes..."
	for R in `./tools/parser $MKXPUD_CONFIG recipe`; do 
		for P in `./tools/parser package/recipe/$R.recipe package`; do
		PACKAGE="$PACKAGE $P"
		done
	done
	# add OPT packages
	for R in `./tools/parser $MKXPUD_CONFIG opt`; do 
		for P in `./tools/parser package/recipe/$R.recipe package`; do
		PACKAGE="$PACKAGE $P"
		done
	done

	if [ "$MKXPUD_PKGMGR" != 'skip' ]; then
		sudo $MKXPUD_PKGMGR $PACKAGE
	else 
		echo "You need to install following packages according to your cookbook: "
		echo "$PACKAGE"
	fi

}
# copy files from recipe section to working fs
# copyfiles (section_name) (destination) (file)
function copyfiles {
	case $1 in
		binary)
			## host binaries
			[ -d $2/`dirname $3` ] || mkdir -p $2/`dirname $3`
			cp -rfpl --remove-destination $3 $2/$3
		;;
		data)
			## host data files
			[ -d $2/`dirname $3` ] || mkdir -p $2/`dirname $3`
			cp -rfpl --remove-destination $3 $2/$3
		;;
		config)
			## package/config/*
			[ -d $2/`dirname $3` ] || mkdir -p $2/`dirname $3`
			cp -rfp --remove-destination package/config/$3 $2/$3
		;;
		alternative)
			## package/alternative/$MKXPUD_CODENAME
			if [ -e package/alternative/$MKXPUD_CODENAME ]; then
				[ -d $2/`dirname $3` ] || mkdir -p $2/`dirname $3`
				cp -rfpL --remove-destination package/alternative/$MKXPUD_CODENAME/$3 $2/$3
			fi
		;;
		overwrite)
			## skeleton/overwrite/
			[ -d $2/`dirname $3` ] || mkdir -p $2/`dirname $3`
			cp -rfp skeleton/overwrite/$3 $2/$3
		;;
	esac
}

function strip {

	echo "    Copying files to rootfs..."
	for R in `./tools/parser $MKXPUD_CONFIG recipe`; do 
		
		# action 
		eval `./tools/parser package/recipe/$R.recipe action`

		for S in binary data config alternative overwrite; do
		
			for A in `./tools/parser package/recipe/$R.recipe $S`; do
				copyfiles $S $MKXPUD_TARGET $A
			done 
		done 

		# post action 
		eval `./tools/parser package/recipe/$R.recipe post_action`
		
	done

}

function init {

	echo "    Creating initramfs..."
	
	R="initramfs"
	COPY_DESTINATION="working/$MKXPUD_CODENAME/initramfs"
	# action 
	eval `./tools/parser package/recipe/$R.recipe action`

	for S in binary data config alternative overwrite; do
		
		for A in `./tools/parser package/recipe/$R.recipe $S`; do
				copyfiles $S $COPY_DESTINATION $A
			done
			
	done 

	# post action 
	eval `./tools/parser package/recipe/$R.recipe post_action`
}

function opt {

	echo "    Creating Opt files..."
	for R in `./tools/parser $MKXPUD_CONFIG opt`; do 
		
		# action 
		eval `./tools/parser package/recipe/$R.recipe action`
		
		# create opt directory
		NAME=`./tools/parser package/recipe/$R.recipe name`
		mkdir -p $MKXPUD_TARGET/opt/$NAME

		for S in binary data config overwrite; do
		
			for A in `./tools/parser package/recipe/$R.recipe $S`; do
				copyfiles $S $MKXPUD_TARGET/opt/$NAME $A
			done 
		done 

		# post action 
		eval `./tools/parser package/recipe/$R.recipe post_action`
		
	done

}

function kernel {

	echo "[mkxpud] Adding kernel modules"
	
	MKXPUD_CODENAME=$1
	export MKXPUD_CONFIG=config/$MKXPUD_CODENAME.cookbook
	eval export `./tools/parser $MKXPUD_CONFIG config`
	export MKXPUD_TARGET=working/$MKXPUD_CODENAME/rootfs
	
	for MOD in `./tools/parser $MKXPUD_CONFIG module`; do
		for M in `./tools/module-helper $MOD`; do
		
		## FIXME: workaround with different kernel path
		if [ `echo $M | grep "^/"` ]; then
			[ -d $MKXPUD_TARGET/`dirname $M` ] || mkdir -p $MKXPUD_TARGET/`dirname $M` 
			cp -rfpL --remove-destination $M $MKXPUD_TARGET/$M
		else 
			[ -d $MKXPUD_TARGET/$MKXPUD_MOD_PATH/`dirname $M` ] || mkdir -p $MKXPUD_TARGET/$MKXPUD_MOD_PATH/`dirname $M` 
			cp -rfpL --remove-destination $MKXPUD_MOD_PATH/$M $MKXPUD_TARGET/$MKXPUD_MOD_PATH/$M
		fi
		done
	done

	depmod -b $MKXPUD_TARGET $MKXPUD_KERNEL

}

# helper for copying file dependencies
# copydeps (file) (target directory)
function copydeps {
		for i in `./tools/ldd-helper $1`; do 
			TARGET=`dirname $i`
			if [ ! -e $MKXPUD_TARGET$i ]; then
				if [ ! -e $2$TARGET ]; then
					mkdir -p $2$TARGET
				fi
				cp -rfpL --remove-destination $i $2$TARGET
				# uncomment following line for thorough dependency check
				#copydeps $i $2
			fi
		done
}

# post 
function post {

	echo "[mkxpud] Post-install scripts"

	# create symbolic links for /bin/*
	./tools/busybox-helper
	rm $MKXPUD_TARGET/bin/busybox
	
	# check initramfs dependencies
	# FIXME: set initramfs directory as variable
	echo "    Checking initramfs dependencies"
	INITRAMFS_DIR="working/$MKXPUD_CODENAME/initramfs"
	for s in `find $INITRAMFS_DIR -type f`; do
		copydeps $s $INITRAMFS_DIR
	done
	
	# check file dependencies
	echo "    Checking rootfs dependencies"
	for s in `find $MKXPUD_TARGET/{usr,lib,bin,sbin}/ -type f`; do
		copydeps $s $MKXPUD_TARGET
	done
	
	# check dependencies of opt
	for O in `./tools/parser $MKXPUD_CONFIG opt`; do 
		NAME=`./tools/parser package/recipe/$O.recipe name`
		echo "    Checking dependencies of $NAME opt"
		for s in `find $MKXPUD_TARGET/opt/$NAME/ -type f`; do
			copydeps $s $MKXPUD_TARGET/opt/$NAME
		done
	done

	eval `./tools/parser $MKXPUD_CONFIG action`
	
	# pack binaries with upx 
	if [ -e /usr/bin/upx ]; then 
		for o in `./tools/parser $MKXPUD_CONFIG obfuscate`; do
			if [ -e $MKXPUD_TARGET/$o ]; then
			upx $MKXPUD_TARGET/$o
			fi
		done
	fi

}

function image {

	echo "[mkxpud] Generating image..."
	MKXPUD_CODENAME=$1
	export MKXPUD_CONFIG=config/$MKXPUD_CODENAME.cookbook
	eval export `./tools/parser $MKXPUD_CONFIG config`
	export MKXPUD_TARGET=working/$MKXPUD_CODENAME/rootfs

	# temporary hook for squashfs version 
	if [ `mksquashfs -version | grep '0.4'` ]; then 
		MKSQF="/usr/bin/mksquashfs" 
	else 
		MKSQF="`pwd`/tools/mksquashfs"	
	fi
		
	# enable multi-layered rootfs support for Opts 
	for R in `./tools/parser $MKXPUD_CONFIG opt`; do 
		NAME=`./tools/parser package/recipe/$R.recipe name`
		
		if [ ! -e $MKXPUD_TARGET/opt/$NAME ];then

			mv working/$MKXPUD_CODENAME/$NAME $MKXPUD_TARGET/opt/
			
		fi

			# create .opt file
			cd  $MKXPUD_TARGET/opt/
			$MKSQF $NAME $NAME.opt -noappend 
			cd -
			
			# move opt directory out from rootfs
			mv $MKXPUD_TARGET/opt/$NAME working/$MKXPUD_CODENAME/
		
		cd $MKXPUD_TARGET
			# create the cpio.gz format file to be loaded at boot
			find opt/$NAME.opt | cpio -H newc -o | gzip -9 > ../../../deploy/$MKXPUD_CODENAME/$NAME
			# clean up .opt file 
			rm -f opt/$NAME.opt
		cd -
		
	done
	
	# create compressed rootfs to /opt/rootfs.sqf 
	$MKSQF $MKXPUD_TARGET/ working/$MKXPUD_CODENAME/initramfs/opt/rootfs.sqf -noappend 


	## FIXME: use variable instead of actual initramfs path
	cd working/$MKXPUD_CODENAME/initramfs
	find | cpio -H newc -o > ../../../deploy/$MKXPUD_CODENAME.cpio
	cd -

	for format in `./tools/parser $MKXPUD_CONFIG image`; do 
	
	case $format in
			gz)
				cat deploy/$MKXPUD_CODENAME.cpio | gzip -9 > deploy/$MKXPUD_CODENAME/core
				du -h deploy/$MKXPUD_CODENAME/core
			;;
			iso)
				cp -r skeleton/boot/iso/ deploy/$MKXPUD_CODENAME/
				cp $MKXPUD_KERNEL_IMAGE deploy/$MKXPUD_CODENAME/iso/boot/bzImage
				cp deploy/$MKXPUD_CODENAME/* deploy/$MKXPUD_CODENAME/iso/opt/
				mkisofs -R -l -V 'xPUD' -input-charset utf-8 -b isolinux.bin -c boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -o deploy/$MKXPUD_CODENAME.iso deploy/$MKXPUD_CODENAME/iso/
				rm -rf deploy/$MKXPUD_CODENAME/iso/
				./tools/isohybrid deploy/$MKXPUD_CODENAME.iso
				du -h deploy/$MKXPUD_CODENAME.iso
			;;
			exe)
				cp -r skeleton/boot/exe/ deploy/$MKXPUD_CODENAME/
				cp $MKXPUD_KERNEL_IMAGE deploy/$MKXPUD_CODENAME/exe/bzImage
				cp deploy/$MKXPUD_CODENAME/* deploy/$MKXPUD_CODENAME/exe/
				cd deploy/$MKXPUD_CODENAME/exe/
				makensis xpud-installer.nsi
				cd -
				mv deploy/$MKXPUD_CODENAME/exe/xpud-installer.exe deploy/$MKXPUD_CODENAME.exe
				rm -rf deploy/$MKXPUD_CODENAME/exe/
				du -h deploy/$MKXPUD_CODENAME.exe
			;;
			*)
			echo "$format: not supported format"
			;;
	esac 
	done

}

# create standalone opt package from recipe or directory (if recipe was not found)
function makeopt {
	if [ ! -z $2 ] && [ -f "config/$2.cookbook" ]; then export MKXPUD_CODENAME=$2
	 else export MKXPUD_CODENAME='default'; fi
	export MKXPUD_TARGET=working/$MKXPUD_CODENAME/rootfs
	export OPT_PKG=$1
	# check if recipe exists
	if [ ! -z $OPT_PKG ] && [ -f "package/recipe/$OPT_PKG.recipe" ]; then
		echo "Creating standalone opt package $OPT_PKG for $MKXPUD_CODENAME"
		echo "Using recipe, copying files"
		# action 
		eval `./tools/parser package/recipe/$OPT_PKG.recipe action`
		# create opt directory
		NAME=`./tools/parser package/recipe/$OPT_PKG.recipe name`
		# cleanup working opt directory
		rm -rf $MKXPUD_TARGET/opt/$NAME deploy/opt/$MKXPUD_CODENAME/$NAME.opt
		mkdir -p $MKXPUD_TARGET/opt/$NAME
		# copy files
		echo "Copying files"
		for S in binary data config overwrite; do
			for A in `./tools/parser package/recipe/$OPT_PKG.recipe $S`; do
				copyfiles $S $MKXPUD_TARGET/opt/$NAME $A
			done
		done
		# post action 
		eval `./tools/parser package/recipe/$OPT_PKG.recipe post_action`
		# check dependencies
		echo "Checking dependencies..."
		for s in `find $MKXPUD_TARGET/opt/$NAME/ -type f`; do
			copydeps $s $MKXPUD_TARGET/opt/$NAME
		done
		SOURCE_DIR=$MKXPUD_TARGET/opt/$NAME
		TARGET_OPT=deploy/opt/$MKXPUD_CODENAME/$NAME
	else
		# create opt from directory
		if [ -d $OPT_PKG ]; then
			echo "Creating opt from directory"
			SOURCE_DIR=$OPT_PKG
			TARGET_OPT=deploy/opt/`basename $OPT_PKG`
		else
			echo "Error - please specify recipe file or directory."
			exit;
		fi
	fi
	# temporary hook for squashfs version 
	if [ `mksquashfs -version | grep '0.4'` ]; then 
		MKSQF="/usr/bin/mksquashfs" 
	else 
		MKSQF="`pwd`/tools/mksquashfs"	
	fi
	
	mkdir -p deploy/opt/$MKXPUD_CODENAME
	$MKSQF $SOURCE_DIR $TARGET_OPT.opt -noappend
	
	echo "Complete!"
}
