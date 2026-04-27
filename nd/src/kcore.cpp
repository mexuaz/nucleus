#include "main.h"
#include "json.h"
using namespace json;

void base_kcore (Graph& graph, bool hierarchy, edge nEdge, vector<vertex>& K, vertex* maxCore, string vfile, FILE* fp) {

	timestamp p1;
	vertex nVtx = graph.size();
	vertex maxDeg = 0;
	for (auto g : graph)
		if (g.size() > maxDeg)
			maxDeg = g.size();

	// Peeling
	K.resize (graph.size(), -1);
	Naive_Bucket nBucket;
	nBucket.Initialize(maxDeg, nVtx);
	for (vertex i = 0; i < graph.size(); i++) {
		if (graph[i].size() > 0)
			nBucket.Insert (i, graph[i].size());
		else
			K[i] = 0;
	}

	vertex deg_u = 0;

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

		deg_u = K[u] = val;

		for (auto v : graph[u]) { // decrease the degree of the neighbors with greater degree
			vertex deg_v = nBucket.CurrentValue(v);
			if (deg_v > deg_u)
				nBucket.DecVal(v);
			else if (hierarchy) // hierarchy related
				createSkeleton (u, {v}, &nSubcores, K, skeleton, component, unassigned, relations);
		}
		if (hierarchy)
			updateUnassigned (u, component, &cid, relations, unassigned);
	}

	nBucket.Free();
	*maxCore = deg_u; // deg_u is degree of the last popped vertex
	
	timestamp p2;

	field(false, "K_avg", [&K](){ 
		double sumK = 0.;
		for (auto k : K)
			sumK += k;
		return K.size() ? (sumK / K.size()) : 0.;
	}());

	if (!hierarchy) {
		field(false, "peeling_time_sec", p2 - p1);
		field(false, "total_time_sec", p2 - p1);
	}
	else  {
		field(false, "peeling_hierarchy_time_sec", p2 - p1);
		timestamp b1;
		buildHierarchy (*maxCore, relations, skeleton, &nSubcores, nEdge, nVtx);
		timestamp b2;

		field(false, "build_hierarchy_time_sec", b2 - b1);
		field(false, "total_time_sec_exclude_density", (p2 - p1) + (b2 - b1));
		field(false, "subcore_count", nSubcores);
		field(false, "subsubcore_count", (int)skeleton.size());

		timestamp d1;
		helpers hp;
		presentNuclei (12, skeleton, component, graph, nEdge, hp, vfile, fp);
		timestamp d2;

		field(false, "total_time_sec", (p2 - p1) + (b2 - b1) + (d2 - d1));
	}
}
