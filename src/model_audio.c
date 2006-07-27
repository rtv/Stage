///////////////////////////////////////////////////////////////////////////
//
// File: model_audio.c
// Authors: Pooya Karimian
// Date: July 24 2006
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_audio.c,v $
//  $Author: pooya $
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_audio Audio model 

<b>WARNING:</b> This is an experimental implementation. Any aspect of model, 
implementation, messages, and world file properties may change in future.

This is a model of audio emitter and omni-directional sensors. It
models audio propagation through the world as simple shortest-path method.

<h2>Worldfile properties</h2>
@par Summary and default values

@verbatim
audio
(
    say ""
    say_period 1
)
@endverbatim

@par Details
- say "string"
  - the string that is announced automatically every <i>say_period</i> times. 
    Can be used to emulate the behaviour of a beacon.
- say_period int
  - in milliseconds simulation time

<h2>Author</h2>
Pooya Karimian
*/


//#include <sys/time.h>
#include <math.h>
#include "gui.h"
#include "dijkstra.h"

//#define DEBUG

#define AUDIO_HEARING_RANGE 	(INFINITY)
//#define AUDIO_HEARING_RANGE   (10.0)

#define AUDIO_USE_CACHE
//#define AUDIO_MAX_CACHE_SIZE  (1000)
//#define AUDIO_CACHE_STATS

#define AUDIO_FIG_DESTROY_LATER

//#define AUDIO_DEBUG_ALLOC(...) {printf("Audio:memcheck:"); printf(__VA_ARGS__); fflush(stdout);} 
#define AUDIO_DEBUG_ALLOC(...) {}

#define AUDIO_DEBUG1(...) {printf("Audio: "); printf(__VA_ARGS__); fflush(stdout);}

//#define AUDIO_DEBUG2(...) {printf("Audio debug2: "); printf(__VA_ARGS__); fflush(stdout);} 
#define AUDIO_DEBUG2(...) {}

#include "stage_internal.h"

static int audio_points_n = 0;
static GArray *audio_points;

#ifdef AUDIO_USE_CACHE
static GHashTable *audio_pointpair_cache;
static GQueue *audio_pointpair_queue;
#ifdef AUDIO_CACHE_STATS
static unsigned long int audio_cache_n;
static unsigned long int audio_cache_n_hits;
#endif
#endif

static GArray *audiodev_models;
static GArray *audiodev_pos;
static int audiodev_n;

static double audio_min_resolution;

void audio_free_path(gpointer data, gpointer user);

/*
typedef struct {
    stg_model_t *mod1;
    stg_model_t *mod2;
} audio_model_pair_t;

guint audio_model_pair_hash (gconstpointer v)
{
  audio_model_pair_t* ap=( audio_model_pair_t* ) v;
  return (ap->mod1->id ^ ap->mod2->id);
}

gboolean audio_model_pair_equal (gconstpointer v1,
		gconstpointer v2)
{
  audio_model_pair_t* ap1=( audio_model_pair_t* ) v1;
  audio_model_pair_t* ap2=( audio_model_pair_t* ) v2;
  return  ( (ap1->mod1->id==ap2->mod1->id) && (ap1->mod2->id==ap2->mod2->id) ) ||
	  ( (ap1->mod1->id==ap2->mod2->id) && (ap1->mod2->id==ap2->mod1->id) );
}
*/

typedef struct {
    stg_pose_t p1;
    stg_pose_t p2;
} audio_pointpair_key_t;

guint audio_pointpair_hash(gconstpointer v)
{
    audio_pointpair_key_t *pp = (audio_pointpair_key_t *) v;
//    return ((int) (pp->p1.x + pp->p1.y) ^ (int) (pp->p2.x + pp->p2.y));
    return ((guint) (pp->p1.x + pp->p1.y) ^ (guint) (pp->p2.x + pp->p2.y));
}

gboolean audio_stg_pose_t_equal(const stg_pose_t p1, const stg_pose_t p2)
{
    return
	(fabs(p1.x - p2.x) < audio_min_resolution) &&
	(fabs(p1.y - p2.y) < audio_min_resolution);
}

gboolean audio_pointpair_equal(gconstpointer v1, gconstpointer v2)
{
    audio_pointpair_key_t *pp1 = (audio_pointpair_key_t *) v1;
    audio_pointpair_key_t *pp2 = (audio_pointpair_key_t *) v2;

    return
	audio_stg_pose_t_equal(pp1->p1, pp2->p1) &&
	audio_stg_pose_t_equal(pp1->p2, pp2->p2);

/*
    return  
	    (audio_stg_pose_t_equal(pp1->p1,pp2->p1) && 
	     audio_stg_pose_t_equal(pp1->p2,pp2->p2)) ||
	    (audio_stg_pose_t_equal(pp1->p1,pp2->p2) && 
	     audio_stg_pose_t_equal(pp1->p2,pp2->p1))
	   ;
*/
//    return ((pp1->p1.x == pp2->p1.x) && (pp1->p1.y == pp2->p1.y) &&
//          (pp1->p2.x == pp2->p2.x) && (pp1->p2.y == pp2->p2.y));
}

/*
typedef struct {
  double dist;
  GList *lines;
} audio_pointpair_val_t;
*/
typedef struct {
    double dist;
    double angle;
    GList *lines;
} audio_path_t;

/*
void audio_pointpair_print ( gpointer key, gpointer data, gpointer user )
{
  audio_pointpair_key_t* pp_key = (audio_pointpair_key_t*) key;
  audio_path_t* pp_val = (audio_path_t*) data;
  
  printf("[(%f,%f)->(%f,%f)=%f]\n",pp_key->p1.x,pp_key->p1.y,pp_key->p2.x,pp_key->p2.y,
     pp_val->dist);
}
*/

void audio_pointpair_key_destroy(gpointer data)
{
    AUDIO_DEBUG_ALLOC("free:key_destroy:%p\n", data);
    free((audio_pointpair_key_t *) data);
}


void audio_path_destroy(gpointer data)
{
//    printf("path destroy: %p\n",data);
    audio_free_path(data, NULL);
}

