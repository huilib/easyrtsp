##
## File Name: mkvars.mk
## Create Author: Joseph Hui
## Create Date: Thu Apr 7 1039 2022
## Description:
## 		variables define for huicpp's main makefile.
##


DEPS_HOME = ${output_dir}/deps

#CURL_DEP_HOME = ${DEPS_HOME}/curl
JSON_DEP_HOME = ${DEPS_HOME}/json
OPENSSL_DEP_HOME = ${DEPS_HOME}/openssl
HUICPP_DEP_HOME = ${DEPS_HOME}/huicpp
FFMPEG_DEP_HOME = ${DEPS_HOME}/ffmpeg

JSON_INCLUDE_DIR = ${JSON_DEP_HOME}
OPENSSL_INCLUDE_DIR = ${OPENSSL_DEP_HOME}/include
HUICPP_INCLUDE_DIR = ${HUICPP_DEP_HOME}/include
FFMPEG_INCLUDE_DIR = ${FFMPEG_DEP_HOME}/include

OPENSSL_LIB_DIR = ${OPENSSL_DEP_HOME}/lib
HUICPP_LIB_DIR = ${HUICPP_DEP_HOME}/lib
FFMPEG_LIB_DIR = ${FFMPEG_DEP_HOME}/lib

PRONAME = huicpp
CTNUM = 8
