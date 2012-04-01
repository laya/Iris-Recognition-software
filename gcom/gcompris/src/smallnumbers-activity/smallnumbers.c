/* gcompris - smallnumbers.c
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
#include <string.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint drop_items_id = 0;

static char *numbers = "123456";
static  int  gamewon;
static guint number_of_dices=1;

static GooCanvasItem *boardRootItem = NULL;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str);

static void smallnumbers_create_item(GooCanvasItem *parent);
static gboolean smallnumbers_drop_items (gpointer data);
static gboolean smallnumbers_move_items (gpointer data);
static void smallnumbers_destroy_all_items(void);
static void smallnumbers_next_level(void);
static gboolean smallnumbers_gotkey_item(GooCanvasItem *item, guint key);

static void		 smallnumber_config_start(GcomprisBoard *agcomprisBoard,
					     GcomprisProfile *aProfile);
static void		 smallnumber_config_stop(void);

static void player_win(GooCanvasItem *item);
static void player_loose(void);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;
static  double               imageZoom = 0.0;

/* if board has alternate locale */
static gchar *locale_sound = NULL;
static gboolean with_sound = FALSE;

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   "Numbers With Dice",
   "Type the keyboard number 1 to 9 before the dice reaches the ground",
   "Bruno Coudoin <bruno.coudoin@free.fr>",
   NULL,
   NULL,
   NULL,
   NULL,
   start_board,
   pause_board,
   end_board,
   is_our_board,
   key_press,
   NULL,
   set_level,
   NULL,
   NULL,
   smallnumber_config_start,
   smallnumber_config_stop
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(smallnumbers)

/*
 * in : boolean TRUE = PAUSE : FALSE = UNPAUSE
 *
 */
static void pause_board (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

  if(pause)
    {
      if (dummy_id) {
	g_source_remove (dummy_id);
	dummy_id = 0;
      }
      if (drop_items_id) {
	g_source_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(gamewon == TRUE) /* the game is won */
	{
	  smallnumbers_next_level();
	}

      if(!drop_items_id) {
	drop_items_id = g_timeout_add (1000,
				       smallnumbers_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = g_timeout_add (1000, smallnumbers_move_items, NULL);
      }
    }
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  GHashTable *config = gc_db_get_board_conf();

  locale_sound = g_strdup(g_hash_table_lookup( config, "locale_sound"));

  gchar *control_sound = g_hash_table_lookup( config, "with_sound");

  if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
    with_sound = TRUE;
  else
    with_sound = FALSE;

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"smallnumbers/scenery7_background.png");

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gcomprisBoard->number_of_sublevel=10;
      gc_score_start(SCORESTYLE_NOTE,
			   BOARDWIDTH - 195,
			   BOARDHEIGHT - 30,
			   gcomprisBoard->number_of_sublevel);
      gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL);


      number_of_dices=1;
      if(!gcomprisBoard->mode)
	number_of_dices=1;
      else if(g_ascii_strncasecmp(gcomprisBoard->mode, "2_DICES", 7)==0) {
	/* 2 Dices mode */
	number_of_dices=2;
      }

      smallnumbers_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}

static void
end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      smallnumbers_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      smallnumbers_next_level();
    }
}

static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  char str[2];
  int i;

  if(!gcomprisBoard || !boardRootItem)
    return FALSE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
    case GDK_Shift_L:
    case GDK_Shift_R:
    case GDK_Control_L:
    case GDK_Control_R:
    case GDK_Caps_Lock:
    case GDK_Shift_Lock:
    case GDK_Meta_L:
    case GDK_Meta_R:
    case GDK_Alt_L:
    case GDK_Alt_R:
    case GDK_Super_L:
    case GDK_Super_R:
    case GDK_Hyper_L:
    case GDK_Hyper_R:
    case GDK_Mode_switch:
    case GDK_dead_circumflex:
    case GDK_Num_Lock:
      return FALSE;
    }

  sprintf(str, "%c", keyval);

  keyval = atoi(str);

  /* Warning, deleting items in a loop is not safe */
  int gotit = FALSE;
  int nb_item = goo_canvas_item_get_n_children(boardRootItem);

  for(i=0;
      (!gotit && i< nb_item);
      i++)
    gotit = smallnumbers_gotkey_item( goo_canvas_item_get_child(boardRootItem, i),
				      keyval );

  return TRUE;
}

gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_ascii_strcasecmp(gcomprisBoard->type, "smallnumbers")==0)
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
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/* set initial values for the next level */
static void smallnumbers_next_level()
{

  gamewon = FALSE;
  gc_bar_set_level(gcomprisBoard);

  smallnumbers_destroy_all_items();

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  /* Try the next level */
  speed=100+(40/gcomprisBoard->level);
  fallSpeed=5000-gcomprisBoard->level*200;
  imageZoom=0.4+(0.5/gcomprisBoard->level);
  gcomprisBoard->sublevel=1;
  gc_score_set(gcomprisBoard->sublevel);
}


/* Destroy all the items */
static void smallnumbers_destroy_all_items()
{

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;

}
static gboolean
smallnumbers_gotkey_item(GooCanvasItem *item, guint key)
{
  guint number;
  gboolean gotit = FALSE;

  if(G_OBJECT (item)) {
    number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "dice_number"));

    if(number == key) {
      gotit = TRUE;
      player_win(item);
    }
  }
  return(gotit);
}

static void smallnumbers_move_item(GooCanvasItem *item)
{
  if (item == NULL )
    return;

  goo_canvas_item_translate(item, 0, 2.0);

  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds (item, &bounds);

  if(bounds.y1>BOARDHEIGHT) {
    player_loose();
    goo_canvas_item_remove(item);
  }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gboolean smallnumbers_move_items (gpointer data)
{
  int i;
  int count = goo_canvas_item_get_n_children(boardRootItem);

  /* For each item we need to move */
  for(i=0; i<count; i++)
    smallnumbers_move_item(goo_canvas_item_get_child(boardRootItem, i));

  dummy_id = g_timeout_add (gc_timing (speed, count),
			    smallnumbers_move_items, NULL);

  return(FALSE);
}

static void smallnumbers_create_item(GooCanvasItem *parent)
{
  GooCanvasItem *item;
  GooCanvasItem *group_item;
  guint i;
  guint total_number = 0;
  double x = 0;
  static gdouble x_previous = 0; //remember the position of the first dice
  guint number_of_dice = number_of_dices;

  group_item = goo_canvas_group_new (parent, NULL);
  goo_canvas_item_translate(group_item, 0, 40);

  while(number_of_dice-- > 0) {
    gchar *str1 = NULL;
    gchar *str2 = NULL;

    /* Take care not to go above 9 anyway */
    if(total_number==0) {
      i=g_random_int()%6;
    } else {
      int rando = g_random_int()%(9-total_number);
      i=MIN(rando, 5);
    }

    total_number += i + 1;

    /*
     * Play the sound
     */

    if (with_sound)
      {
	gunichar *unichar_letterItem;
	char *lettersItem;
	gchar *letter;

	lettersItem = g_malloc (2);

	sprintf(lettersItem, "%c", numbers[i]);
	lettersItem[1] = '\0';

	unichar_letterItem = g_new(gunichar,1);

	*unichar_letterItem = g_utf8_get_char (lettersItem);

	letter = g_new0(gchar, 6);

	g_unichar_to_utf8(*unichar_letterItem, letter);

	str1 = gc_sound_alphabet(letter);

	g_free(letter);
	g_free(lettersItem);
	g_free(unichar_letterItem);

	str2 = g_strdup_printf("voices/$LOCALE/alphabet/%s", str1);

	gc_sound_play_ogg(str2, NULL);

	g_free(str1);
	g_free(str2);
      }

    /*
     * Now the images
     */
    RsvgHandle *svg_handle;
    RsvgDimensionData rsvg_dimension;
    str1 = g_strdup_printf("smallnumbers/dice%c.svgz", numbers[i]);
    svg_handle = gc_rsvg_load(str1);
    rsvg_handle_get_dimensions (svg_handle, &rsvg_dimension);

    g_free(str1);

    gdouble item_w = rsvg_dimension.width * imageZoom;
    if (x == 0)
      {
	if ( x_previous < BOARDWIDTH / 2 )
	  x = x_previous + item_w +
	    (gdouble)(g_random_int() % (guint)(BOARDWIDTH - x_previous
					       - item_w * 3));

	else
	  x = (double)(g_random_int() % (guint)(x_previous - item_w));

	x_previous = x;
      }
    else
      {
	x += item_w;
      }

    item = goo_canvas_svg_new (group_item, svg_handle, NULL);
    goo_canvas_item_translate(item,
			      x,
			      - (rsvg_dimension.height * imageZoom ));
    goo_canvas_item_scale(item, imageZoom, imageZoom);
    g_object_unref(svg_handle);

  }
  g_object_set_data (G_OBJECT (group_item), "dice_number", GINT_TO_POINTER (total_number));

}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gboolean smallnumbers_drop_items (gpointer data)
{
  gc_sound_play_ogg ("sounds/level.wav", NULL);
  smallnumbers_create_item(boardRootItem);

  drop_items_id = g_timeout_add (fallSpeed,
				 smallnumbers_drop_items, NULL);
  return (FALSE);
}

static void player_win(GooCanvasItem *item)
{
  goo_canvas_item_remove(item);
  gc_sound_play_ogg ("sounds/flip.wav", NULL);

  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel)
    {
      /* Try the next level */
      gcomprisBoard->level++;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel)
	gcomprisBoard->level = gcomprisBoard->maxlevel;

      gamewon = TRUE;
      smallnumbers_destroy_all_items();
      gc_bonus_display(gamewon, GC_BONUS_SMILEY);
    }
  else
    {
      gc_score_set(gcomprisBoard->sublevel);
    }
}

