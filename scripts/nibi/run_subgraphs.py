#!/usr/bin/env python3
"""Report the maximal nuclei (connected components of the nucleus forest) for the
large datasets.

For a selected dataset, this runs the sequential `nd/nucleus` binary in
hierarchy mode for three decompositions:

    kcore     -> (1,2)-nucleus  (algorithm 12)
    ktruss    -> (2,3)-nucleus  (algorithm 23)
    nucleus34 -> (3,4)-nucleus  (algorithm 34)

Each run writes a `<dataset>_<algo>_NUCLEI` file describing the full hierarchy
forest. A *maximal nucleus* is a connected component in that forest, i.e. a
direct child of the artificial whole-graph root (the only node whose parent is
-1). Each such node's |V|/|E| already aggregates its whole subtree.

One CSV is written per tool with the columns:

    dataset, level, nucleus_id, vertex_count, edge_count, density

where `level` is the tool name and density = edge_count / C(vertex_count, 2).

The nucleus binary only emits the *_NUCLEI / *_Hierarchy files when the
NUCLEUS_REPORT_SUBGRAPH environment variable is set (this runner sets it); the
default timing builds skip them.
"""

from __future__ import annotations

import argparse
import csv
import os
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

# Reuse the dataset catalog and paths from the timing runner so the two stay in
# sync (same indices, same dataset directory, same bin directory).
from run import LARGE_DATASETS, DATASET_DIR, BIN_DIR  # noqa: E402

NUCLEUS_BIN = BIN_DIR / "nd" / "nucleus"

# Tool name -> nd algorithm code. Order is the order tools run / CSVs appear.
TOOLS: dict[str, str] = {
    "kcore": "12",
    "ktruss": "23",
    "nucleus34": "34",
}

CSV_COLUMNS = ["dataset", "level", "nucleus_id", "vertex_count", "edge_count", "density"]


def density(vertex_count: int, edge_count: int) -> float:
    """Edge density = edges / C(vertices, 2). Zero for fewer than two vertices."""
    if vertex_count < 2:
        return 0.0
    return edge_count / (vertex_count * (vertex_count - 1) / 2)


def parse_nuclei(path: Path) -> dict[int, dict[str, int]]:
    """Parse a *_NUCLEI file into {id: {id, K, V, E, leaf, parent}}.

    Each line is: `id K |V| |E| ed leaf parent<TAB>v1 v2 ... -1`.
    """
    nodes: dict[int, dict[str, int]] = {}
    with path.open() as handle:
        for line in handle:
            head = line.split("\t", 1)[0].split()
            if len(head) < 7:
                continue
            node = {
                "id": int(head[0]),
                "K": int(head[1]),
                "V": int(head[2]),
                "E": int(head[3]),
                "leaf": int(head[5]),
                "parent": int(head[6]),
            }
            nodes[node["id"]] = node
    return nodes


def maximal_nuclei(nodes: dict[int, dict[str, int]]) -> list[dict[str, int]]:
    """Return the connected components of the forest: the direct children of the
    artificial whole-graph root(s) (the nodes whose parent is -1)."""
    roots = {nid for nid, node in nodes.items() if node["parent"] == -1}
    components = [node for node in nodes.values() if node["parent"] in roots]
    # Largest component first, ties broken by id for determinism.
    components.sort(key=lambda node: (-node["V"], node["id"]))
    return components


