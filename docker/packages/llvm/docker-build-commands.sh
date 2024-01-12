#!/usr/bin/env bash

set -e
sudo pacman -S --noconfirm clang lld
cd /home/builder/work
sudo mkdir /llvm
sudo chown builder:builder /llvm
cd /llvm
cp -r /home/builder/work/* .
makepkg --syncdeps --noconfirm --needed --cleanbuild --force --skipinteg --config makepkg.conf
mv *.zst /home/builder/work/
