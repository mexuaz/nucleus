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

// Report the per-nucleus subgraphs (the *_NUCLEI and *_Hierarchy files) only when
// asked for via the NUCLEUS_REPORT_SUBGRAPH env var. This is expensive for large
// graphs, so it stays off by default to preserve the timing-run behavior.
// Enable with NUCLEUS_REPORT_SUBGRAPH=1 (or yes/true/on).
inline bool report_subgraph_enabled()
{
	std::string v = env("NUCLEUS_REPORT_SUBGRAPH");
	for (auto& c : v) c = std::tolower(static_cast<unsigned char>(c));
	return v == "1" || v == "yes" || v == "true" || v == "on";
}

int main (int argc, char *argv[]) {

	timestamp t1;
	if (argc < 3) {
		cerr << "usage: " << argv[0] << 
				"\n <filename>"
				"\n <nucleus type: 12, 13, 14, 23, 24, 34>"
				"\n [k_values_file] (optional, if provided K values are written to this file)"
				"\n [hierarchy: YES or NO] (optional, defaults to NO)" << endl;
		exit(1);
	}
		
	std::ostringstream jsonSS;

	try {
	
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

		bool hierarchy = false;
		if (argc >= 5) {
			string hrc (argv[4]);
			if (hrc == "YES")
				hierarchy = true;
			else if (hrc != "NO") {
				cerr << "Invalid hierarchy option, options are YES and NO" << endl;
				exit(1);
			}
		}
		
		string out_file;
		if (hierarchy)
			out_file = vfile + "_Hierarchy";
		else
			out_file = vfile + "_K";

		const bool report_subgraph = report_subgraph_enabled();
		FILE* fp = nullptr;
		if(report_subgraph) {
			fp = fopen (out_file.c_str(), "w");
		}

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

		if (argc >= 4) {
			const char* kfile = argv[3];
			FILE* kf = fopen (kfile, "w");
			if (!kf) {
				cerr << "Could not open K values file for writing: " << kfile << endl;
			} else {
				for (vertex i = 0; i < K.size(); i++)
					fprintf (kf, "%lld\n", K[i]);
				fclose (kf);
			}
		}

		timestamp t2;
		field(false, "K_max", maxK);
		field(false, "app_time_sec", t2 - t1);
		endObject();
		lastObject();

		if(fp)
			fclose (fp);

		g_jsonSS = nullptr;
		std::cout << jsonSS.str() << std::endl;

	} catch (const std::exception& e) {
        g_jsonSS = nullptr;
        cerr << "ERROR: " << e.what() << std::endl;
		cerr << "Json so far: " << std::endl << jsonSS.str() << std::endl;
        return 1;
    } catch (...) {
        g_jsonSS = nullptr;
        cerr << "ERROR: unknown exception" << std::endl;
        cerr << "Json so far: " << std::endl << jsonSS.str() << std::endl;
        return 1;
    }

	return 0;
}
