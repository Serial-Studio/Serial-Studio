#!/bin/bash
set -ev
BREWS="qt5 cmake"
for i in $BREWS; do
  brew outdated | grep -q $i && brew upgrade $i
done
for i in $BREWS; do
  brew list | grep -q $i || brew install $i
done
