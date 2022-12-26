#include "clique_graph.h"
#include <vector>
#include <stack>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <omp.h>

#define INFINITE 100000

clique_graph::clique_graph(/* args */)
{
}
//------------ initialize the grid_clique_graph ------------------------
clique_graph::clique_graph(int height, int width){
    //generate random num to generate random edge and clique

    unsigned seed;
    seed = time(0);
    srand(seed);
   #define vector_data_structure 0
   #if vector_data_structure
   //--- Create edges ---
   //Create horizontal edges
   vector<vector<int>> edges = vector<vector<int>>(E_num,vector<int>(2, 0));
   int edge_counter = 0;
   for(int x_coor = 0; x_coor < (width - 1); x_coor++){
        for(int y_coor = 0; y_coor < height; y_coor++){
            edges[edge_counter][0] = Vertex_id_in_grid(x_coor, y_coor, width);
            edges[edge_counter][1] = Vertex_id_in_grid(x_coor + 1, y_coor, width);
            edge_counter++;
        }
   }
   //Create vertical edges
   for(int x_coor = 0; x_coor < width; x_coor++){
        for(int y_coor = 0; y_coor < height - 1; y_coor++){
            edges[edge_counter][0] = Vertex_id_in_grid(x_coor, y_coor, width);
            edges[edge_counter][1] = Vertex_id_in_grid(x_coor, y_coor + 1, width);
            edge_counter++;
        }
   }
   if (edge_counter == E_num){
        cout << "successful initialize the edges" << endl;
   }
    //generate random edge costs
   vector<double> edge_costs = {};
   for(int e = 0; e < E_num; e++){
        double edge_cost = double(rand() % Max_edge_cost + 1);
        edge_costs.push_back(edge_cost);
   }
   //----- Create Clique -------
   //Each Clique contains 5 points
   vector<int> C_Vnum(C_num, 5);

   vector<double> clique_costs = {};
   //generate random clique costs
   for(int e = 0; e < C_num; e++){
        double clique_cost = double(rand() % Max_clique_cost + 1);
        clique_costs.push_back(clique_cost);
   }

   //Create the cliques
   vector<vector<int>> cliques;
   for(int center_xcoor = 1; center_xcoor < width - 1; center_xcoor++){
    for(int center_ycoor = 1; center_ycoor < height - 1; center_ycoor++){
        int center_Num = Vertex_id_in_grid(center_xcoor, center_ycoor, width);
        vector<int> temp_clique = {center_Num, center_Num - 1, center_Num + 1, center_Num + width, center_Num - width};
        cliques.push_back(temp_clique);
    }
   }
   //    NEW_CLIQUE_GRAPH.Initialize_edges();
    this->Initialize(V_num,E_num,C_num, edges, edge_costs, C_Vnum, clique_costs, cliques);
   #endif
    int Max_edge_cost = 100;
   int Max_clique_cost = 200;

   //Create the number of Vertex, Edge and Clique
   int V_num = height * width;
   int E_num = height * (width - 1) + width * (height - 1);
   int C_num = (height - 2) * (width - 2);

    //--- Create edges ---
   //Create horizontal edges
   int* start_points = new int[E_num];
   int* end_points = new int[E_num];
   int edge_counter = 0;
   for(int x_coor = 0; x_coor < (width - 1); x_coor++){
        for(int y_coor = 0; y_coor < height; y_coor++){
            start_points[edge_counter] = Vertex_id_in_grid(x_coor, y_coor, width);
            end_points[edge_counter] = Vertex_id_in_grid(x_coor + 1, y_coor, width);
            edge_counter++;
        }
   }
   //Create vertical edges
   for(int x_coor = 0; x_coor < width; x_coor++){
        for(int y_coor = 0; y_coor < height - 1; y_coor++){
            start_points[edge_counter] = Vertex_id_in_grid(x_coor, y_coor, width);
            end_points[edge_counter] = Vertex_id_in_grid(x_coor, y_coor + 1, width);
            edge_counter++;
        }
   }

   if (edge_counter == E_num){
        cout << "---------successful initialize the edges---------" << endl;
   }
   //generate random edge costs
   double* edge_costs = new double[E_num];
   for(int e = 0; e < E_num; e++){
        double edge_cost = double(rand() % Max_edge_cost + 1);
        edge_costs[e] = edge_cost;
   }
   //----- Create Clique -------
   //Each Clique contains 5 points
   int* C_Vnum = new int[C_num];

   double* clique_costs = new double[C_num];
   //generate random clique costs
   for(int e = 0; e < C_num; e++){
        double clique_cost = double(rand() % Max_clique_cost + 1);
        clique_costs[e] = clique_cost;
        C_Vnum[e] = 5;
   }
   //Create the cliques
   vector<int>* cliques = new vector<int>[C_num];
   int clique_counter = 0;
   for(int center_xcoor = 1; center_xcoor < width - 1; center_xcoor++){
    for(int center_ycoor = 1; center_ycoor < height - 1; center_ycoor++){
        int center_Num = Vertex_id_in_grid(center_xcoor, center_ycoor, width);
        vector<int> temp_clique = {center_Num, center_Num - 1, center_Num + 1, center_Num + width, center_Num - width};
        cliques[clique_counter] = temp_clique;
        clique_counter++;
    }
   }
   this -> Initialize_memory(V_num, E_num, C_num, start_points, end_points, edge_costs, cliques, C_Vnum, clique_costs);
   delete []edge_costs;
   delete []C_Vnum;
   delete []clique_costs;
   delete []cliques;
   delete []start_points;
   delete []end_points;
}