/*
gboolean audio_free_cache(gpointer key, gpointer value, gpointer user_data) {
    printf("cachefree: %p %p\n",key,value);
//    audio_pointpair_key_destroy(key);
//    audio_path_destroy(value);
    return TRUE;
}
*/
// standard callbacks
int audio_update(stg_model_t * mod);
int audio_startup(stg_model_t * mod);
int audio_shutdown(stg_model_t * mod);
void audio_load(stg_model_t * mod);

int audio_render_data(stg_model_t * mod, void *userp);
int audio_render_cfg(stg_model_t * mod, void *userp);
int audio_unrender_data(stg_model_t * mod, void *userp);
int audio_unrender_cfg(stg_model_t * mod, void *userp);

/*
double *audio_double_dup(const double d)
{
    double *r = (double *) malloc(sizeof(double));
    *r = d;
    return r;
}
*/

stg_pose_t *audio_stg_pose_dup(const stg_pose_t * d)
{
    stg_pose_t *r = (stg_pose_t *) malloc(sizeof(stg_pose_t));
    memcpy(r, d, sizeof(stg_pose_t));
    AUDIO_DEBUG_ALLOC("malloc:pose_dup:%p\n", r);
    return r;
}

void audio_load(stg_model_t * mod)
{

    // TODO: read audio params from the world file
    //  stg_model_set_cfg( mod, &cfg, sizeof(cfg));
    AUDIO_DEBUG2("Load\n");

}

int audio_init(stg_model_t * mod)
{

    AUDIO_DEBUG2("Init\n");
    // we don't consume any power until subscribed
    // mod->watts = 0.0; 

    // override the default methods; these will be called by the simualtion
    // engine
    mod->f_startup = audio_startup;
    mod->f_shutdown = audio_shutdown;
    mod->f_update = audio_update;
    mod->f_load = audio_load;

    // sensible audio defaults; it doesn't take up any physical space
    stg_geom_t geom;
    geom.pose.x = 0.0;
    geom.pose.y = 0.0;
    geom.pose.a = 0.0;
    geom.size.x = 0.0;
    geom.size.y = 0.0;
    stg_model_set_geom(mod, &geom);

    // TODO: how about a global set invisible function?
    // is this overridable from .world file?
    // audio is invisible
    stg_model_set_obstacle_return(mod, 0);
    stg_model_set_laser_return(mod, LaserTransparent);
    stg_model_set_blob_return(mod, 0);
    stg_model_set_audio_return(mod, 0);
    stg_model_set_color(mod, (stg_color_t) 0);

    stg_audio_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    if (wf_property_exists(mod->id, "say")) {
	char *st = (char *) wf_read_string(mod->id, "say", NULL);
	if (st) {
	    strncpy(cfg.say_string, st, STG_AUDIO_MAX_STRING_LEN);
	}
    } else {
	cfg.say_string[0] = 0;
    }

    cfg.say_period = wf_read_int(mod->id, "say_period", 1);

    stg_model_set_cfg(mod, &cfg, sizeof(cfg));

    // sensible starting command
    stg_audio_cmd_t cmd;
    cmd.cmd = STG_AUDIO_CMD_NOP;
    cmd.string[0] = 0;
    stg_model_set_cmd(mod, &cmd, sizeof(cmd));

    stg_audio_data_t data;
    memset(&data, 0, sizeof(data));
    data.audio_paths = NULL;
    stg_model_set_data(mod, &data, sizeof(data));

// adds a menu item and associated on-and-off callbacks
    stg_model_add_property_toggles(mod, &mod->data,
				   audio_render_data, NULL,
				   audio_unrender_data, NULL,
				   "audio_data", "audio data", TRUE);

    stg_model_add_property_toggles(mod, &mod->cfg, audio_render_cfg,	// called when toggled on
				   NULL, audio_unrender_cfg,	// called when toggled off
				   NULL,
				   "audiocfg", "audio config", FALSE);

    return 0;
}



void audio_render_line(int8_t * buf,
		       int w, int h,
		       int x1, int y1, int x2, int y2, int8_t val)
{
    // if both ends of the line are out of bounds, there's nothing to do
    if (x1 < 0 && x1 > w && y1 < 0 && y1 > h &&
	x2 < 0 && x2 > w && y2 < 0 && y2 > h)
	return;

    // if the two ends are adjacent, fill in the pixels
    if (abs(x2 - x1) <= 1 && abs(y2 - y1) <= 1) {
	if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h) {
	    if (x1 == w)
		x1 = w - 1;
	    if (y1 == h)
		y1 = h - 1;

	    buf[y1 * w + x1] = val;
	}


	if (x2 >= 0 && x2 <= w && y2 >= 0 && y2 <= h) {
	    if (x2 == w)
		x2 = w - 1;
	    if (y2 == h)
		y2 = h - 1;

	    buf[y2 * w + x2] = val;
	}
    } else			// recursively draw two half-lines
    {
	int xm = x1 + (x2 - x1) / 2;
	int ym = y1 + (y2 - y1) / 2;
	audio_render_line(buf, w, h, x1, y1, xm, ym, val);
	audio_render_line(buf, w, h, xm, ym, x2, y2, val);
    }
}

