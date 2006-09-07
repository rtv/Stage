/*
 * dijkstra.c - C implementation of Dijkstra's shortest path algorithm.
 * 		Changed to fit for use in audio model of Stage 
 *
 * Copyright (C) 2005-2006 W. Evan Sheehan
 * Copyright (C) 2006      Pooya Karimian
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "dijkstra.h"

GHashTable *dijkstra_adjacency;
GHashTable *dijkstra_previous;
GHashTable *dijkstra_d;

typedef struct {
    int node;
    double weight;
    gboolean temp;
} dijkstra_node;

double *double_dup(const double d)
{
    double *r = (double *) malloc(sizeof(double));
    *r = d;
    return r;
}

void initialize_dijkstra_d(int u, GList * vs, GHashTable * d)
{
//      g_hash_table_insert (d, u, GINT_TO_POINTER (-1));
    g_hash_table_insert(d, GINT_TO_POINTER(u), double_dup(INFINITY));
}

static void add_to_queue(int u, GList * vs, GQueue * q)
{
    g_queue_push_head(q, GINT_TO_POINTER(u));
}

/* Comparison function for sorting the agenda queue for Dijkstra's algorithm.
 */
static gint dijkstra_compare(int u, int v, GHashTable * d)
{
//      gint u_cost = GPOINTER_TO_INT (g_hash_table_lookup (d, GINT_TO_POINTER(u)));
//      gint v_cost;
    double *u_cost = g_hash_table_lookup(d, GINT_TO_POINTER(u));
    double *v_cost;

    /* If u is infinity, put it after v in the queue */
    if (u_cost == NULL)
	return 1;
    if (*u_cost >= INFINITY) {
	return 1;
    }
//      v_cost = GPOINTER_TO_INT (g_hash_table_lookup (d, GINT_TO_POINTER(v)));
    v_cost = g_hash_table_lookup(d, GINT_TO_POINTER(v));
    /* If v is infinitiy, put it after u. Otherwise, compare the costs
     * directly.
     */
    if (v_cost == NULL)
	return 1;
//      if (v_cost == -1) {
    if (*v_cost >= INFINITY) {
	return -1;
    } else if (*u_cost > *v_cost) {
	return 1;
    } else if (*u_cost == *v_cost) {
	return 0;
    }

    return -1;
}

static GQueue *build_dijkstra_queue(GHashTable * adjacency, GHashTable * d)
{
    GQueue *q = g_queue_new();

    g_hash_table_foreach(adjacency, (GHFunc) add_to_queue, q);
    g_queue_sort(q, (GCompareDataFunc) dijkstra_compare, d);

    return q;
}

/* Update the table of shortest path estimates. If the path that ends with the
 * edge (u,v) is shorter than the currently stored path to v, replace the cost
 * in d with the new cost, and replace the old value of previous[v] with u.
 */
static void
relax(GHashTable * d, GHashTable * previous, int u, int v, double w)
{
/*
	gint u_cost = GPOINTER_TO_INT (g_hash_table_lookup (d, GINT_TO_POINTER(u)));
	gint v_cost = GPOINTER_TO_INT (g_hash_table_lookup (d, GINT_TO_POINTER(v)));

	if (v_cost == -1 || v_cost > u_cost + 1) {
		g_hash_table_insert (d, GINT_TO_POINTER(v), GINT_TO_POINTER (u_cost + 1));
		g_hash_table_insert (previous, GINT_TO_POINTER(v), GINT_TO_POINTER(u));
	}
*/
    double *u_cost = g_hash_table_lookup(d, GINT_TO_POINTER(u));
    double *v_cost = g_hash_table_lookup(d, GINT_TO_POINTER(v));

    if (!u_cost)
	u_cost = double_dup(INFINITY);
    if (!v_cost)
	v_cost = double_dup(INFINITY);
    if ((*v_cost >= INFINITY) || ((*v_cost) > (*u_cost) + w)) {
	g_hash_table_insert(d, GINT_TO_POINTER(v),
			    double_dup((*u_cost) + w));
	g_hash_table_insert(previous, GINT_TO_POINTER(v),
			    GINT_TO_POINTER(u));
    }

}

