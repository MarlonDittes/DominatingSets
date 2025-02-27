#!/bin/bash

solvers=("scip")
testset="ds_exact"
time_limit=1800
seed=1

input_dir="./graphs/$testset"
num_cores=2  # Detect number of available CPU cores

process_graph() {
    local solver="$1"
    local graph_file="$2"
    local output_csv="./graphs/$testset/${solver}.csv"
    local graph_name
    graph_name=$(basename "$graph_file")

    case "$solver" in
        "findminhs")
            time_taken=$( { time timeout "$time_limit" ./build/main "$graph_file" findminhs solution.json settings.json; } 2>&1 | grep real | awk '{split($2, a, /m|s/); print a[1]*60 + a[2]}')

            if [ "$(echo "$time_taken > $time_limit" | bc -l)" -eq 1 ]; then
                time_taken=""
            fi

            echo "$graph_name,$time_taken" >> "$output_csv"
            ;;
        "highs" | "scip" | "lp")
            ./runlim --time-limit="$time_limit" ./build/main "$graph_file" "$solver" > "temp_${graph_name}.txt"
            output=$(cat "temp_${graph_name}.txt")
            rm -f "temp_${graph_name}.txt"

            echo "$graph_name,$output" >> "$output_csv"
            ;;
        "domsat")
            ./build/main "$graph_file" "$solver" "$time_limit" > "temp_${graph_name}.txt"
            read solution_size time_taken < <(tac "temp_${graph_name}.txt" | grep '^o' | head -n 1 | awk '{print $2, $3}')
            rm -f "temp_${graph_name}.txt"

            echo "$graph_name,$solution_size,$time_taken" >> "$output_csv"
            ;;
        "nusc")
            ./build/main "$graph_file" "$solver" "$time_limit" "$seed" > "temp_${graph_name}.txt"
            read solution_size time_taken < <(tac "temp_${graph_name}.txt" | grep '^o' | head -n 1 | awk '{print $2, $3}')
            rm -f "temp_${graph_name}.txt"

            echo "$graph_name,$solution_size,$time_taken" >> "$output_csv"
            ;;
        *)
            echo "ERROR: Unsupported solver $solver"
            exit 1
            ;;
    esac
}

export -f process_graph  # Export function for parallel execution
export time_limit  # Ensure time_limit is available for subprocesses

for solver in "${solvers[@]}"; do
    output_csv="./graphs/$testset/${solver}.csv"

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

    # Use xargs correctly, ensuring variables expand properly
    find "$input_dir" -name "*.gr" -print0 | xargs -0 -P "$num_cores" -I{} bash -c 'process_graph "'"$solver"'" "{}"'

    echo "Processing complete for $solver. Results saved to $output_csv"
done
