#!/bin/sh
#
# provides an X server for the given program to run in
# (loosely based on xvfb-run from Debian's Xvfb package)
#
set -e

# grab the first available X server number >=42
srv=42
while true; do
	lock=/tmp/.X$srv-lock
	mkdir $lock 2>/dev/null && break || :
	
	# keep trying until we hit 100
	srv=$((srv+1))
	[ $srv -lt 100 ]
done

# set up authorization
export DISPLAY=:$srv
export XAUTHORITY=$(mktemp -t xsh.XXXXXX)
echo "add $DISPLAY . $(mcookie)" | xauth source -

# start the virtual display and wait for SIGUSR1 from Xvfb
trap : USR1
(
	trap '' USR1
	rmdir $lock
	exec Xvfb $DISPLAY \
		-screen 0 1024x768x24 \
		-nolisten tcp
) &

pid=$!
wait || :

# perform cleanup on exit
cleanup() {
	kill $pid || :
	xauth remove $DISPLAY || :
	rm -f $XAUTHORITY || :
}
trap cleanup EXIT

# ensure Xvfb started properly
kill -0 $pid 2>/dev/null

# run the given program + arguments
rv=0
"$@" || rv=$?
exit $rv
