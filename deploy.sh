#!/bin/bash

rsync -avz \
  --delete \
  ./ \
  --exclude build/ \
  kahmic@192.168.0.27:/home/kahmic/sense/
