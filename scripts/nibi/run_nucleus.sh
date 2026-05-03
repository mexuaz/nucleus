#!/usr/bin/env bash
#SBATCH --account=def-thomo
#SBATCH --job-name=Sariyuce
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err
#SBATCH --time=0-09:00          # time (DD-HH:MM)
#SBATCH --nodes=1               # Forces the job to run on exactly one physical node
#SBATCH --ntasks=1              # Single task for the whole node
#SBATCH --cpus-per-task=96      # Nibi CPU nodes have 192 cores, we use one socket with 96 cores, so we set this to 96. The script will use fewer cores if specified by the --cores argument.
#SBATCH --sockets-per-node=1    # Keep all cores on the same physical chip, To avoid Numa effects
#SBATCH --mem=384G              # There are 4G per CPU so totaling 384G

module --force purge; module load StdEnv/2023 gcc/12.3 openmpi/4.1.5 cmake/3.31.0 python/3.14.2

# Usage: run_nucleus.sh [OPTIONS] [INDEX]
# Options:
#   --program PROGRAM:MODE      Run program with mode (can be specified multiple times or comma-separated default: pnd:341)
#   --cores CORES               Comma-separated core counts (default: 1,2,4,8,16,24,32,48,64,80,96)
#
# Modes:
#   For nd:  12, 13, 14, 23, 24, 34
#   For pnd: 120, 1200, 230, 2300, 340, 3400, 341, 734, 342, 34000, 340000 etc.
#
# Examples:
#   Run nd with modes 34, 23, 12 for all large datasets:
#     run_nucleus.sh --program nd:34 --program nd:23 --program nd:12 large
#
#   Run pnd with modes 341, 340 for all large datasets:
#     run_nucleus.sh --program pnd:341,pnd:340 large
#
#   Run both nd and pnd with multiple modes across specific cores:
#     run_nucleus.sh --cores "1,4,16,64" --program nd:34,nd:23,nd:12 --program pnd:341,pnd:340 large
#
#   Run on sample dataset:
#     run_nucleus.sh --program nd:34 --program pnd:341 sample 1

# Job for submitting to Compute Canada Slurm Scheduler from RESULTS/ncx directory
# sbatch ../../related/nucleus/scripts/nibi/run_nucleus.sh --program pnd:341,pnd:340,nd:34 --cores 1,4,16,32,64,96 large

SCRIPT_DIR="$HOME/scratch/gpu-nucleus/related/nucleus/scripts/nibi"
exec python3 "$SCRIPT_DIR/run.py" "$@"

