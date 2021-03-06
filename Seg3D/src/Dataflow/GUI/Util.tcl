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


# Utility functions

# To link two variables, so that when one changes the other changes, too
# (update_vars is just the helper function, link_vars is the one to use)

proc link_vars {v1 v2} {
    global $v1
    global $v2
    trace variable $v1 w "update_vars $v2"
    trace variable $v2 w "update_vars $v1"
}

# Helper function, do NOT call directly.
proc update_vars {v2 v1 vtmp op} {
    global $v1
    global $v2
    if {[set $v1] != [set $v2]} {
	set $v2 [set $v1]
    }
}


# Convenience routine for making radiobuttons

proc make_labeled_radio {root labeltext command packside numColumns variable list} {
    frame $root
    frame $root.label
    frame $root.columns
    pack $root.label $root.columns -side $packside -expand yes -fill both

    label $root.label.radiolabel -text $labeltext
    pack $root.label.radiolabel -side left

    for {set i 0} {$i < $numColumns } {incr i } {
	frame $root.columns.$i
	pack $root.columns.$i -side left -expand yes -fill both
    }

    set w $root.col-0

    set numItems [llength $list]

    set i 0
    foreach t $list {

	set column [expr $i % $numColumns]
	set w $root.columns.$column

	set name [lindex $t 0]
	set value $name
	if {[llength $t] == 2} {
	    set value [lindex $t 1]
	}
	radiobutton $w.$i -text "$name" -variable $variable \
		-value $value -command $command
	pack $w.$i -side top -anchor nw
	incr i
    }
}

proc change_radio_var {root newvar} {
    foreach t [winfo children $root] {
	set name [winfo name $t]
	if {$name != "radiolabel"} {
	    $t config -variable $newvar
	}
    }
}


# Exponential scale
itcl_class expscale {

    protected decimalplaces

    method modname { c } {
	set n [string range $c [expr [string last "::" $c] + 2] end]
	return $n
    }

    constructor {config} {

	#  Create a window with the same name as this object

	#puts "entering expscale constructor"
	#puts "dollar this is $this"
	#puts "name received by expscale = [modname $this]"
	#puts "dollar this info class = [ $this info class] "

	set decimalplaces 3

	set class [modname [$this info class]]
	if {[catch {set w}]} {
	    set w [modname $this]
	}

	frame $w -class $class -borderwidth 2 -relief groove

	# This tells the expscale to delete itself when its
	# window is destroyed.  That way, when the expscale
	# is "re-created" there isn't a name conflict.
	# (A better solution is to not let the GUI windows
	# be destroyed in the first place... just close 
	# (unmap) them and remap them when they need to
	# be seen again.
	bind $w <Destroy> "::delete object $this"

	pack $w -side top -fill x -expand 1
	if {[catch {set variable}]} {
	    set variable $w-variable
	}
	global $variable
	if {[catch {set $variable}]} {
	    set $variable 1
	}

	set tmpvar [set $variable]

	set resolution [getResolution ]
	set built 1
	scale $w.scale -label $label -orient $orient \
		-from -1.0e-17 -to 1.0e17 -resolution $resolution \
		-variable $variable -command $command
	pack $w.scale -fill x -side left -expand yes

	frame $w.e
	pack $w.e -fill y -padx 2 -anchor se

	label $w.e.e -text "E"
	pack $w.e.e -side left

	button $w.e.upexp -text "+" -pady 0 -command "$this upexp"
	button $w.e.signchange -text "S" -pady 0 -command "$this chngsign"
	button $w.e.downexp -text "-" -pady 0 -command "$this downexp"
	pack $w.e.upexp -side top -fill x
	pack $w.e.signchange -side bottom -fill x
	pack $w.e.downexp -side bottom -fill x

	entry $w.e.exp -width 3 -selectborderwidth 0 -highlightthickness 0 \
		-insertborderwidth 0
	pack $w.e.exp

	newvalue $tmpvar
    }

    destructor {
	destroy $w.scale
	destroy $w.e
    }

    method config {config} {
    }

    public label "" 
    public orient ""
    public w
    protected built 0
    public variable "" {
	if {$built} {
	    $w.scale config -variable ""
	    updatevalue [set $variable]
	    $w.scale config -variable $variable
	}
    }

    method getResolution {} {
	if { $decimalplaces == 0 } {
	    return 1
	} else {
	    set val 1

	    for { set i 1} { $i < $decimalplaces } {incr i } {
		set val "0$val"
	    }
	    return "0.$val"
	}
    }
    protected exp 0
    protected sign 1
    public command ""

    method upexp {} {
	if {$exp > 16} {
	    return;
	}
	incr exp
	if { $exp > 2 } {
	    set decimalplaces 0
	} else {
	    incr decimalplaces -1
	    if {$decimalplaces < 0} { set decimalplaces 0 }
	}
	global $variable
	updatevalue [set $variable]
    }

    method downexp {} {
	if {$exp < -16} {
	    return;
	}
	incr exp -1
	if { $exp > 2 } {
	    set decimalplaces 0
	} else {
	    incr decimalplaces
	}
	global $variable
	updatevalue [set $variable]
    }

    method chngsign {} {
	incr sign [expr $sign * -2]
	global $variable
	updatevalue [expr -1 * [set $variable]]
	if {[set $variable] == 0} {
	    updatevalue -0.00000000001
	}
    }

    # don't change the number of decimalplaces or the exponent - just move
    #   the variable and set the to-from bounds on the scale

    method updatevalue {value} {
	set mag [expr pow(10, $exp)]
	# Round the value down to get from...
	if {$value < 0} {
	    set from [expr int($value/$mag-1)*$mag]
	} else {
	    set i [expr int($value/$mag)]
	    set from [expr $i * $mag]
	}

	set to [expr $from+$mag]
	set ti [expr $mag/3]
	
	set resolution [getResolution ]
	$w.scale configure -from $from -to $to \
	    -tickinterval $ti -resolution $resolution

	global $variable
	set $variable [format "%e" $value]
	$w.e.exp delete 0 end
	$w.e.exp insert 0 $exp
    }
	
    # figure out everything - decimalplaces, exponent, and to-from bounds

    method newvalue {value} {
	global $variable
	# crop of trailing zeros
	set value [expr $value * 1]

	set enum [format "%e" $value]
	set spl [split $enum e] 
	set left [lindex $spl 0]
	set right [format "%.0f" [lindex $spl end]]

	set decimalplaces [expr abs($right) + 2]

	set exp 2
	if {$value != 0} {
	    while {([expr pow(10, [expr $exp-1])] > [expr abs($value)]) && \
		       ($exp >= -16)} {
		
		incr exp -1
	    }
	}
	set mag [expr pow(10, $exp)]
	if {$value < 0} {
	    set sign -1
	}
	# Round the value down to get from...
	if {$value < 0} {
	    set from [expr int($value/$mag-1)*$mag]
	} else {
	    set from [expr int($value/$mag)*$mag]
	}
	
	set to [expr $from+$mag]
	set ti [expr $mag/3]
	
	set resolution [getResolution ]
	
	$w.scale configure -from $from -to $to \
	    -tickinterval $ti -resolution $resolution

	set $variable [format "%e" $value]

	$w.e.exp delete 0 end
	$w.e.exp insert 0 $exp
    }	
}

