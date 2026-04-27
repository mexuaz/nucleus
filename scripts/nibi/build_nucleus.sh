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
  # Human-readable label printed to the user
  local component="$1"
  local tree_dir="$2"
  # If non-empty and not "0", build with DUMP_K
  local dump_k="${3:-}"
  # Name of the produced binary to locate. If omitted we use the component name.
  local source_binary_name="${4:-$component}"
  # Installed/output binary base name in $BIN_DIR (before optional _dump_k suffix).
  local output_binary_name="${5:-$source_binary_name}"
  # Optional subdirectory under $BIN_DIR to install into (e.g., pnd).
  local output_subdir="${6:-}"
  # Optional make target to build explicitly. Defaults to source binary name.
  local make_target="${7:-$source_binary_name}"
  local tree_name
  tree_name="$(basename "$tree_dir")"

  if [[ ! -d "$tree_dir" ]]; then
    echo "Skipping $component: missing directory $tree_dir" >&2
    return 0
  fi

  echo "Building $component in $tree_dir"
  if [[ -n "$dump_k" && "$dump_k" != "0" ]]; then
    # Ensure objects are rebuilt with DUMP_K defined
    make -C "$tree_dir" clean || true
    make -C "$tree_dir" -j"$MAKE_JOBS" INCLUDES="-I$SPARSEHASH_DIR" DUMP_K=yes "$make_target"
  else
    make -C "$tree_dir" -j"$MAKE_JOBS" INCLUDES="-I$SPARSEHASH_DIR" "$make_target"
  fi

  # Install/copy built binary into $BIN_DIR. Try a few common locations relative to the tree.
  local src=""
  # Prefer known output locations used by nd/bnd/pnd makefiles.
  if [[ -n "$source_binary_name" ]]; then
    if [[ -x "$NUCLEUS_ROOT/bin/$tree_name/$source_binary_name" && -f "$NUCLEUS_ROOT/bin/$tree_name/$source_binary_name" ]]; then
      src="$NUCLEUS_ROOT/bin/$tree_name/$source_binary_name"
    elif [[ -x "$tree_dir/bin/$tree_name/$source_binary_name" && -f "$tree_dir/bin/$tree_name/$source_binary_name" ]]; then
      src="$tree_dir/bin/$tree_name/$source_binary_name"
    elif [[ -x "$tree_dir/bin/$source_binary_name" && -f "$tree_dir/bin/$source_binary_name" ]]; then
      src="$tree_dir/bin/$source_binary_name"
    elif [[ -x "$tree_dir/$source_binary_name" && -f "$tree_dir/$source_binary_name" ]]; then
      src="$tree_dir/$source_binary_name"
    fi
  fi

  # Fallback: look for an executable matching the requested binary name under the tree.
  if [[ -z "${src}" && -n "$source_binary_name" ]]; then
    src=$(find "$tree_dir" -maxdepth 4 -type f -name "$source_binary_name" -executable -print -quit 2>/dev/null || true)
  fi

  if [[ -n "${src}" ]]; then
    local dest_name="$output_binary_name"
    local dest_dir="$BIN_DIR"
    if [[ -n "$dump_k" && "$dump_k" != "0" ]]; then
      dest_name="${output_binary_name}_dump_k"
    fi
    if [[ -n "$output_subdir" ]]; then
      dest_dir="$BIN_DIR/$output_subdir"
    fi
    local dest_path="$dest_dir/$dest_name"
    echo "Installing $src -> $dest_path"
    mkdir -p "$dest_dir"
    if [[ -e "$dest_path" ]] && [[ "$(readlink -f "$src")" == "$(readlink -f "$dest_path")" ]]; then
      echo "Already installed: $dest_path"
    else
      cp "$src" "$dest_path"
      chmod +x "$dest_path" || true
    fi
  else
    echo "Error: could not find built binary for $component in $tree_dir (expected name: $source_binary_name)" >&2
    return 1
  fi
}

build_tree nd "$NUCLEUS_ROOT/nd" "" nucleus nd nd nucleus
build_tree nd_dump_k "$NUCLEUS_ROOT/nd" "1" nucleus nd nd nucleus
build_tree bnd "$NUCLEUS_ROOT/bnd" "" binucleus bnd bnd binucleus
build_tree pnd "$NUCLEUS_ROOT/pnd" "" pnd pnd pnd pnd
build_tree pnd_dump_k "$NUCLEUS_ROOT/pnd" "1" pnd pnd pnd pnd
build_tree dnd "$DND_DIR" "" dnd dnd dnd dnd
