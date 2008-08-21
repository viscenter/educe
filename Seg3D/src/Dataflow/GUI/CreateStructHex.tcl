#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2004 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
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


itcl_class SCIRun_NewField_CreateStructHex {
    inherit Module

    constructor {config} {
        set name CreateStructHex
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        sci_toplevel $w

        sci_frame $w.row1
        sci_frame $w.row2
        sci_frame $w.row3
        sci_frame $w.row31
        sci_frame $w.which -relief groove -borderwidth 2

        pack $w.row1 $w.row2 $w.row3 $w.row31 $w.which -side top \
          -e y -f both -padx 5 -pady 5
        
        sci_label $w.row1.xsize_label -text "X Size   "
        sci_entry $w.row1.xsize -textvariable $this-sizex
        sci_label $w.row2.ysize_label -text "Y Size   "
        sci_entry $w.row2.ysize -textvariable $this-sizey
        sci_label $w.row3.zsize_label -text "Z Size   "
        sci_entry $w.row3.zsize -textvariable $this-sizez

        sci_label $w.row31.zsize_label -text "Pad Percentage"
        sci_entry $w.row31.zsize -textvariable $this-padpercent

        pack $w.row1.xsize_label $w.row1.xsize -side left
        pack $w.row2.ysize_label $w.row2.ysize -side left
        pack $w.row3.zsize_label $w.row3.zsize -side left
        pack $w.row31.zsize_label $w.row31.zsize -side left

        sci_label $w.which.l -text "Data at Location"
        sci_radiobutton $w.which.node -text "Nodes (linear basis)" \
          -variable $this-data-at -value Nodes
        sci_radiobutton $w.which.cell -text "Cells (constant basis)" \
          -variable $this-data-at -value Cells
        sci_radiobutton $w.which.none -text "None" \
          -variable $this-data-at -value None
        pack $w.which.l -side top
        pack $w.which.node $w.which.cell $w.which.none -anchor nw

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}


