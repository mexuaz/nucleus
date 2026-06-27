#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
WORK_DIR="${REPO_DIR}/subgraph_outputs"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

sbatch "$SCRIPT_DIR/run_subgraphs.sh" 1 --tools kcore --output-dir "$WORK_DIR/d1_12"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 2 --tools kcore --output-dir "$WORK_DIR/d2_12"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 3 --tools kcore --output-dir "$WORK_DIR/d3_12"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 4 --tools kcore --output-dir "$WORK_DIR/d4_12"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 5 --tools kcore --output-dir "$WORK_DIR/d5_12"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 6 --tools kcore --output-dir "$WORK_DIR/d6_12"


sbatch "$SCRIPT_DIR/run_subgraphs.sh" 1 --tools ktruss --output-dir "$WORK_DIR/d1_23"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 2 --tools ktruss --output-dir "$WORK_DIR/d2_23"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 3 --tools ktruss --output-dir "$WORK_DIR/d3_23"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 4 --tools ktruss --output-dir "$WORK_DIR/d4_23"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 5 --tools ktruss --output-dir "$WORK_DIR/d5_23"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 6 --tools ktruss --output-dir "$WORK_DIR/d6_23"


sbatch "$SCRIPT_DIR/run_subgraphs.sh" 1 --tools nucleus34 --output-dir "$WORK_DIR/d1_34"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 2 --tools nucleus34 --output-dir "$WORK_DIR/d2_34"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 3 --tools nucleus34 --output-dir "$WORK_DIR/d3_34"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 4 --tools nucleus34 --output-dir "$WORK_DIR/d4_34"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 5 --tools nucleus34 --output-dir "$WORK_DIR/d5_34"
sbatch "$SCRIPT_DIR/run_subgraphs.sh" 6 --tools nucleus34 --output-dir "$WORK_DIR/d6_34"
