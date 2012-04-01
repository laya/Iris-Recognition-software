/*
 * GooCanvas Demo. Copyright (C) 2006 Damon Chaplin.
 * Released under the GNU LGPL license. See COPYING for details.
 *
 * goocanvassvg.c - a simple svg item.
 */
#include "goocanvas.h"
#include "goocanvassvg.h"

enum {
  PROP_0,

  /* Convenience properties. */
  PROP_SVGHANDLE,
  PROP_SVG_ID,
  PROP_AUTOCROP,
};

static void goo_canvas_svg_finalize     (GObject            *object);
static void goo_canvas_svg_set_property (GObject            *object,
					 guint               param_id,
					 const GValue       *value,
					 GParamSpec         *pspec);


/* Use the GLib convenience macro to define the type. GooCanvasSvg is the
   class struct, goo_canvas_svg is the function prefix, and our class is a
   subclass of GOO_TYPE_CANVAS_ITEM_SIMPLE. */
G_DEFINE_TYPE (GooCanvasSvg, goo_canvas_svg, GOO_TYPE_CANVAS_ITEM_SIMPLE)

static void
goo_canvas_svg_install_common_properties (GObjectClass *gobject_class)
{
  g_object_class_install_property (gobject_class, PROP_SVGHANDLE,
				   g_param_spec_object ("svg-handle",
							"Rsvg Handle",
							"The RsvgHandle to display",
							RSVG_TYPE_HANDLE,
							G_PARAM_WRITABLE));
  /* Convenience properties - writable only. */
  g_object_class_install_property (gobject_class, PROP_SVG_ID,
				   g_param_spec_string ("svg-id",
							"SVG ID",
							"The svg id to display, NULL to display all",
							NULL,
							G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, PROP_AUTOCROP,
				   g_param_spec_boolean ("autocrop",
							 "Auto Crop",
							 "Set to True to crop the image, default is False",
							 FALSE,
							 G_PARAM_WRITABLE));

}

/* Return a new surface with the item in src_surface being
 * assigned to 0,0 coordinate.
 */
static cairo_surface_t*
_autocrop(cairo_surface_t *src_surface,
	  double x1, double x2, double y1, double y2)
{
  cairo_surface_t* dst_surface =
    cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
				x2 - x1 + 1,
				y2 - y1 + 1);

  unsigned char* src_data = cairo_image_surface_get_data(src_surface);
  unsigned char* dst_data = cairo_image_surface_get_data(dst_surface);

  int src_stride = cairo_image_surface_get_stride(src_surface);
  int dst_stride = cairo_image_surface_get_stride(dst_surface);

  int x;
  for (x = x1; x <= x2; x++)
    {
      int y;
      for (y = y1; y <= y2; y++)
	{
	  guint32 point = *(guint32*)&src_data[y*src_stride + x*4];
	  *(guint32*)&dst_data[((y-(int)y1)*dst_stride)
			    + (x*4 - (int)x1*4)] = point;
	}
    }

  return dst_surface;
}

