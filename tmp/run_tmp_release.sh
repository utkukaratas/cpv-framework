#!/usr/bin/env bash
set -e

BUILDDIR=../build/cpvframework-tmp-release

mkdir -p ${BUILDDIR}
cd ${BUILDDIR}
cmake -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=gcc-9 \
	-DCMAKE_CXX_COMPILER=g++-9 \
	../../tmp
make V=1 --jobs=$(printf "%d\n4" $(nproc) | sort -n | head -1)

./CPVFrameworkTmp \
	--task-quota-ms=20 \
	--reactor-backend epoll