void audio_render_line2(int8_t * buf,
			int w, int h,
			int x1, int y1, int x2, int y2, int8_t val)
{
    // if both ends of the line are out of bounds, there's nothing to do
    if (x1 < 0 && x1 > w && y1 < 0 && y1 > h &&
	x2 < 0 && x2 > w && y2 < 0 && y2 > h)
	return;

    // if the two ends are adjacent, fill in the pixels
    if (abs(x2 - x1) <= 1 && abs(y2 - y1) <= 1) {
	if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h) {
	    if (x1 == w)
		x1 = w - 1;
	    if (y1 == h)
		y1 = h - 1;

	    buf[y1 * w + x1] = val;

	    if (x1 != 0)
		buf[y1 * w + x1 - 1] = val;
	    if (x1 != w - 1)
		buf[y1 * w + x1 + 1] = val;
	    if (y1 != 0)
		buf[(y1 - 1) * w + x1] = val;
	    if (y1 != h - 1)
		buf[(y1 + 1) * w + x1] = val;

	}

	if (x2 >= 0 && x2 <= w && y2 >= 0 && y2 <= h) {
	    if (x2 == w)
		x2 = w - 1;
	    if (y2 == h)
		y2 = h - 1;

	    buf[y2 * w + x2] = val;

	    if (x2 != 0)
		buf[y2 * w + x2 - 1] = val;
	    if (x2 != w - 1)
		buf[y2 * w + x2 + 1] = val;
	    if (y2 != 0)
		buf[(y2 - 1) * w + x2] = val;
	    if (y2 != h - 1)
		buf[(y2 + 1) * w + x2] = val;
	}
    } else			// recursively draw two half-lines
    {
	int xm = x1 + (x2 - x1) / 2;
	int ym = y1 + (y2 - y1) / 2;
	audio_render_line2(buf, w, h, x1, y1, xm, ym, val);
	audio_render_line2(buf, w, h, xm, ym, x2, y2, val);
    }
}

//int audio_pkpn=0;
//double audio_pkp[400][2];

int audio_raytrace_match(stg_model_t * mod, stg_model_t * hitmod)
{
    // TODO: any other condition?
    if (hitmod->audio_return == 1)
	return 1;
    return 0;
}


int
audio_checkvisible(stg_model_t * mod, double x1, double y1, double x2,
		   double y2)
{
    if (hypot(x1 - x2, y1 - y2) > AUDIO_HEARING_RANGE)
	return 0;

    itl_t *itl = itl_create(x1, y1,
			    x2, y2,
			    mod->world->matrix,
			    PointToPoint);

    // TODO: how about the extended match that ignores current cell?
    stg_model_t *hitmod =
	itl_first_matching(itl, audio_raytrace_match, mod);
    if (!hitmod) {
	stg_pose_t pose1, pose2;
	pose1.x = x1;
	pose1.y = y1;
	pose2.x = x2;
	pose2.y = y2;

//      stg_model_global_to_local (mod, &pose1);
//      stg_model_global_to_local (mod, &pose2);
//      stg_rtk_fig_line (fig, pose1.x, pose1.y, pose2.x, pose2.y);
    }
//        stg_rtk_fig_ellipse(fig, pose1.x, pose1.y, 0.0, 0.05,0.05,1);

//        printf("%d",hitmod?1:0);

    itl_destroy(itl);

    if (!hitmod) {
	return 1;
    }
    return 0;
}

#define GMC(X,Y) (mapdata[(X)+(Y)*w])
#define PX2G(X)	 ((X)*mres-geom.size.x/2.0)
#define PY2G(Y)	 ((Y)*mres-geom.size.y/2.0)
#define GX2P(X)	 ((int) ( ( (X) + geom.size.x/2.0 ) / mres ) )
#define GY2P(Y)	 ((int) ( ( (Y) + geom.size.y/2.0 ) / mres ) )

