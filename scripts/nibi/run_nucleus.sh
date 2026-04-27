#!/usr/bin/env bash
#SBATCH --account=def-thomo
#SBATCH --job-name=Sariyuce
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err
#SBATCH --time=1-09:00          # time (DD-HH:MM)
#SBATCH --nodes=1               # Forces the job to run on exactly one physical node
#SBATCH --ntasks=1              # Single task for the whole node
#SBATCH --cpus-per-task=96      # Nibi CPU nodes have 192 cores, we use one socket with 96 cores, so we set this to 96. The script will use fewer cores if specified by the --cores argument.
#SBATCH --sockets-per-node=1    # Keep all cores on the same physical chip, To avoid Numa effects
#SBATCH --mem=384G              # There are 4G per CPU so totaling 384G

module --force purge
module load StdEnv/2023 gcc/12.3 openmpi/4.1.5 cmake/3.31.0 python/3.14.2

SCRIPT_DIR="$HOME/scratch/gpu-nucleus/related/nucleus/scripts/nibi"
exec python3 "$SCRIPT_DIR/run.py" "$@"