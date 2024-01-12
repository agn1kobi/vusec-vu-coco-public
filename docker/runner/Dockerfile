FROM archlinux:base-devel

# Install the wrappers we provide (frontend, etc.)
COPY wrappers/* /bin/

# Install some dependencies/debuggers/editors
RUN pacman -Sy --noconfirm cmake vim gdb nano zsh fish tmux python-pip wget libedit git

# Add a custom user and allow sudo
RUN useradd coco -u 1000 -m -s /bin/zsh
RUN echo "coco ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Setup a tmp dir for our llvm/lldb installation.
RUN mkdir /llvm-tmp
WORKDIR /llvm-tmp

# Install LLVM and a custom LLDB that doesn't depend on system LLVM.
RUN wget "https://download.vusec.net/vms/coco/lldb-14.0.6-1-x86_64.pkg.tar.zst"
RUN wget "https://download.vusec.net/vms/coco/llvm-10.0.1-2-x86_64.pkg.tar.zst"
RUN wget "https://download.vusec.net/vms/coco/libclang-10.0.1-2-x86_64.pkg.tar.zst"
RUN pacman -U --noconfirm lldb-14.0.6-1-x86_64.pkg.tar.zst
RUN pacman -U --noconfirm llvm-10.0.1-2-x86_64.pkg.tar.zst
RUN pacman -U --noconfirm libclang-10.0.1-2-x86_64.pkg.tar.zst

# Setup the LLVM source for debuggers to use.
RUN mkdir -p /llvm/src
RUN wget "https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.1/llvm-project-10.0.1.tar.xz" -O llvm.tar.xz
RUN tar xf llvm.tar.xz
RUN mv llvm-project-10.0.1 /llvm/src/llvm-project

# Cleanup all the downloaded LLVM temporaries.
RUN rm -rf /llvm-tmp

USER coco

# Disable annoying zsh startup spam.
RUN echo "" > ~/.zshrc

# Setup vim completion
RUN mkdir -p ~/.vim/pack/plugins/start
RUN git clone --depth=1 https://github.com/ervandew/supertab.git ~/.vim/pack/plugins/start/supertab
RUN git clone --depth=1 https://github.com/xavierd/clang_complete.git ~/.vim/pack/completion/start/clang_complete
RUN git clone --depth=1 https://github.com/bfrg/vim-cpp-modern ~/.vim/pack/git-plugins/start/vim-cpp-modern
RUN echo -e "let g:clang_library_path='/opt/libclang/lib/libclang.so'\nsyntax on" > ~/.vimrc

# Install Python dependencies
RUN pip install ply termcolor llvmlite==0.34 setuptools
