#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include "../system/serialization.h"
#include "../system/ioser.h"
#include "data.h"
#include <map>

#pragma warning (disable:4996)

double gdmin_deg_ratio;
int gnmin_size;
int gnmax_size;
int gnmin_deg;
//Guimu-condense
int *index2id;

struct VERTEX // variables start with 'b' is boolean, 'n' is integer
{
	int nvertex_no; // vertex ID
	int nclique_deg; // degree in the context of X
	int ncand_deg; // degree in the context of cand(X)
	int nlvl2_nbs;
	bool bis_cand;
	bool bto_be_extended;
};

obinstream & operator>>(obinstream & m, VERTEX & v)
{
    m >> v.nvertex_no;
    m >> v.nclique_deg;
    m >> v.ncand_deg;
    m >> v.nlvl2_nbs;
    m >> v.bis_cand;
    m >> v.bto_be_extended;
    return m;
}

ibinstream & operator<<(ibinstream & m, const VERTEX & v)
{
	m << v.nvertex_no;
	m << v.nclique_deg;
	m << v.ncand_deg;
	m << v.nlvl2_nbs;
	m << v.bis_cand;
	m << v.bto_be_extended;
	return m;
}

ofbinstream & operator>>(ofbinstream & m, VERTEX & v)
{
	m >> v.nvertex_no;
	m >> v.nclique_deg;
	m >> v.ncand_deg;
	m >> v.nlvl2_nbs;
	m >> v.bis_cand;
	m >> v.bto_be_extended;
	return m;
}

ifbinstream & operator<<(ifbinstream & m, const VERTEX & v)
{
	m << v.nvertex_no;
	m << v.nclique_deg;
	m << v.ncand_deg;
	m << v.nlvl2_nbs;
	m << v.bis_cand;
	m << v.bto_be_extended;
	return m;
}

#define CG_PAGE_SIZE (1<<12)
#define MAX_COND_GRAPH 5

struct INT_PAGE
{
	int parray[CG_PAGE_SIZE];
	INT_PAGE *pnext;
};

struct COND_GRAPH_BUF
{
	INT_PAGE *phead;
	INT_PAGE *pcur_page;
	int ncur_pos;
	int ntotal_pages;
};

struct COND_GRAPH
{
	int **ppadj_lists;
	int **pplvl2_nbs;
	int num_of_vertices;
	INT_PAGE *pcur_page;
	int ncur_pos;
};

struct CLQ_STAT //lower and upper bounds: see Lines 602-607 of graph.cpp
{
	int nmin_cands; //L_X
	int nmax_cands; //U_X
	int nmin_ext_deg;
};

class Graph
{
public:

	// prefix 'm' means it's member of Graph, 'p' is a pointer, 'pp' is a pointer (array) of pointers
	//**mppadj_lists, *mpadj_list_buf, **mpplvl2_nbs are only be used when there is no conditional graphs.

	FILE *gfpout;

	int **mppadj_lists, *mpadj_list_buf, **mpplvl2_nbs; // mppadj_lists[i] = node i's 1-hop neighbors, or more accurately, pointer to the adjlist segment in mpadj_list_buf;    mpplvl2_nbs[i] = node i's 2-hop neighbor array
	int mnum_of_vertices; // total vertex number
	COND_GRAPH *mpcond_graphs; // conditional graphs (shrinked graph)
	int mnum_of_cond_graphs, mnmax_cond_graphs;
	bool mblvl2_flag; // whether need to check 2-hop neighbors, set by Graph::LoadGraph(.)


	int *gpvertex_order_map; // map[vertex_no] = position in pvertices[.] (array of all vertices in X and cand), used to get Vertex object given ID from nb-list of other vertices
	int *gpremoved_vertices;
	int *gpremain_vertices;
	int *gptemp_array;
	int *gpcounters;

	COND_GRAPH_BUF gocondgraph_buf;
	int **gpcondgraph_listpt_buf;

	int *gpmin_degs;
	struct timeb gtime_start;

	//-------

	Graph()
	{
		mppadj_lists = NULL;
		mpplvl2_nbs = NULL;
	}

//	int LoadGraph(char* szgraph_file);
//	Graph(const Graph &obj);
//	void operator=(const Graph &obj);
	void SetupGraph(int nclique_size, int num_of_cands, int num_of_tail_vertices);
	void ClearGraph();
	void CompressGraph(VERTEX *pvertices, int num_of_cands);
	void GenLevel2NBs();
	int LoadGraph(char* szgraph_file);
	void DestroyGraph();
	void DestroySplitGraph();
	void OutputLvl2Graph(char* szoutput_filename);


//	int Cliques(map<VertexID, kc_value> & gmap, FILE *gfpout);
//	int Cliques(char *szgraph_filename, char* szoutput_filename);
//	int Cliques(char *szgraph_filename, VERTEX *global_pvertices);
	VERTEX *Cliques(char *szgraph_filename, int & num_of_cands);
	int Expand(VERTEX *pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, FILE *gfpout);
	int ExpandOnce(VERTEX *pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, FILE *gfpout);

	int RemoveCandVertex(VERTEX *pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, int ncur_pos);
	bool Lookahead(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, int ncur_pos, VERTEX *pnew_vertices, FILE *gfpout);
	int AddOneVertex(VERTEX *pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, int ncur_pos, bool bhas_tail, VERTEX *pnew_vertices, int &num_of_new_tails, CLQ_STAT *pclq_stat);
	void CrtcVtxPrune(VERTEX *pvertices, int &nclique_size, int &num_of_cands, int &num_of_tail_vertices, VERTEX *pclique, CLQ_STAT *pclq_stat);
	bool GenCondGraph(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices);
//	bool ForceGenCondGraph(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, Graph& split_g);
	void ForceGenCondGraph1(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, Graph& split_g);
	void ForceGenCondGraph(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, Graph& split_g);
	void DelCondGraph();
	int ReduceCands(VERTEX *pvertices, int nclique_size, int num_of_cands, bool &bis_subsumed);
	int GenTailVertices(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, int ncur_pos, VERTEX *pnew_vertices, int nnew_clique_size); // cover vertex pruning
	int ReduceTailVertices(VERTEX* pvertices, int nclique_size, int num_of_tail_vertices, int **ppadj_lists);
	void OutputOneClique(VERTEX *pvertices, int nclique_size, int num_of_tail_vertices, FILE *gfpout);
	bool CheckMaximal(VERTEX* pvertices, int nclique_size, int num_of_exts);
	void CalcLUBound(VERTEX *pvertices, int nclique_size, int *pcand_clqdegs, int num_of_valid_cands, CLQ_STAT *pclq_stat);

	void VerifyVertices(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, bool bonly_tobeextended);

	void DelCGIntBuf();

	inline int* NewCGIntArray(int nlen);
	inline void Output1Clique(VERTEX *pclique, int nclique_size, FILE *gfpout);
	inline int GetMinDeg(int nclique_size);
	inline bool IsValidCand(VERTEX* pvertex, int nclique_size, CLQ_STAT *pclq_stat);

	inline int CalcMaxExts(int nmin_clq_totaldeg, int nclique_size, int num_of_cands);
	inline void CalcMinDegs(int *pmin_degs, int num_of_vertices);

	friend ibinstream& operator<<(ibinstream& m, const Graph& g)
	{
		int mnum_of_vertices = g.mnum_of_vertices;
		m << mnum_of_vertices;
		m << g.mblvl2_flag;

		//int **mppadj_lists, *mpadj_list_buf, **mpplvl2_nbs;
		vector<char> adj_size_vec;
		for(int i = 0; i < mnum_of_vertices; i++)
		{
			if(g.mppadj_lists[i] != NULL)
				adj_size_vec.push_back('1');
			else
				adj_size_vec.push_back('0');
		}
		m << adj_size_vec;

		for(int i = 0; i < mnum_of_vertices; i++)
		{
			if(g.mppadj_lists[i] != NULL)
			{
				int size = g.mppadj_lists[i][0];
				m << size;
				for(int j = 1; j <= size; j++)
					m << g.mppadj_lists[i][j];
			}
		}

		if(g.mblvl2_flag)
		{
			vector<char> lvl2_size_vec;
			for(int i = 0; i < mnum_of_vertices; i++)
			{
				if(g.mpplvl2_nbs[i] != NULL)
					lvl2_size_vec.push_back('1');
				else
					lvl2_size_vec.push_back('0');
			}
			m << lvl2_size_vec;

			for(int i = 0; i < mnum_of_vertices; i++)
			{
				if(g.mpplvl2_nbs[i] != NULL)
				{
					int size = g.mpplvl2_nbs[i][0];
					m << size;
					for(int j = 1; j <= size; j++)
						m << g.mpplvl2_nbs[i][j];
				}
			}
		}
		return m;
	}

	friend obinstream& operator>>(obinstream& m, Graph& g)
	{
		int mnum_of_vertices;
		m >> mnum_of_vertices;
		g.mnum_of_vertices = mnum_of_vertices;
		m >> g.mblvl2_flag;

		vector<char> adj_size_vec;

		m >> adj_size_vec;

		int **ppadj_lists;
		ppadj_lists = new int*[mnum_of_vertices];

		memset(ppadj_lists, 0, sizeof(int*)*mnum_of_vertices);
		for(int i = 0; i < adj_size_vec.size(); i++)
		{
			if(adj_size_vec[i] == '1')
			{
				int size;
				m >> size;
				ppadj_lists[i] = new int[size+1];
				ppadj_lists[i][0] = size;
				for(int j = 1; j <= size; j++)
					m >> ppadj_lists[i][j];
			}

		}
		g.mppadj_lists = ppadj_lists;

		if(g.mblvl2_flag)
		{
			vector<char> lvl2_size_vec;
			m >> lvl2_size_vec;

			int **pplvl2_nbs;
			pplvl2_nbs = new int*[mnum_of_vertices];
			memset(pplvl2_nbs, 0, sizeof(int*)*mnum_of_vertices);
			for(int i = 0; i < lvl2_size_vec.size(); i++)
			{
				if(lvl2_size_vec[i] == '1')
				{
					int size;
					m >> size;
					pplvl2_nbs[i] = new int[size+1];
					pplvl2_nbs[i][0] = size;
					for(int j = 1; j <= size; j++)
						m >> pplvl2_nbs[i][j];
				}

			}
			g.mpplvl2_nbs = pplvl2_nbs;
		}

		return m;
	}

	friend ifbinstream& operator<<(ifbinstream& m,  const Graph& g)
	{
		int mnum_of_vertices = g.mnum_of_vertices;
		m << mnum_of_vertices;
		m << g.mblvl2_flag;

		//int **mppadj_lists, *mpadj_list_buf, **mpplvl2_nbs;
		vector<char> adj_size_vec;
		for(int i = 0; i < mnum_of_vertices; i++)
		{
			if(g.mppadj_lists[i] != NULL)
				adj_size_vec.push_back('1');
			else
				adj_size_vec.push_back('0');
		}
		m << adj_size_vec;

		for(int i = 0; i < mnum_of_vertices; i++)
		{
			if(g.mppadj_lists[i] != NULL)
			{
				int size = g.mppadj_lists[i][0];
				m << size;
				for(int j = 1; j <= size; j++)
					m << g.mppadj_lists[i][j];
			}
		}

		if(g.mblvl2_flag)
		{
			vector<char> lvl2_size_vec;
			for(int i = 0; i < mnum_of_vertices; i++)
			{
				if(g.mpplvl2_nbs[i] != NULL)
					lvl2_size_vec.push_back('1');
				else
					lvl2_size_vec.push_back('0');
			}
			m << lvl2_size_vec;

			for(int i = 0; i < mnum_of_vertices; i++)
			{
				if(g.mpplvl2_nbs[i] != NULL)
				{
					int size = g.mpplvl2_nbs[i][0];
					m << size;
					for(int j = 1; j <= size; j++)
						m << g.mpplvl2_nbs[i][j];
				}
			}
		}
		return m;
	}

	friend ofbinstream& operator>>(ofbinstream& m,  Graph& g)
	{
		int mnum_of_vertices;
		m >> mnum_of_vertices;
		g.mnum_of_vertices = mnum_of_vertices;
		m >> g.mblvl2_flag;

		vector<char> adj_size_vec;

		m >> adj_size_vec;

		int **ppadj_lists;
		ppadj_lists = new int*[mnum_of_vertices];

		memset(ppadj_lists, 0, sizeof(int*)*mnum_of_vertices);
		for(int i = 0; i < adj_size_vec.size(); i++)
		{
			if(adj_size_vec[i] == '1')
			{
				int size;
				m >> size;
				ppadj_lists[i] = new int[size+1];
				ppadj_lists[i][0] = size;
				for(int j = 1; j <= size; j++)
					m >> ppadj_lists[i][j];
			}

		}
		g.mppadj_lists = ppadj_lists;

		if(g.mblvl2_flag)
		{
			vector<char> lvl2_size_vec;
			m >> lvl2_size_vec;

			int **pplvl2_nbs;
			pplvl2_nbs = new int*[mnum_of_vertices];
			memset(pplvl2_nbs, 0, sizeof(int*)*mnum_of_vertices);
			for(int i = 0; i < lvl2_size_vec.size(); i++)
			{
				if(lvl2_size_vec[i] == '1')
				{
					int size;
					m >> size;
					pplvl2_nbs[i] = new int[size+1];
					pplvl2_nbs[i][0] = size;
					for(int j = 1; j <= size; j++)
						m >> pplvl2_nbs[i][j];
				}

			}
			g.mpplvl2_nbs = pplvl2_nbs;
		}

		return m;
	}
};

//void Graph::operator=(const Graph &obj)
//{
//	mnum_of_vertices = obj.mnum_of_vertices;
//	mblvl2_flag = obj.mblvl2_flag;
//
//	index2id = new int[obj.mnum_of_vertices];
//	memcpy(index2id, obj.index2id, sizeof(int)*obj.mnum_of_vertices);
//
////-------------
//	/*
//	// need to be handled in ForceGenCondGraph()
//
////force func:
//	int **mppadj_lists, *mpadj_list_buf, **mpplvl2_nbs; //(1) mppadj_lists[i] = node i's 1-hop neighbors, or more accurately, pointer to the adjlist segment in mpadj_list_buf;    mpplvl2_nbs[i] = node i's 2-hop neighbor array
//	int mnum_of_vertices; //(2) total vertex number
//
//	COND_GRAPH *mpcond_graphs; //(3) conditional graphs ->round 3
//	int mnum_of_cond_graphs, mnmax_cond_graphs; //(4) ->round 3
//	bool mblvl2_flag; //(5) whether need to check 2-hop neighbors, set by Graph::LoadGraph(.)
//
//
//	int *gpvertex_order_map; //(6) ->round 3
//	int *gpremoved_vertices; //(7) ->round 3
//	int *gpremain_vertices; //(8) ->round 3
//	int *gptemp_array; //(9) ->round 3
//	int *gpcounters; //(10) ->round 3
//
//	COND_GRAPH_BUF gocondgraph_buf; //(11) ->round 3
//	int **gpcondgraph_listpt_buf; //(12) ->round 3
//
//	int *gpmin_degs; //(13)
//	struct timeb gtime_start; //(14) ->round 3
//
//	int *index2id; //(15)
//	*/
//}

void Graph::SetupGraph(int nclique_size, int num_of_cands, int num_of_tail_vertices)
{
	int num_of_vertices = nclique_size + num_of_cands + num_of_tail_vertices;
	gpmin_degs = new int[mnum_of_vertices+1];
	CalcMinDegs(gpmin_degs, mnum_of_vertices);
	// the following open the space once and initialize again when reused in func
	gptemp_array = new int[num_of_vertices];
	gpremain_vertices = new int[num_of_vertices];
	gpremoved_vertices = new int[num_of_vertices];
	gpvertex_order_map  = new int[mnum_of_vertices];
	memset(gpvertex_order_map, -1, sizeof(int)*num_of_vertices);


	mnmax_cond_graphs = MAX_COND_GRAPH;
	mpcond_graphs = new COND_GRAPH[mnmax_cond_graphs];
	mnum_of_cond_graphs = 0;
	gocondgraph_buf.phead = new INT_PAGE;
	gocondgraph_buf.phead->pnext = NULL;
	gocondgraph_buf.pcur_page = gocondgraph_buf.phead;
	gocondgraph_buf.ncur_pos = 0;
	gocondgraph_buf.ntotal_pages = 1;
	gpcondgraph_listpt_buf = new int*[mnum_of_vertices*2*MAX_COND_GRAPH];
	gpcounters = new int[num_of_vertices];

}

void Graph::ClearGraph()
{
	delete []gpcounters;
	DelCGIntBuf();
	delete []gpcondgraph_listpt_buf;
	delete []mpcond_graphs;
	if(mnum_of_cond_graphs!=0)
		printf("Error: the number of conditiaonal database should be 0\n");

	delete []gpremoved_vertices;
	delete []gpremain_vertices;
	delete []gpvertex_order_map;
	delete []gptemp_array;
	delete []gpmin_degs;
}