/* The meat of Dijkstra's algorithm is here. */
void
find_shortest_paths(GHashTable * adjacency, GHashTable * d,
		    GHashTable * previous)
{
//      GSList *S = NULL;
    GQueue *agenda = build_dijkstra_queue(adjacency, d);

    /* Go through each node and calculate path costs */
    while (!g_queue_is_empty(agenda)) {
	/* Next node */
	int u = GPOINTER_TO_INT(g_queue_pop_head(agenda));
	/* Nodes connected to the current node */
	GList *vs =
	    (GList *) g_hash_table_lookup(adjacency, GINT_TO_POINTER(u));
//              printf("-GL- %p\n",vs);
//              fflush(stdout);
//              printf(" -GL#- %d\n", g_slist_length(vs));
//              fflush(stdout);
	/* Add the current node to the list of visited nodes */
//              S = g_slist_prepend (S, GINT_TO_POINTER(u));
	/* For each connected node update path costs */


	for (GList * v = vs; v; v = g_list_next(v)) {
//                      printf(" -V- %p\n",v);
//                      fflush(stdout);
	    dijkstra_node *vptr = (dijkstra_node *) v->data;
//                      printf(" -VP- %p (%d,%f)\n",vptr,vptr->node,vptr->weight);
//                      printf(" -VN- %p\n",g_slist_next(v));
//              fflush(stdout);
//                      if (vptr!=NULL)
//                      printf(" -1-\n");
//                      fflush(stdout);
	    relax(d, previous, u, vptr->node, vptr->weight);
//                      printf(" -2-\n");
//                      fflush(stdout);
//                      printf(" -3-\n");
//                      fflush(stdout);
	}

	/* Re-sort the queue so that the node with the minimum cost is
	 * in front
	 */
	g_queue_sort(agenda, (GCompareDataFunc) dijkstra_compare, d);
    }

    /* Clean up */
//      g_slist_free (S);
    g_queue_free(agenda);
}

void double_destroy(double *d)
{
//    printf("double_destory: %p\n");
    free(d);
}

void dijkstra_init()
{
    /* Shortest path estimate */
    dijkstra_previous = g_hash_table_new(g_direct_hash, g_direct_equal);
    dijkstra_adjacency = g_hash_table_new(g_direct_hash, g_direct_equal);
    dijkstra_d =
	g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
			      (GDestroyNotify) double_destroy);
}

void dijkstra_destroy()
{
    /* Clean up */
    g_hash_table_destroy(dijkstra_d);
    g_hash_table_destroy(dijkstra_previous);
// TODO: clean up    
//    free_adjacency (adjacency);
}

void dijkstra_remove_temp_edges2(gpointer key, GList * vs, gpointer adj)
{
//    printf("%d (%d): ",GPOINTER_TO_INT(key), g_list_length(vs));
//    g_list_foreach(adj, (GFunc)dijkstra_remove_temp_edge, adj);
//    while(((dijkstra_node*)adj->data)->temp) {

    while (vs) {
	dijkstra_node *vptr = (dijkstra_node *) vs->data;
	if (!vptr->temp)
	    break;
	//printf("%c",vptr->temp?'T':'.');
//        GList *tmp=g_list_next(vs);
//      vs=tmp;
	free(vptr);
	vs = g_list_delete_link(vs, vs);
    }
//    printf("\n");
//    fflush(stdout);
    g_hash_table_replace(adj, GINT_TO_POINTER(key), vs);
}

void dijkstra_remove_temp_edges()
{
    g_hash_table_foreach(dijkstra_adjacency,
			 (GHFunc) dijkstra_remove_temp_edges2,
			 dijkstra_adjacency);
}

void dijkstra_insert_edge(int u, int v, double w, gboolean temp_edge)
{
    GList *vs = (GList *) g_hash_table_lookup(dijkstra_adjacency,
					      GINT_TO_POINTER(u));

    dijkstra_node *vptr = (dijkstra_node *) malloc(sizeof(dijkstra_node));
    vptr->node = v;
    vptr->weight = w;
    vptr->temp = temp_edge;
//    printf("list adding pointer: %p\n",vptr);
//    printf("list before: %p\n",vs);
    vs = g_list_prepend(vs, vptr);
//    printf("list after: %p\n",vs);
//    fflush(stdout);
    g_hash_table_replace(dijkstra_adjacency, GINT_TO_POINTER(u), vs);

//    if (temp_edge) {
//      g_slist_prepend(dijkstra_temp_edges, vs);
//    }
}


void dijkstra_free_d(int u, double *dist, gpointer junk)
{
    free(dist);
}

void dijkstra_run()
{
    /* Initialize shortest path estimates */
//    g_hash_table_foreach(dijkstra_d, dijkstra_free_d, NULL);
    g_hash_table_destroy(dijkstra_d);
//    dijkstra_d = g_hash_table_new (g_direct_hash, g_direct_equal);
    dijkstra_d =
	g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
			      (GDestroyNotify) double_destroy);

    g_hash_table_foreach(dijkstra_adjacency,
			 (GHFunc) initialize_dijkstra_d, dijkstra_d);
    // start node is connected to start node with 0 distance
    g_hash_table_insert(dijkstra_d, GINT_TO_POINTER(0), double_dup(0.0));

    find_shortest_paths(dijkstra_adjacency, dijkstra_d, dijkstra_previous);
}
