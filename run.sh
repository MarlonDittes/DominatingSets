#!/bin/bash

solvers=("findminhs" "highs" "scip")
testset="testset"
time_limit=120

input_dir="./graphs/$testset"

for solver in "${solvers[@]}"; do
    output_csv="./graphs/$testset/$solver.csv"
    
    # Initialize CSV header based on the solver
    case "$solver" in
        "findminhs")
            echo "Name,Time Taken (seconds)" > "$output_csv"
            ;;
        "highs" | "scip")
            echo "Name,Solution Size,Time Taken (seconds)" > "$output_csv"
            ;;
        *)
            echo "ERROR: Unsupported solver $solver"
            continue
            ;;
    esac
    
    # Process each graph file for the current solver
    for graph_file in "$input_dir"/*.gr; do
        case "$solver" in
            "findminhs")
                time_taken=$( { time timeout $time_limit ./build/main "$graph_file" findminhs solution.json settings.json; } 2>&1 | grep real | awk '{split($2, a, /m|s/); print a[1]*60 + a[2]}')

                if [ "$(echo "$time_taken > $time_limit" | bc -l)" -eq 1 ]; then
                    time_taken=""
                fi

                echo "$(basename "$graph_file"),$time_taken" >> "$output_csv"
                ;;
            "highs" | "scip")
                ./runlim --time-limit=$time_limit ./build/main "$graph_file" "$solver" > temp.txt
                output=$(cat temp.txt)

                echo "$(basename "$graph_file"),$output" >> "$output_csv"
                rm -f temp.txt
                ;;
            *)
                echo "ERROR: Unsupported solver $solver"
                continue
                ;;
        esac
    done

    echo "Processing complete for $solver. Results saved to $output_csv"
done