#!/bin/bash

mkdir -p build
gcc main.c -o build/text-align

(
  echo Left
  cat lorem.txt | ./build/text-align --left
  echo

  echo Center
  cat lorem.txt | ./build/text-align --center
  echo

  echo Right
  cat lorem.txt | ./build/text-align --right
  echo

  echo Border Left
  cat lorem.txt | ./build/text-align --border --left
  echo

  echo Border Center
  cat lorem.txt | ./build/text-align --border --center
  echo

  echo Border Right
  cat lorem.txt | ./build/text-align --border --right
) > sample.txt
