#! /usr/bin/env python

#
# "$Id: tree.py 158 2006-01-11 08:00:45Z andreasheld $"
#
# Tree widget test program for pyFLTK the Python bindings
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


"""
Implements a tree widget for pyfltk version 1.1.x

Contains a demo which can be viewed by executing this file

Written Dec 2005 by David McNab <david@rebirthing.co.nz>
Released under the terms of the GNU Lesser General Public License.

No warrantee, yada yada.
"""

import traceback

import fltk

item_height = 20

item_indent = 25

item_box_indent = 10
item_box_width = 9
class Node:
    """
    a tree node contains a payload, a state, a list of children
    and a ref to its parent

    If you want to intercept node open/close events, you
    should subclass this
    """
    def __init__(self, tree, parent, title, payload=None, children=None):
        """
        create a tree node
    
        title is the text to be displayed
    
        payload is an object which this node contains, can be any
        python object
    
        children is a list of child nodes (optional)
        """
        self.tree = tree
        self.parent = parent
        self.level = parent.level + 1
        self.title = title
        self.payload = payload
        if children == None:
            children = []
        self.children = children
        self.isopen = False
    
    def append(self, title, data=None, refresh=True):
        """
        adds data to root node
    
        Arguments:
            - title - text to show in tree display
            - data - an optional data payload
            - refresh - default True - whether to refresh the
              tree after adding this node
        """
        node = self.__class__(self.tree, self, title, data)
        self.children.append(node)
    
        self.tree._nodeToShow = node
        self.tree.refresh()
    
        return node
    
    def refresh(self):
        """
        draws this node, and children (if any, and if open)
        """
        tree = self.tree
    
        line = " " * (self.level * 4 + 1)
    
        if self.children:
            if self.isopen:
                line += "[-] "
            else:
                line += "[+] "
        
        line += self.title
        
        tree.visibleNodes.append(self)
        self.treeIdx = tree.nitems
    
        # if this node was selected, mark it
        if tree._nodeToShow == self:
            tree._nodeToShowIdx = tree.nitems
    
        tree.nitems += 1
        tree.add(line)
    
        if self.isopen:
            for child in self.children:
                child.refresh()
    
    def open(self):
        """
        opens this node
    
        Invokes the on_open handler, if any
        """
        self.isopen = True
        self.on_open()
        self.tree.refresh()
    
    def close(self):
        """
        closes this node
        
        Invokes the on_close handler, if any
        """
        self.isopen = False
        self.on_close()
        self.tree.refresh()
    
    def toggle(self):
        """
        toggles open/close state
        """
        if self.isopen:
            self.close()
        else:
            self.open()
    
    
    def promote(self):
        """
        promotes this node up one level in hierarchy
        """
        parent = self.parent
    
        if parent == self.tree:
            # already at top - bail
            return
    
        grandparent = parent.parent
    
        parentIdx = grandparent.children.index(parent)
        selfIdx = parent.children.index(self)
    
        parent.children.remove(self)
        grandparent.children.insert(parentIdx, self)
        self.parent = grandparent
    
        self._changeLevel(-1)
    
        self.tree._nodeToShow = self
        self.tree.refresh()
    
        #self.tree.value(self.tree.visibleNodes.index(self) + 1)
    
    def demote(self):
        """
        demotest this item, if possible
        """
        selidx = self.tree.value()
    
        siblings = self.parent.children
        
        selfidx = siblings.index(self)
    
        if selfidx == 0:
            # already subordinate to previous visible node
            return
    
        siblings.remove(self)
    
        prevsibling = siblings[selfidx-1]
        if not prevsibling.isopen:
            prevsibling.isopen = True
    
        prevsibling.children.append(self)
    
        self.parent = prevsibling
        self._changeLevel(1)
    
        self.tree._nodeToShow = self
        self.tree.refresh()
        
        #self.tree.value(selidx)
    
    def moveup(self):
        """
        moves this node up one
        """
        selidx = self.tree.value()
    
        siblings = self.parent.children
        selfidx = siblings.index(self)
    
        if selfidx == 0:
            # already top child
            return
    
        prevnode = siblings[selfidx-1]    
        siblings[selfidx-1] = self
        siblings[selfidx] = prevnode
    
        self.tree._nodeToShow = self
        self.tree.refresh()
        
        #self.tree.value(selidx-1)
    
    def movedown(self):
        """
        moves this node down one
        """
        selidx = self.tree.value()
    
        siblings = self.parent.children
        selfidx = siblings.index(self)
    
        if selfidx >= len(siblings)-1:
            # already top child
            return
    
        nextnode = siblings[selfidx+1]    
        siblings[selfidx+1] = self
        siblings[selfidx] = nextnode
    
        self.tree._nodeToShow = self
        self.tree.refresh()
        
        #self.tree.value(selidx+1)
    
    def cut(self):
        
        self.parent.children.remove(self)
        self.tree.refresh()
        return self
    
    def _changeLevel(self, diff=0):
        
        self.level += diff
        for child in self.children:
            child._changeLevel(diff)
    
    def _setLevel(self, level):
        
        self.level = level
        for child in self.children:
            child._setLevel(level)
    
    def on_open(self):
        """
        handler for when this node is opened
    
        You might want to use this, say, when using
        the tree to browse a large hierarchy such as
        a filesystem
    
        Your handler should either execute the .append() method,
        or manipulate the .children list
    
        Override if needed
        """
        print("on_open: not overridden")
    
    def on_close(self):
        """
        handler for when this node is closed
        
        You might want to use this, say, when using
        the tree to browse a large hierarchy such as
        a filesystem
    
        Your handler should either execute the .append() method,
        or manipulate the .children list
    
        Typically, you will want to do::
            
            self.children = []
    
        Override if needed
        """
        print("on_close: not overridden")
    

