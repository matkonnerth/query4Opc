#!/bin/bash

SERVER_BINARY=./build/csvExport/exportCsv
INSTANCES=$(ls -d $PWD/models/projectinstances/*.xml)
NAMESPACE0=/home/matzy/git/nodesetLoader/nodesets/Opc.Ua.NodeSet2.xml

${SERVER_BINARY} ${NAMESPACE0} models/types/types.xml models/instances/imm.xml models/units/units.xml models/projecttypes/immtypes.xml ${INSTANCES}