clique_graph::clique_graph(int V_num, int E_num, int C_num, int* edge_start_points, int* edge_end_points, double* edge_costs, vector<int>* cliques, int* C_Vnum, double* clique_costs){
    bool DEBUG_MODE = false;
    cout << "The number of vertices is " << V_num << endl;
    vertex_num = V_num;
    Vertex_adj_edge_num = new int[vertex_num];
    for (int i = 0; i < vertex_num; i++)
    {
        Vertex_adj_edge_num[i] = 0;
    }
    adjoint_edge = vector<vector<int>>(vertex_num, vector<int>());
    adjoint_edges_id = vector<vector<int>>(vertex_num, vector<int>());
    edge_cost = vector<vector<double>>(vertex_num, vector<double>());
    conflict_clique = vector<vector<int>>(vertex_num, vector<int>());


    cout << "The number of edges is " << E_num << endl;
    edge_num = E_num;
    for(int i = 0; i < edge_num; i++)
    {
        int v_1 = edge_start_points[i];
        int v_2 = edge_end_points[i];

        Vertex_adj_edge_num[v_1]++;
        Vertex_adj_edge_num[v_2]++;

        adjoint_edge[v_1].push_back(v_2);
        adjoint_edge[v_2].push_back(v_1);

        edge_cost[v_1].push_back(edge_costs[i]);
        edge_cost[v_2].push_back(edge_costs[i]);

        adjoint_edges_id[v_1].push_back(i);
        adjoint_edges_id[v_2].push_back(i);
    }

    cout << "The number of clique is" << C_num << endl;
    clique_num = C_num;
    clique_node_num = new int[clique_num];
    clique_cost = new double[clique_num];
    vertices_in_clique = vector<vector<int>>();
    for(int c_i = 0; c_i < clique_num; c_i++){
        clique_node_num[c_i] = C_Vnum[c_i];
        clique_cost[c_i] = clique_costs[c_i];
        vertices_in_clique.push_back(cliques[c_i]);
        for(int j = 0; j < C_Vnum[c_i]; j++){
            int v_j = cliques[c_i][j];
            conflict_clique[v_j].push_back(c_i);
        }
    }
    vertex_state = vector<bool>(vertex_num, false);
    vertex_cost = vector<double>(vertex_num, INFINITE);
    father_vertex = vector<int>(vertex_num, -1);
    father_edge = new int[vertex_num];
    for(int i = 0; i < vertex_num; i++){
        father_edge[i] = -1;
    }
    activated_clique = vector<vector<bool>>(vertex_num, vector<bool>(clique_num, false));
}


