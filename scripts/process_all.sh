#!/bin/bash

for dir in data/songs/*; do
    echo "Process: $(basename ${dir})"
    ./scripts/process.py $(basename ${dir})
done
