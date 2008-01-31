import wx
import sys
import time
import sr_py

ID_EXIT = wx.NewId()
ID_ABOUT = wx.NewId()

def add_pushed(event) :
    print event
    print "add_pushed"

def remove_pushed(event) :
    print event
    print "remove_pushed"

def generate_pushed(event) :
    print event
    print "generate_pushed"

def menu_item_about(event) :
    print "About time this got called..."

def menu_item_quit(event) :
    print "bye bye"
    sr_py.terminate();
    time.sleep(.5)
    sys.exit(0)
