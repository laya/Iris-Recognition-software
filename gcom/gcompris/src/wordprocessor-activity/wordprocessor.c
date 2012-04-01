/* gcompris - wordprocessor.c
 *
 * Copyright (C) 2006, 2008 Bruno Coudoin
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
#include <glib/gstdio.h>
#include <libxml/HTMLparser.h>

#include "gcompris/gcompris.h"

/*
 * Predefined styles
 * -----------------
 */
typedef struct {
  gchar *name;
  gchar *font;
  PangoWeight weight;
  GtkJustification justification;
  gint left_margin;
  gint pixels_above_lines;
  gint pixels_below_lines;
} style_t;

#define NUMBER_OF_STYLE 4 /* h1 h2 h3 p */

static GtkTextTag *tag_list[NUMBER_OF_STYLE];


/*
 * The document styles
 */
typedef struct {
  gchar *name;
  style_t style[NUMBER_OF_STYLE];
} doctype_t;

doctype_t type_normal =
  {
    .name = N_("Research"),
    .style = {
      { "h1", "Serif 30", PANGO_WEIGHT_ULTRABOLD,  GTK_JUSTIFY_CENTER, 0,  40, 20 },
      { "h2", "Serif 26", PANGO_WEIGHT_BOLD,       GTK_JUSTIFY_LEFT,   0,  30, 15 },
      { "h3", "Serif 20", PANGO_WEIGHT_SEMIBOLD,   GTK_JUSTIFY_LEFT,   15,  20, 12 },
      { "p",  "Serif 16", PANGO_WEIGHT_NORMAL,     GTK_JUSTIFY_LEFT,   30, 3,  3 }
    }
  };

doctype_t type_letter =
  {
    .name = N_("Sentimental"),
    .style = {
      { "h1", "Serif 26", PANGO_WEIGHT_ULTRABOLD,  GTK_JUSTIFY_CENTER, 0,  40, 20 },
      { "h2", "Serif 20", PANGO_WEIGHT_BOLD,       GTK_JUSTIFY_LEFT,   0,  30, 15 },
      { "h3", "Serif 16", PANGO_WEIGHT_SEMIBOLD,   GTK_JUSTIFY_LEFT,   10, 20, 12 },
      { "p",  "Serif 14", PANGO_WEIGHT_NORMAL,     GTK_JUSTIFY_LEFT,   30, 3,  3 }
    },
  };

doctype_t type_small =
  {
    .name = N_("Official"),
    .style = {
      { "h1", "Serif 18", PANGO_WEIGHT_ULTRABOLD,  GTK_JUSTIFY_CENTER, 0,  40, 20 },
      { "h2", "Serif 16", PANGO_WEIGHT_BOLD,       GTK_JUSTIFY_LEFT,   0,  30, 15 },
      { "h3", "Serif 14", PANGO_WEIGHT_SEMIBOLD,   GTK_JUSTIFY_LEFT,   10, 20, 12 },
      { "p",  "Serif 12", PANGO_WEIGHT_NORMAL,     GTK_JUSTIFY_LEFT,   30, 3,  3 }
    },
  };

doctype_t type_text =
  {
    .name = N_("Text"),
    .style = {
      { "h1", "Serif 12", PANGO_WEIGHT_ULTRABOLD,  GTK_JUSTIFY_CENTER, 0,  40, 20 },
      { "h2", "Serif 12", PANGO_WEIGHT_BOLD,       GTK_JUSTIFY_LEFT,   0,  30, 15 },
      { "h3", "Serif 12", PANGO_WEIGHT_SEMIBOLD,   GTK_JUSTIFY_LEFT,   15, 20, 12 },
      { "p",  "Serif 12", PANGO_WEIGHT_NORMAL,     GTK_JUSTIFY_LEFT,   30, 3,  3 }
    },
  };

doctype_t type_big =
  {
    .name = N_("Flyer"),
    .style = {
      { "h1", "Serif 34", PANGO_WEIGHT_ULTRABOLD,  GTK_JUSTIFY_CENTER, 0,  40, 20 },
      { "h2", "Serif 30", PANGO_WEIGHT_BOLD,       GTK_JUSTIFY_LEFT,   0,  30, 15 },
      { "h3", "Serif 26", PANGO_WEIGHT_SEMIBOLD,   GTK_JUSTIFY_LEFT,   15, 20, 12 },
      { "p",  "Serif 18", PANGO_WEIGHT_NORMAL,     GTK_JUSTIFY_LEFT,   30, 3,  3 }
    },
  };
