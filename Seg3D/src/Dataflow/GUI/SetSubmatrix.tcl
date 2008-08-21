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


itcl_class SCIRun_Math_SetSubmatrix {
    inherit Module

    constructor {config} {
        set name SetSubmatrix
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            raise $w
            return
        }
        sci_toplevel $w

        sci_frame $w.info
        sci_frame $w.sinfo
        sci_frame $w.row
        sci_frame $w.col

        global $this-nrow $this-ncol
        sci_label $w.info.label -text "Input Matrix:   nrows ="
        sci_label $w.info.row -textvariable $this-nrow
        sci_label $w.info.x -text "  ncols ="
        sci_label $w.info.col -textvariable $this-ncol
        pack $w.info.label $w.info.row $w.info.x $w.info.col -side left -anchor n -expand yes -fill x


        global $this-srow $this-scol
        sci_label $w.sinfo.label -text "Input Submatrix:   nrows ="
        sci_label $w.sinfo.row -textvariable $this-srow
        sci_label $w.sinfo.x -text "  ncols ="
        sci_label $w.sinfo.col -textvariable $this-scol
        pack $w.sinfo.label $w.sinfo.row $w.sinfo.x $w.sinfo.col -side left -anchor n -expand yes -fill x

        global $this-startrow $this-startcol
        sci_label $w.row.label -text "Start Row" -width 10 -just left
        sci_entry $w.row.min -width 10 -text $this-startrow
        pack $w.row.label $w.row.min -side left -anchor n -expand yes -fill x

        sci_label $w.col.label -text "Start Col" -width 10 -just left
        sci_entry $w.col.min -width 10 -text $this-startcol
        pack $w.col.label $w.col.min -side left -anchor n -expand yes -fill x
        
        pack $w.info $w.sinfo $w.row $w.col -side top -expand yes -fill x

        makeSciButtonPanel $w $w $this
        moveToCursor $w

        bind $w.row.min <KeyPress-Return> "$this-c needexecute"
        bind $w.col.min <KeyPress-Return> "$this-c needexecute"
    }
}




