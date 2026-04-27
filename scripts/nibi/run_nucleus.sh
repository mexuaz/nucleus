#!/usr/bin/env bash
#SBATCH --account=def-thomo
#SBATCH --job-name=JeShiNucleusAll
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err
#SBATCH --time=0-03:00          # time (DD-HH:MM)
#SBATCH --nodes=1               # Forces the job to run on exactly one physical node
#SBATCH --ntasks=1              # Single task for the whole node
#SBATCH --cpus-per-task=56      # Nibi H100 nodes (g*) have 112 CPUs total: 2 sockets x 56 cores x 1 thread
#SBATCH --mem=224G              # There are 4G per CPU so totaling 224G

module --force purge; module load StdEnv/2023 gcc/12.3 openmpi/4.1.5 cmake/3.31.0 python/3.14.2

# "Usage: run_nucleus.sh [--program PROGRAM] [--cores CORES] [--program-args ARGS] [sample|s [index]|large|l [index]|<large-index>]"
# Examples:
# run_nucleus.sh --program pnd --program-args 341
# run_nucleus.sh --program nd

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec python3 "$SCRIPT_DIR/run.py" "$@"