void audio_findpoints(gpointer key, gpointer data, gpointer user)
{
//  stg_model_t* mod = 
//      stg_world_model_name_lookup( thismod->world, "cave" );
//      stg_world_model_name_lookup( thismod->world, "hospital" );
//      stg_model_t *topmod = (stg_model_t *) user;
    stg_model_t *mod = (stg_model_t *) data;

    if (mod->audio_return != 1)
	return;

    AUDIO_DEBUG1("Audio: Finding points in model \"%s\": ", mod->token);

    int oi = 0;
    int oj = 0;

    stg_geom_t geom;
    stg_model_get_geom(mod, &geom);
    double mres = mod->map_resolution;

    AUDIO_DEBUG2("WORLD: %lf\n", (1.0 / mod->world->ppm));
    if (mres < (1.0 / mod->world->ppm)) {
	PRINT_WARN1
	    ("Error\nModel '%s' resolution is less than the world resolution, using world resolution instead",
	     mod->token);
	mres = (1.0 / mod->world->ppm);
    }
    if (mres < audio_min_resolution)
	audio_min_resolution = mres;
//  printf("Mres: %f\n",mres);

    int w = (int) ceil(geom.size.x / mres);
    int h = (int) ceil(geom.size.y / mres);
    int8_t *mapdata;
    mapdata = (int8_t *) malloc(w * h * sizeof(uint8_t));
    assert(mapdata);
    memset(mapdata, 0, w * h * sizeof(uint8_t));

    size_t polycount = 0;
    stg_polygon_t *polys = stg_model_get_polygons(mod, &polycount);

//  int pknumpoints=0;
    for (int p = 0; p < (int) polycount; p++) {
	// draw each line in the poly
	int line_count = (int) polys[p].points->len;
	for (int l = 0; l < line_count; l++) {
	    stg_point_t *pt1 =
		&g_array_index(polys[p].points, stg_point_t, l);
	    stg_point_t *pt2 = &g_array_index(polys[p].points, stg_point_t,
					      (l + 1) % line_count);

	    // TODO: audio_render_line2
	    audio_render_line2(mapdata,
			       w, h,
			       (int) ((pt1->x +
				       geom.size.x / 2.0) / mres) - oi,
			       (int) ((pt1->y +
				       geom.size.y / 2.0) / mres) - oj,
			       (int) ((pt2->x +
				       geom.size.x / 2.0) / mres) - oi,
			       (int) ((pt2->y +
				       geom.size.y / 2.0) / mres) - oj, 1);
/*
	  printf("(%d,%d)-(%d,%d)\n",
	  		     (int) ((pt1->x + geom.size.x / 2.0) / mres) - oi,
			     (int) ((pt1->y + geom.size.y / 2.0) / mres) - oj,
			     (int) ((pt2->x + geom.size.x / 2.0) / mres) - oi,
			     (int) ((pt2->y + geom.size.y / 2.0) / mres) - oj);
*/
	}
    }

    int initial_audio_points_n = audio_points_n;
    for (int y = 1; y < h - 1; y++) {
	for (int x = 1; x < w - 1; x++) {

	    int cellb = GMC(x - 1, y - 1) + GMC(x, y - 1) * 2 +
		GMC(x + 1, y - 1) * 4 + GMC(x - 1, y) * 8 +
		GMC(x + 1, y) * 16 + GMC(x - 1, y + 1) * 32 +
		GMC(x, y + 1) * 64 + GMC(x + 1, y + 1) * 128;

	    int cellv = !GMC(x, y) & (cellb != 0) &
		(cellb != 7) & (cellb != 224) &
		(cellb != 41) & (cellb != 148) &
		(cellb != 231) & (cellb != 189) &
		(cellb != 47) & (cellb != 151) &
		(cellb != 233) & (cellb != 244) &
		(cellb != 10) & (cellb != 80) &
		(cellb != 18) & (cellb != 72) &
		(cellb != 212) & (cellb != 105) &
		(cellb != 43) & (cellb != 150) &
		(cellb != 23) & (cellb != 240) &
		(cellb != 15) & (cellb != 232) &
		(cellb != 255) & (cellb != 31) &
		(cellb != 248) & (cellb != 214) &
		(cellb != 107) & (cellb != 253) &
		(cellb != 239) & (cellb != 247) &
		(cellb != 191) & (cellb != 11) &
		(cellb != 22) & (cellb != 208) & (cellb != 104);


	    if (GMC(x, y)) {
//         printf(".");
	    } else if (cellv) {
//         pknumpoints++;

		// TODO: check 
		stg_pose_t pose;
		pose.x = PX2G(x);
		pose.y = PY2G(y);

		g_array_insert_val(audio_points, audio_points_n, pose);
		audio_points_n++;

/*		
		if (audio_points_n > AUDIO_MAX_POINTS) {
		    printf
			("Error\nAudio: Error: Maximum audio points number is %d\n",
			 AUDIO_MAX_POINTS);
		    exit(-1);
		}
*/

/*
	   if (audio_pkpn<400) {
		audio_pkp[audio_pkpn][0]=x*mres-geom.size.x/2.0;
		audio_pkp[audio_pkpn][1]=y*mres-geom.size.y/2.0;
		audio_pkpn++;
	   }
*/
//         printf("*");
	    } else {
//         printf(" ");
	    }

//          printf("%c",mapdata[x+y*w]==1?'*':' ');
	}
//      printf ("\n");
    }

    free(mapdata);
    AUDIO_DEBUG1("Found %d points\n",
		 audio_points_n - initial_audio_points_n);
/*  
  int pknLines=0;
  for (int i=0; i<audio_pkpn-1; i++) {
   for (int j=i+1; j<audio_pkpn; j++) {
//    printf("(%f,%f)->(%f,%f)\n",audio_pkp[i][0]*mres, audio_pkp[i][1]*mres,
//			    audio_pkp[j][0]*mres, audio_pkp[j][1]*mres);
        if(audio_checkvisible(thismod, audio_pkp[i][0], audio_pkp[i][1],
			        audio_pkp[j][0], audio_pkp[j][1]))			        
    	  pknLines++;
   }
  }

  printf("Audio: Found %d Lines\n",pknLines);
*/

}


void audio_create_visibilitygraph(stg_model_t * mod)
{

    long int vpairs = 0;
    int vpairs2, maxvpairs2 = 0;
    int i;

    AUDIO_DEBUG1("Audio: Building visibility graph: ");

    dijkstra_init();
    for (i = 0; i < audio_points_n; i++) {
	vpairs2 = 0;
	stg_pose_t pose_i = g_array_index(audio_points, stg_pose_t, i);
	for (int j = 0; j < audio_points_n; j++) {
	    stg_pose_t pose_j = g_array_index(audio_points, stg_pose_t, j);
	    if (i == j)
		continue;
	    // TODO: check
//        if(audio_checkvisible(thismod, 
	    if (audio_checkvisible(mod,
				   pose_i.x, pose_i.y, pose_j.x,
				   pose_j.y)) {

		vpairs++;
		vpairs2++;

		double xdist = pose_i.x - pose_j.x;
		double ydist = pose_i.y - pose_j.y;
		dijkstra_insert_edge(i + 1, j + 1, hypot(xdist, ydist),
				     FALSE);


//      printf("%d %d %5.2f\n",i,j,sqrt(xdist*xdist+ydist*ydist));
	    }
	}
	if (vpairs2 > maxvpairs2)
	    maxvpairs2 = vpairs2;

    }

    AUDIO_DEBUG1("number of edges: %ld\n", vpairs);

}

void audio_draw_path(gpointer data, gpointer user)
{
#ifdef AUDIO_FIG_DESTROY_LATER
    stg_rtk_fig_t *fig = (stg_rtk_fig_t *) user;
#else
    stg_model_t *mod = (stg_model_t *) user;
    stg_rtk_fig_t *fig = stg_model_get_fig(mod, "audio_data_fig");
#endif
    audio_path_t *path = (audio_path_t *) data;

    GList *lines = path->lines;

    while (lines != NULL) {
	GList *next = g_list_next(lines);
	if (next != NULL) {
	    stg_pose_t p1 = *(stg_pose_t *) lines->data;
	    stg_pose_t p2 = *(stg_pose_t *) next->data;

//        printf("draw_line: (%f,%f)->(%f,%f)\n", p1.x, p1.y, p2.x, p2.y);
#ifndef AUDIO_FIG_DESTROY_LATER
	    stg_model_global_to_local(mod, &p1);
	    stg_model_global_to_local(mod, &p2);
#endif
	    stg_rtk_fig_line(fig, p1.x, p1.y, p2.x, p2.y);
	}
	lines = next;
    }
}

void audio_free_path(gpointer data, gpointer user)
{
    audio_path_t *path = (audio_path_t *) data;

    GList *lines = path->lines;

    while (lines != NULL) {
	GList *next = g_list_next(lines);
	AUDIO_DEBUG_ALLOC("free:free_path_line:%p\n", lines->data);
	free((stg_pose_t *) lines->data);
	lines = next;
    }
    g_list_free(path->lines);
    AUDIO_DEBUG_ALLOC("free:free_path:%p\n", path);
    free(path);
}


