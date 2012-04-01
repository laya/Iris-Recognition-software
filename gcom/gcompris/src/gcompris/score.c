/* gcompris - score.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
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

/**
 * This includes an API to count scores in gcompris
 *
 */

#include "gcompris.h"

static GooCanvasItem *boardRootItem = NULL;

static guint x, y, max;
static ScoreStyleList currentStyle;

/*
 * Forward declarations
 */
static void display_number(GooCanvasItem *parent,
			   guint x,
			   guint y,
			   char *operand_str);

/*
 * Main entry score
 * ----------------
 *
 */


/*
 * Do all the score display
 */
static void
score_start (ScoreStyleList style, guint gx, guint gy, guint gmax)
{

  currentStyle = style;
  x = gx;
  y = gy;
  max = gmax;


  switch(currentStyle) {
  case SCORESTYLE_NOTE :
    break;
  default:
    break;
  }
}


static void
score_end()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem=NULL;
}

static void
score_set_max(guint gmax)
{
  max = gmax;
}

static void
score_set(guint value)
{

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = \
    goo_canvas_group_new (goo_canvas_get_root_item(GOO_CANVAS(gc_board_get_current()->canvas)),
			  NULL);

  switch(currentStyle) {
  case SCORESTYLE_NOTE :
    {
      gchar *tmp;
      GooCanvasItem *item;
      GooCanvasBounds bounds;

      item =
	goo_canvas_svg_new (boardRootItem,
			    gc_skin_rsvg_get(),
			    "svg-id", "#BUTTON_TEXT",
			    NULL);
      goo_canvas_item_get_bounds(item, &bounds);

      SET_ITEM_LOCATION(item,
			x,
			y - (bounds.y2 - bounds.y1)/2 );

      tmp = g_strdup_printf("%d/%d", value, max);
      display_number(boardRootItem,
		     x + (bounds.x2 - bounds.x1)/2,
		     y - (bounds.y2 - bounds.y1)/2,
		     tmp);
      g_free(tmp);
    }
    break;
  default:
    break;
  }
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/


static void
display_number(GooCanvasItem *parent,
	       guint x,
	       guint y,
	       char *operand_str)
{

  goo_canvas_text_new (parent,
		       operand_str,
		       x+2,
		       y+2,
		       -1,
		       GTK_ANCHOR_NORTH,
		       "alignment", PANGO_ALIGN_CENTER,
		       "font", gc_skin_font_board_huge_bold,
		       "fill-color-rgba", 0x696969FF,
		       NULL);

  goo_canvas_text_new (parent,
		       operand_str,
		       x,
		       y,
		       -1,
		       GTK_ANCHOR_NORTH,
		       "alignment", PANGO_ALIGN_CENTER,
		       "font", gc_skin_font_board_huge_bold,
		       "fill-color-rgba", 0xFFFFFFFF,
		       NULL);
}

/* custom score registration */

static Score *custom_score = NULL;

void
gc_score_register (Score *score)
{
    custom_score = score;
}

void
gc_score_start (ScoreStyleList style, guint x, guint y, guint max)
{
    if (custom_score == NULL)
        score_start (style, x, y, max);
    else if (custom_score->start != NULL)
        custom_score->start (style, x, y, max);
}

void
gc_score_end ()
{
    if (custom_score == NULL)
        score_end ();
    else if (custom_score->end != NULL)
        custom_score->end ();
}

/**
 * Set a new max score, will be displayed at next call
 * to score_set
 */
void
gc_score_set_max(guint max)
{
    if (custom_score == NULL)
      score_set_max(max);
    else if (custom_score->set_max != NULL)
        custom_score->set_max (max);
}

void
gc_score_set (guint value)
{
    if (custom_score == NULL)
        score_set (value);
    else if (custom_score->set != NULL)
        custom_score->set (value);
}
