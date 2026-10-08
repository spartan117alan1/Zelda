#!/bin/zsh
arch -arm64 g++ -std=c++17 src/*.cpp -o zelda_game \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lsfml-graphics -lsfml-window -lsfml-system
