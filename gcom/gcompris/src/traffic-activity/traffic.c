/* gcompris - traffic.c
 *
 * Copyright (C) 2002, 2008 Bruno Coudoin
 *
 * Based on the original code from Geoff Reedy <vader21@imsa.edu>
 * Copyright (C) 2000 Geoff Reedy
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "traffic.h"

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

#define DATAFILE "TrafficData"


static GcomprisBoard  *gcomprisBoard = NULL;
static gboolean	board_paused  = TRUE;

static GooCanvasItem *allcars         = NULL;
static gboolean dragging = FALSE;
static double start_x, start_y;
static double hit=0;

#define OFSET_X 250
#define OFSET_Y 128

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);
static void	 repeat(void);

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem	*traffic_create_item(GooCanvasItem *parent);
static void		 traffic_destroy_all_items(void);
static void		 traffic_next_level(void);

typedef struct _car car;
typedef struct _jam jam;

struct _car {
  guint x : 3;
  guint y : 3;
#define CAR_ORIENT_NS	0
#define CAR_ORIENT_EW	1
  guint orient : 1;
  guint goal : 1;
  guint size;
  guint color;
  gchar color_string[50];
  GooCanvasItem *canvasgroup;
};

struct _jam {
  guint num_cars;
  guint card;
  guint level;
#define MAX_NUMBER_OF_CARS 20
  car *cars[MAX_NUMBER_OF_CARS];
};

static gboolean on_button_press (GooCanvasItem  *item,
				 GooCanvasItem  *target,
				 GdkEventButton *event,
				 car *thiscar);
static gboolean on_button_release (GooCanvasItem *item,
				   GooCanvasItem *target,
				   GdkEventButton *event,
				   car *thiscar);
static gboolean on_motion_notify (GooCanvasItem *item,
				  GooCanvasItem *target,
				  GdkEventMotion *event,
				  car *thiscar);

static gboolean  load_level(guint level, guint card);

static jam	 current_card  ={0,0,0,{NULL}};

static void	 draw_grid  (GooCanvasItem *rootBorder);
static gint	 cars_from_strv(char *strv);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "A sliding block puzzle game",
    "",
    "Bruno Coudoin <bruno.coudoin@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    pause_board,
    end_board,
    is_our_board,
    NULL,
    NULL,
    set_level,
    NULL,
    repeat,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(traffic)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    {
      game_won();
    }

  board_paused = pause;
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=13;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=5; /* Go to next level after this number of 'play' */
      gc_score_start(SCORESTYLE_NOTE,
			   BOARDWIDTH - 220,
			   BOARDHEIGHT - 70,
			   gcomprisBoard->number_of_sublevel);

      gc_bar_set(GC_BAR_LEVEL|GC_BAR_REPEAT);

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"traffic/traffic-bg.jpg");

      traffic_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}
/* ======================================= */
static void end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      traffic_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

/* ======================================= */
static void set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      traffic_next_level();
    }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_ascii_strcasecmp(gcomprisBoard->type, "traffic")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/*
 * Repeat let the user restart the current level
 *
 */
static void repeat (){

  traffic_destroy_all_items();

  /* Try the next level */
  traffic_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void
traffic_next_level()
{

  gc_bar_set_level(gcomprisBoard);

  traffic_destroy_all_items();
  gamewon = FALSE;

  gc_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  traffic_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));

}
/* ==================================== */
/* Destroy all the items */
static void
traffic_destroy_all_items()
{
  guint i;

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);
  boardRootItem = NULL;

  for (i=0; i<current_card.num_cars; i++)
    {
      g_free(current_card.cars[i]);
    }
  current_card.num_cars = 0;
}

/* ==================================== */
static GooCanvasItem *
traffic_create_item(GooCanvasItem *parent)
{
  GooCanvasItem *borderItem = NULL;

  boardRootItem = \
    goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
			  NULL);

  borderItem = \
    goo_canvas_group_new (boardRootItem,
			  NULL);
  goo_canvas_item_translate(borderItem, OFSET_X, OFSET_Y);

  draw_grid(borderItem);

  allcars = goo_canvas_group_new (borderItem, NULL);
  goo_canvas_item_translate(allcars, 11, 11);

  g_object_set_data(G_OBJECT(allcars),
		      "whatami", (gpointer)"allcars");

  /* Ready now, let's go */
  load_level(gcomprisBoard->level, gcomprisBoard->sublevel);

  return NULL;
}
/* ==================================== */
static void
game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel)
    {
      /* Try the next level */
      gcomprisBoard->sublevel=1;
      gcomprisBoard->level++;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel)
	gcomprisBoard->level = gcomprisBoard->maxlevel;

      gc_sound_play_ogg ("sounds/bonus.wav", NULL);
    }
  traffic_next_level();
}

