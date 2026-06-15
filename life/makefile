CC = gcc
CFLAGS = -fopenmp -g -Wall -I$(INCLUDE_DIR)

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
PROGRAM = PCAD_test
OUT_DIR = out
ERR_DIR = err

SIZES = 10 20 50 100 200 500 1000 2000
GENS = 10 20 50 100 200

_SRC_FILES = PCAD.c life.c
SRC_FILES = ${addprefix ${SRC_DIR}/, ${_SRC_FILES}}

TARGETS = compile serial_test

all: ${TARGETS}

# life_test:
# 	${CC} ${CFLAGS} ${SRC_FILES} -o ${BUILD_DIR}/$@ && ./${BUILD_DIR}/$@ 

compile:
	${CC} ${CFLAGS} ${SRC_FILES} -o ${BUILD_DIR}/${PROGRAM}

s10_g10_serial:
	./${BUILD_DIR}/${PROGRAM} -s 10 -g 10 > ${OUT_DIR}/$@.txt 2> ${OUT_DIR}/$@_err.txt

serial_test:
	@for s in $(SIZES); do \
		for g in $(GENS); do \
			echo "Rodando s=$$s g=$$g"; \
			./build/PCAD_test -s $$s -g $$g \
			> ${OUT_DIR}/serial_$${s}_$${g}.txt \
			2> ${ERR_DIR}/serial_$${s}_$${g}_err.txt; \
		done \
	done