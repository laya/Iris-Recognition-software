/* gcompris - billard.c
 *
 * Copyright (C) 2001, 2008 Bruno Coudoin
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

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

typedef enum {
  MACH_HORZ_WALL = 0,
  MACH_VERT_WALL,
  MACH_HOLE,
  MACH_DIAG_WALL,
  MACH_BASKET_BALL,
  MACH_FLYING_BALL,
} MachItemType;

struct _MachItem {
  MachItemType	   type;
  gboolean	   moving;
  GooCanvasItem *item;		/* The canvas item      */
  double	   times;
  double	   ax, ay;
  double	   xposo, xpos, vxo;
  double	   yposo, ypos, vyo;
  double	   elasticity;
  double	   width, height;
};

typedef struct _MachItem MachItem;

static GList *item_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;
static gint move_id = 0;
static double		times_inc  = 0.1;
static double		gravity = 0;
static double		velocity = 0.99;
static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem	*minigolf_create_item(GooCanvasItem *parent);
static void		 minigolf_destroy_all_items(void);
static void		 minigolf_next_level(void);
static gboolean		 item_event (GooCanvasItem  *item,
				     GooCanvasItem  *target,
				     GdkEventButton *event,
				     MachItem *machItem);
static gboolean		 minigolf_move(GList *item_list);

static MachItem		*create_machine_item(MachItemType machItemType, double x, double y);

/* The border in the image background */
#define BORDER 40

/* The inner board limit */
#define MIN_X1	60
#define MIN_X2	(BOARDWIDTH-MIN_X1)
#define MIN_Y1	65
#define MIN_Y2	(BOARDHEIGHT-30)

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Move the mouse"),
    "football",
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
    NULL,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(machpuzzle)

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
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gc_bar_set(GC_BAR_LEVEL);

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"billard/foot_background.svgz");

      minigolf_next_level();

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
      minigolf_destroy_all_items();
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
      minigolf_next_level();
    }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_ascii_strcasecmp(gcomprisBoard->type, "billard")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void minigolf_next_level()
{

  gc_bar_set_level(gcomprisBoard);

  minigolf_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  minigolf_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));

  move_id = g_timeout_add (40, (GSourceFunc)minigolf_move, item_list);

}
/* ==================================== */
/* Destroy all the items */
static void minigolf_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  if (move_id) {
    g_source_remove (move_id);
    move_id = 0;
  }

  boardRootItem = NULL;

  if(item_list)
  {
      int i;
      for( i=0; i< g_list_length(item_list); i++)
          g_free(g_list_nth_data(item_list,i));
      g_list_free(item_list);
  }
  item_list = NULL;
}
/* ==================================== */
static GooCanvasItem *minigolf_create_item(GooCanvasItem *parent)
{

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  create_machine_item(MACH_HOLE, 750.0, 260.0);
  create_machine_item(MACH_BASKET_BALL,
		      (double)g_random_int_range(60, 150),
		      (double)RAND(70, 400));


  return NULL;
}
/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel)
      gcomprisBoard->level = gcomprisBoard->maxlevel;

    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  minigolf_next_level();
}

/* ==================================== */
static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    MachItem *machItem)
{
  double item_x, item_y;
  GooCanvasBounds bounds;
  double width;
  item_x = event->x;
  item_y = event->y;

  goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
				     item, &item_x, &item_y);

  goo_canvas_item_get_bounds (item, &bounds);

  if(board_paused)
    return FALSE;

  gc_sound_play_ogg ("sounds/scroll.wav", NULL);
  width = bounds.x2 - bounds.x1;

  machItem->times = 0;
  machItem->yposo = machItem->ypos;
  machItem->xposo = machItem->xpos;

  machItem->vyo = ((item_y-bounds.y1)<width/2?(width/2-(item_y-bounds.y1))*20:-1*(width/2-(bounds.y2-item_y))*20);
  machItem->vxo = ((item_x-bounds.x1)<width/2?(width/2-(item_x-bounds.x1))*20:-1*(width/2-(bounds.x2-item_x))*20);

  return FALSE;
}

#if 0
static void dump_machItem(MachItem *machItem)
{

  printf("Type		= %d\n", machItem->type);
  printf("        times = %f\n", machItem->times);
  printf("           ax = %f\n", machItem->ax);
  printf("           ay = %f\n", machItem->ay);
  printf("        xposo = %f\n", machItem->xposo);
  printf("         xpos = %f\n", machItem->xpos);
  printf("          vxo = %f\n", machItem->vxo);
  printf("        yposo = %f\n", machItem->yposo);
  printf("         ypos = %f\n", machItem->ypos);
  printf("          vyo = %f\n", machItem->vyo);

}
#endif

