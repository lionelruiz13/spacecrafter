#!/bin/bash
echo "Files not compiled :"
((echo "ls src"; ls) | sed -z "s/\n/ | grep -v /g"; echo) | sed -z "s/ | grep -v \n/\n/g" | sed "s/\.spv//g" | bash | grep -v "\.glsl"
