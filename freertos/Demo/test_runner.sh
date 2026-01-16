#!/bin/bash

# --- CONFIGURATION ---
TEST_CASES=(1 2 3 4 5 7 8)
SLEEP_SEC=4
GOLDEN_DIR="./tests/golden"
OUTPUT_DIR="./output"
ELF_FILE="./Output/demo.elf"

BASE_QEMU_CMD="qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel $ELF_FILE -monitor none -nographic -serial stdio -semihosting"

mkdir -p $GOLDEN_DIR
mkdir -p $OUTPUT_DIR

sanitize_log() {
    # sed to rmv time stamp then head to keep first lines only
    grep "\[TRACE\]" "$1" | sed 's/\[TRACE\] [0-9]*:/\[TRACE\]:/g' | head -n 30 > "$2"
}

echo "    STARTING FAST TEST RUN"

echo "Compiling Global Binary..."
rm -f $ELF_FILE
make cleanobj > /dev/null 2>&1
make > /dev/null 2>&1 

if [ ! -f "$ELF_FILE" ]; then
    echo "❌ COMPILE FAILED"
    exit 1
fi

fails=0

for T in "${TEST_CASES[@]}"
do
    echo -n "Test Case $T: "

    
    FULL_CMD="$BASE_QEMU_CMD -semihosting-config enable=on,target=native,arg=TEST=$T"

    # Run QEMU in the background using &
    $FULL_CMD > "$OUTPUT_DIR/raw_$T.log" 2>&1 &
    
    QEMU_PID=$!
    
    sleep $SLEEP_SEC
    
    kill $QEMU_PID > /dev/null 2>&1
    wait $QEMU_PID 2>/dev/null

    if [ ! -s "$OUTPUT_DIR/raw_$T.log" ]; then
        echo "⚠️  EMPTY LOG (Still empty?)"
        fails=$((fails+1))
        continue
    fi

    sanitize_log "$OUTPUT_DIR/raw_$T.log" "$OUTPUT_DIR/actual_$T.txt"
    GOLDEN_FILE="$GOLDEN_DIR/test_$T.txt"

    if [ ! -f "$GOLDEN_FILE" ]; then
        echo "⚠️  NO GOLDEN FILE (Generated: $OUTPUT_DIR/actual_$T.txt)"
    else
        if diff -q -w "$OUTPUT_DIR/actual_$T.txt" "$GOLDEN_FILE" > /dev/null; then
            echo "✅ PASS"
        else
            echo "❌ FAIL"
            diff -y --suppress-common-lines "$GOLDEN_FILE" "$OUTPUT_DIR/actual_$T.txt" | head -n 5
            fails=$((fails+1))
        fi
    fi
done

echo "========================================"
exit $fails