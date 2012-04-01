#  gcompris - board_list.py
#
# Copyright (C) 2005, 2008 Yves Combe
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, see <http://www.gnu.org/licenses/>.
#

import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
import gobject
from gcompris import gcompris_gettext as _

# Board Management
(
   COLUMN_BOARDICON,
   COLUMN_BOARDNAME
) = range(2)

class Board_list:
  """GCompris Boards List Table"""


  # area is the drawing area for the list
  def __init__(self, db_connect, db_cursor, frame):

      self.frame = frame
      self.cur = db_cursor
      self.con = db_connect

  def init(self):

      # ---------------
      # Boards Management
      # ---------------


      # Create the profiles Combo
      self.profiles_list = gcompris.admin.get_profiles_list()

      if not self.profiles_list:
        return

      # Get default pofile id.
      self.cur.execute('SELECT profile_id FROM informations;')
      self.con.commit()

      self.default_profile_id = self.cur.fetchall()[0][0]

      self.out_dict = self.get_boards_out_by_profile()

      self.difficulty = [1, 6]

      # Main box is vertical
      top_box = gtk.VBox(False, 8)
      top_box.show()
      self.frame.add(top_box)

      box1 = gtk.HBox(False, 8)
      box1.show()

      box2 = gtk.HBox(False, 8)
      box2.show()

      box3 = gtk.VBox(False, 8)
      box3.show()

      top_box.pack_start(box1, False, False, 0)
      top_box.pack_start(box2, True, True, 0)

      box2.pack_end(box3, False, False, 0)

      label = gtk.Label(_('Select a profile:'))
      label.show()
      box1.pack_start(label,False, False, 0)

      combobox = gtk.combo_box_new_text()
      combobox.show()
      box1.pack_start(combobox, False, False, 0)

      for profile in self.profiles_list:
        combobox.append_text(profile.name)
        if profile.profile_id == self.default_profile_id:
          combobox.set_active(self.profiles_list.index(profile))

      self.active_profile = self.profiles_list[combobox.get_active()]

      self.progressbar = gtk.ProgressBar(adjustment=None)
      box1.pack_start(self.progressbar, False, False, 0)

      # Create the table
      sw = gtk.ScrolledWindow()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
