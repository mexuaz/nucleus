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

set -euo pipefail

REPO_ROOT="$HOME/scratch/gpu-nucleus"
SCRIPT_DIR="$REPO_ROOT/related/nucleus/scripts/nibi"
BIN_DIR="$REPO_ROOT/related/nucleus/bin"
TARGET="${1:-}"

LARGE_DATASETS=(
  "amazon-2008.mtx" # 1
  "soc-Slashdot0811.mtx" # 2
  "soc-FourSquare.mtx" # 3
  "cit-patent.edges" # 4
  "soc-BlogCatalog.mtx" # 5
  "soc-LiveJournal1.edges" # 6
)

usage() {
  cat <<'EOF'
Usage: sbatch run_nucleus_all.sbatch <target> [target-args...]

If <target> is omitted, the script runs the large-dataset sweep for nd, bnd,
and pnd.

Targets:
  nd    -> bin/nd/nucleus
  bnd   -> bin/bnd/binucleus -> <RIGHT_TIP, LEFT_TIP and WING for bipartite graphs; RUN_WEIGHTED_CORE, RUN_(CORE|TRUSS) for the projections>
  pnd   -> bin/pnd/pnd
  dnd   -> bin/dnd (only if built separately)
EOF
}


run_single_target() {
  local target_name="$1"
  shift

  local runner
  case "$target_name" in
    nd)
      runner="$BIN_DIR/nd/nucleus"
      ;;
    bnd)
      runner="$BIN_DIR/bnd/binucleus"
      ;;
    pnd)
      runner="$BIN_DIR/pnd/pnd"
      ;;
    dnd)
      runner="$BIN_DIR/dnd"
      ;;
    *)
      echo "Unknown target: $target_name" >&2
      usage >&2
      return 1
      ;;
  esac

  if [[ ! -x "$runner" ]]; then
    echo "Runner not found or not executable: $runner" >&2
    return 1
  fi

  srun --cpu-bind=cores "$runner" "$@"

}

if [[ -z "$TARGET" ]]; then
  sweep_status=0
  for dataset in "${LARGE_DATASETS[@]}"; do
    echo "Running nd for dataset: $dataset"
    run_single_target nd "$dataset" 34 YES || sweep_status=$?
    # echo "Running bnd for dataset: $dataset"
    # run_single_target bnd "$dataset" WING YES || sweep_status=$?
    echo "Running pnd for dataset: $dataset"
    run_single_target pnd "$dataset" 340 || sweep_status=$?
  done

  exit "$sweep_status"
fi

shift || true
run_single_target "$TARGET" "$@"
exit $?