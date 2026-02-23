#!/bin/bash

GOLDEN_TESTS=(1 2 3 4 5 6 7 8 9 10 11 19)
STRESS_TESTS=(12 21 22 23 24 25 26 27 28 29)
# GOLDEN_TESTS=(12)
# STRESS_TESTS=(22 29)

SLEEP_SEC=4
GOLDEN_DIR="./tests/golden"
OUTPUT_DIR="./output"
ELF_FILE="./Output/demo.elf"

BASE_QEMU_CMD="qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel $ELF_FILE -monitor none -nographic -serial stdio -semihosting"

mkdir -p $GOLDEN_DIR
mkdir -p $OUTPUT_DIR


sanitize_log() {
    # Removes timestamps to make output deterministic for Golden Files comparion
    grep "\[TRACE\]" "$1" | sed 's/\[TRACE\] [0-9]*:/\[TRACE\]:/g' | head -n 30 > "$2"
}

check_stress_log() {
    local LOG_FILE="$1"
    local FAIL_FOUND=0
    
    echo "[Failure scan]"

    if grep -qE "ASSERT|ERROR|Fault" "$LOG_FILE"; then
        echo "❌ System Crash or Assertion Fail"
        grep -E "ASSERT|ERROR|Fault" "$LOG_FILE"
        return 1
    fi

    #maybe allow overruns
    if grep -qE "MISS|OVERRUN" "$LOG_FILE"; then
        echo "❌ FAIL: Deadline Miss or Overrun Detected:"
        grep -E "MISS|OVERRUN" "$LOG_FILE" | head -n 5
        return 1
    fi

    START_COUNT=$(grep -c ":START" "$LOG_FILE")
    END_COUNT=$(grep -c ":END" "$LOG_FILE")


    if [ "$START_COUNT" -eq 0 ]; then
        echo "❌ No tasks started"
        return 1
    fi

    #ADJUST
    if [ "$END_COUNT" -lt 2 ]; then
        echo "❌ not enough tasks ended, if tasks have long duration the value can be adjusted ctrl F "ADJUST" in script and adjust "
        return 1
    fi

    echo "✅ PASS (0 Misses, $END_COUNT Jobs Completed)"

    echo "✅ PASS"
    return 0
}

compile_project() {
    echo "Compiling Project..."
    rm -f $ELF_FILE
    make cleanobj > /dev/null 2>&1
    make > /dev/null 2>&1 

    if [ ! -f "$ELF_FILE" ]; then
        echo "❌ COMPILE FAILED"
        exit 1
    fi
}

run_qemu() {
    local T="$1"
    FULL_CMD="$BASE_QEMU_CMD -semihosting-config enable=on,target=native,arg=TEST=$T"
    
    # Run QEMU in background
    $FULL_CMD > "$OUTPUT_DIR/raw_$T.log" 2>&1 &
    QEMU_PID=$!
    
    sleep $SLEEP_SEC
    
    kill $QEMU_PID > /dev/null 2>&1
    wait $QEMU_PID 2>/dev/null
}


MODE="golden"
TARGET_TEST=0

if [ "$1" == "stress" ]; then
    MODE="stress"
    TARGET_TEST=$2
    
fi

compile_project

if [ "$MODE" == "stress" ]; then
    # --- STRESS MODE (Logic Check Only) ---
    if [ -z "$TARGET_TEST" ]; then
        # Run all stress tests
        echo "========================================"
        echo " RUNNING ALL STRESS TESTS"
        echo "========================================"
        
        fails=0
        for T in "${STRESS_TESTS[@]}"
        do
            echo -n "Test Case $T: "
            run_qemu $T
            
            if [ ! -s "$OUTPUT_DIR/raw_$T.log" ]; then
                echo "⚠️  EMPTY LOG (QEMU Failed to run?)"
                fails=$((fails+1))
                continue
            fi

            check_stress_log "$OUTPUT_DIR/raw_$T.log"
            if [ $? -ne 0 ]; then
                fails=$((fails+1))
            fi
        done
        
        echo "========================================"
        if [ $fails -eq 0 ]; then
            echo "ALL STRESS TESTS PASSED"
        else
            echo "$fails STRESS TESTS FAILED"
        fi
        exit $fails
    else
        # Run single stress test
        echo "========================================"
        echo " RUNNING STRESS TEST CASE: $TARGET_TEST"
        echo "========================================"
        
        run_qemu $TARGET_TEST
        
        if [ ! -s "$OUTPUT_DIR/raw_$TARGET_TEST.log" ]; then
            echo "⚠️  EMPTY LOG (QEMU Failed to run?)"
            exit 1
        fi

        check_stress_log "$OUTPUT_DIR/raw_$TARGET_TEST.log"
        exit $?
    fi

else
    echo "========================================"
    echo " RUNNING GOLDEN TESTS"
    echo "========================================"
    
    fails=0
    for T in "${GOLDEN_TESTS[@]}"
    do
        echo -n "Test Case $T: "
        run_qemu $T

        if [ ! -s "$OUTPUT_DIR/raw_$T.log" ]; then
            echo "⚠️  EMPTY LOG"
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
                echo "❌ FAIL (Output Changed)"
                # showw diff
                diff -y --suppress-common-lines "$GOLDEN_FILE" "$OUTPUT_DIR/actual_$T.txt" | head -n 3
                fails=$((fails+1))
            fi
        fi
    done
    
    echo "========================================"
    if [ $fails -eq 0 ]; then
        echo "ALL PASSED"
    else
        echo "$fails TESTS FAILED"
    fi
    exit $fails
fi