void
audio_find_dist_multi(stg_model_t * mod, stg_pose_t * srcpose,
		      int ndest, GArray * devpose, double *dists,
		      double *angles)
//                   int ndest, stg_pose_t * devpose, double *dists)
{
    int dij_n = 0;

    int *dij_nodes = (int *) malloc(ndest * sizeof(int));

    stg_audio_data_t *data = (stg_audio_data_t *) mod->data;

  /**
   * Loop over all points and fill the information. 
   * If the points are directly visible and don't need dijkstra then put  
   * them at the end of list. 
   */
    for (int n = 0; n < ndest; n++) {
	dists[n] = INFINITY;
	angles[n] = 0.0;	// junk
#ifdef	AUDIO_USE_CACHE
	/*
	 * Check if already in cache
	 */
	audio_pointpair_key_t *pp_key = (audio_pointpair_key_t *)
	    malloc(sizeof(audio_pointpair_key_t));
	AUDIO_DEBUG_ALLOC("malloc:pp_key:%p\n", pp_key);

	pp_key->p1 = *srcpose;
	pp_key->p2 = g_array_index(devpose, stg_pose_t, n);
//      printf("(%f,%f)  ",pp_key->p1.x,pp_key->p1.y);
	audio_path_t *pp_val =
	    (audio_path_t *) g_hash_table_lookup(audio_pointpair_cache,
						 pp_key);
#ifdef AUDIO_CACHE_STATS
	audio_cache_n++;
#endif

	// Cache hit
	if (pp_val) {
#ifdef AUDIO_CACHE_STATS
	    audio_cache_n_hits++;
#endif
//            printf ("Using cache: (%f)\n", pp_val->dist);
	    data->audio_paths = g_list_append(data->audio_paths, pp_val);
	    dists[n] = pp_val->dist;
	    angles[n] = pp_val->angle;

	    // already in cache. key is not needed anymore
	    free(pp_key);
	    AUDIO_DEBUG_ALLOC("free:pp_key:%p\n", pp_key);
	    continue;
	}
	// Cache miss
	AUDIO_DEBUG2("cache miss\n");
#endif
      /**
       * if directly visible just calculate the distance
       */

	if (audio_checkvisible
	    (mod, srcpose->x, srcpose->y,
	     g_array_index(devpose, stg_pose_t, n).x,
	     g_array_index(devpose, stg_pose_t, n).y)) {

	    dists[n] =
		hypot(srcpose->x - g_array_index(devpose, stg_pose_t, n).x,
		      srcpose->y - g_array_index(devpose, stg_pose_t,
						 n).y);

	    angles[n] =
		atan2(srcpose->y - g_array_index(devpose, stg_pose_t, n).y,
		      srcpose->x - g_array_index(devpose, stg_pose_t,
						 n).x);

	    // TODO: free
	    audio_path_t *path =
		(audio_path_t *) malloc(sizeof(audio_path_t));
	    AUDIO_DEBUG_ALLOC("malloc:path:%p\n", path);

	    path->dist = dists[n];
	    path->angle = angles[n];
	    path->lines = NULL;
	    path->lines =
		g_list_append(path->lines, audio_stg_pose_dup(srcpose));
	    path->lines =
		g_list_append(path->lines,
			      audio_stg_pose_dup(&g_array_index
						 (devpose, stg_pose_t,
						  n)));

	    data->audio_paths = g_list_append(data->audio_paths, path);
#ifdef AUDIO_USE_CACHE
	    //TODO: make sure you free it!
	    g_hash_table_replace(audio_pointpair_cache, pp_key, path);
	    g_queue_push_tail(audio_pointpair_queue, pp_key);
#endif
	    continue;
	}

      /**
       * if not directly visible use dijkstra
       */
	// keep the node numbers
	dij_nodes[dij_n] = n;
	dij_n++;
//      dijkstra_set_graph_size(audio_points_n + 1 + dij_n);

	// We are going to run dijkstra. Find the visible important points from
	// the model (device) and add them as temporary edges to the graph
	for (int i = 0; i < audio_points_n; i++) {
	    stg_pose_t pose_i = g_array_index(audio_points, stg_pose_t, i);
	    if (audio_checkvisible
		(mod, g_array_index(devpose, stg_pose_t, n).x,
		 g_array_index(devpose, stg_pose_t, n).y,
		 pose_i.x, pose_i.y)) {

		double xdist =
		    pose_i.x - g_array_index(devpose, stg_pose_t, n).x;
		double ydist =
		    pose_i.y - g_array_index(devpose, stg_pose_t, n).y;

		// These are temporaray edges, they should be removed
		// for the next cycle
		dijkstra_insert_edge(i + 1, audio_points_n + dij_n,
				     hypot(xdist, ydist), TRUE);
	    }
	}
#ifdef AUDIO_USE_CACHE
	// TODO: check
	// If here, the key is not needed any more, free it
	free(pp_key);
	AUDIO_DEBUG_ALLOC("free:pp_key2:%p\n", pp_key);
#endif
    }

    // Do we need to run Dijkstra?
    if (dij_n > 0)		//TODO:
    {
	// Find all the visible important points from the src 
	// and add them as temporary edges to graph
	for (int i = 0; i < audio_points_n; i++) {
	    stg_pose_t pose_i = g_array_index(audio_points, stg_pose_t, i);
	    if (audio_checkvisible
		(mod, srcpose->x, srcpose->y, pose_i.x, pose_i.y)) {

		double xdist = pose_i.x - srcpose->x;
		double ydist = pose_i.y - srcpose->y;

		// These are temporaray edges, they should be removed
		// for the next cycle
		dijkstra_insert_edge(0, i + 1, hypot(xdist, ydist), TRUE);
	    }
	}

	// okay, run dijkstra now
	dijkstra_run();

	// now remove the temp edges and make it clean for the next cycle.
	// The remaining permanant edges are inter-important-points edges.
	dijkstra_remove_temp_edges();

	for (int i = 0; i < dij_n; i++) {
	    // TODO: dijkstra test
	    //double d = dijkstra_d[audio_points_n + i + 1];
	    int dn = audio_points_n + i + 1;
	    double *dp =
		g_hash_table_lookup(dijkstra_d, GINT_TO_POINTER(dn));
	    double d = INFINITY;
// TODO: why is this needed?
	    if (dp != NULL) {
		d = *dp;
	    }
	    // TODO: free
	    audio_path_t *path =
		(audio_path_t *) malloc(sizeof(audio_path_t));
	    AUDIO_DEBUG_ALLOC("malloc:path2:%p\n", path);
	    path->dist = INFINITY;
	    path->angle = 0;	// junk!
	    path->lines = NULL;

	    if (d < AUDIO_HEARING_RANGE) {
		// node number of receiving model/device in graph
		int u = audio_points_n + 1 + i;

		// last-node-before-device 
		int u2 = GPOINTER_TO_INT(g_hash_table_lookup
					 (dijkstra_previous,
					  GINT_TO_POINTER(u)));

		if (u2 > audio_points_n) {
		    printf("Audio: Error! impossible...\n");
		}
		// calc the last-node-before-device position to find the recv angle
		stg_pose_t p1 =
		    g_array_index(audio_points, stg_pose_t, u2 - 1);

		// calc the device node position to find the recv angle
		stg_pose_t p2 =
		    g_array_index(devpose, stg_pose_t, dij_nodes[i]);

		dists[dij_nodes[i]] = d;
		angles[dij_nodes[i]] = atan2(p1.y - p2.y, p1.x - p2.x);
		path->dist = d;
		path->angle = angles[dij_nodes[i]];

		while (u > 0) {
		    stg_pose_t p;

		    // TODO: check
		    // I guess dij_nodes[i] is not correct, but it will never
		    // happen
		    p = (u >
			 audio_points_n) ? g_array_index(devpose,
							 stg_pose_t,
							 dij_nodes[i]) :
			g_array_index(audio_points, stg_pose_t, u - 1);

		    path->lines =
			g_list_append(path->lines, audio_stg_pose_dup(&p));
//      TODO: dijkstra test     
//                  u = dijkstra_pi[u];
//                  u = 0;
		    u = GPOINTER_TO_INT(g_hash_table_lookup
					(dijkstra_previous,
					 GINT_TO_POINTER(u)));
		}

		path->lines =
		    g_list_append(path->lines,
				  audio_stg_pose_dup(srcpose));
	    }
	    data->audio_paths = g_list_append(data->audio_paths, path);

#ifdef AUDIO_USE_CACHE
	    //TODO: make sure you free it!
	    audio_pointpair_key_t *pp_key = (audio_pointpair_key_t *)
		malloc(sizeof(audio_pointpair_key_t));
	    AUDIO_DEBUG_ALLOC("malloc:pp_key2:%p\n", pp_key);
	    pp_key->p1 = *srcpose;
	    pp_key->p2 = g_array_index(devpose, stg_pose_t, dij_nodes[i]);

	    //printf("hashreplace: %p %p\n", pp_key, path);
	    g_hash_table_replace(audio_pointpair_cache, pp_key, path);
	    g_queue_push_tail(audio_pointpair_queue, pp_key);
//          g_hash_table_insert(audio_pointpair_cache, pp_key, path);
#endif
	}
    }


    free(dij_nodes);
}

