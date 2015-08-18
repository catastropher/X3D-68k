#!/bin/bash
rm -rf lib
mkdir lib
wget https://raw.githubusercontent.com/debrouxl/ExtGraph/master/lib/extgraph.h -P ./lib
wget https://github.com/debrouxl/ExtGraph/raw/master/lib/extgraph.a -P ./lib
