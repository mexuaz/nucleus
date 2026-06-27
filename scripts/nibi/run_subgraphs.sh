#!/usr/bin/env bash
#SBATCH --account=def-thomo
#SBATCH --job-name=SariyuceSubgraphs
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err
#SBATCH --time=4-22:00          # time (DD-HH:MM)
#SBATCH --nodes=1               # Forces the job to run on exactly one physical node
#SBATCH --ntasks=1              # Single task for the whole node
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=96      # nd/nucleus is sequential; we hold one socket (96 cores) only to claim its 384 GB of memory for the hierarchy on large graphs
#SBATCH --sockets-per-node=1    # Keep all cores on the same physical chip, To avoid Numa effects
#SBATCH --mem=384G        # Memory; On Nibi's base compute nodes, each of the two processors (sockets) has 96 cores and 384 GB of memory. The hierarchy/subgraph reporting is memory hungry on large graphs.

# CPU-only job: no GPU is requested, so this lands on Nibi's CPU compute nodes.

set -euo pipefail

REPO_ROOT="$HOME/scratch/gpu-nucleus"
SCRIPT_DIR="$REPO_ROOT/related/nucleus/scripts/nibi"

module --force purge
module load StdEnv/2023 gcc/12.3 openmpi/4.1.5 cmake/3.31.0 python/3.14.2

exec python3 "$SCRIPT_DIR/run_subgraphs.py" "$@"
