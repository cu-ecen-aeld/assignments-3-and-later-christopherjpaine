#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$(realpath $1)
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

    # Kernel Build
    #   - mrproper : deep clean the folder
    #   - defconfig : Create the default config, targetting "virt-arm" I'm not
    #                 sure what here targets this though
    #   - all : Build the kernel image for QEMU
    #   - modules : Build kernel modules that can be loaded at runtime
    #   - dtbs : Build the Device Tree
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE mrproper
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
    make -j4 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE all
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE modules
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE dtbs
fi

echo "Adding the Image in outdir"
cp $OUTDIR/linux-stable/arch/${ARCH}/boot/Image $OUTDIR

echo "Creating the staging directory for the root filesystem"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# Create folder tree according to Filesystem Hierarchy Standard
ROOTFS=$OUTDIR/rootfs
mkdir -p $ROOTFS
cd $ROOTFS
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    
    # Configure busybox
    make distclean 
    make defconfig
else 
    cd busybox
fi

# Make and install busybox
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE CONFIG_PREFIX=$ROOTFS
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE CONFIG_PREFIX=$ROOTFS install

# echo "Library dependencies"
# cd $ROOTFS
# ${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
# ${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# Add library dependencies to rootfs
# - Copy from sysroot of toolchain
# - NB: USe option -a to preserve all attributes (permissions etc)
SYSROOT=$(${CROSS_COMPILE}gcc --print-sysroot)
cp -a $SYSROOT/lib/ld-linux-aarch64.so.1 $ROOTFS/lib/
cp -a $SYSROOT/lib64/libm.so.6 $ROOTFS/lib64/
cp -a $SYSROOT/lib64/libresolv.so.2 $ROOTFS/lib64/
cp -a $SYSROOT/lib64/libc.so.6 $ROOTFS/lib64/

# Make the device nodes needed by Busybox
sudo mknod -m 666 $ROOTFS/dev/null c 1 3
sudo mknod -m 600 $ROOTFS/dev/console c 5 1

# Clean and build the writer utility for the cross target
cd $FINDER_APP_DIR
make clean all CROSS_COMPILE=$CROSS_COMPILE

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp $FINDER_APP_DIR/writer $ROOTFS/home/
cp $FINDER_APP_DIR/finder.sh $ROOTFS/home/
cp $FINDER_APP_DIR/finder-test.sh $ROOTFS/home/
cp -r $FINDER_APP_DIR/conf/ $ROOTFS/home/conf
cp $FINDER_APP_DIR/autorun-qemu.sh $ROOTFS/home/

# Create initramfs.cpio.gz
# - Specify root:root for all files
# - Compress the resulting file
cd $ROOTFS
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f $OUTDIR/initramfs.cpio
