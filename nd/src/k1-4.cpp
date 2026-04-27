#include "main.h"
#include "json.h"
using namespace json;

// per vertex
lol count4cliques (Graph& graph, Graph& orientedGraph, vector<vertex>& FC) {

	lol fc = 0;
	for (vertex i = 0; i < orientedGraph.size(); i++) {
		for (vertex j = 0; j < orientedGraph[i].size(); j++) {
			for (vertex k = j + 1; k < orientedGraph[i].size(); k++) {
				for (vertex l = k + 1; l < orientedGraph[i].size(); l++) {
					vertex a = orientedGraph[i][j];
					vertex b = orientedGraph[i][k];
					vertex c = orientedGraph[i][l];
					if (orientedConnected(graph, orientedGraph, a, b) &&
							orientedConnected(graph, orientedGraph, b, c) &&
							orientedConnected(graph, orientedGraph, a, c)) {
						FC[a]++;
						FC[b]++;
						FC[c]++;
						FC[i]++;
						fc++;
					}
				}
			}
		}
	}
	return fc;
}

void base_k14 (Graph& graph, bool hierarchy, edge nEdge, vector<vertex>& K, vertex* max14, string vfile, FILE* fp) {

	timestamp f1;
	vertex nVtx = graph.size();

	// Create directed graph from low degree vertices to higher degree vertices
	Graph orientedGraph;
	createOriented (orientedGraph, graph);

	// 4-clique counting for each vertex
	vector<vertex> FC (nVtx, 0);
	lol fc = count4cliques (graph, orientedGraph, FC);

	vertex maxFC = 0;
	for (vertex i = 0; i < graph.size(); i++)
		if (FC[i] > maxFC)
			maxFC = FC[i];

	field(false, "four_clique_count", fc);
	timestamp f2;
	field(false, "four_clique_time_sec", f2 - f1);

	// Peeling
	timestamp p1;
	K.resize(nVtx, -1);
	Naive_Bucket nBucket;
	nBucket.Initialize(maxFC, nVtx);
	for (vertex i = 0; i < graph.size(); i++) {
		if (FC[i] > 0)
			nBucket.Insert (i, FC[i]);
		else
			K[i] = 0;
	}

	vertex fc_u = 0;

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

		fc_u = K[u] = val;

		for (vertex j = 0; j < graph[u].size(); j++) {
			vertex a = graph[u][j];
			for (vertex k = j + 1; k < graph[u].size(); k++) {
				vertex b = graph[u][k];
				if (orientedConnected(graph, orientedGraph, a, b)) {
					for (vertex l = k + 1; l < graph[u].size(); l++) {
						vertex c = graph[u][l];
						if (orientedConnected (graph, orientedGraph, a, c) && orientedConnected (graph, orientedGraph, b, c)) {
							if (K[a] == -1 && K[b] == -1 && K[c] == -1) {
								if (nBucket.CurrentValue(a) > fc_u)
									nBucket.DecVal(a);
								if (nBucket.CurrentValue(b) > fc_u)
									nBucket.DecVal(b);
								if (nBucket.CurrentValue(c) > fc_u)
									nBucket.DecVal(c);
							}
							else if (hierarchy)
								createSkeleton (u, {a, b, c}, &nSubcores, K, skeleton, component, unassigned, relations);
						}
					}
				}
			}
		}

		if (hierarchy)
			updateUnassigned (u, component, &cid, relations, unassigned);
	}

	nBucket.Free();
	*max14 = fc_u;

	timestamp p2;

	field(false, "K_avg", [&K](){ 
		double sumK = 0.;
		for (auto k : K)
			sumK += k;
		return K.size() ? (sumK / K.size()) : 0.;
	}());

	if (!hierarchy) {
		field(false, "peeling_time_sec", p2 - p1);
		field(false, "total_time_sec", (p2 - p1) + (f2 - f1));
	}
	else {
		field(false, "peeling_hierarchy_time_sec", p2 - p1);
		timestamp b1;
		buildHierarchy (*max14, relations, skeleton, &nSubcores, nEdge, nVtx);
		timestamp b2;

		field(false, "build_hierarchy_time_sec", b2 - b1);
		field(false, "total_time_sec_exclude_density", (p2 - p1) + (f2 - f1) + (b2 - b1));
		field(false, "subcore_count", nSubcores);
		field(false, "subsubcore_count", (int)skeleton.size());

		timestamp d1;
		helpers hp;
		presentNuclei (14, skeleton, component, graph, nEdge, hp, vfile, fp);
		timestamp d2;

		field(false, "total_time_sec", (p2 - p1) + (f2 - f1) + (b2 - b1) + (d2 - d1));
	}
}
