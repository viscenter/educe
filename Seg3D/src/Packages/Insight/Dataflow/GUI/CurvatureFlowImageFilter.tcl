#
#   For more information, please see: http://software.sci.utah.edu
#
#   The MIT License
#
#   Copyright (c) 2004 Scientific Computing and Imaging Institute,
#   University of Utah.
#
#   
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

#############################################################
#  This is an automatically generated file for the 
#  itk::CurvatureFlowImageFilter
#############################################################


 itcl_class Insight_Filters_CurvatureFlowImageFilter {
    inherit Module
    constructor {config} {
         set name CurvatureFlowImageFilter

         global $this-time_step
         global $this-number_of_iterations

         set_defaults
    }

    method set_defaults {} {

         set $this-time_step 0.125
         set $this-number_of_iterations 5
    }


    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
	    set child [lindex [winfo children $w] 0]
	    
	    # $w withdrawn by $child's procedures
	    raise $child
	    return;
        }

        toplevel $w


        frame $w.time_step
        label $w.time_step.label -text "time_step"
        entry $w.time_step.entry \
            -textvariable $this-time_step
        pack $w.time_step.label $w.time_step.entry -side left
        pack $w.time_step

        frame $w.number_of_iterations
        label $w.number_of_iterations.label -text "number_of_iterations"
        entry $w.number_of_iterations.entry \
            -textvariable $this-number_of_iterations
        pack $w.number_of_iterations.label $w.number_of_iterations.entry -side left
        pack $w.number_of_iterations
        
        frame $w.buttons
	makeSciButtonPanel $w.buttons $w $this
	moveToCursor $w
	pack $w.buttons -side top 

    }

}
