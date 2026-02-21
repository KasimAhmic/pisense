#!/bin/bash

rsync -avz \
  --delete \
  ./ \
  --exclude build/ \
  --exclude .cache/ \
  --exclude .git/ \
  kahmic@192.168.0.27:/home/kahmic/sense/