void clique_graph::Initialize_memory(int V_num, int E_num, int C_num, int* edge_start_points, int* edge_end_points, double* edge_costs, vector<int>* cliques, int* C_Vnum, double* clique_costs){
    bool DEBUG_MODE = false;
    cout << "The number of vertices is " << V_num << endl;
    vertex_num = V_num;
    Vertex_adj_edge_num = new int[vertex_num];
    for (int i = 0; i < vertex_num; i++)
    {
        Vertex_adj_edge_num[i] = 0;
    }
    adjoint_edge = vector<vector<int>>(vertex_num, vector<int>());
    adjoint_edges_id = vector<vector<int>>(vertex_num, vector<int>());
    edge_cost = vector<vector<double>>(vertex_num, vector<double>());
    conflict_clique = vector<vector<int>>(vertex_num, vector<int>());

    cout << "The number of edges is " << E_num << endl;
    edge_num = E_num;
    for(int i = 0; i < edge_num; i++)
    {
        int v_1 = edge_start_points[i];
        int v_2 = edge_end_points[i];

        if(DEBUG_MODE){
            cout << "The " << i <<"'th edge : " << v_1 << " to " << v_2 << " with cost " << edge_costs[i] << endl;
        }

        Vertex_adj_edge_num[v_1]++;
        Vertex_adj_edge_num[v_2]++;

        adjoint_edge[v_1].push_back(v_2);
        adjoint_edge[v_2].push_back(v_1);

        edge_cost[v_1].push_back(edge_costs[i]);
        edge_cost[v_2].push_back(edge_costs[i]);

        adjoint_edges_id[v_1].push_back(i);
        adjoint_edges_id[v_2].push_back(i);
    }

    cout << "The number of clique is" << C_num << endl;
    clique_num = C_num;
    clique_node_num = new int[clique_num];
    clique_cost = new double[clique_num];
    vertices_in_clique = vector<vector<int>>();
    for(int c_i = 0; c_i < clique_num; c_i++){
        clique_node_num[c_i] = C_Vnum[c_i];
        clique_cost[c_i] = clique_costs[c_i];
        vertices_in_clique.push_back(cliques[c_i]);
        for(int j = 0; j < C_Vnum[c_i]; j++){
            int v_j = cliques[c_i][j];
            conflict_clique[v_j].push_back(c_i);
        }
    }
    vertex_state = vector<bool>(vertex_num, false);
    vertex_cost = vector<double>(vertex_num, INFINITE);
    father_vertex = vector<int>(vertex_num, -1);
    father_edge = new int[vertex_num];
    for(int i = 0; i < vertex_num; i++){
        father_edge[i] = -1;
    }
    activated_clique = vector<vector<bool>>(vertex_num, vector<bool>(clique_num, false));
}



