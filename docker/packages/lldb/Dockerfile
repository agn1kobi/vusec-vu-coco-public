FROM archlinux:base-devel

RUN pacman -Syu --noconfirm libedit
RUN echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
RUN useradd builder -u 1000 -m -s /bin/bash
USER builder
