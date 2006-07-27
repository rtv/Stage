#ifndef _DIJKSTRA_H_
#define _DIJKSTRA_H_

#include <glib.h>

//void initialize_dijkstra_d (int u, GList *vs, GHashTable *d);
//void find_shortest_paths (GHashTable* adjacency, GHashTable* d, GHashTable* previous);
void dijkstra_init();
void dijkstra_destroy();
//void dijkstra_set_graph_size(int n);
//void dijkstra_remove_edges_from(int u);
//void dijkstra_remove_edges_to(int u);
//void dijkstra_remove_edges_gte(int u);
//void dijkstra_insert_edge(int u, int v, double w);
void dijkstra_insert_edge(int u, int v, double w, gboolean temp_edge);
//void dijkstra_insert_tmp_edge(int u, int v, double w);
void dijkstra_remove_temp_edges();
void dijkstra_run();

GHashTable *dijkstra_adjacency;
GHashTable *dijkstra_previous;
GHashTable *dijkstra_d;

#endif
