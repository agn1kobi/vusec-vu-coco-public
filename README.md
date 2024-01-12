Compiler Construction framework
===============================

Overview
--------

This is the framework documentation for Compiler Construction at the Vrije
Universiteit Amsterdam. During this course, students will develop compiler
features for a custom C-like language.

The framework consists of several parts:

- A compiler frontend, written in Python, that translates our C-like source
  language into LLVM IR which can be compiled to a binary using Clang.

- LLVM passes, written in C++, that optimize/instrument the IR.

- A runtime library, written in C, containing support code for instrumentation
  inserted by LLVM passes.

- Tests for each part/assignments, and a number of test programs that can be
  instrumented with any LLVM pass.

Detailed documentation of these components resides in frontend/README.md,
llvm-passes/README.md and examples/README.md.


Getting started
---------------

Execute the `run.sh` file in this directory like this:

   $ bash run.sh

It will download the docker image for this course and then run it.
If you don't have docker installed on your machine, please follow the
documentation here: https://docs.docker.com/get-docker/
The VM of the course has docker preinstalled if you can not install
docker on your machine.

This directory will be mounted inside that docker container. You can edit the
course files in the docker container from your current machine. If you
execute any command such as `make`, `myopt` and `lldb-myopt` you need
to run them in the shell spawned by `run.sh` (which executes those
commands in the docker container).

Using the Makefile
------------------

The root directory contains a `Makefile` which is used to build parts of the
compiler. It offers a number of commands:

    $ make passes           # build the passes
    $ make runtime          # build the runtime
    $ make check-frontend   # run frontend tests
    $ make check-passes     # run passes tests
    $ make examples         # build all examples (see examples/README.md)
    $ make example-$name    # build a single example (see examples/README.md)
    $ make handin-$num      # create tarball to handin on Canvas, per assignment
    $ make clean            # delete build files
    $ make cleaner          # delete all generated files, including lib/

Note that dependencies are automatically taken care of: the passes and runtime
will automatically rebuild when you run the tests or examples for instance.


Source directories
------------------

    doc/                    # framework/language documentation
    frontend/               # frontend code (Assignment 1)
    frontend/README.md      # further documentation on frontend
    llvm-passes             # LLVM passes (Assignments 2 and 3)
    llvm-passes/README.md   # further documentation on passes
    runtime/                # runtime support code for passes
    examples/               # some example programs to test your compiler
    support/                # FenneC syntax highlighting files for Vim
    Makefile                # build commands, see previous section
    README.md               # this file