/* from canvas.c */

static void
draw_grid(GooCanvasItem *rootBorder)
{
  GooCanvasItem *grid_group;
  int xlooper, ylooper;

  grid_group = \
    goo_canvas_group_new (rootBorder,
			  NULL);

  goo_canvas_item_translate(rootBorder, 10, 10);

  g_object_set_data(G_OBJECT(grid_group),
		      "whatami", (gpointer)"grid_group");

  goo_canvas_item_lower(grid_group, NULL);

  for (xlooper=0; xlooper<6; xlooper++)
    for (ylooper=0; ylooper<6; ylooper++)
      g_object_set_data(G_OBJECT(
				 goo_canvas_rect_new(grid_group,
						     40.0*xlooper,
						     40.0*ylooper,
						     40.0,
						     40.0,
						     "fill-color-rgba", NULL,
						     "stroke-color", "white",
						     "line-width", 2.0,
						     NULL)),
			"whatami",(gpointer)"grid square");;
}


static void
draw_car(car *thiscar)
{
  GooCanvasItem *car_group;
  GooCanvasItem *car_rect;

  g_object_set_data(G_OBJECT(allcars),"whatami",(gpointer)"allcars");

  car_group = goo_canvas_group_new(allcars,
				   NULL);
  goo_canvas_item_translate(car_group,
			    40.0 * thiscar->x - 10,
			    40.0 * thiscar->y - 10);

  car_rect = goo_canvas_rect_new(car_group,
				 0.0,
				 0.0,
				 (thiscar->orient?40.0*thiscar->size:40.0)-2,
				 (thiscar->orient?40.0:40.0*thiscar->size)-2,
				 "fill_color_rgba", thiscar->color,
				 "stroke-color", "white",
				 "line-width", 1.0,
				 NULL);

  g_signal_connect(car_group,"button_press_event",
		   GTK_SIGNAL_FUNC(on_button_press), (gpointer)thiscar);
  g_signal_connect (car_group, "button_release_event",
		    (GCallback) on_button_release, (gpointer)thiscar);
  g_signal_connect (car_group, "motion_notify_event",
		    (GCallback) on_motion_notify, (gpointer)thiscar);

  g_object_set_data(G_OBJECT(car_group), "car", (gpointer)thiscar);
  g_object_set_data(G_OBJECT(car_group), "whatami", (gpointer)"car_group");
  g_object_set_data(G_OBJECT(car_rect), "whatami", (gpointer)"car_rect");
}

static void
draw_jam(jam *myjam)
{
  int whichcar;
  for (whichcar=0;whichcar<myjam->num_cars;whichcar++)
    draw_car(myjam->cars[whichcar]);
}

