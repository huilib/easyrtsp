##===================================================================##
## File Name: inc.mk
## Create Author: Joseph Hui
## Create Date: Tue Sep 12 1359 2015
## Description:
##		Makefile header. This include some declaration for using in 
##	makefile. I create this file for reduce duplication.
## DONOT MODIFY THIS FILE UNTIL YOU UNDERSTAND THE DETAIL.
##===================================================================##

## setup work-dir and dir-name
now_dirname = ${shell basename ${shell pwd}}
now_dir = ${shell pwd}

GG_HEADER = 
PLATSYSCODE = linux64

ifeq (${now_dirname}, src)
include ../mkvars.mk
output_dir = ${now_dir}/..
else 
include mkvars.mk
output_dir = ${now_dir}
endif

CC = ${GG_HEADER}g++
AR_CMD = ${GG_HEADER}ar cr

#BIN_PATH = ${output_dir}/bin/${PLATSYSCODE}
BIN_PATH = ${output_dir}/bin
LIB_PATH = ${output_dir}/lib

CXX_CFLAGS = -fPIC -O0 -g 
CXX_CFLAGS += -Wall -Wextra -Werror -Wno-unknown-pragmas 
CXX_CFLAGS += -fstack-protector-strong -fstack-protector -std=c++17

CXX_DEFINES :=

CXX_INCLUDE = -I${JSON_INCLUDE_DIR}
CXX_INCLUDE += -I${HUICPP_INCLUDE_DIR}
CXX_INCLUDE += -I${FFMPEG_INCLUDE_DIR}

CC_CFLAGS :=
CC_DEFINES :=
CC_INCLUDE :=

SO_CXX_CFLAGS = -fpic -shared 
SO_C_CFLAGS :=

LINK_CXX_CFLAGS :=
#LINK_CXX_LIBS += -L${OPENSSL_LIB_DIR} -lcrypto -lssl
LINK_CXX_LIBS += ${HUICPP_LIB_DIR}/libhuicrypto.a 
LINK_CXX_LIBS += ${OPENSSL_LIB_DIR}/libcrypto.a ${OPENSSL_LIB_DIR}/libssl.a
LINK_CXX_LIBS += ${HUICPP_LIB_DIR}/libhuicpp.a -pthread

LINK_CXX_LIBS += ${FFMPEG_LIB_DIR}/libavformat.a ${FFMPEG_LIB_DIR}/libavcodec.a ${FFMPEG_LIB_DIR}/libavutil.a
LINK_CXX_LIBS += ${FFMPEG_LIB_DIR}/libswresample.a
#LINK_CXX_LIBS += -lavformat -lavcodec -lavdevice -lavutil -lswscale -lswresample -pthread
LINK_CXX_LIBS += -ldl -lz -llzma

LINK_CC_CFLAGS :=
LINK_CC_LIBS :=

AR_CC_CFLAGS :=

.PHONY: all clean install test type_test

OS_NAME = $(shell uname)

TTYPE = AFILE

TARGET :=

SRCS = $(wildcard *.cpp $(foreach ss, ${SUB_SRCS}, ${ss}/*.cpp))
OBJS = ${patsubst %.cpp, %.o, ${SRCS}}