static void player_loose()
{
  gc_sound_play_ogg ("crash", NULL);
}


/* ======================= */
/* = config_start        = */
/* ======================= */

static GcomprisProfile *profile_conf;
static GcomprisBoard   *board_conf;

static GHFunc save_table (gpointer key,
		    gpointer value,
		    gpointer user_data)
{
  gc_db_set_board_conf ( profile_conf,
			    board_conf,
			    (gchar *) key,
			    (gchar *) value);

  return NULL;
}

static gboolean
conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);
    return TRUE;
  }

  g_hash_table_foreach(table, (GHFunc) save_table, NULL);

  if (gcomprisBoard){
    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    if (locale_sound)
      g_free(locale_sound);

    locale_sound = g_strdup(g_hash_table_lookup( config, "locale_sound"));

    gchar *control_sound = g_hash_table_lookup( config, "with_sound");

    if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
      with_sound = TRUE;
    else
      with_sound = FALSE;

    if (profile_conf)
      g_hash_table_destroy(config);

    smallnumbers_next_level();

    gamewon = FALSE;

    pause_board(FALSE);
  }

  board_conf = NULL;
  profile_conf = NULL;
  return TRUE;
}

static void sound_control_box_toggled(GtkToggleButton *togglebutton,
				      gpointer combo)
{
  gtk_widget_set_sensitive(GTK_WIDGET(combo), gtk_toggle_button_get_active (togglebutton));
}


static void
smallnumber_config_start(GcomprisBoard *agcomprisBoard,
		    GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gchar *label;

  label = g_strdup_printf(_("<b>%s</b> configuration\n for profile <b>%s</b>"),
			  agcomprisBoard->name, aProfile ? aProfile->name : "");

  GcomprisBoardConf *bconf;
  bconf = gc_board_config_window_display(label, conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *saved_locale_sound = g_hash_table_lookup( config, "locale_sound");

  gchar *control_sound = g_hash_table_lookup( config, "with_sound");
  if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
    with_sound = TRUE;
  else
    with_sound = FALSE;

  GtkCheckButton  *sound_control = gc_board_config_boolean_box(bconf, _("Enable sounds"), "with_sound", with_sound);

  GtkComboBox *sound_box = gc_board_config_combo_locales_asset(bconf, _("Select sound locale"),
							       saved_locale_sound,
							       "voices/$LOCALE/colors/purple.ogg",
							       NULL);

  gtk_widget_set_sensitive(GTK_WIDGET(sound_box), with_sound);

  g_signal_connect(G_OBJECT(sound_control), "toggled",
		   G_CALLBACK(sound_control_box_toggled),
		   sound_box);

  g_hash_table_destroy(config);

}
/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
smallnumber_config_stop()
{
}
