# Nucleus Decomposition Framework

This is a collection of the codes on the nucleus decomposition framework.

- `nd` has the basic nucleus decomposition[1] and hierarchy construction[2] codes.
- `bnd` has the tip and wing decompositions for bipartite networks[3] along with the hierarchy contructions.
- `pnd` has the parallel nucleus decomposition algorithms[4], based on the h-index formulation -- no hierarchy construction exists.
- `dnd` has the directed 2,3 nucleus decomposition algorithms.

## Instructions to Run on Nibi

Run the following script

```bash
./scripts/nibi/build_nucleus_all.sh
```

The targets will be build inside the `bin` foloder as follow
* bnd/binucleus
* nd/nucleus
* pnd/pnd


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
    
    