def run_tool(
    tool: str,
    algo: str,
    dataset_path: Path,
    output_dir: Path,
) -> list[dict[str, object]]:
    """Run one decomposition and return its maximal-nucleus rows."""
    gname = dataset_path.name
    nuclei_path = output_dir / f"{gname}_{algo}_NUCLEI"

    env = os.environ.copy()
    env["NUCLEUS_REPORT_SUBGRAPH"] = "1"

    command = [str(NUCLEUS_BIN), str(dataset_path), algo, os.devnull, "YES"]
    print(f"[{tool}] {gname}: {' '.join(command)}", file=sys.stderr, flush=True)

    completed = subprocess.run(
        command,
        cwd=output_dir,
        env=env,
        capture_output=True,
        text=True,
    )

    if completed.stderr:
        print(completed.stderr, file=sys.stderr, end="")
    if completed.returncode != 0:
        raise SystemExit(
            f"ERROR: {tool} failed on {gname} (exit {completed.returncode})"
        )
    if not nuclei_path.exists():
        raise SystemExit(
            f"ERROR: expected NUCLEI file not produced: {nuclei_path}"
        )

    nodes = parse_nuclei(nuclei_path)
    rows: list[dict[str, object]] = []
    for node in maximal_nuclei(nodes):
        rows.append(
            {
                "dataset": gname,
                "level": tool,
                "nucleus_id": node["id"],
                "vertex_count": node["V"],
                "edge_count": node["E"],
                "density": f"{density(node['V'], node['E']):.6f}",
            }
        )
    print(
        f"[{tool}] {gname}: {len(rows)} maximal nuclei", file=sys.stderr, flush=True
    )
    return rows


def write_csv(path: Path, rows: list[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=CSV_COLUMNS)
        writer.writeheader()
        writer.writerows(rows)


def resolve_datasets(index_arg: str) -> list[Path]:
    """Map the dataset selector to a list of dataset paths.

    '0' / 'all' -> every large dataset; otherwise a 1-based index into
    LARGE_DATASETS.
    """
    if index_arg in {"0", "all"}:
        return [DATASET_DIR / name for name in LARGE_DATASETS]

    try:
        index = int(index_arg)
    except ValueError:
        raise SystemExit(f"Invalid dataset index: {index_arg!r}")
    if index < 1 or index > len(LARGE_DATASETS):
        raise SystemExit(
            f"Invalid large dataset index: {index} "
            f"(valid range: 1-{len(LARGE_DATASETS)} or 0/all)"
        )
    return [DATASET_DIR / LARGE_DATASETS[index - 1]]


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Run kcore/ktruss/nucleus34 decompositions on a large dataset and "
            "write one CSV per tool listing every maximal nucleus."
        )
    )
    parser.add_argument(
        "index",
        nargs="?",
        default="0",
        help=(
            "1-based large-dataset index (1.."
            f"{len(LARGE_DATASETS)}), or '0'/'all' for every dataset. "
            "Datasets: "
            + ", ".join(f"{i + 1}={name}" for i, name in enumerate(LARGE_DATASETS))
        ),
    )
    parser.add_argument(
        "--tools",
        default=",".join(TOOLS),
        help=(
            "Comma-separated subset of tools to run "
            f"(choices: {', '.join(TOOLS)}; default: all)."
        ),
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path.cwd(),
        help="Directory for the CSV outputs and the raw *_NUCLEI files (default: CWD).",
    )

    args = parser.parse_args()

    selected_tools = [t.strip() for t in args.tools.split(",") if t.strip()]
    unknown = [t for t in selected_tools if t not in TOOLS]
    if unknown:
        parser.error(
            f"Unknown tool(s): {', '.join(unknown)}. Choices: {', '.join(TOOLS)}"
        )

    if not NUCLEUS_BIN.exists():
        raise SystemExit(
            f"ERROR: nucleus binary not found: {NUCLEUS_BIN}\n"
            "Build it first with scripts/nibi/build_nucleus.sh"
        )

    datasets = resolve_datasets(args.index)
    missing = [str(path) for path in datasets if not path.exists()]
    if missing:
        raise SystemExit("ERROR: dataset(s) not found:\n  " + "\n  ".join(missing))

    output_dir = args.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    # One CSV per tool, accumulating rows across the selected dataset(s).
    rows_by_tool: dict[str, list[dict[str, object]]] = {t: [] for t in selected_tools}
    for dataset_path in datasets:
        for tool in selected_tools:
            rows_by_tool[tool].extend(
                run_tool(tool, TOOLS[tool], dataset_path, output_dir)
            )

    for tool in selected_tools:
        csv_path = output_dir / f"{tool}.csv"
        write_csv(csv_path, rows_by_tool[tool])
        print(
            f"Wrote {len(rows_by_tool[tool])} rows -> {csv_path}",
            file=sys.stderr,
            flush=True,
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
