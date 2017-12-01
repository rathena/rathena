#!/usr/bin/env bash

out=../../npc/scripts_custom.conf

printf "\n" >> $out
echo "// Custom Scripts" >> $out

find ../../npc/ \( -wholename "*npc/custom/*.txt" -and -not -wholename "*npc/custom/battleground/*" \) | cut -c 7- | xargs -I % echo "npc: %" >> $out

echo "// Test Scripts" >> $out

find ../../npc/ \( -wholename "*npc/test/*.txt" \) | cut -c 7- | xargs -I % echo "npc: %" >> $out
