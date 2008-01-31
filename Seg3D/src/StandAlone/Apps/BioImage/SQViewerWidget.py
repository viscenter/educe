#  
#  For more information, please see: http://software.sci.utah.edu
#  
#  The MIT License
#  
#  Copyright (c) 2004 Scientific Computing and Imaging Institute,
#  University of Utah.
#  
#  License for the specific language governing rights and limitations under
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#  
#    File   : SQViewerWidget.py
#    Author : Martin Cole
#    Date   : Tue Nov  6 10:45:45 2007


import sys
from PyQt4 import QtCore, QtGui, QtOpenGL, uic
import math
import sr_py
import uuid



class SQViewerWidget(QtOpenGL.QGLWidget):
    """
    SQViewerWidget is the actual rendering widget that can display
    a SCIRun scene graph inside a Qt QWidget
    """
    def __init__(self, parent=None):
        QtOpenGL.QGLWidget.__init__(self, parent)

        self.vid = None
        self.sci_context = sr_py.CallbackOpenGLContext()
        self.sci_context.set_pymake_current_func(self.make_current)
        self.sci_context.set_pyswap_func(self.swap)
        self.sci_context.set_pywidth_func(self.get_width)
        self.sci_context.set_pyheight_func(self.get_height)
        self.sci_context.set_pyrelease_func(self.release)
        unique_id = uuid.uuid4()
        self.id = "sr_viewer_%s" % str(unique_id)
        self.full_speed = False
        self.timer = QtCore.QTimer()
        self.connect(self.timer,
                     QtCore.SIGNAL("timeout()"), self.updateGL)
        
    def make_current(self) :
        if self.isValid() :
            self.makeCurrent()
            return 1
        return 0

    def swap(self) :
        if self.isValid() :
            self.swapBuffers() 
            return 1
        return 0

    def get_width(self) :
        if self.isValid() :
            w = self.width()
            #print w
            return w
        return 0

    def get_height(self) :
        if self.isValid() :
            h = self.height()
            #print h
            return h
        return 0
    
    def release(self) :
        #self.doneCurrent()
        pass

    def pointer_event(self, e, event) :
        e.set_x(event.x())
        e.set_y(event.y())

        mask = e.get_modifiers()
        mask = self.get_sr_py_modifier_mask(event, mask) 
        e.set_modifiers(mask)
        
        state = e.get_pointer_state()
        state, n = self.get_sr_py_pointer_modifier_mask(event, state)
        e.set_pointer_state(state)
        sr_py.add_pointer_event(e, self.id)
        
    def get_sr_py_modifier_mask(self, event, mask) :
        if event.modifiers() & QtCore.Qt.ShiftModifier :
            mask  |= sr_py.EventModifiers.SHIFT_E
        if event.modifiers() & QtCore.Qt.ControlModifier :
            mask  |= sr_py.EventModifiers.CONTROL_E
        if event.modifiers() & QtCore.Qt.AltModifier :
            mask  |= sr_py.EventModifiers.ALT_E
        if event.modifiers() & QtCore.Qt.MetaModifier :
            mask  |= sr_py.EventModifiers.M1_E
        return mask

    def get_sr_py_pointer_modifier_mask(self, event, mask) :
        n = 0

        if event.buttons() & QtCore.Qt.LeftButton :
            mask  |= sr_py.PointerEvent.BUTTON_1_E
            n = 1
        if event.buttons() & QtCore.Qt.MidButton :
            mask  |= sr_py.PointerEvent.BUTTON_2_E
            n = 2
        if event.buttons() & QtCore.Qt.RightButton :
            mask  |= sr_py.PointerEvent.BUTTON_3_E
            n = 3
        return mask, n

    def initializeGL(self):
        self.vid = sr_py.create_viewer(self.sci_context, self.id)
        print "created viewer:%d" % self.vid 

    def glDraw(self):
        sr_py.update_viewer(self.vid);

        if self.timer.isActive() and not self.full_speed :
            self.full_speed = True
            self.timer.start(55)

        if not self.timer.isActive() :
            self.timer.start(1000)

    def mousePressEvent(self, event):
        """ mousePressEvent(e: QMouseEvent) -> None
        Echo mouse event to SCIRun event system.
        
        """
        e = sr_py.PointerEvent()
        e.set_pointer_state(sr_py.PointerEvent.BUTTON_PRESS_E)
        self.pointer_event(e, event)

    def mouseReleaseEvent(self, event):
        """ mouseReleaseEvent(e: QEvent) -> None
        Echo mouse event to SCIRun event system.
        
        """
        e = sr_py.PointerEvent()
        #e.set_time(long(event.time))
        e.set_pointer_state(sr_py.PointerEvent.BUTTON_RELEASE_E)
        self.pointer_event(e, event)

    def mouseMoveEvent(self, event):
        """ mouseMoveEvent(e: QMouseEvent) -> None
        Echo mouse event to SCIRun event system.
        
        """
        e = sr_py.PointerEvent()

        #e.set_time(long(event.time))
        e.set_pointer_state(sr_py.PointerEvent.MOTION_E)
        self.pointer_event(e, event)
                  
    def enterEvent(self,e):
        """ enterEvent(e: QEvent) -> None
        
        """

    def leaveEvent(self,e):
        """ leaveEvent(e: QEvent) -> None
        
        """
        
        
    def keyPressEvent(self, event):
        """ keyPressEvent(e: QKeyEvent) -> None
        Echo the key event to the SCIRun event system.
        
        """

        ascii_key = None
        if event.text().length()>0:
            ascii_key = event.text().toLatin1()[0]
        else:
            ascii_key = chr(0)

        #for now use native...
        keysym = event.nativeVirtualKey()
        e = sr_py.KeyEvent()
        #e.set_time(long(event.time))
        # translate qt modifiers to sci modifiers
        mask = e.get_modifiers()
        e.set_modifiers(self.get_sr_py_modifier_mask(event, mask))
        e.set_keyval(keysym)
        e.set_key_string(ascii_key)
        e.set_key_state(sr_py.KeyEvent.KEY_PRESS_E)
        sr_py.add_key_event(e, self.id)
        
    def keyReleaseEvent(self,e):
        """ keyReleaseEvent(e: QKeyEvent) -> None
        
        """
        #print "keyReleaseEvent"

    def wheelEvent(self,e):
        """ wheelEvent(e: QWheelEvent) -> None
        Zoom in/out while scrolling the mouse
        
        """
        print "wheelEvent"