void audio_deliver_msgs(gpointer key, gpointer data, gpointer user)
{

    stg_model_t *mod1 = (stg_model_t *) user;
    stg_model_t *mod2 = (stg_model_t *) data;


    if ((mod1->typerec != mod2->typerec)
	|| (mod1 == mod2)
	|| (stg_model_is_related(mod1, mod2)))
	return;

//  printf( "mod1: [%s:%s]", 
//       mod1->token , mod1->typerec->keyword );
//  printf( "\tmod2: [%s:%s]\n", 
//       mod2->token , mod2->typerec->keyword );

//  stg_rtk_fig_t* fig = stg_model_get_fig( mod1, "audio_data_fig" );

//  printf( "mod: [%s:%s]\n", 
//       mod->token , mod->typerec->keyword );

    stg_pose_t pose1;
    stg_model_get_global_pose(mod1, &pose1);
    stg_pose_t pose2;
    stg_model_get_global_pose(mod2, &pose2);

//  printf("(%f,%f)->(%f,%f)\n",pose1.x, pose1.y, pose2.x, pose2.y);

//    audiodev_models[audiodev_n] = mod2;
//    audiodev_pos[audiodev_n] = pose2;
    g_array_insert_val(audiodev_models, audiodev_n, mod2);
    g_array_insert_val(audiodev_pos, audiodev_n, pose2);
    audiodev_n++;
/*
    if (audiodev_n > AUDIO_MAX_DEVICES) {
	printf("Error: maximum audio devices can be %d\n",
	       AUDIO_MAX_DEVICES);
	exit(-1);
    }
*/
}

