#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from collections.abc import Callable
from datetime import datetime, timezone
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parents[3]  # nibi -> scripts -> nucleus -> related -> repo root
DATASET_DIR = REPO_ROOT / "src" / "build" / "datasets"
BIN_DIR = REPO_ROOT / "related" / "nucleus" / "bin"

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


def load_output_file(output_path: Path) -> dict[str, object]:
    if not output_path.exists():
        return {}

    try:
        existing = json.loads(output_path.read_text())
    except json.JSONDecodeError as exc:
        raise SystemExit(f"Existing output file is not valid JSON: {output_path}: {exc}") from exc

    if not isinstance(existing, dict):
        raise SystemExit(
            f"Existing output file must contain a top-level JSON object: {output_path}"
        )

    return existing


def write_output_file(output_path: Path, new_output: dict[str, object]) -> None:
    merged_output = load_output_file(output_path)
    merged_output.update(new_output)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    temp_path = output_path.with_suffix(output_path.suffix + ".tmp")
    temp_path.write_text(json.dumps(merged_output, indent=2))
    temp_path.replace(output_path)


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
    output_path: Path | None = None,
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
            parsed_output = json.loads(completed.stdout)
        except json.JSONDecodeError as exc:
            print(
                f"ERROR: Program output is not valid JSON for {dataset_path} (cores={n}): {exc}",
                file=sys.stderr,
            )
            if completed.stdout:
                print(completed.stdout, file=sys.stderr, end="")
            return 1, outputs

        outputs[timestamp] = parsed_output
        if output_path is not None:
            write_output_file(output_path, {timestamp: parsed_output})

    return 0, outputs


def run_sample(
    program_name: str,
    graph_num: str,
    program_args: list[str],
    output_path: Path | None = None,
) -> tuple[int, dict[str, object]]:
    dataset_path = REPO_ROOT / "datasets" / "tests" / f"sample_input_graph_{graph_num}.mtx"
    return run_program(
        program_name,
        dataset_path,
        program_args,
        core_counts=[1],
        output_path=output_path,
    )


