#!/bin/bash

set -eu
mkdir -p output
found=0

for input_file in input/*.txt; do

    [ -f "$input_file" ] || continue

    found=1
    filename="${input_file##*/}"
    name="${filename%.txt}"
    output_file="output/${name}-result.txt"
    
    echo "Processing: $input_file"

    ./drop_calc \
        --input-file "$input_file" \
        --output-file "$output_file"

    echo "Created: $output_file"
done

if [ "$found" -eq 0 ]; then
    echo "No .txt files found in input/"
fi
