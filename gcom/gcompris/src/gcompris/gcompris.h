/* gcompris - gcompris.h
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

#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <goocanvas.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <glib.h>
#include <libintl.h>

#include <gmodule.h>

#include "plugin.h"
#include "gcompris-board.h"
#include "board.h"
#include "board_config.h"
#include "properties.h"
#include "gameutil.h"
#include "gc_net.h"
#include "bonus.h"
#include "timer.h"
#include "score.h"
#include "skin.h"
#include "anim.h"

#include "profile.h"
#include "gcompris_db.h"
#include "wordlist.h"
#include "gcompris_im.h"
#include "gcompris_alphabeta.h"

#include "drag.h"

#ifdef USE_CAIRO
#include "gcompris-cairo.h"
#endif

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#include <librsvg/rsvg.h>

#define BOARDWIDTH  800
#define BOARDHEIGHT 520
#define BARHEIGHT   40

#define DEFAULT_SKIN "gartoon"

#define _(String) gettext (String)
#define gettext_noop(String) String

#ifndef N_
#define N_(String) gettext_noop (String)
#endif

#define GC_DEFAULT_LOCALE "en_US.UTF-8"

/*****************************************************************************/
/* Method usefull for boards provided by gcompris */

void		 gc_board_end(void);

/* Control Bar methods */
void		 gc_bar_start (GtkContainer *workspace, GooCanvas *theCanvas);

/** Set the default background of your activity.
 *  Always set the background in your activity startup.
 *  activity startup.
 *  @param[in] parent is the canvas root item
 */
void		 gc_set_default_background(GooCanvasItem *parent);
/** Set the given background of your activity.
 *  Always set the background in your activity startup.
 *  activity startup.
 *  @param[in] parent is the canvas root item
 *  @param[in] file is a relative file. It can be svg, png or jpg.
 */
void		 gc_set_background(GooCanvasItem *parent, gchar *file);
/** Set the given rsvg image as the activity background.
 *  Always set the background in your activity startup.
 *  @param[in] parent is the canvas root item
 *  @param[in] rsvg_handle is the svg background
 *  @param[in] id is the id of the item to display in rsvg_handle.
 *             Set id to NULL to display all the image.
 */
void		 gc_set_background_by_id(GooCanvasItem *parent,
					 RsvgHandle *rsvg_handle,
					 gchar *id);
/** Update the bar to display the current level
 *  You must maintain the current level in your GcomprisBoard->level.
 *  @param[in] gcomprisBoard is your activity handle
 */
void		 gc_bar_set_level (GcomprisBoard *gcomprisBoard);
/** The repeat icon as set with the flag GC_BAR_REPEAT can be
 *  overrided with your specific icon. Use this if the default
 *  repeat icon does not match your action.
 *  @param[in] svg_handle is the handle to the svg image.
 */
void		 gc_bar_set_repeat_icon (RsvgHandle *svg_handle);

/* Status bar control */
typedef enum
{
  GC_BAR_LEVEL	   = 1 << 0,
  GC_BAR_REPEAT	   = 1 << 1,
  GC_BAR_CONFIG	   = 1 << 2,
  GC_BAR_ABOUT	   = 1 << 3,
  GC_BAR_REPEAT_ICON = 1 << 4,
  /* Internal use */
  GC_BAR_HOME	   = 1 << 5,
  GC_BAR_EXIT	   = 1 << 6,
  GC_BAR_LEVEL_DOWN = 1 << 7,
  GC_BAR_HELP      = 1 << 8,
} GComprisBarFlags;

/* Difficulty filtering */
typedef enum {
  GCOMPRIS_FILTER_NONE,
  GCOMPRIS_FILTER_EQUAL,
  GCOMPRIS_FILTER_UNDER,
  GCOMPRIS_FILTER_ABOVE,
} GcomprisFilterType;
/** Set the different option that must be present in the control bar.
 *  @param[in} glags is a bit mask of GComprisBarFlags.
 */
void		 gc_bar_set (const GComprisBarFlags flags);
/** Hide of show the control bar.
 *  @param[in] hide set to true to hide the bar.
 */
void		 gc_bar_hide (gboolean hide);
/** Specify where in your activity must be set the control bar
 *  @param[in] x is the x coordinate. Set to -1 to keep the default.
 *  @param[in] y is the x coordinate Set to -1 to keep the default.
 *  @param[in] zoom is the zoom factor. Set to -1 to keep the default.
 */
void		 gc_bar_location (int x, int y, double zoom);
/** Pronounce level changing notification
 *  @param[in] level to pronounce, should be < 10
 */
void		 gc_bar_play_level_voice (int level);

/* General */
GooCanvas       *gc_get_canvas(void);
GtkWidget	*gc_get_window(void);

const gchar	*gc_locale_get(void);
void		 gc_locale_set(const gchar *locale);
char		*gc_locale_get_user_default(void);
const gchar	*gc_locale_get_name(const gchar *locale);
const gchar	*gc_locale_get_locale(const gchar *name);
gchar		*gc_locale_short(const gchar *locale);
gchar		*gc_locale_long(const gchar *locale);

