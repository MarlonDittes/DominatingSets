#!/bin/bash

#solvers=("findminhs" "highs" "scip")
solvers=("scip" "highs")
testset="testset"
time_limit=120
seed=1

input_dir="./graphs/$testset"

for solver in "${solvers[@]}"; do
    output_csv="./graphs/$testset/${solver}_reduced.csv"
    
    # Initialize CSV header based on the solver
    case "$solver" in
        "findminhs")
            echo "Name,Time Taken (seconds)" > "$output_csv"
            ;;
        "highs" | "scip" | "lp" | "domsat" | "nusc")
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
            "highs" | "scip" | "lp")
                ./runlim --time-limit=$time_limit ./build/main "$graph_file" "$solver" > temp.txt
                output=$(cat temp.txt)

                echo "$(basename "$graph_file"),$output" >> "$output_csv"
                rm -f temp.txt
                ;;
            "domsat")
                ./build/main "$graph_file" "$solver" "$time_limit" > temp.txt

                read solution_size time_taken < <(tac temp.txt | grep '^o' | head -n 1 | awk '{print $2, $3}')

                echo "$(basename "$graph_file"),$solution_size,$time_taken" >> "$output_csv"
                rm -f temp.txt
                ;;
            "nusc")
                ./build/main "$graph_file" "$solver" "$time_limit" "$seed"> temp.txt

                read solution_size time_taken < <(tac temp.txt | grep '^o' | head -n 1 | awk '{print $2, $3}')

                echo "$(basename "$graph_file"),$solution_size,$time_taken" >> "$output_csv"
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