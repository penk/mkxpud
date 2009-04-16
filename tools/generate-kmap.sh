#!/bin/bash
	dumpkmap > current.kmap || return 1
	for i in $(cd /usr/share/keymaps/i386 ; ls */*.kmap.gz); do
		[ "$(dirname $i)" = "include" ] && continue
		i=$(basename $i)
		i=${i%.map.gz}
		loadkeys $i && dumpkmap > $i.kmap
	done
	loadkmap < current.kmap

