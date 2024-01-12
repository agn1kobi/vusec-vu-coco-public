#!/usr/bin/env bash

set -e

# Build the docker image if necessary.
docker build -t coco-llvm .

docker run -w /home/builder/work -it \
  --mount type=bind,source="$(pwd)"/.,target=/home/builder/work \
  coco-llvm \
  bash -x /home/builder/work/docker-build-commands.sh
