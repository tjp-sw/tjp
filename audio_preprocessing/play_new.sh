#!/bin/bash
FILES=/Users/raysmets/Downloads/tjp_songs/*
for f in $FILES
do
  #cat $f
  python envelope_detector.py $f

done
