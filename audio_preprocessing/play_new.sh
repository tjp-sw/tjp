#!/bin/bash
FILES=/Users/raysmets/Downloads/Processed/*
for f in $FILES
do
  #cat $f
  python envelope_detector.py $f

done
