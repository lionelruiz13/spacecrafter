#!/bin/bash
clear
BUILD_MODE=Debug ./install_src.sh --update && gdb spacecrafter -ex="set debuginfod enabled off" -ex="handle SIGUSR1 nostop" -ex="run" -ex="bt"
# Note - to generate a coredump, use generate-core-file.