#define NUMBER_OF_DOCTYPE 5
static doctype_t *doctype_list[NUMBER_OF_DOCTYPE];

/*
 * The color styles
 */
#define NUMBER_OF_COLOR_STYLE 4
static gchar *color_style_list[NUMBER_OF_COLOR_STYLE][NUMBER_OF_STYLE+1] =
{
  {N_("Spring"), "red",  "blue",  "lightblue",  "black"},
  {N_("Summer"), "DeepPink",  "HotPink",  "MediumOrchid",  "black"},
  {N_("Autumn"), "blue",  "red",  "lightblue",  "black"},
  {N_("Winter"), "black",  "black",  "black",  "black"},
};

static GcomprisBoard	*gcomprisBoard = NULL;
static gboolean		 board_paused = TRUE;
static GtkWidget	*gtk_combo_styles = NULL;
static GtkWidget	*gtk_combo_colors = NULL;
static GtkWidget	*gtk_button_style[NUMBER_OF_STYLE];
static GtkWidget	*sw = NULL;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static gboolean  key_release_event (GtkWidget *text_view,
				    GdkEventKey *event);

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem	*wordprocessor_create(void);
static void		 wordprocessor_destroy_all_items(void);
static void		 item_event(GtkWidget *item, gchar *data);
static int		 display_style_buttons(GooCanvasItem *boardRootItem,
					       int x,
					       int y);
static void		 create_tags (GtkTextBuffer *buffer, doctype_t *doctype);
static void		 set_default_tag (GtkTextBuffer *buffer, GtkTextTag *tag);
static void		 display_style_selector(GooCanvasItem *boardRootItem, double y);
static void		 display_color_style_selector(GooCanvasItem *boardRootItem, double y);
static void		 item_event_style_selection (GtkComboBox *widget, void *data);
static void		 item_event_color_style_selection (GtkComboBox *widget, void *data);
static gboolean		 save_event (GooCanvasItem  *item,
				     GooCanvasItem  *target,
				     GdkEventButton *event,
				     gchar *data);
static gboolean		 load_event (GooCanvasItem  *item,
				     GooCanvasItem  *target,
				     GdkEventButton *event,
				     gchar *data);
static int		 get_style_index(gchar *style);
static int		 get_style_current_index();
static gint		 get_color_style_index(gchar *color_style);
static gint		 get_color_style_current_index();
static GtkTextTag       *get_tag_from_name(gchar *name);
static void		 apply_style(int style_index);
static void		 apply_color_style(int style_index);

#define word_area_x1 120
#define word_area_y1 20
#define word_area_width 650
#define word_area_height 485

#define combo_style_x1 5
#define combo_style_width 105

static GtkTextBuffer *buffer;
static GtkWidget *view;
GtkTextTag *selected_tag;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Wordprocessor",
    "A basic word processor",
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

GET_BPLUGIN_INFO(wordprocessor)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  int i;

  if(gcomprisBoard==NULL)
    return;

  /* Widgets don't like being overlapped */
  if(pause)
    {
      gtk_widget_hide(GTK_WIDGET(sw));
      gtk_widget_hide(GTK_WIDGET(gtk_combo_styles));
      gtk_widget_hide(GTK_WIDGET(gtk_combo_colors));
      for(i=0; i<NUMBER_OF_STYLE; i++)
	gtk_widget_hide(gtk_button_style[i]);
    }
  else
    {
      gtk_widget_show(GTK_WIDGET(sw));
      gtk_widget_show(GTK_WIDGET(gtk_combo_styles));
      gtk_widget_show(GTK_WIDGET(gtk_combo_colors));
      for(i=0; i<NUMBER_OF_STYLE; i++)
	gtk_widget_show(gtk_button_style[i]);
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
      gcomprisBoard->maxlevel=1;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gc_bar_set(0);
      gc_bar_location(10, -1, 0.6);

      gc_set_default_background(goo_canvas_get_root_item(gcomprisBoard->canvas));

      wordprocessor_create();

      pause_board(FALSE);

    }
}
/* ======================================= */
static void end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      wordprocessor_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

