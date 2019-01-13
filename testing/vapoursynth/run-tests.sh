#!/bin/sh
set -e

# use in-tree build
mkdir -p xdg/vapoursynth
echo "SystemPluginDir=../.libs" > xdg/vapoursynth/vapoursynth.conf

# test ImageMagick plugin
convert -size 256x256 xc:#dd2222 red.png
cat > red.vpy <<"EOF"
import vapoursynth as vs
core = vs.get_core()
c = core.imwri.Read("red.png")
c.set_output()
EOF

# IM segfaults depend on unknowns, including cpu features and aslr
i=0
while [ $i -lt 16 ]; do
	i=$((i+1))
	
	XDG_CONFIG_HOME=xdg \
	PYTHONPATH=../.libs \
	LD_LIBRARY_PATH=../.libs \
	../.libs/vspipe red.vpy - > red.yuv
	
	# check that we produced video data
	[ $(stat -c%s red.yuv) -gt 32768 ]
done

exit 0

# run the bundled unit tests
for file in *.py; do
	echo "[*] $file"
	
	XDG_CONFIG_HOME=xdg \
	PYTHONPATH=../.libs \
	python3 "$file"
done
