#ifndef _CLIQUE_GRAPH_H_
#define _CLIQUE_GRAPG_H_
#include <iostream>

#include <vector>
using namespace std;


class clique_graph
{
private:

    // ----- the variables to show the graph -------

    //the number of vertices, cliques and edges
    int     vertex_num;
    int     clique_num;
    int     edge_num;
    
    //node-edge adjacency list
    /* 
    [ Vertex_adj_edge_num ] : the number of edges adjacent to a vertex
    [ adjoint_edge ]        : the edge adjacent to a vertex
    [ edge_cost ]           : cost of each edge
    [ conflict_clique ]     : vertex is included in witch cliques
    */
    int*                    Vertex_adj_edge_num;
    vector<vector<int>>     adjoint_edge;
    vector<vector<double>>  edge_cost;
    vector<vector<int>>     adjoint_edges_id;

    vector<vector<int>>     conflict_clique;

    //clique
    /*
    [ clique_node_num ]         : the number of nodes in a specific clique
    [ clique_cost ]             : cost of each clique
    [ vertices_in_clique ]      : each clique contains witch vertices
    */ 
    int*                    clique_node_num;
    double*                 clique_cost;
    vector<vector<int>>     vertices_in_clique;


    // ------ the variables to excute the dijstra algorithm ------
    /* the state of a vertex to show weather it is inclued in the shortest path tree.
    false   for not inclue in the shortest path tree. 
    true    for include in the shortest path tree.
    */
    vector<bool>            vertex_state;
    // the output, dist[i] show the ith vertext's shortest distance form origin vertex
    vector<double>          vertex_cost;
    // take down the shortest path's father vertex
    vector<int>             father_vertex;
    // weather a clique is activated
    vector<vector<bool>>    activated_clique;

    int*                    father_edge;





public:
    clique_graph();
    clique_graph(int, int);


    void Initialize_input();
    void Initialize(int V_num, int E_num, int C_num, vector<vector<int>> edges, vector<double> edge_costs, vector<int> C_Vnum, vector<double> clique_costs, vector<vector<int>> cliques);
    void Initialize_memory(int V_num, int E_num, int C_num, int* edge_start_points, int* edge_end_points, double* edge_costs, vector<int>* cliques, int* C_Vnum, double* clique_costs);
    int chooseVertex();
    void update_v_from_u(int u, int v, double uv_cost, int uv_id);
    void dijstraAll(int);
    vector<int> printSolution(int);


    //auxiliary function
    int Vertex_id_in_grid(int, int, int);


    // version 20221214

    // use this to create the graph with the edges and the cliques.
    clique_graph(int V_num, int E_num, int C_num, int* edge_start_points, int* edge_end_points, double* edge_costs, vector<int>* cliques, int* C_Vnum, double* clique_costs);

    //? what about multi-origin and multi-sinks ?
    void dijstra(int first_point, int last_point); 
    ~clique_graph();
};



#endif