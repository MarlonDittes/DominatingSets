#!/bin/bash

solvers=("highs")
testset="ds_exact"
time_limit=1800
seed=1
start=14

input_dir="./graphs/$testset"

for solver in "${solvers[@]}"; do
    output_csv="./results/$testset/${solver}_reduced.csv"
    
    # Initialize CSV header based on the solver
    case "$solver" in
        "findminhs")
            echo "Name,Time Taken (seconds)" > "$output_csv"
            ;;
        "highs" | "scip" | "lp")
            echo "Name,Solution Size,Time Taken (seconds)" > "$output_csv"
            ;;
        "domsat" | "nusc")
            echo "Name,Solution Size,Time Taken (seconds), Exec Time (seconds)" > "$output_csv"
            ;;
        *)
            echo "ERROR: Unsupported solver $solver"
            continue
            ;;
    esac
    
    # Process each graph file for the current solver
    for graph_file in "$input_dir"/*.gr; do
        graph_number=$(basename "$graph_file" | grep -o '[0-9]\+' | head -n 1)
        if [ "$graph_number" -lt $start ]; then
            continue
        fi
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
                #./build/main "$graph_file" "$solver" "$time_limit" > temp.txt

                #read solution_size time_taken < <(tac temp.txt | grep '^o' | head -n 1 | awk '{print $2, $3}')

                #echo "$(basename "$graph_file"),$solution_size,$time_taken" >> "$output_csv"
                #rm -f temp.txt
                #;;
                { time ./build/main "$graph_file" "$solver" "$time_limit"; } 2> time_log.txt | tee temp.txt
                
                read solution_size time_taken < <(tac temp.txt | grep '^o' | head -n 1 | awk '{print $2, $3}')
                exec_time=$(grep real time_log.txt | awk '{split($2, a, /m|s/); print a[1]*60 + a[2]}')

                echo "$(basename "$graph_file"),$solution_size,$time_taken,$exec_time" >> "$output_csv"
                rm -f temp.txt time_log.txt
                ;;

            "nusc")
                #./build/main "$graph_file" "$solver" "$time_limit" "$seed"> temp.txt

                #read solution_size time_taken < <(tac temp.txt | grep '^o' | head -n 1 | awk '{print $2, $3}')

                #echo "$(basename "$graph_file"),$solution_size,$time_taken" >> "$output_csv"
                #rm -f temp.txt
                #;;
                { time ./build/main "$graph_file" "$solver" "$time_limit" "$seed"; } 2> time_log.txt | tee temp.txt
                
                read solution_size time_taken < <(tac temp.txt | grep '^o' | head -n 1 | awk '{print $2, $3}')
                exec_time=$(grep real time_log.txt | awk '{split($2, a, /m|s/); print a[1]*60 + a[2]}')

                echo "$(basename "$graph_file"),$solution_size,$time_taken,$exec_time" >> "$output_csv"
                rm -f temp.txt time_log.txt
                ;;
            *)
                echo "ERROR: Unsupported solver $solver"
                continue
                ;;
        esac
    done

    echo "Processing complete for $solver. Results saved to $output_csv"
done