#!/bin/bash

# TODO: Add architecture argument

set -e

function configure() {
  docker run --rm -it \
    -v "$(pwd)":/work \
    -w /work \
    pisense-builder \
    bash -lc "cmake --preset arm64"
}

function build() {
  docker run --rm -it \
    -v "$(pwd)":/work \
    -w /work \
    pisense-builder \
    bash -lc "cmake --build --parallel --preset build-arm64"
}

# TODO: May or may not actaully be correctly rebuilding everything, look into later
function rebuild() {
  docker run --rm -i \
    -v "$(pwd)":/work \
    -w /work \
    pisense-builder \
    bash -lc "cmake --fresh --preset arm64"

  docker run --rm -i \
    -v "$(pwd)":/work \
    -w /work \
    pisense-builder \
    bash -lc "cmake --build --parallel --preset build-arm64"
}

function deploy() {
  echo "Copying files..."
  rsync -avzq \
    ./build/pisense-arm64/sense \
    ./config.ini \
    kahmic@192.168.0.27:/home/kahmic/sense/
  echo "Files copied."
  echo ""

  echo "Stopping Sense..."
  ssh kahmic@192.168.0.27 "sudo pkill -f sense"
  echo "Sense stopped."
  echo ""

  echo "Starting Sense..."
  ssh kahmic@192.168.0.27 '
    set -e
    cd sense
    nohup ./sense > sense.log 2>&1 < /dev/null &
    sleep 0.5
    pgrep -x sense >/dev/null
  '
  echo "Sense started."
}

function watch() {
  inotifywait \
    --monitor \
    --event close_write,create,moved_to \
    --format "%w%f" \
    ./build/pisense-arm64 \
  | while read -r path; do
    [[ "$path" == */sense ]] || continue

    echo ""
    echo "Detected change to sense binary, deploying Sense..."
    echo ""

    scripts/ci.sh deploy
  done
}

function help() {
  echo "Usage: $0 {configure|build|deploy|watch|help}"
}

COMMAND="$1"

case "${COMMAND:-help}" in
  configure)
    configure
    exit 0
    ;;
  build)
    build
    exit 0
    ;;
  rebuild)
    rebuild
    exit 0
    ;;
  deploy)
    deploy
    exit 0
    ;;
  watch)
    watch
    exit 0
    ;;
  help)
    help
    exit 0
    ;;
  *)
    echo "Unknown command: $COMMAND"
    help
    exit 1
    ;;
esac