/* ======================================= */
static void set_level (guint level)
{
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_ascii_strcasecmp(gcomprisBoard->type, "wordprocessor")==0)
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
/* ==================================== */
/* Destroy all the items */
static void wordprocessor_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}
/* ==================================== */
static GooCanvasItem *wordprocessor_create()
{
  GooCanvasItem *item = NULL;
  GdkPixbuf *pixmap;
  double y;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  selected_tag = NULL;
  view = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
  /* Change left margin throughout the widget */
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 1);
  g_signal_connect (view, "key-release-event",
		    G_CALLBACK (key_release_event), NULL);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));


  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_container_add (GTK_CONTAINER (sw), view);

  item = goo_canvas_widget_new (boardRootItem,
				GTK_WIDGET(sw),
				word_area_x1,
				word_area_y1,
				word_area_width,
				word_area_height,
				"anchor", GTK_ANCHOR_NW,
				NULL);
  gtk_widget_show(GTK_WIDGET(view));
  gtk_widget_show(GTK_WIDGET(sw));

  /*
   * Create the default style tags
   */
  doctype_list[0] = &type_text;
  doctype_list[1] = &type_normal;
  doctype_list[2] = &type_letter;
  doctype_list[3] = &type_small;
  doctype_list[4] = &type_big;

  y = 20.0;
  /*
   * The save button
   */
  pixmap = gc_pixmap_load("wordprocessor/tool-save.png");
  item = \
    goo_canvas_image_new (boardRootItem,
			  pixmap,
			  17.0,
			  y,
			  NULL);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
  gdk_pixbuf_unref(pixmap);
#else
  g_object_unref(pixmap);
#endif
  g_signal_connect(item, "button_press_event",
		   (GCallback) save_event, buffer);
  gc_item_focus_init(item, NULL);


  /*
   * The load button
   */
  pixmap = gc_pixmap_load("wordprocessor/tool-load.png");
  item = \
    goo_canvas_image_new (boardRootItem,
			  pixmap,
			  60.0,
			  y,
			  NULL);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
  gdk_pixbuf_unref(pixmap);
#else
  g_object_unref(pixmap);
#endif
  g_signal_connect(item, "button_press_event",
		   (GCallback) load_event, buffer);
  gc_item_focus_init(item, NULL);


  y += 45;
  /*
   * Display the style buttons
   */
  y = display_style_buttons(boardRootItem,
			    20.0,
			    y);

  y += 20;
  display_style_selector(boardRootItem, y);

  y += 40;
  display_color_style_selector(boardRootItem, y);

  /* Now we can create the tags */
  create_tags(buffer, doctype_list[0]);

  gtk_widget_grab_focus(view);

 return NULL;
}

/*
 * Display the style buttons
 *
 * \return the new y coordinate
 */
static int
display_style_buttons(GooCanvasItem *boardRootItem,
		      int x,
		      int y)
{
  int offset_y = 40;
  int i = 0;
  static gchar *styles_tab[] = { N_("Title"), "h1",
				 N_("Heading 1"), "h2",
				 N_("Heading 2"), "h3",
				 N_("Text"), "p",
				 NULL, NULL };

  while(styles_tab[i*2])
    {
      gtk_button_style[i] = gtk_button_new_with_label(gettext(styles_tab[i*2]));

      goo_canvas_widget_new (boardRootItem,
			     GTK_WIDGET(gtk_button_style[i]),
			     combo_style_x1,
			     y,
			     combo_style_width,
			     35.0,
			     "anchor", GTK_ANCHOR_NW,
			     NULL);

      g_signal_connect(GTK_OBJECT(gtk_button_style[i]), "pressed",
			 (GCallback)item_event, styles_tab[i*2+1] );

      y += offset_y;

      i++;
    }

  return(y);
}

/* \brief callback on a style button (h1, h2, h3, p)
 *
 *
 */
static void
item_event(GtkWidget *button, gchar *data)
{
  GtkTextIter    iter_start, iter_end;
  gchar *current_style_name;

  if(board_paused)
    return;

  current_style_name = (char *)data;

  selected_tag = get_tag_from_name(current_style_name);
  set_default_tag(buffer, selected_tag);

  gtk_text_buffer_get_iter_at_mark(buffer,
				   &iter_start,
				   gtk_text_buffer_get_insert (buffer));
  gtk_text_iter_set_line_offset(&iter_start, 0);

  iter_end = iter_start;
  gtk_text_iter_forward_to_line_end(&iter_end);

  gtk_text_buffer_remove_all_tags(buffer,
				  &iter_start,
				  &iter_end);

  gtk_text_buffer_apply_tag_by_name(buffer,
				    current_style_name,
				    &iter_start,
				    &iter_end);

  gtk_widget_grab_focus(view);

  return;
}