class Fl_Tree(fltk.Fl_Hold_Browser):
    """
    Implements a tree widget

    If you want handlers for node open/close,
    you should subclass this class, and override
    the 'nodeClass' attribute
    """
    nodeClass = Node
    
    def __init__(self, x, y, w, h, label=0):
        """
        Create the tree widget, initially empty
    
        The label will be the text of the root node
        """
        fltk.Fl_Hold_Browser.__init__(self, x, y, w, h, label)
    
        self.children = []
    
        self.widget = self
        self.isopen = True
    
        self.visibleNodes = []
        self.level = -1
    
        self._nodeToShow = None
        self._nodeToShowIdx = -1
    
        # add the box deco
        self.box(fltk.FL_DOWN_BOX)
    
        self.callback(self._on_click)
    
    def handle(self, evid):
    
        ret = fltk.Fl_Hold_Browser.handle(self, evid)
    
        return ret
    
    def append(self, title, data = None):
        """
        adds data to root node
        """
        node = self.nodeClass(self, self, title, data)
    
        self.children.append(node)
    
        self.refresh()
    
        return node
    
    def refresh(self):
        """
        redraws all the contents of this tree
        """
        self.clear()
        
        # enumerate out all the children, and children's children
        self.nitems = 0
        self._nodeToShowIdx = -1
    
        self.visibleNodes = []
        for child in self.children:
            child.refresh()
    
        if self._nodeToShowIdx >= 0:
            self.value(self._nodeToShowIdx+1)
    
        self._nodeToShowIdx = -1
        self._nodeToShow = None
    
    def _on_click(self, ev):
        
        selidx = self.value()
        
        if selidx <= 0:
            return
        thisidx = selidx - 1
    
        node = self.visibleNodes[thisidx]
    
        x = fltk.Fl.event_x() - self.x()
    
        xMin = (node.level) * 16 + 8
        xMax = xMin + 16
    
        if x >= xMin and x <= xMax:
            node.toggle()
            self.value(selidx)
        else:
            self.on_select(node)
    
    def on_select(self, node):
        """
        override in callbacks
        """
    def valuenode(self):
        
        idx = self.value()
        
        if idx <= 0:
            return None
        
        idx -= 1
    
        return self.visibleNodes[idx]
    
    def cut(self):
        """
        does a cut of selected node
        """
        node = self.valuenode()
        if node is None:
            return None
        
        node.parent.children.remove(node)
        self.refresh()
        return node
    
    def paste(self, node):
        """
        does a paste of selected node
        """
        parent = self.valuenode()
        if parent is None:
            return None
        
        parent.children.append(node)
        node._setLevel(parent.level+1)
        self.refresh()
    

def demo():
    """
    runs a small demo program
    """
    xWin = 200
    yWin = 200
    wWin = 300
    hWin = 400

    xTree = 20
    yTree = 20
    wTree = 200
    hTree = hWin - 2 * yTree

    xButs = xTree + wTree + 20

    class MyTree(Fl_Tree):
        """
        """
        def on_select(self, node):
            print(f"on_select: node={node.title}")
        
    print("creating window")
    win = fltk.Fl_Window(xWin, yWin, wWin, hWin, "Fl_Tree demo")

    print("adding tree")
    tree = MyTree(xTree, yTree, wTree, hTree, "something")
    tree.align(fltk.FL_ALIGN_TOP)

    def on_promote(ev):
        node = tree.valuenode()
        if node:
            print(f"promote: {node.title}")
            node.promote()

    def on_demote(ev):
        node = tree.valuenode()
        if node:
            print(f"demote: {node.title}")
            node.demote()

    def on_moveup(ev):
        node = tree.valuenode()
        if node:
            print(f"moveup: {node.title}")
            node.moveup()

    def on_movedown(ev):
        node = tree.valuenode()
        if node:
            print(f"movedown: {node.title}")
            node.movedown()

    but_promote = fltk.Fl_Button(xButs, 20, 20, 20, "@<-")
    but_promote.callback(on_promote)

    but_demote = fltk.Fl_Button(xButs, 50, 20, 20, "@->")
    but_demote.callback(on_demote)

    but_moveup = fltk.Fl_Button(xButs, 80, 20, 20, "up")
    but_moveup.callback(on_moveup)

    but_movedown = fltk.Fl_Button(xButs, 110, 20, 20, "dn")
    but_movedown.callback(on_movedown)


    print("ending window")
    win.end()

    print("showing window")
    win.show()

    # add stuff to root node
    if 1:
        for i in range(3):
            node = tree.append(f"item-{i}")
            if 1:
                for j in range(3):
                    subnode = node.append(f"item-{i}{j}")
                    if 0:
                        for k in range(2):
                            subnode.append(f"item-{i}{j}{k}")
    
    print("entering main loop")
    fltk.Fl.run()

if __name__ == '__main__':
    demo()


