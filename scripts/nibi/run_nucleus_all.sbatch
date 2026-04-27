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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NUCLEUS_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN_DIR="$NUCLEUS_ROOT/bin"
RESULTS_ROOT="${SLURM_SUBMIT_DIR:-$NUCLEUS_ROOT}/results"
RESULTS_DIR="$RESULTS_ROOT/${SLURM_JOB_NAME:-JeShiNucleusAll}-${SLURM_JOB_ID:-manual}"
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
  bnd   -> bin/bnd/binucleus
  pnd   -> bin/pnd/pnd
  dnd   -> bin/dnd (only if built separately)
EOF
}

mkdir -p "$RESULTS_DIR"

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

  local timestamp
  timestamp="$(date +%Y%m%dT%H%M%S)"
  local result_file="$RESULTS_DIR/${target_name}-${timestamp}.log"

  set +e
  srun --cpu-bind=cores "$runner" "$@" | tee "$result_file"
  local run_status=${PIPESTATUS[0]}
  local tee_status=${PIPESTATUS[1]}
  set -e

  if [[ $run_status -ne 0 ]]; then
    return "$run_status"
  fi

  return "$tee_status"
}

if [[ -z "$TARGET" ]]; then
  sweep_status=0
  for dataset in "${LARGE_DATASETS[@]}"; do
    run_single_target nd "$dataset" 34 YES || sweep_status=$?
    run_single_target bnd "$dataset" 34 YES || sweep_status=$?
    run_single_target pnd "$dataset" 340 || sweep_status=$?
  done

  exit "$sweep_status"
fi

shift || true
run_single_target "$TARGET" "$@"
exit $?