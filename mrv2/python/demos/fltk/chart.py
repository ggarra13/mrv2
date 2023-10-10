#
# "$Id: chart.py 28 2003-07-16 20:00:27Z andreasheld $"
#
# Charts test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
#
# FLTK copyright 1998-1999 by Bill Spitzak and others.
# pyFLTK copyright 2003 by Andreas Held and others.
#
# This library is free software you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License, version 2.0 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to "pyfltk-user@lists.sourceforge.net".
#

from fltk14 import *
import sys

# Note:  the child windows are assigned to the charts list
#        for the program to exit correctly, the Python shadows
#        must be deleted.  this is accomplished by the charts=None
#        in the exit function
charts =[]

def cb_OK(ptr):
        global charts
        charts = None  ### <- this allows the program to exit
        ptr.parent().hide()

graphWidth = 150
graphHeight = 100
border = 30
numRows = 3
numCols = 3
buttonWidth = 60
buttonHeight = 30


# ugly, hacked coordinates
x1 = border
x2 = (border*2)+graphWidth
x3 = (border+graphWidth)*2+border
y1 = border
y2 = border*2+graphHeight
y3 = border*3+graphHeight*2
chartInfo = [ \
	(FL_BAR_CHART,"FL_BAR_CHART", x1, y1), 
	(FL_HORBAR_CHART,"FL_HORBAR_CHART", x2, y1),
	(FL_LINE_CHART,"FL_LINE_CHART", x3, y1),
	(FL_SPIKE_CHART,"FL_SPIKE_CHART", x1, y2),
	(FL_PIE_CHART,"FL_PIE_CHART", x2, y2),
	(FL_SPECIALPIE_CHART,"FL_SPECIALPIE_CHART", x3, y2),
	(FL_FILL_CHART,"FL_FILL_CHART", x1, y3) ]


foo_window = Fl_Window(100, 100, border+numCols*(border+graphWidth), 
				numRows*(border+graphHeight)+border+buttonHeight+border)

for chartParams in 	chartInfo:
	chart = Fl_Chart(chartParams[2], chartParams[3], 
				graphWidth, graphHeight, chartParams[1])
	chart.type(chartParams[0])
	chart.add(2, "Win 3.1", 0)
	chart.add(3, "Dos", 1)
	chart.add(5, "NT", 3)
	chart.add(10, "Linux", 2)
	charts.append(chart)

okButton = Fl_Return_Button( (numCols-1)*(graphWidth+border), 
		numRows*(graphHeight+border)+border, buttonWidth, buttonHeight, "OK")
okButton.callback(cb_OK)

foo_window.end()
foo_window.show()