int audio_update(stg_model_t * mod)
{
    AUDIO_DEBUG2("Update\n");
    // no work to do if we're unsubscribed
    if (mod->subs < 1)
	return 0;

    stg_audio_data_t *data = (stg_audio_data_t *) mod->data;
    stg_audio_cmd_t *cmd = (stg_audio_cmd_t *) mod->cmd;
    stg_audio_config_t *cfg = (stg_audio_config_t *) mod->cfg;

    switch (cmd->cmd) {
    case STG_AUDIO_CMD_NOP:	//NOP
	if ((cfg->say_string[0] != 0)
	    && ((mod->world->sim_time - cfg->say_last_time) >
		cfg->say_period)) {
//        printf("pkhere1: %d\n",cfg->say_last_time-mod->world->sim_time);
	    strcpy(cmd->string, cfg->say_string);
	    cfg->say_last_time = mod->world->sim_time;
	    // no break; continue to the next case
	} else {
	    data->string[0] = 0;
	    cfg->string[0] = 0;
	    break;
	}

    case STG_AUDIO_CMD_SAY:
// TODO: !?
	AUDIO_DEBUG2("Cmd [%s]: %s\n", mod->token, cmd->string);
	strcpy(cfg->string, cmd->string);
	strcpy(data->string, cmd->string);
	cmd->cmd = STG_AUDIO_CMD_NOP;	// clear the cmd
	model_change(mod, &mod->cmd);
	break;

    default:
	printf("unknown audio command %d\n", cmd->cmd);
    }

// Free old data
#ifdef AUDIO_USE_CACHE
//    g_hash_table_foreach_remove(audio_pointpair_cache, audio_free_cache, mod);

/*
    g_hash_table_destroy(audio_pointpair_cache);
    audio_pointpair_cache =
	    g_hash_table_new_full(audio_pointpair_hash,
				  audio_pointpair_equal, audio_pointpair_key_destroy, audio_path_destroy);
*/
/*
    printf("Queue:%d  Cache:%d\n",
	   g_queue_get_length(audio_pointpair_queue)
	   , g_hash_table_size(audio_pointpair_cache));
    fflush(stdout);
*/
//    audio_pointpair_list=g_slist_delete_link(audio_pointpair_list,audio_pointpair_list);

#ifdef AUDIO_MAX_CACHE_SIZE
    while (g_queue_get_length(audio_pointpair_queue) >
	   AUDIO_MAX_CACHE_SIZE) {
	audio_pointpair_key_t *ppkey = (audio_pointpair_key_t *)
	    g_queue_pop_head(audio_pointpair_queue);
	if (ppkey) {
	    g_hash_table_remove(audio_pointpair_cache, ppkey);
//      free(ppkey);
	}
    }
    AUDIO_DEBUG2("Cache size: %d\n",
		 g_queue_get_length(audio_pointpair_queue));
#endif

#else
//  if cache is being used the paths should not be destroyed each time
//  so that they later can be retrived from the cached
    // TODO: check
//    printf("NEWCYCLE UPDATE\n");
//    printf("ll1: %d\n",g_list_length(data->audio_paths));
    g_list_foreach(data->audio_paths, audio_free_path, mod);
#endif
    g_list_free(data->audio_paths);
    // TODO: check
//    printf("ll2: %d\n",g_list_length(data->audio_paths));
    data->audio_paths = NULL;

    // TODO: no array
    audiodev_n = 0;

    g_hash_table_foreach(mod->world->models, audio_deliver_msgs, mod);

    stg_pose_t pose;
    stg_model_get_global_pose(mod, &pose);

//    audio_find_dist_multi(mod, pose.x, pose.y, audiodev_n, (double *)&audiodev_posx, (double *)&audiodev_posy, (double *)&dists);
    double *dists = malloc(audiodev_n * sizeof(double));
    double *angles = malloc(audiodev_n * sizeof(double));

    audio_find_dist_multi(mod, &pose, audiodev_n,
			  audiodev_pos, dists, angles);
//                       (stg_pose_t *) & audiodev_pos, (double *) &dists);
//    printf("audiodev_n: %d\n",audiodev_n);


    for (int i = 0; i < audiodev_n; i++) {
//      printf("[%d]=%f ",i,dists[i]);
	if ((dists[i] < AUDIO_HEARING_RANGE) && (data->string[0] != 0)) {
	    stg_model_t *trg_mod = g_array_index(audiodev_models,
						 stg_model_t *, i);

	    stg_pose_t trg_pos = g_array_index(audiodev_pos,
					       stg_pose_t, i);

	    stg_audio_data_t *trg_data =
		(stg_audio_data_t *) trg_mod->data;

	    double recvangle = NORMALIZE(angles[i] - trg_pos.a);

	    AUDIO_DEBUG2("%15s->%15s, %-5.1f, %-5.1f, %s\n", mod->token,
			 trg_mod->token, dists[i], RTOD(recvangle),
			 data->string);

	    sprintf(trg_data->recv, "%s,%5.2f,%5.2f,%s", mod->token,
		    dists[i], recvangle, data->string);
	    //TODO: call model_change?!
	}
    }
    AUDIO_DEBUG2("\n");

    free(dists);
    free(angles);
//    printf("\n");



//  printf("GList update: %d\n",g_list_length(data->audio_paths));

    // publish the new stuff
    //stg_model_set_data( mod, &data, sizeof(data));
    // stg_model_set_cfg( mod,  &cfg, sizeof(cfg));
    model_change(mod, &mod->data);
    model_change(mod, &mod->cfg);

    // inherit standard update behaviour
    _model_update(mod);

#ifdef AUDIO_USE_CACHE
#ifdef AUDIO_CACHE_STATS
    printf("Cache stats: %ld hits of %ld total= %ld%% \n",
	   audio_cache_n_hits,
	   audio_cache_n, audio_cache_n_hits * 100 / audio_cache_n);
#endif
#endif
    return 0;			//ok
}

