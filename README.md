# Nucleus Decomposition Framework

This is a collection of the codes on the nucleus decomposition framework.

- `nd` has the basic nucleus decomposition[1] and hierarchy construction[2] codes.
- `bnd` has the tip and wing decompositions for bipartite networks[3] along with the hierarchy contructions.
- `pnd` has the parallel nucleus decomposition algorithms[4], based on the h-index formulation -- no hierarchy construction exists.
- `dnd` has the directed 2,3 nucleus decomposition algorithms.

## Instructions to Run on Nibi

Build the targets with:

```bash
./scripts/nibi/build_nucleus.sh
```

The executables are written to `./bin/` inside this submodule:

- `./bin/nd/nucleus`
- `./bin/bnd/binucleus`
- `./bin/pnd/pnd`
- `./bin/dnd` if you have a separate directed-nucleus checkout and point `DND_DIR` at it

Run jobs with:

```bash
sbatch ./scripts/nibi/run_nucleus.sh [OPTIONS] [INDEX]
```

Options:

- `--program PROGRAM:MODE` Run program with mode (can be specified multiple times or comma-separated, default: `pnd:341`)
- `--cores CORES` Comma-separated core counts (default: `1,2,4,8,16,24,32,48,64,80,96`)
- `--output-json PATH` Append each completed execution result into `PATH` as a merged JSON object instead of printing the final aggregate to stdout

Modes:

- For `nd`: `12, 13, 14, 23, 24, 34`
- For `pnd`: `120, 1200, 230, 2300, 340, 3400, 341, 734, 342, 34000, 340000`, etc.

Examples:

```bash
# Run nd with modes 34, 23, 12 for all large datasets
sbatch ./scripts/nibi/run_nucleus.sh --program nd:34 --program nd:23 --program nd:12 large

# Run pnd with modes 341, 340 for all large datasets
sbatch ./scripts/nibi/run_nucleus.sh --program pnd:341,pnd:340 large

# Run both nd and pnd with multiple modes across specific cores
sbatch ./scripts/nibi/run_nucleus.sh --cores "1,4,16,64" --program nd:34,nd:23,nd:12 --program pnd:341,pnd:340 large

# Persist results incrementally to a JSON file
sbatch ./scripts/nibi/run_nucleus.sh --output-json RESULTS/nc1/run.json --program pnd:341,pnd:340,nd:34 large

# Run on sample dataset
sbatch ./scripts/nibi/run_nucleus.sh --program nd:34 --program pnd:341 sample 1

# If submitting from RESULTS/ncx directory
sbatch ../../related/nucleus/scripts/nibi/run_nucleus.sh --program pnd:341,pnd:340 --cores 1,4,16,32,64,96 large
```

The wrapper forwards arguments to `scripts/nibi/run.py`, which executes the selected configurations.

## Reporting Maximal Nuclei (subgraph CSVs)

To enumerate the dense subgraphs of a large dataset instead of timing it, use the
CPU runner:

```bash
sbatch ./scripts/nibi/run_subgraphs.sh [OPTIONS] [INDEX]
```

It runs the sequential `nd/nucleus` binary in hierarchy mode for three
decompositions on the selected large dataset:

- `kcore` &rarr; `(1,2)`-nucleus (algorithm `12`)
- `ktruss` &rarr; `(2,3)`-nucleus (algorithm `23`)
- `nucleus34` &rarr; `(3,4)`-nucleus (algorithm `34`)

`INDEX` is a 1-based index into the large-dataset list (`0`/`all` runs every
dataset). For each dataset+tool it writes one CSV named `{dataset-name}_{tool}.csv`
(e.g. `amazon-2008_kcore.csv`, `amazon-2008_ktruss.csv`, `amazon-2008_nucleus34.csv`)
with one row per **maximal nucleus** — a connected component in the nucleus
forest, i.e. a direct child of the artificial whole-graph root:

```
dataset, level, nucleus_id, vertex_count, edge_count, density
```

where `level` is the tool name and `density = edge_count / C(vertex_count, 2)`.
The raw `<dataset>_<algo>_NUCLEI` / `_Hierarchy` files are kept alongside the CSVs.

Options:

- `--tools LIST` Comma-separated subset of `kcore,ktruss,nucleus34` (default: all)
- `--output-dir DIR` Where to write the CSVs and raw NUCLEI files (default: CWD)

```bash
# All three tools on dataset index 1 (amazon-2008), writing into RESULTS/sg1
sbatch ./scripts/nibi/run_subgraphs.sh --output-dir RESULTS/sg1 1

# Only k-truss on every large dataset
sbatch ./scripts/nibi/run_subgraphs.sh --tools ktruss all
```

The subgraph files are only emitted when the `NUCLEUS_REPORT_SUBGRAPH`
environment variable is set; `run_subgraphs.py` sets it automatically. The
default timing builds skip this work for speed.

## References

1. Finding the Hierarchy of Dense Subgraphs using Nucleus Decompositions<br>
	A. Erdem Sariyuce, C. Seshadhri, Ali Pinar, Umit V. Catalyürek<br>
	International World Wide Web Conference (WWW), 2015. [external PDF](https://sariyuce.com/papers/www15.pdf) | [repo PDF](./www15.pdf)<br>

2. Fast Hierarchy Construction for Dense Subgraphs<br>
	A. Erdem Sariyuce, Ali Pinar<br>
	International Conference on Very Large Data Bases (VLDB), 2017. [external PDF](https://sariyuce.com/papers/vldb17.pdf) | [repo PDF](./vldb17.pdf)<br>

3. Peeling Bipartite Networks for Dense Subgraph Discovery<br>
	A. Erdem Sariyuce, Ali Pinar<br>
	[arXiv:1611.02756](https://arxiv.org/pdf/1611.02756.pdf) | [repo PDF](./1611.02756.pdf)<br>

4. Parallel Local Algorithms for Core, Truss, and Nucleus Decompositions<br>
	A. Erdem Sariyuce, C. Seshadhri, Ali Pinar<br>
	[arXiv:1704.00386](https://arxiv.org/pdf/1704.00386.pdf) | [repo PDF](./1704.00386.pdf)<br>