void clique_graph::Initialize_input(){
    cout << "please input the number of vertices" << endl;
    cin >> vertex_num;
    Vertex_adj_edge_num = new int[vertex_num];
    for (int i = 0; i < vertex_num; i++)
    {
        Vertex_adj_edge_num[i] = 0;
    }
    
    adjoint_edge = vector<vector<int>>(vertex_num, vector<int>());
    edge_cost = vector<vector<double>>(vertex_num, vector<double>());
    conflict_clique = vector<vector<int>>(vertex_num, vector<int>());


    cout << "please input the number of edge" << endl;
    cin >> edge_num;
    for(int i = 0; i < edge_num; i++)
    {
        cout << "Now input the " << i <<"'th edge : ";
        int v_1 = 0, v_2 = 0; 
        cin >> v_1 >> v_2;
        
        Vertex_adj_edge_num[v_1]++;
        Vertex_adj_edge_num[v_2]++;

        adjoint_edge[v_1].push_back(v_2);
        adjoint_edge[v_2].push_back(v_1);

        cout << "Now input the cost of this edge : ";
        double temp_cost = 0;
        cin >> temp_cost;
        edge_cost[v_1].push_back(temp_cost);
        edge_cost[v_2].push_back(temp_cost);
    }

    cout << "please input the number of clique" << endl;
    cin >> clique_num;
    clique_node_num = new int[clique_num];
    clique_cost = new double[clique_num];
    for(int i = 0; i < clique_num; i++){
        clique_node_num[i] = 0;
        clique_cost[i] = 0;
    }
    
    vertices_in_clique = vector<vector<int>>(clique_num, vector<int>());


    for(int c_i = 0; c_i < clique_num; c_i++){
        cout << "Now input the " << c_i <<"'th clique contains how many vertices : ";
        int node_num_in_clique_ci = 0;
        cin >> node_num_in_clique_ci;
        clique_node_num[c_i] = node_num_in_clique_ci;
        cout << "Now input the " << c_i <<"'th clique 's cost' : ";
        cin >> clique_cost[c_i];

        cout << "Now input the vertices in this clique: ";
        for (int j = 0; j < node_num_in_clique_ci; j++){
            int v_j;
            cin >> v_j;
            vertices_in_clique[c_i].push_back(v_j);
            conflict_clique[v_j].push_back(c_i);
        }
    }

    vertex_state = vector<bool>(vertex_num, false);
    vertex_cost = vector<double>(vertex_num, INFINITE);
    father_vertex = vector<int>(vertex_num, -1);
    father_edge = new int[vertex_num];
    for(int i = 0; i < vertex_num; i++){
        father_edge[i] = -1;
    }
    activated_clique = vector<vector<bool>>(vertex_num, vector<bool>(clique_num, false));
    return;
}

void clique_graph::Initialize(int V_num, int E_num, int C_num, vector<vector<int>> edges, vector<double> edge_costs, vector<int> C_Vnum, vector<double> clique_costs, vector<vector<int>> cliques){
    bool DEBUG_MODE = false;
    cout << "The number of vertices is " << V_num << endl;
    vertex_num = V_num;
    Vertex_adj_edge_num = new int[vertex_num];
    for (int i = 0; i < vertex_num; i++)
    {
        Vertex_adj_edge_num[i] = 0;
    }
    adjoint_edge = vector<vector<int>>(vertex_num, vector<int>());
    adjoint_edges_id = vector<vector<int>>(vertex_num, vector<int>());
    edge_cost = vector<vector<double>>(vertex_num, vector<double>());
    conflict_clique = vector<vector<int>>(vertex_num, vector<int>());


    cout << "The number of edges is " << E_num << endl;
    edge_num = E_num;
    for(int i = 0; i < edge_num; i++)
    {
        int v_1 = 0, v_2 = 0;
        v_1 = edges[i][0];
        v_2 = edges[i][1];
        if(DEBUG_MODE){
            cout << "The " << i <<"'th edge : " << v_1 << " to " << v_2 << " with cost " << edge_costs[i] << endl;
        }

        Vertex_adj_edge_num[v_1]++;
        Vertex_adj_edge_num[v_2]++;

        adjoint_edge[v_1].push_back(v_2);
        adjoint_edge[v_2].push_back(v_1);

        edge_cost[v_1].push_back(edge_costs[i]);
        edge_cost[v_2].push_back(edge_costs[i]);

        adjoint_edges_id[v_1].push_back(i);
        adjoint_edges_id[v_2].push_back(i);
    }

    cout << "The number of clique is" << C_num << endl;
    clique_num = C_num;
    clique_node_num = new int[clique_num];
    clique_cost = new double[clique_num];
    for(int i = 0; i < clique_num; i++){
        clique_node_num[i] = 0;
        clique_cost[i] = 0;
    }
    vertices_in_clique = vector<vector<int>>(clique_num, vector<int>());

    for(int c_i = 0; c_i < clique_num; c_i++){
        if(DEBUG_MODE){
            cout << "The " << c_i <<" 'th clique contains " << C_Vnum[c_i] << " vertices with cost "  << clique_costs[c_i] << endl;
        }
        int node_num_in_clique_ci = C_Vnum[c_i];

        clique_node_num[c_i] = node_num_in_clique_ci;
        clique_cost[c_i] = clique_costs[c_i];
        if(DEBUG_MODE){
            cout << "The " << c_i << " 'th clique is ";
        }
        for (int j = 0; j < node_num_in_clique_ci; j++){
            int v_j = cliques[c_i][j];
            vertices_in_clique[c_i].push_back(v_j);
            conflict_clique[v_j].push_back(c_i);
            if(DEBUG_MODE){
                cout << " " << v_j << " ";
            }
        }
        if(DEBUG_MODE){
            cout << endl;
        }
    }

    vertex_state = vector<bool>(vertex_num, false);
    vertex_cost = vector<double>(vertex_num, INFINITE);
    father_vertex = vector<int>(vertex_num, -1);
    father_edge = new int[vertex_num];
    for(int i = 0; i < vertex_num; i++){
        father_edge[i] = -1;
    }
    activated_clique = vector<vector<bool>>(vertex_num, vector<bool>(clique_num, false));
    return;
}



