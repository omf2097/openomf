#!/bin/sh

MODULES=`cat modules | awk -e '{print $2}'`
for MOD in $MODULES; do
		URL=`grep -A 1 $MOD ../.gitmodules | tail -1 | awk -e '{print $3}'`
		SHA=`grep $MOD modules | awk -e '{print $1}'`
		DIR=`basename $MOD`
		if [ ! -d $DIR ]; then
				git clone $URL
		fi
		cd $DIR
		git reset --hard $SHA
		cd ..
done
