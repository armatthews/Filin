cat test_positions.txt | sed 's/#.*$//' | sed '/^\s*$/d' | sed 's/\s*perft\s*/\t/;s/ = /\t/' | while read -r line; do pos=$(echo "$line" | cut -f 1); depth=$(echo "$line" | cut -f 2); expected=$(echo "$line" | cut -f 3); echo $pos; actual=$(echo "setboard $pos
perft $depth
quit" | bin/filin 2>/dev/null | grep 'Perft' | cut -f 3 -d ' '); echo -n "$actual	$expected	"; if [[ $actual == $expected ]]; then echo "PASS"; else echo "FAIL"; fi; done
