import sys
from PyQt4 import QtCore, QtGui, QtOpenGL, uic
import math, os
import sr_py
from SQViewerWidget import SQViewerWidget
import uuid
import time

env = []
for k in os.environ.keys() :
    estr = "%s=%s" % (k, os.environ[k])
    env.append(estr)
    
sr_py.init_sr_py(env)

app = QtGui.QApplication(sys.argv)
form_class, base_class = uic.loadUiType("basic_layout.ui")

class AppWin(QtGui.QMainWindow, form_class):
    def __init__(self, *args):
        QtGui.QMainWindow.__init__(self, *args)
        self.setupUi(self)
        self.nrrd_reader = sr_py.get_read_nrrd_alg()
        #self.create_1d_cmap = sr_py.get_create_standard_colormaps_alg()
        self.texture_builder = sr_py.get_convert_nrrds_to_texture_alg()
        self.volume_renderer = sr_py.get_show_texture_volume_alg()
        #self.cmap2edit = sr_py.get_create_and_edit_colormap2d_alg()
        #set editor alg to a unique id based on its default id
#        unique_id = uuid.uuid4()
 #       vrid = "%s_%s" % (self.volume_renderer.get_p_id(), str(unique_id))
  #      self.volume_renderer.set_p_id(vrid)
        #connect cm2view notify target to the volume renderer.
        self.cm2edit_wid.set_notify_id(self.volume_renderer.get_p_id())
        
        self.connect(self.actionOpen, QtCore.SIGNAL('triggered()'),
                     self.open_browser)
        self.connect(self.actionQuit, QtCore.SIGNAL('triggered()'),
                     self.quit)

        self.connect(self.actionColorMap_2D, QtCore.SIGNAL('triggered()'),
                     self.open_cmap2_editor)

        self.connect(self.color_button, QtCore.SIGNAL('clicked()'),
                     self.choose_color)

        self.connect(self.add_rectangle_button, QtCore.SIGNAL('clicked()'),
                     self.add_rectangle)



    def open_browser(self) :

        self.fbrowser = QtGui.QFileDialog()
        msg = app.translate("MainWindow",
                            "Select patch files To Convert",
                            None, app.UnicodeUTF8)
        ext = "Nrrd File(*.nrrd);;Nrrd Header (*.nhdr);;All Files (*)"
        fileName = self.fbrowser.getOpenFileNames(None,
                                                  msg ,
                                                  os.getcwd(),
                                                  ext)
        for f in fileName :
            nrrd_handle, s = self.nrrd_reader.execute(str(f))
            #cmap1 = self.create_1d_cmap.execute()
            texture, histo = self.texture_builder.execute(nrrd_handle, 0)
            vol_geom, n = self.volume_renderer.execute(texture, 0, 0)
            sr_py.send_scene(vol_geom)

      
    def choose_color(self) :
        color = QtGui.QColorDialog.getColor()
        print "chose: ", color

    def add_rectangle(self) :
        hid = self.cm2edit_wid.get_handle_id()
        sr_py.add_rectangle_to_cm2view(hid)
        
    def open_cmap2_editor(self) :
        if self.actionColorMap_2D.isChecked() :
            #open the dialog
            self.cm2edit_wid.show()
            pass
        else :
            #close the dialog
            self.cm2edit_wid.hide()
            pass

    def quit(self) :
        sr_py.terminate()
        self.close()
        

if __name__ == '__main__' :

    form = AppWin()
    form.show()
    app.exec_()

    
