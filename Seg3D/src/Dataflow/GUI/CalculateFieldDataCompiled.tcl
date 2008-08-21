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


itcl_class SCIRun_ChangeFieldData_CalculateFieldDataCompiled {
    inherit Module

    constructor {config} {
        set name CalculateFieldDataCompiled
	
	# Trace variable for optionmenu so that it will display
	# the correct value when opening a saved network.
	global $this-outputdatatype
	trace variable $this-outputdatatype w \
	    "$this set_combobox .otype.c $this-outputdatatype"
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
	    $w.functio.row1 clear
	    $w.function.row1 insert end [set $this-function]
        }
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            raise $w
            return
        }
        sci_toplevel $w

# Output Data Type
	sci_frame $w.output -relief groove -borderwidth 2

	labelcombo $w.output.otype "Output Data Type" \
	    {"port 0 input" \
		 "unsigned char" "unsigned short" "unsigned int" \
		 char short int float double Vector Tensor} \
	    $this-outputdatatype

	pack $w.output.otype -side top -anchor w

	pack $w.output -side top -fill x -padx 5 -pady 5

# Function
	sci_frame $w.function -borderwidth 2

	sci_label $w.function.info -text "F(x, y, z, v0, v1, ..., result, count):"


	sci_label $w.function.info1 -text "where 'x', 'y', and 'z' are the coordinate values, 'v0', 'v1', 'v2', etc."
	sci_label $w.function.info2 -text "are data values that correspond to each valid input field, and optionally"
	sci_label $w.function.info3 -text "'result' corresponds to the previous output field (if cached - otherwise zero) and"
	sci_label $w.function.info4 -text "'count' corresponds to the number of times the cached output field has been used."

	option add *textBackground white
	sci_scrolledtext $w.function.row1 -height 60 -hscrollmode dynamic

	$w.function.row1 insert end [set $this-function]

	pack $w.function.info  -side top -anchor w -padx 5 -pady 5
	pack $w.function.row1  -side top -fill both -expand true -padx 5
	pack $w.function.info1 -side top -anchor w -padx 5
	pack $w.function.info2 -side top -anchor w -padx 5
	pack $w.function.info3 -side top -anchor w -padx 5
	pack $w.function.info4 -side top -anchor w -padx 5

	pack $w.function -side top -fill both -expand true -padx 5 -pady 5

# Caching
	sci_labeledframe $w.caching -labelpos nw -labeltext "Caching"
	set caching [$w.caching childsite]

	sci_checkbutton $caching.cache -text "Cache result" \
		-variable $this-cache
	sci_button $caching.clear \
	    -text " Clear cached result " -command "$this-c clear"

	sci_label $caching.label -text "Count:"

	sci_entry $caching.count -width 5 -textvariable $this-count  -state disabled
	
	pack $caching.cache -side left -padx  5 -pady 5
	pack $caching.label -side left -pady 5
	pack $caching.count -side left -padx 20 -pady 5
	pack $caching.clear -side left -padx  5 -pady 5

	pack $w.caching -side top -fill x -padx 5 -pady 5


	makeSciButtonPanel $w $w $this
	moveToCursor $w
    }


    method labelcombo { win text1 arglist var} {
	sci_frame $win 
	pack $win -side top -padx 5
	sci_label $win.l1 -text $text1 \
		      -anchor w -just left
	sci_label $win.colon  -text ":" -width 2 -anchor w -just left
	sci_optionmenu $win.c -foreground darkred \
		-command " $this comboget $win.c $var "

	set i 0
	set found 0
	set length [llength $arglist]
	for {set elem [lindex $arglist $i]} {$i<$length} \
	    {incr i 1; set elem [lindex $arglist $i]} {
	    if {"$elem"=="[set $var]"} {
		set found 1
	    }
	    $win.c insert end $elem
	}

	if {!$found} {
	    $win.c insert end [set $var]
	}

	$win.c select [set $var]

	sci_label $win.l2 -text "" -width 20 -anchor w -just left

	# hack to associate optionmenus with a textvariable
	# bind $win.c <Map> "$win.c select {[set $var]}"

	pack $win.l1 $win.colon -side left
	pack $win.c $win.l2 -side left	
    }

    method comboget { win var } {
	if {![winfo exists $win]} {
	    return
	}
	if { "$var"!="[$win get]" } {
	    set $var [$win get]
	}
    }

    method set_combobox { win var name1 name2 op } {
	set w .ui[modname]
	set menu $w.$win
	if {[winfo exists $menu]} {
	    $menu select $var
	}
    }
}