def run_large(
    program_name: str,
    dataset_index: str,
    core_counts: list[int],
    program_args: list[str],
    output_path: Path | None = None,
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
        exit_code, output = run_program(
            program_name,
            dataset_path,
            program_args,
            core_counts,
            output_path=output_path,
        )
        aggregate_output.update(output)
        return exit_code, aggregate_output

    exit_code = 0
    for dataset_name in LARGE_DATASETS:
        dataset_path = DATASET_DIR / dataset_name
        result_code, output = run_program(
            program_name,
            dataset_path,
            program_args,
            core_counts,
            output_path=output_path,
        )
        exit_code = exit_code or result_code
        aggregate_output.update(output)

    return exit_code, aggregate_output


def parse_program_specs(raw_values: list[str]) -> list[tuple[str, list[str]]]:
    specs: list[tuple[str, list[str]]] = []

    for raw_value in raw_values:
        entries = [entry.strip() for entry in raw_value.split(",") if entry.strip()]
        for entry in entries:
            if ":" not in entry:
                raise ValueError(
                    f"Invalid --program value '{entry}'. Expected 'program:mode'."
                )

            program_name, mode_str = entry.split(":", 1)
            program_name = program_name.strip()
            mode_args = mode_str.split() if mode_str.strip() else []

            if not program_name:
                raise ValueError(
                    f"Invalid --program value '{entry}'. Program name cannot be empty."
                )

            if not mode_args:
                raise ValueError(
                    f"Invalid --program value '{entry}'. Mode cannot be empty."
                )

            specs.append((program_name, mode_args))

    if not specs:
        raise ValueError("At least one --program value is required.")

    return specs


def run_program_specs(
    program_specs: list[tuple[str, list[str]]],
    runner: Callable[[str, list[str]], tuple[int, dict[str, object]]],
) -> tuple[int, dict[str, object]]:
    aggregate_output: dict[str, object] = {}

    for program_name, mode_args in program_specs:
        exit_code, output = runner(program_name, mode_args)
        aggregate_output.update(output)
        if exit_code != 0:
            return exit_code, aggregate_output

    return 0, aggregate_output


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run a nucleus decomposition program on sample or large datasets."
    )
    parser.add_argument(
        "--program",
        action="append",
        default=None,
        help=(
            "Program and mode pair in format 'program:mode' (e.g. --program pnd:341). "
            "Use multiple --program flags or comma-separated values to run several entries "
            "(e.g. --program nd:34 --program pnd:341 or --program nd:34,pnd:341)."
        ),
    )
    parser.add_argument(
        "--cores",
        default="1,2,4,8,16,24,32,48,64,80,96",
        help="Comma-separated list of core counts for the sweep.",
    )
    parser.add_argument(
        "--output-json",
        type=Path,
        help=(
            "Write results to a JSON file instead of stdout. The file is updated after each "
            "successful execution by reading any existing JSON object, merging the new result, "
            "and writing valid JSON back to disk."
        ),
    )

    args, rest = parser.parse_known_args()
    core_counts = [int(x) for x in args.cores.split(",") if x.strip()]
    output_path = args.output_json.resolve() if args.output_json is not None else None
    try:
        program_specs = parse_program_specs(args.program or ["pnd:341"])
    except ValueError as exc:
        parser.error(str(exc))

    if rest:
        if rest[0] in {"sample", "s"}:
            if len(rest) > 2:
                parser.error("sample mode accepts at most one dataset index")
            graph_num = rest[1] if len(rest) == 2 else "50"
            exit_code, output = run_program_specs(
                program_specs,
                lambda program_name, mode_args: run_sample(
                    program_name,
                    graph_num,
                    mode_args,
                    output_path=output_path,
                ),
            )
            if output_path is None:
                print(json.dumps(output, indent=2))
            return exit_code

        if rest[0] in {"large", "l"}:
            if len(rest) > 2:
                parser.error("large mode accepts at most one dataset index")
            dataset_index = rest[1] if len(rest) == 2 else "0"
            exit_code, output = run_program_specs(
                program_specs,
                lambda program_name, mode_args: run_large(
                    program_name,
                    dataset_index,
                    core_counts,
                    mode_args,
                    output_path=output_path,
                ),
            )
            if output_path is None:
                print(json.dumps(output, indent=2))
            return exit_code

        if len(rest) == 1 and rest[0].isdigit():
            dataset_index = rest[0]
            exit_code, output = run_program_specs(
                program_specs,
                lambda program_name, mode_args: run_large(
                    program_name,
                    dataset_index,
                    core_counts,
                    mode_args,
                    output_path=output_path,
                ),
            )
            if output_path is None:
                print(json.dumps(output, indent=2))
            return exit_code

        parser.error(
            "Usage: run.py [OPTIONS] [sample|s [index] | large|l [index] | <large-index>]\n\n"
            "OPTIONS:\n"
            "  --program PROGRAM:MODE         Program and mode pair; repeat or comma-separate values\n"
            "  --cores CORES                  Comma-separated core counts\n"
            "                                 (default: 1,2,4,8,16,24,32,48,64,80,96)\n\n"
            "  --output-json PATH            Persist merged JSON results to PATH after each run\n\n"
            "EXAMPLES:\n"
            "  # Run nd with modes 34, 23, 12 on all large datasets:\n"
            "  run.py --program nd:34 --program nd:23 --program nd:12 large\n\n"
            "  # Run pnd with modes 341, 340 on all large datasets:\n"
            "  run.py --program pnd:341,pnd:340 large\n\n"
            "  # Run both nd and pnd with specific cores:\n"
            "  run.py --cores '1,4,16,64' --program nd:34 --program nd:23 "
            "--program pnd:341 --program pnd:340 large\n\n"
            "  # Run on sample dataset:\n"
            "  run.py --program nd:34 --program pnd:341 sample 1\n\n"
            "  # Run single program:\n"
            "  run.py --program pnd:341 large"
        )

    exit_code, output = run_program_specs(
        program_specs,
        lambda program_name, mode_args: run_large(
            program_name,
            "0",
            core_counts,
            mode_args,
            output_path=output_path,
        ),
    )
    if output_path is None:
        print(json.dumps(output, indent=2))
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
