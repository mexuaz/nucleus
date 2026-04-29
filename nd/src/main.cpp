#include "main.h"
#include "json.h"
using namespace json;

// Definition of the global JSON stream pointer (declared extern in json.h)
std::ostringstream* g_jsonSS = nullptr;

inline std::string env(const char* e)
{
	auto v = std::getenv(e);
	if (v)
		return std::string(v);
	return "";
}

int main (int argc, char *argv[]) {

	timestamp t1;
	if (argc < 3) {
		cerr << "usage: " << argv[0] << 
				"\n <filename>"
				"\n <nucleus type: 12, 13, 14, 23, 24, 34>"
				"\n [hierarchy: YES or NO] (optional, defaults to NO)" << endl;
		exit(1);
	}

	char *filename = argv[1];
	string tmp (argv[1]);
	string gname = tmp.substr (tmp.find_last_of("/") + 1);

	string nd (argv[2]);
	if (!(nd == "12" || nd == "13" || nd == "14" || nd == "23" || nd == "24" || nd == "34")) {
		cerr << "Invalid algorithm, options are 12, 13, 14, 23, 24, and 34" << endl;
		exit(1);
	}

	// read the graph, give sorted edges in graph
	edge nEdge = 0;
	Graph graph;
	readGraph<vertex, edge> (filename, graph, &nEdge);
	string vfile = gname + "_" + nd;
	string out_file;

	bool hierarchy = false;
	if (argc >= 4) {
		string hrc (argv[3]);
		if (hrc == "YES")
			hierarchy = true;
		else if (hrc != "NO") {
			cerr << "Invalid hierarchy option, options are YES and NO" << endl;
			exit(1);
		}
	}
	if (hierarchy)
		out_file = vfile + "_Hierarchy";
	else
		out_file = vfile + "_K";

	FILE* fp = fopen (out_file.c_str(), "w");

	std::ostringstream jsonSS;
	g_jsonSS = &jsonSS;

	beginObject(true, "environment");
	field(true, "app", string("nd") + nd);
	field(false, "host", env("HOSTNAME"));
	endObject();
	beginObject(false, "dataset");
	field(true, "file", gname);
	field(false, "vertex_count", (int)graph.size());
	field(false, "edge_count", nEdge);
	field(false, "hierarchy", hierarchy);

	vertex maxK; // maximum K value in the graph
	vector<vertex> K;

	if (nd == "12")
		base_kcore (graph, hierarchy, nEdge, K, &maxK, vfile, fp);
	else if (nd == "13")
		base_k13 (graph, hierarchy, nEdge, K, &maxK, vfile, fp);
	else if (nd == "14")
		base_k14 (graph, hierarchy, nEdge, K, &maxK, vfile, fp);
	else if (nd == "23") {
		base_ktruss (graph, hierarchy, nEdge, K, &maxK, vfile, fp);
		//		base_ktruss_storeTriangles (graph, hierarchy, nEdge/2, K, &maxK, vfile, fp);
	}
	else if (nd == "24")
		base_k24 (graph, hierarchy, nEdge, K, &maxK, vfile, fp);
	else if (nd == "34")
		base_k34 (graph, hierarchy, nEdge, K, &maxK, vfile, fp);

#ifdef K_VALUES
	string kfile = vfile + "_K_values";
	FILE* kf = fopen (kfile.c_str(), "w");
	for (vertex i = 0; i < K.size(); i++)
		fprintf (kf, "%lld\n", K[i]);
	fclose (kf);
#endif

	timestamp t2;
	field(false, "K_max", maxK);
	field(false, "app_time_sec", t2 - t1);
	endObject();
	lastObject();
	fclose (fp);

	g_jsonSS = nullptr;
	std::cout << jsonSS.str() << std::endl;

	return 0;
}