#      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
      sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)

      # create tree model
      self.model = self.__create_model()

      # update the combobox
      combobox.connect('changed', self.changed_cb)

      # create tree view
      treeview = gtk.TreeView(self.model)
      treeview.set_rules_hint(True)
      treeview.set_search_column(COLUMN_BOARDNAME)

      sw.add(treeview)

      # Some constants for the layout
      #but_height = hgap * 2
      #but_width  = hgap * 6

      box2.pack_start(sw, True, True, 0)

      sw.show()
      treeview.show()

      # add columns to the tree view
      self.__add_columns(treeview)

      # Add buttons

      self.button_configure = gtk.Button(stock=gtk.STOCK_PREFERENCES)
      self.button_configure.connect("clicked", self.configure_board)
      self.button_configure.show()
      box3.pack_start(self.button_configure, False, False, 0)
      self.button_configure.set_sensitive(False)

      self.button_filter = gtk.Button(_('Filter'))
      self.button_filter.connect("clicked", self.filter_boards)
      self.button_filter.show()
      box3.pack_start(self.button_filter, False, False, 0)

      self.button_select_all = gtk.Button(_('Select all'))
      self.button_select_all.connect("clicked", self.select_all_boards, True)
      self.button_select_all.show()
      box3.pack_start(self.button_select_all, False, False, 0)

      self.button_unselect_all = gtk.Button(_('Unselect all'))
      self.button_unselect_all.connect("clicked", self.select_all_boards, False)
      self.button_unselect_all.show()
      box3.pack_start(self.button_unselect_all, False, False, 0)

      self.button_locales = gtk.Button(_('Locales'))
      self.button_locales.connect("clicked", self.locales)
      self.button_locales.show()
      box3.pack_end(self.button_locales, False, False, 0)

      self.button_locales_sound = gtk.Button(_('Locales sound'))
      self.button_locales_sound.connect("clicked", self.locales_sound)
      self.button_locales_sound.show()
      box3.pack_end(self.button_locales_sound, False, False, 0)

      self.button_login = gtk.Button(_('Login'))
      self.button_login.connect("clicked", self.login_configure)
      self.button_login.show()
      box3.pack_end(self.button_login, False, False, 0)


  def show(self, db_connect, db_cursor):
    self.cur = db_cursor
    self.con = db_connect
    self.frame.show()

  def hide(self):
    self.frame.hide()

  # -------------------
  # Board Management
  # -------------------

  # Add boards in the model
  def get_board_by_name(self, name, list):
    for board in list:
      if (board.section + '/' + board.name) == name:
        return board
    return None

  def get_board_from_menu(self, menu, list):
    if menu.name =='':
      section = ''
    else:
      section = menu.section + '/' + menu.name
    return_list = []
    for board in list:
      if board.section == section:
        return_list.append([section, board])

    return return_list

  def add_boards_in_model(self, model, boards_list):
    root_menu = '/'
    root = self.get_board_by_name(root_menu, boards_list)
    boards_list.remove(root)
    menu_list = [[ None,  root ]]

    list = menu_list[:]
    while len(list) > 0:
      menu = list.pop(0)[1]
      list_board = self.get_board_from_menu(menu, boards_list)
      menu_list = menu_list + list_board

      for board_cell in list_board:
        if board_cell[1].type == 'menu':
          list.append(board_cell)


    row_dict = {}
    self.board_dict = {}
    height = 24

    self.progressbar.show()
    index = 0.0
    for board_cell in menu_list:
      self.board_dict['%s/%s' % (board_cell[1].section,board_cell[1].name)] = board_cell[1]

      self.progressbar.set_fraction(index / len(menu_list))
      index = index + 1
      while gtk.events_pending():
        gtk.main_iteration(False)

      if  board_cell[0] == None:
        row_dict[''] =  \
                     model.append(None,
                                  [self.pixbuf_admin_at_height('administration/tuxplane.svg', height),
                                   _('Main menu') + '\n' + '/',
                                   not board_cell[1].board_id in self.out_dict[self.active_profile.profile_id],
                                   '%s/%s' % (board_cell[1].section,board_cell[1].name), self.pixbuf_configurable(board_cell[1])])

      else:
        row_dict['%s/%s' % (board_cell[1].section,board_cell[1].name)] = \
                         model.append(row_dict[board_cell[1].section],
                                      [self.pixbuf_at_height(board_cell[1].icon_name, height),
                                       _(board_cell[1].title) + '\n' + '%s/%s' % (board_cell[1].section,board_cell[1].name),
                                       not board_cell[1].board_id in self.out_dict[self.active_profile.profile_id],
                                       '%s/%s' % (board_cell[1].section,board_cell[1].name), self.pixbuf_configurable(board_cell[1])])
    self.progressbar.hide()

  def pixbuf_admin_at_height(self, file, height):
    pixbuf = gcompris.utils.load_pixmap(file)
    width = pixbuf.get_width()* height / pixbuf.get_height()
    del pixbuf
    pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(gcompris.DATA_DIR + '/' + file,
                                                  width, height)
    return pixbuf

  def pixbuf_at_height(self, file, height):
    pixbuf = gcompris.utils.load_pixmap(file)
    width = pixbuf.get_width()* height / pixbuf.get_height()
    del pixbuf
    pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(gcompris.DATA_DIR + '/' + file,
                                                  width, height)
    return pixbuf

  def __create_model(self):
    model = gtk.TreeStore(
      gtk.gdk.Pixbuf,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      )

    self.boards_list = gcompris.admin.get_boards_list()

    self.add_boards_in_model(model, self.boards_list)

    return model


  def __add_columns(self, treeview):

    model = treeview.get_model()

    # Render for Board name with icon.
    cell_board_icon = gtk.CellRendererPixbuf()

    cell_board_title = gtk.CellRendererText()

    cell_active_board = gtk.CellRendererToggle()
    cell_active_board.set_property('activatable', True)
    cell_active_board.connect( 'toggled', self.board_acive_cb, model )

    cell_board_configure = gtk.CellRendererPixbuf()