/* Create a machine item */
static MachItem *create_machine_item(MachItemType machItemType, double x, double y)
{
  MachItem		*machItem;
  guint			 width;
  guint			 height;

  machItem = g_new (MachItem, 1);

  machItem->type	= machItemType;

  switch (machItemType)
    {
    case MACH_HORZ_WALL:
      width = 100;
      height = 20;
      machItem->moving	= FALSE;
      machItem->times   = 0.0;

      machItem->xposo	= x;
      machItem->xpos	= x;
      machItem->vxo	= 0;
      machItem->ax	= 0;

      machItem->yposo	= y;
      machItem->ypos	= y;
      machItem->vyo	= 0;
      machItem->ay	= 0;

      machItem->elasticity  = 5;

      machItem->item = goo_canvas_rect_new (boardRootItem,
					      machItem->xposo,
					      machItem->yposo,
					      width,
					      height,
					      "stroke-color", "black",
					      "fill_color_rgba", 0xFF10C0FF,
					      "line-width", (double)1,
					      NULL);

      g_signal_connect(machItem->item,
		       "button_press_event",
		       (GCallback) item_event,
		       machItem);
      break;
    case MACH_VERT_WALL:
      break;
    case MACH_DIAG_WALL:
      break;
    case MACH_HOLE:
      /* Make the hole be smaller based on the level */
      width = 110 - gcomprisBoard->level*3;
      machItem->moving	= FALSE;
      machItem->times   = 0.0;

      machItem->xposo	= x;
      machItem->xpos	= x;
      machItem->vxo	= 0;
      machItem->ax	= 0;

      machItem->yposo	= y;
      machItem->ypos	= y;
      machItem->vyo	= 0;
      machItem->ay	= 0;

      machItem->width	= width;
      machItem->height	= width;

      machItem->elasticity  = 3;

      machItem->item = goo_canvas_ellipse_new (boardRootItem,
					      machItem->xposo,
					      machItem->yposo,
					      width/2,
					      width/2,
					      "stroke-color-rgba", 0xEEEEEEFF,
					      "fill-color-rgba", 0x111111FF,
					      "line-width", (double)2,
					      NULL);

      break;
    case MACH_BASKET_BALL:
      width = 40;
      machItem->moving	= TRUE;
      machItem->times   = 0.0;

      machItem->xposo	= x;
      machItem->xpos	= x;
      machItem->vxo	= 0;
      machItem->ax	= 0;

      machItem->yposo	= y;
      machItem->ypos	= y;
      machItem->vyo	= 0;
      machItem->ay	= gravity;

      machItem->width	= width;
      machItem->height	= width;

      machItem->elasticity  = 4;

      machItem->item = goo_canvas_ellipse_new (boardRootItem,
					      machItem->xposo,
					      machItem->yposo,
					      width/2,
					      width/2,
					      "stroke-color", "black",
					      "fill-color", "white",
					      "line-width", (double)1,
					      NULL);

      g_signal_connect(machItem->item, "button_press_event",
		       (GCallback) item_event,
		       machItem);

      break;
    case MACH_FLYING_BALL:
      width = 40;
      machItem->moving	= TRUE;
      machItem->times   = 0.0;

      machItem->xposo	= x;
      machItem->xpos	= x;
      machItem->vxo	= 10;
      machItem->ax	= 0;

      machItem->yposo	= y;
      machItem->ypos	= y;
      machItem->vyo	= -5;
      machItem->ay	= -0.5;

      machItem->width	= width;
      machItem->height	= width;

      machItem->elasticity  = 1;

      machItem->item = goo_canvas_ellipse_new (boardRootItem,
					      machItem->xposo,
					      machItem->yposo,
					      width/2,
					      width/2 * 1.5,
					      "stroke-color", "black",
					      "fill_color_rgba", 0xE03000FF,
					      "line-width", (double)1,
					      NULL);

      g_signal_connect(machItem->item, "enter_notify_event",
		       (GCallback) item_event,
		       machItem);
      break;
    }

  g_object_set_data(G_OBJECT(machItem->item), "mach", (gpointer)machItem);

  item_list = g_list_append (item_list, machItem);

  return machItem;
}

/*
 * Returns true if at least 3 corners of 's' rectangle are inside 'd' rectangle
 *
 */
static gint rectangle_in(double sx1, double sy1, double sx2, double sy2,
			 double dx1, double dy1, double dx2, double dy2)
{
  guint corner_in = 0;
  //  printf("rectangle_in %10f %10f %10f %10f\n             %10f %10f %10f %10f\n", sx1,sy1,sx2,sy2,dx1,dy1,dx2,dy2);

  if(sx1>dx1 && sx1<dx2 && sy1>dy1 && sy1<dy2)
    corner_in++;

  if(sx2>dx1 && sx2<dx2 && sy2>dy1 && sy2<dy2)
    corner_in++;

  if(sx2>dx1 && sx2<dx2 && sy1>dy1 && sy1<dy2)
    corner_in++;

  if(sx1>dx1 && sx1<dx2 && sy2>dy1 && sy2<dy2)
    corner_in++;

  return (corner_in>=3 ? TRUE : FALSE);
}

