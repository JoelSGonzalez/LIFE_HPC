CC = gcc
CFLAGS = -g -Wall -I$(INCLUDE_DIR)

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
OUTOUT_DIR = out

_SRC_FILES = PCAD.c life.c
SRC_FILES = ${addprefix ${SRC_DIR}/, ${_SRC_FILES}}

TARGETS = test_PCAD

all: ${TARGETS}

# life_test:
# 	${CC} ${CFLAGS} ${SRC_FILES} -o ${BUILD_DIR}/$@ && ./${BUILD_DIR}/$@ 

test_PCAD:
	${CC} ${CFLAGS} ${SRC_FILES} -o ${BUILD_DIR}/$@
	./${BUILD_DIR}/$@ -s 200 -g 10 > teste_20_10.txt 2> t_20_10_err.txt
	./${BUILD_DIR}/$@ -s 200 -g 10 > teste_200_10.txt 2> t_200_10_err.txt
	./${BUILD_DIR}/$@ -s 2000 -g 10 > teste_2000_10.txt 2> t_2000_10_err.txt