/* Create a bunch of tags. Note that it's also possible to
 * create tags with gtk_text_tag_new() then add them to the
 * tag table for the buffer, gtk_text_buffer_create_tag() is
 * just a convenience function. Also note that you don't have
 * to give tags a name; pass NULL for the name to create an
 * anonymous tag.
 *
 * In any real app, another useful optimization would be to create
 * a GtkTextTagTable in advance, and reuse the same tag table for
 * all the buffers with the same tag set, instead of creating
 * new copies of the same tags for every buffer.
 *
 * Tags are assigned default priorities in order of addition to the
 * tag table.	 That is, tags created later that affect the same text
 * property affected by an earlier tag will override the earlier
 * tag.  You can modify tag priorities with
 * gtk_text_tag_set_priority().
 */

static void
create_tags (GtkTextBuffer *buffer, doctype_t *doctype)
{
  gint i;
  gint c = get_color_style_current_index();

  for(i=0; i<NUMBER_OF_STYLE; i++)
    {
      GtkTextTag *tag;

      tag = gtk_text_buffer_create_tag (buffer, doctype->style[i].name,
					"weight", doctype->style[i].weight,
					"font", doctype->style[i].font,
					"justification", doctype->style[i].justification,
					"left-margin", doctype->style[i].left_margin,
					"pixels-above-lines", doctype->style[i].pixels_above_lines,
					"pixels-below-lines", doctype->style[i].pixels_below_lines,
					"foreground",color_style_list[c][i+1],
					NULL);
      tag_list[i] = tag;
      g_object_set_data (G_OBJECT (tag), "style", &doctype->style[i]);
    }

  /* Point to the last style */
  i--;

  set_default_tag(buffer, tag_list[i]);
}

/*
 * Set the default style
 */
static void
set_default_tag (GtkTextBuffer *buffer, GtkTextTag *tag)
{
  PangoFontDescription *font_desc;
  GdkColor *color = (GdkColor *)g_malloc(sizeof(GdkColor));
  int val;
  GtkJustification justification;

  if(!tag)
    return;

  g_object_get (G_OBJECT (tag), "foreground-gdk", &color, NULL);
  g_object_get (G_OBJECT (tag), "font-desc", &font_desc, NULL);

  gtk_widget_modify_font (view, font_desc);
  gtk_widget_modify_text (view, GTK_STATE_NORMAL, color);

  g_object_get (G_OBJECT (tag), "left-margin", &val, NULL);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), val);

  g_object_get (G_OBJECT (tag), "justification", &justification, NULL);
  gtk_text_view_set_justification(GTK_TEXT_VIEW (view), justification);

  g_object_get (G_OBJECT (tag), "pixels-below-lines", &val, NULL);
  gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW (view), val);

  g_object_get (G_OBJECT (tag), "pixels-above-lines", &val, NULL);
  gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW (view), val);
}

static GtkTextTag *
get_tag_from_name(gchar *tag_name)
{
  gint i;

  for(i=0; i<NUMBER_OF_STYLE; i++)
    {
      gchar *name;
      g_object_get(G_OBJECT (tag_list[i]), "name", &name, NULL);
      if(strcmp(name, tag_name)==0)
	return(tag_list[i]);

    }
  return(tag_list[i-1]);
}

/*
 * Create the combo with the styles
 * --------------------------------
 */
static void
display_style_selector(GooCanvasItem *boardRootItem, double y)
{
  int i = 0;

  gtk_combo_styles = gtk_combo_box_new_text();

  while (i < NUMBER_OF_DOCTYPE)
    gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_combo_styles),
			      gettext(doctype_list[i++]->name));

  goo_canvas_widget_new (boardRootItem,
			 GTK_WIDGET(gtk_combo_styles),
			 combo_style_x1,
			 y,
			 combo_style_width,
			 35.0,
			 "anchor", GTK_ANCHOR_NW,
			 NULL);

  gtk_widget_show(GTK_WIDGET(gtk_combo_styles));
  gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_combo_styles), 0);

  g_signal_connect(G_OBJECT(gtk_combo_styles),
		   "changed",
		   G_CALLBACK(item_event_style_selection),
		   NULL);
}

/*
 * Create the combo with the color styles
 * --------------------------------------
 */
