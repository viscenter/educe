itcl_class SCIRun_ChangeFieldData_RegisterWithCorrespondences {
    inherit Module
    constructor {config} {
        set name RegisterWithCorrespondences
        set_defaults
    }

    method set_defaults {} {
    	global $this-method
        global $this-ed-method
        
        set $this-method "transform"
        set $this-ed-method "affine"
        
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        sci_toplevel $w
        
        sci_frame $w.frame
        pack $w.frame -fill x
        
        sci_tabnotebook $w.frame.filters -height 100 -tabpos n        
        $w.frame.filters add -label "Transform" -command "set $this-method transform"
        $w.frame.filters select 0

        pack $w.frame.filters -fill x -expand yes
        
        set transform  [$w.frame.filters childsite 0]
 

        sci_frame $transform.f
        pack $transform.f -side left -padx 5p -anchor n

        sci_radiobutton $transform.f.affine -text "Affine" -variable $this-ed-method -value "affine"
        sci_radiobutton $transform.f.morph -text "Morph" -variable $this-ed-method -value "morph"
        sci_radiobutton $transform.f.none -text "None" -variable $this-ed-method -value "none"
     
        
        grid $transform.f.affine -row 0 -column 0 -sticky w
        grid $transform.f.morph -row 0 -column 1 -sticky w
        grid $transform.f.none -row 1 -column 0 -columnspan 2 -sticky w
      
        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}