//------------------------------------------------------------------------
//extern COND_GRAPH_BUF gocondgraph_buf;
//extern int** gpcondgraph_listpt_buf;

inline int* Graph::NewCGIntArray(int nlen)
{
	INT_PAGE *pnew_page;
	int *parray;

	if(nlen>CG_PAGE_SIZE)
		return new int[nlen]; //new an array directly if nlen > PageSize
	else if(gocondgraph_buf.ncur_pos+nlen>CG_PAGE_SIZE)
	{
		if(gocondgraph_buf.pcur_page->pnext==NULL)
		{
			pnew_page = new INT_PAGE;
			pnew_page->pnext = NULL;
			gocondgraph_buf.pcur_page->pnext = pnew_page;
			gocondgraph_buf.pcur_page = pnew_page;
			gocondgraph_buf.ntotal_pages++;
		}
		else
			gocondgraph_buf.pcur_page = gocondgraph_buf.pcur_page->pnext;

		gocondgraph_buf.ncur_pos = 0;
	}

	parray = &gocondgraph_buf.pcur_page->parray[gocondgraph_buf.ncur_pos];
	gocondgraph_buf.ncur_pos += nlen;

	return parray;
}





//===============================================================
int comp_int(const void *e1, const void *e2);
int comp_int_des(const void* e1, const void *e2);
int comp_vertex_freq(const void *e1, const void *e2);
int comp_vertex_clqdeg(const void *e1, const void *e2);

//
//extern FILE *gfpout; // output file for keeping QCs


inline int Graph::CalcMaxExts(int nmin_clq_totaldeg, int nclique_size, int num_of_cands)
{
	return (int)(nmin_clq_totaldeg/gdmin_deg_ratio)-nclique_size;
//	return num_of_cands;
}


inline void Graph::CalcMinDegs(int *pmin_degs, int num_of_vertices)
{
	int i, nmin_deg;

	pmin_degs[0] = 0;
	for(i=1;i<=num_of_vertices;i++)
	{
		nmin_deg = (int)(gdmin_deg_ratio*(i-1));
		if(nmin_deg<gdmin_deg_ratio*(i-1))
			nmin_deg++;
		pmin_degs[i] = nmin_deg; //^^^^^ precomputed here
	}
}

inline int Graph::GetMinDeg(int nclique_size)
{
	if(nclique_size<=gnmin_size)
		return gnmin_deg; // Q cannot be less the min-size, so not point to have a smaller degree than r * min_size
	else
		return gpmin_degs[nclique_size]; //^^^^^ use precomputed mindeg for the particular clique-size
}

inline bool Graph::IsValidCand(VERTEX* pvertex, int nclique_size, CLQ_STAT *pclq_stat)
{
	if(pvertex->nclique_deg+pvertex->ncand_deg<pclq_stat->nmin_ext_deg)
		return false;
	else if(pvertex->nclique_deg+pvertex->ncand_deg<GetMinDeg(nclique_size+pvertex->ncand_deg+1))
		return false;
	else if(pvertex->nclique_deg+pclq_stat->nmax_cands-1<gpmin_degs[nclique_size+pclq_stat->nmax_cands])
		return false;
	else if(pvertex->nclique_deg+pvertex->ncand_deg<GetMinDeg(nclique_size+pclq_stat->nmin_cands))
		return false;
	else
		return true;
}

inline void Graph::Output1Clique(VERTEX *pclique, int nclique_size, FILE *gfpout)
{
	int i;
	fprintf(gfpout, "%d ", nclique_size);
	for(i=0; i<nclique_size; i++) 
		fprintf(gfpout, "%d ", index2id[pclique[i].nvertex_no]);
	fprintf(gfpout, "\n");

}