static void _init_surface(GooCanvasSvg *canvas_svg,
			  RsvgHandle *svg_handle, double zoom)
{
  g_assert(svg_handle);
  RsvgDimensionData dimension_data_max;
  rsvg_handle_get_dimensions (svg_handle, &dimension_data_max);

  RsvgDimensionData dimension_data;
  rsvg_handle_get_dimensions_sub (svg_handle, &dimension_data,
				  canvas_svg->id);
  canvas_svg->width = dimension_data.width * zoom;
  canvas_svg->height = dimension_data.height * zoom;

  RsvgPositionData position_data;
  rsvg_handle_get_position_sub (svg_handle, &position_data,
				canvas_svg->id);

  canvas_svg->svg_handle = svg_handle;
  g_object_ref(svg_handle);

  if (canvas_svg->pattern)
      cairo_pattern_destroy(canvas_svg->pattern);
  canvas_svg->pattern = NULL;

  if (canvas_svg->cr)
    cairo_destroy(canvas_svg->cr);
  canvas_svg->cr = NULL;

  cairo_surface_t* cst =
    cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
				dimension_data_max.width,
				dimension_data_max.height);
  canvas_svg->cr = cairo_create (cst);
  rsvg_handle_render_cairo_sub (svg_handle, canvas_svg->cr,
				canvas_svg->id);

  /* Keep the real coordinates */
  canvas_svg->x1 = position_data.x;
  canvas_svg->x2 = position_data.x + dimension_data.width;
  canvas_svg->y1 = position_data.y;
  canvas_svg->y2 = position_data.y + dimension_data.height;

  if(dimension_data.width > 0 && canvas_svg->autocrop)
    {
      canvas_svg->pattern =				\
	cairo_pattern_create_for_surface ( _autocrop(cst,
						     canvas_svg->x1, canvas_svg->x2,
						     canvas_svg->y1, canvas_svg->y2) );
      canvas_svg->x2 = canvas_svg->x2 - canvas_svg->x1;
      canvas_svg->y2 = canvas_svg->y2 - canvas_svg->y1;
      canvas_svg->x1 = 0;
      canvas_svg->y1 = 0;
    }
  else
    canvas_svg->pattern = cairo_pattern_create_for_surface (cst);

  cairo_surface_destroy(cst);
}

/* The standard object initialization function. */
static void
goo_canvas_svg_init (GooCanvasSvg *canvas_svg)
{
  canvas_svg->width = 0.0;
  canvas_svg->height = 0.0;
  canvas_svg->x1 = 0.0;
  canvas_svg->y1 = 0.0;
  canvas_svg->x2 = 0.0;
  canvas_svg->y2 = 0.0;
  canvas_svg->id = NULL;
  canvas_svg->cr = NULL;
  canvas_svg->pattern = NULL;
  canvas_svg->autocrop = FALSE;
}


/* The convenience function to create new items. This should start with a
   parent argument and end with a variable list of object properties to fit
   in with the standard canvas items. */
GooCanvasItem*
goo_canvas_svg_new (GooCanvasItem      *parent,
                    RsvgHandle         *svg_handle,
                    ...)
{
  GooCanvasItem *item;
  GooCanvasSvg *canvas_svg;
  const char *first_property;
  va_list var_args;

  item = g_object_new (GOO_TYPE_CANVAS_SVG, NULL);

  va_start (var_args, svg_handle);
  first_property = va_arg (var_args, char*);
  if (first_property)
    g_object_set_valist ((GObject*) item, first_property, var_args);
  va_end (var_args);

  canvas_svg = (GooCanvasSvg*) item;
  if(svg_handle)
    _init_surface(canvas_svg, svg_handle, 1.0);

  if (parent)
    {
      goo_canvas_item_add_child (parent, item, -1);
      g_object_unref (item);
    }

  return item;
}


/* The update method. This is called when the canvas is initially shown and
   also whenever the object is updated and needs to change its size and/or
   shape. It should calculate its new bounds in its own coordinate space,
   storing them in simple->bounds. */
static void
goo_canvas_svg_update  (GooCanvasItemSimple *simple,
			cairo_t             *cr)
{
  GooCanvasSvg *canvas_svg = (GooCanvasSvg*) simple;

  /* Compute the new bounds. */
  simple->bounds.x1 = canvas_svg->x1;
  simple->bounds.y1 = canvas_svg->y1;
  simple->bounds.x2 = canvas_svg->x2;
  simple->bounds.y2 = canvas_svg->y2;
}


/* The paint method. This should draw the item on the given cairo_t, using
   the item's own coordinate space. */
static void
goo_canvas_svg_paint (GooCanvasItemSimple   *simple,
		     cairo_t               *cr,
		     const GooCanvasBounds *bounds)
{
  GooCanvasSvg *canvas_svg = (GooCanvasSvg*) simple;

  if(canvas_svg->pattern)
    {
      cairo_set_source (cr, canvas_svg->pattern);
      cairo_paint (cr);
    }
}


