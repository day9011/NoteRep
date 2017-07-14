#!/bin/bash

command_dir=`pwd`
value=$1

echo "call test_input"
${command_dir}/test_input << EOF
${value}
EOF
