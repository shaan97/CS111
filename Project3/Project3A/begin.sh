#!/bin/sh

sudo mount -o loop EXT2_test.img fs
sudo chown -R $1 fs
