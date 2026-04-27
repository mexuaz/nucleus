#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parents[3]  # nibi -> scripts -> nucleus -> related -> repo root
DATASET_DIR = REPO_ROOT / "src" / "build" / "datasets"
BIN_DIR = SCRIPT_DIR.parents[1] / "related" / "nucleus" / "bin"

LARGE_DATASETS = [
    "amazon-2008.mtx",         # 1
    "soc-Slashdot0811.mtx",    # 2
    "soc-FourSquare.mtx",      # 3
    "cit-patent.edges",        # 4
    "soc-BlogCatalog.mtx",     # 5
    "soc-digg.mtx",            # 6
    "soc-LiveJournal1.edges",  # 7
]

PROGRAM_PATHS = {
    "nd":  BIN_DIR / "nd"  / "nucleus",
    "bnd": BIN_DIR / "bnd" / "binucleus",
    "pnd": BIN_DIR / "pnd" / "pnd",
    "dnd": BIN_DIR / "dnd",
}


def build_env() -> dict[str, str]:
    env = os.environ.copy()
    return env


def run_timestamp() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="microseconds")


def resolve_program(program_name: str) -> Path:
    program_path = PROGRAM_PATHS.get(program_name)
    if program_path is not None:
        return program_path

    candidate = Path(program_name)
    if candidate.exists():
        return candidate

    available = ", ".join(sorted(PROGRAM_PATHS))
    raise SystemExit(f"Unknown program: {program_name}. Available programs: {available}")


def run_program(
    program_name: str,
    dataset_path: Path,
    program_args: list[str],
    core_counts: list[int],
) -> tuple[int, dict[str, object]]:
    program_path = resolve_program(program_name)

    if not program_path.exists():
        print(f"ERROR: Program not found: {program_path}", file=sys.stderr)
        return 1, {}

    if not dataset_path.exists():
        print(f"ERROR: Dataset not found: {dataset_path}", file=sys.stderr)
        return 1, {}

    allocated_cpus = int(os.environ.get("SLURM_CPUS_PER_TASK", os.cpu_count() or 1))
    outputs: dict[str, object] = {}

    for n in core_counts:
        if n > allocated_cpus:
            print(
                f"WARNING: Skipping core count {n} (exceeds allocated {allocated_cpus} CPUs)",
                file=sys.stderr,
            )
            continue

        timestamp = run_timestamp()

        srun_command = [
            "srun",
            "--ntasks=1",
            f"--cpus-per-task={n}",
            "--cpu-bind=cores",
            "--mem=0",
            "--exact",
            str(program_path),
            str(dataset_path),
            *program_args,
        ]

        completed = subprocess.run(
            srun_command,
            env=build_env(),
            capture_output=True,
            text=True,
        )

        if completed.stderr:
            print(completed.stderr, file=sys.stderr, end="")

        if completed.returncode != 0:
            return completed.returncode, outputs

        try:
            outputs[timestamp] = json.loads(completed.stdout)
        except json.JSONDecodeError as exc:
            print(
                f"ERROR: Program output is not valid JSON for {dataset_path} (cores={n}): {exc}",
                file=sys.stderr,
            )
            if completed.stdout:
                print(completed.stdout, file=sys.stderr, end="")
            return 1, outputs

    return 0, outputs


def run_sample(
    program_name: str,
    graph_num: str,
    program_args: list[str],
) -> tuple[int, dict[str, object]]:
    dataset_path = REPO_ROOT / "datasets" / "tests" / f"sample_input_graph_{graph_num}.mtx"
    return run_program(program_name, dataset_path, program_args, core_counts=[1])


def run_large(
    program_name: str,
    dataset_index: str,
    core_counts: list[int],
    program_args: list[str],
) -> tuple[int, dict[str, object]]:
    aggregate_output: dict[str, object] = {}

    if dataset_index != "0":
        index = int(dataset_index)
        if index < 1 or index > len(LARGE_DATASETS):
            print(
                f"Invalid large dataset index: {index} (valid range: 1-{len(LARGE_DATASETS)})",
                file=sys.stderr,
            )
            return 1, aggregate_output

        dataset_name = LARGE_DATASETS[index - 1]
        dataset_path = DATASET_DIR / dataset_name
        exit_code, output = run_program(program_name, dataset_path, program_args, core_counts)
        aggregate_output.update(output)
        return exit_code, aggregate_output

    exit_code = 0
    for dataset_name in LARGE_DATASETS:
        dataset_path = DATASET_DIR / dataset_name
        result_code, output = run_program(program_name, dataset_path, program_args, core_counts)
        exit_code = exit_code or result_code
        aggregate_output.update(output)

    return exit_code, aggregate_output


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run a nucleus decomposition program on sample or large datasets."
    )
    parser.add_argument(
        "--program",
        default="pnd",
        help=(
            "Program to execute. One of: nd, bnd, pnd, dnd, or an executable path."
        ),
    )
    parser.add_argument(
        "--cores",
        default="1,2,4,8,16,32,64,96,192",
        help="Comma-separated list of core counts for the sweep.",
    )
    parser.add_argument(
        "--program-args",
        default="",
        help=(
            "Extra space-separated arguments forwarded to the program after the dataset path "
            "(e.g. --program-args='341' for pnd (3,4)-nucleus decomposition)."
        ),
    )

    args, rest = parser.parse_known_args()
    core_counts = [int(x) for x in args.cores.split(",") if x.strip()]
    program_args = args.program_args.split() if args.program_args.strip() else []

    if rest:
        if rest[0] in {"sample", "s"}:
            if len(rest) > 2:
                parser.error("sample mode accepts at most one dataset index")
            graph_num = rest[1] if len(rest) == 2 else "50"
            exit_code, output = run_sample(args.program, graph_num, program_args)
            print(json.dumps(output, indent=2))
            return exit_code

        if rest[0] in {"large", "l"}:
            if len(rest) > 2:
                parser.error("large mode accepts at most one dataset index")
            dataset_index = rest[1] if len(rest) == 2 else "0"
            exit_code, output = run_large(args.program, dataset_index, core_counts, program_args)
            print(json.dumps(output, indent=2))
            return exit_code

        if len(rest) == 1 and rest[0].isdigit():
            exit_code, output = run_large(args.program, rest[0], core_counts, program_args)
            print(json.dumps(output, indent=2))
            return exit_code

        parser.error(
            "Usage: run.py [--program PROGRAM] [--cores CORES] [--program-args ARGS] "
            "[sample|s [index] | large|l [index] | <large-index>]"
        )

    # Default: all large datasets for pnd with 341
    if not program_args and args.program == "pnd":
        program_args = ["341"]

    exit_code, output = run_large(args.program, "0", core_counts, program_args)
    print(json.dumps(output, indent=2))
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
