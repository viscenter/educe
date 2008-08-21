itcl_class SCIRun_ChangeFieldData_ApplyFilterToFieldData {
    inherit Module
    
    constructor {config} {
        set name ApplyFilterToFieldData
        set_defaults
    }

    method set_defaults {} {
    
        global $this-method
        global $this-ed-method
        global $this-ed-niterations
        
        set $this-method "erodedilate"
        set $this-ed-method "erode"
        set $this-ed-iterations 3
        
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        
        sci_toplevel $w
        
        sci_frame $w.frame
        pack $w.frame -fill x
        
        sci_tabnotebook $w.frame.filters -height 200 -tabpos n        
        $w.frame.filters add -label "Erode/Dilate" -command "set $this-method erodedilate"
 #       $w.frame.filters add -label "Gaussian Blur" -command "set $this-method gaussblur"
        $w.frame.filters select 0

        pack $w.frame.filters -fill x -expand yes
        
        set erodedilate  [$w.frame.filters childsite 0]
  #      set gaussianblur [$w.frame.filters childsite 1]

        sci_frame $erodedilate.f
        pack $erodedilate.f -side left -padx 5p -anchor n

        sci_radiobutton $erodedilate.f.erode -text "Erode" -variable $this-ed-method -value "erode"
        sci_radiobutton $erodedilate.f.dilate -text "Dilate" -variable $this-ed-method -value "dilate"
        sci_radiobutton $erodedilate.f.dilateerode -text "Dilate -> Erode" -variable $this-ed-method -value "dilateerode"
        sci_label $erodedilate.f.niterationslabel -text "number of iterations"
        sci_entry $erodedilate.f.niterations -textvariable $this-ed-iterations
        
        grid $erodedilate.f.erode -row 0 -column 0 -sticky w
        grid $erodedilate.f.dilate -row 0 -column 1 -sticky w
        grid $erodedilate.f.dilateerode -row 1 -column 0 -columnspan 2 -sticky w
        grid $erodedilate.f.niterationslabel -row 2 -column 0 -sticky w
        grid $erodedilate.f.niterations -row 2 -column 1 -sticky w

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}


