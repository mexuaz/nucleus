#include "main.h"
#include "json.h"
using namespace json;

// per vertex
lol countTriangles (Graph& graph, Graph& orientedGraph, vector<vertex>& TC) {

	lol tc = 0;
	for (size_t i = 0; i < orientedGraph.size(); i++) {
		for (size_t j = 0; j < orientedGraph[i].size(); j++) {
			for (size_t k = j + 1; k < orientedGraph[i].size(); k++) {
				vertex a = orientedGraph[i][j];
				vertex b = orientedGraph[i][k];
				if (orientedConnected (graph, orientedGraph, a, b)) {
					TC[a]++;
					TC[b]++;
					TC[i]++;
					tc++;
				}
			}
		}
	}
	return tc;
}

void base_k13 (Graph& graph, bool hierarchy, edge nEdge, vector<vertex>& K, vertex* max13, string vfile, FILE* fp) {

	timestamp t1;
	vertex nVtx = graph.size();

	// Create directed graph from low degree vertices to higher degree vertices
	Graph orientedGraph;
	createOriented (orientedGraph, graph);

	// Triangle counting for each vertex
	vector<vertex> TC (nVtx, 0);
	lol tric = countTriangles (graph, orientedGraph, TC);

	field(false, "triangle_count", tric);
	timestamp t2;
	field(false, "triangle_time_sec", t2 - t1);

	// Peeling
	timestamp p1;
	K.resize (nVtx, -1);
	Naive_Bucket nBucket;
	nBucket.Initialize (nEdge, nVtx); // maximum triangle count of a vertex is nEdge
	for (vertex i = 0; i < graph.size(); i++)
		if (TC[i] > 0)
			nBucket.Insert (i, TC[i]);
		else
			K[i] = 0;

	vertex tc_u = 0;

	// required for hierarchy
	vertex cid; // subcore id number
	vector<subcore> skeleton; // equal K valued cores
	vector<vertex> component; // subcore ids for each vertex
	vector<vp> relations;
	vector<vertex> unassigned;
	vertex nSubcores;

	if (hierarchy) {
		cid = 0;
		nSubcores = 0;
		component.resize (graph.size(), -1);
	}

	while (true) {
		vertex u, val;
		if (nBucket.PopMin(&u, &val) == -1) // if the bucket is empty
			break;

		if (hierarchy) {
			unassigned.clear();
			subcore sc (val);
			skeleton.push_back (sc);
		}

		tc_u = K[u] = val;

		for (vertex j = 0; j < graph[u].size(); j++) {
			vertex a = graph[u][j];
			for (vertex k = j + 1; k < graph[u].size(); k++) {
				vertex b = graph[u][k];
				if (orientedConnected (graph, orientedGraph, a, b)) {
					if (K[a] == -1 && K[b] == -1) {
						if (nBucket.CurrentValue(a) > tc_u)
							nBucket.DecVal(a);
						if (nBucket.CurrentValue(b) > tc_u)
							nBucket.DecVal(b);
					}
					else if (hierarchy)
						createSkeleton (u, {a, b}, &nSubcores, K, skeleton, component, unassigned, relations);
				}
			}
		}

		if (hierarchy)
			updateUnassigned (u, component, &cid, relations, unassigned);
	}

	nBucket.Free();
	*max13 = tc_u;

	timestamp p2;

	field(false, "K_avg", [&K](){ 
		double sumK = 0.;
		for (auto k : K)
			sumK += k;
		return K.size() ? (sumK / K.size()) : 0.;
	}());

	if (!hierarchy) {
		field(false, "peeling_time_sec", p2 - p1);
		field(false, "total_time_sec", (p2 - p1) + (t2 - t1));
	}
	else {
		field(false, "peeling_hierarchy_time_sec", p2 - p1);
		timestamp b1;
		buildHierarchy (*max13, relations, skeleton, &nSubcores, nEdge, nVtx);
		timestamp b2;

		field(false, "build_hierarchy_time_sec", b2 - b1);
		field(false, "total_time_sec_exclude_density", (p2 - p1) + (t2 - t1) + (b2 - b1));
		field(false, "subcore_count", nSubcores);
		field(false, "subsubcore_count", (int)skeleton.size());

		timestamp d1;
		helpers hp;
		presentNuclei (13, skeleton, component, graph, nEdge, hp, vfile, fp);
		timestamp d2;

		field(false, "total_time_sec", (p2 - p1) + (t2 - t1) + (b2 - b1) + (d2 - d1));
	}
}