static gboolean
on_motion_notify (GooCanvasItem *item,
		  GooCanvasItem *target,
		  GdkEventMotion *event,
		  car *thiscar)
{
  GooCanvas *canvas;
  double small_x, big_x, small_y, big_y;
  double dx,dy;
  double item_x, item_y;
  GooCanvasItem *atdest = NULL;
  car *othercar = NULL;

  canvas = goo_canvas_item_get_canvas (item);

  item_x = event->x;
  item_y = event->y;
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(item),
				   goo_canvas_item_get_parent(item),
				   &item_x, &item_y);

  if (dragging && (event->state & GDK_BUTTON1_MASK))
    {
      switch (thiscar->orient) {
      case CAR_ORIENT_EW:
	small_x=0;
	small_y=0;

	big_x=40*thiscar->size-1;
	big_y=40-1;

	goo_canvas_convert_from_item_space(canvas,
					   item, &small_x, &small_y);
	goo_canvas_convert_from_item_space(canvas,
					   item, &big_x, &big_y);

	dy = CLAMP(item_y - start_y, -39, 39);
	dx = CLAMP(item_x - start_x, -39, 39);

	if (thiscar->goal && big_x==250+OFSET_X)
	  {
	    gc_canvas_item_ungrab(item,event->time);
	    g_object_set (item,
			  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
			  NULL);
	    dragging=FALSE;

	    gamewon = TRUE;
	    gc_bonus_display(gamewon, GC_BONUS_SMILEY);
	  }

	if (small_x+dx < 11+OFSET_X)
	  {
	    dx = 11-small_x+OFSET_X;
	  }
	else if (big_x+dx > 250+OFSET_X)
	  {
	    dx = 250-big_x+OFSET_X;
	  }

	if ((hit<0) != (dx<0)) { hit=0;}

	if (hit==0)
	  {
	    if (dx>0)
	      {
		do
		  {
		    atdest = goo_canvas_get_item_at(canvas,
						    big_x+dx, small_y+20, TRUE);
		    if (atdest)
		      othercar = (car*)g_object_get_data(G_OBJECT(goo_canvas_item_get_parent(atdest)),
							 "car");
		    if (othercar)
		      {
			hit=1;
			dx=0;
		      }
		  } while (othercar);
	      }
	    else if (dx<0)
	      {
		do {
		  atdest=goo_canvas_get_item_at(canvas,
						small_x+dx-1,small_y+20, TRUE);
		  if (atdest)
		    othercar=(car*)g_object_get_data(G_OBJECT(goo_canvas_item_get_parent(atdest)),
						     "car");
		  if (othercar) {
		    hit=-1;
		    dx=0;
		  }
		} while (othercar);
	      }
	  }
	else
	  { dx=0; }

	goo_canvas_item_translate(item, dx, 0);
	break;

      case CAR_ORIENT_NS:
	small_x=0;
	small_y=0;

	big_x=40-1;
	big_y=40*thiscar->size-1;

	goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
					   item,
					   &small_x, &small_y);
	goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
					   item,
					   &big_x, &big_y);

	dy = CLAMP(item_y - start_y, -39, 39);
	dx = CLAMP(item_x - start_x, -39, 39);

	if (small_y+dy<11+OFSET_Y)
	  {
	    dy=11-small_y+OFSET_Y;
	  }
	else if (big_y+dy>250+OFSET_Y)
	  {
	    dy=250-big_y+OFSET_Y;
	  }

	if ((hit<0)!=(dy<0)) { hit=0; }

	if (hit==0) {
	  if (dy>0) {
	    do {
	      atdest = goo_canvas_get_item_at(gcomprisBoard->canvas,
					      small_x + 20,
					      big_y + dy,
					      TRUE);
	      if (atdest)
		othercar=(car*)g_object_get_data(G_OBJECT(goo_canvas_item_get_parent(atdest)),
						 "car");
	      if (othercar) {
		hit=1;
		dy=0;
	      }
	    } while (othercar);
	  } else if (dy<0) {
	    do {
	      atdest=goo_canvas_get_item_at(gcomprisBoard->canvas,
					    small_x+20,small_y+dy-1, TRUE);
	      if (atdest)
		othercar=(car*)g_object_get_data(G_OBJECT(goo_canvas_item_get_parent(atdest)),
						   "car");
	      if (othercar) {
		hit=-1;
		dy=0;
	      }
	    } while (othercar);
	  }
	} else { dy=0; }

	goo_canvas_item_translate(item, 0, dy);
      }

    }
  return TRUE;
}

static gboolean
on_button_release (GooCanvasItem *item,
		   GooCanvasItem *target,
		   GdkEventButton *event,
		   car *thiscar)
{
  GooCanvas *canvas;
  double dx,dy;

  canvas = goo_canvas_item_get_canvas (item);

#if 0
  g_print ("received 'button-release' signal\n");
#endif

  if (dragging)
    {
      double even_vals_x[]={11+OFSET_X,51+OFSET_X,91+OFSET_X,131+
			    OFSET_X,171+OFSET_X,211+OFSET_X,HUGE_VAL};
      double even_vals_y[]={11+OFSET_Y,51+OFSET_Y,91+OFSET_Y,131+
			    OFSET_Y,171+OFSET_Y,211+OFSET_Y,HUGE_VAL};
      double *ptr;
      double x=0,y=0;

      goo_canvas_convert_from_item_space(canvas, item,
					 &x, &y);

      for (ptr=even_vals_x;*ptr<x;ptr++);
      if (*ptr-x>20)
	dx=*(ptr-1)-x;
      else
	dx=*ptr-x;

      for (ptr=even_vals_y;*ptr<y;ptr++);
      if (*ptr-y>20)
	dy=*(ptr-1)-y;
      else
	dy=*ptr-y;

      goo_canvas_item_translate(item, dx, dy);
      gc_canvas_item_ungrab(item, event->time);
      hit=0;
      dragging=FALSE;
    }

  return TRUE;
}

 static gboolean
 on_button_press (GooCanvasItem  *item,
		  GooCanvasItem  *target,
		  GdkEventButton *event,
		  car *thiscar)
{
  double item_x, item_y;
  GdkCursor *cursor;

  item_x = event->x;
  item_y = event->y;
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(item),
				   goo_canvas_item_get_parent(item),
				   &item_x, &item_y);

  start_x = item_x;
  start_y = item_y;

  if (thiscar->orient==CAR_ORIENT_NS)
    cursor = gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
  else
    cursor = gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW);

  gc_canvas_item_grab(item,
		      GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
		      cursor,
		      event->time);

  gdk_cursor_unref(cursor);
  dragging=TRUE;

  return TRUE;
}

