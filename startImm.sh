#!/bin/bash

SERVER_BINARY=./build/server
INSTANCES=$(ls -d $PWD/models/projectinstances/*.xml)

${SERVER_BINARY} models/types/types.xml models/instances/imm.xml models/units/units.xml models/projecttypes/immtypes.xml ${INSTANCES}