#!/bin/bash

## ��ǰ����·�� ##
CUR_DIR_ABSLOTE=`pwd`

## ��ǰ���·�� ##
CUR_DIR_RELATIVE=`dirname $0`

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../${CUR_DIR_RELATIVE}/bin/

cd bin
./output/basicsrv