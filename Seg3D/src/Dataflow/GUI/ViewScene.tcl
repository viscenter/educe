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

  method addViewerSameSize { { old_vw "" } w } {
    set i 0
    set rid [makeViewWindowID]

    $this-c addviewwindow $rid
    set width [winfo width $w]
    set height [winfo height $w]
    
    if { [winfo ismapped $w.bframe]} {
      set height [expr $height - [winfo reqheight $w.bframe]]
    }
    
    ViewWindow $rid -viewer $this -xsize $width -ysize $height
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
  public xsize -1
  public ysize -1
  

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
    initGlobal $this-ambient-scale 0.2
    initGlobal $this-diffuse-scale 1.0
    initGlobal $this-specular-scale 0.4
    initGlobal $this-emission-scale 1.0
    initGlobal $this-shininess-scale 1.0

    # Initialize point size, line width, and polygon offset
    initGlobal $this-point-size 1.0
    initGlobal $this-line-width 1.0
    initGlobal $this-polygon-offset-factor 0.0
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
    
    if {[expr $xsize > 0]} {
      set $this-x-resize $xsize   
    } else {
      setGlobal $this-x-resize 700
    }
    if {[expr $ysize > 0]} {
      set $this-y-resize $ysize   
    } else {
      setGlobal $this-y-resize 512
    }
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
  
  method removeObject {objid} { 
    global ModuleSavedVars
    set vid [$viewer modname]
    foreach state {type light fog debug clip cull dl} {
        listFindAndRemove ModuleSavedVars($vid) "ViewWindow[number]-$objid-$state"
    }
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

    if {![info exists x]} {
      set visible 0
      set widget 0
      set reverse 0
      set d 0.0
      set x 1.0
      set y 0.0
      set z 0.0    
    }

    if {[expr [llength [set visible] ] == 0]} {
      set visible 0
    }
    if {[expr [llength [set reverse] ] == 0]} {
      set reverse 0
    }
    if {[expr [llength [set widget] ] == 0]} {
      set widget 0
    }
    if {[expr [llength [set x] ] == 0]} {
      set x 1
    }
    if {[expr [llength [set y] ] == 0]} {
      set y 0
    }
    if {[expr [llength [set z] ] == 0]} {
      set z 0
    }  
    if {[expr [llength [set d] ] == 0]} {
      set d 0
    }  
        
    setGlobal $this-clip-normal-x $x
    setGlobal $this-clip-normal-y $y
    setGlobal $this-clip-normal-z $z
    setGlobal $this-clip-normal-d $d
    setGlobal $this-clip-normal-reverse $reverse
    setGlobal $this-clip-visible  $visible
    setGlobal $this-clip-frame  $widget

  }

  method setClip {} {

    upvar \#0 $this-clip-selected cs
    upvar \#0 $this-clip-normal-x x $this-clip-normal-y y
    upvar \#0 $this-clip-normal-z z $this-clip-normal-d d
    upvar \#0 $this-clip-normal-reverse reverse
    upvar \#0 $this-clip-visible visible
    upvar \#0 $this-clip-frame widget

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
    global $this-lock-view-window Color   
  
    set w .ui[modname]
    if {[winfo exists $w]} {
      if {[set $this-lock-view-window] == 0} {
        $w.bsframe.v1.lock configure -foreground $Color(MenuForeGround) -activeforeground $Color(MenuSelectForeGround)       
      } else {
        $w.bsframe.v1.lock configure -foreground $Color(LockColor) -activeforeground $Color(LockColor)
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
    global Color
    set w .ui[modname]-help
  
    if {[winfo exists $w]} {
      return
    }    

    toplevel $w -background $Color(MainBackGround)
    
    frame $w.f -background $Color(MainBackGround)
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
      label $w.f.k$i -text $key -background $Color(MainBackGround) \
        -foreground $Color(MainForeGround)
      label $w.f.c$i -text " : " -background $Color(MainBackGround) \
        -foreground $Color(MainForeGround)
      label $w.f.e$i -text $expl -background $Color(MainBackGround) \
        -foreground $Color(MainForeGround)
      grid $w.f.k$i -row $i -column 0 -sticky w
      grid $w.f.c$i -row $i -column 1 -sticky w
      grid $w.f.e$i -row $i -column 2 -sticky w    
      incr i
    }
    
    button $w.close -text "Close" -command "wm withdraw $w"\
      -bd $Color(BorderWidth)\
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) \
      -background $Color(ButtonBackGround) \
      -foreground $Color(ButtonForeGround)
    
    pack $w.close -side bottom -anchor s -pady 3
  }
  
  
  
  constructor {config} {

    global Color
  
    set w .ui[modname]

    # create the window 
    toplevel $w

    # (immediately withdraw it so that on the Mac it will size correctly.)
    wm withdraw $w

    wm protocol $w WM_DELETE_WINDOW "$viewer deleteViewWindow $this"
    set title "ViewScene [expr [$viewer number]+1] Window [expr [number]+1]"
    wm title $w $title
    wm iconname $w $title
    wm minsize $w 200 200
    if {[expr $xsize > 0]} {
      wm geometry $w "${xsize}x${ysize}"
    }

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

    frame $w.menu -borderwidth 0 -background $Color(MenuBarBackGround)
    pack $w.menu -fill x

    menubutton $w.menu.file -text "File" -underline 0 \
      -menu $w.menu.file.menu -bd 0\
      -background $Color(MenuBarBackGround) \
      -foreground $Color(MenuBarForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) 

    menu $w.menu.file.menu -bd 2 -activeborderwidth 0 \
      -tearoff false\
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) \
      -background $Color(MenuBackGround) \
      -foreground $Color(MenuForeGround)

    menubutton $w.menu.help -text "Help" -underline 0 \
      -menu $w.menu.help.menu -bd 0\
      -background $Color(MenuBarBackGround) \
      -foreground $Color(MenuBarForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) 

    menu $w.menu.help.menu -bd 2 -activeborderwidth 0 \
      -tearoff false\
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) \
      -background $Color(MenuBackGround) \
      -foreground $Color(MenuForeGround)
    
    $w.menu.file.menu add command -label "Save Image..." \
        -underline 0 -command "$this makeSaveImagePopup"
    $w.menu.file.menu add command -label "Record Movie..." \
        -underline 0 -command "$this makeSaveMoviePopup"
        
    $w.menu.help.menu add command -label "Keyboard shortcuts" \
        -underline 0 -command "$this showhelpwindow"    
   
    
    # Views... Menu Button

    menubutton $w.menu.views -text "Views" \
      -menu $w.menu.views.menu -bd 0\
      -background $Color(MenuBarBackGround) \
      -foreground $Color(MenuBarForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround)

    create_view_menu $w.menu.views.menu

    pack $w.menu.file $w.menu.views $w.menu.help -side left

    frame $w.bsframe -borderwidth 0 -background $Color(MainBackGround)
    pack $w.bsframe -fill x -side bottom

    frame $w.wframe -borderwidth 0
    pack $w.wframe -side bottom -expand yes -fill both -padx 2 -pady 2
    
    # get the childsite to add stuff to
    set bsframe $w.bsframe

    # View Buttons Frame
    frame $bsframe.v1 -background $Color(MainBackGround)
    pack $bsframe.v1 -side left

    # New ViewWindow button
    button $bsframe.v1.newviewer -text "NewWindow"\
      -bd $Color(BorderWidth)\
      -background $Color(ButtonBackGround) \
      -foreground $Color(ButtonForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) \
      -width 10

    pack $bsframe.v1.newviewer -side left -pady 3 -padx 6

    Tooltip $bsframe.v1.newviewer "Right button duplicates this window.\nLeft button duplicates this window and makes the new window of the same size."
   
    bind  $bsframe.v1.newviewer <ButtonPress-3> "$viewer addViewer [modname]"
    bind  $bsframe.v1.newviewer <ButtonPress-2> "$viewer addViewerSameSize [modname] $w"
    bind  $bsframe.v1.newviewer <ButtonPress-1> "$viewer addViewerSameSize [modname] $w"       


    # AutoView Button
    button $bsframe.v1.autoview -text "Autoview" \
      -command "$this-c autoview"\
      -bd $Color(BorderWidth)\
      -background $Color(ButtonBackGround) \
      -foreground $Color(ButtonForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) \
      -width 10
    Tooltip $bsframe.v1.autoview "Tool for automatically finding an angle the displays the full scene."
        
    pack $bsframe.v1.autoview -side left -pady 3 -padx 6

    button $bsframe.v1.home -text "Home" \
      -bd $Color(BorderWidth)\
      -background $Color(ButtonBackGround) \
      -foreground $Color(ButtonForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) \
      -width 10
   Tooltip $bsframe.v1.home "Tool for marking a certain view angle.\nLeft button returns to a preset orientation and scale.\nRight button sets the orientation and scale."
   
    bind  $bsframe.v1.home <ButtonPress-1> "$this-c gohome"
    bind  $bsframe.v1.home <ButtonPress-2> "$this-c sethome"
    bind  $bsframe.v1.home <ButtonPress-3> "$this-c sethome"   

    pack $bsframe.v1.home -side left -pady 3 -padx 6


    # Locking Indicator
    button $bsframe.v1.lock -text "LockView"\
      -bd $Color(BorderWidth)\
      -background $Color(ButtonBackGround) \
      -foreground $Color(ButtonForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) \
      -width 10 \
      -command "$this changelock"

    Tooltip $bsframe.v1.lock "The lock indicates whether rotation and scaling is locked between muliple Viewers."
    if {[set $this-lock-view-window] == 0} {
      $bsframe.v1.lock configure -foreground $Color(MenuForeGround) -activeforeground $Color(MenuSelectForeGround)       
    } else {
      $bsframe.v1.lock configure -foreground $Color(LockColor) -activeforeground $Color(LockColor)
    }
    
    pack $bsframe.v1.lock -side left -padx 6 -pady 3

    # Detach Settings Frame button
    sci_button $bsframe.detach -text "< Configure >" \
      -width 12

    sci_button $bsframe.detachp -text "+"  
      
    Tooltip $bsframe.detach \
        "Shows/hides the configuration tab for this window.\n Left button open settings attached.\nRight button open settings detached."

    bind $bsframe.detach <ButtonPress-1> "$this switch_frames3"
    bind $bsframe.detach <ButtonPress-2> "$this switch_frames"
    bind $bsframe.detach <ButtonPress-3> "$this switch_frames2"
    bind $bsframe.detachp <ButtonPress-1> "$this switch_frames2"

    frame $bsframe.space2 -width 24 -background $Color(MainBackGround)
    pack $bsframe.space2 $bsframe.detachp $bsframe.detach  -side right

    # SETUP THE SETTING WINDOWS CONFIGURATIONS

    # DEFINE THE WINDOW AND AN EMPTY FRAME INSIDE
    toplevel $w.hdetached
    frame $w.hdetached.f -background $Color(MainBackGround)
    pack $w.hdetached.f -side top -anchor n -fill y -expand yes
    
    wm title $w.hdetached "ViewScene Configuration"
    wm sizefrom  $w.hdetached user
    wm positionfrom  $w.hdetached user
    wm protocol $w.hdetached WM_DELETE_WINDOW "$this removeMFrame $w"
    wm withdraw $w.hdetached
    
    # This is the frame for the geometry controls
    iwidgets::scrolledframe $w.bframe -width 350 -height 230 \
      -hscrollmode dynamic\
      -vscrollmode none\
      -background $Color(MainBackGround) \
      -foreground $Color(MainForeGround) -borderwidth 0
      
    set sc [$w.bframe component horizsb]  
    $sc configure -elementborderwidth 1 \
      -bd 1 -troughcolor $Color(Trough) \
      -background $Color(MainBackGround) 

    set sc [$w.bframe component vertsb] 
    $sc configure -elementborderwidth 1 \
      -bd 1 -troughcolor $Color(Trough) \
      -background $Color(MainBackGround) 
        
        
    # get the childsite to add stuff to
    set bframe [$w.bframe childsite]

    frame $bframe.f
    pack $bframe.f -side top -anchor n
    pack $bframe -side top -anchor n

    set IsAttached 1
    set IsDisplayed 0
    
    set detachedFr $w.hdetached
    set attachedFr $w.bframe
    
    init_frame $w.hdetached.f $bframe.f $w
    init_frame $bframe.f $w.hdetached.f $w
    # End initialization of attachment
          
    $this-c startup
    
    pack slaves $w

    SciRaise $w
		switchvisual
	}


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
    global Color
    menu $m -postcommand \
       "$this create_other_viewers_view_menu $m.otherviewers" \
       -tearoff false\
       -bd 2 -activeborderwidth 0 \
       -activebackground $Color(MenuSelectBackGround)  \
       -activeforeground $Color(MenuSelectForeGround) \
       -background $Color(MenuBackGround) \
       -foreground $Color(MenuForeGround)
       
    $m add cascade -menu $m.otherviewers -label "Other Viewer Windows"

    foreach sign1 {1 0} {
      foreach dir1 {x y z} {
        set pn1 [expr $sign1?"+":"-"]
        set posneg1 [expr $sign1?"+":"-"]
        set sub $m.$posneg1$dir1
        $m add cascade -menu $sub \
            -label "Look down $pn1[string toupper $dir1] Axis"
        menu $sub -bd 2 -activeborderwidth 0 \
          -activebackground $Color(MenuSelectBackGround)  \
          -activeforeground $Color(MenuSelectForeGround) \
          -background $Color(MenuBackGround) \
          -foreground $Color(MenuForeGround)
       
        foreach dir2 { x y z } {
          if { ![string equal $dir1 $dir2] } {
            foreach sign2 { 1 0 } {
              set pn2 [expr $sign2?"+":"-"]
              $sub add command -label \
                "Up vector $pn2[string toupper $dir2]" \
                -command "setGlobal $this-pos ${dir1}${sign1}_${dir2}${sign2}; $this-c Views" 
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
        set height [expr [winfo height $w]-[winfo height $w.bframe]]
        wm geometry $w [winfo width $w]x${height}
        update
    } else { 
        wm withdraw $detachedFr
    }
    
    set bsframe $w.bsframe
    $bsframe.detach configure -text "< Configure >"
    $bsframe.detachp configure -text "+"
    bind $bsframe.detach <ButtonPress-1> "$this switch_frames3"
    bind $bsframe.detach <ButtonPress-2> "$this switch_frames"
    bind $bsframe.detach <ButtonPress-3> "$this switch_frames2"
    bind $bsframe.detachp <ButtonPress-1> "$this switch_frames2"
   
    set IsDisplayed 0
  }
    
  method addMFrame {w} {
    if { $IsAttached!=0} {
        pack $attachedFr -anchor w -side bottom -before $w.bsframe -fill x
        set w1 [winfo width $w]
        set w2 [winfo width $w.bframe]
        set width [expr $w1 > $w2 ? $w1 : $w2]
        set height [expr [winfo height $w]+[winfo reqheight $w.bframe]]
        wm geometry $w ${width}x${height}
        update
    } else {
        wm deiconify $detachedFr
    }
    set bsframe $w.bsframe
    $bsframe.detach configure -text "> Configure <"
    $bsframe.detach configure -text "S"
    bind $bsframe.detach <ButtonPress-1> "$this removeMFrame $w"
    bind $bsframe.detach <ButtonPress-2> "$this switch_frames"
    bind $bsframe.detach <ButtonPress-3> "$this removeMFrame $w"
    bind $bsframe.detachp <ButtonPress-1> "$this switch_frames"
   
    set IsDisplayed 1
  }

  method init_frame {m m2 w} {
    if { ![winfo exists $m] } return

    global Color

    frame $m.config -background $Color(UIBackGround) -bd 1 -relief sunken
    pack $m.config -fill x -expand yes -side top

    iwidgets::tabnotebook $m.config.tabs -height 210 -width 600 -tabpos n \
      -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) \
      -backdrop $Color(UIBackDrop) \
      -tabbackground $Color(UIBackDrop) \
      -bevelamount 0 -gap 0 -borderwidth 0 \
      -equaltabs false
      
    $m.config.tabs add -label "Render"
    $m.config.tabs add -label "Objects"
    $m.config.tabs add -label "Clipping"
    $m.config.tabs add -label "Lights"
    $m.config.tabs add -label "Materials"
    $m.config.tabs add -label "View"
    $m.config.tabs add -label "Camera"
    $m.config.tabs select 0

    pack $m.config.tabs -fill x -expand yes

    set render    [$m.config.tabs childsite 0]
    set objects   [$m.config.tabs childsite 1]
    set clipping  [$m.config.tabs childsite 2]
    set lights    [$m.config.tabs childsite 3]
    set materials [$m.config.tabs childsite 4]
    set view      [$m.config.tabs childsite 5]
    set camera    [$m.config.tabs childsite 6]

    frame $render.eframe -background $Color(UIBackGround)
    frame $render.oframe -background $Color(UIBackGround)
    frame $view.iframe -background $Color(UIBackGround)
    
    # Set the label
    label $render.eframe.label -text "Global Settings :" \
    -background $Color(UIBackGround) 
    grid $render.eframe.label -row 0 -column 0 -columnspan 2 -sticky w

    # Global Lighting Checkbutton
    sci_checkbutton $render.eframe.light -text "Lighting" \
    -variable $this-global-light -command "$this-c redraw" 
    Tooltip $render.eframe.light \
    "Toggles on/off whether lights effect the rendering."
    grid $render.eframe.light -row 1 -column 0 -sticky w

    # Global Fog Checkbutton
    sci_checkbutton $render.eframe.fog -text "Fog" \
    -variable $this-global-fog -command "$this-c redraw" 
    Tooltip $render.eframe.fog \
    "Toggles on/off fog.  This will make objects further\n" \
    "away from the viewer appear dimmer and make it easier\n" \
    "to judge distances."
    grid $render.eframe.fog -row 2 -column 0 -sticky w

    # Global BBox Checkbutton
    sci_checkbutton $render.eframe.bbox -text "BBox" \
    -variable $this-global-debug  -command "$this-c redraw" 
    Tooltip $render.eframe.bbox \
    "Toggles on/off whether only the bounding box of every piece\n" \
    "of geometry is displayed.  Individual bounding boxes may be\n" \
    "toggled on/off using the 'Options' button in the 'Objects' frame."
    grid $render.eframe.bbox -row 3 -column 0 -sticky w

    # Global Clip Checkbutton
    sci_checkbutton $render.eframe.clip -text "Use Clip" \
    -variable $this-global-clip -command "$this-c redraw" 
    Tooltip $render.eframe.clip "Toggles on/off whether clipping is enabled."
    grid $render.eframe.clip -row 4 -column 0 -sticky w

    # Global Cull Checkbutton
    sci_checkbutton $render.eframe.cull -text "Back Cull" \
    -variable $this-global-cull -command "$this-c redraw" 
    Tooltip $render.eframe.cull \
    "Toggles on/off whether polygons that face away from\n" \
    "the camera are rendered."
    grid $render.eframe.cull -row 1 -column 2 -sticky w

    # Global Display List Checkbutton
    sci_checkbutton $render.eframe.dl -text "Display List" \
    -variable $this-global-dl -command "$this-c redraw" 
    Tooltip $render.eframe.dl \
    "Toggles on/off whether GL display lists are used."
    grid $render.eframe.dl -row 2 -column 2 -sticky w

    # Automatic Autoview when a network loads check button
    sci_checkbutton $render.eframe.autoav -text "Autoview on Load" \
    -variable $this-autoav -onvalue 1 -offvalue 0
    Tooltip $render.eframe.autoav \
    "Toggles on/off automatic viewer adjustment when new networks load."
    grid $render.eframe.autoav -row 1 -column 1 -sticky w

    # Show Axes Check Button
    sci_checkbutton $render.eframe.caxes -text "Show Axes" -variable $this-caxes \
    -onvalue 1 -offvalue 0 \
    -command "$this-c centerGenAxes; $this-c redraw" 
    Tooltip $render.eframe.caxes \
    "Toggles on/off the the set of three axes displayed at 0,0,0."
    grid $render.eframe.caxes -row 2 -column 1 -sticky w

    # Orientation Axes Checkbutton
    sci_checkbutton $render.eframe.raxes -text "Orientation" -variable $this-raxes \
    -onvalue 1 -offvalue 0 -command "$this-c redraw" 
    Tooltip $render.eframe.raxes \
    "Toggles on/off the orientation axes displayed in\n" \
    "the upper right corner of the viewer window."
    grid $render.eframe.raxes -row 3 -column 1 -sticky w

    # Ortho View Checkbutton
    sci_checkbutton $render.eframe.ortho -text "Ortho View" -variable $this-ortho-view \
    -onvalue 1 -offvalue 0 -command "$this-c redraw" 
    Tooltip $render.eframe.ortho  \
    "Toggles on/off the use of an orthographic projection.\n" \
    "SCIRun defaults to using the prospective projection."
    grid $render.eframe.ortho -row 4 -column 1 -sticky w
  
    # Render Style Radio Buttons

    label $render.eframe.label2 -text "Shading Settings :" \
    -background $Color(UIBackGround) 
    grid $render.eframe.label2 -row 6 -column 0 -columnspan 2 -sticky w
    sci_radiobutton $render.eframe.wire -text Wire -value Wire \
    -variable $this-global-type -command "$this-c redraw" 
    grid $render.eframe.wire -row 7 -column 0 -sticky w
    sci_radiobutton $render.eframe.flat -text Flat -value Flat \
    -variable $this-global-type -command "$this-c redraw"     
    grid $render.eframe.flat -row 8 -column 0 -sticky w
    sci_radiobutton $render.eframe.gouraud -text Gouraud -value Gouraud \
    -variable $this-global-type -command "$this-c redraw"
    grid $render.eframe.gouraud -row 7 -column 1 -sticky w

    # Stereo View Options
    sci_checkbutton $render.eframe.stereo -text "Stereo" -variable $this-do_stereo \
    -command "$this-c redraw" 
    Tooltip $render.eframe.stereo \
    "Switch into stereo rendering mode.  Special hardware may be\n" \
    "necessary to use this function."
    grid $render.eframe.stereo -row 3 -column 2 -sticky w
  
  
    set ir [expr int([set $this-bgcolor-r] * 65535)]
    set ig [expr int([set $this-bgcolor-g] * 65535)]
    set ib [expr int([set $this-bgcolor-b] * 65535)]

    frame $render.eframe.col -relief ridge -borderwidth 2 \
      -height 0.8c -width 2.0c \
      -background [format #%04x%04x%04x $ir $ig $ib]
    grid $render.eframe.col -row 7 -column 2

    button $render.eframe.set_color -text "Scene Color" \
      -command "$this makeBackGroundColorPopup $m $m2" \
      -bd 0 -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) 
    grid $render.eframe.set_color -row 8 -column 2

    label $render.oframe.label1 -text "Stereo Fusion:" -background $Color(UIBackGround)
    label $render.oframe.label2 -text "Polygon Offset:" -background $Color(UIBackGround)
    label $render.oframe.label3 -text "Point Size:" -background $Color(UIBackGround)
    label $render.oframe.label4 -text "Line Width:" -background $Color(UIBackGround)
    label $render.oframe.label5 -text "Field of View:" -background $Color(UIBackGround)

    scale $render.oframe.poffset -command "$this-c redraw" \
    -variable $this-polygon-offset-factor \
    -orient horizontal -from -4 -to 4 \
    -resolution .01 -showvalue true \
    -background $Color(UIBackGround) 

    scale $render.oframe.stereo -variable $this-sbase -length 100 -from 0.1 -to 2 \
    -resolution 0.02 -orient horizontal -command "$this-c redraw" \
    -background $Color(UIBackGround) 
    
    Tooltip $render.oframe.stereo \
    "Specifies how far the left and right eye images are\n" \
    "offset when rendering in stereo mode."

    scale $render.oframe.pointsize -variable $this-point-size -length 100 -from 1 -to 12 \
    -resolution 0.1 -orient horizontal -command "$this-c redraw" \
    -background $Color(UIBackGround) 

    scale $render.oframe.linewidth -variable $this-line-width -length 100 -from 1 -to 12 \
    -resolution 0.1 -orient horizontal -command "$this-c redraw" \
    -background $Color(UIBackGround) 
        
    scale $render.oframe.fov -variable $this-view-fov \
    -orient horizontal -from 0 -to 180 \
    -showvalue true -command "$this-c redraw" \
    -background $Color(UIBackGround)
                
    grid $render.oframe.label1 -row 0 -column 0 -sticky w
    grid $render.oframe.label2 -row 1 -column 0 -sticky w
#    grid $render.oframe.label3 -row 2 -column 0 -sticky w
#    grid $render.oframe.label4 -row 3 -column 0 -sticky w
    grid $render.oframe.label5 -row 4 -column 0 -sticky w
    grid $render.oframe.stereo -row 0 -column 1 -sticky w
    grid $render.oframe.poffset -row 1 -column 1 -sticky w
# THESE DO NOT SEEM TO WORK HENCE DISABLE THEM
#    grid $render.oframe.pointsize -row 2 -column 1 -sticky w
#    grid $render.oframe.linewidth -row 3 -column 1 -sticky w
     grid $render.oframe.fov       -row 4 -column 1 -sticky w


    # Auto Rotate
    label $view.iframe.label -text "Auto Rotate Settings :" \
      -background $Color(UIBackGround)
    pack  $view.iframe.label -side top
      
    sci_checkbutton $view.iframe.autorotate -text "Auto Rotate" \
        -variable $this-inertia_mode -command \
        "setGlobal $this-inertia_recalculate 1; $this-c redraw" 
    pack $view.iframe.autorotate -side top

    # Inertia Direction Buttons
    frame $view.iframe.buttons \
      -bd 0 -background $Color(UIBackGround) 
    pack $view.iframe.buttons -expand yes -fill x -side top
    
    button $view.iframe.buttons.up -text "Up" -padx 3 \
        -command "$this inertiaGo  0  1" \
      -bd 0 -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround)
    pack $view.iframe.buttons.up -side top
    
    frame $view.iframe.buttons.lr -relief flat
    pack $view.iframe.buttons.lr -side top -expand yes -fill x
    button $view.iframe.buttons.lr.left -text "Left" -padx 3 \
        -command "$this inertiaGo -1  0" \
      -bd 0 -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround)
    button $view.iframe.buttons.lr.right -text "Right" -padx 3 \
        -command "$this inertiaGo  1  0" \
      -bd 0 -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround)
    pack $view.iframe.buttons.lr.left -side left -anchor w -expand yes -fill x
    pack $view.iframe.buttons.lr.right -side left -anchor e -expand yes -fill x
    
    button $view.iframe.buttons.down -text "Down" -padx 3 \
        -command "$this inertiaGo  0 -1" \
      -bd 0 -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround)
    pack $view.iframe.buttons.down -side bottom

    # Frames per Second slider
    scale $view.iframe.loop_count -label "Frames per rotation" \
        -from 4 -to 360 -showvalue 45 \
        -variable $this-inertia_loop_count -resolution 1 -orient horizontal \
        -command "setGlobal $this-inertia_recalculate 1;\#" \
        -background $Color(UIBackGround) 
    pack $view.iframe.loop_count -expand yes -fill x -side top


    pack $view.iframe -anchor n -padx 8 -side left
    pack $render.eframe $render.oframe -anchor n -padx 8 -side left



    # Geometry Objects Options List
    frame $objects.objlist -bd 0 -background $Color(UIBackGround)
    pack $objects.objlist -side left -padx 2 -pady 2 -fill both -expand yes

    canvas $objects.objlist.canvas -height 168 \
    -scrollregion "0 0 450 168" -borderwidth 0 \
    -yscrollcommand "$objects.objlist.yscroll set" -yscrollincrement 10 \
    -bd 0 -background $Color(UIBackGround)
