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

itcl_class SCIRun_Render_ViewScene {
    inherit Module

    # List of ViewWindows that are children of this ViewScene
    protected openViewScenesList ""

    constructor {config} {
	set name ViewScene
	set make_progress_graph 0
	set make_time 0
    }

    destructor {
	foreach rid $openViewScenesList {
	    deleteViewWindow $rid
	}
    }

    method number {} {
	return [lindex [split $this _] end]
    }

    method makeViewWindowID {} {
	set nextrid 0
	set id $this-ViewWindow_$nextrid
	while { [string length [::info commands $id]] } {
	    incr nextrid
	    set id $this-ViewWindow_$nextrid
	}
	return $id
    }

    method addViewer { { old_vw "" } } {
	set i 0
	set rid [makeViewWindowID]

	$this-c addviewwindow $rid
	ViewWindow $rid -viewer $this
	lappend openViewScenesList $rid

	if { [string length $old_vw] } {
	    set oldvars [uplevel \#0 info vars $old_vw-view-*]
	    foreach oldvar $oldvars {
		set pieces [split $oldvar -]
		set newvar [join [lreplace $pieces 0 1 $rid] -]
		upvar \#0 $newvar newView $oldvar oldView
		set newView $oldView
	    }
	}
	$rid-c redraw

	return $rid
    }

    method deleteViewWindow { rid } {
	$this-c deleteviewwindow $rid
	listFindAndRemove openViewScenesList $rid
	destroy .ui[$rid modname]
	$rid delete
    }
    
    method ui {} {
	if { [llength $openViewScenesList] == 0 } {;# If there are no open viewers
	    $this addViewer ;# then create one
	} else { ;# else, raise them all.
	    foreach rid $openViewScenesList {
		SciRaise .ui[$rid modname]
	    }
	}
    }

    method ui_embedded {} {
	set rid [makeViewWindowID]
	$this-c addviewwindow $rid
	BaseViewWindow $rid -viewer $this
	lappend openViewScenesList $rid
	return $rid
    }

    # writeStateToScript
    # Called from genSubnetScript, it will append the TCL
    # commands needed to initialize this module's variables
    # after it is created.  This is located here in the Module class
    # so sub-classes (like SCIRun_Render_ViewScene) can specialize
    # the variables they write out
    #
    # 'scriptVar' is the name of the TCL variable one level
    # up that we will append our commands to 
    # 'prefix' is the number indicating the prefix for the variables
    # 'tab' is the indent string to make it look pretty
    method writeStateToScript { scriptVar prefix { tab "" }} {
	upvar 1 $scriptVar script
	set module [modname]
	set num 0
	foreach w [winfo children .] {
	    if { [string first .ui$module $w] == 0 && \
		     [winfo exists $w.bsframe] } {
		netedit add-modgui-callback $prefix addViewer

		# since the viewer always initially comes up without
		# the extended controls, save the geometry to only
		# include the menu, viewer gl window, and standard controls
		set width [winfo width $w.bsframe]
		set height1 [winfo height $w.menu]
		set height2 [winfo height $w.wframe]
		set height3 [winfo height $w.bsframe]
		
		# Depending if the extended controls are attached/detached,
		# there are 5-8 pixels used for padding, hence the magic 7
		set height [expr $height1 + $height2 + $height3 + 7]
		set x [winfo rootx $w]
		set y [winfo rooty $w]
		netedit add-mod-var $prefix ViewWindow_$num-geometry $width\x$height\+$x\+$y
		incr num
	    }
	}

	Module::writeStateToScript $scriptVar $prefix $tab
    }
}



itcl_class BaseViewWindow {
    protected renderWindow ""
    public viewer ""

    constructor {config} {
	set_defaults
    }

    destructor {
	if { [winfo exists $renderWindow] } {
	    destroy $renderWindow
	}
    }

    method modname {} {
	return [string trimleft $this :]
    }

    method number {} {
	set parts [split $this _]
	return [lindex $parts end]
    }

    method set_defaults {} {
	# set defaults values for parameters that weren't set in a script
	initGlobal $this-saveFile "MyImage.png"
	initGlobal $this-saveType "png"
        initGlobal $this-resScale "1.0"

	# Animation parameters
	initGlobal $this-current_time 0
	initGlobal $this-tbeg 0
	initGlobal $this-tend 1
	initGlobal $this-framerate 15
	initGlobal $this-totframes 30
	initGlobal $this-caxes 0
	initGlobal $this-raxes 1

	# Need to initialize the background color
	initGlobal $this-bgcolor-r 0
	initGlobal $this-bgcolor-g 0
	initGlobal $this-bgcolor-b 0

	# Need to initialize the scene material scales
	initGlobal $this-ambient-scale 1.0
	initGlobal $this-diffuse-scale 1.0
	initGlobal $this-specular-scale 0.4
	initGlobal $this-emission-scale 1.0
	initGlobal $this-shininess-scale 1.0

	# Initialize point size, line width, and polygon offset
	initGlobal $this-point-size 1.0
	initGlobal $this-line-width 1.0
	initGlobal $this-polygon-offset-factor 1.0
	initGlobal $this-polygon-offset-units 0.0

	# Set up lights
	initGlobal $this-global-light0 1 ; # light 0 is the head light
	initGlobal $this-global-light1 0
	initGlobal $this-global-light2 0
	initGlobal $this-global-light3 0
	initGlobal $this-lightVectors \
	    {{ 0 0 1 } { 0 0 1 } { 0 0 1 } { 0 0 1 }}
	initGlobal $this-lightColors \
	    {{1.0 1.0 1.0} {1.0 1.0 1.0} {1.0 1.0 1.0} {1.0 1.0 1.0}}

	initGlobal $this-sbase 0.4
	initGlobal $this-sr 1
	initGlobal $this-do_stereo 0

	initGlobal $this-def-color-r 1.0
	initGlobal $this-def-color-g 1.0
	initGlobal $this-def-color-b 1.0

	initGlobal $this-ortho-view 0

  initGlobal $this-lock-view-window 1
	initGlobal $this-trackViewWindow0 1

	# Fog variables
	initGlobal $this-fogusebg 1
	initGlobal $this-fogcolor-r 0.0
	initGlobal $this-fogcolor-g 0.0
	initGlobal $this-fogcolor-b 1.0
	initGlobal $this-fog-start 0.0
	initGlobal $this-fog-end 0.714265
	initGlobal $this-fog-visibleonly 1

	setGlobal $this-global-light 1
	setGlobal $this-global-fog 0
	setGlobal $this-global-type Gouraud
	setGlobal $this-global-debug 0
	setGlobal $this-global-clip 1
	setGlobal $this-global-cull 0
	setGlobal $this-global-dl 0
	setGlobal $this-global-movie 0
	setGlobal $this-global-movieName "./movie.%04d"
	setGlobal $this-global-movieUseTimestamp 0
	setGlobal $this-global-movieFrame 0
	setGlobal $this-global-resize 0
	setGlobal $this-global-movieMessage "Waiting ..."
	setGlobal $this-global-sync_with_execute 0
	setGlobal $this-x-resize 700
	setGlobal $this-y-resize 512
	setGlobal $this-do_bawgl 0
	setGlobal $this-tracker_state 0
	setGlobal $this-currentvisual 0
    }

    method bindEvents {w} {
	bind $w <Expose> "$this-c redraw"
	bind $w <Configure> "$this-c redraw 1"

	bind $w <ButtonPress-1> "$this-c mtranslate start %x %y"
	bind $w <Button1-Motion> "$this-c mtranslate move %x %y"
	bind $w <ButtonRelease-1> "$this-c mtranslate end %x %y"
	bind $w <ButtonPress-2> "$this-c mrotate start %x %y %t"
	bind $w <Button2-Motion> "$this-c mrotate move %x %y %t"
	bind $w <ButtonRelease-2> "$this-c mrotate end %x %y %t"
	bind $w <ButtonPress-3> "$this-c mscale start %x %y"
	bind $w <Button3-Motion> "$this-c mscale move %x %y"
	bind $w <ButtonRelease-3> "$this-c mscale end %x %y"

	bind $w <Control-ButtonPress-1> "$this-c mdolly start %x %y"
	bind $w <Control-Button1-Motion> "$this-c mdolly move %x %y"
	bind $w <Control-ButtonRelease-1> "$this-c mdolly end %x %y"
	bind $w <Control-ButtonPress-2> "$this-c mrotate_eyep start %x %y %t"
	bind $w <Control-Button2-Motion> "$this-c mrotate_eyep move %x %y %t"
	bind $w <Control-ButtonRelease-2> "$this-c mrotate_eyep end %x %y %t"
	bind $w <Control-ButtonPress-3> "$this-c municam start %x %y %t"
	bind $w <Control-Button3-Motion> "$this-c municam move %x %y %t"
	bind $w <Control-ButtonRelease-3> "$this-c municam end %x %y %t"

	bind $w <Shift-ButtonPress-1> "$this-c mpick start %x %y %s %b"
	bind $w <Shift-ButtonPress-2> "$this-c mpick start %x %y %s %b"
	bind $w <Shift-ButtonPress-3> "$this-c mpick start %x %y %s %b"
	bind $w <Shift-Button1-Motion> "$this-c mpick move %x %y %s 1"
	bind $w <Shift-Button2-Motion> "$this-c mpick move %x %y %s 2"
	bind $w <Shift-Button3-Motion> "$this-c mpick move %x %y %s 3"
	bind $w <Shift-ButtonRelease-1> "$this-c mpick end %x %y %s %b"
	bind $w <Shift-ButtonRelease-2> "$this-c mpick end %x %y %s %b"
	bind $w <Shift-ButtonRelease-3> "$this-c mpick end %x %y %s %b"
	bind $w <Lock-ButtonPress-1> "$this-c mpick start %x %y %s %b"
	bind $w <Lock-ButtonPress-2> "$this-c mpick start %x %y %s %b"
	bind $w <Lock-ButtonPress-3> "$this-c mpick start %x %y %s %b"
	bind $w <Lock-Button1-Motion> "$this-c mpick move %x %y %s 1"
	bind $w <Lock-Button2-Motion> "$this-c mpick move %x %y %s 2"
	bind $w <Lock-Button3-Motion> "$this-c mpick move %x %y %s 3"
	bind $w <Lock-ButtonRelease-1> "$this-c mpick end %x %y %s %b"
	bind $w <Lock-ButtonRelease-2> "$this-c mpick end %x %y %s %b"
	bind $w <Lock-ButtonRelease-3> "$this-c mpick end %x %y %s %b"

    }

    method setWindow { w width height } {
	set renderWindow $w
	if {[winfo exists $renderWindow]} {
	    destroy $renderWindow
	} 

	upvar \#0 $this-currentvisual visual
	$this-c setgl $renderWindow $visual $width $height
	bindEvents $renderWindow
	$this-c startup
    }

    method addObject {objid name} {
	initGlobal "$this-$objid-useglobal" 1
	initGlobal "$this-$objid-light" 1
	initGlobal "$this-$objid-fog" 0
	initGlobal "$this-$objid-debug" 0
	initGlobal "$this-$objid-clip" 1
	initGlobal "$this-$objid-cull" 0
	initGlobal "$this-$objid-dl" 0
	initGlobal "$this-$objid-type" Gouraud
	global ModuleSavedVars
	set vid [$viewer modname]
	foreach state {type light fog debug clip cull dl} {
	    lappend ModuleSavedVars($vid) "ViewWindow[number]-$objid-$state"
	}
    }

	
    method makeSaveImagePopup {} {
	upvar \#0 $this-resx resx $this-resy resy
	set resx [winfo width $renderWindow]
	set resy [winfo height $renderWindow]
	
	set w .ui[modname]-saveImage
	if {[winfo exists $w]} {
	    SciRaise $w
	    return
        }
	toplevel $w -class TkFDialog

	set initdir [pwd]
	set defext "" ;# extension to append if no extension supplied by user
	set defname "MyImage.png" ;# filename to appear initially
	set title "Save ViewWindow Image"

	# file types to appers in filter box
	set types {
	    {{All Files}    {.*}}
	    {{PPM File}     {.ppm}}
	    {{Raw File}     {.raw}}
      {{PNG File}     {.png}}
	}
	
	makeSaveFilebox \
	    -parent $w \
	    -filevar $this-saveFile \
	    -command "$this doSaveImage; wm withdraw $w" \
      -cancel "wm withdraw $w" \
      -commandname Save \
	    -title $title \
	    -filetypes $types \
	    -initialfile $defname \
	    -initialdir $initdir \
	    -defaultextension $defext \
	    -formatvar $this-saveType \
	    -formats {png ppm raw "by_extension"} \
	    -imgwidth $this-resx \
	    -imgheight $this-resy \
            -imgscale $this-resScale
	moveToCursor $w
	wm deiconify $w
    }

    method doSaveImage {} {
	upvar \#0 $this-saveFile file $this-saveType type
	upvar \#0 $this-resx resx $this-resy resy
        $this-c dump_viewwindow $file $type \
	    [expr $resx * [set $this-resScale]] \
            [expr $resy * [set $this-resScale]]
	$this-c redraw
    }

    method updateMode {args} {}
    method updatePerf {args} {}
    method removeObject {objid} { 
      global ModuleSavedVars
      set vid [$viewer modname]
      foreach state {type light fog debug clip cull dl} {
          listFindAndRemove ModuleSavedVars($vid) "ViewWindow[number]-$objid-$state"
      }
    }

    method makeClipPopup {} {
	set w .clip[modname]
	if {[winfo exists $w]} {
	    SciRaise $w
	    return
	}
	toplevel $w
	wm title $w "Clipping Planes"
	wm minsize $w 200 100 

	initGlobal $this-clip-num 6
	initGlobal $this-clip-selected 1
	for {set i 1} {$i <= 6} {incr i 1} {
	    initGlobal $this-clip-visible-$i 0
            initGlobal $this-clip-frame-$i 0
            initGlobal $this-clip-normal-reverse-$i 0
	    initGlobal $this-clip-normal-d-$i 0.0
	    initGlobal $this-clip-normal-x-$i 1.0
	    initGlobal $this-clip-normal-y-$i 0.0
	    initGlobal $this-clip-normal-z-$i 0.0
	}

	set menup [tk_optionMenu $w.which $this-clip-selected 1 2 3 4 5 6]
	for {set i 0}  {$i < 6} {incr i} {
	    $menup entryconfigure $i -command "$this useClip"
	}
	
	pack $w.which
	checkbutton $w.visibile -text "Visible" -variable $this-clip-visible \
	    -relief flat -command "$this setClip;$this-c redraw"
	pack $w.visibile

        checkbutton $w.widget -text "Show clipping plane frame" \
            -variable $this-clip-frame -relief flat \
            -command "$this setClip; $this-c redraw" -state disabled
        pack $w.widget

	makePlane $w.normal "Plane Normal" $this-clip-normal \
	    "$this setClip ; $this-c redraw"
	pack $w.normal -side left -expand yes -fill x
	frame $w.f -relief groove -borderwidth 2
	pack $w.f -expand yes -fill x

	useClip
    }
    method updateClipFrame {} {
        upvar \#0 $this-clip-selected cs
        $this-c clipFrame $cs
    }


    method useClip {} {
	upvar \#0 $this-clip-selected cs
	upvar \#0 $this-clip-normal-x-$cs x $this-clip-normal-y-$cs y
	upvar \#0 $this-clip-normal-z-$cs z $this-clip-normal-d-$cs d
	upvar \#0 $this-clip-normal-reverse-$cs reverse
 	upvar \#0 $this-clip-visible-$cs visible
        upvar \#0 $this-clip-frame-$cs widget

	setGlobal $this-clip-normal-x $x
	setGlobal $this-clip-normal-y $y
	setGlobal $this-clip-normal-z $z
        setGlobal $this-clip-normal-reverse $reverse
	setGlobal $this-clip-visible  $visible
        setGlobal $this-clip-frame  $widget
#	.clip[modname].normal.e newvalue $d
    }

    method setClip {} {
        set w .clip[modname]
	upvar \#0 $this-clip-selected cs
	upvar \#0 $this-clip-normal-x x $this-clip-normal-y y
	upvar \#0 $this-clip-normal-z z $this-clip-normal-d d
        upvar \#0 $this-clip-normal-reverse reverse
 	upvar \#0 $this-clip-visible visible
        upvar \#0 $this-clip-frame widget

        if { $visible == 1 } {
            $w.widget configure -state normal
        } else {
            $w.widget configure -state disabled
            set widget 0
        }

	setGlobal $this-clip-normal-x-$cs $x
	setGlobal $this-clip-normal-y-$cs $y
	setGlobal $this-clip-normal-z-$cs $z
	setGlobal $this-clip-normal-d-$cs $d
        setGlobal $this-clip-normal-reverse-$cs  $reverse
	setGlobal $this-clip-visible-$cs  $visible
        setGlobal $this-clip-frame-$cs  $widget

        $this updateClipFrame
    }



}



itcl_class ViewWindow {
    inherit BaseViewWindow

    # parameters to hold current state of detachable part
    protected IsAttached 1
    protected IsDisplayed 0
    # hold names of detached and attached windows
    protected detachedFr ""
    protected attachedFr ""

    method set_traces {} {
	trace variable $this-global-light0 w "$this traceLight 0"
	trace variable $this-global-light1 w "$this traceLight 1"
	trace variable $this-global-light2 w "$this traceLight 2"
	trace variable $this-global-light3 w "$this traceLight 3"
	
	initGlobal $this-geometry [wm geometry .ui[modname]]
	trace variable $this-geometry w "$this traceGeom"
        trace variable $this-lock-view-window w "$this updatelock" 
    }

    destructor {
	destroy .ui[modname]
    }

  method updatelock {args} {
    global $this-lock-view-window    
  
    set w .ui[modname]
    if {[winfo exists $w]} {
      set bsframe [$w.bsframe childsite]
      if {[set $this-lock-view-window] == 0} {
        $bsframe.lock configure -text ""
      } else {
        $bsframe.lock configure -text "L"        
      }
    }
  
  }

  method changelock {} {
    global $this-lock-view-window
          
    if {[set $this-lock-view-window] == 0} {
      set $this-lock-view-window 1
    } else {
      set $this-lock-view-window 0
    }
  }

  method changeorientation {} {
    global $this-raxes
          
    if {[set $this-raxes] == 0} {
      set $this-raxes 1
    } else {
      set $this-raxes 0
    }
  }

  method changeaxes {} {
    global $this-caxes
          
    if {[set $this-caxes] == 0} {
      set $this-caxes 1
    } else {
      set $this-caxes 0
    }
  }
  
  method changewire {} {
    global $this-global-type
          
    if {[set $this-global-type] == "Wire"} {
      set $this-global-type "Gouraud"
    } else {
      set $this-global-type "Wire"
    }
  }

  method changeflat {} {
    global $this-global-type
          
    if {[set $this-global-type] == "Flat"} {
      set $this-global-type "Gouraud"
    } else {
      set $this-global-type "Flat"
    }
  }

  method changeortho {} {
    global $this-ortho-view
          
    if {[set $this-ortho-view] == 0} {
      set $this-ortho-view 1
    } else {
      set $this-ortho-view 0
    }
  }

  method changelight {} {
    global $this-global-light
          
    if {[set $this-global-light] == 0} {
      set $this-global-light 1
    } else {
      set $this-global-light 0
    }
  }

  method changefog {} {
    global $this-global-fog
          
    if {[set $this-global-fog] == 0} {
      set $this-global-fog 1
    } else {
      set $this-global-fog 0
    }
  }
  
  method changebbox {} {
    global $this-global-debug
          
    if {[set $this-global-debug] == 0} {
      set $this-global-debug 1
    } else {
      set $this-global-debug 0
    }
  }
  
  method changeclip {} {
    global $this-global-clip
          
    if {[set $this-global-clip] == 0} {
      set $this-global-clip 1
    } else {
      set $this-global-clip 0
    }
  }

  method changecull {} {
    global $this-global-cull
          
    if {[set $this-global-cull] == 0} {
      set $this-global-cull 1
    } else {
      set $this-global-cull 0
    }
  }  

  method changestereo {} {
    global $this-do_stereo
          
    if {[set $this-do_stereo] == 0} {
      set $this-do_stereo 1
    } else {
      set $this-do_stereo 0
    }
  }  
  
  method showhelpwindow {} {
    set w .ui[modname]-help
  
    if {[winfo exists $w]} {
        return
    }    

    toplevel $w
    
    frame $w.f
    pack $w.f

    set keys { "1 - 8" "Preprogrammed views aligning the view with the X, Y, or Z-axis" \
               "0" "Find a view that shows all the data (Autoview)" \
               "CTRL 0" "Snap to the closest axis-aligned view" \
               "CTRL 1-9" "Copy view from Viewer Window 1-9" \
               "CTRL H" "Store the current view (Set Home)" \
               "H" "Go back to the stored view (Goto Home)" \
               "A" "Switch axes on/off" \
               "B" "Switch bounding box mode on/off" \
               "C" "Switch clipping on/off" \
               "D" "Switch fog on/off" \
               "F" "Switch flat shading on/off" \
               "I" "Obtain this help window" \
               "L" "Switch view locking on/off" \
               "K" "Switch lighting on/off" \
               "O" "Switch orientation icon on/off" \
               "P" "Switch orthographic projection on/off" \
               "S" "Switch stereo mode on/off" \
               "U" "Switch backculling on/off" \
               "W" "Switch wire frame on/off" \
             }
    
    set i 0
    foreach {key expl} $keys {
        label $w.f.k$i -text $key
        label $w.f.c$i -text " : "
        label $w.f.e$i -text $expl
        grid $w.f.k$i -row $i -column 0 -sticky w
        grid $w.f.c$i -row $i -column 1 -sticky w
        grid $w.f.e$i -row $i -column 2 -sticky w    
      incr i
    }
    
    button $w.close -text "Close" -command "wm withdraw $w"
    pack $w.close -side bottom -anchor s
  }
  
  
  
    constructor {config} {
	set w .ui[modname]

	# create the window 
	toplevel $w

	# (immediately withdraw it so that on the Mac it will size correctly.)
	wm withdraw $w

	wm protocol $w WM_DELETE_WINDOW "$viewer deleteViewWindow $this"
	set title "ViewScene [expr [$viewer number]+1] Window [expr [number]+1]"
	wm title $w $title
	wm iconname $w $title
	wm minsize $w 100 100
	set_traces

  bind $w <KeyPress-0> "$this-c autoview"

  bind $w <KeyPress-1> "setGlobal $this-pos x0_z1 ; $this-c Views"
  bind $w <KeyPress-2> "setGlobal $this-pos x1_z1 ; $this-c Views"
  bind $w <KeyPress-3> "setGlobal $this-pos y0_z1 ; $this-c Views"
  bind $w <KeyPress-4> "setGlobal $this-pos y1_z1 ; $this-c Views"
  bind $w <KeyPress-5> "setGlobal $this-pos z0_x0 ; $this-c Views"
  bind $w <KeyPress-6> "setGlobal $this-pos z1_x0 ; $this-c Views"
  bind $w <KeyPress-7> "setGlobal $this-pos x0_y1 ; $this-c Views"
  bind $w <KeyPress-8> "setGlobal $this-pos x0_y0 ; $this-c Views"

  bind $w <Control-KeyPress-1> "setGlobal $this-pos ViewWindow0 ; $this-c Views"
  bind $w <Control-KeyPress-2> "setGlobal $this-pos ViewWindow1 ; $this-c Views"
  bind $w <Control-KeyPress-3> "setGlobal $this-pos ViewWindow2 ; $this-c Views"
  bind $w <Control-KeyPress-4> "setGlobal $this-pos ViewWindow3 ; $this-c Views"
  bind $w <Control-KeyPress-5> "setGlobal $this-pos ViewWindow4 ; $this-c Views"
  bind $w <Control-KeyPress-6> "setGlobal $this-pos ViewWindow5 ; $this-c Views"
  bind $w <Control-KeyPress-7> "setGlobal $this-pos ViewWindow6 ; $this-c Views"
  bind $w <Control-KeyPress-8> "setGlobal $this-pos ViewWindow7 ; $this-c Views"
  bind $w <Control-KeyPress-9> "setGlobal $this-pos ViewWindow8 ; $this-c Views"
  bind $w <Control-KeyPress-0> "setGlobal $this-pos closest ; $this-c Views"

  bind $w <KeyPress-l> "$this changelock"
  bind $w <KeyPress-o> "$this changeorientation ; $this-c redraw"
  bind $w <KeyPress-a> "$this changeaxes ; $this-c redraw"
  bind $w <KeyPress-w> "$this changewire ; $this-c redraw"
  bind $w <KeyPress-f> "$this changeflat ; $this-c redraw"
  bind $w <KeyPress-p> "$this changeortho ; $this-c redraw"
  bind $w <KeyPress-k> "$this changelight ; $this-c redraw"
  bind $w <KeyPress-d> "$this changefog ; $this-c redraw"
  bind $w <KeyPress-b> "$this changebbox ; $this-c redraw"  
  bind $w <KeyPress-c> "$this changeclip ; $this-c redraw"
  bind $w <KeyPress-u> "$this changecull ; $this-c redraw"
  bind $w <KeyPress-s> "$this changestereo ; $this-c redraw"
  bind $w <Control-KeyPress-h> "$this-c sethome"
  bind $w <KeyPress-h> "$this-c gohome"
  bind $w <KeyPress-i> "$this showhelpwindow"

	frame $w.menu -relief raised -borderwidth 3
	pack $w.menu -fill x

	menubutton $w.menu.file -text "File" -underline 0 \
		-menu $w.menu.file.menu
	menu $w.menu.file.menu
	$w.menu.file.menu add command -label "Save Image..." \
	    -underline 0 -command "$this makeSaveImagePopup"
	$w.menu.file.menu add command -label "Record Movie..." \
	    -underline 0 -command "$this makeSaveMoviePopup"
	$w.menu.file.menu add command -label "Keyboard shortcuts" \
	    -underline 0 -command "$this showhelpwindow"

	frame $w.wframe -borderwidth 3 -relief sunken
	pack $w.wframe -side bottom -expand yes -fill both -padx 4 -pady 4



	# Edit Menu
	set editmenu $w.menu.edit.menu
	menubutton $w.menu.edit -text "Edit" -underline 0 -menu $editmenu
	menu $editmenu
	$editmenu add command -label "View/Camera..." -underline 0 \
		-command "$this makeViewPopup"
	$editmenu add command -label "Light Sources..." -underline 0 \
	    -command "$this makeLightSources"
	$editmenu add command -label "Background..." -underline 0 \
		-command "$this makeBackgroundPopup"
	$editmenu add command -label "Clipping Planes..." -underline 0 \
	    -command "$this makeClipPopup"
	$editmenu add command -label "Point Size..." -underline 0 \
		-command "$this makePointSizePopup"
	$editmenu add command -label "Line Width..." -underline 0 \
		-command "$this makeLineWidthPopup"
	$editmenu add command -label "Polygon Offset..." -underline 0 \
		-command "$this makePolygonOffsetPopup"
	$editmenu add command -label "Scene Materials..." -underline 0 \
		-command "$this makeSceneMaterialsPopup"
	$editmenu add command -label "Fog Controls..." -underline 0 \
		-command "$this makeFogControlsPopup"

	# Open-GL Visual Menu
	menubutton $w.menu.visual -text "Visual" -underline 0 \
    -menu $w.menu.visual.menu
	menu $w.menu.visual.menu
	set i 0
	upvar \#0 $this-currentvisual visual
	foreach t [$this-c listvisuals] {
	    $w.menu.visual.menu add radiobutton -value $i -label $t \
		-variable $this-currentvisual \
		-font "-Adobe-Helvetica-bold-R-Normal-*-12-75-*" \
		-command "$this switchvisual"
	    incr i
	}


	# New ViewWindow button
	button $w.menu.newviewer -text "NewWindow" \
	    -command "$viewer addViewer [modname]" -borderwidth 0
	
	pack $w.menu.file -side left
	pack $w.menu.edit -side left
	pack $w.menu.visual -side left
	pack $w.menu.newviewer -side left

	# create the scrolled frame
	iwidgets::scrolledframe $w.bsframe -width 640 -height 90 \
		-vscrollmode none -hscrollmode dynamic \
		-sbwidth 10 -relief groove
	pack $w.bsframe -side bottom -before $w.wframe -anchor w -fill x

	# get the childsite to add stuff to
	set bsframe [$w.bsframe childsite]

	# Performance Stats Report window
	frame $bsframe.pf
	pack $bsframe.pf -side left -anchor n
	label $bsframe.pf.perf1 -width 32 -text "? polygons in ? seconds"
	pack $bsframe.pf.perf1 -side top -anchor n
	label $bsframe.pf.perf2 -width 32 -text "? polygons/second"
	pack $bsframe.pf.perf2 -side top -anchor n
	label $bsframe.pf.perf3 -width 32 -text "? frames/sec"
	pack $bsframe.pf.perf3 -side top -anchor n

	# Mouse Mode Report Window
	canvas $bsframe.mousemode -width 175 -height 60 \
		-relief groove -borderwidth 2
	pack $bsframe.mousemode -side left -fill y -pady 2 -padx 2
	global $bsframe.mousemode.text
	set mouseModeText $bsframe.mousemode.text
	$bsframe.mousemode create text 5 30 -tag mouseModeText \
		-text " Current Mouse Mode " -anchor w

	# View Buttons Frame
	frame $bsframe.v1
	pack $bsframe.v1 -side left

	# AutoView Button
	button $bsframe.v1.autoview -text "Autoview" \
	    -command "$this-c autoview" -width 10
	pack $bsframe.v1.autoview -fill x -pady 2 -padx 2
	Tooltip $bsframe.v1.autoview \
           "Instructs the ViewScene to move the camera to a position that will\n"\
           "allow all geometry to be rendered visibly in the viewing window."

	# Views... Menu Button
	frame $bsframe.v1.views             
	pack $bsframe.v1.views -side left -anchor nw -fill x -expand 1
	menubutton $bsframe.v1.views.def -text "Views..." \
	    -menu $bsframe.v1.views.def.m \
	    -relief raised -padx 2 -pady 2 -width 10	
	Tooltip $bsframe.v1.views.def \
	  "Allows the user to easily specify that the viewer align the axes\n"\
          "such that they are perpendicular and/or horizontal to the viewer."
	create_view_menu $bsframe.v1.views.def.m
	pack $bsframe.v1.views.def -side left -pady 2 -padx 2 -fill x

	# Set Home View Frame
	frame $bsframe.v2 -relief groove -borderwidth 2
	pack $bsframe.v2 -side left -padx 2 -pady 2
	button $bsframe.v2.sethome -text "Set Home View" -padx 2 \
		-command "$this-c sethome" -width 15
	Tooltip $bsframe.v2.sethome \
	    "Tells the ViewScene to remember the current camera position."	
	button $bsframe.v2.gohome -text "Go home" \
	    -command "$this-c gohome" -width 15
	Tooltip $bsframe.v2.gohome \
	    "Tells the ViewScene to recall the last saved camera position."
	pack $bsframe.v2.sethome $bsframe.v2.gohome \
	    -side top -fill x -pady 2 -padx 4

	# Settings Frame button
	button $bsframe.more -text "+" -padx 3 \
		-font "-Adobe-Helvetica-bold-R-Normal-*-12-75-*" \
		-command "$this addMFrame $w"
	Tooltip $bsframe.more \
	    "Shows/hides the ViewScene's geometry settings panel."
	pack $bsframe.more -pady 2 -padx 2 -anchor se -side bottom

	# Detach Settings Frame button
	button $bsframe.detach -text "\[+\]" -padx 3 \
		-font "-Adobe-Helvetica-bold-R-Normal-*-12-75-*" \
		-command "$this switch_frames2"
	Tooltip $bsframe.detach \
	    "Detaches the ViewScene's geometry settings panel."
	pack $bsframe.detach -pady 2 -padx 2 -anchor se -side bottom

	# Locking Indicator
	label $bsframe.lock -text "L" -padx 3 \
		-font "-Adobe-Helvetica-bold-R-Normal-*-12-75-*" 
	Tooltip $bsframe.lock \
	    "L indicates that the viewer is locked"
	pack $bsframe.lock -pady 2 -padx 2 -anchor se -side top

  if {[set $this-lock-view-window] == 0} {
    $bsframe.lock configure -text ""
  } else {
    $bsframe.lock configure -text "L"        
  }


	# Initialization of attachment
	toplevel $w.detached
	frame $w.detached.f
	pack $w.detached.f -side top -anchor w -fill y -expand yes
	
	wm title $w.detached "VIEWWINDOW settings"
	wm sizefrom  $w.detached user
	wm positionfrom  $w.detached user
	wm protocol $w.detached WM_DELETE_WINDOW "$this removeMFrame $w"
	wm withdraw $w.detached
	
	# This is the frame for the geometry controls
	iwidgets::scrolledframe $w.msframe -width 640 -height 320 \
	    -vscrollmode dynamic -hscrollmode dynamic \
	    -sbwidth 10 -relief groove

	# get the childsite to add stuff to
	set msframe [$w.msframe childsite]

	frame $msframe.f -relief solid
	pack $msframe.f -side top

	set IsAttached 1
	set IsDisplayed 0
	
	set detachedFr $w.detached
	set attachedFr $w.msframe
	init_frame $detachedFr.f "Double-click here to attach - - - - - - - - - - - - - - - - - - - - -"
	init_frame $msframe.f "Double-click here to detach - - - - - - - - - - - - - - - - - - - - -"
	# End initialization of attachment
        
	$this-c startup
	
	pack slaves $w

        # To avoid Mac bizarro behavior of not sizing the window correctly
        # this hack is necessary when loading from a script.
        global tcl_platform
#	if { [envBool SCI_REGRESSION_TESTING] && $tcl_platform(os) == "Darwin" } {
           # The added benefit of this is that I can make the ViewScene Window
           # appear after all the other windows and thus on systems without
           # pbuffers, we don't get the drawing window obscured.  Three seconds
           # seems to be enough time.
           # after 3000 "SciRaise $w; $this switchvisual"
#        } else {
            SciRaise $w
            switchvisual
#        }
    }
    # end constructor()

    method create_other_viewers_view_menu { m } {
      if { [winfo exists $m] } {
          destroy $m
      }
      menu $m
      set myparts [split [modname] -]
      set myviewer .ui[lindex $myparts 0]
      set mywindow [lindex $myparts 1]
      set actual 0
      foreach w [winfo children .] {
          set parts [split $w -]
          set viewer_id [lindex $parts 0]
          set window [lindex $parts 1]
          if { [string equal $myviewer $viewer_id] } {
            if { ![string equal $mywindow $window] } {
                set num [lindex [split $window _] end]
                $m add command -label "Get View from Window [expr $num+1]" \
              -command "set $this-pos ViewWindow$actual; \
                                          $this-c Views"
            }
            incr actual
          }
      }
    }
		    
    method create_view_menu { m } {
	menu $m -postcommand \
	    "$this create_other_viewers_view_menu $m.otherviewers"
  $m add checkbutton -label "Lock Viewer Window" -variable $this-lock-view-window
	$m add cascade -menu $m.otherviewers -label "Other Viewer Windows"

	foreach sign1 {1 0} {
	    foreach dir1 {x y z} {
		set pn1 [expr $sign1?"+":"-"]
		set posneg1 [expr $sign1?"+":"-"]
		set sub $m.$posneg1$dir1
		$m add cascade -menu $sub \
		    -label "Look down $pn1[string toupper $dir1] Axis"
		menu $sub
		foreach dir2 { x y z } {
		    if { ![string equal $dir1 $dir2] } {
			foreach sign2 { 1 0 } {
			    set pn2 [expr $sign2?"+":"-"]
			    $sub add command -label \
				"Up vector $pn2[string toupper $dir2]" \
				-command "setGlobal $this-pos ${dir1}${sign1}_${dir2}${sign2};
                                              $this-c Views" 
			}
		    }
		}
	    }
	    $m add separator
	}
    }

    method removeMFrame {w} {
	if { $IsAttached != 0 } {
	    pack forget $attachedFr
	    set height [expr [winfo height $w]-[winfo height $w.msframe]]
	    wm geometry $w [winfo width $w]x${height}
	    update
	} else { 
	    wm withdraw $detachedFr
	}
	
	set bsframe [$w.bsframe childsite]
	$bsframe.more configure -command "$this addMFrame $w" -text "+"
	set IsDisplayed 0
    }
    
    method addMFrame {w} {
	if { $IsAttached!=0} {
	    pack $attachedFr -anchor w -side bottom -before $w.bsframe -fill x
	    set w1 [winfo width $w]
	    set w2 [winfo width $w.msframe]
	    set width [expr $w1 > $w2 ? $w1 : $w2]
	    set height [expr [winfo height $w]+[winfo reqheight $w.msframe]]
	    wm geometry $w ${width}x${height}
	    update
	} else {
	    wm deiconify $detachedFr
	}
	set bsframe [$w.bsframe childsite]
	$bsframe.more configure -command "$this removeMFrame $w" -text "-"
	set IsDisplayed 1
    }

    method init_frame {m msg} {
	if { ![winfo exists $m] } return
	
	bind $m <Double-ButtonPress-1> "$this switch_frames"

	# Label indicating click here to detach/attach
	label $m.cut -anchor w -text $msg -font \
	    "-Adobe-Helvetica-bold-R-Normal-*-12-75-*"
	pack $m.cut -side top -anchor w -pady 5 -padx 5
	bind $m.cut <Double-ButtonPress-1> "$this switch_frames"
	
	frame $m.eframe
	
	# Global Lighting Checkbutton
	checkbutton $m.eframe.light -text "Lighting" \
	    -variable $this-global-light -command "$this-c redraw"
	Tooltip $m.eframe.light \
	    "Toggles on/off whether lights effect the rendering."

	# Global Fog Checkbutton
	checkbutton $m.eframe.fog -text "Fog" \
	    -variable $this-global-fog -command "$this-c redraw"
	Tooltip $m.eframe.fog \
	    "Toggles on/off fog.  This will make objects further\n" \
	    "away from the viewer appear dimmer and make it easier\n" \
	    "to judge distances."

	# Global BBox Checkbutton
	checkbutton $m.eframe.bbox -text "BBox" \
	    -variable $this-global-debug  -command "$this-c redraw"
	Tooltip $m.eframe.bbox \
	    "Toggles on/off whether only the bounding box of every piece\n" \
	    "of geometry is displayed.  Individual bounding boxes may be\n" \
	    "toggled on/off using the 'Options' button in the 'Objects' frame."

	# Global Clip Checkbutton
	checkbutton $m.eframe.clip -text "Use Clip" \
	    -variable $this-global-clip -command "$this-c redraw"
	Tooltip $m.eframe.clip "Toggles on/off whether clipping is enabled."

	# Global Cull Checkbutton
	checkbutton $m.eframe.cull -text "Back Cull" \
	    -variable $this-global-cull -command "$this-c redraw"
	Tooltip $m.eframe.cull \
	    "Toggles on/off whether polygons that face away from\n" \
	    "the camera are rendered."

	# Global Display List Checkbutton
	checkbutton $m.eframe.dl -text "Display List" \
	    -variable $this-global-dl -command "$this-c redraw"
	Tooltip $m.eframe.dl \
	    "Toggles on/off whether GL display lists are used."
	
	pack $m.eframe -anchor n -padx 2 -side left
	pack  $m.eframe.light $m.eframe.fog $m.eframe.bbox $m.eframe.clip \
	    $m.eframe.cull $m.eframe.dl -in $m.eframe -side top -anchor w
	  
	# Render Style Radio Buttons
	frame $m.eframe.separator -relief sunken -borderwidth 4 -height 2
	radiobutton $m.eframe.wire -text Wire -value Wire \
	    -variable $this-global-type -command "$this-c redraw"
	radiobutton $m.eframe.flat -text Flat -value Flat \
	    -variable $this-global-type -command "$this-c redraw"
	radiobutton $m.eframe.gouraud -text Gouraud -value Gouraud \
	    -variable $this-global-type -command "$this-c redraw"

	pack $m.eframe.separator -in $m.eframe \
	    -side top -anchor w -expand y -fill x
	pack $m.eframe.wire $m.eframe.flat $m.eframe.gouraud \
	    -in $m.eframe -side top -anchor w
	    
	# Geometry Objects Options List
	frame $m.objlist -relief groove -borderwidth 2
	pack $m.objlist -side left -padx 2 -pady 2 -fill y -expand yes
	label $m.objlist.title -text "Objects:"
	pack $m.objlist.title -side top
	canvas $m.objlist.canvas -width 370 -height 128 \
	    -scrollregion "0 0 370 128" -borderwidth 0 \
	    -xscrollcommand "$m.objlist.xscroll set" -xscrollincrement 10 \
	    -yscrollcommand "$m.objlist.yscroll set" -yscrollincrement 10
	
	frame $m.objlist.canvas.frame -relief sunken -borderwidth 2
	pack $m.objlist.canvas.frame
	$m.objlist.canvas create window 0 1 \
	    -window $m.objlist.canvas.frame -anchor nw
	
	# Scrollbars for Geometry Objects Options List
	scrollbar $m.objlist.xscroll -relief sunken -orient horizontal \
		-command "$m.objlist.canvas xview"
	scrollbar $m.objlist.yscroll -relief sunken -orient vertical \
		-command "$m.objlist.canvas yview"
	pack $m.objlist.yscroll -fill y -side left -padx 2 -pady 2
	pack $m.objlist.canvas -side top -padx 2 -pady 2 -fill both -expand yes
	pack $m.objlist.xscroll -fill x -side top  -padx 2 -pady 2
	
	# Automatic Autoview when a network loads check button
        checkbutton $m.autoav -text "Autoview on Load" \
	    -variable $this-autoav -onvalue 1 -offvalue 0 
	Tooltip $m.autoav \
	  "Toggles on/off automatic viewer adjustment when new networks load."

	# Show Axes Check Button
        checkbutton $m.caxes -text "Show Axes" -variable $this-caxes \
	    -onvalue 1 -offvalue 0 \
	    -command "$this-c centerGenAxes; $this-c redraw"
	Tooltip $m.caxes \
	    "Toggles on/off the the set of three axes displayed at 0,0,0."

	# Orientation Axes Checkbutton
        checkbutton $m.raxes -text "Orientation" -variable $this-raxes \
	    -onvalue 1 -offvalue 0 -command "$this-c redraw"
	Tooltip $m.raxes \
	    "Toggles on/off the orientation axes displayed in\n" \
	    "the upper right corner of the viewer window."

	# Ortho View Checkbutton
	checkbutton $m.ortho -text "Ortho View" -variable $this-ortho-view \
	    -onvalue 1 -offvalue 0 -command "$this-c redraw"
	Tooltip $m.ortho  \
	    "Toggles on/off the use of an orthographic projection.\n" \
	    "SCIRun defaults to using the prospective projection."

	pack $m.autoav -side top -anchor w
	pack $m.caxes -side top -anchor w
	pack $m.raxes -side top -anchor w
	pack $m.ortho -side top -anchor w
    
	# Stereo View Options
	checkbutton $m.stereo -text "Stereo" -variable $this-do_stereo \
		-command "$this-c redraw"
	Tooltip $m.stereo \
	    "Switch into stereo rendering mode.  Special hardware may be\n" \
	    "necessary to use this function."
	pack $m.stereo -side top -anchor w

	# Stereo Fusion Scale
	scale $m.sbase -variable $this-sbase -length 100 -from 0.1 -to 2 \
		-resolution 0.02 -orient horizontal -label "Fusion Scale:" \
		-command "$this-c redraw"
	Tooltip $m.sbase \
	    "Specifies how far the left and right eye images are\n" \
	    "offset when rendering in stereo mode."
	pack $m.sbase -side top -anchor w

        # Inertia frame
        frame $m.inertia -relief groove -bd 2
        pack $m.inertia -side top -fill x -expand yes

        # Auto Rotate
        checkbutton $m.inertia.autorotate -text "Auto Rotate" \
            -variable $this-inertia_mode -command \
            "setGlobal $this-inertia_recalculate 1; $this-c redraw"
        pack $m.inertia.autorotate -side top

        # Inertia Direction Buttons
        frame $m.inertia.buttons -relief flat
        pack $m.inertia.buttons -expand yes -fill x -side top
        #
        button $m.inertia.buttons.up -text "Up" -padx 3 \
            -command "$this inertiaGo  0  1"
        pack $m.inertia.buttons.up -side top
        #
        frame $m.inertia.buttons.lr -relief flat
        pack $m.inertia.buttons.lr -side top -expand yes -fill x
        button $m.inertia.buttons.lr.left -text "Left" -padx 3 \
            -command "$this inertiaGo -1  0"
        button $m.inertia.buttons.lr.right -text "Right" -padx 3 \
            -command "$this inertiaGo  1  0"
        pack $m.inertia.buttons.lr.left -side left -anchor w -expand yes -fill x
        pack $m.inertia.buttons.lr.right -side left -anchor e -expand yes -fill x
        #
        button $m.inertia.buttons.down -text "Down" -padx 3 \
            -command "$this inertiaGo  0 -1"
        pack $m.inertia.buttons.down -side bottom

        # Frames per Second slider
        scale $m.inertia.loop_count -label "Frames per rotation" \
            -from 4 -to 360 -showvalue 45 \
            -variable $this-inertia_loop_count -resolution 1 -orient horizontal \
            -command "setGlobal $this-inertia_recalculate 1;\#"
        pack $m.inertia.loop_count -expand yes -fill x -side top

    }

    method resize {} {
	set w .ui[modname]
	set wmovie .ui[modname]-saveMovie

	if { [set $this-global-resize] == 0 } {
	    wm geometry $w "="
	    pack configure $w.wframe -expand yes -fill both

	    set color "#505050"
            $wmovie.resize_f.x configure -foreground $color
            $wmovie.resize_f.e1 configure -state disabled -foreground $color
            $wmovie.resize_f.e2 configure -state disabled -foreground $color
	} else {
	    if { $IsAttached == 1 } { $this switch_frames }
	    #Findout the current window sizes
	    set winx [winfo width $w]
	    set winy [winfo height $w]
	    set resx [winfo width $renderWindow]
	    set resy [winfo height $renderWindow]

	    set xsize [set $this-x-resize]
	    set ysize [set $this-y-resize]
	    set size "$xsize\x$ysize"

	    #Dynamically calculate the offset for 
	    #what is outside the render window
	    set xsize [expr $xsize + $winx-$resx]
	    set ysize [expr $ysize + $winy-$resy]
	    set geomsize "$xsize\x$ysize"
	    wm geometry $w "=$geomsize"

	    pack configure $w.wframe -expand yes -fill both
	    $wmovie.resize_f.x configure -foreground black
	    $wmovie.resize_f.e1 configure -state normal -foreground black
	    $wmovie.resize_f.e2 configure -state normal -foreground black
	}
    }

    method switch_frames {} {
	set w .ui[modname]
	if { $IsDisplayed } {
	    if { $IsAttached!=0} {
		pack forget $attachedFr
		set hei [expr [winfo height $w]-[winfo reqheight $w.msframe]]
		append geom [winfo width $w]x${hei}
		wm geometry $w $geom
		wm deiconify $detachedFr
		set IsAttached 0
	    } else {
		wm withdraw $detachedFr
		pack $attachedFr -anchor w -side bottom \
		    -before $w.bsframe -fill x
		set hei [expr [winfo height $w]+[winfo reqheight $w.msframe]]
		append geom [winfo width $w]x${hei}
		wm geometry $w $geom
		set IsAttached 1
	    }
	    update
	}
    }

    method switch_frames2 {} {
	set w .ui[modname]
	if { !$IsDisplayed } {
	    set IsAttached 0
	    set IsDisplayed 1
	    wm deiconify $detachedFr
	    update
	}
    }

    method updatePerf { p1 p2 p3 } {
	set w .ui[modname]
	set bsframe [$w.bsframe childsite]
	$bsframe.pf.perf1 configure -text $p1
	$bsframe.pf.perf2 configure -text $p2
	$bsframe.pf.perf3 configure -text $p3
    }

    method switchvisual {} {
	upvar \#0 $this-currentvisual visual
	set w .ui[modname]
	set renderWindow $w.wframe.draw
	if { [winfo exists $renderWindow] } {
	    destroy $renderWindow
	}
	$this-c setgl $renderWindow $visual
	if { [winfo exists $renderWindow] } {
	    bindEvents $renderWindow
	    pack $renderWindow -expand yes -fill both
	}
    }	

    method makeViewPopup {} {
	set w .view[modname]

	if { [winfo exists $w] } {
	    SciRaise $w
	    return
	}

	toplevel $w
	wm title $w "View"
	wm iconname $w view
	wm minsize $w 100 100
	set view $this-view
	makePoint $w.eyep "Eye Point" $view-eyep "$this-c redraw"
	pack $w.eyep -side left -expand yes -fill x
	makePoint $w.lookat "Look at Point" $view-lookat "$this-c redraw"
	pack $w.lookat -side left -expand yes -fill x
	makeNormalVector $w.up "Up Vector" $view-up "$this-c redraw"
	pack $w.up -side left -expand yes -fill x
	global $view-fov
	frame $w.f -relief groove -borderwidth 2
	pack $w.f
	scale $w.f.fov -label "Field of View:"  -variable $view-fov \
	    -orient horizontal -from 0 -to 180 -tickinterval 90 -digits 3 \
	    -showvalue true -command "$this-c redraw"
	    
	pack $w.f.fov -expand yes -fill x
    }

    method makeSceneMaterialsPopup {} {
	set w .scenematerials[modname]
	if {[winfo exists $w]} {
	    SciRaise $w
	    return
	}

	toplevel $w
	wm title $w "Scene Materials"
	wm iconname $w materials
	wm minsize $w 100 100

	foreach property {ambient diffuse specular shininess emission} {
	    frame $w.$property
	    set text "[string totitle $property] Scale"
	    label $w.$property.l -width 16 -text $text -anchor w
	    entry $w.$property.e -relief sunken -width 6 \
		-textvariable $this-$property-scale
	    bind $w.$property.e <Return> "$this-c redraw"
	    pack $w.$property.l $w.$property.e -side left -fill x
	    pack $w.$property -side top -fill both
	}
    }

    method makeFogControlsPopup {} {
	set w .fogcontrols[modname]
	if [winfo exists $w] {
	    SciRaise $w
	    return
	}
	toplevel $w
	wm title $w "Fog Controls"
	wm iconname $w fog

	checkbutton $w.vis -text "Compute using only visible objects" \
	    -variable $this-fog-visibleonly -command "$this-c redraw"

	checkbutton $w.usebg -text "Use background color" \
	    -variable $this-fogusebg -command "$this-c redraw"

	set ir [expr int([set $this-fogcolor-r] * 65535)]
	set ig [expr int([set $this-fogcolor-g] * 65535)]
	set ib [expr int([set $this-fogcolor-b] * 65535)]

	frame $w.colorFrame
	frame $w.colorFrame.col -relief ridge -borderwidth 4 \
	    -height 0.8c -width 1.0c \
	    -background [format #%04x%04x%04x $ir $ig $ib]

	button $w.colorFrame.set_color -text "Fog Color" \
            -command "$this makeFogColorPopup"

	pack $w.colorFrame.set_color $w.colorFrame.col -side left -padx 2

	scale $w.start -label "Fog Start:" -command "$this-c redraw" \
	    -variable $this-fog-start -orient horizontal -from 0 -to .5 \
	    -resolution 0.01 -showvalue true -tickinterval 1 -digits 3

	scale $w.end -label "Fog End:" -command "$this-c redraw" \
	    -variable $this-fog-end -orient horizontal -from 0 -to 1.75 \
	    -resolution 0.01 -showvalue true -tickinterval 1 -digits 3
	    
	pack $w.vis $w.usebg $w.colorFrame $w.start $w.end \
	    -side top -padx 4 -pady 4 -anchor w
	pack $w.start $w.end -side top -padx 4 -pady 4 -anchor w -fill x
    }

    method makeFogColorPopup {} {
	set w .fogcolor[modname]
	if [winfo exists $w] {
	    SciRaise $w
	    return
	}

	makeColorPicker $w $this-fogcolor \
            "$this updateFogColor; $this-c redraw" "destroy $w"
	wm title $w "Choose Fog Color"
    }

    method updateFogColor {} {
	set w .fogcontrols[modname]
	
	set ir [expr int([set $this-fogcolor-r] * 65535)]
	set ig [expr int([set $this-fogcolor-g] * 65535)]
	set ib [expr int([set $this-fogcolor-b] * 65535)]
	
	$w.colorFrame.col config -background [format #%04x%04x%04x $ir $ig $ib]
    }

    method makeBackgroundPopup {} {
	set w .bg[modname]
	if [winfo exists $w] {
	    SciRaise $w
	    return
	}
	makeColorPicker $w $this-bgcolor "$this-c redraw" "destroy $w"
	wm title $w "Choose Background Color"
    }

    method updateMode {msg} {
	set bsframe [.ui[modname].bsframe childsite]
	$bsframe.mousemode itemconfigure mouseModeText -text $msg
    }   

    method addObject {objid name} {
	BaseViewWindow::addObject $objid $name
	addObjectToFrame $objid $name $detachedFr
	addObjectToFrame $objid $name [$attachedFr childsite]
    }

    method addObjectToFrame {objid name frame} {
	set w .ui[modname]
	set m $frame.f
	# if the object frame exists already, assume it was pack
	# forgotten by removeObject, just pack it again to show it
	if { [winfo exists $m.objlist.canvas.frame.objt$objid] } {
	    pack $m.objlist.canvas.frame.objt$objid \
		-side top -anchor w -fill x -expand y
	    # I think the next two lines are un-necessary
	    pack $m.objlist.canvas.frame.obj$objid  \
		-in $m.objlist.canvas.frame.objt$objid -side left
	    pack $m.objlist.canvas.frame.menu$objid \
		-in $m.objlist.canvas.frame.objt$objid -side right \
		-padx 1 -pady 1
	    return
	}

	frame $m.objlist.canvas.frame.objt$objid
	checkbutton $m.objlist.canvas.frame.obj$objid -text $name \
		-relief flat -variable "$this-$name" -command "$this-c redraw"
	
	set newframeheight [winfo reqheight $m.objlist.canvas.frame.obj$objid]
	
	set menun $m.objlist.canvas.frame.menu$objid.menu

	menubutton $m.objlist.canvas.frame.menu$objid -text "Options..." \
		-relief raised -menu $menun
	menu $menun

	$menun add checkbutton -label "Use Global Controls" \
	    -variable $this-$objid-useglobal -command "$this-c redraw"

	$menun add separator

	$menun add checkbutton -label Lighting -variable $this-$objid-light \
		-command "$this-c redraw"
	$menun add checkbutton -label BBox -variable $this-$objid-debug \
		-command "$this-c redraw"
	$menun add checkbutton -label Fog -variable $this-$objid-fog \
		-command "$this-c redraw"
	$menun add checkbutton -label "Use Clip" -variable $this-$objid-clip \
		-command "$this-c redraw"
	$menun add checkbutton -label "Back Cull" -variable $this-$objid-cull \
		-command "$this-c redraw"
	$menun add checkbutton -label "Display List" -variable $this-$objid-dl\
		-command "$this-c redraw"

	$menun add separator

	$menun add radiobutton -label Wire -variable $this-$objid-type \
	    -command "$this-c redraw"
	$menun add radiobutton -label Flat -variable $this-$objid-type \
	    -command "$this-c redraw"
	$menun add radiobutton -label Gouraud -variable $this-$objid-type \
	    -command "$this-c redraw"

	pack $m.objlist.canvas.frame.objt$objid \
	    -side top -anchor w -fill x -expand y
	pack $m.objlist.canvas.frame.obj$objid \
	    -in $m.objlist.canvas.frame.objt$objid -side left
	pack $m.objlist.canvas.frame.menu$objid \
	    -in $m.objlist.canvas.frame.objt$objid -side right -padx 1 -pady 1

	update idletasks
	set width [winfo width $m.objlist.canvas.frame]
	set height [lindex [$m.objlist.canvas cget -scrollregion] end]

	incr height [expr $newframeheight+20]

	$m.objlist.canvas configure -scrollregion "0 0 $width $height"

	set view [$m.objlist.canvas xview]
	$m.objlist.xscroll set [lindex $view 0] [lindex $view 1]

	set view [$m.objlist.canvas yview]
	$m.objlist.yscroll set [lindex $view 0] [lindex $view 1]
    }

    method removeObject {objid} {
	removeObjectFromFrame $objid $detachedFr
	removeObjectFromFrame $objid [$attachedFr childsite]
	BaseViewWindow::removeObject $objid
    }

    method removeObjectFromFrame {objid frame} {
	pack forget $frame.f.objlist.canvas.frame.objt$objid
    }

    method makeLineWidthPopup {} {
	set w .lineWidth[modname]
	if {[winfo exists $w]} {
	    SciRaise $w
	    return
	}
	toplevel $w
	wm title $w "Line Width"
	wm minsize $w 250 100
	frame $w.f
	scale $w.f.scale -label "Line Width:" -command "$this-c redraw" \
	    -variable $this-line-width -orient horizontal -from 1 -to 5 \
	    -resolution .1 -showvalue true -tickinterval 1 -digits 0
	    
	pack $w.f.scale -fill x -expand 1
	pack $w.f -fill x -expand 1
    }	

    method makePolygonOffsetPopup {} {
	set w .polygonOffset[modname]
	if {[winfo exists $w]} {
	    SciRaise $w
	    return
	}
	toplevel $w
	wm title $w "Polygon Offset"
	wm minsize $w 250 100
	frame $w.f
	scale $w.f.factor -label "Offset Factor:" -command "$this-c redraw" \
	    -variable $this-polygon-offset-factor \
	    -orient horizontal -from -4 -to 4 \
	    -resolution .01 -showvalue true -tickinterval 2 -digits 3
	    
	scale $w.f.units -label "Offset Units:" -command "$this-c redraw" \
	    -variable $this-polygon-offset-units \
	    -orient horizontal -from -4 -to 4 -resolution .01 \
	    -showvalue true -tickinterval 2 -digits 3
	    
	pack $w.f.factor -fill x -expand 1
	pack $w.f -fill x -expand 1
    }	

    method makePointSizePopup {} {
	set w .psize[modname]
	if {[winfo exists $w]} {
	    SciRaise $w
	    return
	}
	toplevel $w
	wm title $w "Point Size"
	wm minsize $w 250 100 
	frame $w.f
	scale $w.f.scale -label "Pixel Size:" -command "$this-c redraw" \
	    -variable $this-point-size -orient horizontal -from 1 -to 5 \
	    -resolution .1 -showvalue true -tickinterval 1 -digits 0
	    
	pack $w.f.scale -fill x -expand 1
	pack $w.f -fill x -expand 1
    }	

    method makeLightSources {} {
	set w .ui[modname]-lightSources

        if {[winfo exists $w]} {
	    SciRaise $w
            return
        }
	toplevel $w ;# create the window        
	wm withdraw $w ;# immediately withdraw it to avoid flicker
        wm title $w "Light Position and Colors"
	frame $w.tf -relief flat
	pack $w.tf -side top
	for {set i 0} {$i < 4} {incr i 1} {
	    $this makeLightControl $w.tf $i
	}

	label $w.l -text \
	    "Click on number to move light. Note: Headlight will not move."
	label $w.o -text \
	    "Click in circle to change light color/brightness"

 	button $w.breset -text "Reset Lights" -command "$this resetLights $w"
	button $w.bclose -text Close -command "destroy $w"
	pack $w.l $w.o $w.breset $w.bclose -side top -expand yes -fill x

	moveToCursor $w "leave_up"
	wm deiconify $w
    }
	
    method makeLightControl { w i } {
	frame $w.f$i -relief flat
	pack $w.f$i -side left
	canvas $w.f$i.c -bg "#BDBDBD" -width 100 -height 100
	pack $w.f$i.c -side top
	set c $w.f$i.c
	checkbutton $w.f$i.b$i -text "on/off" \
	    -variable $this-global-light$i -command "$this lightSwitch $i"
	pack $w.f$i.b$i

	upvar \#0 $this-lightColors lightColors $this-lightVectors lightVectors
	set ir [expr int([lindex [lindex $lightColors $i] 0] * 65535)]
	set ig [expr int([lindex [lindex $lightColors $i] 1] * 65535)]
	set ib [expr int([lindex [lindex $lightColors $i] 2] * 65535)]
       
	set window .ui[modname]
	set color [format "#%04x%04x%04x" $ir $ig $ib]
	set news [$c create oval 5 5 95 95 -outline "#000000" \
		      -fill $color -tags lc ]

	set x [expr int([lindex [lindex $lightVectors $i] 0] * 50) + 50]
	set y [expr int([lindex [lindex $lightVectors $i] 1] * -50) + 50]
	set t  $i
	if { $t == 0 } { set t "HL" }
	set newt [$c create text $x $y -fill "#555555" -text $t -tags lname ]
	$c bind lname <B1-Motion> "$this moveLight $c $i %x %y"
	$c bind lc <ButtonPress-1> "$this lightColor $w $c $i"
	$this lightSwitch $i
    }

    # Sets the axis of rotation based on the input, {x,y}.  If the
    # inertia_mode is 1 and the axis of ratation is the same, then it
    # turns it off.  This allows a user to, for example, press "Rotate
    # left" to turn it on and then off again.  If you press a
    # different direction while the rotation is going it will change
    # to that direction.
    method inertiaGo { x y } {
        if { [set $this-inertia_mode] == 1 } {
            # If inertia_mode is on, we need to determine if the direction is the same.
            set old_inertia_x [set $this-inertia_x]
            set old_inertia_y [set $this-inertia_y]
            if { $old_inertia_x == $x && $old_inertia_y == $y } {
                # Directions are the same so turn it off.
                setGlobal $this-inertia_mode 0
            }
        } else {
            # It was off before, so turn it on now.
            setGlobal $this-inertia_mode 1
        }
            
        setGlobal $this-inertia_x $x
        setGlobal $this-inertia_y $y
        setGlobal $this-inertia_recalculate 1
        $this-c redraw
    }

    method lightColor { w c i } {
	upvar \#0 $this-lightColors lightColors
 	setGlobal $this-def-color-r [lindex [lindex $lightColors $i] 0]
 	setGlobal $this-def-color-g [lindex [lindex $lightColors $i] 1]
 	setGlobal $this-def-color-b [lindex [lindex $lightColors $i] 2]
	if { [winfo exists $w.color] } { destroy $w.color } 
	makeColorPicker $w.color $this-def-color \
	    "$this setColor $w.color $c $i $this-def-color " \
	    "destroy $w.color"
    }

    method setColor { w c i color} {
	upvar \#0 $color-r r $color-g g $color-b b $this-lightColors lColors
	set lColors [lreplace $lColors $i $i "$r $g $b"]
	
	set ir [expr int($r * 65535)]
	set ig [expr int($g * 65535)]
	set ib [expr int($b * 65535)]
	
	set window .ui[modname]
	$c itemconfigure lc -fill [format "#%04x%04x%04x" $ir $ig $ib]
	$this lightSwitch $i
    }
    
    method resetLights { w } {
	upvar \#0 $this-lightColors lCol $this-lightVectors lVec
	for { set i 0 } { $i < 4 } { incr i 1 } {
	    if { $i == 0 } {
		set $this-global-light$i 1
		set c $w.tf.f$i.c
		$c itemconfigure lc -fill \
		    [format "#%04x%04x%04x" 65535 65535 65535 ]
		set lCol [lreplace $lCol $i $i [list 1.0 1.0 1.0]]
		$this lightSwitch $i
	    } else {
		set $this-global-light$i 0
		set coords [$w.tf.f$i.c coords lname]
		set curX [lindex $coords 0]
		set curY [lindex $coords 1]
		set xn [expr 50 - $curX]
		set yn [expr 50 - $curY]
		$w.tf.f$i.c move lname $xn $yn
		set vec [list 0 0 1 ]
		set $this-lightVectors \
		    [lreplace [set $this-lightVectors] $i $i $vec]
		$w.tf.f$i.c itemconfigure lc -fill \
		    [format "#%04x%04x%04x" 65535 65535 65535 ]
		set lightColor [list 1.0 1.0 1.0]
		set $this-lightColors \
		    [lreplace [set $this-lightColors] $i $i $lightColor]
		$this lightSwitch $i
	    }
	}
    }

    method moveLight { c i x y } {
	if { $i == 0 } return
	upvar \#0 $this-global-light$i light 
	upvar \#0 $this-lightColors lCol $this-lightVectors lVec
	set cw [winfo width $c]
	set ch [winfo height $c]
	set selected [$c find withtag current]
	set coords [$c coords current]
	set curX [lindex $coords 0]
	set curY [lindex $coords 1]
	set xn $x
	set yn $y
	set len2 [expr (( $x-50 )*( $x-50 ) + ($y-50) * ($y-50))]
	if { $len2 < 2025 } { 
	    $c move $selected [expr $xn-$curX] [expr $yn-$curY]
	} else { 
	    # keep the text inside the circle
	    set scale [expr 45 / sqrt($len2)]
	    set xn [expr 50 + ($x - 50) * $scale]
	    set yn [expr 50 + ($y - 50) * $scale]
	    $c move $selected [expr $xn-$curX] [expr $yn-$curY]
	}
	set newz [expr ($len2 >= 2025) ? 0 : sqrt(2025 - $len2)]
	set newx [expr $xn - 50]
	set newy [expr $yn - 50]
	# normalize the vector
	set len3 [expr sqrt($newx*$newx + $newy*$newy + $newz*$newz)]
	set vec "[expr $newx/$len3] [expr -$newy/$len3] [expr $newz/$len3]"
	set lVec [lreplace [set $this-lightVectors] $i $i $vec]
	if { $light } {
	    $this lightSwitch $i
	}
    }

    method lightSwitch {i} {
	upvar \#0 $this-global-light$i light $this-lightVectors lVec
	upvar \#0 $this-lightColors lCol
	$this-c edit_light $i $light [lindex $lVec $i] [lindex $lCol $i]
    }

    method traceLight {which name1 name2 op } {
	set w .ui[modname]-lightSources
	if {![winfo exists $w]} {
	    $this lightSwitch $which
	}
    }

    method traceGeom { args } {
	upvar \#0 $this-geometry geometry
	wm geometry .ui[modname] $geometry
    }

    method checkMPGlicense {} {

	# check license env var
	if { [$this-c have_mpeg] && ![envBool SCIRUN_MPEG_LICENSE_ACCEPT]} {
	    tk_messageBox -message "License information describing the mpeg_encode software can be found in SCIRun's Thirdparty directory, in the mpeg_encode/README file.\n\nThe MPEG software is freely distributed and may be used for any non-commercial purpose.  However, patents are held by several companies on various aspects of the MPEG video standard. Companies or individuals who want to develop commercial products that include this code must acquire licenses from these companies. For information on licensing, see Appendix F in the standard. For more information, please see the mpeg_encode README file.\n\nIf you are allowed to use the MPEG functionality based on the above license, you may enable MPEG movie recording in SCIRun (accessible via the SCIRun ViewScene's \"File->Record Movie\" menu) by setting the value of SCIRUN_MPEG_LICENSE_ACCEPT to \"true\". This can be done by uncommenting the reference to the SCIRUN_MPEG_LICENSE_ACCEPT variable in your scirunrc and changing the value from false to true." -type ok -icon info -parent .ui[modname] -title "MPEG License"

	    set $this-global-movie 0
	} else {
	    $this-c redraw
	}
    }

    method makeSaveMoviePopup {} {
	set w .ui[modname]-saveMovie

	if {[winfo exists $w]} {
	   SciRaise $w
           return
        }
	toplevel $w

        wm title $w "Record Movie"

        frame $w.movieType -relief groove -borderwidth 2
	label $w.movieType.l -text "Record Movie as:"

        radiobutton $w.movieType.none -text "Stop Recording" \
            -variable $this-global-movie -value 0 -command "$this-c redraw"
	radiobutton $w.movieType.raw -text "PPM Frames" \
            -variable $this-global-movie -value 1 -command "$this-c redraw"
	radiobutton $w.movieType.png -text "PNG Frames" \
            -variable $this-global-movie -value 3 -command "$this-c redraw"
	radiobutton $w.movieType.mpeg -text "Mpeg" \
	    -variable $this-global-movie -value 2 -command "$this checkMPGlicense"
	
	Tooltip $w.movieType.none "Press to stop recording the movie."
	Tooltip $w.movieType.raw \
           "When pressed, SCIRun will begin recording raw frames as they\n"\
           "are displayed.  The frames are stored in PPM format and can\n" \
           "be merged together into a movie using programs such as Quicktime."
	Tooltip $w.movieType.mpeg \
           "When pressed, SCIRun will begin recording an MPEG'd movie."

	if { ![$this-c have_mpeg] } {
	    $w.movieType.mpeg configure -state disabled -disabledforeground ""
	} 

        checkbutton $w.sync -text "Sync with Execute" \
            -variable $this-global-sync_with_execute

        Tooltip $w.sync \
            "Synchronizes movie frame output with the execution of the\n"\
            "module.  Mouse events and other viewer interaction will\n"\
            "not be recorded."

	frame $w.moviebase
	label $w.moviebase.label -text "File Name:"
        entry $w.moviebase.entry -relief sunken -width 15 -textvariable "$this-global-movieName" 

        TooltipMultiWidget "$w.moviebase.label $w.moviebase.entry" \
            "Name of the movie file.  The %%#d specifies number of digits\nto use in the frame number.  Eg: movie.%%04d will\nproduce names such as movie.0001.png"

	frame $w.movieframe
	label $w.movieframe.label -text "Next Frame No:" -width 15
        entry $w.movieframe.entry -relief sunken -width 6 \
	    -textvariable "$this-global-movieFrame" 

        frame $w.movieUseTimestampFrame
	label $w.movieUseTimestampFrame.label -text "Add timestamp to name:"
        checkbutton $w.movieUseTimestampFrame.button -variable $this-global-movieUseTimestamp \
            -offvalue 0 -onvalue 1

        TooltipMultiWidget "$w.movieUseTimestampFrame.button $w.movieUseTimestampFrame.label" \
            "Appends a timestamp to the name of each (PNG/PPM) frame. This can be useful\nwhen you are dumping frames from multiple different ViewScene windows."

        TooltipMultiWidget "$w.movieframe.label $w.movieframe.entry" \
            "Frame number at which to start numbering generated frames."

        frame $w.resize_f
	checkbutton $w.resize_f.resize -text "Resize: " \
	    -variable $this-global-resize \
	    -offvalue 0 -onvalue 1 -command "$this resize; $this-c redraw"
	entry $w.resize_f.e1 -textvariable $this-x-resize -width 4
	label $w.resize_f.x -text x
	entry $w.resize_f.e2 -textvariable $this-y-resize -width 4
 
        Tooltip $w.resize_f.resize \
            "When selected, the output will be resized to these dimensions."
	bind $w.resize_f.e1 <Return> "$this resize"
	bind $w.resize_f.e2 <Return> "$this resize"

	entry $w.message -textvariable $this-global-movieMessage \
	    -relief flat -width 20 -state disabled
	frame $w.separator -height 2 -relief sunken -borderwidth 2
	button $w.close -width 10 -text "Close" \
	  -command "wm withdraw $w"

        pack $w.movieType -pady 4
        pack $w.movieType.l -padx 4 -anchor w
        pack $w.movieType.none $w.movieType.raw $w.movieType.png $w.movieType.mpeg -padx 4 -anchor w

        puts "here: $w"
        pack $w.sync 

        pack $w.moviebase.label $w.moviebase.entry -side left -padx 4
        pack $w.moviebase -pady 5 -padx 4 -anchor w

        pack $w.movieUseTimestampFrame.label $w.movieUseTimestampFrame.button -side left -padx 4
        pack $w.movieUseTimestampFrame -padx 4 -pady 4 -anchor w

        pack $w.movieframe.label $w.movieframe.entry -side left -padx 4
        pack $w.movieframe -pady 2 -padx 4 -anchor w

	pack $w.resize_f.resize $w.resize_f.e1 $w.resize_f.x $w.resize_f.e2 \
               -side left -pady 5 -padx 4
        pack $w.resize_f -padx 4 -anchor w

	pack $w.message -fill x -padx 4 -pady 5
	pack $w.separator -fill x -pady 5
        pack $w.close -padx 4 -anchor e

	if {[set $this-global-resize] == 0} {
	    set color "#505050"
	    $w.resize_f.x configure -foreground $color
	    $w.resize_f.e1 configure -state disabled -foreground $color
	    $w.resize_f.e2 configure -state disabled -foreground $color
	}
    }
}


