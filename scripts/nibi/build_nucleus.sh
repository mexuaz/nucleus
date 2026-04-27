#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NUCLEUS_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SPARSEHASH_DIR="${SPARSEHASH_DIR:-$NUCLEUS_ROOT/../sparsehash/src}"
MAKE_JOBS="${MAKE_JOBS:-${SLURM_CPUS_PER_TASK:-$(nproc)}}"
DND_DIR="${DND_DIR:-$NUCLEUS_ROOT/dnd}"
BIN_DIR="$NUCLEUS_ROOT/bin"

module --force purge;module load StdEnv/2023 gcc/12.3 openmpi/4.1.5 cmake/3.31.0 python/3.14.2

if [[ ! -d "$SPARSEHASH_DIR" ]]; then
  echo "Sparsehash headers not found at: $SPARSEHASH_DIR" >&2
  echo "Set SPARSEHASH_DIR to the sparsehash src directory before building." >&2
  exit 1
fi

echo "Using sparsehash: $SPARSEHASH_DIR"
echo "Using dnd path: $DND_DIR"

mkdir -p "$BIN_DIR"
echo "Using bin directory: $BIN_DIR"

build_tree() {
  local label="$1"
  local tree_dir="$2"

  if [[ ! -d "$tree_dir" ]]; then
    echo "Skipping $label: missing directory $tree_dir" >&2
    return 0
  fi

  echo "Building $label in $tree_dir"
  make -C "$tree_dir" -j"$MAKE_JOBS" INCLUDES="-I$SPARSEHASH_DIR"
}

build_tree nd "$NUCLEUS_ROOT/nd"
build_tree bnd "$NUCLEUS_ROOT/bnd"
build_tree pnd "$NUCLEUS_ROOT/pnd"
build_tree dnd "$DND_DIR"
