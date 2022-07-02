#!/bin/bash
OUTPUT_PATH="Dockerfile"
CMAKE_VERSION="3.20"
PROJECT_VERSION="0.0.1"
CONTEXT_DIR="."

get_os() 
{
    COMPARE_CMAKE_VERSION="3.19"
    LESSER_CMAKE_VERSION="3.19"

    LESSER_CMAKE_VERSION="$(echo -e "$COMPARE_CMAKE_VERSION\n$CMAKE_VERSION" | sort -V | head -n1)"

    if [ "$LESSER_CMAKE_VERSION" = "$COMPARE_CMAKE_VERSION" ]; then
        echo "linux"
    else
        echo "Linux"
    fi

}

version_compare() {
    [ "$1" = "$2" ] && return 1 || verlte $1 $2
}

while getopts ":o:c:v:d:" opt; do
  case $opt in
    o)
      OUTPUT_PATH="$OPTARG"
      ;;
    c)
      CMAKE_VERSION=$OPTARG
      ;;
    v)
      PROJECT_VERSION=$OPTARG
      ;;
    d)
      CONTEXT_DIR=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

echo -e "FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update -y && \\
    apt-get autoremove -y && \\    
    apt-get install -y build-essential && \\
    apt-get install -y wget && \\
    apt-get install -y git && \\
    apt-get install -y clang-tidy && \\
    apt-get install -y clang-format && \\
    apt-get install -y python3-pip && \\
    pip install cmake-format

# Install correct cmake version
RUN mkdir ~/temp && \\
    cd ~/temp && \\
    wget https://cmake.org/files/v$CMAKE_VERSION/cmake-$CMAKE_VERSION.0-$(get_os)-x86_64.sh && \\
    mkdir /opt/cmake && \\
    sh cmake-$CMAKE_VERSION.0-$(get_os)-x86_64.sh --skip-license --prefix=/opt/cmake && \\
    ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

# Add non root user
RUN groupadd --gid $(id -g) grow && \\
    useradd --no-log-init -rm -s /bin/bash -u $(id -u) -g $(id -g) grow" > "${OUTPUT_PATH}"

if [ -f "$OUTPUT_PATH" ]; then
  docker build -t grow_build:v${PROJECT_VERSION} -f ${OUTPUT_PATH} ${CONTEXT_DIR}
else
  echo "Unable to find generated Dockerfile"
  exit 1
fi

exit 0