static void
display_color_style_selector(GooCanvasItem *boardRootItem, double y)
{
  int i = 0;

  gtk_combo_colors = gtk_combo_box_new_text();

  while (i < NUMBER_OF_COLOR_STYLE)
    gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_combo_colors),
			      gettext(color_style_list[i++][0]));

  goo_canvas_widget_new (boardRootItem,
			 GTK_WIDGET(gtk_combo_colors),
			 combo_style_x1,
			 y,
			 combo_style_width,
			 35.0,
			 "anchor", GTK_ANCHOR_NW,
			 NULL);

  gtk_widget_show(GTK_WIDGET(gtk_combo_colors));
  gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_combo_colors), 0);

  g_signal_connect(G_OBJECT(gtk_combo_colors),
		   "changed",
		   G_CALLBACK(item_event_color_style_selection),
		   NULL);
}

static int
get_style_index(gchar *style)
{
  int i;

  /* Search the doctype */
  for(i=0; i<NUMBER_OF_DOCTYPE; i++)
    if(strcmp(gettext(doctype_list[i]->name), style)==0)
      return(i);

  return(0);
}

static int
get_style_current_index()
{
  return( get_style_index(gtk_combo_box_get_active_text(GTK_COMBO_BOX(gtk_combo_styles))) );
}

static void
apply_style(int style_index)
{
  int i = style_index;
  int j;

  for(j=0; j<NUMBER_OF_STYLE; j++)
    {
      g_object_set(tag_list[j],
		   "weight", doctype_list[i]->style[j].weight,
		   "font", doctype_list[i]->style[j].font,
		   "justification", doctype_list[i]->style[j].justification,
		   "left-margin", doctype_list[i]->style[j].left_margin,
		   "pixels-above-lines", doctype_list[i]->style[j].pixels_above_lines,
		   "pixels-below-lines", doctype_list[i]->style[j].pixels_below_lines,
		   NULL);
    }
}

/* Set a new style from the combo box selection
 *
 */
static void
item_event_style_selection (GtkComboBox *widget,
			    void *data)
{

  apply_style(get_style_current_index());

  gtk_widget_grab_focus(view);
}

static gint
get_color_style_index(gchar *color_style)
{
  int i;

  /* Search the color style */
  for(i=0; i<NUMBER_OF_COLOR_STYLE; i++)
    if(strcmp(gettext(color_style_list[i][0]), color_style)==0)
      return(i);

  return(0);
}

static gint
get_color_style_current_index()
{
  return( get_color_style_index(gtk_combo_box_get_active_text(GTK_COMBO_BOX(gtk_combo_colors))) );
}

static void
apply_color_style(int color_style_index)
{
  int i = color_style_index;
  int j;

  /* Change the color */
  for(j=0; j<NUMBER_OF_STYLE; j++)
    g_object_set(tag_list[j],
		 "foreground",color_style_list[i][j+1],
		 NULL);
}

/* Set a new color style from the combo box selection
 *
 */
static void
item_event_color_style_selection (GtkComboBox *widget,
				  void *data)
{
  apply_color_style(get_color_style_current_index());
  gtk_widget_grab_focus(view);
}


/* Catch all typing events to apply the proper tags
 *
 */
static gboolean
key_release_event (GtkWidget *text_view,
		   GdkEventKey *event)
{
  GtkTextIter iter_start, iter_end;
  GtkTextBuffer *buffer;

  {
    GSList *tags = NULL, *tagp = NULL;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    gtk_text_buffer_get_iter_at_mark(buffer,
				     &iter_start,
				     gtk_text_buffer_get_insert (buffer));
    gtk_text_iter_set_line_offset(&iter_start, 0);

    iter_end = iter_start;
    gtk_text_iter_forward_to_line_end(&iter_end);

    tags = gtk_text_iter_get_tags (&iter_start);

    if(g_slist_length(tags) == 0)
      {
	gtk_text_iter_backward_char (&iter_end);
	tags = gtk_text_iter_get_tags (&iter_end);
	gtk_text_iter_forward_char (&iter_end);
      }

    for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
      {
	GtkTextTag *tag = tagp->data;
	gchar *name;
	g_object_get (G_OBJECT (tag), "name", &name, NULL);

	set_default_tag(buffer, tag);
	gtk_text_buffer_apply_tag_by_name(buffer,
					  name,
					  &iter_start,
					  &iter_end);
	g_free(name);
	selected_tag = NULL;
      }

    if (tags)
      g_slist_free (tags);
    else
      {
	/* Set the default style */
	if(selected_tag)
	  {
	    set_default_tag(buffer, selected_tag);

	    gtk_text_buffer_apply_tag(buffer,
				      selected_tag,
				      &iter_start,
				      &iter_end);
	  }
	else
	  {
	    set_default_tag(buffer, tag_list[NUMBER_OF_STYLE-1]);

	    gtk_text_buffer_apply_tag(buffer,
				      tag_list[NUMBER_OF_STYLE-1],
				      &iter_start,
				      &iter_end);
	  }
      }
  }

  return FALSE;
}