int audio_render_data(stg_model_t * mod, void *userp)
{
//  puts( "audio render data" );
    // only draw if someone is using the audio
    if (mod->subs < 1)
	return 0;
    {
	stg_rtk_fig_t *fig =
	    stg_rtk_fig_create(mod->world->win->canvas, NULL,
			       STG_LAYER_AUDIODATA);
	stg_rtk_fig_destroy(fig);

    }

#ifdef AUDIO_FIG_DESTROY_LATER
    //stg_model_fig_create(mod, "audio_data_fig", "top",
    //                       STG_LAYER_AUDIODATA);
    stg_rtk_fig_t *fig = stg_rtk_fig_create(mod->world->win->canvas, NULL,
					    STG_LAYER_AUDIODATA);

//          stg_model_fig_create(mod, "audio_data_fig", "top",
//                               STG_LAYER_AUDIODATA);
    stg_rtk_fig_color_rgb32(fig, stg_lookup_color(STG_AUDIO_COLOR));
    stg_rtk_fig_linewidth(fig, 1);

#else

    stg_rtk_fig_t *fig = stg_model_get_fig(mod, "audio_data_fig");


    if (!fig) {
	fig =
	    stg_model_fig_create(mod, "audio_data_fig", "top",
				 STG_LAYER_AUDIODATA);
//          stg_model_fig_create(mod, "audio_data_fig", "top",
//                               STG_LAYER_AUDIODATA);
	stg_rtk_fig_color_rgb32(fig, stg_lookup_color(STG_AUDIO_COLOR));
	stg_rtk_fig_linewidth(fig, 1);
    } else
	stg_rtk_fig_clear(fig);
#endif

    stg_audio_data_t *data = (stg_audio_data_t *) mod->data;
    assert(data);

    stg_audio_config_t *cfg = (stg_audio_config_t *) mod->cfg;
    assert(cfg);

//  stg_geom_t *geom = &mod->geom;  

    stg_pose_t pose;
    stg_model_get_global_pose(mod, &pose);

    //    stg_rtk_fig_origin( fig, pose.x, pose.y, pose.a );

    // only draw if the string is not empty
    if (data->string[0] != 0) {
	stg_rtk_fig_color_rgb32(fig,
				stg_lookup_color(STG_AUDIO_BUBBLE_COLOR));
#ifdef AUDIO_FIG_DESTROY_LATER
	stg_rtk_fig_text_bubble(fig, pose.x, pose.y, 0, data->string, 0.1,
				0.1);
#else
	stg_rtk_fig_text_bubble(fig, 0, 0, 0, data->string, 0.1, 0.1);
#endif
	stg_rtk_fig_color_rgb32(fig, stg_lookup_color(STG_AUDIO_COLOR));
	stg_rtk_fig_linewidth(fig, 3);
    } else {
	stg_rtk_fig_linewidth(fig, 1);
    }

//  printf("GList render: %d\n",g_list_length(data->audio_paths));
//pooya
#ifdef AUDIO_FIG_DESTROY_LATER
    g_list_foreach(data->audio_paths, audio_draw_path, fig);
#else
    g_list_foreach(data->audio_paths, audio_draw_path, mod);
#endif

#ifdef AUDIO_FIG_DESTROY_LATER
    stg_rtk_fig_destroy_later(fig, 10);
//    stg_rtk_fig_destroy(fig);
#endif

/*
  char tmpstr[STG_AUDIO_MAX_STRING_LEN*3];
  sprintf(tmpstr,"Said: %s\nHeard: %s",data->string,data->recv);
  stg_rtk_fig_text_bubble( fig, 0.0, 0.0, 0, tmpstr, 0.15, 0.15);
*/

    return 0;
}

int audio_render_cfg(stg_model_t * mod, void *userp)
{
//  puts( "audio render cfg" );
    // only draw if someone is using the audio
    if (mod->subs < 1)
	return 0;

    stg_rtk_fig_t *fig = stg_model_get_fig(mod, "audio_cfg_fig");

    if (!fig) {
	fig =
	    stg_model_fig_create(mod, "audio_cfg_fig", "top",
				 STG_LAYER_AUDIODATA);
	stg_rtk_fig_color_rgb32(fig, stg_lookup_color(STG_AUDIO_COLOR));
	stg_rtk_fig_linewidth(fig, 2);

    }
    // TODO: only draw once and redraw when moved
    // maybe by using a different layer
    stg_rtk_fig_clear(fig);

    for (int i = 0; i < audio_points_n; i++) {
	stg_pose_t pose1 = g_array_index(audio_points, stg_pose_t, i);

	stg_model_global_to_local(mod, &pose1);
	stg_rtk_fig_ellipse(fig, pose1.x, pose1.y, 0.0, 0.02, 0.02, 1);
    }

    return 0;
}


int audio_startup(stg_model_t * mod)
{
    AUDIO_DEBUG2("Startup\n");
/*
  stg_model_set_watts( mod, STG_WIFI_WATTS );
*/
    if (audio_points_n == 0) {
	// TODO: double check
//    g_hash_table_foreach( mod->world->models, audio_findpoints, mod);
	// initialize minimum resolution used for comparision to infinite
	// this valued will lated be set to the minimum resolution of maps
	audio_min_resolution = INFINITY;

	// an array to keep the important points
	//   zero terminated: flase
	//   auto clear: true
	//   element size: sizeof(stg_pose_t)
	// TODO: destroy at shutdown
	audio_points = g_array_new(FALSE, TRUE, sizeof(stg_pose_t));

	// TODO: description
	audiodev_models = g_array_new(FALSE, TRUE, sizeof(stg_model_t *));
	audiodev_pos = g_array_new(FALSE, TRUE, sizeof(stg_pose_t));

	g_hash_table_foreach(mod->world->models, audio_findpoints, mod);
	AUDIO_DEBUG1("Audio: Total of %d points found\n", audio_points_n);
	audio_create_visibilitygraph(mod);

#ifdef AUDIO_USE_CACHE
	// TODO: destroy it somewhere
	audio_pointpair_cache =
	    g_hash_table_new_full(audio_pointpair_hash,
				  audio_pointpair_equal,
				  audio_pointpair_key_destroy,
				  audio_path_destroy);
	audio_pointpair_queue = g_queue_new();
#endif

    }
    return 0;			// ok
}

int audio_shutdown(stg_model_t * mod)
{
    AUDIO_DEBUG2("Shutdown\n");
    dijkstra_destroy();
/*
  stg_model_set_watts( mod, 0 );
*/

    // unrender the data
//    stg_model_fig_clear(mod, "audio_data_fig");
    return 0;			// ok
}

int audio_unrender_data(stg_model_t * mod, void *userp)
{
    // CLEAR STUFF THAT YOU DREW
//    stg_model_fig_clear(mod, "audio_data_fig");
    return 1;			// callback just runs one time
}

int audio_unrender_cfg(stg_model_t * mod, void *userp)
{
    // CLEAR
    stg_model_fig_clear(mod, "audio_cfg_fig");
    return 1;			// callback just runs one time
}
