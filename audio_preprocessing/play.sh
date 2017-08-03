#!/bin/bash

# USED FOR OLD AUDIO PROCESSING METHOD

FILES=/Users/raysmets/Downloads/tjp_songs/*
for f in $FILES
do
  echo "+++"
  echo "Processing $f file..."
  echo "+++"
  #cat $f
  afplay -q 1  -v 5 $f
  afplay /Users/raysmets/Downloads/Silence/Silence_.1_sec.mp3
done
