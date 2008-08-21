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


itcl_class SCIRun_NewField_ClipFieldByFunctionCompiled {
    inherit Module

    constructor {config} {
        set name ClipFieldByFunctionCompiled
    }

    method update_text {} {
	set w .ui[modname]
        if {[winfo exists $w]} {
	    set $this-function [$w.function.row1 get 1.0 end]
        }
    }

    method set_text {} {
	set w .ui[modname]
        if {[winfo exists $w]} {
	    $w.function.row1 clear
	    $w.function.row1 insert end [set $this-function]
        }
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }

        sci_toplevel $w

        set cmd "$this-c needexecute"

      # Location
        sci_labeledframe $w.location -labelpos nw \
            -labeltext "Location To Test"
        set location [$w.location childsite]

              Tooltip $location "The user defined function will be evaluated at this location to determine which elements are preserved.\nThe v variable will be zero if the location does not match the field basis."

        sci_radiobutton $location.cell -text "Element Center" \
            -variable $this-mode -value element -command $cmd
        sci_radiobutton $location.nodeone -text "One Node" \
            -variable $this-mode -value onenode -command $cmd
        sci_radiobutton $location.nodemost -text "Most Nodes" \
            -variable $this-mode -value mostnodes -command $cmd
        sci_radiobutton $location.nodeall -text "All Nodes" \
            -variable $this-mode -value allnodes -command $cmd

        pack $location.cell $location.nodeone $location.nodemost \
            $location.nodeall -side top -anchor w

        pack $w.location -side top -fill x -padx 5 -pady 5

      # Function
        sci_frame $w.function -borderwidth 2

        sci_label $w.function.info -text "F(x, y, z, v0, v1, ...):"
        sci_label $w.function.info1 -text "where 'x', 'y', and 'z' are the coordinate values, 'v0', 'v1', 'v2', etc."
        sci_label $w.function.info2 -text "are data values that correspond to each valid input field. The last"
        sci_label $w.function.info3 -text "statement must be a 'return' with a conditional value."

        option add *textBackground white
        sci_scrolledtext $w.function.row1 -height 60 -hscrollmode dynamic

        $w.function.row1 insert end [set $this-function]

        pack $w.function.info  -side top -anchor w -padx 5 -pady 5
        pack $w.function.row1  -side top -expand y -fill both -padx 5 -pady 5
        pack $w.function.info1 -side top -anchor w -padx 5
        pack $w.function.info2 -side top -anchor w -padx 5
        pack $w.function.info3 -side top -anchor w -padx 5

        pack $w.function -side top -expand y -fill both \
            -padx 5 -pady 5

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}