#    -xscrollcommand "$objects.objlist.xscroll set" -xscrollincrement 10 \

    frame $objects.objlist.canvas.frame -borderwidth 0 -background $Color(UIBackGround)
    pack $objects.objlist.canvas.frame -fill both -expand yes
    $objects.objlist.canvas create window 0 1 \
    -window $objects.objlist.canvas.frame -anchor nw

    # Scrollbars for Geometry Objects Options List
    # scrollbar $objects.objlist.xscroll -relief sunken -orient horizontal \
    # -command "$objects.objlist.canvas xview" \
    # -elementborderwidth 1 \
    # -bd 1 -troughcolor $Color(Trough) \
    # -background $Color(UIBackGround) 
    scrollbar $objects.objlist.yscroll -relief sunken -orient vertical \
    -command "$objects.objlist.canvas yview" \
    -elementborderwidth 1 \
    -bd 1 -troughcolor $Color(Trough) \
    -background $Color(UIBackGround) 
    pack $objects.objlist.yscroll -fill y -side left -padx 2 -pady 2
    pack $objects.objlist.canvas -side top -padx 2 -pady 2 -fill both -expand yes
  
  
    # CLIPPING MENU
    
    frame $clipping.select -background $Color(UIBackGround) 
    
    initGlobal $this-clip-num 6
    initGlobal $this-clip-selected 1
    for {set i 1} {$i <= 6} {incr i 1} {
        initGlobal $this-clip-visible-$i 0
        if {[expr [llength [set $this-clip-visible-$i] ] == 0]} {
          set $this-clip-visible-$i 0
        }
        initGlobal $this-clip-frame-$i 0
        if {[expr [llength [set $this-clip-frame-$i] ] == 0]} {
          set $this-clip-frame-$i 0
        }
        initGlobal $this-clip-normal-reverse-$i 0
        if {[expr [llength [set $this-clip-normal-reverse-$i] ] == 0]} {
          set $this-clip-normal-reverse-$i 0
        }
        initGlobal $this-clip-normal-d-$i 0.0
        if {[expr [llength [set $this-clip-normal-d-$i] ] == 0]} {
          set $this-clip-normal-d-$i 0
        }
        initGlobal $this-clip-normal-x-$i 1.0
        if {[expr [llength [set $this-clip-normal-x-$i] ] == 1.0]} {
          set $this-clip-normal-x-$i 0
        }
        initGlobal $this-clip-normal-y-$i 0.0
        if {[expr [llength [set $this-clip-normal-y-$i] ] == 0]} {
          set $this-clip-normal-y-$i 0
        }
        initGlobal $this-clip-normal-z-$i 0.0
        if {[expr [llength [set $this-clip-normal-z-$i] ] == 0]} {
          set $this-clip-normal-z-$i 0
        }
    }

    sci_radiobutton $clipping.select.clip1 -text "Plane 1" -value 1 \
    -variable $this-clip-selected -command "$this useClip" 

    sci_radiobutton $clipping.select.clip2 -text "Plane 2" -value 2 \
    -variable $this-clip-selected -command "$this useClip" 

    sci_radiobutton $clipping.select.clip3 -text "Plane 3" -value 3 \
    -variable $this-clip-selected -command "$this useClip"     

    sci_radiobutton $clipping.select.clip4 -text "Plane 4" -value 4 \
    -variable $this-clip-selected -command "$this useClip"     

    sci_radiobutton $clipping.select.clip5 -text "Plane 5" -value 5 \
    -variable $this-clip-selected -command "$this useClip"   

    sci_radiobutton $clipping.select.clip6 -text "Plane 6" -value 6 \
    -variable $this-clip-selected -command "$this useClip"   

    frame $clipping.select.space -height 8 -background $Color(UIBackGround)
    label $clipping.select.label -text "Plane Settings :" -background $Color(UIBackGround)

 	  sci_checkbutton $clipping.select.visible -text "Clipping Plane Visible" -variable $this-clip-visible \
    -relief flat -command "$this setClip;$this-c redraw"
    
    sci_checkbutton $clipping.select.widget -text "Show Clipping Plane Frame" \
    -variable $this-clip-frame -relief flat \
    -command "$this setClip; $this-c redraw"     

    sci_checkbutton $clipping.select.reverse -text "Reverse Clipping Plane Normal" \
    -variable $this-clip-normal-reverse -relief flat\
    -command "$this setClip; $this-c redraw" 

    grid $clipping.select.clip1 -row 0 -column 0 -sticky w
    grid $clipping.select.clip2 -row 1 -column 0 -sticky w
    grid $clipping.select.clip3 -row 2 -column 0 -sticky w
    grid $clipping.select.clip4 -row 0 -column 1 -sticky w
    grid $clipping.select.clip5 -row 1 -column 1 -sticky w
    grid $clipping.select.clip6 -row 2 -column 1 -sticky w
    grid $clipping.select.space    -row 3 -column 0 -columnspan 2 -sticky w
    grid $clipping.select.label    -row 4 -column 0 -columnspan 2 -sticky w
    grid $clipping.select.visible  -row 5 -column 0 -columnspan 2 -sticky w
    grid $clipping.select.widget   -row 6 -column 0 -columnspan 2 -sticky w
    grid $clipping.select.reverse  -row 7 -column 0 -columnspan 2 -sticky w
        
    makePlaneClip $clipping.normal "Plane Normal" $this-clip-normal \
	    "$this setClip ; $this-c redraw"
    
    pack $clipping.select $clipping.normal -side left -anchor n -padx 15
    useClip
    
    # LIGHTS MENU

    frame $lights.sources -background $Color(UIBackGround)
    frame $lights.menu -background $Color(UIBackGround)
    