int clique_graph::chooseVertex(){
    int min_value = INFINITE;
    int min_index = -1;
    for(int v = 0; v < vertex_num; v++){
        if(vertex_state[v] == false && vertex_cost[v] <= min_value && vertex_cost[v] < INFINITE){
            min_index = v;
            min_value = vertex_cost[v];
        }
    }
    if(min_index != -1){
        return min_index;
    }
    else{
        cout << "The graph is not connected" << endl;
        return -1;
    }
}

void clique_graph::update_v_from_u(int u, int v, double uv_cost, int uv_id){
    if(0){
        cout << "we try to update " << v << " from " << u <<endl;
    }
    //---if v is already in the shortest tree, we ignore v---
    if (vertex_state[v] == true)
        return;
    //---Calculate the new cost of v from u---
    //initialize the cost
    int v_cost_from_u = vertex_cost[u];
    //add edge cost
    v_cost_from_u += uv_cost;
    //add clique cost
    for(int i = 0; i < conflict_clique[v].size(); i++){
        int c_index = conflict_clique[v][i];
        if(activated_clique[u][c_index] == 0){
            v_cost_from_u += clique_cost[c_index];
        }
    }
    //---check wether to update and excute update---
    if (v_cost_from_u < vertex_cost[v]){
        //--V need update,we update the Cost, Father Vertex, and activated clique--
        //update cost
        vertex_cost[v] = v_cost_from_u;
        //update Father
        father_vertex[v] = u;
        father_edge[v] = uv_id;
        //update activated clique
        activated_clique[v] = activated_clique[u];
        for(int i = 0; i < conflict_clique[v].size(); i++){
            int c_index = conflict_clique[v][i];
            if(activated_clique[v][c_index] == false){
                activated_clique[v][c_index] = true;
            }
        }
        if(0)
        {
            cout << "we update" << v << " with cost " << v_cost_from_u << endl;
        }
    }


    return;
}

vector<int> clique_graph::printSolution(int terminal_point){
    int temp_father = father_vertex[terminal_point];
    int temp_father_edge = father_edge[terminal_point];
    int counter = 1;
    stack<int> shortest_path_vertex;
    stack<int> shortest_path_edge;
    while(temp_father != -1){
        counter++;
        shortest_path_vertex.push(temp_father);
        shortest_path_edge.push(temp_father_edge);
        temp_father_edge = father_edge[temp_father];
        temp_father = father_vertex[temp_father];
    }
   vector<int> output_edge(counter - 1, 0);
    cout << "The shortest path is : "<< endl; 
    for(int i = 0; i < (counter - 1); i++){
        int newest_vertex = shortest_path_vertex.top();
        shortest_path_vertex.pop();
        cout << newest_vertex << " -> ";
    }
    cout << terminal_point << endl << "The edge id of the shortest path is :" << endl;
    for(int i = 0; i < (counter - 2); i++){
        int newest_edge = shortest_path_edge.top();
        output_edge[i] = newest_edge;
        shortest_path_edge.pop();
        cout << newest_edge << " -> ";
    }
    cout << shortest_path_edge.top() << endl;
    output_edge[counter - 2] = shortest_path_edge.top();
    shortest_path_edge.pop();
    cout << "with the cost " << vertex_cost[terminal_point] << endl;
    return output_edge;
}

