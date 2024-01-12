#!/usr/bin/env bash

set -e
cd /home/builder/work
sudo pacman -S --noconfirm clang lld
makepkg --syncdeps --noconfirm --needed --clean --cleanbuild --force --skipinteg
