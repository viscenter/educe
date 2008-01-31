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


itcl_class SCIRun_MiscField_GenerateLinearSegments {
    inherit Module

    constructor {config} {
        set name GenerateLinearSegments
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }

        toplevel $w

	frame $w.params -relief groove -borderwidth 2
	frame $w.params.labs
	frame $w.params.ents
	label $w.params.labs.stepsize -text "Step Size" -just left
	entry $w.params.ents.stepsize -textvariable $this-stepsize
	label $w.params.labs.maxsteps -text "Maximum Steps" -just left
	entry $w.params.ents.maxsteps -textvariable $this-maxsteps

	pack $w.params.labs.stepsize $w.params.labs.maxsteps -side top -anchor w
	pack $w.params.ents.stepsize $w.params.ents.maxsteps -side top -anchor w
	pack $w.params.labs $w.params.ents -side left

	frame $w.direction -relief groove -borderwidth 2
	label $w.direction.label -text "Direction"
	radiobutton $w.direction.neg -text "Negative" \
	    -variable $this-direction -value 0
	radiobutton $w.direction.both -text "Both" \
	    -variable $this-direction -value 1
	radiobutton $w.direction.pos -text "Positive" \
	    -variable $this-direction -value 2

	pack $w.direction.label -side top -fill both
	pack $w.direction.neg $w.direction.both $w.direction.pos \
	    -side left -fill both


	frame $w.value -relief groove -borderwidth 2
	label $w.value.label -text "Segment Value"
	frame $w.value.left
	frame $w.value.middle
	frame $w.value.right
	radiobutton $w.value.left.value -text   "Seed Value" \
	    -variable $this-value -value 0
	label $w.value.left.blank -text ""
	radiobutton $w.value.middle.index -text "Seed Index" \
	    -variable $this-value -value 1
	radiobutton $w.value.middle.incr -text  "Integration Step" \
	    -variable $this-value -value 2
# 	radiobutton $w.value.right.delta -text  "Distance from Seed" \
# 	    -variable $this-value -value 3
# 	radiobutton $w.value.right.total -text  "Segment Length" \
# 	    -variable $this-value -value 4

	pack $w.value.left.value $w.value.left.blank -side top -anchor w
	pack $w.value.middle.index $w.value.middle.incr -side top -anchor w
#	pack $w.value.right.delta $w.value.right.total -side top -anchor w

	pack $w.value.label -side top -fill both
	pack $w.value.left $w.value.middle $w.value.right -side left -anchor w


	pack $w.params $w.direction $w.value \
	    -side top -e y -f both -padx 5 -pady 5

	makeSciButtonPanel $w $w $this
	moveToCursor $w
    }
}