void clique_graph::dijstraAll(int origin_point){
    cout << "------------ START ALGORITHM ------------" << endl;
    //---Initialize the origin point, include the origin point and the adjoint vertex
    vertex_state[origin_point] = true;
    vertex_cost[origin_point] = 0;
    for(int i = 0; i < conflict_clique[origin_point].size(); i++){
        int clique_index = conflict_clique[origin_point][i];
        activated_clique[origin_point][clique_index] = true;
        vertex_cost[origin_point] += clique_cost[clique_index];
    }
    //update the adjoint vertex
    for (int i = 0; i < Vertex_adj_edge_num[origin_point]; i++){
        int v = adjoint_edge[origin_point][i];
        double adjoint_v_cost = edge_cost[origin_point][i];
        int adjoint_edge_id = adjoint_edges_id[origin_point][i];
        update_v_from_u(origin_point, v, adjoint_v_cost, adjoint_edge_id);
    }

    for(int counter = 0; counter < (vertex_num-1) ; counter++){
        // choose the next vertex
        int u = chooseVertex();
        if(0)
        {
            cout << "the No." << counter << " vertex in the shortest tree is " << u << endl << "with cost" << vertex_cost[u] << endl;
        }
        // add u to the shortest tree
        vertex_state[u] = true;

        //update the cost of vetices adjacent with u
        for (int i = 0; i < Vertex_adj_edge_num[u]; i++){
            int v = adjoint_edge[u][i];
            double uv_cost = edge_cost[u][i];
            int uv_id = adjoint_edges_id[u][i];
            update_v_from_u(u, v, uv_cost, uv_id);
        }
    }
}

void clique_graph::dijstra(int start_point, int end_point){
    cout << "------------ START ALGORITHM ------------" << endl;
    //---Initialize the origin point, include the origin point and the adjoint vertex
    vertex_state[start_point] = true;
    vertex_cost[start_point] = 0;
    for(int i = 0; i < conflict_clique[start_point].size(); i++){
        int clique_index = conflict_clique[start_point][i];
        activated_clique[start_point][clique_index] = true;
        vertex_cost[start_point] += clique_cost[clique_index];
    }
    //update the adjoint vertex
    for (int i = 0; i < Vertex_adj_edge_num[start_point]; i++){
        int v = adjoint_edge[start_point][i];
        double adjoint_v_cost = edge_cost[start_point][i];
        int adjoint_edge_id = adjoint_edges_id[start_point][i];
        update_v_from_u(start_point, v, adjoint_v_cost, adjoint_edge_id);
    }
    int temp_point_u = -1;
    while(temp_point_u != end_point){
        temp_point_u = chooseVertex();
        vertex_state[temp_point_u] = true;
        //update the cost of vetices adjacent with u
        if(temp_point_u != end_point)
        {
            for (int i = 0; i < Vertex_adj_edge_num[temp_point_u]; i++){
                int v = adjoint_edge[temp_point_u][i];
                double uv_cost = edge_cost[temp_point_u][i];
                int uv_id = adjoint_edges_id[temp_point_u][i];
                update_v_from_u(temp_point_u, v, uv_cost, uv_id);
            }
        }
    }
}

clique_graph::~clique_graph(){
    if(vertex_num > 0){
        delete []Vertex_adj_edge_num;
        delete []father_edge;
    }
    if(clique_num > 0){
        delete []clique_node_num;
        delete []clique_cost;
    }
}



int clique_graph::Vertex_id_in_grid(int x_coor, int y_coor, int width){
    return(x_coor + y_coor * width); 
}