/* Move */
static gboolean minigolf_move(GList *item_list)
{
  GooCanvasBounds bounds;
  MachItem		*machItem;
  GooCanvasItem	*item;
  guint			 i;
  gboolean		 collision = FALSE;
  double	         xpos, ypos;

  for(i=0; i<g_list_length(item_list); i++)
    {
      gint j;

      machItem = g_list_nth_data(item_list, i);

      //      dump_machItem(machItem);
      item = machItem->item;

      if(machItem->moving)
	{
	  goo_canvas_item_get_bounds(item, &bounds);

	  machItem->times += times_inc;

	  /* Collision detection */
	  for(j=0; j<g_list_length(item_list); j++)
	    {
	      MachItem	      *collMachItem;

	      collMachItem = g_list_nth_data(item_list, j);

	      if(collMachItem != machItem) {

		GooCanvasBounds bounds2;
		goo_canvas_item_get_bounds(collMachItem->item, &bounds2);

		if(rectangle_in(bounds.x1, bounds.y1, bounds.x2, bounds.y2,
				bounds2.x1, bounds2.y1, bounds2.x2, bounds2.y2))
		  {
		    //printf("!!! Collision detected with:\n");
		    //dump_machItem(collMachItem);
		    gc_sound_play_ogg ("sounds/flip.wav", NULL);
		    collision = TRUE;

		    gamewon = TRUE;
		    minigolf_destroy_all_items();
		    gc_bonus_display(gamewon, GC_BONUS_SMILEY);
		    return(FALSE);
		  }
	      }
	    }

	  ypos=machItem->yposo
	    + (machItem->vyo*machItem->times)
	    + (.5*machItem->ay * (machItem->times*machItem->times));

	  /* Simulate going slower */
	  if(ABS(machItem->ypos-ypos)>0.3) {
	    machItem->vyo = machItem->vyo * velocity;
	  } else {
	    machItem->yposo = ypos;
	    machItem->vyo = 0;
	  }

	  xpos=machItem->xposo
	    + (machItem->vxo*machItem->times)
	    + (.5*machItem->ax * (machItem->times*machItem->times));

	  /* Simulate going slower */
	  if(ABS(machItem->xpos-xpos)>0.3) {
	    machItem->vxo = machItem->vxo * velocity;
	  } else {
	    machItem->xposo = xpos;
	    machItem->vxo = 0;
	  }

	  machItem->xpos=xpos;
	  machItem->ypos=ypos;

	  /* v = u + at */
	  machItem->vxo += (machItem->ax * machItem->times);
	  machItem->vyo += (machItem->ay * machItem->times);

	  if(machItem->ypos >= MIN_Y2 - machItem->height -1)
	    {
	      machItem->ypos = MIN_Y2 - machItem->height;
	    }

	  if(machItem->ypos < MIN_Y1)
	    {
	      machItem->ypos = MIN_Y1;
	    }

	  if(machItem->xpos < MIN_X1)
	    {
	      machItem->xpos = MIN_X1;
	    }

	  if(machItem->xpos > MIN_X2)
	    {
	      machItem->xpos = MIN_X2;
	    }


	  if (machItem->vxo != 0 || machItem->vyo != 0)
	    gc_item_absolute_move(item, machItem->xpos, machItem->ypos);

	  if((machItem->ypos>=MIN_Y2-machItem->height-BORDER
	      && (bounds.y1 - machItem->ypos)<=0) || collision == TRUE)
	    {
	      machItem->vyo   = machItem->vyo * -0.5;
	      machItem->vxo   = machItem->vxo * 0.5;
	      machItem->times = 0;
	      machItem->yposo = machItem->ypos;
	      machItem->xposo = machItem->xpos;

	      /* Floor touch */
	      //machItem->vxo *= 0.9;
	      gc_sound_play_ogg ("sounds/line_end.wav", NULL);
	    }

	  if((bounds.y1<=MIN_Y1 && (bounds.y1 - machItem->ypos)>=0)
	     || collision == TRUE)
	    {
	      machItem->vyo   = machItem->vyo * -0.5;
	      machItem->vxo   = machItem->vxo * 0.5;
	      machItem->times=0;
	      machItem->yposo=machItem->ypos;
	      machItem->xposo=machItem->xpos;
	      gc_sound_play_ogg ("sounds/line_end.wav", NULL);
	    }

	  //	  if(x1<=5 && (x1 - machItem->xpos)>0 || collision == TRUE)
	  if((bounds.x1<=MIN_X1 && machItem->vxo<0) || collision == TRUE)
	    {
	      machItem->vyo   = machItem->vyo * 0.5;
	      machItem->vxo   = machItem->vxo * -0.5;
	      machItem->times=0;
	      machItem->yposo=machItem->ypos;
	      machItem->xposo=machItem->xpos;
	      gc_sound_play_ogg ("sounds/line_end.wav", NULL);
	    }

	  if((bounds.x2>=MIN_X2 && machItem->vxo>0) || collision == TRUE)
	    {
	      machItem->vyo = 0.5 * machItem->vyo;
	      machItem->vxo = machItem->vxo * -0.5;
	      machItem->times=0;
	      machItem->yposo=machItem->ypos;
	      machItem->xposo=machItem->xpos;
	      gc_sound_play_ogg ("sounds/line_end.wav", NULL);
	    }

	}

    }
  return(TRUE);
}

