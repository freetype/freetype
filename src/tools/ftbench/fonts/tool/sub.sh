#!/bin/bash
# This tool is to subset fonts.

# Define the Unicode range
unicodes="U+0021-007E"

# Loop over all .ttf files in the current directory
for fontfile in *.ttf
do
  # Generate the output filename
  output="${fontfile%.ttf}_subset.ttf"

  # Run the pyftsubset command
  pyftsubset "$fontfile" --unicodes=$unicodes --output-file="$output"
done

