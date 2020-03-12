#!/bin/bash

# Bash script to install supporting software for the Robotics Cape
# This is specifically for Debian Jessie


################################################################################
# Variables collected here for convenience
################################################################################

KERNEL="$(uname -r)"
DEBIAN="$(cat /etc/debian_version)"
MODEL="$(cat /proc/device-tree/model)"
DOGTAG="$(cat /etc/dogtag)"

echo " "
echo "Detected Linux kernel: $KERNEL"
echo "Detected Debian version: $DEBIAN"
echo "Detected Model: $MODEL"
echo "Detected $DOGTAG"
echo " "
echo " "

# kernel version check
MINDEBIAN=8.6
MINKERNEL="4.4.32"
function version_lt() { test "$(echo "$@" | tr " " "\n" | sort -rV | head -n 1)" != "$1"; }

################################################################################
# Sanity Checks
################################################################################

# make sure the user is root
if [ `whoami` != 'root' ]; then
	echo "You must be root to install this."
	exit 1
fi

# make sure the release is really jessie
if ! grep -q "9." /etc/debian_version ; then
	echo "ERROR: This is not Debian Jessie."
	echo "Flash the latest Jessie image to your BBB"
	echo "or use the Wheezy branch of this installer."
	exit 1
fi

# check that the remoteproc driver is there
if modprobe -n remoteproc | grep -q "not found" ; then
	echo "ERROR: remoteproc module not found"
	echo "Use a standard TI kernel with remoteproc instead."
	exit 1
fi

# debian version check
if version_lt $DEBIAN $MINDEBIAN; then
	echo "WARNING: Debian version $MINDEBIAN or newer is required"
	exit 1
fi

# kernel version check
if version_lt $KERNEL $MINKERNEL; then
	echo "WARNING: Kernel $MINKERNEL or newer is required for full functionality"
	echo "Motor 1, PINMUX, and PRU functions will not work as-is"
	echo "You may still continue the installation"
	echo " "
fi

# make sure the user really wants to install
echo "This script will install all Redperipherallib supporting software"
read -r -p "Continue? [y/n] " response
case $response in
	[yY]) echo " " ;;
	*) echo "cancelled"; exit;;
esac
echo " "



################################################################################
# Compile and install library, examples, and services
# This works for Black and Blue
################################################################################
find . -exec touch {} \;
make clean
make install
ldconfig


echo " "
echo " "
echo " "
echo "Redperipherallib package installation complete."
echo "Please reboot now."
echo " "