// assumes UTF-8 or UTF-16 as encoding,
static char *
escape(char *input)
{
  gsize size = strlen(input)*6; /* 6 is the most increase we can get */
  gchar *result = g_malloc(size);
  int i;
  int o = 0;

  result[0] = '\0';

  for(i = 0; i < strlen(input); i++)
    {
      char c = input[i];
      if(c == '<')
	o = g_strlcat(result, "&lt;", size);
      else if(c == '>')
	o = g_strlcat(result, "&gt;", size);
      else if(c == '&')
	o = g_strlcat(result, "&amp;", size);
      else if(c == '"')
	o = g_strlcat(result, "&quot;", size);
      else if(c == '\'')
	o = g_strlcat(result, "&apos;", size);
      else
	{
	  result[o++] = c;
	  result[o+1] = '\0';
	}
    }
  g_free(input);
  return result;
}

static void
save_buffer(gchar *file, gchar *file_type, void *unused)
{
  GtkTextIter iter_start, iter_end;
  GSList *tags = NULL, *tagp = NULL;
  gchar *tag_name;
  FILE *filefd;
  int style_index = get_style_current_index();
  int color_index = get_color_style_current_index();

  filefd = g_fopen(file, "w+");

  /*
   * XHTML Header
   */
  fprintf(filefd,
	  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
	  "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
	  "<head>\n"
	  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
	  "<meta http-equiv=\"GCompris-doctype\" content=\"%s\" />\n"
	  "<meta http-equiv=\"GCompris-color-style\" content=\"%s\" />\n"
	  "<title>GCompris</title>\n",
	  doctype_list[style_index]->name,
	  color_style_list[color_index][0]);

  /*
   * HTML Style
   */
  fprintf(filefd,
	  "<style type=\"text/css\">\n");

  {
    int i;
    int font_size[NUMBER_OF_STYLE] = { 28, 22, 16, 12 };
    char *align[NUMBER_OF_STYLE] = { "center", "left", "left", "justify" };
    int left_margin[NUMBER_OF_STYLE] = { 0, 10, 20, 30 };

    for(i=0; i<NUMBER_OF_STYLE; i++)
      {
	style_t style = doctype_list[style_index]->style[i];
	fprintf(filefd,
		"%s {\n"
		"  color : %s;\n"
		"  font-size : %dpx;\n"
		"  text-align : %s;\n"
		"  margin-left : %dpx;\n"
		"}\n",
		style.name,
		color_style_list[color_index][i+1],
		font_size[i],
		align[i],
		left_margin[i]);
      }

  }
  fprintf(filefd,
	  "</style>\n"
	  "</head>\n");

  /*
   * Header end
   */
  fprintf(filefd,
	  "<body>\n");

  gtk_text_buffer_get_iter_at_offset(buffer,
				     &iter_start,
				     0);

  do
    {
      iter_end = iter_start;
      gtk_text_iter_forward_to_line_end(&iter_end);

      if(gtk_text_iter_ends_line(&iter_start))
	continue;

      tags = gtk_text_iter_get_tags (&iter_start);
      if(g_slist_length(tags) == 0)
	{
	  gtk_text_iter_backward_char (&iter_end);
	  tags = gtk_text_iter_get_tags (&iter_end);
	  gtk_text_iter_forward_char (&iter_end);
	}

      tag_name = "p";
      for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
	{
	  GtkTextTag *tag = tagp->data;
	  g_object_get (G_OBJECT (tag), "name", &tag_name, NULL);

	}
      fprintf(filefd, "<%s>", tag_name);

      char *result = escape(gtk_text_buffer_get_text(buffer,
						     &iter_start,
						     &iter_end,
						     0));

      for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
	{
	  GtkTextTag *tag = tagp->data;
	  g_object_get (G_OBJECT (tag), "name", &tag_name, NULL);

	}
      fprintf(filefd, "%s</%s>\n", result, tag_name);
      g_free(result);

      if (tags)
	g_slist_free (tags);

    } while(gtk_text_iter_forward_line(&iter_start));

  /*
   * HTML Footer
   */
  fprintf(filefd, ""
	  "</body>\n"
	  "</html>\n");

  fclose(filefd);

  pause_board(FALSE);

}

