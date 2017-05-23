#!/usr/bin/env bash

DEV_DIR="grammar/"
GEN_DIR="gen/"
SOURCE_NAME="grammar"
FLEX_SOURCE=${SOURCE_NAME}".lex"
FLEX_OUTPUT=${SOURCE_NAME}".lex.c"
BISON_SOURCE=${SOURCE_NAME}".y"
BISON_OUTPUT=${SOURCE_NAME}".tab.c"
OUTPUT="hpdb"

cd ${DEV_DIR}

flex -o ${GEN_DIR}${FLEX_OUTPUT} ${FLEX_SOURCE}
#echo "-- FLEX SUCCESS" ${GEN_DIR}${FLEX_OUTPUT}
bison -o ${GEN_DIR}${BISON_OUTPUT} -vd ${BISON_SOURCE}
#echo "-- BISON SUCCESS" ${GEN_DIR}${BISON_OUTPUT}
cd ..
g++ -o ${OUTPUT} main.cpp -lfl -Wno-write-strings