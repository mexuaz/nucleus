#include "main.h"

#include <iterator>
#include <stdexcept>

#define MAXLINE 1000000
//#define WRITE_BINARY

typedef struct asdf {
	int f;
	int s;
} pprr;

int pcmp(const void *v1, const void *v2) {
	vertex diff = (((pprr *)v1)->f - ((pprr *)v2)->f);
	if (diff != 0)
		return diff;
	else
		return (((pprr *)v1)->s - ((pprr *)v2)->s);
}

static int really_read(std::istream& is, char* buf, size_t global_size) {
	char* temp2 = buf;
	while (global_size != 0) {
		is.read(temp2, global_size);
		size_t s = is.gcount();
		if (!is)
			return -1;

		global_size -= s;
		temp2 += s;
	}
	return 0;
}

template <typename VtxType, typename EdgeType>
void writeBinary (char* filename, VtxType nVtx, EdgeType nEdge, vector<vector<VtxType>>& graph) {

	string str(filename);
	string fl = str + ".bin";
	FILE* filep = fopen (fl.c_str(), "w");
	int vtxt = sizeof (VtxType);
	int edget = sizeof (EdgeType);
	fwrite (&vtxt, sizeof(int), 1, filep);
	fwrite (&edget, sizeof(int), 1, filep);

	fwrite (&nVtx, sizeof(VtxType), 1, filep);
	nEdge /= 2;
	fwrite (&nEdge, sizeof(EdgeType), 1, filep);
	nEdge *= 2;

	for (VtxType i = 0; i < nVtx; i++) {
		VtxType sz = graph[i].size();
		fwrite (&sz, sizeof(VtxType), 1, filep);
	}

	for (VtxType i = 0; i < nVtx; i++) {
		size_t sz = graph[i].size();
		fwrite (&(graph[i][0]), sizeof(VtxType), sz, filep);
	}

	fclose (filep);
}

template <typename VtxType, typename EdgeType>
void readBinary(char *filename, vector<vector<VtxType>>& graph, EdgeType* nEdge) {

	ifstream in (filename);
	int vtxsize; //in bytes
	int edgesize; //in bytes

	//reading header
	in.read((char *)&vtxsize, sizeof(int));
	in.read((char *)&edgesize, sizeof(int));

	if (!in) {
		cerr << "IOError" << endl;
		return;
	}

	if (vtxsize != sizeof(VtxType)) {
		cerr << "Incompatible VertexSize." << endl;
		return;
	}

	if (edgesize != sizeof(EdgeType)) {
		cerr << "Incompatible EdgeSize." << endl;
		return;
	}

	vertex nVtx;
	in.read((char*)&nVtx, sizeof(VtxType));
	in.read((char*)nEdge, sizeof(EdgeType));
//	*nEdge *= 2;

	printf ("nVtx: %d   nEdge:%d\n", nVtx, *nEdge);
	graph.resize (nVtx);
	EdgeType *pxadj = (EdgeType*) malloc (sizeof(EdgeType) * nVtx);
	really_read(in, (char*)pxadj, sizeof(EdgeType) * nVtx);
	for (vertex i = 0; i < nVtx; i++) {
		graph[i].resize (pxadj[i]);
		really_read (in, (char*)&(graph[i][0]), sizeof(VtxType) * pxadj[i]);
	}
}

template <typename VtxType, typename EdgeType>
void readChaco (char *filename, Graph& graph, EdgeType* nEdge) {

	char* line = (char*) malloc (sizeof (char) * MAXLINE);
	FILE* matfp = fopen(filename, "r");

	// skip comments
	do {
		fgets(line, MAXLINE, matfp);
	} while (line[0] == '%' || line[0] == '#');

	VtxType nVtx, neig;
	string s = line;
	stringstream ss (s);
	ss >> nVtx >> *nEdge;
	nVtx += 1; // Since our graphs are zero-based

	graph.resize (nVtx);
	// read each edge list
	for (VtxType i = 0; i < nVtx; i++) {
		fgets(line, MAXLINE, matfp);
		stringstream ss (line);
		while (ss >> neig)
			graph[i].push_back (neig);
	}

	// sort each neighbor list
	for(VtxType i = 0; i < nVtx; i++)
		hashUniquify (graph[i]);

	fclose (matfp);
}

