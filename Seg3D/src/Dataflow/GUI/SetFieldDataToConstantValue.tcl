##
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

itcl_class SCIRun_ChangeFieldData_SetFieldDataToConstantValue {
    inherit Module
    constructor {config} {
        set name SetFieldDataToConstantValue
    }
    
    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        
        sci_toplevel $w
        
        sci_frame $w.f
        pack $w.f
        sci_label $w.f.lab1 -text "Data Type output field"
        grid $w.f.lab1 -row 0 -column 0 -sticky e
        sci_label $w.f.lab2 -text "Basis Order output field"
        grid $w.f.lab2 -row 1 -column 0 -sticky e
        sci_label $w.f.lab3 -text "Constant value for all nodes/elements"
        grid $w.f.lab3 -row 2 -column 0 -sticky e

        myselectionbutton $w.f.sel1 0 1 { "same as input" "char" "short" "unsigned short" "unsigned int" "int" "float" "double" } $this-data-type
        myselectionbutton $w.f.sel2 1 1 { "same as input" "nodata" "constant" "linear" "quadratic"} $this-basis-order
        sci_entry             $w.f.e1 -textvariable $this-value
        grid              $w.f.e1 -row 2 -column 1 -sticky news
        
        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }

   method myselectionbutton { win x y arglist var} {
        sci_frame $win 
        grid $win  -row $x -column $y -sticky news
        sci_optionmenu $win.c -foreground darkred -command " $this comboget $win.c $var "

        set i 0
        set found 0
        set length [llength $arglist]
        for {set elem [lindex $arglist $i]} {$i<$length} {incr i 1; set elem [lindex $arglist $i]} {
          if {"$elem"=="[set $var]"} {
            set found 1
          }
          $win.c insert end $elem
        }

        if {!$found} {
          $win.c insert end [set $var]
        }

        $win.c select [set $var] 
        pack $win.c	-fill x
    }


    method comboget { win var } {
        if {![winfo exists $win]} {
          return
        }
        if { "$var"!="[$win get]" } {
          set $var [$win get]
        }
    }
}

    