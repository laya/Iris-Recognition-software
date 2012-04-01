#  gcompris - algorithm
#
# Copyright (C) 2004, 2008 Christof Petig and Ingo Konrad
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
import gcompris.bonus
import gcompris.score
import gcompris.sound
import gtk
import gtk.gdk
import random

def algo(index): return index %4
def algo1(index): return index %3
def algo2(index): return index %5
def algo3(i):
  if i %6 > 2:
   return 2 - i %3
  else:
   return i %3

class Gcompris_algorithm:
  """The algorithm activity"""

  def paint_image (self, i, x2, y2):


    return goocanvas.Image(parent = self.rootitem,
                            pixbuf = self.pixlist[i],
                            x = self.leftx+x2*self.distance,
                            y = y2
                            )

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.anzahl = 8
    self.algos = [algo, algo1, algo2, algo3]
    self.rootitem = None
    self.distance = 80
    self.leftx = 90

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;

  def start(self):
    gcompris.bar_set (0)
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "algorithm/scenery5_background.png")
    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=5
    self.gcomprisBoard.maxlevel = 1

    self.symbollist = ["algorithm/apple.png",
                       "algorithm/strawberry.png",
                       "algorithm/peer.png",
                       "algorithm/football.png",
                       "algorithm/cerise.png",
                       "algorithm/egg.png",
                       "algorithm/glass.png",
                       "algorithm/eggpot.png"]
    self.pixlist = []
    for i in range (len(self.symbollist)):
     pixbuf2 = gcompris.utils.load_pixmap(self.symbollist [i])
     h2 = 60
     w2 = pixbuf2.get_width()*h2/pixbuf2.get_height()
     self.pixlist.append (pixbuf2.scale_simple(w2, h2, gtk.gdk.
      INTERP_BILINEAR))
     del pixbuf2
    self.display_current_level()

  def end(self):
    self.cleanup()

  def ok(self):
    pass

  # Called by gcompris core
  def pause(self, pause):

    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(pause == 0):
      self.increment_level()
      self.gamewon = 0

    return

  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------


  def cleanup(self):

    # Remove the root item removes all the others inside it
    if self.rootitem != None:
     self.rootitem.remove()
     self.rootitem = None
     gcompris.score.end()

  def display_current_level(self):
    self.cleanup()
    gcompris.score.start(gcompris.score.STYLE_NOTE, 570, 305,
     self.gcomprisBoard.number_of_sublevel)
    gcompris.bar_set_level(self.gcomprisBoard)
    gcompris.score.set(self.gcomprisBoard.sublevel)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

    # Display our list of items
    for i in range(len(self.symbollist)):
     s = self.paint_image(i ,i ,390)
     s.connect ("button_press_event", self.apple_click, i)
     gcompris.utils.item_focus_init(s, None)

    # Display the algorithm
    self.algo = random.choice(self.algos)

    # Create a uniq list of index in random order
    self.random_index = []
    self.selector  = range(self.anzahl)
    for i in range(self.anzahl):
      j=random.randrange(len(self.selector))
      self.random_index.append(self.selector[j])
      self.selector.pop(j)

    # Display what to search
    for i in range(len(self.symbollist)):
     self.paint_image(self.random_index[self.algo(i)], i, 45)

    # Display the uncomplete user area
    self.random_index = []

    for i in range(self.anzahl):
     self.random_index.append(random.randrange(len(self.symbollist)))

    for i in range(5):
     self.paint_image(self.random_index[self.algo(i)], i, 147)

    self.place = 5
    self.paint_qm ()

  def paint_qm (self):
    self.qm = goocanvas.Text(parent = self.rootitem,
                             text = "?",
                             x = self.place*self.distance+30+self.leftx,
                             y = 165,
                             fill_color_rgba = 0x000000ffL,
                             font = gcompris.skin.get_font("gcompris/board/huge bold"))

  def key_press(self, keyval, commit_str, preedit_str):
    return False

  def set_level(self, level):
    self.gcomprisBoard.level=level;
    self.gcomprisBoard.sublevel=1;
    self.cleanup()
    self.display_current_level()

  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel):
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

    self.display_current_level()
    return 1


  def apple_click (self, widget, target, event=None, index=0):
    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
     if index == self.random_index[self.algo(self.place)]:
      gcompris.sound.play_ogg("sounds/bleep.wav")
      self.qm.remove()
      self.paint_image(index, self.place, 147)
      self.place +=1
      if self.place == self.anzahl:
        gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)
        return
      self.paint_qm ()
     else:
       gcompris.sound.play_ogg("sounds/brick.wav")



