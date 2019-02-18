#include "main.h"

vertex count_cycleps (Graph& dgraph, vector<vertex>& TC) {
	vertex count = 0;
	for (vertex u = 0; u < dgraph.size(); u++) {
		vector<vertex> ret;
		asymmetric_undirecteds (u, dgraph[u], ret);
		for (vertex r = 0; r < ret.size(); r++) {
			vertex v = dgraph[u][ret[r]];
			vector<vertex> ints;
			inter (2, 1, dgraph, u, v, ints);
			for (size_t k = 0; k < ints.size(); k+=2) {
				vertex w = dgraph[u][ints[k]];
				TC[u]++;
				TC[v]++;
				TC[w]++;
				count++;
			}

			ints.clear();
			inter (1, 2, dgraph, u, v, ints);
			for (size_t k = 0; k < ints.size(); k+=2) {
				vertex w = dgraph[v][ints[k+1]];
				TC[u]++;
				TC[v]++;
				TC[w]++;
				count++;
			}
		}
	}
	printf ("total cyclep count: %d\n", count);
	return count;
}

void cyclep_core (Graph& graph, bool hierarchy, edge nEdge, vector<vertex>& K, vertex* maxK, FILE* fp) {
	const auto t1 = chrono::steady_clock::now();
	// Cyclep counting for each node
	vertex nVtx = graph.size();
	vector<vertex> TC (nVtx, 0);
	lol tric = count_cycleps (graph, TC);
	fprintf (fp, "# cycles: %lld\n", tric);
	const auto t2 = chrono::steady_clock::now();
	print_time (fp, "Cycle counting: ", t2 - t1);

	// Peeling
	const auto p1 = chrono::steady_clock::now();
	K.resize (nVtx, -1);
	Naive_Bucket nBucket;
	nBucket.Initialize (nEdge, nVtx);
	// each node and its cycle-count is inserted to bucket
	for (vertex i = 0; i < graph.size(); i++) {
		printf ("cyclep count of %d is %d\n", i, TC[i]);
		if (TC[i] > 0)
			nBucket.Insert (i, TC[i]);
		else
			K[i]= 0;
	}
	vertex tc_u = 0;
	while (true) {
		vertex u, val;
		if (nBucket.PopMin(&u, &val) == -1) // if the bucket is empty
			break;
		tc_u = K[u] = val;
		// triangle node orbit
		vector<vertex> ret;
		outgoings (graph[u], ret);
		for (vertex r = 0; r < ret.size(); r++) {
			vertex v = graph[u][ret[r]]; // square node orbit
			if (K[v] == -1) { // v might not be a part of cyclep, but must have -1 kappa if so anyways
				vector<vertex> ints;
				inter (0, 2, graph, u, v, ints);
				for (auto k = 0; k < ints.size(); k+=2) {
					vertex w = graph[v][ints[k+1]];
					checkAndDec (K[w], -1, w, v, &nBucket, tc_u);
				}
				ints.clear();
				inter (1, 0, graph, u, v, ints);
				for (auto k = 0; k < ints.size(); k+=2) {
					vertex w = M2P (graph[u][ints[k]]);
					checkAndDec (K[w], -1, w, v, &nBucket, tc_u);
				}
			}
		}
		// triangle node orbit
		ret.clear();
		incomings (graph[u], ret);
		for (vertex r = 0; r < ret.size(); r++) {
			vertex v = M2P (graph[u][ret[r]]); // circle node orbit
			if (K[v] == -1) { // v might not be a part of cyclep, but must have -1 kappa if so anyways
				vector<vertex> ints;
				inter (2, 0, graph, u, v, ints);
				for (auto k = 0; k < ints.size(); k+=2) {
					vertex w = graph[u][ints[k]];
					checkAndDec (K[w], -1, w, v, &nBucket, tc_u);
				}
				ints.clear();
				inter (0, 1, graph, u, v, ints);
				for (auto k = 0; k < ints.size(); k+=2) {
					vertex w = M2P (graph[v][ints[k+1]]);
					checkAndDec (K[w], -1, w, v, &nBucket, tc_u);
				}
			}
		}
	}
	nBucket.Free();
	*maxK = tc_u;
	const auto p2 = chrono::steady_clock::now();

	if (!hierarchy) {
		print_time (fp, "Only peeling time: ", p2 - p1);
		print_time (fp, "Total time: ", (p2 - p1) + (t2 - t1));
	}
	for (auto i = 0; i < K.size(); i++)
		printf ("core of %d is %d\n", i, K[i]);
	return;
}

