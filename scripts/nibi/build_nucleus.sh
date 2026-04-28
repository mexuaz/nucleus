#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NUCLEUS_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SPARSEHASH_DIR="${SPARSEHASH_DIR:-$NUCLEUS_ROOT/../sparsehash/src}"
MAKE_JOBS="${MAKE_JOBS:-${SLURM_CPUS_PER_TASK:-$(nproc)}}"
DND_DIR="${DND_DIR:-$NUCLEUS_ROOT/dnd}"
BIN_DIR="$NUCLEUS_ROOT/bin"

load_first_module() {
  local module_name
  for module_name in "$@"; do
    if module load "$module_name" >/dev/null 2>&1; then
      return 0
    fi
  done
  return 1
}

if command -v module >/dev/null 2>&1; then
  module purge || true
  module spider gcc || true
  load_first_module stdenv/2023 StdEnv/2023 StdEnv/2020 || true
  load_first_module gcc/12.3.0 gcc/11.4.0 gcc/10.3.0 gcc || true
fi

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