#    label $lights.sources.label -text "Light Position and Colors :" -background $Color(UIBackGround)
    frame $lights.sources.frame -background $Color(UIBackGround)

    for {set i 0} {$i < 4} {incr i 1} {
        $this makeLightControl $m $m2 $w $i
    }
    
    pack $lights.sources $lights.menu -side left -anchor n
    
    label $lights.sources.label2 \
      -text "Click on number to move light. Note: Headlight will not move." \
      -background $Color(UIBackGround)
    label $lights.sources.label3 \
      -text "Click in circle to change light color/brightness" \
      -background $Color(UIBackGround)

    pack $lights.sources.frame \
         $lights.sources.label2 $lights.sources.label3 -side top 

    button $lights.menu.reset -text "Reset" \
      -command "$this resetLights $m $m2" \
      -bd $Color(BorderWidth) \
      -background $Color(ButtonBackGround) \
      -foreground $Color(ButtonForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround)   
  
    pack $lights.menu.reset -anchor n -side top
    
    # MATERIALS MENU
    
    frame $materials.f -background $Color(UIBackGround)
    
    label $materials.f.label1 -text "Ambient"\
      -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) 

    label $materials.f.label2 -text "Diffuse"\
      -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) 

    label $materials.f.label3 -text "Specular"\
      -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) 
      
    label $materials.f.label4 -text "Shininess"\
      -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) 
  
    label $materials.f.label5 -text "Emission"\
      -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround) 

    grid $materials.f.label1 -row 0 -column 0 -sticky w -pady 0
    grid $materials.f.label2 -row 1 -column 0 -sticky w -pady 0
    grid $materials.f.label3 -row 2 -column 0 -sticky w -pady 0
    grid $materials.f.label4 -row 3 -column 0 -sticky w -pady 0
    grid $materials.f.label5 -row 4 -column 0 -sticky w -pady 0
    
   scale $materials.f.ambient -orient horizontal -variable $this-ambient-scale \
	    -from 0 -to 5 -showvalue true -background $Color(UIBackGround)\
	    -resolution 0.05 -digits 3 -command "$this-c redraw"  

   scale $materials.f.diffuse -orient horizontal -variable $this-diffuse-scale \
	    -from 0 -to 5 -showvalue true -background $Color(UIBackGround)\
	    -resolution 0.05 -digits 3 -command "$this-c redraw"  

   scale $materials.f.specular -orient horizontal -variable $this-specular-scale \
	    -from 0 -to 5 -showvalue true -background $Color(UIBackGround)\
	    -resolution 0.05 -digits 3 -command "$this-c redraw"  

   scale $materials.f.shininess -orient horizontal -variable $this-shininess-scale \
	    -from 0 -to 1 -showvalue true -background $Color(UIBackGround)\
	    -resolution 0.05 -digits 3 -command "$this-c redraw"  

   scale $materials.f.emission -orient horizontal -variable $this-emission-scale \
	    -from 0 -to 1 -showvalue true -background $Color(UIBackGround)\
	    -resolution 0.05 -digits 3 -command "$this-c redraw"  

    grid $materials.f.ambient -row 0 -column 1 -sticky we -pady 0
    grid $materials.f.diffuse -row 1 -column 1 -sticky we -pady 0
    grid $materials.f.specular -row 2 -column 1 -sticky we -pady 0
    grid $materials.f.shininess -row 3 -column 1 -sticky we -pady 0
    grid $materials.f.emission -row 4 -column 1 -sticky we -pady 0

    frame $materials.fog -background $Color(UIBackGround)
    label $materials.fog.label -text "Fog Controls:" -background $Color(UIBackGround)
    
    label $materials.fog.label1 -text "Fog Start:" -background $Color(UIBackGround)
    label $materials.fog.label2 -text "Fog End:" -background $Color(UIBackGround)
    scale $materials.fog.start -command "$this-c redraw" \
    -variable $this-fog-start -orient horizontal -from 0 -to 1.0 \
    -resolution 0.01 -showvalue true -background $Color(UIBackGround)

    scale $materials.fog.end -command "$this-c redraw" \
    -variable $this-fog-end -orient horizontal -from 0 -to 2.0 \
    -resolution 0.01 -showvalue true -background $Color(UIBackGround)
 
    sci_checkbutton $materials.fog.vis -text "Visible Objects Only" \
    -variable $this-fog-visibleonly -command "$this-c redraw"

    sci_checkbutton $materials.fog.usebg -text "Use Background Color" \
    -variable $this-fogusebg -command "$this-c redraw"

    set ir [expr int([set $this-fogcolor-r] * 65535)]
    set ig [expr int([set $this-fogcolor-g] * 65535)]
    set ib [expr int([set $this-fogcolor-b] * 65535)]

    frame $materials.fog.col -relief ridge -borderwidth 4 \
    -height 0.6c -width 2.0c \
    -background [format #%04x%04x%04x $ir $ig $ib]

    button $materials.fog.set_color -text "Fog Color" \
    -command "$this makeFogColorPopup $m $m2" \
    -bd 0 -background $Color(UIBackGround) \
    -foreground $Color(UIForeGround) \
    -activebackground $Color(MenuSelectBackGround)  \
    -activeforeground $Color(MenuSelectForeGround) 
 
#    grid $materials.fog.label -row 0 -column 0 -columnspan 2 -sticky w
    grid $materials.fog.label1 -row 1 -column 0 -sticky w
    grid $materials.fog.label2 -row 2 -column 0 -sticky w
    grid $materials.fog.start -row 1 -column 1 -sticky we
    grid $materials.fog.end   -row 2 -column 1 -sticky we
    grid $materials.fog.vis -row 3 -column 1 -sticky w
    grid $materials.fog.usebg -row 4 -column 1 -sticky w
    grid $materials.fog.col -row 3 -column 0 
    grid $materials.fog.set_color -row 4 -column 0 

    pack $materials.f $materials.fog -side left -padx 25 -anchor n
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
    global Color

    set w .ui[modname]
    if { $IsDisplayed } {
      if { $IsAttached!=0} {        
        pack forget $attachedFr
        set hei [expr [winfo height $w]-[winfo reqheight $w.bframe]]
        append geom [winfo width $w]x${hei}
        wm geometry $w $geom
        wm deiconify $detachedFr
        set IsAttached 0
      } else {
        wm withdraw $detachedFr
        pack $attachedFr -anchor w -side bottom \
            -before $w.bsframe -fill x
        set hei [expr [winfo height $w]+[winfo reqheight $w.bframe]]
        append geom [winfo width $w]x${hei}
        wm geometry $w $geom
        set IsAttached 1
      }
      update
      set bsframe $w.bsframe
      $bsframe.detach configure -text "> Configure <"
      $bsframe.detachp configure -text "S"
      bind $bsframe.detach <ButtonPress-1> "$this removeMFrame $w"
      bind $bsframe.detach <ButtonPress-2> "$this switch_frames"
      bind $bsframe.detach <ButtonPress-3> "$this removeMFrame $w"    
      bind $bsframe.detachp <ButtonPress-1> "$this switch_frames"

    }
  }

  method switch_frames2 {} {
    set w .ui[modname]
    if { !$IsDisplayed } {
      pack forget $attachedFr
      wm deiconify $detachedFr
      set IsAttached 0
      set IsDisplayed 1
      update
    
      set bsframe $w.bsframe
      $bsframe.detachp configure -text "S"
      $bsframe.detach configure -text "> Configure <"
      bind $bsframe.detach <ButtonPress-1> "$this removeMFrame $w"
      bind $bsframe.detach <ButtonPress-2> "$this switch_frames"
      bind $bsframe.detach <ButtonPress-3> "$this removeMFrame $w"   
      bind $bsframe.detachp <ButtonPress-1> "$this switch_frames"
    } 
  }


  method switch_frames3 {} {
    set w .ui[modname]
    if { !$IsDisplayed } {
      wm withdraw $detachedFr
      pack $attachedFr -anchor w -side bottom \
          -before $w.bsframe -fill x
      set hei [expr [winfo height $w]+[winfo reqheight $w.bframe]]
      append geom [winfo width $w]x${hei}
      wm geometry $w $geom
      set IsAttached 1
      set IsDisplayed 1
      update
    
      set bsframe $w.bsframe
      $bsframe.detach configure -text "> Configure <"
      $bsframe.detachp configure -text "S"
      bind $bsframe.detach <ButtonPress-1> "$this removeMFrame $w"
      bind $bsframe.detach <ButtonPress-2> "$this switch_frames"
      bind $bsframe.detach <ButtonPress-3> "$this removeMFrame $w"   
      bind $bsframe.detachp <ButtonPress-1> "$this switch_frames"
    } 
  }