//Guimu-condense:
void Graph::CompressGraph(VERTEX *pvertices, int num_of_cands)
{
	int i, j, nvertex_no, norder;
	for(i=0;i<num_of_cands;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i;

	int nlist_len, ncand_nbs, **ppnew_adjlists, **ppnew_lvl2_nbs, v_index;
	ppnew_adjlists = new int*[num_of_cands];
	ppnew_lvl2_nbs = new int*[num_of_cands];

	//translate candidate vertex id to index
	map<int, int> id2index_map;
	index2id = new int[num_of_cands];
	for(i=0;i<num_of_cands;i++)
	{
		nvertex_no = pvertices[i].nvertex_no;
		index2id[i] = nvertex_no;
		id2index_map[nvertex_no] = i;
	}

	//reset 2hop adj
	for(i=0;i<num_of_cands;i++)
	{
		nlist_len = 0;
		ncand_nbs = 0;
		nvertex_no = pvertices[i].nvertex_no;
		v_index = id2index_map[nvertex_no];
		int* adj_2hop = mpplvl2_nbs[nvertex_no];
		if(adj_2hop!=NULL)
		{
			for(j=1;j<=adj_2hop[0];j++)
			{
				norder = gpvertex_order_map[adj_2hop[j]];
				if(norder>=0)
				{
					//add nb's translated id
					gptemp_array[nlist_len++] = id2index_map[adj_2hop[j]];
					ncand_nbs++;
				}
			}
		}
		if(nlist_len>0)
		{
			ppnew_lvl2_nbs[v_index] = new int[nlist_len+1];
			ppnew_lvl2_nbs[v_index][0] = nlist_len;
			memcpy(&ppnew_lvl2_nbs[v_index][1], gptemp_array, sizeof(int)*nlist_len);
		}
		pvertices[i].nlvl2_nbs = ncand_nbs;
	}


	//reset 1hop adj
	for(i=0;i<num_of_cands;i++)
	{
		nlist_len = 0;
		ncand_nbs = 0;
		nvertex_no = pvertices[i].nvertex_no;
		v_index = id2index_map[nvertex_no];
		int* adj_1hop = mppadj_lists[nvertex_no];
		if(adj_1hop!=NULL)
		{
			for(j=1;j<=adj_1hop[0];j++)
			{
				norder = gpvertex_order_map[adj_1hop[j]];
				if(norder>=0)
				{
					gptemp_array[nlist_len++] = id2index_map[adj_1hop[j]];
					ncand_nbs++;
				}
			}
		}
		//check cand degree
		if(pvertices[i].ncand_deg!=ncand_nbs)
			printf("Error: inconsistent candidate degree\n");
		if(nlist_len>0)
		{
			ppnew_adjlists[v_index] = new int[nlist_len+1];
			ppnew_adjlists[v_index][0] = nlist_len;
			memcpy(&ppnew_adjlists[v_index][1], gptemp_array, sizeof(int)*nlist_len);
		}
	}

	//translate pvertex
	for(i=0;i<num_of_cands;i++)
		pvertices[i].nvertex_no = i;

	DestroyGraph();
	mppadj_lists = ppnew_adjlists;
	mpplvl2_nbs = ppnew_lvl2_nbs;

	mnum_of_vertices = num_of_cands;
}


VERTEX * Graph::Cliques(char *szgraph_filename, int & num_of_cands)
{
	int i, j, num_of_vertices, nmax_deg, nmaxdeg_vertex;
	int nrm_start, nrm_end, nvertex_no, norder, num_of_freq_cands;
	struct timeb start, end;
	VERTEX *pvertices, onevertex;


	num_of_vertices = LoadGraph(szgraph_filename);
	gpmin_degs = new int[num_of_vertices+1];
	CalcMinDegs(gpmin_degs, num_of_vertices); // precompute min-deg threshold for different clique size
	gpremoved_vertices = new int[num_of_vertices]; // ---1---: those vertices that fail the check: degree >= r * (min_size - 1)  AND  |vertices_within-2hops| > min_size - 1
	gpremain_vertices = new int[num_of_vertices]; // ---2---: those vertices that pass the check: degree >= r * (min_size - 1)  AND  |vertices_within-2hops| > min_size - 1
	gpvertex_order_map  = new int[num_of_vertices]; // map[vertex_no] = position in pvertices[.], this is needed as vertices change their positions in pvertices[.]
	memset(gpvertex_order_map, -1, sizeof(int)*num_of_vertices);
	gptemp_array = new int[num_of_vertices];


	//=== Setup VERTEX and recursive degree pruning ===

	pvertices = new VERTEX[num_of_vertices];
	num_of_cands = num_of_vertices;
	num_of_freq_cands = 0;
	nrm_start = 0;
	nrm_end = 0;
	for(i=0;i<num_of_vertices;i++)
	{
		pvertices[i].nvertex_no = i;
		pvertices[i].nclique_deg = 0;
		pvertices[i].ncand_deg = mppadj_lists[i][0];//exdeg is the degree at the beginning
		if(mblvl2_flag)
			pvertices[i].nlvl2_nbs = mpplvl2_nbs[i][0];// set 2hop size
		else
			pvertices[i].nlvl2_nbs = 0;
		if(mppadj_lists[i][0]>=gnmin_deg && (!mblvl2_flag || mblvl2_flag && mpplvl2_nbs[i][0]>=gnmin_size-1)) // degree >= r * (min_size - 1)  AND  |vertices_within-2hops| > min_size - 1
		{
			pvertices[i].bis_cand = true;//all is candidate at the beginning
			pvertices[i].bto_be_extended = true;
			gpremain_vertices[num_of_freq_cands++] = i;//record in remain_array
		}
		else
		{
			pvertices[i].bis_cand = false;
			pvertices[i].bto_be_extended = false;
			gpremoved_vertices[nrm_end++] = i;//record in remove_array
		}
	}

	//num_of_freq_cands will record how many vertices still remain.
	//num_of_cands = num_of_vertices at the beginning;
	//Update only when less than half of the candidate need to be remained. recursively update
	while(num_of_freq_cands<num_of_cands/2)
	{
		num_of_cands = num_of_freq_cands;
		for(i=0;i<num_of_cands;i++)
		{
			pvertices[gpremain_vertices[i]].ncand_deg = 0;
			pvertices[gpremain_vertices[i]].nlvl2_nbs = 0;
		}
		for(i=0;i<num_of_cands;i++)
		{
			//set the ncand_deg for candidate vertex
			nvertex_no = gpremain_vertices[i];
			for(j=1;j<=mppadj_lists[nvertex_no][0];j++)
			{
				norder = mppadj_lists[nvertex_no][j];
				if(pvertices[norder].bis_cand)//some neighbor may not be a candidate
					pvertices[norder].ncand_deg++;
			}
		}

		//Guimu-2hop-kcore: 2hop re-calculate
		for(i=0;i<num_of_cands;i++)
		{
			//set the ncand_deg for candidate vertex
			nvertex_no = gpremain_vertices[i];
			for(j=1;j<=mpplvl2_nbs[nvertex_no][0];j++)
			{
				norder = mpplvl2_nbs[nvertex_no][j];
				if(pvertices[norder].bis_cand)//some neighbor may not be a candidate
					pvertices[norder].nlvl2_nbs++;
			}
		}

		num_of_freq_cands = 0;
		nrm_start = 0;
		nrm_end = 0;
		//degree was updated in above code. need to remove vertex based on degree recursively here.
		for(i=0;i<num_of_cands;i++)
		{
			norder = gpremain_vertices[i];
			//Guimu-2hop-kcore: add 2hop check
			if(pvertices[norder].ncand_deg>=gnmin_deg && pvertices[norder].nlvl2_nbs >= gnmin_size-1) // degree updated: degree >= r * (min_size - 1)
				gpremain_vertices[num_of_freq_cands++] = norder;
			else
			{
				pvertices[norder].bis_cand = false;
				pvertices[norder].bto_be_extended = false;
				gpremoved_vertices[nrm_end++] = norder;
			}
		}
	}
	num_of_cands = num_of_freq_cands;

	//gpremoved_vertices record the last time removed vertices. Need to update this v's nbs's degree.
	while(nrm_end>nrm_start)
	{
		nvertex_no = gpremoved_vertices[nrm_start];
		for(i=1;i<=mppadj_lists[nvertex_no][0];i++)
		{
			//recursively prune cand when update degree, just like kcore
			norder = mppadj_lists[nvertex_no][i];
			if(pvertices[norder].bis_cand)
			{
				pvertices[norder].ncand_deg--;
				if(pvertices[norder].ncand_deg<gnmin_deg) // degree updated: degree < r * min_size
				{
					pvertices[norder].bis_cand = false;
					pvertices[norder].bto_be_extended = false;
					num_of_cands--;
					gpremoved_vertices[nrm_end++] = norder;
				}
			}
		}

		//Guimu-2hop-kcore: 2hop size kcore pruning
		if(mblvl2_flag)
		{
			for(i=1;i<=mpplvl2_nbs[nvertex_no][0];i++)
			{
				//recursively prune cand when update degree, just like kcore
				norder = mpplvl2_nbs[nvertex_no][i];
				if(pvertices[norder].bis_cand)
				{
					pvertices[norder].nlvl2_nbs--;
					if(pvertices[norder].nlvl2_nbs<gnmin_size-1) // degree updated: degree < r * min_size
					{
						pvertices[norder].bis_cand = false;
						pvertices[norder].bto_be_extended = false;
						num_of_cands--;
						gpremoved_vertices[nrm_end++] = norder;
					}
				}
			}
		}

		nrm_start++;
	}

	//=== first round cover prunning ===

	nmax_deg = 0;
	nmaxdeg_vertex = 0;

	for(i=0;i<num_of_vertices;i++)
	{
		if(pvertices[i].bis_cand)
		{
			if(nmax_deg<pvertices[i].ncand_deg)
			{
				nmax_deg = pvertices[i].ncand_deg;
				nmaxdeg_vertex = i;
			}
		}
	}

	for(i=1;i<=mppadj_lists[nmaxdeg_vertex][0];i++)
		pvertices[mppadj_lists[nmaxdeg_vertex][i]].bto_be_extended = false;

	if(nmaxdeg_vertex!=0) //!!! swap max-degree vertex (ncand_deg) with the first vertex ------> try out to see if this makes a big difference in performance?
	{
		onevertex = pvertices[nmaxdeg_vertex];
		pvertices[nmaxdeg_vertex] = pvertices[0];
		pvertices[0] = onevertex;
	}

	qsort(&pvertices[1], num_of_vertices-1, sizeof(VERTEX), comp_vertex_freq); // sort by pvertices[1, ...] by (nclique_deg, ncand_deg) ------> try out to see if this makes a big difference in performance?

	//Guimu-condense: condense the graph
	CompressGraph(pvertices, num_of_cands);


//	delete []pvertices;
	delete []gpremoved_vertices;
	delete []gpremain_vertices;
//	delete []gpvertex_order_map;
//	delete []gptemp_array;
	delete []gpmin_degs;
//	DestroyGraph();
//	fclose(gfpout);

//	ftime(&end);
	if(num_of_cands>=gnmin_size)
		return pvertices;
	else
		return NULL;
}


//int Graph::Cliques(char *szgraph_filename)
//{
//	int i, j, num_of_vertices, num_of_cands, nmax_deg, nmaxdeg_vertex;
//	int nrm_start, nrm_end, nvertex_no, norder, num_of_freq_cands;
//	struct timeb start, end;
//	VERTEX *pvertices, onevertex;
//
//	ftime(&start);
//	gtime_start = start;
//
////	gfpout = fopen(szoutput_filename, "wt");
////	if(gfpout==NULL)
////	{
////		printf("Error: cannot open file %s for write\n", szoutput_filename);
////		return 0;
////	}
//
//
//
//	num_of_vertices = LoadGraph(szgraph_filename);
//	gpmin_degs = new int[num_of_vertices+1];
//	CalcMinDegs(gpmin_degs, num_of_vertices); // precompute min-deg threshold for different clique size
//	gpremoved_vertices = new int[num_of_vertices]; // ---1---: those vertices that fail the check: degree >= r * (min_size - 1)  AND  |vertices_within-2hops| > min_size - 1
//	gpremain_vertices = new int[num_of_vertices]; // ---2---: those vertices that pass the check: degree >= r * (min_size - 1)  AND  |vertices_within-2hops| > min_size - 1
//	gpvertex_order_map  = new int[num_of_vertices]; // map[vertex_no] = position in pvertices[.], this is needed as vertices change their positions in pvertices[.]
//	memset(gpvertex_order_map, -1, sizeof(int)*num_of_vertices);
//	gptemp_array = new int[num_of_vertices];
//
//
//	//=== Setup VERTEX and recursive degree pruning ===
//
//	pvertices = new VERTEX[num_of_vertices];
//	num_of_cands = num_of_vertices;
//	num_of_freq_cands = 0;
//	nrm_start = 0;
//	nrm_end = 0;
//	for(i=0;i<num_of_vertices;i++)
//	{
//		pvertices[i].nvertex_no = i;
//		pvertices[i].nclique_deg = 0;
//		pvertices[i].ncand_deg = mppadj_lists[i][0];//exdeg is the degree at the beginning
//		if(mblvl2_flag)
//			pvertices[i].nlvl2_nbs = mpplvl2_nbs[i][0];// set 2hop size
//		else
//			pvertices[i].nlvl2_nbs = 0;
//		if(mppadj_lists[i][0]>=gnmin_deg && (!mblvl2_flag || mblvl2_flag && mpplvl2_nbs[i][0]>=gnmin_size-1)) // degree >= r * (min_size - 1)  AND  |vertices_within-2hops| > min_size - 1
//		{
//			pvertices[i].bis_cand = true;//all is candidate at the beginning
//			pvertices[i].bto_be_extended = true;
//			gpremain_vertices[num_of_freq_cands++] = i;//record in remain_array
//		}
//		else
//		{
//			pvertices[i].bis_cand = false;
//			pvertices[i].bto_be_extended = false;
//			gpremoved_vertices[nrm_end++] = i;//record in remove_array
//		}
//	}
//
//	//num_of_freq_cands will record how many vertices still remain.
//	//num_of_cands = num_of_vertices at the beginning;
//	//Update only when half of the candidate need to be removed. recurivly update
//	while(num_of_freq_cands<num_of_cands/2)
//	{
//		num_of_cands = num_of_freq_cands;
//		for(i=0;i<num_of_cands;i++)
//			pvertices[gpremain_vertices[i]].ncand_deg = 0;
//		for(i=0;i<num_of_cands;i++)
//		{
//			//set the ncand_deg for candidate vertex
//			nvertex_no = gpremain_vertices[i];
//			for(j=1;j<=mppadj_lists[nvertex_no][0];j++)
//			{
//				norder = mppadj_lists[nvertex_no][j];
//				if(pvertices[norder].bis_cand)//some neighbor may not be a candidate
//					pvertices[norder].ncand_deg++;
//			}
//		}
//		num_of_freq_cands = 0;
//		nrm_start = 0;
//		nrm_end = 0;
//		//degree was updated in above code. need to remove vertex based on degree recursively here.
//		for(i=0;i<num_of_cands;i++)
//		{
//			norder = gpremain_vertices[i];
//			if(pvertices[norder].ncand_deg>=gnmin_deg) // degree updated: degree >= r * (min_size - 1)
//				gpremain_vertices[num_of_freq_cands++] = norder;
//			else
//			{
//				pvertices[norder].bis_cand = false;
//				pvertices[norder].bto_be_extended = false;
//				gpremoved_vertices[nrm_end++] = norder;
//			}
//		}
//	}
//	num_of_cands = num_of_freq_cands;
//
//	//gpremoved_vertices record the last time removed vertices. Need to update this v's nbs's degree.
//	while(nrm_end>nrm_start)
//	{
//		nvertex_no = gpremoved_vertices[nrm_start];
//		for(i=1;i<=mppadj_lists[nvertex_no][0];i++)
//		{
//			//recursively prune cand when update degree, just like kcore
//			norder = mppadj_lists[nvertex_no][i];
//			if(pvertices[norder].bis_cand)
//			{
//				pvertices[norder].ncand_deg--;
//				if(pvertices[norder].ncand_deg<gnmin_deg) // degree updated: degree < r * min_size
//				{
//					pvertices[norder].bis_cand = false;
//					pvertices[norder].bto_be_extended = false;
//					num_of_cands--;
//					gpremoved_vertices[nrm_end++] = norder;
//				}
//			}
//		}
//		nrm_start++;
//	}
//
//	//=== first round cover prunning ===
//
//	nmax_deg = 0;
//	nmaxdeg_vertex = 0;
//
//	for(i=0;i<num_of_vertices;i++)
//	{
//		if(pvertices[i].bis_cand)
//		{
//			if(nmax_deg<pvertices[i].ncand_deg)
//			{
//				nmax_deg = pvertices[i].ncand_deg;
//				nmaxdeg_vertex = i;
//			}
//		}
//	}
//
//	for(i=1;i<=mppadj_lists[nmaxdeg_vertex][0];i++)
//		pvertices[mppadj_lists[nmaxdeg_vertex][i]].bto_be_extended = false;
//
//	if(nmaxdeg_vertex!=0) //!!! swap max-degree vertex (ncand_deg) with the first vertex ------> try out to see if this makes a big difference in performance?
//	{
//		onevertex = pvertices[nmaxdeg_vertex];
//		pvertices[nmaxdeg_vertex] = pvertices[0];
//		pvertices[0] = onevertex;
//	}
//
//	qsort(&pvertices[1], num_of_vertices-1, sizeof(VERTEX), comp_vertex_freq); // sort by pvertices[1, ...] by (nclique_deg, ncand_deg) ------> try out to see if this makes a big difference in performance?
//
//	VerifyVertices(pvertices, 0, num_of_cands, 0, false);
//
//	if(num_of_cands>=gnmin_size)
//	{
//		//setup COND_GRAPH
//		mnmax_cond_graphs = MAX_COND_GRAPH;
//		mpcond_graphs = new COND_GRAPH[mnmax_cond_graphs];
//		mnum_of_cond_graphs = 0;
//		gocondgraph_buf.phead = new INT_PAGE;
//		gocondgraph_buf.phead->pnext = NULL;
//		gocondgraph_buf.pcur_page = gocondgraph_buf.phead;
//		gocondgraph_buf.ncur_pos = 0;
//		gocondgraph_buf.ntotal_pages = 1;
//		//todo gpcondgraph_listpt_buf = new int*[mnum_of_vertices*3*MAX_COND_GRAPH]
//		gpcondgraph_listpt_buf = new int*[mnum_of_vertices*2*MAX_COND_GRAPH];
//		gpcounters = new int[num_of_cands];
//
//		Expand(pvertices, 0, num_of_cands, 0);
//
//		delete []gpcounters;
//		DelCGIntBuf();
//		delete []gpcondgraph_listpt_buf;
//		delete []mpcond_graphs;
//		if(mnum_of_cond_graphs!=0)
//			printf("Error: the number of conditiaonal database should be 0\n");
//	}
//
//	delete []pvertices;
//	delete []gpremoved_vertices;
//	delete []gpremain_vertices;
//	delete []gpvertex_order_map;
//	delete []gptemp_array;
//	delete []gpmin_degs;
//	DestroyGraph();
////	fclose(gfpout);
//
//	ftime(&end);
//
//	return num_of_vertices;
//}





int Graph::Expand(VERTEX *pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, FILE *gfpout)
{
	VERTEX *pnew_vertices, *pnew_cands, *pclique;
	int num_of_vertices, num_of_new_cands, i, j, num_of_new_tail_vertices, nmin_deg;
	bool bis_subsumed, blook_succeed, bgen_new_lvl2nbs;
	int nisvalid, nremoved_vertices;
	int nsuperclique_size, nmax_clique_size, nnew_clique_size;
	struct timeb cur_time;
	double drun_time;
	CLQ_STAT one_clq_stat;

	num_of_vertices = nclique_size+num_of_cands+num_of_tail_vertices;
	pnew_vertices = new VERTEX[num_of_vertices];
	pclique = new VERTEX[nclique_size+1];
	nmax_clique_size = 0;
//???all bis_cand == false and bto_be_extended == false at the end???
	for(i=nclique_size;i<nclique_size+num_of_cands && pvertices[i].bis_cand && pvertices[i].bto_be_extended;i++) // not iterating covered vertices (segment 3)
	{
		if(nclique_size==0 && i%500==0)
		{
			ftime(&cur_time);
			drun_time = cur_time.time-gtime_start.time+(double)(cur_time.millitm-gtime_start.millitm)/1000;
//			printf("%d %d %d %d %d %d %.3f %d\n", i, num_of_cands, gntotal_cliques, gnmax_clique_size, gntotal_calls, gnmax_depth, drun_time, gnlookahead_suceeds);
		}

		VerifyVertices(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, false);
		if(i>nclique_size)
			nisvalid = RemoveCandVertex(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i);
		else
			nisvalid = 1;
		if(nisvalid==-1)
			break;
		else if(nisvalid==1 && nclique_size+1<gnmax_size)
		{
			if(i<nclique_size+num_of_cands-1)
			{
				//copy all valid v in pvertices to pnew_vertices here
				blook_succeed = Lookahead(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i, pnew_vertices, gfpout);
				if(blook_succeed)
				{
					if(nmax_clique_size<nclique_size+(nclique_size+num_of_cands-i))
						nmax_clique_size = nclique_size+(nclique_size+num_of_cands-i);
					break;
				}
			}

			if(gdmin_deg_ratio==1)
				num_of_new_cands = AddOneVertex(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i, true, pnew_vertices, num_of_new_tail_vertices, &one_clq_stat);
			else
				num_of_new_cands = AddOneVertex(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i, false, pnew_vertices, num_of_new_tail_vertices, &one_clq_stat);
			nnew_clique_size = nclique_size+1;
			if(num_of_new_cands>0)
				CrtcVtxPrune(pnew_vertices, nnew_clique_size, num_of_new_cands, num_of_new_tail_vertices, pclique, &one_clq_stat);

			bis_subsumed = false;
			pnew_cands = &pnew_vertices[nnew_clique_size];
			if(num_of_new_cands>0)
			{
				bgen_new_lvl2nbs = GenCondGraph(pnew_vertices, nnew_clique_size, num_of_new_cands, num_of_new_tail_vertices);
				nremoved_vertices = ReduceCands(pnew_vertices, nnew_clique_size, num_of_new_cands+num_of_new_tail_vertices, bis_subsumed);

				if(num_of_new_cands-nremoved_vertices>0) // there is still some candidate left for expansion
					nsuperclique_size = Expand(pnew_vertices, nnew_clique_size, num_of_new_cands, num_of_new_tail_vertices, gfpout);
				else
					nsuperclique_size = 0;
				if(bgen_new_lvl2nbs)
					DelCondGraph();
			}
			else
				nsuperclique_size = 0;

			if(nsuperclique_size==0 && !bis_subsumed)
			{
				if(nnew_clique_size>=gnmin_size)
				{
					nmin_deg = GetMinDeg(nnew_clique_size);
					for(j=0;j<nnew_clique_size;j++)
					{
						if(pnew_vertices[j].nclique_deg<nmin_deg)
							break;
					}
					if(j>=nnew_clique_size)
					{
						if(gdmin_deg_ratio<1)
							num_of_new_tail_vertices = 0;
						else if(num_of_new_tail_vertices==0)
							num_of_new_tail_vertices = GenTailVertices(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i, pnew_vertices, nnew_clique_size);
						OutputOneClique(pnew_vertices, nnew_clique_size, num_of_new_tail_vertices, gfpout);
						if(nmax_clique_size<nnew_clique_size)
							nmax_clique_size = nnew_clique_size;
					}
					else if(nnew_clique_size>nclique_size+1 && nclique_size+1>=gnmin_size)
					{
						nmin_deg = GetMinDeg(nclique_size+1);
						for(j=0;j<=nclique_size;j++)
						{
							if(pclique[j].nclique_deg<nmin_deg)
								break;
						}
						if(j>nclique_size)
						{
							memcpy(pnew_vertices, pclique, sizeof(VERTEX)*(nclique_size+1));
							num_of_new_tail_vertices = 0;
							OutputOneClique(pnew_vertices, nclique_size+1, num_of_new_tail_vertices, gfpout);
							if(nmax_clique_size<nclique_size+1)
								nmax_clique_size = nclique_size+1;
						}
					}
				}
			}
			else if(nsuperclique_size>0)
			{
				if(nmax_clique_size<nsuperclique_size)
					nmax_clique_size = nsuperclique_size;
			}
			else if(nmax_clique_size<nnew_clique_size)
				nmax_clique_size = nnew_clique_size;
		}
	}
	delete []pnew_vertices;
	delete []pclique;

//	if(num_of_cands>=10 && nmax_clique_size==0 && nisvalid==1)
//		printf("stop\n");

	return nmax_clique_size;
}


int Graph::ExpandOnce(VERTEX *pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, FILE *gfpout)
{
	VERTEX *pnew_vertices, *pnew_cands, *pclique;
	int num_of_vertices, num_of_new_cands, i, j, num_of_new_tail_vertices, nmin_deg;
	bool bis_subsumed, blook_succeed, bgen_new_lvl2nbs;
	int nisvalid, nremoved_vertices;
	int nsuperclique_size, nmax_clique_size, nnew_clique_size;
	struct timeb cur_time;
	double drun_time;
	CLQ_STAT one_clq_stat;

	num_of_vertices = nclique_size+num_of_cands+num_of_tail_vertices;
	pnew_vertices = new VERTEX[num_of_vertices];
	pclique = new VERTEX[nclique_size+1];
	nmax_clique_size = 0;

	for(i=nclique_size;i<nclique_size+1 && pvertices[i].bis_cand && pvertices[i].bto_be_extended;i++) // not iterating covered vertices (segment 3)
	{
		if(nclique_size==0 && i%500==0)
		{
			ftime(&cur_time);
			drun_time = cur_time.time-gtime_start.time+(double)(cur_time.millitm-gtime_start.millitm)/1000;
//			printf("%d %d %d %d %d %d %.3f %d\n", i, num_of_cands, gntotal_cliques, gnmax_clique_size, gntotal_calls, gnmax_depth, drun_time, gnlookahead_suceeds);
		}

		VerifyVertices(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, false);
		if(i>nclique_size)
			nisvalid = RemoveCandVertex(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i);
		else
			nisvalid = 1;
		if(nisvalid==-1)
			break;
		else if(nisvalid==1 && nclique_size+1<gnmax_size)
		{
			if(i<nclique_size+num_of_cands-1)
			{
				blook_succeed = Lookahead(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i, pnew_vertices, gfpout);
				if(blook_succeed)
				{
					if(nmax_clique_size<nclique_size+(nclique_size+num_of_cands-i))
						nmax_clique_size = nclique_size+(nclique_size+num_of_cands-i);
					break;
				}
			}

			if(gdmin_deg_ratio==1)
				num_of_new_cands = AddOneVertex(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i, true, pnew_vertices, num_of_new_tail_vertices, &one_clq_stat);
			else
				num_of_new_cands = AddOneVertex(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i, false, pnew_vertices, num_of_new_tail_vertices, &one_clq_stat);
			nnew_clique_size = nclique_size+1;
			if(num_of_new_cands>0)
				CrtcVtxPrune(pnew_vertices, nnew_clique_size, num_of_new_cands, num_of_new_tail_vertices, pclique, &one_clq_stat);

			bis_subsumed = false;
			pnew_cands = &pnew_vertices[nnew_clique_size];
			if(num_of_new_cands>0)
			{
				bgen_new_lvl2nbs = GenCondGraph(pnew_vertices, nnew_clique_size, num_of_new_cands, num_of_new_tail_vertices);
				nremoved_vertices = ReduceCands(pnew_vertices, nnew_clique_size, num_of_new_cands+num_of_new_tail_vertices, bis_subsumed);

				if(num_of_new_cands-nremoved_vertices>0) // there is still some candidate left for expansion
					nsuperclique_size = Expand(pnew_vertices, nnew_clique_size, num_of_new_cands, num_of_new_tail_vertices, gfpout);
				else
					nsuperclique_size = 0;
				if(bgen_new_lvl2nbs)
					DelCondGraph();
			}
			else
				nsuperclique_size = 0;

			if(nsuperclique_size==0 && !bis_subsumed)
			{
				if(nnew_clique_size>=gnmin_size)
				{
					nmin_deg = GetMinDeg(nnew_clique_size);
					for(j=0;j<nnew_clique_size;j++)
					{
						if(pnew_vertices[j].nclique_deg<nmin_deg)
							break;
					}
					if(j>=nnew_clique_size)
					{
						if(gdmin_deg_ratio<1)
							num_of_new_tail_vertices = 0;
						else if(num_of_new_tail_vertices==0)
							num_of_new_tail_vertices = GenTailVertices(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, i, pnew_vertices, nnew_clique_size);
						OutputOneClique(pnew_vertices, nnew_clique_size, num_of_new_tail_vertices, gfpout);
						if(nmax_clique_size<nnew_clique_size)
							nmax_clique_size = nnew_clique_size;
					}
					else if(nnew_clique_size>nclique_size+1 && nclique_size+1>=gnmin_size)
					{
						nmin_deg = GetMinDeg(nclique_size+1);
						for(j=0;j<=nclique_size;j++)
						{
							if(pclique[j].nclique_deg<nmin_deg)
								break;
						}
						if(j>nclique_size)
						{
							memcpy(pnew_vertices, pclique, sizeof(VERTEX)*(nclique_size+1));
							num_of_new_tail_vertices = 0;
							OutputOneClique(pnew_vertices, nclique_size+1, num_of_new_tail_vertices, gfpout);
							if(nmax_clique_size<nclique_size+1)
								nmax_clique_size = nclique_size+1;
						}
					}
				}
			}
			else if(nsuperclique_size>0)
			{
				if(nmax_clique_size<nsuperclique_size)
					nmax_clique_size = nsuperclique_size;
			}
			else if(nmax_clique_size<nnew_clique_size)
				nmax_clique_size = nnew_clique_size;
		}
	}
	delete []pnew_vertices;
	delete []pclique;

//	if(num_of_cands>=10 && nmax_clique_size==0 && nisvalid==1)
//		printf("stop\n");

	return nmax_clique_size;
}
//remove the vertex that has just be processed from candidate list
int Graph::RemoveCandVertex(VERTEX *pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, int ncur_pos)
{
	int i, nvertex_no, norder, nisvalid, nmin_deg, num_of_vertices;
	int *plvl2_nbs, *padj_list;

	nmin_deg = GetMinDeg(nclique_size+1);

	num_of_vertices = nclique_size+num_of_cands+num_of_tail_vertices;

	for(i=0;i<num_of_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i; // update map[vertex_no] = position in pvertices[.]

	nisvalid = 1;
	pvertices[ncur_pos-1].bis_cand = false;
	nvertex_no = pvertices[ncur_pos-1].nvertex_no;
	if(mnum_of_cond_graphs==0)
		padj_list = mppadj_lists[nvertex_no];
	else
		padj_list = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists[nvertex_no];

	if(padj_list!=NULL)
	{
		for(i=1;i<=padj_list[0];i++)
		{
			norder = gpvertex_order_map[padj_list[i]];
			if(norder>=0)
			{
				pvertices[norder].ncand_deg--;
				if(pvertices[norder].ncand_deg+pvertices[norder].nclique_deg<nmin_deg)
				{
					if(norder<nclique_size)
					{
						nisvalid = -1;
						break;
					}
					else if(norder==ncur_pos)
						nisvalid = 0;
				}
			}
		}
	}

	if(nisvalid>=0 && mblvl2_flag)
	{
		if(mnum_of_cond_graphs==0)
			plvl2_nbs = mpplvl2_nbs[nvertex_no];
		else
			plvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs[nvertex_no];

		if(plvl2_nbs!=NULL)
		{
			for(i=1;i<=plvl2_nbs[0];i++)
			{
				norder = gpvertex_order_map[plvl2_nbs[i]];
				if(norder>i && norder<nclique_size+num_of_cands)
					pvertices[norder].nlvl2_nbs--;
			}
		}
	}

	for(i=0;i<num_of_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1;

	return nisvalid;
}

bool Graph::Lookahead(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, int ncur_pos, VERTEX *pnew_vertices, FILE *gfpout)
{
	int i, nmin_deg, num_of_freq_tails, nsize;
	VERTEX *ptail_vertices;

	//return false;

	nmin_deg = GetMinDeg(nclique_size+(nclique_size+num_of_cands-ncur_pos));
	for(i=0;i<nclique_size;i++)
	{
		if(pvertices[i].ncand_deg+pvertices[i].nclique_deg<nmin_deg)
			break;
	}
	if(i<nclique_size)
		return false;
	for(i=ncur_pos;i<nclique_size+num_of_cands;i++)
	{
		if(pvertices[i].nclique_deg+pvertices[i].ncand_deg<nmin_deg)
			break;
		if(mblvl2_flag && pvertices[i].nlvl2_nbs<nclique_size+num_of_cands-ncur_pos-1)
			break;
	}
	if(i<nclique_size+num_of_cands)
		return false;

	for(i=0;i<nclique_size;i++)
	{
		pnew_vertices[i].nvertex_no = pvertices[i].nvertex_no;
		pnew_vertices[i].nclique_deg = pvertices[i].nclique_deg+pvertices[i].ncand_deg;
		pnew_vertices[i].ncand_deg = 0;
	}
	nsize = nclique_size;
	for(i=ncur_pos;i<nclique_size+num_of_cands;i++)
	{
		pnew_vertices[nsize].nvertex_no = pvertices[i].nvertex_no;
		pnew_vertices[nsize].nclique_deg = pvertices[i].nclique_deg+pvertices[i].ncand_deg;
		pnew_vertices[nsize].ncand_deg = 0;
		nsize++;
	}

	num_of_freq_tails = 0;
	if(gdmin_deg_ratio==1)
	{
		ptail_vertices = &pnew_vertices[nsize];
		for(i=nclique_size+num_of_cands;i<nclique_size+num_of_cands+num_of_tail_vertices;i++)
		{
			if(pvertices[i].ncand_deg+pvertices[i].nclique_deg>=nmin_deg)
			{
				ptail_vertices[num_of_freq_tails].nvertex_no = pvertices[i].nvertex_no;
				ptail_vertices[num_of_freq_tails].nclique_deg = pvertices[i].nclique_deg+pvertices[i].ncand_deg;
				ptail_vertices[num_of_freq_tails].ncand_deg = 0;
				num_of_freq_tails++;
			}
		}
		for(i=nclique_size;i<ncur_pos;i++)
		{
			if(pvertices[i].ncand_deg+pvertices[i].nclique_deg>=nmin_deg)
			{
				ptail_vertices[num_of_freq_tails].nvertex_no = pvertices[i].nvertex_no;
				ptail_vertices[num_of_freq_tails].nclique_deg = pvertices[i].nclique_deg+pvertices[i].ncand_deg;
				ptail_vertices[num_of_freq_tails].ncand_deg = 0;
				num_of_freq_tails++;
			}
		}
	}

	OutputOneClique(pnew_vertices, nsize, num_of_freq_tails, gfpout);

	return true;
}

void Graph::CalcLUBound(VERTEX *pvertices, int nclique_size, int *pcand_clqdegs, int num_of_valid_cands, CLQ_STAT *pclq_stat)
{
	int i, nmin_clq_clqdeg, nminclqdeg_candeg, nmin_clq_totaldeg, nclq_clqdeg_sum, ncand_clqdeg_sum, ntightened_max_cands;

	nmin_clq_clqdeg = pvertices[0].nclique_deg;
	nminclqdeg_candeg = pvertices[0].ncand_deg;
	nclq_clqdeg_sum = pvertices[0].nclique_deg;
	nmin_clq_totaldeg = pvertices[0].nclique_deg+pvertices[0].ncand_deg;
	for(i=1;i<nclique_size;i++)
	{
		nclq_clqdeg_sum += pvertices[i].nclique_deg;
		if(nmin_clq_clqdeg>pvertices[i].nclique_deg)
		{
			nmin_clq_clqdeg = pvertices[i].nclique_deg;
			nminclqdeg_candeg = pvertices[i].ncand_deg;
		}
		else if(nmin_clq_clqdeg==pvertices[i].nclique_deg)
		{
			if(nminclqdeg_candeg>pvertices[i].ncand_deg)
				nminclqdeg_candeg = pvertices[i].ncand_deg;
		}
		if(nmin_clq_totaldeg>pvertices[i].nclique_deg+pvertices[i].ncand_deg)
			nmin_clq_totaldeg = pvertices[i].nclique_deg+pvertices[i].ncand_deg;
	}
	pclq_stat->nmin_ext_deg = GetMinDeg(nclique_size+1);
	if(nmin_clq_clqdeg<gpmin_degs[nclique_size+1])
	{
		//initialize lower bound
		pclq_stat->nmin_cands = GetMinDeg(nclique_size+1)-nmin_clq_clqdeg;
		while(pclq_stat->nmin_cands<=nminclqdeg_candeg && nmin_clq_clqdeg+pclq_stat->nmin_cands<gpmin_degs[nclique_size+pclq_stat->nmin_cands])
			pclq_stat->nmin_cands++;
		if(nmin_clq_clqdeg+pclq_stat->nmin_cands<gpmin_degs[nclique_size+pclq_stat->nmin_cands])//todo
			pclq_stat->nmin_cands = num_of_valid_cands+1;

		//initialize upper bound
		pclq_stat->nmax_cands = (int)(nmin_clq_totaldeg/gdmin_deg_ratio)+1-nclique_size;
		if(pclq_stat->nmax_cands>num_of_valid_cands)
			pclq_stat->nmax_cands = num_of_valid_cands;

		//tighten lower bound and upper bound based on the clique degree of candidates
		if(pclq_stat->nmin_cands<pclq_stat->nmax_cands)
		{
			//tighten lower bound
			ncand_clqdeg_sum = 0;
			for(i=0;i<pclq_stat->nmin_cands;i++)
				ncand_clqdeg_sum += pcand_clqdegs[i];
			while(i<pclq_stat->nmax_cands && nclq_clqdeg_sum+ncand_clqdeg_sum<nclique_size*gpmin_degs[nclique_size+i])
			{
				ncand_clqdeg_sum += pcand_clqdegs[i];
				i++;
			}
			if(nclq_clqdeg_sum+ncand_clqdeg_sum<nclique_size*gpmin_degs[nclique_size+i])
				pclq_stat->nmin_cands = pclq_stat->nmax_cands+1;
			else //tighten upper bound
			{
				pclq_stat->nmin_cands = i;
				ntightened_max_cands = i;
				while(i<pclq_stat->nmax_cands)
				{
					ncand_clqdeg_sum += pcand_clqdegs[i];
					i++;
					if(nclq_clqdeg_sum+ncand_clqdeg_sum>=nclique_size*gpmin_degs[nclique_size+i])
						ntightened_max_cands = i;
				}
				if(pclq_stat->nmax_cands>ntightened_max_cands)
					pclq_stat->nmax_cands = ntightened_max_cands;

				if(pclq_stat->nmin_cands>1)
					pclq_stat->nmin_ext_deg = GetMinDeg(nclique_size+pclq_stat->nmin_cands);
			}
		}
	}
	else
	{
		pclq_stat->nmin_ext_deg = GetMinDeg(nclique_size+1);
		pclq_stat->nmax_cands = num_of_valid_cands;
		if(nclique_size+1<gnmin_size)
			pclq_stat->nmin_cands = gnmin_size-nclique_size;
		else
			pclq_stat->nmin_cands = 1;
	}

	//disable lower bound pruning
	//pclq_stat->nmin_cands = 1;
	//pclq_stat->nmin_ext_deg = GetMinDeg(nclique_size+1);

	//disable upper bound pruning
	//pclq_stat->nmax_cands = num_of_valid_cands;

	if(nclique_size+pclq_stat->nmax_cands<gnmin_size)
		pclq_stat->nmax_cands = 0;

	if(pclq_stat->nmax_cands>0 && pclq_stat->nmax_cands>=pclq_stat->nmin_cands)
	{
		for(i=0;i<nclique_size;i++)
		{
			if(pvertices[i].nclique_deg+pvertices[i].ncand_deg<pclq_stat->nmin_ext_deg ||
				pvertices[i].nclique_deg+pvertices[i].ncand_deg<GetMinDeg(nclique_size+pvertices[i].ncand_deg) ||
				pvertices[i].ncand_deg==0 && pvertices[i].nclique_deg<GetMinDeg(nclique_size+1) ||
				pvertices[i].nclique_deg+pclq_stat->nmax_cands<gpmin_degs[nclique_size+pclq_stat->nmax_cands] ||
				pvertices[i].nclique_deg+pvertices[i].ncand_deg<GetMinDeg(nclique_size+pclq_stat->nmin_cands))
			{
				pclq_stat->nmax_cands = 0;
				break;
			}
		}
	}
}

int Graph::AddOneVertex(VERTEX *pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, int ncur_pos, bool bhas_tail, VERTEX *pnew_vertices, int &num_of_new_tails, CLQ_STAT *pclq_stat)
{
	VERTEX *pnew_cands;
	int i, j, ntotal_vertices, nvertex_no, norder;
	int ntotal_new_exts, num_of_new_cands, num_of_valid_cands, num_of_rmved_cands, num_of_valid_exts;
	int nmin_ext_deg;
	int **pplvl2_nbs, **ppadj_lists, nvertexset_size, *pcand_clqdegs;

	VerifyVertices(pvertices, nclique_size, num_of_cands, num_of_tail_vertices, false);

	ntotal_vertices = nclique_size+num_of_cands+num_of_tail_vertices;
	nmin_ext_deg = GetMinDeg(nclique_size+2);

	//========================================================
	//copy pvertices to pnew_vertices
	if(nclique_size>0)
		memcpy(pnew_vertices, pvertices, sizeof(VERTEX)*nclique_size);
	nvertex_no = pvertices[ncur_pos].nvertex_no;
	pnew_vertices[nclique_size].nvertex_no = nvertex_no;
	pnew_vertices[nclique_size].ncand_deg = pvertices[ncur_pos].ncand_deg;
	pnew_vertices[nclique_size].nclique_deg = pvertices[ncur_pos].nclique_deg;
	pnew_vertices[nclique_size].bis_cand = false;
	pnew_vertices[nclique_size].bto_be_extended = false;

	pnew_cands = &(pnew_vertices[nclique_size+1]);
	num_of_new_cands = nclique_size+num_of_cands-ncur_pos-1;
	if(num_of_new_cands>0)
		memcpy(pnew_cands, &pvertices[ncur_pos+1], sizeof(VERTEX)*num_of_new_cands);
	ntotal_new_exts = num_of_new_cands;
	if(bhas_tail)
	{
		for(i=nclique_size+num_of_cands;i<ntotal_vertices;i++)
		{
			if(pvertices[i].ncand_deg+pvertices[i].nclique_deg>=nmin_ext_deg)
				pnew_cands[ntotal_new_exts++] = pvertices[i];
		}
		for(i=nclique_size;i<ncur_pos;i++)
		{
			if(pvertices[i].ncand_deg+pvertices[i].nclique_deg>=nmin_ext_deg)
				pnew_cands[ntotal_new_exts++] = pvertices[i];
		}
	}
	//========================================================


	for(i=0;i<nclique_size+1+ntotal_new_exts;i++)
		gpvertex_order_map[pnew_vertices[i].nvertex_no] = i;

	if(mnum_of_cond_graphs==0)
		ppadj_lists = mppadj_lists;
	else
		ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;


	//-----------------------------------------------------------
	//update clique-degree and candidate-degree of the vertices
	if(ppadj_lists[nvertex_no]!=NULL)
	{
		for(i=1;i<=ppadj_lists[nvertex_no][0];i++)
		{
			norder = gpvertex_order_map[ppadj_lists[nvertex_no][i]];
			if(norder>=0)
			{
				pnew_vertices[norder].nclique_deg++;
				pnew_vertices[norder].ncand_deg--;
			}
		}
	}
	//-----------------------------------------------------------

	pcand_clqdegs = gptemp_array;

	//========================================================
	//removing vertices not a level-1 or level-2 neighbor of vertex_no
	for(i=0;i<ntotal_new_exts;i++)
		pnew_cands[i].bto_be_extended = false;
	num_of_valid_cands = 0;
	nvertexset_size = nclique_size+(nclique_size+num_of_cands-ncur_pos);
	if(gdmin_deg_ratio>(double)(nvertexset_size-2)/(nvertexset_size-1))
	{
		if(ppadj_lists[nvertex_no]!=NULL)
		{
			for(i=1;i<=ppadj_lists[nvertex_no][0];i++)
			{
				norder = gpvertex_order_map[ppadj_lists[nvertex_no][i]]-nclique_size-1;
				if(norder>=0)
				{
					pnew_cands[norder].bto_be_extended = true;
					if(norder<num_of_new_cands && pnew_cands[norder].ncand_deg+pnew_cands[norder].nclique_deg>=nmin_ext_deg)
						pcand_clqdegs[num_of_valid_cands++] = pnew_cands[norder].nclique_deg;
				}
			}
		}
	}
	else
	{
		if(mnum_of_cond_graphs==0)
			pplvl2_nbs = mpplvl2_nbs;
		else
			pplvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs;
		if(pplvl2_nbs[nvertex_no]!=NULL)
		{
			for(i=1;i<=pplvl2_nbs[nvertex_no][0];i++)
			{
				norder = gpvertex_order_map[pplvl2_nbs[nvertex_no][i]]-nclique_size-1;
				if(norder>=0)
				{
					pnew_cands[norder].bto_be_extended = true;
					if(norder<num_of_new_cands && pnew_cands[norder].ncand_deg+pnew_cands[norder].nclique_deg>=nmin_ext_deg)
						pcand_clqdegs[num_of_valid_cands++] = pnew_cands[norder].nclique_deg;
				}
			}
		}
	}
	//========================================================

	qsort(pcand_clqdegs, num_of_valid_cands, sizeof(int), comp_int_des);
	CalcLUBound(pnew_vertices, nclique_size+1, pcand_clqdegs, num_of_valid_cands, pclq_stat);

	//----------------------------------------------------------------------------------------
	if(pclq_stat->nmin_cands<=pclq_stat->nmax_cands && pclq_stat->nmax_cands>0)
	{
		num_of_valid_cands = 0;
		num_of_rmved_cands = 0;
		for(i=0;i<num_of_new_cands;i++)
		{
			if(pnew_cands[i].bto_be_extended && IsValidCand(&pnew_cands[i], nclique_size+1, pclq_stat))
				gpremain_vertices[num_of_valid_cands++] = i;
			else
				gpremoved_vertices[num_of_rmved_cands++] = i;
		}

		while(num_of_valid_cands>0 && num_of_rmved_cands>0)
		{
			if(num_of_valid_cands<num_of_rmved_cands)
			{
				for(i=0;i<nclique_size+1+ntotal_new_exts;i++)
					pnew_vertices[i].ncand_deg = 0;
				for(i=0;i<num_of_valid_cands;i++)
				{
					nvertex_no = pnew_cands[gpremain_vertices[i]].nvertex_no;
					if(ppadj_lists[nvertex_no]!=NULL)
					{
						for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
						{
							norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
							if(norder>=0)
								pnew_vertices[norder].ncand_deg++;
						}
					}
				}
			}
			else
			{
				for(i=0;i<num_of_rmved_cands;i++)
				{
					nvertex_no = pnew_cands[gpremoved_vertices[i]].nvertex_no;
					if(ppadj_lists[nvertex_no]!=NULL)
					{
						for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
						{
							norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
							if(norder>=0)
								pnew_vertices[norder].ncand_deg--;
						}
					}
				}
			}

			num_of_valid_exts = 0;
			for(i=0;i<num_of_valid_cands;i++)
			{
				if(IsValidCand(&pnew_cands[gpremain_vertices[i]], nclique_size+1, pclq_stat))
					pcand_clqdegs[num_of_valid_exts++] = pnew_cands[gpremain_vertices[i]].nclique_deg;
			}
			qsort(pcand_clqdegs, num_of_valid_exts, sizeof(int), comp_int_des);
			CalcLUBound(pnew_vertices, nclique_size+1, pcand_clqdegs, num_of_valid_exts, pclq_stat);
			if(pclq_stat->nmax_cands==0 || pclq_stat->nmax_cands<pclq_stat->nmin_cands)
				break;

			num_of_valid_exts = 0;
			num_of_rmved_cands = 0;
			for(i=0;i<num_of_valid_cands;i++)
			{
				if(IsValidCand(&pnew_cands[gpremain_vertices[i]], nclique_size+1, pclq_stat))
					gpremain_vertices[num_of_valid_exts++] = gpremain_vertices[i];
				else
					gpremoved_vertices[num_of_rmved_cands++] = gpremain_vertices[i];
			}
			num_of_valid_cands = num_of_valid_exts;
		}
	}
	//--------------------------------------------------------------------

	for(i=0;i<nclique_size+1+ntotal_new_exts;i++)
		gpvertex_order_map[pnew_vertices[i].nvertex_no] = -1;

	VerifyVertices(pnew_vertices, nclique_size+1, num_of_new_cands, ntotal_new_exts-num_of_new_cands, true);

	if(pclq_stat->nmax_cands==0 || pclq_stat->nmax_cands<pclq_stat->nmin_cands || num_of_valid_cands<pclq_stat->nmin_cands)
	{
		num_of_new_tails = 0;
		return 0;
	}

	for(i=0;i<num_of_valid_cands;i++)
		pnew_cands[i] = pnew_cands[gpremain_vertices[i]];

	num_of_new_tails = 0;
	if(bhas_tail)
	{
		for(i=num_of_new_cands;i<ntotal_new_exts;i++)
		{
			if(pnew_cands[i].bto_be_extended)
			{
				pnew_cands[num_of_valid_cands+num_of_new_tails] = pnew_cands[i];
				num_of_new_tails++;
			}
		}
	}

	return num_of_valid_cands;
}

void Graph::CrtcVtxPrune(VERTEX *pvertices, int &nclique_size, int &num_of_cands, int &num_of_tail_vertices, VERTEX *pclique, CLQ_STAT *pclq_stat)
{
	VERTEX onevertex, *pcands;
	int i, j, num_of_vertices, num_of_necvtx, num_of_new_tails, num_of_valid_exts;
	int nvertex_no, norder, **ppadj_lists, **pplvl2_nbs, ncand_deg, *pcand_clqdegs;
	int num_of_valid_cands, num_of_rmved_cands;

	//return;

	if(!mblvl2_flag)
		return;

	num_of_vertices = nclique_size+num_of_cands+num_of_tail_vertices;
	for(i=0;i<num_of_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i;

	if(mnum_of_cond_graphs==0)
	{
		ppadj_lists = mppadj_lists;
		pplvl2_nbs = mpplvl2_nbs;
	}
	else
	{
		ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;
		pplvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs;
	}

	num_of_necvtx = 0;
	vector<int> gptemp_vec;
	for(i=0;i<nclique_size;i++)
	{
		if(pvertices[i].nclique_deg+pvertices[i].ncand_deg==gpmin_degs[nclique_size+pclq_stat->nmin_cands] &&
			pvertices[i].ncand_deg>0)
		{
			nvertex_no = pvertices[i].nvertex_no;
			ncand_deg = 0;
			if(ppadj_lists[nvertex_no]!=NULL)
			{
				for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
				{
					norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
					if(norder>=nclique_size && norder<nclique_size+num_of_cands)
					{
						gptemp_vec.push_back(norder);
//						gptemp_array[num_of_necvtx++] = norder;
						ncand_deg++;
					}
				}
			}
			if(ncand_deg!=pvertices[i].ncand_deg)
				printf("Error: inconsistent candidate degree 1\n");
		}
	}
	num_of_necvtx = gptemp_vec.size();
	for(i=0;i<num_of_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1;
	if(num_of_necvtx==0)
		return;

	memcpy(pclique, pvertices, sizeof(VERTEX)*nclique_size);

	qsort(&gptemp_vec[0], num_of_necvtx, sizeof(int), comp_int);
//	qsort(gptemp_array, num_of_necvtx, sizeof(int), comp_int);
	j =1;
//	for(i=1;i<num_of_necvtx;i++)
//	{
//		if(gptemp_array[i]!=gptemp_array[i-1])
//			gptemp_array[j++] = gptemp_array[i];
//	}
//	num_of_necvtx = j;
//	for(i=0;i<num_of_necvtx;i++)
//	{
//		if(gptemp_array[i]!=nclique_size+i)
//		{
//			onevertex = pvertices[nclique_size+i];
//			pvertices[nclique_size+i] = pvertices[gptemp_array[i]];
//			pvertices[gptemp_array[i]] = onevertex;
//		}
//		pvertices[nclique_size+i].bis_cand = false;
//	}


	for(i=1;i<num_of_necvtx;i++)
	{
		if(gptemp_vec[i]!=gptemp_vec[i-1])
			gptemp_vec[j++] = gptemp_vec[i];
	}
	num_of_necvtx = j;
	for(i=0;i<num_of_necvtx;i++)
	{
		if(gptemp_vec[i]!=nclique_size+i)
		{
			onevertex = pvertices[nclique_size+i];
			pvertices[nclique_size+i] = pvertices[gptemp_vec[i]];
			pvertices[gptemp_vec[i]] = onevertex;
		}
		pvertices[nclique_size+i].bis_cand = false;
	}

	for(i=0;i<num_of_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i;

	memset(gpcounters, 0, sizeof(int)*num_of_vertices);
	for(i=0;i<num_of_necvtx;i++)
	{
		nvertex_no = pvertices[nclique_size+i].nvertex_no;
		if(ppadj_lists[nvertex_no]!=NULL)
		{
			for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
			{
				norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
				if(norder>=0)
				{
					pvertices[norder].nclique_deg++;
					pvertices[norder].ncand_deg--;
				}
			}
		}
		if(pplvl2_nbs[nvertex_no]!=NULL)
		{
			for(j=1;j<=pplvl2_nbs[nvertex_no][0];j++)
			{
				norder = gpvertex_order_map[pplvl2_nbs[nvertex_no][j]];
				if(norder>=0)
					gpcounters[norder]++;
			}
		}
	}
	for(i=0;i<nclique_size;i++)
	{
		if(gpcounters[i]!=num_of_necvtx)
			break;
	}
	if(i<nclique_size)
	{
		memcpy(pvertices, pclique, sizeof(VERTEX)*nclique_size);
		num_of_cands = 0;
		for(i=0;i<num_of_vertices;i++)
			gpvertex_order_map[pvertices[i].nvertex_no] = -1;
		return;
	}
	for(i=0;i<num_of_necvtx;i++)
	{
		if(gpcounters[nclique_size+i]<num_of_necvtx-1)
			break;
	}
	if(i<num_of_necvtx)
	{
		memcpy(pvertices, pclique, sizeof(VERTEX)*nclique_size);
		num_of_cands = 0;
		for(i=0;i<num_of_vertices;i++)
			gpvertex_order_map[pvertices[i].nvertex_no] = -1;
		return;
	}

	nclique_size += num_of_necvtx;
	num_of_cands -= num_of_necvtx;
	pcands = &pvertices[nclique_size];

	pcand_clqdegs = gptemp_array;
	//---------------------------------------------------------------------------------
	//removing vertices not a level-1 or level-2 neighbor of the newly added vertices
	num_of_valid_cands = 0;
	for(i=nclique_size;i<num_of_vertices;i++)
	{
		if(gpcounters[i]==num_of_necvtx)
		{
			pvertices[i].bto_be_extended = true;
			if(i<nclique_size+num_of_cands && pvertices[i].ncand_deg+pvertices[i].nclique_deg>=GetMinDeg(nclique_size+1))
				pcand_clqdegs[num_of_valid_cands++] = pvertices[i].nclique_deg;
		}
		else
			pvertices[i].bto_be_extended = false;
	}
	//---------------------------------------------------------------------

	if(num_of_valid_cands>0)
	{
		qsort(pcand_clqdegs, num_of_valid_cands, sizeof(int), comp_int_des);
		CalcLUBound(pvertices, nclique_size, pcand_clqdegs, num_of_valid_cands, pclq_stat);
	}

	//==================================================================================================
	if(num_of_valid_cands>0 && pclq_stat->nmax_cands>0 && pclq_stat->nmax_cands>=pclq_stat->nmin_cands)
	{
		num_of_valid_cands = 0;
		num_of_rmved_cands = 0;
		for(i=0;i<num_of_cands;i++)
		{
			if(pcands[i].bto_be_extended && IsValidCand(&pcands[i], nclique_size, pclq_stat))
				gpremain_vertices[num_of_valid_cands++] = i;
			else
				gpremoved_vertices[num_of_rmved_cands++] = i;
		}

		while(num_of_valid_cands>0 && num_of_rmved_cands>0)
		{
			if(num_of_valid_cands<num_of_rmved_cands)
			{
				for(i=0;i<num_of_vertices;i++)
					pvertices[i].ncand_deg = 0;
				for(i=0;i<num_of_valid_cands;i++)
				{
					nvertex_no = pcands[gpremain_vertices[i]].nvertex_no;
					if(ppadj_lists[nvertex_no]!=NULL)
					{
						for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
						{
							norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
							if(norder>=0)
								pvertices[norder].ncand_deg++;
						}
					}
				}
			}
			else
			{
				for(i=0;i<num_of_rmved_cands;i++)
				{
					nvertex_no = pcands[gpremoved_vertices[i]].nvertex_no;
					if(ppadj_lists[nvertex_no]!=NULL)
					{
						for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
						{
							norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
							if(norder>=0)
								pvertices[norder].ncand_deg--;
						}
					}
				}
			}

			num_of_valid_exts = 0;
			for(i=0;i<num_of_valid_cands;i++)
			{
				if(IsValidCand(&pcands[gpremain_vertices[i]], nclique_size, pclq_stat))
					pcand_clqdegs[num_of_valid_exts++] = pcands[gpremain_vertices[i]].nclique_deg;
			}
			if(num_of_valid_exts>0)
			{
				qsort(pcand_clqdegs, num_of_valid_exts, sizeof(int), comp_int_des);
				CalcLUBound(pvertices, nclique_size, pcand_clqdegs, num_of_valid_exts, pclq_stat);
			}
			else
				num_of_valid_cands = 0;
			if(num_of_valid_exts==0 || pclq_stat->nmax_cands==0 || pclq_stat->nmax_cands<pclq_stat->nmin_cands)
				break;

			num_of_valid_exts = 0;
			num_of_rmved_cands = 0;
			for(i=0;i<num_of_valid_cands;i++)
			{
				if(IsValidCand(&pcands[gpremain_vertices[i]], nclique_size, pclq_stat))
					gpremain_vertices[num_of_valid_exts++] = gpremain_vertices[i];
				else
					gpremoved_vertices[num_of_rmved_cands++] = gpremain_vertices[i];
			}
			num_of_valid_cands = num_of_valid_exts;
		}
	}
	//==================================================================================================

	for(i=0;i<num_of_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1;

	if(num_of_valid_cands==0 || pclq_stat->nmax_cands==0 || pclq_stat->nmax_cands<pclq_stat->nmin_cands ||
		num_of_valid_cands<pclq_stat->nmin_cands)
	{
		num_of_cands = 0;
		return;
	}

	if(num_of_valid_cands<num_of_cands)
	{
		for(i=0;i<num_of_valid_cands;i++)
			pcands[i] = pcands[gpremain_vertices[i]];

		num_of_new_tails = 0;
		for(i=num_of_cands;i<num_of_cands+num_of_tail_vertices;i++)
		{
			if(pcands[i].bto_be_extended)
				pcands[num_of_valid_cands+num_of_new_tails] = pcands[i];
		}
		num_of_cands = num_of_valid_cands;
		num_of_tail_vertices = num_of_new_tails;
	}
}

bool Graph::GenCondGraph(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices)
{
	int num_of_vertices, num_of_new_vertices, i, j, **pplvl2_nbs, **ppnew_lvl2_nbs, ncand_nbs;
	int nlist_len, nvertex_no, norder, **ppnew_adjlists, **ppadj_lists;
	bool bnew_condgraph;

	num_of_new_vertices = nclique_size+num_of_cands+num_of_tail_vertices;

	for(i=0;i<num_of_new_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i;

	if(mnum_of_cond_graphs==0)
		num_of_vertices = mnum_of_vertices;
	else
		num_of_vertices = mpcond_graphs[mnum_of_cond_graphs-1].num_of_vertices;

	if(mnum_of_cond_graphs<mnmax_cond_graphs && num_of_vertices>10 && num_of_new_vertices<=num_of_vertices/2)
	{
		mpcond_graphs[mnum_of_cond_graphs].pcur_page = gocondgraph_buf.pcur_page;
		mpcond_graphs[mnum_of_cond_graphs].ncur_pos = gocondgraph_buf.ncur_pos;
		//===================================================================
		if(!mblvl2_flag)
		{
			for(i=nclique_size;i<nclique_size+num_of_cands;i++)
				pvertices[i].nlvl2_nbs = 0;
		}
		else
		{
			ppnew_lvl2_nbs = &gpcondgraph_listpt_buf[mnum_of_cond_graphs*2*mnum_of_vertices];
			memset(ppnew_lvl2_nbs, 0, sizeof(int*)*mnum_of_vertices);
			mpcond_graphs[mnum_of_cond_graphs].pplvl2_nbs = ppnew_lvl2_nbs;
			if(mnum_of_cond_graphs==0)
				pplvl2_nbs = mpplvl2_nbs;
			else
				pplvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs;
			for(i=nclique_size;i<num_of_new_vertices;i++)
			{
				nlist_len = 0;
				ncand_nbs = 0;
				nvertex_no = pvertices[i].nvertex_no;
				if(pplvl2_nbs[nvertex_no]!=NULL)
				{
					for(j=1;j<=pplvl2_nbs[nvertex_no][0];j++)
					{
						norder = gpvertex_order_map[pplvl2_nbs[nvertex_no][j]];
						if(norder>=0)
						{
							gptemp_array[nlist_len++] = pplvl2_nbs[nvertex_no][j];
							if(pvertices[norder].bis_cand)
								ncand_nbs++;
						}
					}
				}
				if(nlist_len>0)
				{
					ppnew_lvl2_nbs[nvertex_no] = NewCGIntArray(nlist_len+1);
					ppnew_lvl2_nbs[nvertex_no][0] = nlist_len;
					memcpy(&ppnew_lvl2_nbs[nvertex_no][1], gptemp_array, sizeof(int)*nlist_len);
				}
				pvertices[i].nlvl2_nbs = ncand_nbs;
			}
		}
		//===================================================================

		//--------------------------------------------------------------------
		ppnew_adjlists = &gpcondgraph_listpt_buf[(mnum_of_cond_graphs*2+1)*mnum_of_vertices];
		memset(ppnew_adjlists, 0, sizeof(int*)*mnum_of_vertices);
		mpcond_graphs[mnum_of_cond_graphs].ppadj_lists = ppnew_adjlists;
		if(mnum_of_cond_graphs==0)
			ppadj_lists = mppadj_lists;
		else
			ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;
		for(i=0;i<num_of_new_vertices;i++)
		{
			nlist_len = 0;
			ncand_nbs = 0;
			nvertex_no = pvertices[i].nvertex_no;
			if(ppadj_lists[nvertex_no]!=NULL)
			{
				for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
				{
					norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
					if(norder>=0)
					{
						gptemp_array[nlist_len++] = ppadj_lists[nvertex_no][j];
						if(pvertices[norder].bis_cand)
							ncand_nbs++;
					}
				}
			}
			if(pvertices[i].ncand_deg!=ncand_nbs)
				printf("Error: inconsistent candidate degree 2\n");
			if(nlist_len>0)
			{
				ppnew_adjlists[nvertex_no] = NewCGIntArray(nlist_len+1);
				ppnew_adjlists[nvertex_no][0] = nlist_len;
				memcpy(&ppnew_adjlists[nvertex_no][1], gptemp_array, sizeof(int)*nlist_len);
			}
		}
		//-------------------------------------------------------------------------------
		mpcond_graphs[mnum_of_cond_graphs].num_of_vertices = num_of_new_vertices;
		mnum_of_cond_graphs++;
		bnew_condgraph = true;
	}
	else
	{
		if(!mblvl2_flag)
		{
			for(i=nclique_size;i<nclique_size+num_of_cands;i++)
				pvertices[i].nlvl2_nbs = 0;
		}
		else
		{
			if(mnum_of_cond_graphs==0)
				pplvl2_nbs = mpplvl2_nbs;
			else
				pplvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs;
			for(i=nclique_size;i<nclique_size+num_of_cands;i++)
			{
				nvertex_no = pvertices[i].nvertex_no;
				ncand_nbs = 0;
				if(pplvl2_nbs[nvertex_no]!=NULL)
				{
					for(j=1;j<=pplvl2_nbs[nvertex_no][0];j++)
					{
						norder = gpvertex_order_map[pplvl2_nbs[nvertex_no][j]];
						if(norder>=nclique_size && pvertices[norder].bis_cand)
							ncand_nbs++;
					}
				}
				pvertices[i].nlvl2_nbs = ncand_nbs;
			}
		}
		bnew_condgraph = false;
	}

	for(i=0;i<num_of_new_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1;

	return bnew_condgraph;
}

//new local variable for openMP
void Graph::ForceGenCondGraph1(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, Graph& split_g)
{
	int * gpvertex_order_map  = new int[mnum_of_vertices]; // map[vertex_no] = position in pvertices[.], this is needed as vertices change their positions in pvertices[.]
	memset(gpvertex_order_map, -1, sizeof(int)*mnum_of_vertices);
	int * gptemp_array = new int[mnum_of_vertices];

	int num_of_vertices, num_of_new_vertices, i, j, **pplvl2_nbs, **ppnew_lvl2_nbs, ncand_nbs;
	int nlist_len, nvertex_no, norder, **ppnew_adjlists, **ppadj_lists;

	num_of_new_vertices = nclique_size+num_of_cands+num_of_tail_vertices;

	//record current order
	for(i=0;i<num_of_new_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i;

	if(mnum_of_cond_graphs==0)
		num_of_vertices = mnum_of_vertices;
	else//use last mpcond_graph's num_of_vertices
		num_of_vertices = mpcond_graphs[mnum_of_cond_graphs-1].num_of_vertices;

	//===================================================================
	//*** to set new mpcond_graphs's pplvl2_nbs
	if(!mblvl2_flag)
	{
		for(i=nclique_size;i<nclique_size+num_of_cands;i++)
			pvertices[i].nlvl2_nbs = 0;
	}
	else
	{
		//set a new lvl2_nbs for new cond_graph
		//ppnew_lvl2_nbs and ppnew_adjlists are both int ** with size == mnum_of_vertices
		ppnew_lvl2_nbs = new int*[mnum_of_vertices];
		memset(ppnew_lvl2_nbs, 0, sizeof(int*)*mnum_of_vertices);
		split_g.mpplvl2_nbs = ppnew_lvl2_nbs;


		//get previous lvl2_nbs and prune it
		if(mnum_of_cond_graphs==0)
			pplvl2_nbs = mpplvl2_nbs;
		else
			pplvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs;
		for(i=nclique_size;i<num_of_new_vertices;i++) //loop v in cand and tail
		{
			nlist_len = 0;
			ncand_nbs = 0;
			nvertex_no = pvertices[i].nvertex_no;
			if(pplvl2_nbs[nvertex_no]!=NULL)
			{
				for(j=1;j<=pplvl2_nbs[nvertex_no][0];j++)//loop v's 2hop nb
				{
					norder = gpvertex_order_map[pplvl2_nbs[nvertex_no][j]];
					//if this 2hop_nb is not in new_vertices(clique+cand+tail), it will be -1 in order_map
					if(norder>=0)
					{
						gptemp_array[nlist_len++] = pplvl2_nbs[nvertex_no][j];
						if(pvertices[norder].bis_cand)
							ncand_nbs++;
					}
				}
			}
			//set new cond_graph's ppnew_lvl2_nbs
			if(nlist_len>0)
			{
				ppnew_lvl2_nbs[nvertex_no] = new int[nlist_len+1];
				ppnew_lvl2_nbs[nvertex_no][0] = nlist_len;
				memcpy(&ppnew_lvl2_nbs[nvertex_no][1], gptemp_array, sizeof(int)*nlist_len);
			}
			//always update pvertices' 2hop
			pvertices[i].nlvl2_nbs = ncand_nbs;
		}
	}
	//===================================================================
	//*** to set new mpcond_graphs's ppadj_lists
	//--------------------------------------------------------------------
	ppnew_adjlists = new int*[mnum_of_vertices];
	memset(ppnew_adjlists, 0, sizeof(int*)*mnum_of_vertices);

	split_g.mppadj_lists = ppnew_adjlists;

	//get last ppadj_lists and then prune it. similar like 2hop prune above
	if(mnum_of_cond_graphs==0)
		ppadj_lists = mppadj_lists;
	else
		ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;
	for(i=0;i<num_of_new_vertices;i++)
	{
		nlist_len = 0;
		ncand_nbs = 0;
		nvertex_no = pvertices[i].nvertex_no;
		if(ppadj_lists[nvertex_no]!=NULL)
		{
			for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
			{
				norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
				if(norder>=0)
				{
					gptemp_array[nlist_len++] = ppadj_lists[nvertex_no][j];
					if(pvertices[norder].bis_cand)
						ncand_nbs++;
				}
			}
		}
		if(pvertices[i].ncand_deg!=ncand_nbs)
			printf("Error: inconsistent candidate degree 3\n");
		if(nlist_len>0)
		{
			ppnew_adjlists[nvertex_no] = new int[nlist_len+1];
			ppnew_adjlists[nvertex_no][0] = nlist_len;
			memcpy(&ppnew_adjlists[nvertex_no][1], gptemp_array, sizeof(int)*nlist_len);
		}
	}
	//-------------------------------------------------------------------------------

	//reset gpvertex_order_map all to -1
	for(i=0;i<num_of_new_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1;

	delete []gpvertex_order_map;
	delete []gptemp_array;
}

void Graph::ForceGenCondGraph(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, Graph& split_g)
{

	int num_of_vertices, num_of_new_vertices, i, j, **pplvl2_nbs, **ppnew_lvl2_nbs, ncand_nbs;
	int nlist_len, nvertex_no, norder, **ppnew_adjlists, **ppadj_lists;

	num_of_new_vertices = nclique_size+num_of_cands+num_of_tail_vertices;

	//record current order
	for(i=0;i<num_of_new_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i;

	if(mnum_of_cond_graphs==0)
		num_of_vertices = mnum_of_vertices;
	else//use last mpcond_graph's num_of_vertices
		num_of_vertices = mpcond_graphs[mnum_of_cond_graphs-1].num_of_vertices;

	//===================================================================
	//*** to set new mpcond_graphs's pplvl2_nbs
	if(!mblvl2_flag)
	{
		for(i=nclique_size;i<nclique_size+num_of_cands;i++)
			pvertices[i].nlvl2_nbs = 0;
	}
	else
	{
		//set a new lvl2_nbs for new cond_graph
		//ppnew_lvl2_nbs and ppnew_adjlists are both int ** with size == mnum_of_vertices
		ppnew_lvl2_nbs = new int*[mnum_of_vertices];
		memset(ppnew_lvl2_nbs, 0, sizeof(int*)*mnum_of_vertices);
		split_g.mpplvl2_nbs = ppnew_lvl2_nbs;


		//get previous lvl2_nbs and prune it
		if(mnum_of_cond_graphs==0)
			pplvl2_nbs = mpplvl2_nbs;
		else
			pplvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs;
		for(i=nclique_size;i<num_of_new_vertices;i++) //loop v in cand and tail
		{
			nlist_len = 0;
			ncand_nbs = 0;
			nvertex_no = pvertices[i].nvertex_no;
			if(pplvl2_nbs[nvertex_no]!=NULL)
			{
				for(j=1;j<=pplvl2_nbs[nvertex_no][0];j++)//loop v's 2hop nb
				{
					norder = gpvertex_order_map[pplvl2_nbs[nvertex_no][j]];
					//if this 2hop_nb is not in new_vertices(clique+cand+tail), it will be -1 in order_map
					if(norder>=0)
					{
						gptemp_array[nlist_len++] = pplvl2_nbs[nvertex_no][j];
						if(pvertices[norder].bis_cand)
							ncand_nbs++;
					}
				}
			}
			//set new cond_graph's ppnew_lvl2_nbs
			if(nlist_len>0)
			{
				ppnew_lvl2_nbs[nvertex_no] = new int[nlist_len+1];
				ppnew_lvl2_nbs[nvertex_no][0] = nlist_len;
				memcpy(&ppnew_lvl2_nbs[nvertex_no][1], gptemp_array, sizeof(int)*nlist_len);
			}
			//always update pvertices' 2hop
			pvertices[i].nlvl2_nbs = ncand_nbs;
		}
	}
	//===================================================================
	//*** to set new mpcond_graphs's ppadj_lists
	//--------------------------------------------------------------------
	ppnew_adjlists = new int*[mnum_of_vertices];
	memset(ppnew_adjlists, 0, sizeof(int*)*mnum_of_vertices);

	split_g.mppadj_lists = ppnew_adjlists;

	//get last ppadj_lists and then prune it. similar like 2hop prune above
	if(mnum_of_cond_graphs==0)
		ppadj_lists = mppadj_lists;
	else
		ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;
	for(i=0;i<num_of_new_vertices;i++)
	{
		nlist_len = 0;
		ncand_nbs = 0;
		nvertex_no = pvertices[i].nvertex_no;
		if(ppadj_lists[nvertex_no]!=NULL)
		{
			for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
			{
				norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
				if(norder>=0)
				{
					gptemp_array[nlist_len++] = ppadj_lists[nvertex_no][j];
					if(pvertices[norder].bis_cand)
						ncand_nbs++;
				}
			}
		}
		if(pvertices[i].ncand_deg!=ncand_nbs)
			printf("Error: inconsistent candidate degree 3\n");
		if(nlist_len>0)
		{
			ppnew_adjlists[nvertex_no] = new int[nlist_len+1];
			ppnew_adjlists[nvertex_no][0] = nlist_len;
			memcpy(&ppnew_adjlists[nvertex_no][1], gptemp_array, sizeof(int)*nlist_len);
		}
	}
	//-------------------------------------------------------------------------------

	//reset gpvertex_order_map all to -1
	for(i=0;i<num_of_new_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1;

}


//bool Graph::ForceGenCondGraph(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, Graph& split_g)
//{
//
//	int num_of_vertices, num_of_new_vertices, i, j, **pplvl2_nbs, **ppnew_lvl2_nbs, ncand_nbs;
//	int nlist_len, nvertex_no, norder, **ppnew_adjlists, **ppadj_lists;
//	bool bnew_condgraph;
//
//	num_of_new_vertices = nclique_size+num_of_cands+num_of_tail_vertices;
//
//	//record current order
//	for(i=0;i<num_of_new_vertices;i++)
//		gpvertex_order_map[pvertices[i].nvertex_no] = i;
//
//	if(mnum_of_cond_graphs==0)
//		num_of_vertices = mnum_of_vertices;
//	else//use last mpcond_graph's num_of_vertices
//		num_of_vertices = mpcond_graphs[mnum_of_cond_graphs-1].num_of_vertices;
//
//	bnew_condgraph = (mnum_of_cond_graphs<mnmax_cond_graphs && num_of_vertices>10 && num_of_new_vertices<=num_of_vertices/2);
//	if(bnew_condgraph)
//	{
//		mpcond_graphs[mnum_of_cond_graphs].pcur_page = gocondgraph_buf.pcur_page;
//		mpcond_graphs[mnum_of_cond_graphs].ncur_pos = gocondgraph_buf.ncur_pos;
//	}
//	//===================================================================
//	//*** to set new mpcond_graphs's pplvl2_nbs
//	if(!mblvl2_flag)
//	{
//		for(i=nclique_size;i<nclique_size+num_of_cands;i++)
//			pvertices[i].nlvl2_nbs = 0;
//	}
//	else
//	{
//		//set a new lvl2_nbs for new cond_graph
//		//ppnew_lvl2_nbs and ppnew_adjlists are both int ** with size == mnum_of_vertices
//		if(bnew_condgraph)
//		{
//			ppnew_lvl2_nbs = &gpcondgraph_listpt_buf[mnum_of_cond_graphs*2*mnum_of_vertices];
//			memset(ppnew_lvl2_nbs, 0, sizeof(int*)*mnum_of_vertices);
//			mpcond_graphs[mnum_of_cond_graphs].pplvl2_nbs = ppnew_lvl2_nbs;
//		} else {
//			ppnew_lvl2_nbs = new int*[mnum_of_vertices];
//			memset(ppnew_lvl2_nbs, 0, sizeof(int*)*mnum_of_vertices);
//		}
//		split_g.mpplvl2_nbs = ppnew_lvl2_nbs;
//
//
//		//get previous lvl2_nbs and prune it
//		if(mnum_of_cond_graphs==0)
//			pplvl2_nbs = mpplvl2_nbs;
//		else
//			pplvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs;
//		for(i=nclique_size;i<num_of_new_vertices;i++) //loop v in cand and tail
//		{
//			nlist_len = 0;
//			ncand_nbs = 0;
//			nvertex_no = pvertices[i].nvertex_no;
//			if(pplvl2_nbs[nvertex_no]!=NULL)
//			{
//				for(j=1;j<=pplvl2_nbs[nvertex_no][0];j++)//loop v's 2hop nb
//				{
//					norder = gpvertex_order_map[pplvl2_nbs[nvertex_no][j]];
//					//if this 2hop_nb is not in new_vertices(clique+cand+tail), it will be -1 in order_map
//					if(norder>=0)
//					{
//						gptemp_array[nlist_len++] = pplvl2_nbs[nvertex_no][j];
//						if(pvertices[norder].bis_cand)
//							ncand_nbs++;
//					}
//				}
//			}
//			//set new cond_graph's ppnew_lvl2_nbs
//			if(nlist_len>0)
//			{
//				if(bnew_condgraph)
//					ppnew_lvl2_nbs[nvertex_no] = NewCGIntArray(nlist_len+1);
//				else
//					ppnew_lvl2_nbs[nvertex_no] = new int[nlist_len+1];
//
//				ppnew_lvl2_nbs[nvertex_no][0] = nlist_len;
//				memcpy(&ppnew_lvl2_nbs[nvertex_no][1], gptemp_array, sizeof(int)*nlist_len);
//			}
//			//always update pvertices' 2hop
//			pvertices[i].nlvl2_nbs = ncand_nbs;
//		}
//	}
//	//===================================================================
//	//*** to set new mpcond_graphs's ppadj_lists
//	//--------------------------------------------------------------------
//	if(bnew_condgraph)
//	{
//		ppnew_adjlists = &gpcondgraph_listpt_buf[(mnum_of_cond_graphs*2+1)*mnum_of_vertices];
//		memset(ppnew_adjlists, 0, sizeof(int*)*mnum_of_vertices);
//		mpcond_graphs[mnum_of_cond_graphs].ppadj_lists = ppnew_adjlists;
//	} else {
//		ppnew_adjlists = new int*[mnum_of_vertices];
//		memset(ppnew_adjlists, 0, sizeof(int*)*mnum_of_vertices);
//	}
//	split_g.mppadj_lists = ppnew_adjlists;
//
//	//get last ppadj_lists and then prune it. similar like 2hop prune above
//	if(mnum_of_cond_graphs==0)
//		ppadj_lists = mppadj_lists;
//	else
//		ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;
//	for(i=0;i<num_of_new_vertices;i++)
//	{
//		nlist_len = 0;
//		ncand_nbs = 0;
//		nvertex_no = pvertices[i].nvertex_no;
//		if(ppadj_lists[nvertex_no]!=NULL)
//		{
//			for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
//			{
//				norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
//				if(norder>=0)
//				{
//					gptemp_array[nlist_len++] = ppadj_lists[nvertex_no][j];
//					if(pvertices[norder].bis_cand)
//						ncand_nbs++;
//				}
//			}
//		}
//		if(pvertices[i].ncand_deg!=ncand_nbs)
//			printf("Error: inconsistent candidate degree\n");
//		if(nlist_len>0)
//		{
//			if(bnew_condgraph)
//				ppnew_adjlists[nvertex_no] = NewCGIntArray(nlist_len+1);
//			else
//				ppnew_adjlists[nvertex_no] = new int[nlist_len+1];
//
//			ppnew_adjlists[nvertex_no][0] = nlist_len;
//			memcpy(&ppnew_adjlists[nvertex_no][1], gptemp_array, sizeof(int)*nlist_len);
//		}
//	}
//	//-------------------------------------------------------------------------------
//	if(bnew_condgraph)
//	{
//		mpcond_graphs[mnum_of_cond_graphs].num_of_vertices = num_of_new_vertices;
//		mnum_of_cond_graphs++;
//	}
//
//	/*
//	else //always update pvertices' 2hop even don't shrink the graph
//	{
//		if(!mblvl2_flag)
//		{
//			for(i=nclique_size;i<nclique_size+num_of_cands;i++)
//				pvertices[i].nlvl2_nbs = 0;
//		}
//		else
//		{
//			if(mnum_of_cond_graphs==0)
//				pplvl2_nbs = mpplvl2_nbs;
//			else
//				pplvl2_nbs = mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs;
//			for(i=nclique_size;i<nclique_size+num_of_cands;i++)
//			{
//				nvertex_no = pvertices[i].nvertex_no;
//				ncand_nbs = 0;
//				if(pplvl2_nbs[nvertex_no]!=NULL)
//				{
//					for(j=1;j<=pplvl2_nbs[nvertex_no][0];j++)
//					{
//						norder = gpvertex_order_map[pplvl2_nbs[nvertex_no][j]];
//						if(norder>=nclique_size && pvertices[norder].bis_cand)
//							ncand_nbs++;
//					}
//				}
//				//always update pvertices' 2hop
//				pvertices[i].nlvl2_nbs = ncand_nbs;
//			}
//		}
//		bnew_condgraph = false;
//	}
//	*/
//
//	//reset gpvertex_order_map all to -1
//	for(i=0;i<num_of_new_vertices;i++)
//		gpvertex_order_map[pvertices[i].nvertex_no] = -1;
//
//	return bnew_condgraph;
//}


//only delete the last one
void Graph::DelCondGraph()
{
	if(mnum_of_cond_graphs==0)
		printf("Error: no conditional graph is constructed\n");

	gocondgraph_buf.pcur_page = mpcond_graphs[mnum_of_cond_graphs-1].pcur_page;
	gocondgraph_buf.ncur_pos = mpcond_graphs[mnum_of_cond_graphs-1].ncur_pos;

	mpcond_graphs[mnum_of_cond_graphs-1].pplvl2_nbs = NULL;
	mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists = NULL;
	mnum_of_cond_graphs--;
}

//This is actually the cover prune
int Graph::ReduceCands(VERTEX *pvertices, int nclique_size, int num_of_cands, bool &bis_subsumed)
{
	int i, j, nremoved_vertices;
	int nvertex_no, norder, ndiscnt_clq_vertices, nmin_deg;
	int nmax_clq_deg, nmaxdeg_pos, nmaxdeg_canddeg;
	bool bmaxdeg_iscand;
	VERTEX *pcands, onevertex;
	int **ppadj_lists;

	//qsort(&pvertices[nclique_size], num_of_cands, sizeof(VERTEX), comp_vertex_freq);
	//return 0;

	nmin_deg = gpmin_degs[nclique_size+2]-1;

	nremoved_vertices = 0;

	pcands = &pvertices[nclique_size];

	nmax_clq_deg = 0;
	nmaxdeg_canddeg = 0;
	nmaxdeg_pos = 0;
	bmaxdeg_iscand = true;
	for(i=0;i<num_of_cands;i++)
	{
		if(nmax_clq_deg<pcands[i].nclique_deg)
		{
			nmax_clq_deg = pcands[i].nclique_deg;
			nmaxdeg_pos = i;
			nmaxdeg_canddeg = pcands[i].ncand_deg;
			bmaxdeg_iscand = pcands[i].bis_cand;
		}
		else if(nmax_clq_deg==pcands[i].nclique_deg)
		{
			if(nmaxdeg_canddeg<pcands[i].ncand_deg)
			{
				nmaxdeg_pos = i;
				nmaxdeg_canddeg = pcands[i].ncand_deg;
				bmaxdeg_iscand = pcands[i].bis_cand;
			}
			else if(nmaxdeg_canddeg==pcands[i].ncand_deg)
			{
				if(bmaxdeg_iscand && !pcands[i].bis_cand)
				{
					nmaxdeg_pos = i;
					bmaxdeg_iscand = pcands[i].bis_cand;
				}
			}
		}
	}
	if(nmax_clq_deg==0)
		return 0;

	if(mnum_of_cond_graphs==0)
		ppadj_lists = mppadj_lists;
	else//it use the mpcond_graphs here
		ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;

	for(i=0;i<nclique_size+num_of_cands;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i;

	if(pcands[nmaxdeg_pos].nclique_deg==nclique_size)
	{
		if(pcands[nmaxdeg_pos].ncand_deg>0)
		{
			nvertex_no = pcands[nmaxdeg_pos].nvertex_no;
			if(ppadj_lists[nvertex_no]!=NULL)
			{
				for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
				{
					norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
					if(norder>=nclique_size)
					{//the covered vertex will not extend in the future.
						pvertices[norder].bto_be_extended = false;
						if(pvertices[norder].bis_cand)
							nremoved_vertices++;
					}
				}
			}
			if(nremoved_vertices!=pcands[nmaxdeg_pos].ncand_deg)
				printf("Error: inconsistent candidate degree 4\n");
			if(!pcands[nmaxdeg_pos].bis_cand)
				bis_subsumed = true;
		}
	}
	else if(pcands[nmaxdeg_pos].nclique_deg>=nmin_deg)// && pcands[nmaxdeg_pos].bis_cand)
	{
		memset(gptemp_array, 0, sizeof(int)*(nclique_size+num_of_cands));
		nvertex_no = pcands[nmaxdeg_pos].nvertex_no;
		if(ppadj_lists[nvertex_no]!=NULL)
		{
			for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
			{
				norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
				if(norder>=0 && (norder<nclique_size || pvertices[norder].bis_cand))
					gptemp_array[norder] = 1;
			}
		}
		ndiscnt_clq_vertices = 0;
		for(i=0;i<nclique_size;i++)
		{// here is to find the v in nclique which is not u's nb
			if(gptemp_array[i]==0)
			{
				if(pvertices[i].nclique_deg>=nmin_deg)
				{
					ndiscnt_clq_vertices++;
					nvertex_no = pvertices[i].nvertex_no;
					if(ppadj_lists[nvertex_no]!=NULL)
					{
						for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
						{
							norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
							if(norder>=nclique_size && pvertices[norder].bis_cand)
								gptemp_array[norder]++;
						}
					}
				}
				else
					break;
			}
		}
		if(i>=nclique_size)
		{
			for(i=nclique_size;i<nclique_size+num_of_cands;i++)
			{
				if(pvertices[i].bis_cand && gptemp_array[i]==ndiscnt_clq_vertices+1)
				{
					pvertices[i].bto_be_extended = false;
					nremoved_vertices++;
				}
			}
		}
	}
	for(i=0;i<nclique_size+num_of_cands;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1;

	if(nremoved_vertices>0 && pcands[nmaxdeg_pos].bis_cand)
	{
		if(nmaxdeg_pos!=0)
		{
			onevertex = pcands[0];
			pcands[0] = pcands[nmaxdeg_pos];
			pcands[nmaxdeg_pos] = onevertex;
		}
		qsort(&pvertices[nclique_size+1], num_of_cands-1, sizeof(VERTEX), comp_vertex_freq);
	}
	else
		qsort(&pvertices[nclique_size], num_of_cands, sizeof(VERTEX), comp_vertex_freq);

	return nremoved_vertices;
}

int Graph::GenTailVertices(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, int ncur_pos, VERTEX *pnew_vertices, int nnew_clique_size)
{
	VERTEX *ptail_vertices;
	int i, j, num_of_new_tails, **ppadj_lists, nvertex_no, norder;

	ptail_vertices = &pnew_vertices[nnew_clique_size];

	for(i=0;i<num_of_tail_vertices;i++)
		ptail_vertices[i] = pvertices[nclique_size+num_of_cands+i];
	num_of_new_tails = num_of_tail_vertices;
	for(i=nclique_size;i<ncur_pos;i++)
		ptail_vertices[num_of_new_tails++] = pvertices[i];

	for(i=0;i<num_of_new_tails;i++)
		gpvertex_order_map[ptail_vertices[i].nvertex_no] = i;

	if(mnum_of_cond_graphs==0)
		ppadj_lists = mppadj_lists;
	else
		ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;

	for(j=nclique_size;j<nnew_clique_size;j++)
	{
		nvertex_no = pnew_vertices[j].nvertex_no;
		if(ppadj_lists[nvertex_no]!=NULL)
		{
			for(i=1;i<=ppadj_lists[nvertex_no][0];i++)
			{
				norder = gpvertex_order_map[ppadj_lists[nvertex_no][i]];
				if(norder>=0)
					ptail_vertices[norder].nclique_deg++;
			}
		}
	}
	for(i=0;i<num_of_new_tails;i++)
		gpvertex_order_map[ptail_vertices[i].nvertex_no] = -1;

	for(i=0;i<nnew_clique_size+num_of_new_tails;i++)
		pnew_vertices[i].ncand_deg = 0;

	return num_of_new_tails;
}

int Graph::ReduceTailVertices(VERTEX* pvertices, int nclique_size, int num_of_tail_vertices, int** ppadj_lists)
{
	VERTEX *ptail_vertices;
	int num_of_vertices, num_of_tails, num_of_freq_tails, nrm_start, nrm_end, i, j, nvertex_no, norder;

	num_of_vertices = nclique_size+num_of_tail_vertices;

	for(i=0;i<num_of_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i;

	ptail_vertices = &pvertices[nclique_size];
	num_of_tails = num_of_tail_vertices;
	num_of_freq_tails = 0;
	nrm_start = 0;
	nrm_end = 0;
	for(i=0;i<num_of_tail_vertices;i++)
	{
		if(ptail_vertices[i].ncand_deg+ptail_vertices[i].nclique_deg>=gpmin_degs[nclique_size+2])
		{
			if(ptail_vertices[i].nclique_deg>=gpmin_degs[nclique_size+1] ||
				ptail_vertices[i].nclique_deg+ptail_vertices[i].ncand_deg>=gpmin_degs[nclique_size+1+ptail_vertices[i].ncand_deg])
			{
				ptail_vertices[i].bis_cand = true;
				gpremain_vertices[num_of_freq_tails++] = i;
			}
			else
			{
				ptail_vertices[i].bis_cand = false;
				gpremoved_vertices[nrm_end++] = i;
			}
		}
		else
		{
			ptail_vertices[i].bis_cand = false;
			gpremoved_vertices[nrm_end++] = i;
		}
	}
	while(num_of_tails>0 && num_of_freq_tails<num_of_tails/2)
	{
		num_of_tails = num_of_freq_tails;
		for(i=0;i<num_of_vertices;i++)
			pvertices[i].ncand_deg = 0;
		for(i=0;i<num_of_tails;i++)
		{
			nvertex_no = ptail_vertices[gpremain_vertices[i]].nvertex_no;
			if(ppadj_lists[nvertex_no]!=NULL)
			{
				for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
				{
					norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
					if(norder>=0)
						pvertices[norder].ncand_deg++;
				}
			}
		}
		num_of_freq_tails = 0;
		nrm_end = 0;
		nrm_start = 0;
		for(i=0;i<num_of_tails;i++)
		{
			norder = gpremain_vertices[i];
			if(ptail_vertices[norder].ncand_deg+ptail_vertices[norder].nclique_deg<gpmin_degs[nclique_size+2])
			{
				ptail_vertices[norder].bis_cand = false;
				gpremoved_vertices[nrm_end++] = norder;
			}
			else if(ptail_vertices[norder].nclique_deg<gpmin_degs[nclique_size+1] &&
				ptail_vertices[norder].nclique_deg+ptail_vertices[norder].ncand_deg<gpmin_degs[nclique_size+1+ptail_vertices[norder].ncand_deg])
			{
				ptail_vertices[norder].bis_cand = false;
				gpremoved_vertices[nrm_end++] = norder;
			}
			else
				gpremain_vertices[num_of_freq_tails++] = norder;

		}
	}
	num_of_tails = num_of_freq_tails;

	while(nrm_end>nrm_start)
	{
		nvertex_no = ptail_vertices[gpremoved_vertices[nrm_start]].nvertex_no;
		if(ppadj_lists[nvertex_no]!=NULL)
		{
			for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
			{
				norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
				if(norder>=0)
				{
					pvertices[norder].ncand_deg--;
					norder = norder-nclique_size;
					if(norder>=0 && ptail_vertices[norder].bis_cand)
					{
						if(ptail_vertices[norder].nclique_deg+ptail_vertices[norder].ncand_deg<gpmin_degs[nclique_size+2])
						{
							gpremoved_vertices[nrm_end++] = norder;
							ptail_vertices[norder].bis_cand = false;
							num_of_tails--;
						}
						else if(ptail_vertices[norder].nclique_deg<gpmin_degs[nclique_size+1] &&
							ptail_vertices[norder].nclique_deg+ptail_vertices[norder].ncand_deg<gpmin_degs[nclique_size+1+ptail_vertices[norder].ncand_deg])
						{
							ptail_vertices[norder].bis_cand = false;
							gpremoved_vertices[nrm_end++] = norder;
							num_of_tails--;
						}
					}
				}
			}
		}
		nrm_start++;
	}

	for(i=0;i<num_of_vertices;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1;

	if(num_of_tails>0)
	{
		num_of_freq_tails = 0;
		for(i=0;i<num_of_tail_vertices;i++)
		{
			if(ptail_vertices[i].bis_cand)
			{
				ptail_vertices[num_of_freq_tails] = ptail_vertices[i];
				ptail_vertices[num_of_freq_tails].bto_be_extended = true;
				num_of_freq_tails++;
			}
		}
		num_of_tail_vertices = num_of_freq_tails;
		if(num_of_freq_tails!=num_of_tails)
			printf("Error: inconsistent number of tail vertices 5\n");
	}
	else
		num_of_tail_vertices = 0;

	return num_of_tail_vertices;

}

void Graph::OutputOneClique(VERTEX *pvertices, int nclique_size, int num_of_tail_vertices, FILE *gfpout)
{
	VERTEX *pclique, *ptail_vertices;
	int i, j, num_of_vertices, nvertex_no, norder, nmin_deg, nmax_clq_deg;
	bool bismaximal;
	int **ppadj_lists;

	pclique = pvertices;
	ptail_vertices = &pvertices[nclique_size];
	if(num_of_tail_vertices==0)
		Output1Clique(pclique, nclique_size, gfpout);
	else
	{
		nmin_deg = gpmin_degs[nclique_size+1];
		bismaximal = true;
		nmax_clq_deg = 0;
		for(i=0;i<num_of_tail_vertices;i++)
		{
			if(ptail_vertices[i].nclique_deg==nclique_size)
			{
				bismaximal = false;
				break;
			}
			if(nmax_clq_deg<ptail_vertices[i].nclique_deg)
				nmax_clq_deg = ptail_vertices[i].nclique_deg;
		}
		if(bismaximal && nmax_clq_deg>=nmin_deg)
		{
			for(i=0;i<nclique_size;i++)
			{
				if(pclique[i].nclique_deg<nmin_deg)
					break;
			}
			if(i>=nclique_size)
				bismaximal = false;
		}
		if(bismaximal)
		{
			num_of_vertices = nclique_size+num_of_tail_vertices;

			if(mnum_of_cond_graphs==0)
				ppadj_lists = mppadj_lists;
			else
				ppadj_lists = mpcond_graphs[mnum_of_cond_graphs-1].ppadj_lists;

			//-----------------------------------------------------------------
			//update candidate degree of vertices
			for(i=0;i<num_of_vertices;i++)
			{
				pvertices[i].ncand_deg = 0;
				gpvertex_order_map[pvertices[i].nvertex_no] = i;
			}
			for(i=0;i<num_of_tail_vertices;i++)
			{
				for(j=0;j<nclique_size;j++)
					pvertices[j].bis_cand = false;
				nvertex_no = ptail_vertices[i].nvertex_no;
				if(ppadj_lists[nvertex_no]!=NULL)
				{
					for(j=1;j<=ppadj_lists[nvertex_no][0];j++)
					{
						norder = gpvertex_order_map[ppadj_lists[nvertex_no][j]];
						if(norder>=0)
						{
							pvertices[norder].ncand_deg++;
							pvertices[norder].bis_cand = true;
						}
					}
				}
				if(ptail_vertices[i].nclique_deg>=nmin_deg)
				{
					for(j=0;j<nclique_size;j++)
					{
						if(pvertices[j].nclique_deg<nmin_deg && !pvertices[j].bis_cand)
							break;
					}
					if(j>=nclique_size)
					{
						bismaximal = false;
						break;
					}
				}
			}
			for(i=0;i<nclique_size;i++)
			{
				if(pvertices[i].nclique_deg+pvertices[i].ncand_deg<gpmin_degs[nclique_size+2])
					break;
			}
			if(i<nclique_size)
				num_of_tail_vertices = 0;

			for(i=0;i<num_of_vertices;i++)
				gpvertex_order_map[pvertices[i].nvertex_no] = -1;
			//-----------------------------------------------------------------


			if(bismaximal && num_of_tail_vertices>1)
				num_of_tail_vertices = ReduceTailVertices(pvertices, nclique_size, num_of_tail_vertices, ppadj_lists);

			if(bismaximal && num_of_tail_vertices>1)
			{
				nmin_deg = gpmin_degs[nclique_size+2];
				for(i=0;i<nclique_size;i++)
				{
					if(pvertices[i].nclique_deg+pvertices[i].ncand_deg<nmin_deg)
						break;
				}
				if(i>=nclique_size)
				{
					qsort(ptail_vertices, num_of_tail_vertices, sizeof(VERTEX), comp_vertex_clqdeg);
					bismaximal = CheckMaximal(pvertices, nclique_size, num_of_tail_vertices);
				}
			}
		}
		if(bismaximal)
			Output1Clique(pclique, nclique_size, gfpout);
	}
}


bool Graph::CheckMaximal(VERTEX* pvertices, int nclique_size, int num_of_exts)
{
	VERTEX *pnew_vertices;
	int i, j, nisvalid, nmin_deg, nmin_ext_deg, num_of_vertices, num_of_new_cands, num_of_tail_vertices;
	bool bis_maximal;
	CLQ_STAT one_clq_stat;

	bis_maximal = true;

	nmin_deg = gpmin_degs[nclique_size+1];
	nmin_ext_deg = gpmin_degs[nclique_size+2];

	num_of_vertices = nclique_size+num_of_exts;
	pnew_vertices = new VERTEX[num_of_vertices];
	for(i=nclique_size;i<num_of_vertices;i++)
	{
		if(i>nclique_size)
			nisvalid = RemoveCandVertex(pvertices, nclique_size, num_of_exts, 0, i);
		else
			nisvalid = 1;

		if(nisvalid==-1)
			break;
		else if(nisvalid==1)
		{
			num_of_new_cands = AddOneVertex(pvertices, nclique_size, num_of_exts, 0, i, false, pnew_vertices, num_of_tail_vertices, &one_clq_stat);
			for(j=0;j<=nclique_size;j++)
			{
				if(pnew_vertices[j].nclique_deg<nmin_deg)
					break;
			}
			if(j>nclique_size)
			{
				bis_maximal = false;
				break;
			}
			if(num_of_new_cands>0) // && gnmaxcheck_depth<=2)
			{
				//qsort(&pnew_vertices[nclique_size+1], num_of_new_cands, sizeof(VERTEX), comp_vertex_clqdeg);
				bis_maximal = CheckMaximal(pnew_vertices, nclique_size+1, num_of_new_cands);
				if(!bis_maximal)
					break;
			}
		}
	}
	delete []pnew_vertices;


	return bis_maximal;
}

int Graph::LoadGraph(char* szgraph_file) // create 1-hop neighbors
{
	Data *pdata;
	Transaction *ptransaction;
	int num_of_vertices, nbuf_size, nvertex_no, nbuf_pos;
	int *padj_lens, nsize, *ptemp, nlist_len, i, nmax_deg;

	pdata = new Data(szgraph_file);

	nsize = 5000; // capacity of padj_lens
	padj_lens = new int[nsize]; // buffer for keeping each transaction for parsing
	memset(padj_lens, 0, sizeof(int)*nsize);

	num_of_vertices = 0;
	nbuf_size = 0; // ---> all transactions are saved in one buffer.... (concat adj-lists)
	nmax_deg = 0; // track longest transaction, just for printing at Line 1855...
	ptransaction = pdata->getNextTransaction();
	while(ptransaction)
	{
		if(nmax_deg<ptransaction->length)
			nmax_deg = ptransaction->length;
		if(num_of_vertices>=nsize)
		{
			ptemp = new int[2*nsize]; // ptemp is used to double the capacity of padj_lens
			memcpy(ptemp, padj_lens, sizeof(int)*nsize);
			memset(&ptemp[nsize], 0, sizeof(int)*nsize);
			delete []padj_lens;
			padj_lens = ptemp;
			nsize *= 2;
		}
		padj_lens[num_of_vertices] = ptransaction->length;
		num_of_vertices++;
		nbuf_size += ptransaction->length; // ---> all transactions are saved in one buffer....
		ptransaction = pdata->getNextTransaction(); // line by line transaction reading
	}

	mppadj_lists = new int*[num_of_vertices]; // mppadj_lists[] keeps the start position (pointer) of each vertex's adj-list in mpadj_list_buf
	mpadj_list_buf = new int[num_of_vertices+nbuf_size];
	nvertex_no = 0;
	nbuf_pos = 0;
	pdata->reOpen(szgraph_file);
	ptransaction = pdata->getNextTransaction(); // note the rewind operation inside getNextTransaction()
	while(ptransaction)
	{
		nlist_len = 1;
		mppadj_lists[nvertex_no] = &mpadj_list_buf[nbuf_pos];
		for(i=0;i<ptransaction->length;i++)
		{
			if(ptransaction->t[i]!=nvertex_no) // remove self-loop
				mppadj_lists[nvertex_no][nlist_len++] = ptransaction->t[i];
		}
		nbuf_pos += nlist_len;
		mppadj_lists[nvertex_no][0] = nlist_len-1; // from position 1 onwards, neighbors are kept; position 0 keeps the number of neighbors (adj-list length)
		qsort(&mppadj_lists[nvertex_no][1], mppadj_lists[nvertex_no][0], sizeof(int), comp_int); // adj-lists are sorted !!!

		nvertex_no++;
		ptransaction = pdata->getNextTransaction();
	}
	delete pdata;
	delete []padj_lens;

	mnum_of_vertices = num_of_vertices;

	if(gdmin_deg_ratio<=(double)(num_of_vertices-2)/(num_of_vertices-1)) //whether 2-hop neighbors need to be considered (otherwise, it only need to consider one-hop neighbors)
	{
		mblvl2_flag = true;
		GenLevel2NBs();
		//OutputLvl2Graph("lvl2graph.txt");
	}
	else
		mblvl2_flag = false;

	//printf("Maximal vertex degree: %d\n", nmax_deg); // max_deg just for printing...

	return num_of_vertices;
}


void Graph::GenLevel2NBs()  // create 2-hop neighbors
{
	int i, j, k, *pnb_list, nlist_len, nnb_no;
	bool *pbflags;

	mpplvl2_nbs = new int*[mnum_of_vertices]; // mpplvl2_nbs[i] = node i's level-2 neighbors, first element keeps the 2-hop-list length
	pnb_list = new int[mnum_of_vertices]; // buffer to keep the concat of level-2 neighbors' adj-lists

	pbflags = new bool[mnum_of_vertices]; // true if the vertex is a 2-hop neighbor
	memset(pbflags, 0, sizeof(bool)*mnum_of_vertices);

	for(i=0;i<mnum_of_vertices;i++)
	{
		memcpy(pnb_list, &mppadj_lists[i][1], sizeof(int)*mppadj_lists[i][0]);
		nlist_len = mppadj_lists[i][0];
		for(j=1;j<=mppadj_lists[i][0];j++)
			pbflags[mppadj_lists[i][j]] = true;

		for(j=1;j<=mppadj_lists[i][0];j++)
		{
			nnb_no = mppadj_lists[i][j];
			for(k=1;k<=mppadj_lists[nnb_no][0];k++)
			{
				if(!pbflags[mppadj_lists[nnb_no][k]] && mppadj_lists[nnb_no][k]!=i)
				{
					pbflags[mppadj_lists[nnb_no][k]] = true;
					pnb_list[nlist_len++] = mppadj_lists[nnb_no][k];
				}
			}
		}
		if(nlist_len>1)
			qsort(pnb_list, nlist_len, sizeof(int), comp_int); //2-hop-list are also sorted.
		mpplvl2_nbs[i] = new int[nlist_len+1];
		mpplvl2_nbs[i][0] = nlist_len; //first element keeps the 2-hop-list length
		if(nlist_len>0)
			memcpy(&mpplvl2_nbs[i][1], pnb_list, sizeof(int)*nlist_len);

		for(j=0;j<nlist_len;j++)
			pbflags[pnb_list[j]] = false;
	}
	delete []pbflags;
	delete []pnb_list;
}

void Graph::OutputLvl2Graph(char* szoutput_filename) // seems just for debugging GenLevel2NBs() in Line 1850
{
	FILE *fp;
	int i, j, nmax_len, nmin_len, *plen_vtxnum;
	double davg_len;

	fp = fopen(szoutput_filename, "wt");
	if(fp==NULL)
	{
		printf("Error: cannot open file %s for write\n", szoutput_filename);
		return;
	}

	plen_vtxnum = new int[mnum_of_vertices+1];
	memset(plen_vtxnum, 0, sizeof(int)*(mnum_of_vertices+1));

	nmax_len = 0;
	nmin_len = 0;
	davg_len = 0;
	for(i=0;i<mnum_of_vertices;i++)
	{
		if(nmax_len<mpplvl2_nbs[i][0])
			nmax_len = mpplvl2_nbs[i][0];
		if(nmin_len==0 || nmin_len>mpplvl2_nbs[i][0])
			nmin_len = mpplvl2_nbs[i][0];
		davg_len += mpplvl2_nbs[i][0];
		plen_vtxnum[mpplvl2_nbs[i][0]]++;
		for(j=1;j<=mpplvl2_nbs[i][0];j++)
			fprintf(fp, "%d ", mpplvl2_nbs[i][j]);
		fprintf(fp, "\n");
	}
	fclose(fp);


	davg_len /= mnum_of_vertices;
/*
	for(i=nmin_len;i<=nmax_len;i++)
	{
		if(plen_vtxnum[i]>0)
			printf("%d %d\n", i, plen_vtxnum[i]);
	}
*/
	printf("Average level-2 adjacent list length: %.3f\n", davg_len);
	printf("level-2 Graph density: %.3f\n", davg_len/(mnum_of_vertices-1));

	delete []plen_vtxnum;

}

void Graph::DestroyGraph()
{
	int i;

	if(mblvl2_flag)
	{
		for(i=0;i<mnum_of_vertices;i++)
			delete []mpplvl2_nbs[i];
		delete []mpplvl2_nbs;
	}

	delete []mppadj_lists;
	delete []mpadj_list_buf;
}

void Graph::DestroySplitGraph()
{
	int i;

	if(mblvl2_flag && mpplvl2_nbs != NULL)
	{
		for(i=0;i<mnum_of_vertices;i++)
			if(mpplvl2_nbs[i] != NULL)
				delete []mpplvl2_nbs[i];
		delete []mpplvl2_nbs;
	}

	if(mppadj_lists != NULL)
	{
		for(i=0;i<mnum_of_vertices;i++)
			if(mppadj_lists[i] != NULL)
				delete []mppadj_lists[i];
		delete []mppadj_lists;
	}
}

void Graph::DelCGIntBuf()
{
	INT_PAGE *ppage;
	int ntotal_pages;

	ntotal_pages = gocondgraph_buf.ntotal_pages;
	ppage = gocondgraph_buf.phead;
	while(ppage!=NULL)
	{
		gocondgraph_buf.phead = ppage->pnext;
		delete ppage;
		ntotal_pages--;
		ppage = gocondgraph_buf.phead;
	}
	if(ntotal_pages!=0)
		printf("Error: inconsistent number of pages\n");
}

void Graph::VerifyVertices(VERTEX* pvertices, int nclique_size, int num_of_cands, int num_of_tail_vertices, bool bonly_tobeextended) //detailed checking
{
	int i, j, nvertex_no, norder, nclique_deg, ncand_deg;

	return;

	for(i=0;i<mnum_of_vertices;i++)
	{
		if(gpvertex_order_map[i]!=-1) // >>> map[vid] should have been reset to be not used
			printf("stop\n");
	}

	for(i=0;i<nclique_size+num_of_cands;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = i; // >>> set map[vid] = vobj

	for(i=0;i<nclique_size+num_of_cands+num_of_tail_vertices;i++)
	{
		nvertex_no = pvertices[i].nvertex_no;
		nclique_deg = 0;
		ncand_deg = 0;
		for(j=1;j<=mppadj_lists[nvertex_no][0];j++)
		{
			norder = gpvertex_order_map[mppadj_lists[nvertex_no][j]];
			if(norder>=0)
			{
				if(norder<nclique_size) // depending on the position "noorder" in pvertices[.], we know which segment the vertex belongs
					nclique_deg++;
				else if(norder<nclique_size+num_of_cands)
				{
					if(pvertices[norder].bis_cand)
					{
						if(!bonly_tobeextended)
							ncand_deg++;
						else if(pvertices[norder].bto_be_extended)
							ncand_deg++;
						else
							norder = norder; //??? what's the point? no effect...
					}
					else
						norder = norder;
				}
				else
					norder = norder;
			}
			else
				norder = norder;
		}
		if(nclique_deg!=pvertices[i].nclique_deg)
			printf("Error: inconsistent clique degree\n");
		if(ncand_deg!=pvertices[i].ncand_deg)
			printf("Error: inconsistent candidate degree 7\n");
	}

	for(i=0;i<nclique_size+num_of_cands;i++)
		gpvertex_order_map[pvertices[i].nvertex_no] = -1; // reset map[vid] to be not used

}

int comp_vertex_clqdeg(const void *e1, const void *e2)
{
	VERTEX *p1, *p2;

	p1 = (VERTEX*)e1;
	p2 = (VERTEX*)e2;

	if(p1->nclique_deg > p2->nclique_deg)
		return -1;
	else if(p1->nclique_deg < p2->nclique_deg)
		return 1;
	else if(p1->ncand_deg > p2->ncand_deg)
		return -1;
	else if(p1->ncand_deg < p2->ncand_deg)
		return 1;
	else if(!p1->bis_cand && p2->bis_cand)
		return -1;
	else if(p1->bis_cand && !p2->bis_cand)
		return 1;
	else if(p1->nvertex_no < p2->nvertex_no)
		return -1;
	else if(p1->nvertex_no > p2->nvertex_no)
		return 1;
	else
		return 0;
}


int comp_vertex_freq(const void *e1, const void *e2) // sort by (nclique_deg, ncand_deg)
{
	VERTEX *p1, *p2;

	p1 = (VERTEX*)e1;
	p2 = (VERTEX*)e2;

	if(p1->bis_cand && !p2->bis_cand)
		return -1;
	else if(!p1->bis_cand && p2->bis_cand)
		return 1;
	else if(p1->bto_be_extended && !p2->bto_be_extended)
		return -1;
	else if(!p1->bto_be_extended && p2->bto_be_extended)
		return 1;
	else if(p1->nclique_deg < p2->nclique_deg) // primary key: nclique_deg
		return -1;
	else if(p1->nclique_deg > p2->nclique_deg)
		return 1;
	else if(p1->ncand_deg < p2->ncand_deg) // secondary key: ncand_deg
		return -1;
	else if(p1->ncand_deg > p2->ncand_deg)
		return 1;
	else if(p1->nlvl2_nbs < p2->nlvl2_nbs) // next key: |vertices_within_2hops|
		return -1;
	else if(p1->nlvl2_nbs > p2->nlvl2_nbs)
		return 1;
	else if(p1->nvertex_no < p2->nvertex_no) // next key: ID
		return -1;
	else if(p1->nvertex_no > p2->nvertex_no)
		return 1;
	else
		return 0;
}

int comp_int(const void *e1, const void *e2)
{
	int n1, n2;
	n1 = *(int *) e1;
	n2 = *(int *) e2;

	if (n1>n2)
		return 1;
	else if (n1<n2)
		return -1;
	else
		return 0;
}

int comp_int_des(const void* e1, const void *e2)
{
	int n1, n2;
	n1 = *(int *) e1;
	n2 = *(int *) e2;

	if (n1>n2)
		return -1;
	else if (n1<n2)
		return 1;
	else
		return 0;
}


#endif



