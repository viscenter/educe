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



itcl_class MatlabInterface_DataIO_ImportBundlesFromMatlab {
	inherit Module

	constructor {config} {
		set name ImportBundlesFromMatlab
		set_defaults
	}

	method set_defaults {} {
    global $this-filename
    global $this-matrixinfotexts
    global $this-matrixnames
    global $this-pnrrds
    global $this-pbundles
    global $this-pnrrd
    global $this-pbundle    
    global $this-matrixname
    global $this-matriceslistbox		
    global $this-pnrrdsmenu
    global $this-pbundlesmenu
    global $this-filename-set
    global $this-portsel
    global $this-numport
    
    set $this-filename ""
    set $this-matrixinfotexts ""
    set $this-matrixnames ""
    set $this-matrixname ""
    set $this-matriceslistbox ""
    set $this-pnrrdsmenu ""
    set $this-pbundlesmenu ""
    set $this-filename-set ""
    set $this-portsel 0
    set $this-numport 6
    set $this-pnrrd ""
    set $this-pbundle ""
    set $this-pnrrds {{prefer matrices} {prefer nrrds} {prefer fields}}
    set $this-pbundles {{prefer sciobjects} {prefer bundles}}
                
    for {set x 0} {$x < [set $this-numport]} {incr x} {
      lappend $this-pnrrd {prefer matrices}
      lappend $this-pbundle {prefer sciobjects}
    }
    
	}


	method ui {} {

    global $this-filename
    global $this-matrixinfotexts		
    global $this-matriceslistbox		
    global $this-pnrrdsmenu		
    global $this-pbundlesmenu		
    global $this-filename-entry
    global $this-port
    global $this-numport
    global $this-portsel
    global $this-pnrrds
    global $this-pbundles
    global $this-pnrrd
    global $this-pbundle    


    set w .ui[modname]

    # test whether gui is already out there
    # raise will move the window to the front
    # so the user can modify the settings

    if {[winfo exists $w]} {
            raise $w
            return
    }

    # create a new gui window

    toplevel $w 

    iwidgets::labeledframe $w.fileframe -labeltext "SELECT MATLAB FILE"
    set childfile [$w.fileframe childsite]
    pack $w.fileframe -fill x

    frame $childfile.f1
    frame $childfile.f2
    
    pack $childfile.f1 $childfile.f2 -side top -fill x -expand yes

    label $childfile.f1.label -text ".MAT FILE "
    entry $childfile.f1.file -textvariable $this-filename
    set $this-filename-entry $childfile.f1.file  
    button $childfile.f2.open -text "Open" -command "$this OpenMatfile"
    button $childfile.f2.browse -text "Browse" -command "$this ChooseFile"

    pack $childfile.f1.label -side left -padx 3p -pady 2p -padx 4p
    pack $childfile.f1.file -side left -fill x -expand yes -padx 3p -pady 2p
    pack $childfile.f2.browse -side right -padx 3p -pady 2p -anchor e
    pack $childfile.f2.open -side right -padx 3p -pady 2p -anchor e

    iwidgets::labeledframe $w.matrixframe -labeltext "SELECT MATLAB MATRIX"
    set childframe [$w.matrixframe childsite]
    pack $w.matrixframe -fill both -expand yes

    frame $childframe.portframe 
    pack $childframe.portframe -fill x -pady 4p
    set $this-port $childframe.portframe
    
    for {set x 0} {$x < [set $this-numport]} {incr x} {
            button [set $this-port].$x -text [format "Port %d" [expr $x + 1]] -command [format "%s SetPort %d" $this $x]
            pack [set $this-port].$x  -side left -fill x -padx 2p -anchor w
    }

    [set $this-port].[set $this-portsel] configure -fg #FFFFFF

    iwidgets::scrolledlistbox $childframe.listbox -selectmode single \
          -selectioncommand "$this ChooseMatrix" \
          -width 500p -height 300p \
          -selectbackground #ffc0c0
    set $this-matriceslistbox $childframe.listbox
    $childframe.listbox component listbox configure -listvariable $this-matrixinfotexts
    pack $childframe.listbox -fill both -expand yes

    frame $childframe.f1
    
    iwidgets::optionmenu $childframe.f1.pnrrds -command "$this Setpnrrds"
    foreach opt {{prefer matrices} {prefer nrrds} {prefer fields}} {
      $childframe.f1.pnrrds insert end $opt
    }

    iwidgets::optionmenu $childframe.f1.pbundles -command "$this Setpbundles"
    foreach opt {{prefer sciobjects} {prefer bundles}} {
      $childframe.f1.pbundles insert end $opt
    }

    pack $childframe.f1 -fill x
    pack $childframe.f1.pnrrds $childframe.f1.pbundles -side left -anchor w

    makeSciButtonPanel $w $w $this

    set matrixname [lindex [set $this-matrixname] [set $this-portsel] ]
    set selnum [lsearch [set $this-matrixnames] $matrixname]
    [set $this-matriceslistbox] component listbox selection clear 0 end
    if [expr $selnum > -1] { [set $this-matriceslistbox] component listbox selection set $selnum }

    set pnrrd [lindex [set $this-pnrrd] [set $this-portsel] ]
    set selnum [lsearch [set $this-pnrrds] $pnrrd]
    set $this-pnrrdsmenu $childframe.f1.pnrrds
    if [expr $selnum > -1] { [set $this-pnrrdsmenu] select $selnum }

    set pbundle [lindex [set $this-pbundle] [set $this-portsel] ]
    set selnum [lsearch [set $this-pbundles] $pbundle]
    set $this-pbundlesmenu $childframe.f1.pbundles
    if [expr $selnum > -1] { [set $this-pbundlesmenu] select $selnum }


	}

	method SetPort {num} {
    global $this-matriceslistbox
    global $this-pnrrdsmenu
    global $this-pbundlesmenu
    global $this-matrixnames
    global $this-matrixname
    global $this-port
    global $this-numport
    global $this-portsel

    
    set $this-portsel $num

    set matrixname [lindex [set $this-matrixname] [set $this-portsel] ]
    set selnum [lsearch [set $this-matrixnames] $matrixname]
    [set $this-matriceslistbox] component listbox selection clear 0 end
    if [expr $selnum > -1] { [set $this-matriceslistbox] component listbox selection set $selnum }

    set pnrrd [lindex [set $this-pnrrd] [set $this-portsel] ]
    set selnum [lsearch [set $this-pnrrds] $pnrrd]
    if [expr $selnum > -1] { [set $this-pnrrdsmenu] select $selnum }

    set pbundle [lindex [set $this-pbundle] [set $this-portsel] ]
    set selnum [lsearch [set $this-pbundles] $pbundle]
    if [expr $selnum > -1] { [set $this-pbundlesmenu] select $selnum }

    for {set x 0} {$x < [set $this-numport]} {incr x} {
            [set $this-port].$x configure -fg #000000
    }
    [set $this-port].[set $this-portsel] configure -fg #FFFFFF

    $this ChooseMatrix
	}

	method ChooseMatrix { } {
    global $this-matriceslistbox
    global $this-matrixnames
    global $this-matrixname
    global $this-portsel
    
    set matrixnum [[set $this-matriceslistbox] curselection]
    if [expr [string equal $matrixnum ""] == 0] {
            set $this-matrixname [lreplace [set $this-matrixname] [set $this-portsel] [set $this-portsel] [lindex [set $this-matrixnames] $matrixnum] ]
    }
	}

	method Setpnrrds { } {
          global $this-pnrrdsmenu
          global $this-pnrrds
          global $this-pnrrd
          global $this-portsel
          
          set num [[set $this-pnrrdsmenu] get]
          if [expr [string equal $num ""] == 0] {
                  set $this-pnrrd [lreplace [set $this-pnrrd] [set $this-portsel] [set $this-portsel] $num ]
          }
	}

	method Setpbundles { } {
          global $this-pbundlesmenu
          global $this-pbundles
          global $this-pbundle
          global $this-portsel
          
          set num [[set $this-pbundlesmenu] get]
          if [expr [string equal $num ""] == 0] {
                  set $this-pbundle [lreplace [set $this-pbundle] [set $this-portsel] [set $this-portsel] $num ]
          }
	}


	method ChooseFile { } {

          global env
          global $this-filename
          global $this-filename-set


          # Create a unique name for the file selection window
          set w [format "%s-filebox" .ui[modname]]

          # if the file selector is open, bring it to the front
          # in case it is iconified, deiconify
          if { [winfo exists $w] } {
                  if { [winfo ismapped $w] == 1} {
                          raise $w
                  } else {
                          wm deiconify $w
                  }
                  return
          }

          toplevel $w -class TkFDialog

          set initdir ""

          # place to put preferred data directory
          # it's used if $this-filename is empty

          # Use the standard data dirs
          # I guess there is no .mat files in there
          # at least not yet

          if {[info exists env(SCIRUN_DATA)]} {
                  set initdir $env(SCIRUN_DATA)
          } elseif {[info exists env(SCI_DATA)]} {
                  set initdir $env(SCI_DATA)
          } elseif {[info exists env(PSE_DATA)]} {
                  set initdir $env(PSE_DATA)
          }

          if {[expr [file exists [set $this-filename-set]] == 0]} { 
            set pathname [file dirname [set $this-filename-set]]
            if {[expr [file exists $pathname] == 0]} {
              set pathname $initdir
            }
            set $this-filename-set $pathname
          }
          
          makeOpenFilebox \
                  -parent $w \
                  -filevar $this-filename-set \
                  -command "wm withdraw $w;  $this OpenNewMatfile" \
                  -commandname "Open" \
                  -cancel "wm withdraw $w" \
                  -title "SELECT MATLAB FILE" \
                  -filetypes {{ "Matlab files" "*.mat" } { "All files"  "*" } }\
                  -initialdir $initdir \
                  -defaultextension "*.mat" \
                  -selectedfiletype 0
          wm deiconify $w	
	}
	
	method OpenNewMatfile {} {

          global $this-filename
          global $this-filename-set
          global $this-matrixnames
          global $this-matrixname
          global $this-matrixinfotexts
          global $this-portsel
          
          set $this-filename [set $this-filename-set] 
          
          # get the matrices in this file from the C++ side
          
          $this-c indexmatlabfile

          set matrixname [lindex [set $this-matrixname] [set $this-portsel] ]
          set selnum [lsearch [set $this-matrixnames] $matrixname]
          [set $this-matriceslistbox] component listbox selection clear 0 end
          if [expr $selnum > -1] { [set $this-matriceslistbox] component listbox selection set $selnum }
	}
	
	method OpenMatfile {} {

          global $this-filename
          global $this-matrixnames
          global $this-matrixinfotexts
          global $this-matrixname
          global $this-filename-entry
          global $this-portsel
          
          set $this-filename [[set $this-filename-entry] get] 
          
          # get the matrices in this file from the C++ side
          
          $this-c indexmatlabfile
          
          set matrixname [lindex [set $this-matrixname] [set $this-portsel] ]
          set selnum [lsearch [set $this-matrixnames] $matrixname]
          [set $this-matriceslistbox] component listbox selection clear 0 end
          if [expr $selnum > -1] { [set $this-matriceslistbox] component listbox selection set $selnum }
	}

}
