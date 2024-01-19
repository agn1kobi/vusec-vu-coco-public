#!/usr/bin/env bash

# Invocation to just use the prebuilt docker image:
#   run.sh
# OR
#   run.sh --build
# which will build the docker image locally.


set -e
set -o pipefail

# cd to the framework dir.
cd "$(dirname "$0")"

# Some stuff to pretty print errors/info.
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
END='\033[0m'

OK="[ ${GREEN}OK${END}]"
INFO="[${BLUE}---${END}]"
ERROR="[${RED}ERR${END}]"

# Tell students that they don't need to run docker on the VM.
if test -f "/home/coco/.coco-machine-token"; then
    echo -e "${INFO} You are on the CoCo VM. You don't need to"
    echo -e "      use docker. You can run all commands in this shell."
    exit
fi

# Sanity check if docker is installed.
if ! command -v docker &> /dev/null
then
    echo -e "${ERROR} Docker does not seem to be installed."
    echo "      Follow one of the install guides here:"
    echo "      https://docs.docker.com/get-docker/"
    exit
fi
echo -e "${OK} Found docker"

# The docker user to pull the image from.
# Has to be a docker.io user account with a 'coco' image.
export DOCKER_USER=teemperor

# Decide whether we can just pull the image or build it locally.
if [ "$1" == "--build" ]; then
    echo -e "${INFO} Building docker image"
    # Build the docker image if necessary.
    docker build --network=host -t ${DOCKER_USER}/coco docker/runner
    echo -e "${OK} Build docker image"
else
    echo -en "${INFO} Downloading docker image (This might take a while): "
    set +e
    docker pull --quiet ${DOCKER_USER}/coco
    if [ $? -eq 0 ]
    then
        echo -e "${OK} Downloaded docker image"
    else
        echo -e "${ERROR} Failed to download docker image."
        echo -e "${INFO} Check if you are connected to the internet."
        echo -e "${INFO} If you are, try rerunning script like this: '${0} --build'."
        exit 1
    fi
    set -e
fi

# Use the host shell if possible.
case "$SHELL" in
  */fish)
    export DOCKER_SHELL=fish
    ;;

  *)
    export DOCKER_SHELL=zsh
    ;;
esac

echo -e "${INFO} Starting shell."
echo -e "${INFO} Use 'make check-frontend' and 'make check-passes' to run tests."

# Go into the docker environment.
# The ptrace permission allows debugging.
# The mount makes our dir persistent.
docker run -w /home/coco/work --hostname coco -it \
  --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
  -v "$(pwd)":/home/coco/work:z \
  ${DOCKER_USER}/coco /usr/bin/${DOCKER_SHELL}


 # --mount type=bind,source="$(pwd)"/.,target=/home/coco/work \
