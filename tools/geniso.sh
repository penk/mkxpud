#!/bin/sh

mkisofs -R -l -V 'xPUD' -b isolinux.bin -c boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -o ../xpud-0.8.9.iso .
