itcl_class SCIRun_DataIO_StreamACQFileFromDisk {
    inherit Module
    constructor {config} {
        set name StreamACQFileFromDisk
        set_defaults
    }

    method set_defaults {} {
    
        # Variables for file selector
        setGlobal $this-filename	""
         
    }

    method maybeRestart { args } {
      upvar \#0 $this-execmode execmode
      if [string equal $execmode play] return
      $this-c needexecute
    }


    method update_range { args } {
    
      set w .ui[modname]
      if {[winfo exists $w]} {
      
      
        set scrollbar [set $this-scrollbar]
        upvar \#0 $this-slider_min min $this-slider_max max 
        upvar \#0 $this-lead_or_frame roc	
        
        $scrollbar.min.slider configure -label "Start $roc:" -from $min -to $max -variable $this-range_min
        $scrollbar.cur.slider config -label "Current $roc:" -from $min -to $max -variable $this-current
        $scrollbar.max.slider config -label "End $roc:" -from $min -to $max -variable $this-range_max
        $scrollbar.inc.slider config -label "Increment current $roc by:" -from 1 -to [expr $max-$min] -variable $this-inc
        $scrollbar.amount.slider config -label "Number of $roc (s) to emit:" -from 1 -to [expr $max-$min] -variable $this-amount

      }
      
      set w [format "%s-control" .ui[modname]]
      if {[winfo exists $w]} {
      
        set scrollbar [set $this-cur]
        upvar \#0 $this-slider_min min $this-slider_max max 
        set pre $roc		
        
        $scrollbar.cur config -label "Current $pre:" -from $min -to $max	
      }
    }
	


    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        sci_toplevel $w

        sci_frame $w.fileframe  -relief groove -borderwidth 2
        set fileframe $w.fileframe 
        pack $w.fileframe -fill x

        sci_frame $fileframe.f1
        sci_frame $fileframe.f2
        
        pack $fileframe.f1 $fileframe.f2 -side top -fill x -expand yes

        sci_label $fileframe.f1.label -text "ACQFile file"
        sci_entry $fileframe.f1.file -textvariable $this-filename
        sci_button $fileframe.f1.bleadse -text "Browse" -command "$this ChooseACQFileFile"
        set $this-filename-entry $fileframe.f1.file  
        pack $fileframe.f1.label -side left
        pack $fileframe.f1.file  -side left -fill x -expand yes
        pack $fileframe.f1.bleadse -side left
      
        sci_frame $w.infoframe 
        set infoframe $w.infoframe 
        pack $w.infoframe -fill x

        sci_frame $w.loopframe -relief groove -borderwidth 2 
        set loopframe $w.loopframe
        pack $w.loopframe -fill x

        sci_frame $loopframe.f1
        sci_frame $loopframe.f2 
        pack $loopframe.f1 -side left -anchor n
        pack $loopframe.f2 -side left -anchor n -fill x  -expand yes 
        sci_frame $loopframe.f1.playmode -relief groove -borderwidth 2
        sci_frame $loopframe.f1.vcr -relief groove -borderwidth 2
        sci_frame $loopframe.f1.roc -relief groove -borderwidth 2
        sci_frame $loopframe.f1.detach -relief groove -borderwidth 2
        sci_frame $loopframe.f2.scrollbar -relief groove -borderwidth 2
        pack $loopframe.f1.vcr $loopframe.f1.roc $loopframe.f1.playmode $loopframe.f1.detach -side top -anchor w -fill x 
        pack $loopframe.f2.scrollbar -side top -anchor w -fill x -expand yes 
                      
        set playmode $loopframe.f1.playmode
        set vcr $loopframe.f1.vcr
        set roc $loopframe.f1.roc
        set scrollbar $loopframe.f2.scrollbar
        set detach $loopframe.f1.detach
        set $this-scrollbar $scrollbar

        sci_button $detach.open -text "Open small control window" -command "$this OpenSmall"
        pack $detach.open 

        # load the VCR button bitmaps
        set image_dir [netedit getenv SCIRUN_SRCDIR]/pixmaps
        set rewind [image create photo -file ${image_dir}/rewind-icon.ppm]
        set stepb [image create photo -file ${image_dir}/step-back-icon.ppm]
        set pause [image create photo -file ${image_dir}/pause-icon.ppm]
        set play [image create photo -file ${image_dir}/play-icon.ppm]
        set stepf [image create photo -file ${image_dir}/step-forward-icon.ppm]
        set fforward [image create photo -file ${image_dir}/fast-forward-icon.ppm]

        # Create and pack the Row of Column frame
        sci_frame $roc.r 
        pack $roc.r -fill both -expand yes
      
        sci_radiobutton $roc.r.lead -text "Lead" -variable $this-lead_or_frame -value lead -command "$this update_range; set $this-execmode update; $this-c needexecute"
        sci_radiobutton $roc.r.col -text "Frame" -variable $this-lead_or_frame -value frame -command "$this update_range; set $this-execmode update; $this-c needexecute"
        pack $roc.r.lead $roc.r.col -side left -expand yes -fill both
    
        # Create and pack the VCR buttons frame
        sci_button $vcr.rewind -image $rewind -command "set $this-execmode rewind;   $this-c needexecute"
        sci_button $vcr.stepb -image $stepb -command "set $this-execmode stepb;    $this-c needexecute"
        sci_button $vcr.pause -image $pause -command "set $this-execmode stop;     $this-c needexecute"
        sci_button $vcr.play  -image $play  -command "set $this-execmode play;     $this-c needexecute"
        sci_button $vcr.stepf -image $stepf -command "set $this-execmode step;     $this-c needexecute"
        sci_button $vcr.fforward -image $fforward -command "set $this-execmode fforward; $this-c needexecute"
        
        pack $vcr.rewind $vcr.stepb $vcr.pause $vcr.play $vcr.stepf $vcr.fforward -side left -fill both -expand 1
        global ToolTipText
        Tooltip $vcr.rewind $ToolTipText(VCRrewind)
        Tooltip $vcr.stepb $ToolTipText(VCRstepback)
        Tooltip $vcr.pause $ToolTipText(VCRpause)
        Tooltip $vcr.play $ToolTipText(VCRplay)
        Tooltip $vcr.stepf $ToolTipText(VCRstepforward)
        Tooltip $vcr.fforward $ToolTipText(VCRfastforward)

        # Save range, creating the scale resets it to defaults.
        set rmin [set $this-slider_min]
        set rmax [set $this-slider_max]
        set rroc [set $this-lead_or_frame]

        # Create the various range sliders
        sci_frame $scrollbar.min
        sci_frame $scrollbar.cur
        sci_frame $scrollbar.max
        sci_frame $scrollbar.inc
        sci_frame $scrollbar.amount
        pack $scrollbar.min $scrollbar.cur $scrollbar.max $scrollbar.inc $scrollbar.amount -side top -anchor w -fill x -expand yes
    
        set newval [set $this-range_min]
        sci_scale $scrollbar.min.slider -variable $this-range_min -length 200 -showvalue true -orient horizontal -relief groove -label "Start $rroc:" -from $rmin -to $rmax 
        sci_spinint $scrollbar.min.count -range {0 86400000} -justify right -width 5 -step 1 -textvariable $this-range_min -repeatdelay 300 -repeatinterval 10 -command "$this maybeRestart"
        set $this-range_min $newval
        pack $scrollbar.min.slider -side left -anchor w -fill x -expand yes
        pack $scrollbar.min.count -side left

        set newval [set $this-current]
        sci_scale $scrollbar.cur.slider -variable $this-current -length 200 -showvalue true -orient horizontal -relief groove -label "Current $rroc:" -from $rmin -to $rmax
        sci_spinint $scrollbar.cur.count -range {0 86400000} -justify right -width 5 -step 1 -textvariable $this-current -repeatdelay 300 -repeatinterval 10 -command "$this maybeRestart" -decrement "incr $this-current -1; $this maybeRestart" -increment "incr $this-current 1; $this maybeRestart"  
        set $this-current $newval
        pack $scrollbar.cur.slider -side left -anchor w -fill x -expand yes
        pack $scrollbar.cur.count -side left
        bind $scrollbar.cur.slider <ButtonRelease> "$this maybeRestart"
        
        set newval [set $this-range_max]
        sci_scale $scrollbar.max.slider -variable $this-range_max -length 200 -showvalue true -orient horizontal -relief groove -label "End $rroc:" -from $rmin -to $rmax
        sci_spinint $scrollbar.max.count -range {0 86400000} -justify right -width 5 -step 1 -textvariable $this-range_max -repeatdelay 300 -repeatinterval 10 -command "$this maybeRestart"
        set $this-range_max $newval
        pack $scrollbar.max.slider -side left -anchor w -fill x -expand yes
        pack $scrollbar.max.count -side left

        set newval [set $this-inc-amount]
        sci_scale $scrollbar.inc.slider -variable $this-inc-amount -length 200 -showvalue true -orient horizontal -relief groove -label "Increment current $rroc by:"  -from 1 -to [expr $rmax-$rmin]
        sci_spinint $scrollbar.inc.count -range {0 86400000} -justify right -width 5 -step 1 -textvariable $this-inc-amount -repeatdelay 300 -repeatinterval 10 -command "$this maybeRestart"
        set $this-inc-amount $newval
        pack $scrollbar.inc.slider -side left -anchor w -fill x -expand yes
        pack $scrollbar.inc.count -side left

        set newval [set $this-send-amount]
        sci_scale $scrollbar.amount.slider -variable $this-send-amount -length 200 -showvalue true -orient horizontal -relief groove -label "Number of $rroc (s) to emit:" -from 1 -to [expr $rmax-$rmin]
        sci_spinint $scrollbar.amount.count -range {0 86400000} -justify right -width 5 -step 1 -textvariable $this-send-amount -repeatdelay 300 -repeatinterval 10 -command "$this maybeRestart"
        set $this-send-amount $newval
        pack $scrollbar.amount.slider -side left -anchor w -fill x -expand yes
        pack $scrollbar.amount.count -side left
                 
        # Create and pack the play mode frame
        sci_label $playmode.label -text "Play Mode"
        sci_radiobutton $playmode.once -text "Once" -variable $this-playmode -value once -command "$this maybeRestart"
        sci_radiobutton $playmode.loop -text "Loop" -variable $this-playmode -value loop -command "$this maybeRestart"
        sci_radiobutton $playmode.bounce1 -text "Bounce" -variable $this-playmode -value bounce1 -command "$this maybeRestart"
        sci_radiobutton $playmode.bounce2 -text "Bounce with repeating endpoints" -variable $this-playmode -value bounce2 -command "$this maybeRestart"
        sci_checkbutton $playmode.autoplay -text "Autoplay" -variable $this-autoplay 
        sci_spinint $playmode.delay -labeltext {Step Delay (ms)} -range {0 86400000} -justify right -width 5 -step 10 -textvariable $this-delay -repeatdelay 300 -repeatinterval 10

        pack $playmode.label -side top -expand yes -fill both
        pack $playmode.once $playmode.loop $playmode.bounce1 $playmode.bounce2 $playmode.autoplay $playmode.delay -side top -anchor w

        trace variable $this-current w "update idletasks;\#"
        trace variable $this-delay w "$this maybeRestart;\#"

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
    
 
	method OpenSmall {} {
	
        global $this-current
        global $this-cur
      
        # Create a unique name for the file selection window
        set w [format "%s-control" .ui[modname]]

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

        upvar \#0 $this-selectable_min min $this-selectable_max max 
        upvar \#0 $this-selectable_units units 

        sci_toplevel $w -class TkFDialog
            
        sci_frame $w.vcr -relief groove -borderwidth 2		
        sci_frame $w.cur -relief groove -borderwidth 2		
        set vcr $w.vcr
        set cur $w.cur
        
        pack $w.vcr 
        pack $w.cur -side top -expand yes -fill x
        
        # load the VCR button bitmaps
        set image_dir [netedit getenv SCIRUN_SRCDIR]/pixmaps
        set rewind [image create photo -file ${image_dir}/rewind-icon.ppm]
        set stepb [image create photo -file ${image_dir}/step-back-icon.ppm]
        set pause [image create photo -file ${image_dir}/pause-icon.ppm]
        set play [image create photo -file ${image_dir}/play-icon.ppm]
        set stepf [image create photo -file ${image_dir}/step-forward-icon.ppm]
        set fforward [image create photo -file ${image_dir}/fast-forward-icon.ppm]

        # Create and pack the VCR buttons frame
        sci_button $vcr.rewind -image $rewind -command "set $this-execmode rewind;   $this-c needexecute"
        sci_button $vcr.stepb -image $stepb -command "set $this-execmode stepb;    $this-c needexecute"
        sci_button $vcr.pause -image $pause -command "set $this-execmode stop;     $this-c needexecute"
        sci_button $vcr.play  -image $play  -command "set $this-execmode play;     $this-c needexecute"
        sci_button $vcr.stepf -image $stepf -command "set $this-execmode step;     $this-c needexecute"
        sci_button $vcr.fforward -image $fforward -command "set $this-execmode fforward; $this-c needexecute"
        
        pack $vcr.rewind $vcr.stepb $vcr.pause $vcr.play $vcr.stepf $vcr.fforward -side left -fill both -expand yes
        global ToolTipText
        Tooltip $vcr.rewind $ToolTipText(VCRrewind)
        Tooltip $vcr.stepb $ToolTipText(VCRstepback)
        Tooltip $vcr.pause $ToolTipText(VCRpause)
        Tooltip $vcr.play $ToolTipText(VCRplay)
        Tooltip $vcr.stepf $ToolTipText(VCRstepforward)
        Tooltip $vcr.fforward $ToolTipText(VCRfastforward)
        
        set $this-cur $cur
        sci_scale $cur.cur -variable $this-current -length 200 -showvalue true -orient horizontal -relief groove 
        sci_spinint $cur.count -range {0 86400000} -justify right -width 5 -step 1 -textvariable $this-current -repeatdelay 300 -repeatinterval 10 -command "$this maybeRestart" -decrement "incr $this-current -1; $this maybeRestart" -increment "incr $this-current 1; $this maybeRestart" 
        bind $cur.cur <ButtonRelease> "$this maybeRestart"
        pack $cur.cur	 -fill x -expand yes -side left
        pack $cur.count -side left
        $this update_range
      
        moveToCursor $w
	}


	method ChooseACQFileFile { } {

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

        sci_toplevel $w -class TkFDialog

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
      
        makeOpenFilebox \
          -parent $w \
          -filevar $this-filename-set \
          -command "wm withdraw $w;  $this OpenNewACQFilefile" \
          -commandname "Open" \
          -cancel "wm withdraw $w" \
          -title "Select timeseries data file" \
          -filetypes {{ "CVRTI ACQ File" "*.acq" } }\
          -initialdir $initdir \
          -defaultextension "*.*" \
          -selectedfiletype 0

        wm deiconify $w	
      }
	
	method OpenACQFilefile {} {

		global $this-filename
		global $this-filename-entry
		
		set $this-filename [[set $this-filename-entry] get] 
		
	}

	method OpenNewACQFilefile {} {

		global $this-filename
		global $this-filename-set
		global $this-filename-entry
		
		set $this-filename [set $this-filename-set] 
		
	}   
  
}


