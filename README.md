# Nucleus Decomposition Framework

This is a collection of the codes on the nucleus decomposition framework.

- `nd` has the basic nucleus decomposition[1] and hierarchy construction[2] codes.
- `bnd` has the tip and wing decompositions for bipartite networks[3] along with the hierarchy contructions.
- `pnd` has the parallel nucleus decomposition algorithms[4], based on the h-index formulation -- no hierarchy construction exists.
- `dnd` has the directed 2,3 nucleus decomposition algorithms.

## Instructions to Run on Nibi

Build the targets with:

```bash
./scripts/nibi/build_nucleus_all.sh
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
sbatch ../../related/nucleus/scripts/nibi/run_nucleus.sh --program pnd:341,pnd:340,nd:34 --cores 1,4,16,32,64,96 large
```

The wrapper forwards arguments to `scripts/nibi/run.py`, which executes the selected configurations.

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