/* From jam.c */

static gboolean
load_level(guint level, guint sublevel)
{
  char *car_strv=NULL;

  current_card.level = level;
  current_card.card  = sublevel;

  car_strv = DataList[(level-1) * gcomprisBoard->number_of_sublevel + (sublevel-1)];

  current_card.num_cars = cars_from_strv(car_strv);

  if(current_card.num_cars == -1)
    g_error("In loading dataset for traffic activity");

  draw_jam(&current_card);

  return TRUE;
}

/* Returns the number of cars
 * I took the formatting from
 *  http://www.javascript-games.org/puzzle/rushhour/
 */
static gint
cars_from_strv(char *strv)
{
  car *ccar;
  char x,y,id;
  int number_of_cars = 0;
  gboolean more_car = TRUE;

  while (more_car) {

    current_card.cars[number_of_cars] = (car *)g_new(car, 1);
    ccar = current_card.cars[number_of_cars];

    /* By default, not a goal car */
    ccar->goal   = 0;

    number_of_cars++;

    if (sscanf(strv,"%c%c%c",
	       &id,&x,&y)!=3) {
      return -1;
    }

    /* Point to the next car */
    strv += 3;

    if(strv[0] != ',')
      more_car = FALSE;

    strv += 1;

    if (id == 'O' || id == 'P' || id == 'Q' || id == 'R') ccar->size = 3;
    else ccar->size = 2;

    ccar->orient = 1;
    ccar->x = 0;
    ccar->y = y-'1';

    if (x == 'A') ccar->x = 0;
    else if (x == 'B') ccar->x = 1;
    else if (x == 'C') ccar->x = 2;
    else if (x == 'D') ccar->x = 3;
    else if (x == 'E') ccar->x = 4;
    else if (x == 'F') ccar->x = 5;
    else {
      ccar->y = x-'1';
      ccar->orient = 0;

      if (y == 'A') ccar->x = 0;
      else if (y == 'B') ccar->x = 1;
      else if (y == 'C') ccar->x = 2;
      else if (y == 'D') ccar->x = 3;
      else if (y == 'E') ccar->x = 4;
      else if (y == 'F') ccar->x = 5;
    }

    if (id == 'X')
      {
	ccar->color  = 0xFF0000FF;
	ccar->goal   = 1;
      }
    else if (id == 'A') ccar->color = 0x80FF80FF;
    else if (id == 'B') ccar->color = 0xC0C000FF;
    else if (id == 'C') ccar->color = 0x8080FFFF;
    else if (id == 'D') ccar->color = 0xFF80FFFF;
    else if (id == 'E') ccar->color = 0xC00000FF;
    else if (id == 'F') ccar->color = 0x008000FF;
    else if (id == 'G') ccar->color = 0xC0C0C0FF;
    else if (id == 'H') ccar->color = 0x6000efFF;
    else if (id == 'I') ccar->color = 0xFFFF00FF;
    else if (id == 'J') ccar->color = 0xFFA801FF;
    else if (id == 'K') ccar->color = 0x00FF00FF;
    else if (id == 'O') ccar->color = 0xFFFF00FF;
    else if (id == 'P') ccar->color = 0xFF80FFFF;
    else if (id == 'Q') ccar->color = 0x0000FFFF;
    else if (id == 'R') ccar->color = 0x00FFFFFF;

  }
  return number_of_cars;
}

