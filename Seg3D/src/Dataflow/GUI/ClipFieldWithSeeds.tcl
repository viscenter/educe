itcl_class SCIRun_NewField_ClipFieldWithSeeds {
    inherit Module
    constructor {config} {
        set name ClipFieldWithSeeds
        set_defaults
    }

   method set_defaults {} {
      global $this-function
      global $this-help

      set $this-function "DATA < 0;"
      set $this-help ""
    }

    method update_text {} {
      set w .ui[modname]
      if {[winfo exists $w]} {
        set function [$w.ff childsite]
        set $this-function [$function.function get 1.0 end]
        }
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }

        sci_toplevel $w

        sci_labeledframe $w.inf -labeltext "Create Clipping Expression"
        set infoframe [$w.inf childsite]
        sci_frame $infoframe.info
        pack $infoframe.info -side left
        set info $infoframe.info
        sci_label $info.info1 -text "Function: expression(FDATA,SDATA,FX,FY,FZ,SX,SY,SZ)"
        sci_label $info.info2 -text "Input array: FDATA (scalar/vector/tensor: data from field port) "
        sci_label $info.info2a -text "Input array: SDATA (scalar/vector/tensor: data from field port) "
        sci_label $info.info3 -text "Input array: FX, FY, FZ (scalar: Cartensian coordinates of node/element)"
        sci_label $info.info3a -text "Input array: SX, SY, SZ (scalar: Cartensian coordinates of node/element)"
        sci_label $info.info6 -text "Input array: INDEX (scalar: number of the element)"
        sci_label $info.info7 -text "Input array: SIZE (scalar: number of elements)"

        grid $info.info1 -row 0 -column 0 -columnspan 2 -sticky w
        grid $info.info2 -row 1 -column 0 -sticky w
        grid $info.info2a -row 2 -column 0 -sticky w
        grid $info.info3 -row 3 -column 0 -sticky w
        grid $info.info3a -row 0 -column 1 -sticky w
        grid $info.info6 -row 1 -column 1 -sticky w
        grid $info.info7 -row 2 -column 1 -sticky w

        pack $w.inf -side top -anchor w -fill x

        sci_labeledframe $w.ff -labeltext "Expression"
        set function [$w.ff childsite]
        option add *textBackground white	
        sci_scrolledtext $function.function -height 60 -hscrollmode dynamic
        $function.function insert end [set $this-function]
        bind $function.function <Leave> "$this update_text"

        pack $w.ff -side top -anchor w -fill both 
        pack $function.function -side top -fill both 
  
        sci_labeledframe $w.clip -labeltext "Clipping location"
        set clip [$w.clip childsite]
        
        sci_radiobutton $clip.cell -text "Element Center" \
            -variable $this-method -value element
        sci_radiobutton $clip.nodeone -text "One Node" \
            -variable $this-method -value onenode
        sci_radiobutton $clip.nodemost -text "Most Nodes" \
            -variable $this-method -value mostnodes
        sci_radiobutton $clip.nodeall -text "All Nodes" \
            -variable $this-method -value allnodes
 
        pack $w.clip -side top -anchor w -fill x

        grid $clip.cell -row 0 -column 0 -sticky w
        grid $clip.nodeone -row 0 -column 1 -sticky w
        grid $clip.nodemost -row 0 -column 2 -sticky w
        grid $clip.nodeall -row 0 -column 3 -sticky w


        sci_button $w.help -text "Available Functions" -command "$this showhelp"
        pack $w.help -side top -anchor e    

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
    
     method showhelp { } {

      # Create a unique name for the file selection window
      set w [format "%s-functionhelp" .ui[modname]]

      if { [winfo exists $w] } {
        if { [winfo ismapped $w] == 1} {
          raise $w
        } else {
          wm deiconify $w
        }
	    	return
      }
	
      sci_toplevel $w -class TkFDialog

      global $this-help
      $this-c gethelp   
            
      sci_labeledframe $w.hf -labeltext "available functions"
      set help [$w.hf childsite]
      option add *textBackground white	
      iwidgets::scrolledhtml $help.help -height 60 -hscrollmode dynamic -width 500p -height 300p        
      $help.help render [set $this-help]
      pack $help.help -side top -anchor w -fill both -expand yes
      pack $w.hf -side top -anchor w -fill both -expand yes
    }
    
}


