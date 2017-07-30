@echo off
em++ -std=c++11 -O2 src/unity_build.cpp -o ld39.html -s USE_SDL=2 -s TOTAL_MEMORY=67108864 --preload-file data
rem em++ -std=c++11 src/unity_build.cpp -o ld39.html -s USE_SDL=2 -s TOTAL_MEMORY=67108864 --embed-file data