guint		 gc_cursor_get();
void		 gc_cursor_set(guint gdk_cursor_type);

typedef void     (*ImageSelectorCallBack)     (gchar* image,
					       void *user_context);
void		 gc_selector_images_start (GcomprisBoard *gcomprisBoard,
					   gchar *dataset,
					   ImageSelectorCallBack imscb,
					   void *user_context);
void		 gc_selector_images_stop (void);

typedef void     (*FileSelectorCallBack)     (gchar *file, gchar *file_type,
					      void *user_context); /* file_type is one string from file_types in the save */
void		 gc_selector_file_load (GcomprisBoard *gcomprisBoard,
					gchar *rootdir, /* Relative to property->user_dir */
					gchar *file_types, /* A Comma separated text explaining the different file types */
					FileSelectorCallBack fscb,
					void *user_contact);
void		 gc_selector_file_save (GcomprisBoard *gcomprisBoard,
					gchar *rootdir, /* Relative to property->user_dir */
					gchar *file_types, /* A Comma separated text explaining the different file types */
					FileSelectorCallBack fscb,
					void *user_context);
void		 gc_selector_file_stop (void);

gchar		*gc_db_get_filename();

/* Dialog box */
typedef void     (*DialogBoxCallBack)     ();
void		 gc_dialog(gchar *str, DialogBoxCallBack dbcb);
void		 gc_dialog_close();

/* Confirm box */
typedef void     (*ConfirmCallBack)     (gboolean answer);

void		 gc_confirm_box (gchar *title,
				 gchar *question_text,
				 gchar *yes_text,
				 gchar *no_text,
				 ConfirmCallBack iscb);

void		 gc_confirm_box_stop (void);

/* Use these instead of the goo_canvas ones for proper fullscreen mousegrab
   handling. */
int		 gc_canvas_item_grab (GooCanvasItem *item, unsigned int event_mask,
				      GdkCursor *cursor, guint32 etime);
void		 gc_canvas_item_ungrab (GooCanvasItem *item, guint32 etime);

/* Use it to tell the teacher where the kid failed */
void		 gc_log_set_comment (GcomprisBoard *gcomprisBoard, gchar *expected, gchar* got);

/* Do not use it if you use the bonus API in your board */
void		 gc_log_end (GcomprisBoard *gcomprisBoard, GCBonusStatusList status);

/* Refresh canvas zoom e.g. after setting zoom setting */
void         gc_update_canvas_zoom();

/* For menu type activity */
GList		*gc_menu_getlist(gchar *section);
GcomprisBoard   *gc_menu_section_get(gchar *section);
GList           *gc_menu_get_boards();

/* Correct timeout delay for activity timings */
gint gc_timing (gint timeout, gint actors_number);

/*=========================================================*/
/* Some global definition to keep a constant look and feel */
/* Boards coders are invited to use them                   */
#define COLOR_TITLE		0x0F0FC0FF
#define COLOR_TEXT_BUTTON       0x0F0FC0FF
#define COLOR_CONTENT		0x0D0DFAFF
#define COLOR_SUBTITLE		0xB00400FF
#define COLOR_SHADOW		0x000000FF

#define FONT_TITLE		"sans 20"
#define FONT_TITLE_FALLBACK	"sans 12"
#define FONT_SUBTITLE		"sans 16"
#define FONT_SUBTITLE_FALLBACK	"sans 12"
#define FONT_CONTENT		"sans 12"
#define FONT_CONTENT_FALLBACK	"sans 12"

#define FONT_BOARD_TINY		"sans 10"
#define FONT_BOARD_SMALL	"sans 12"
#define FONT_BOARD_MEDIUM	"sans 14"
#define FONT_BOARD_BIG		"sans 16"
#define FONT_BOARD_BIG_BOLD	"sans bold 16"
#define FONT_BOARD_FIXED	"fixed 12"
#define FONT_BOARD_TITLE	"sans 20"
#define FONT_BOARD_TITLE_BOLD	"sans bold 20"
#define FONT_BOARD_HUGE		"sans 28"
#define FONT_BOARD_HUGE_BOLD	"sans bold 28"

/*=========================================================*/
// These are gcompris defined cursors
// cursor defines must be over the last gnome cursor defined in gdkcursors.h
#define GCOMPRIS_FIRST_CUSTOM_CURSOR	1000
#define GCOMPRIS_DEFAULT_CURSOR		1001
#define GCOMPRIS_LINE_CURSOR		1003
#define GCOMPRIS_FILLRECT_CURSOR	1004
#define GCOMPRIS_RECT_CURSOR		1005
#define GCOMPRIS_FILLCIRCLE_CURSOR	1006
#define GCOMPRIS_CIRCLE_CURSOR		1007
#define GCOMPRIS_DEL_CURSOR		1008
#define GCOMPRIS_FILL_CURSOR		1009
#define GCOMPRIS_SELECT_CURSOR		1010

#endif
