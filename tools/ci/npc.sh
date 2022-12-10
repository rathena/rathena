#!/usr/bin/env bash

out=npc/scripts_custom.conf

printf "\n" >> $out
echo "// Custom Scripts" >> $out

find npc/custom \( -name "*.txt" \) | sort | xargs -I % echo "npc: %" >> $out

echo "// Test Scripts" >> $out

find npc/test \( -name "*.txt" \) | sort | xargs -I % echo "npc: %" >> $out
