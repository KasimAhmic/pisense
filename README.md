# PiSense

A C++ app that leverages the Raspberry Pi SenseHat to report ambient environment information.

## Why?

I of course could have done this in Python in an hour or so, but I wanted to challenge myself. I wanted to practice more with C++ as well as learn more about low level, I2C communication.

## Building and Running

You will need CMake and a C++ compiler. I have tested this with GCC (G++) on Raspberry Pi OS 64-bit though any Linux OS or Docker container should work in theory.

```bash
git clone --recurse-submodules https://github.com/KasimAhmic/pisense
cd pisense
mkdir build

cmake --preset arm64
cmake --build --preset build-arm64

./build/sense
```

If you're on a platform that doesn't have access to aarch64 components, you can use the provided build container and `ci.sh` script:

```bash
docker build -t pisense-builder .

./scripts/ci.sh configure
./scripts/ci.sh build
```

You should start to see messages being logged to the console.

## Reporting Data

There is no reporting built in yet aside from the console logger. My goal is to provide "adapters" for things like CSV, Postgres, MySQL, Telegraf, etc. Still have no idea what shape that will take, only time will tell.

## Contributing

I don't _really_ intend on this being a major community project or anything, but if there's interest and you wanna help out, feel free to send a PR!