static void
goo_canvas_svg_finalize (GObject *object)
{
  GooCanvasSvg *canvas_svg = (GooCanvasSvg*) object;

  if (canvas_svg->id)
    g_free(canvas_svg->id);
  canvas_svg->id = NULL;

  if (canvas_svg->pattern)
      cairo_pattern_destroy(canvas_svg->pattern);
  canvas_svg->pattern = NULL;

  if (canvas_svg->cr)
    cairo_destroy(canvas_svg->cr);
  canvas_svg->cr = NULL;

  if (canvas_svg->svg_handle)
    g_object_unref (canvas_svg->svg_handle);
  canvas_svg->svg_handle = NULL;

  G_OBJECT_CLASS (goo_canvas_svg_parent_class)->finalize (object);
}


static void
goo_canvas_svg_set_common_property (GObject              *object,
				    GooCanvasSvg         *canvas_svg,
				    guint                 prop_id,
				    const GValue         *value,
				    GParamSpec           *pspec)
{
  RsvgHandle *svg_handle;

  switch (prop_id)
    {
    case PROP_SVGHANDLE:
      svg_handle = g_value_get_object (value);
      if(canvas_svg->svg_handle)
      g_object_unref (canvas_svg->svg_handle);
      _init_surface(canvas_svg, svg_handle, 1.0);
      break;
    case PROP_SVG_ID:
      if(canvas_svg->id)
	g_free(canvas_svg->id);
      if (!g_value_get_string (value))
	canvas_svg->id = NULL;
      else
        canvas_svg->id = g_value_dup_string(value);
      if (canvas_svg->svg_handle)
	_init_surface(canvas_svg, canvas_svg->svg_handle, 1.0);
      break;
    case PROP_AUTOCROP:
      canvas_svg->autocrop = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
goo_canvas_svg_set_property (GObject              *object,
                             guint                 prop_id,
                             const GValue         *value,
                             GParamSpec           *pspec)
{
  GooCanvasItemSimple *simple = (GooCanvasItemSimple*) object;
  GooCanvasSvg *image = (GooCanvasSvg*) object;

  if (simple->model)
    {
      g_warning ("Can't set property of a canvas item with a model - set the model property instead");
      return;
    }

  goo_canvas_svg_set_common_property (object, image, prop_id,
                                      value, pspec);
  goo_canvas_item_simple_changed (simple, TRUE);
}



/* Hit detection. This should check if the given coordinate (in the item's
   coordinate space) is within the item. If it is it should return TRUE,
   otherwise it should return FALSE. */
static gboolean
goo_canvas_svg_is_item_at (GooCanvasItemSimple *simple,
                           gdouble              x,
                           gdouble              y,
                           cairo_t             *cr,
                           gboolean             is_pointer_event)
{
  GooCanvasSvg *canvas_svg = (GooCanvasSvg*) simple;

  if ( x < canvas_svg->x1 || x > canvas_svg->x2
       || y < canvas_svg->y1 || y > canvas_svg->y2 )
    return FALSE;

  return TRUE;
}

/* The class initialization function. Here we set the class' update(), paint()
   and is_item_at() methods. */
static void
goo_canvas_svg_class_init (GooCanvasSvgClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass*) klass;
  GooCanvasItemSimpleClass *simple_class = (GooCanvasItemSimpleClass*) klass;

  gobject_class->set_property = goo_canvas_svg_set_property;

  gobject_class->finalize            = goo_canvas_svg_finalize;
  simple_class->simple_update        = goo_canvas_svg_update;
  simple_class->simple_paint         = goo_canvas_svg_paint;
  simple_class->simple_is_item_at    = goo_canvas_svg_is_item_at;

  goo_canvas_svg_install_common_properties (gobject_class);

}


