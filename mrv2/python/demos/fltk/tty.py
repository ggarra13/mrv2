#
# "$Id: adjuster.py 35 2003-09-29 21:39:48Z andreasheld $"
#
# Adjuster test program for pyFLTK the Python bindings
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
import sys, os, platform

#
# fltk-ttys - Open several processes, display their output in fltk widgets
# Greg Ercolano 02/21/2005 1.00
#

# Globals
G_disp = [0,0,0];     # one display per child
G_buff = [0,0,0];     # one buffer per child
G_outfd = [0,0,0];    # read pipe for childs stderr, one per child
G_pids = [0,0,0]      # pid for each child

# Start child process, makes a read pipe to its stderr
def start_child(t):
    global G_pids
    #int out[2]; pipe(out);
    out = os.pipe()
    G_pids[t] = os.fork()
    print("pid = ", G_pids[t])
    #G_pids[t] = -1
    if G_pids[t] == -1:
        # error
        os.close(out[0])
        os.close(out[1])
    elif G_pids[t] == 0:
        # child
        os.close(out[0])
        os.dup2(out[1],2)
        os.close(out[1])
        if t == 0:
            os.execlp("/bin/sh", "sh", "-c", "ps auxww 1>&2")
            print("execlp(ps)")
            #perror("execlp(ps)")
            sys.exit(1)
        elif t == 1:
            os.execlp("/bin/sh", "sh", "-c", "perl -e 'for($t=0; sleep(1); $t++) {print STDERR rand().\"\\n\"; if ($t>5) {kill(9,$$);}}' 1>&2")
            print("execlp(perl)")
            #perror("excelp(perl)")
            sys.exit(1)
        elif t == 2:
            os.execlp("/bin/sh", "sh", "-c", "(ls -la; ping -c 8 localhost) 1>&2")
            #perror("execlp(ls/ping)")
            print("execlp(ls/ping)")
            sys.exit(1)
    else:
        # parent
        G_outfd[t] = out[0]
        os.close(out[1])


# Data ready interrupt
def data_ready(fd, data):
    t = data
    s = ""
    bytes = os.read(fd, 4096-1);
    if len(bytes) == 0:          # EOF
        G_buff[t].append("\n\n*** EOF ***\n")
        try:
          (pid,status) = os.waitpid(G_pids[t], os.WNOHANG)
        except:
            bytes = "waitpid(): no child process\n"
        else:
            if os.WIFEXITED(status):
                bytes = f"Exit={os.WEXITSTATUS(status)}\n"
                os.close(fd)
                Fl.remove_fd(fd)
                G_pids[t] = -1
            elif os.WIFSIGNALED(status):
                bytes = f"Killed with {os.WTERMSIG(status)}\n"
                os.close(fd)
                Fl.remove_fd(fd)
                G_pids[t] = -1
            elif os.WIFSTOPPED(status):
                bytes = f"Stopped with {os.WSTOPSIG(status)}\n"

        G_buff[t].append(bytes)
    else:                            # DATA
        if sys.version >= '3':
            G_buff[t].append(bytes.decode('utf-8'))
        else:
            G_buff[t].append(bytes)


# Clean up if someone closes the window
def close_cb(widget):
    print("Killing child processes..\n")
    for t in range(3):
        if G_pids[t] == -1:
            continue
        os.kill(G_pids[t], 9)
    sys.exit(0)


if __name__=='__main__':
    if platform.platform().find("Windows") >= 0:
        fl_message("Not supported on Windows platform!")
    else:

        win = Fl_Double_Window(620,520,"fltk-tty");
        win.callback(close_cb)             # kill children if window closed

        # Start children, one tty for each
        for t in range(3):
            start_child(t)
            G_buff[t] = Fl_Text_Buffer()
            G_disp[t] = Fl_Text_Display(10+t*200, 10, 200, 500)
            G_disp[t].buffer(G_buff[t])
            G_disp[t].textfont(FL_COURIER)
            G_disp[t].textsize(12)
            Fl.add_fd(G_outfd[t], data_ready, t)
    
        win.resizable(win)
        win.show()
        

