#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
  brew install glfw3 glew
  brew install freeglut  
  brew install libpng   
else
  if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if command -v apt-get &>/dev/null; then
      sudo apt-get update
      sudo apt-get install libglfw3 libglfw3-dev libglew-dev freeglut3-dev libpng-dev

    elif command -v dnf &>/dev/null; then
      sudo dnf install glfw glew-devel freeglut-devel libpng-devel

    elif command -v pacman &>/dev/null; then
      sudo pacman -S glfw glew freeglut libpng
    fi
  fi
fi

echo "Installation complete."
