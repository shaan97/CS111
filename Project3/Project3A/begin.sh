#!/bin/sh

sudo mount -o loop EXT2_test.img fs
sudo chown -R $1 fs

if [ $? -ne 0 ]
then
    echo "FAILURE. chown failed. Unmounting."
    sudo umount fs
fi