# WE REMOVED THIS TO GET MORE VISUAL SPACE
    method updatePerf { p1 p2 p3 } {
#      set w .ui[modname]
#      set bsframe [$w.bsframe childsite]
#      $bsframe.pf.perf1 configure -text $p1
#      $bsframe.pf.perf2 configure -text $p2
#      $bsframe.pf.perf3 configure -text $p3
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

  method makeFogColorPopup {m m2} {
    set w .fogcolor[modname]
    if [winfo exists $w] {
      SciRaise $w
      return
    }

    makeColorPicker $w $this-fogcolor \
            "$this updateFogColor $m $m2; $this-c redraw" "destroy $w"
    wm title $w "Choose Fog Color"
    SciRaise $w
  }

  method updateFogColor {m m2} {
    set materials [$m.config.tabs childsite 4]  
    set c $materials.fog.col

    set materials [$m2.config.tabs childsite 4]  
    set c2 $materials.fog.col
        
    set ir [expr int([set $this-fogcolor-r] * 65535)]
    set ig [expr int([set $this-fogcolor-g] * 65535)]
    set ib [expr int([set $this-fogcolor-b] * 65535)]
    
    $c config -background [format #%04x%04x%04x $ir $ig $ib]
    $c2 config -background [format #%04x%04x%04x $ir $ig $ib]
  }

  method makeBackGroundColorPopup {m m2} {
    set w .bgcolor[modname]
    if [winfo exists $w] {
      SciRaise $w
      return
    }

    makeColorPicker $w $this-bgcolor \
            "$this updateBackGroundColor $m $m2; $this-c redraw" "destroy $w"
    wm title $w "Choose BackGround Color"
    SciRaise $w
  }

  method updateBackGroundColor {m m2} {
    set render [$m.config.tabs childsite 0]  
    set c $render.eframe.col

    set render [$m2.config.tabs childsite 0]  
    set c2 $render.eframe.col
        
    set ir [expr int([set $this-bgcolor-r] * 65535)]
    set ig [expr int([set $this-bgcolor-g] * 65535)]
    set ib [expr int([set $this-bgcolor-b] * 65535)]
    
    $c config -background [format #%04x%04x%04x $ir $ig $ib]
    $c2 config -background [format #%04x%04x%04x $ir $ig $ib]
  }

  method addObject {objid name} {
    BaseViewWindow::addObject $objid $name
    addObjectToFrame $objid $name $detachedFr
    addObjectToFrame $objid $name [$attachedFr childsite]
  }

  method addObjectToFrame {objid name frame} {
    global Color
    set w .ui[modname]
    set m $frame.f
    set objects [$m.config.tabs childsite 1]

    # if the object frame exists already, assume it was pack
    # forgotten by removeObject, just pack it again to show it
    
    if { [winfo exists $objects.objlist.canvas.frame.objt$objid] } {
      pack $objects.objlist.canvas.frame.objt$objid \
      -side top -anchor w -fill x -expand y

      pack $objects.objlist.canvas.frame.menu$objid \
      -in $objects.objlist.canvas.frame.objt$objid -side left \
      -padx 1 -pady 1
      pack $objects.objlist.canvas.frame.obj$objid  \
      -in $objects.objlist.canvas.frame.objt$objid -side left
      return
    }

    frame $objects.objlist.canvas.frame.objt$objid \
      -background $Color(UIBackGround) 
    
    sci_checkbutton $objects.objlist.canvas.frame.obj$objid -text $name \
      -relief flat -variable "$this-$name" -command "$this-c redraw" 
    
    set newframeheight [winfo reqheight $objects.objlist.canvas.frame.obj$objid]
    
    set menun $objects.objlist.canvas.frame.menu$objid.menu

    menubutton $objects.objlist.canvas.frame.menu$objid -text "Settings..." \
      -menu $menun \
      -background $Color(MenuBarBackGround) \
      -foreground $Color(MenuBarForeGround) \
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround)

    menu $menun -bd 2 -activeborderwidth 0 \
      -tearoff false\
      -activebackground $Color(MenuSelectBackGround)  \
      -activeforeground $Color(MenuSelectForeGround) \
      -background $Color(UIBackGround) \
      -foreground $Color(UIForeGround)

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

    pack $objects.objlist.canvas.frame.objt$objid \
        -side top -anchor w -fill x -expand y
    pack $objects.objlist.canvas.frame.menu$objid \
        -in $objects.objlist.canvas.frame.objt$objid -side left -padx 1 -pady 1
    pack $objects.objlist.canvas.frame.obj$objid \
        -in $objects.objlist.canvas.frame.objt$objid -side left

    update idletasks
    set width [winfo width $objects.objlist.canvas.frame]
    set height [lindex [$objects.objlist.canvas cget -scrollregion] end]

    incr height [expr $newframeheight+20]

    $objects.objlist.canvas configure -scrollregion "0 0 $width $height"

#    set view [$objects.objlist.canvas xview]
#    $objects.objlist.xscroll set [lindex $view 0] [lindex $view 1]

    set view [$objects.objlist.canvas yview]
    $objects.objlist.yscroll set [lindex $view 0] [lindex $view 1]
  }

  method removeObject {objid} {
    removeObjectFromFrame $objid $detachedFr
    removeObjectFromFrame $objid [$attachedFr childsite]
    BaseViewWindow::removeObject $objid
  }

  method removeObjectFromFrame {objid frame} {
    set objects [$frame.f.config.tabs childsite 1]
    pack forget $objects.objlist.canvas.frame.objt$objid
  }


  method makeLightControl { m m2 w i } {
    global Color
    set lights [$m.config.tabs childsite 3]
    set f $lights.sources.frame
    
    frame $f.f$i -background $Color(UIBackGround) 
    pack $f.f$i -side left
    canvas $f.f$i.c -background $Color(UIBackGround) -width 100 -height 100
    pack $f.f$i.c -side top
    set c $f.f$i.c
    sci_checkbutton $f.f$i.b$i -text "on/off"\
        -variable $this-global-light$i -command "$this lightSwitch $i"
    pack $f.f$i.b$i

    upvar \#0 $this-lightColors lightColors $this-lightVectors lightVectors
    set ir [expr int([lindex [lindex $lightColors $i] 0] * 65535)]
    set ig [expr int([lindex [lindex $lightColors $i] 1] * 65535)]
    set ib [expr int([lindex [lindex $lightColors $i] 2] * 65535)]
         
    set window .ui[modname]
    set color [format "#%04x%04x%04x" $ir $ig $ib]
    set news [$c create oval 5 5 95 95 -outline "#000000" -fill $color -tags lc ]

    set x [expr int([lindex [lindex $lightVectors $i] 0] * 50) + 50]
    set y [expr int([lindex [lindex $lightVectors $i] 1] * -50) + 50]
    set t  $i
    if { $t == 0 } { set t "HL" }
    set newt [$c create text $x $y -fill "#555555" -text $t -tags lname ]
    
    $c bind lname <B1-Motion> "$this moveLight $m $m2 $i %x %y"
    $c bind lc <ButtonPress-1> "$this lightColor $m $m2 $w $c $i"
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

  method lightColor { m m2 w c i } {
    set lights [$m.config.tabs childsite 3]  
    set c $lights.sources.frame.f$i.c

    set lights [$m2.config.tabs childsite 3]  
    set c2 $lights.sources.frame.f$i.c
    
    upvar \#0 $this-lightColors lightColors
    setGlobal $this-def-color-r [lindex [lindex $lightColors $i] 0]
    setGlobal $this-def-color-g [lindex [lindex $lightColors $i] 1]
    setGlobal $this-def-color-b [lindex [lindex $lightColors $i] 2]

    if { [winfo exists $w.lightcolor] } {
      destroy $w.lightcolor
    }
    
    makeColorPicker $w.lightcolor $this-def-color \
	    "$this setColor $w.lightcolor $c $i $this-def-color; $this setColor $w.lightcolor $c2 $i $this-def-color " \
	    "destroy $w.lightcolor"

    SciRaise $w.lightcolor
    wm title $w.lightcolor "Choose Light Color"
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
    
  method resetLights { m m2 } {
    set lights [$m.config.tabs childsite 3]  
    set f $lights.sources.frame

    set lights [$m2.config.tabs childsite 3]  
    set f2 $lights.sources.frame
      
    upvar \#0 $this-lightColors lCol $this-lightVectors lVec
    for { set i 0 } { $i < 4 } { incr i 1 } {
	    if { $i == 0 } {
        set $this-global-light$i 1
        $f.f$i.c itemconfigure lc -fill \
		    [format "#%04x%04x%04x" 65535 65535 65535 ]
        $f2.f$i.c itemconfigure lc -fill \
		    [format "#%04x%04x%04x" 65535 65535 65535 ]
        set lCol [lreplace $lCol $i $i [list 1.0 1.0 1.0]]
        $this lightSwitch $i
	    } else {
        set $this-global-light$i 0
        set coords [$f.f$i.c coords lname]
        set curX [lindex $coords 0]
        set curY [lindex $coords 1]
        set xn [expr 50 - $curX]
        set yn [expr 50 - $curY]
        $f.f$i.c move lname $xn $yn
        $f2.f$i.c move lname $xn $yn
        set vec [list 0 0 1 ]
        set $this-lightVectors \
            [lreplace [set $this-lightVectors] $i $i $vec]
        $f.f$i.c itemconfigure lc -fill \
            [format "#%04x%04x%04x" 65535 65535 65535 ]
        $f2.f$i.c itemconfigure lc -fill \
            [format "#%04x%04x%04x" 65535 65535 65535 ]
        set lightColor [list 1.0 1.0 1.0]
        set $this-lightColors \
		    [lreplace [set $this-lightColors] $i $i $lightColor]
        $this lightSwitch $i
	    }
    }
  }

  method moveLight { m m2 i x y } {
  
    set lights [$m.config.tabs childsite 3]  
    set c $lights.sources.frame.f$i.c

    set lights [$m2.config.tabs childsite 3]  
    set c2 $lights.sources.frame.f$i.c

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
      $c2 move $selected [expr $xn-$curX] [expr $yn-$curY]
    } else { 
      # keep the text inside the circle
      set scale [expr 45 / sqrt($len2)]
      set xn [expr 50 + ($x - 50) * $scale]
      set yn [expr 50 + ($y - 50) * $scale]
      $c move $selected [expr $xn-$curX] [expr $yn-$curY]
      $c2 move $selected [expr $xn-$curX] [expr $yn-$curY]
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
    sci_toplevel $w

    wm title $w "Record Movie"

    sci_frame $w.movieType -relief groove -borderwidth 2
    sci_label $w.movieType.l -text "Record Movie as:"

    sci_radiobutton $w.movieType.none -text "Stop Recording" \
    -variable $this-global-movie -value 0 -command "$this-c redraw"
    sci_radiobutton $w.movieType.raw -text "PPM Frames" \
    -variable $this-global-movie -value 1 -command "$this-c redraw"
    sci_radiobutton $w.movieType.png -text "PNG Frames" \
    -variable $this-global-movie -value 3 -command "$this-c redraw"
    sci_radiobutton $w.movieType.mpeg -text "Mpeg" \
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

    sci_checkbutton $w.sync -text "Sync with Execute" \
        -variable $this-global-sync_with_execute

    Tooltip $w.sync \
        "Synchronizes movie frame output with the execution of the\n"\
        "module.  Mouse events and other viewer interaction will\n"\
        "not be recorded."

    sci_frame $w.moviebase
    sci_label $w.moviebase.label -text "File Name:"
    sci_entry $w.moviebase.entry -relief sunken -width 15 -textvariable "$this-global-movieName" 

    TooltipMultiWidget "$w.moviebase.label $w.moviebase.entry" \
      "Name of the movie file.  The %%#d specifies number of digits\nto use in the frame number.  Eg: movie.%%04d will\nproduce names such as movie.0001.png"

    sci_frame $w.movieframe
    sci_label $w.movieframe.label -text "Next Frame No:" -width 15
          entry $w.movieframe.entry -relief sunken -width 6 \
        -textvariable "$this-global-movieFrame" 

          frame $w.movieUseTimestampFrame
    sci_label $w.movieUseTimestampFrame.label -text "Add timestamp to name:"
    sci_checkbutton $w.movieUseTimestampFrame.button -variable $this-global-movieUseTimestamp \
        -offvalue 0 -onvalue 1

    TooltipMultiWidget "$w.movieUseTimestampFrame.button $w.movieUseTimestampFrame.label" \
        "Appends a timestamp to the name of each (PNG/PPM) frame. This can be useful\nwhen you are dumping frames from multiple different ViewScene windows."

    TooltipMultiWidget "$w.movieframe.label $w.movieframe.entry" \
        "Frame number at which to start numbering generated frames."

    sci_frame $w.resize_f
    sci_checkbutton $w.resize_f.resize -text "Resize: " \
        -variable $this-global-resize \
        -offvalue 0 -onvalue 1 -command "$this resize; $this-c redraw"
    sci_entry $w.resize_f.e1 -textvariable $this-x-resize -width 4
    sci_label $w.resize_f.x -text x
    sci_entry $w.resize_f.e2 -textvariable $this-y-resize -width 4
   
    Tooltip $w.resize_f.resize \
              "When selected, the output will be resized to these dimensions."
    bind $w.resize_f.e1 <Return> "$this resize"
    bind $w.resize_f.e2 <Return> "$this resize"

    sci_entry $w.message -textvariable $this-global-movieMessage \
        -relief flat -width 20 -state disabled
    sci_frame $w.separator -height 2 -relief sunken -borderwidth 2
    sci_button $w.close -width 10 -text "Close" \
      -command "wm withdraw $w"

    pack $w.movieType -pady 4
    pack $w.movieType.l -padx 4 -anchor w
    pack $w.movieType.none $w.movieType.raw $w.movieType.png $w.movieType.mpeg -padx 4 -anchor w

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

proc makePlaneClip {w title name command} {
   global Color
   frame $w -background $Color(UIBackGround)
   
   label $w.label -text $title -background $Color(UIBackGround)
   
   label $w.xlabel -text "X: " -background $Color(UIBackGround)
   label $w.ylabel -text "Y: " -background $Color(UIBackGround)
   label $w.zlabel -text "Z: " -background $Color(UIBackGround)
   label $w.dlabel -text "D: " -background $Color(UIBackGround)
  
   scale $w.x -orient horizontal -variable $name-x \
	    -from -1 -to 1 -showvalue true -background $Color(UIBackGround)\
	    -resolution 0.01 -digits 3 -command $command  
   scale $w.y -orient horizontal -variable $name-y \
	    -from -1 -to 1 -showvalue true -background $Color(UIBackGround)\
	    -resolution 0.01 -digits 3 -command $command  
   scale $w.z -orient horizontal -variable $name-z \
	    -from -1 -to 1 -showvalue true -background $Color(UIBackGround)\
	    -resolution 0.01 -digits 3 -command $command  
  scale $w.d -orient horizontal -variable $name-d \
      -from -1.01 -to 1.01 -background $Color(UIBackGround)\
      -showvalue true \
      -resolution 0.01 -digits 3 \
      -command $command
         
   grid $w.xlabel -row 1 -column 0 -sticky w
   grid $w.ylabel -row 2 -column 0 -sticky w
   grid $w.zlabel -row 3 -column 0 -sticky w
   grid $w.dlabel -row 4 -column 0 -sticky w
   grid $w.x -row 1 -column 1 -sticky w
   grid $w.y -row 2 -column 1 -sticky w
   grid $w.z -row 3 -column 1 -sticky w
   grid $w.d -row 4 -column 1 -sticky w
   
}

