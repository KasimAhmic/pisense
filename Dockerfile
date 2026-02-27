# Portable container for building PiSense for arm64 on x86_64 machines.
#
# Building: docker build -t pisense-builder .
# Running: docker run --rm -it -v $(pwd):/work -w /work pisense-builder bash -lc "{CMAKE_COMMAND}"
#
# See scripts/ci.sh for premade examples.

FROM debian:13.3-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  git \
  pkg-config \
  openssh-client \
  ca-certificates \
  gcc-aarch64-linux-gnu \
  g++-aarch64-linux-gnu \
  dpkg-dev \
  ccache \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /work

CMD ["cmake", "--version"]
