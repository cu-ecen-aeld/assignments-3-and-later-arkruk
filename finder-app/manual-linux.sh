#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
HOME_SCRIPT_LOCATION=`pwd`
LIBC_LOCATION="$(dirname `which ${CROSS_COMPILE}gcc`)/../aarch64-none-linux-gnu/libc/"

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} all

    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} modules
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} dtbs
    cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image $OUTDIR

    # TODO: Add your kernel build steps here
fi



echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

pwd
# TODO: Create necessary base directories
mkdir -p rootfs
mkdir -p rootfs/bin rootfs/dev rootfs/etc rootfs/home rootfs/lib rootfs/lib64 rootfs/proc rootfs/sbin rootfs/sys rootfs/tmp rootfs/usr rootfs/var
mkdir -p rootfs/usr/bin rootfs/usr/lib rootfs/usr/sbin
mkdir -p rootfs/var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    

    # TODO:  Configure busybox
else
    cd busybox
fi

make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
# TODO: Make and install busybox
cd ${OUTDIR}/rootfs
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
cp -r ${LIBC_LOCATION} ${OUTDIR}/rootfs

# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
cd ${HOME_SCRIPT_LOCATION}
make clean
make CROSS_COMPILE=${CROSS_COMPILE}gcc

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
sudo cp finder.sh ${OUTDIR}/rootfs/home
sudo cp writer ${OUTDIR}/rootfs/home
sudo cp finder-test.sh ${OUTDIR}/rootfs/home
sudo cp autorun-qemu.sh ${OUTDIR}/rootfs/home
sudo cp -r conf ${OUTDIR}/rootfs/home/conf

cd ${OUTDIR}/rootfs
sudo chown -R root:root *
# TODO: Chown the root directory

# TODO: Create initramfs.cpio.gz
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip -f initramfs.cpio
