#!/bin/sh
cat << EOF
%!PS-Adobe-3.0
vmstatus pop /vmstart exch def pop
EOF
cat calchart.ps
cat << EOF
vmstatus pop dup vmstart sub (Max: ) print == flush
/vmstart exch def pop
EOF
cat calchart.ps
cat << EOF
vmstatus pop vmstart sub (Min: ) print == flush pop
EOF