template <typename VtxType, typename EdgeType>
void readMM (char *filename, Graph& graph, EdgeType* nEdge) {

	std::ifstream fs(filename);
	if (!fs.is_open()) {
		throw std::runtime_error(std::string("Error opening file ") + filename);
	}

	std::string line;
	// skip comments
	do {
		std::getline(fs, line);
	} while (!line.empty() && (line[0] == '%' || line[0] == '#'));
	
	VtxType nVtx;
	std::istringstream iss (line);
	std::vector<std::string> tokens{
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{}};

	if (tokens.size()==2) {
		nVtx = std::stoi(tokens[0]);
		*nEdge = std::stoi(tokens[1]);
	} else if(tokens.size()==3) {
		auto nVtx1 = std::stoi(tokens[0]);
		auto nVtx2 = std::stoi(tokens[1]);
		nVtx = std::max(nVtx1, nVtx2);
		*nEdge = std::stoi(tokens[2]);
	} else {
		std::cerr << "MMX format with " << tokens.size() << " tokens not supported!" << std::endl;
		std::cerr << "line: " << line << std::endl;
		exit(1);
	}
	nVtx++; // Since our graphs are zero-based
	
	// remove duplicate edges, take one direction
	std::vector<pprr> coords(2 * *nEdge);
	VtxType itemp, jtemp, index = 0;

	for (EdgeType i = 0; i < *nEdge; i++) {
 		if (!std::getline(fs, line)) {
 			throw std::runtime_error(
 				std::string("Unexpected end of file while reading edges from ") + filename);
 		}
 		stringstream ss (line);
 		if (!(ss >> itemp >> jtemp)) {
 			throw std::runtime_error(
 				std::string("Invalid edge line in ") + filename + ": " + line);
 		}
		if(itemp != jtemp) {
			coords[index].f = coords[index + 1].s = itemp;
			coords[index + 1].f = coords[index].s = jtemp;
			index += 2;
		}
	}
	fs.close();

	qsort(coords.data(), index, sizeof(pprr), pcmp);

	VtxType onnz = 1; // onnz is # of edges
	for(EdgeType i = 1; i < index; i++) {
		if(coords[i].f != coords[onnz-1].f || coords[i].s != coords[onnz-1].s) {
			coords[onnz].f = coords[i].f;
			coords[onnz++].s = coords[i].s;
		}
	}

	// begin constructing graph
	graph.resize (nVtx);
	for(EdgeType i = 0; i < onnz; i++)
		graph[coords[i].f].push_back(coords[i].s);

	// sort each neighbor list
	edge numedge = 0;
	for(VtxType i = 0; i < nVtx; i++) {
		sort (graph[i].begin(), graph[i].end());
		numedge += graph[i].size();
	}
	*nEdge = numedge;


}

template <typename VtxType, typename EdgeType>
void readOut (char *filename, Graph& graph, EdgeType* nEdge) {

	std::ifstream fs(filename);
	if (!fs.is_open()) {
		throw std::runtime_error(std::string("Error opening file ") + filename);
	}

	// skip comments
	std::string line;
	do {
		std::getline(fs, line);
	} while (!line.empty() && (line[0] == '%' || line[0] == '#'));

	VtxType u, v;
	string dum, sum;
	unordered_map<pair<int, int>, bool> mp;
	VtxType maxV = 0;
	while (std::getline(fs, line)) {
		stringstream ss (line);
		ss >> u >> v;
		if (u > v)
			swap (u, v);
		auto at = mp.find (make_pair (u, v));
		if (at == mp.end()) {
			mp.emplace (make_pair(make_pair(u, v), true));

			if (v > maxV) {
				graph.resize (v+1);
				maxV = v;
			}
			graph[u].push_back (v);
			graph[v].push_back (u);
			(*nEdge)++;
		}
	}
	for (auto& w : graph)
		sort (w.begin(), w.end());
	fs.close();

}

template <typename VtxType, typename EdgeType>
void readGraph (char *filename, vector<vector<VtxType>>& graph, EdgeType* nEdge) {

	string st (filename);
	string gname = st.substr (st.find_last_of("/") + 1);
	int idx = gname.find_last_of(".");
	string ext = gname.substr(idx);

	if (ext == ".bin")
		readBinary<VtxType, EdgeType> (filename, graph, nEdge);
	else if (ext == ".graph")
		readChaco<VtxType, EdgeType> (filename, graph, nEdge);
	else if (gname.find("out") == 0 || ext == ".txt" || ext == ".edges") {
		readOut<VtxType, EdgeType> (filename, graph, nEdge);
	}
	else {
		// .mtx
		readMM<VtxType, EdgeType> (filename, graph, nEdge);
	}

#ifdef WRITE_BINARY
	if (ext != ".bin") {
		vertex nVtx = graph.size();
		writeBinary (filename, nVtx, *nEdge, graph);
		printf ("Binary graph is written\n");
	}
#endif

	return;
}

template void readGraph (char *filename, vector<vector<vertex>>& graph, edge* nEdge);