static gboolean
save_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    gchar *data)
{
  if (event->button != 1)
    return FALSE;

  pause_board(TRUE);

  gc_selector_file_save(gcomprisBoard,
			"wordprocessor",
			".xhtml",
			save_buffer, NULL);

  return FALSE;
}

static void
load_buffer(gchar *file, gchar *file_type, void *unused)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
  xmlDocPtr doc;
  xmlNodePtr node;
  GtkTextIter iter_start, iter_end;

  /* parse the new file and put the result into newdoc */
  doc = xmlParseFile(file);

  /* in case something went wrong */
  if(!doc)
    return;

  /* Get the root element node */
  node = xmlDocGetRootElement(doc);

  for(node = node; node != NULL; node = node->next)
    if ( g_ascii_strcasecmp((char *)node->name, "html") == 0 &&
	 node->children )
      break;

  if(!node)
    goto done;

  for(node = node->children; node != NULL; node = node->next)
    {
      if ( g_ascii_strcasecmp((char *)node->name, "head") == 0 &&
	   node->children )
	{
	  /* Search and apply the saved style in the META */
	  xmlNodePtr snode;
	  for(snode = node->children; snode != NULL; snode = snode->next)
	    {
	      if ( ( g_ascii_strcasecmp((char *)snode->name, "meta") == 0 ) &&
		   xmlHasProp(snode, BAD_CAST "http-equiv") )
		{
		  xmlChar *key = xmlGetProp(snode, BAD_CAST "http-equiv");
		  xmlChar *content = xmlGetProp(snode, BAD_CAST "content");

		  if(g_ascii_strcasecmp((char *)key, "GCompris-doctype") == 0)
		    {
		      int style_index = get_style_index(gettext((char *)content));
		      apply_style(style_index);
		      gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_combo_styles),
					       style_index);
		    }

		  if(g_ascii_strcasecmp((char *)key, "GCompris-color-style") == 0)
		    {
		      int cstyle_index = get_color_style_index(gettext((char *)content));
		      apply_color_style(cstyle_index);
		      gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_combo_colors),
					       cstyle_index);
		    }

		  xmlFree(key);
		  xmlFree(content);
		}
	    }
	}

      if ( g_ascii_strcasecmp((char *)node->name, "body") == 0 &&
	   node->children )
	break;
    }

  if(!node)
    goto done;

  gtk_text_buffer_get_start_iter(buffer,
				 &iter_start);
  gtk_text_buffer_get_end_iter(buffer,
			       &iter_end);
  gtk_text_buffer_delete(buffer,
			 &iter_start,
			 &iter_end);

  gtk_text_buffer_get_start_iter(buffer,
				 &iter_start);

  for(node = node->children; node != NULL; node = node->next)
    {

      if ( g_ascii_strcasecmp((char *)node->name, "h1") == 0 ||
	   g_ascii_strcasecmp((char *)node->name, "h2") == 0 ||
	   g_ascii_strcasecmp((char *)node->name, "h3") == 0 ||
	   g_ascii_strcasecmp((char *)node->name, "p") == 0 )
	{
	  xmlChar *content;
	  content = xmlNodeGetContent(node);
	  gtk_text_buffer_insert_with_tags_by_name(buffer,
						   &iter_start,
						   (char *)content,
						   strlen((char *)content),
						   (char *)node->name,
						   NULL);
	  xmlFree(content);
	  gtk_text_buffer_get_end_iter(buffer,
				       &iter_start);
	  gtk_text_buffer_insert(buffer,&iter_start, "\n", 1);

	  gtk_text_buffer_get_end_iter(buffer,
					 &iter_start);

	}

    }

 done:
  xmlFreeDoc(doc);
}

static gboolean
load_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    gchar *data)
{
  if (event->button != 1)
    return FALSE;

  pause_board(TRUE);

  gc_selector_file_load(gcomprisBoard,
			"wordprocessor",
			".xhtml",
			load_buffer, NULL);

  return FALSE;
}

