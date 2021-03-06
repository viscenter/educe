#!/usr/bin/env python
# -*- coding: utf-8 -*-
# generated by wxGlade 0.5 on Wed Aug  8 10:56:48 2007 from /home/scratch/mjc/cibc/SCIRun/src/StandAlone/Apps/FEMesher/wxMesher

import wx
from ViewerFrame import ViewerFrame
from controls import controls
import sr_py
import os

class Mesher3D(wx.App):
    def OnInit(self):
        wx.InitAllImageHandlers()

        # init the SCIRun python interface
        env = []
        for k in os.environ.keys() :
            estr = "%s=%s" % (k, os.environ[k])
            env.append(estr)
            
        sr_py.init_sr_py(env)

        
        Viewer = ViewerFrame(None, -1, "")
        controls_ = controls(None, -1, "")
        self.SetTopWindow(Viewer)
        Viewer.Show()
        controls_.Show()
        return 1

# end of class Mesher3D

if __name__ == "__main__":
    Mesher3D = Mesher3D(0)
    Mesher3D.MainLoop()
