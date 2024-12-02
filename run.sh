#!/bin/bash

solver="scip"

input_dir="./graphs/testset"
output_csv="./graphs/testset/$solver.csv"

echo "Name,Time Taken (seconds)" > "$output_csv"

for graph_file in "$input_dir"/*.gr; do
    case "$solver" in 
    "findminhs")
        time_taken=$( { time ./build/main "$graph_file" findminhs solution.json settings.json; } 2>&1 | grep real | awk '{print $2}' | sed 's/[^0-9.]//g')
        ;;
    "highs")
        time_taken=$( { time ./build/main "$graph_file" highs; } 2>&1 | grep real | awk '{print $2}' | sed 's/[^0-9.]//g')
        ;;
    "scip")
        time_taken=$( { time ./build/main "$graph_file" scip; } 2>&1 | grep real | awk '{print $2}' | sed 's/[^0-9.]//g')
        ;;
    *)
        time_taken="ERROR"
        ;;
    esac

    echo "$(basename "$graph_file"),$time_taken" >> "$output_csv"
done

echo "Processing complete. Results saved to $output_csv"