#    columns for Board name
#    column_pref = gtk.TreeViewColumn(_('Conf'))
#    image = gtk.image_new_from_stock(gtk.STOCK_PREFERENCES, gtk.ICON_SIZE_MENU)
#    image.show()
#    column_pref.set_widget(image)

    column_active = gtk.TreeViewColumn(_('Active'))
    column_title = gtk.TreeViewColumn(_('Board title'))
    column_title.pack_start(cell_board_icon, False)
    column_active.pack_start(cell_board_configure, False)
    column_title.pack_start(cell_board_title, True)
    column_active.pack_start(cell_active_board, False)

    treeview.append_column(column_active)
    treeview.append_column(column_title)

    column_title.add_attribute(cell_board_icon, 'pixbuf', 0)
    column_title.add_attribute(cell_board_title, 'text', 1)
    column_active.add_attribute(cell_active_board, 'active', 2)
    column_active.set_attributes(cell_board_configure, stock_id=4)

    treeview.connect("cursor-changed", self.row_selected, model)

  def board_acive_cb(self, cell, path, model):
    model[path][2] = not model[path][2]

    if model[path][2]:
      self.update_parent(model[path].parent)
    else:
      self.check_parent_active(model[path].parent)

    try:
      self.update_child(model[path].iterchildren().next(), model[path][2])
    except StopIteration:
      pass

    self.update_selected(model, path)

  def update_selected(self, model, path):
    if model[path][2]:
      self.cur.execute('DELETE FROM activities_out WHERE board_id=%d AND out_id=%d' % (
        self.board_dict[model[path][3]].board_id,
                                                                                       self.active_profile.profile_id
        ))
    else:
      self.cur.execute('INSERT INTO activities_out (board_id, out_id) VALUES (%d, %d)' % (
        self.board_dict[model[path][3]].board_id,
        self.active_profile.profile_id
        ))

    self.con.commit()

    # update infos
    self.out_dict = self.get_boards_out_by_profile()

    return

  def dict_from_list(self, list):
    dict = {}

    for profile in  self.profiles_list:
      dict[profile.profile_id] = []

    for l in list:
      dict[l[1]].append(l[0])

    return dict

  def changed_cb(self, combobox):
    index = combobox.get_active()
    if(index<0):
      return
    self.active_profile = self.profiles_list[index]

    self.model.foreach(self.update_active)

  def update_active(self, model, path, iter):
    model[path][2] = not self.board_dict[model[path][3]].board_id in self.out_dict[self.active_profile.profile_id]

  def get_boards_out_by_profile(self):
    # Grab the user data
    self.cur.execute('select board_id, out_id from activities_out')

    return self.dict_from_list(self.cur.fetchall())

  def pixbuf_configurable(self, board):
    if board.is_configurable and board.type != "menu":
      return gtk.STOCK_PREFERENCES
    else:
      return None

  def preference_clicked(self, widget, event, board):
    pass

  def row_selected(self, treeview,  model):
    if treeview.get_selection().count_selected_rows() != 1:
      return
    path = model.get_path(treeview.get_selection().get_selected()[1])

    self.selected_board = self.board_dict[model[path][3]]
    if self.selected_board.is_configurable:
      self.button_configure.set_sensitive(True)
    else:
      self.button_configure.set_sensitive(False)

  def configure_board(self, button):
    gcompris.admin.board_config_start(self.selected_board,
                                      self.active_profile)

  def select_all_boards(self, button, Value):
    self.model.foreach(self.update_all, Value)

  def update_all(self, model, path, iter, Value):
    model[path][2] = Value

    self.update_selected( model, path)


  def filter_boards(self, button):
    window = gtk.Window()
    window.set_title(_("Filter Boards difficulty for profile %s") % (self.active_profile.name))
    window.set_border_width(8)
    window.set_default_size(320, 350)
    window.set_transient_for(self.frame.get_toplevel())
    window.set_modal(True)
    window.show()

    button_close = gtk.Button(stock=gtk.STOCK_CLOSE)
    button_close.connect("clicked", self.filter_close, window)
    button_close.show()

    button_apply = gtk.Button(stock=gtk.STOCK_APPLY)
    button_apply.connect("clicked", self.filter_apply)
    button_apply.show()

    main_box = gtk.VBox(False, 8)
    main_box.show()
    window.add(main_box)

    box_bottom = gtk.HBox(False, 0)
    box_bottom.show()
    main_box.pack_end(box_bottom, False, False, 0)

    sep = gtk.HSeparator()
    sep.show()
    main_box.pack_end(sep, False, False, 8)

    box_bottom.pack_end(button_close, False, False, 0)
    box_bottom.pack_start(button_apply, False, False, 0)

    label = gtk.Label()
    label.set_markup(_("<span size='x-large'> Select the difficulty range \nfor profile <b>%s</b></span>") % self.active_profile.name)
    label.show()
    label.set_line_wrap(True)
    label.set_justify(gtk.JUSTIFY_CENTER)
    main_box.pack_start(label, False, False, 0)

    symbols_box = gtk.HBox(False, 0)
    symbols_box.show()
    main_box.pack_start(symbols_box, False, False, 0)

    arrows_box = gtk.HBox(False, 0)
    arrows_box.show()
    main_box.pack_start(arrows_box, False, False, 0)

    self.stars = {}

    for i in range(1,7):
      box = gtk.VBox(False, 8)
      box.show()
      symbols_box.pack_start(box, True, False, 0)
      self.stars[i] = gtk.Image()
      self.stars[i].set_from_pixbuf(gcompris.utils.load_pixmap('administration/difficulty%d.svg' % (i)))
      self.stars[i].show()
      box.pack_start(self.stars[i], False, False, 0)
      i_label = gtk.Label()
      i_label.set_markup("<span size='x-large'><b>%d</b></span>" % i)
      i_label.show()
      box.pack_start(i_label, False, False, 0)

    self.arrows = {}

    self.arrows[1] = self.create_arrow_button(gtk.ARROW_LEFT, gtk.SHADOW_IN)
    self.arrows[2] = self.create_arrow_button(gtk.ARROW_RIGHT, gtk.SHADOW_IN)
    self.arrows[3] = self.create_arrow_button(gtk.ARROW_LEFT, gtk.SHADOW_IN)
    self.arrows[4] = self.create_arrow_button(gtk.ARROW_RIGHT, gtk.SHADOW_IN)

    arrows_box.pack_start(self.arrows[1], False, False, 3)
    arrows_box.pack_start(self.arrows[2], False, False, 3)
    arrows_box.pack_end(self.arrows[4], False, False, 3)
    arrows_box.pack_end(self.arrows[3], False, False, 3)

    for i in range (1,5):
      self.arrows[i].connect("clicked", self.arrow_clicked, i)

    self.update_arrows_active()

  def filter_close(self, button, window):
    window.destroy()

  def create_arrow_button(self, arrow_type, shadow_type):
    button = gtk.Button();
    arrow = gtk.Arrow(arrow_type, shadow_type);
    button.add(arrow)
    button.show()
    arrow.show()
    return button

  def update_arrows_active(self):
    if self.difficulty[0] == 1:
      self.arrows[1].set_sensitive(False)
    else:
      self.arrows[1].set_sensitive(True)

    if self.difficulty[1] == 6:
      self.arrows[4].set_sensitive(False)
    else:
      self.arrows[4].set_sensitive(True)

    if self.difficulty[1] == 1:
      self.arrows[3].set_sensitive(False)
    else:
      self.arrows[3].set_sensitive(True)

    if self.difficulty[0] == 6:
      self.arrows[2].set_sensitive(False)
    else:
      self.arrows[2].set_sensitive(True)

    if self.difficulty[0] == self.difficulty[1]:
      self.arrows[2].set_sensitive(False)
      self.arrows[3].set_sensitive(False)

    for i in range(1,7):
      if i in range( self.difficulty[0], self.difficulty[1]+1):
        self.stars[i].set_sensitive(True)
      else:
        self.stars[i].set_sensitive(False)

  def arrow_clicked(self, arrow, i):
    if i < 3:
      value = 0
      limits = [1, self.difficulty[1]]
    else:
      value = 1
      limits = [self.difficulty[0], 6]

    if (i % 2 == 0):
      if self.difficulty[value] == limits[1]:
        return
      else:
        self.difficulty[value] = self.difficulty[value] + 1
    else:
      if self.difficulty[value] == limits[0]:
        return
      else:
        self.difficulty[value] = self.difficulty[value] - 1

    self.update_arrows_active()

  def filter_apply(self, button):
    self.model.foreach(self.blank)
    self.model.foreach(self.board_filter)

  # Apply the filter as asked.
  def board_filter(self,  model, path, iter):
    if self.board_dict[model[path][3]].type != "menu":
      model[path][2] = ( eval(self.board_dict[model[path][3]].difficulty) \
                         in range( self.difficulty[0],
                                   self.difficulty[1]+1))

    if model[path][2]:
      self.update_parent(model[path].parent)

    self.update_selected( model, path)

  # toggled off.
  def blank(self,  model, path, iter):
    model[path][2] = False

  # This function toggles parents when a child is toggled.
  def update_parent( self, row):
    if row == None:
      return

    if row[2]:
      return

    row[2] = True

    self.update_parent( row.parent)

    self.update_selected(row.model, row.path)

  # This function toggles parents off when all childs are toggled off.
  def check_parent_active( self, row):
    if row == None:
      return

    # should never appends
    if not row[2]:
      return

    child = row.iterchildren().next()
    select = False

    while child != None:
      if child[2]:
        select = True
        break
      child = child.next

    if not select:
      row[2] = False
      self.check_parent_active(row.parent)
      self.update_selected(row.model, row.path)

  # This function recursively update childs
  def update_child( self, row, value):
    if row[2] == value:
      return

    row[2] = value

    try:
      self.update_child(row.iterchildren().next(), value)
    except StopIteration:
      pass

    if row.next != None:
      self.update_child(row.next, value)

    self.update_selected(row.model, row.path)

  ########################################################
  ########################################################

  def locales(self, button):
    conf_locales = self.get_configured(self.active_profile, 'locale', 'NULL')
    bconf = gcompris.configuration_window ( \
      _('<b>{config}</b> configuration\n for profile <b>{profile}</b>').format(config='Locale',
                                                                               profile=self.active_profile.name),
      self.ok_callback
      )

    gcompris.combo_locales(bconf, conf_locales)

  def locales_sound(self, button):

    conf_locales = self.get_configured(self.active_profile,
                                       'locale_sound', 'NULL')
    bconf = gcompris.configuration_window ( \
      _('<b>{config}</b> configuration\n for profile <b>{profile}</b>').format(config='Locale sound',
                                                                               profile=self.active_profile.name),
      self.ok_callback
      )

    gcompris.combo_locales_asset( bconf,
                                  _("Select sound locale"),
                                  conf_locales,
                                  "voices/$LOCALE/colors/red.ogg" )


  def ok_callback(self, dict):

    if not dict:
      return

    for key, value in dict.iteritems():
      if key in self.already_conf:
        req = 'UPDATE board_profile_conf SET conf_value=\'%s\' WHERE profile_id=%d AND board_id=-1 AND conf_key=\'%s\'' % (value, self.active_profile.profile_id, key)
      else:
        req = 'INSERT INTO board_profile_conf (profile_id, board_id, conf_key, conf_value) VALUES (%d, -1, \'%s\', \'%s\')' % (self.active_profile.profile_id, key, value)

      self.cur.execute(req)
      self.con.commit()

  def get_configured(self, profile, key, if_not):
    self.cur.execute('select conf_value from board_profile_conf where profile_id=%d and board_id=-1 and conf_key =\'%s\' '
                     % (profile.profile_id, key))

    value = self.cur.fetchall()

    try:
      s = self.already_conf
    except:
      self.already_conf = []

    if len(value)==1:
      self.already_conf.append(key)
      return value[0][0]
    else:
      return if_not


  def login_configure(self, button):

    board_log = self.get_board_by_name('/login/login', self.boards_list)
    gcompris.admin.board_config_start(board_log, self.active_profile)