#############################################################################
# SciRaise window
#
# The method to raise a window varies depending on the windows state
# (at least on the Mac).  This is a convenience function for taking
# care of the raise.
proc SciRaise { window } {
    if { [winfo ismapped $window] == 1} {
	raise $window
    } else {
	wm deiconify $window
    }
}

#############################################################################
# centerWindow w1 w2 
#
# Centers window w1 over window w2.  If w2 is blank, then the window is 
# centered in the middle of the screen.  (If there are two monitors,
# then attempt to only center on the left one.)
#
proc centerWindow { w1 { w2 "" } } {

    global OnWindows
    if { $OnWindows } {
        # Move it off the screen (to avoid flickering on the screen).
        wm geometry $w1 -3000-3000
        # Under winblows, we can't change the state of a withdrawn window, so
        # we have to bring it back... :(
        wm deiconify $w1
    }

    # The 'update', I think, allows the window to finish resizing itself
    # (in the case that it is newly created and actually hasn't been completely
    # finalized).
    update

    if { [winfo exists $w2] } {
	set w [winfo width $w2]
	set h [winfo height $w2]
	set x [winfo x $w2]
	set y [winfo y $w2]
    } else {
	set w 0
	set h 0
	set x 0
	set y 0
    }

    if {$h < 2} { set h [winfo screenheight .] }    

    if {$w < 2} { 
        set w [winfo screenwidth .]

        # Determine the ratio between width and height... If it is
        # greater than 2, then we probably have two screens.  The '.'
        # in "$w." is to turn it into a floating point number.
        set ratio [expr $w./$h]

        if { $ratio > 2 } {
            # Assume the two screens are the same dimensions... so set w = w/2.
            set w [expr $w / 2]
        }
    }

    set x [expr $x+($w - [winfo width $w1])/2]
    set y [expr $y+($h - [winfo height $w1])/2]

    wm geometry $w1 +${x}+${y}

    if { [winfo ismapped $w1] } {
        if { $OnWindows } { update }
	raise $w1
    } else {
	wm deiconify $w1
    }
}

#############################################################################
# label_pair frame "Label Text" "Label Var"
#
# - frame         : The tcl path to the location name the frame should be created at (eg: .w.f1)
# - Label Text    : Text to use as the label 
# - Label Var     : Variable that will be used to update this field
# - [Label Width] : Optional width of the label text (defaults to 12) 
#
# Example use:
#
#  set temp 212
#  label_pair .w.temperatureFrame "Temperature" tempature
#
#  Creates a new 'frame' displaying:
#
#    Temperature  :  212
# 
# If the var 'tempature' is updated, the label will automatically update too.
#

proc label_pair { f text_label text_var {first_width 12} } {
    frame $f 

    label $f.l1 -text $text_label -width $first_width -anchor w -just left

    label $f.colon  -text ":" -width 2 -anchor w -just left 

    label $f.l2 -textvar $text_var -width 40 -anchor w -just left -fore darkred -borderwidth 0
    pack $f.l1 $f.colon $f.l2 -side left
} 
