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


itcl_class SCIRun_Visualization_ShowFieldGlyphs2 {
    inherit Module
    constructor {config} {
#	puts "constructor"
	set name ShowFieldGlyphs2
	set_defaults
    }

    method set_defaults {} {
#	puts "set_defaults"

	trace variable $this-primary_has_data w "$this glyphs_display_type_changed"
	trace variable $this-active_tab w "$this switch_to_active_tab"
	trace variable $this-glyphs_display_type w "$this glyphs_display_type_changed"
	trace variable $this-colormap_color w "$this glyphs_color_type_changed"

# This is the initial (blank) glyph image displayed.
# Glyphs are Gif images of size 192x132 that are then encoded are Base-64 encoded, and then copied into the code.
# I used  http://www.motobit.com/util/base64-decoder-encoder.asp to encode the files.
	global $this-glyph_image
	set $this-glyph_image "R0lGODlhwACEAIAAAL6+vv///yH5BAEAAAEALAAAAADAAIQAAALHhI+py+0Po5y02ouz3rz7D4biSJbmiabqyrbuC8fyTNf2jef6zvf+DwwKh8Si8YhMKpfMpvMJjUqn1Kr1is1qt9yu9wsOi8fksvmMTqvX7Lb7DY/L5/S6/Y7P6/f8vv8PGCg4SFhoeIiYqLjI2Oj4CBkpOUlZaXmJmam5ydnp+QkaKjpKWmp6ipqqusra6voKGys7S1tre4ubq7vL2+v7CxwsPExcbHyMnKy8zNzs/AwdLT1NXW19jZ2tvc3d7f0NHi4+Tk5ZAAA7"
	
	# no C side component for these variables
	global $this-glyph_tab_exists
	set $this-glyph_tab_exists 0
	
# 	global $this-gs_slider
# 	set $this-gs_slider "not.set.yet"
	
	global parameterType
	set parameterType ""
	
	global parameterDefault
	set parameterDefault "" 
	
	global  chosen_glyph_type
	set chosen_glyph_type ""
	
	global parameters
	set parameters ""
	
	global paramframe
	set paramframe ""
	
	global glyph_choice
	set glyph_choice "Choose Glyph"
	
	global default_backgroundColor
	set default_backgroundColor [format #%04x%04x%04x 48640 48640 48640]
	
 #########
 # Meaning of the Default codes for the parameters
 #########
	global tensor_eigenvectors
	set tensor_eigenvectors "{Primary Eigenvector} {Secondary Eigenvector} {Tertiary Eigenvector}"
	global tensor_eigenvectors_codes
	set tensor_eigenvectors_codes "4010 4020 4030"

	global tensor_components
	set tensor_components  "m11 m12 m13 m21 m22 m23 m31 m32 m33"
	global tensor_components_codes
	set tensor_components_codes  "6001 6002 6003 6004 6005 6006 6007 6008 6009"

	global tensor_eigenvector_magnitudes
	set tensor_eigenvector_magnitudes "{Primary Eigenvector Magnitude} {Secondary Eigenvector Magnitude} \
 {Tertiary Eigenvector Magnitude}"
	global tensor_eigenvector_magnitudes_codes
	set tensor_eigenvector_magnitudes_codes "4001 4002 4003"

	global tensor_eigenvector_components
	set tensor_eigenvector_components "{Primary Eigenvector Vx} {Primary Eigenvector Vy} {Primary Eigenvector Vz}\
 {Secondary Eigenvector Vx}   {Secondary Eigenvector Vy} {Secondary Eigenvector Vz}\
 {Tertiary Eigenvector Vx} {Tertiary Eigenvector Vy} {Tertiary Eigenvector Vz}"
	global tensor_eigenvector_components_codes
	set tensor_eigenvector_components_codes "5001 5002 5003 5004 5005 5006 5007 5008 5009"

	global vector_components
	set vector_components "Vx Vy Vz"
	global vector_components_codes
	set vector_components_codes "2002 2003 2004"
    }

#######################
# Helper functions for generating the menus for the parameters.
# Each Process is for a Specific Input Field Type to a parameter type.
#######################    
    proc Tensor_Scalar_Field_Menu { tab parent field_num  menubutton var val menuname code_var } {
	global tensor_eigenvector_magnitudes
	global tensor_eigenvector_magnitudes_codes
	global tensor_eigenvector_components
	global tensor_eigenvector_components_codes
	global tensor_components
	global tensor_components_codes
	
	#field submenu
	Menu_Cascade $tab $parent $menubutton "Field $field_num: Tensor"

	#Eigenvector Magnitude submenu
	Menu_Cascade $tab $menubutton "Field $field_num: Tensor" "$field_num: Tensor Eigenvector Magnitude"
	foreach t $tensor_eigenvector_magnitudes code $tensor_eigenvector_magnitudes_codes {
	    set cmd [subst {$menuname configure -text "Field $field_num: Tensor $t"; \
				set code_var $code; set ${code_var}_field $field_num}]
	    Menu_Radio $tab "Field $field_num: Tensor" "$field_num: Tensor Eigenvector Magnitude" "$field_num: $t" $var $val $cmd;
	    incr val;
	}

	#Eigenvector Components submenu
	Menu_Cascade $tab $menubutton "Field $field_num: Tensor" "$field_num: Tensor Eigenvector Components"
	foreach t $tensor_eigenvector_components code $tensor_eigenvector_components_codes {
	    set cmd [subst {$menuname configure -text "Field $field_num: Tensor Component $t"; \
				set code_var $code; set ${code_var}_field $field_num}]
	    Menu_Radio $tab "Field $field_num: Tensor" "$field_num: Tensor Eigenvector Components" "$field_num: $t" $var $val $cmd;
	    incr val;
	}

	#Tensor Components submenu
	Menu_Cascade $tab  $menubutton "Field $field_num: Tensor" "$field_num: Tensor Components"
	foreach t $tensor_components code $tensor_components_codes {
	    set cmd [subst {$menuname configure -text "Field $field_num: Tensor Component $t"; \
				set code_var $code; set ${code_var}_field $field_num}]
	    Menu_Radio $tab "Field $field_num: Tensor" "$field_num: Tensor Components" "$field_num: $t" $var $val $cmd;
	    incr val;
	}

	return $val
    }

    proc Tensor_Vector_Field_Menu { tab parent field_num  menubutton var val menuname code_var } {
	global tensor_eigenvectors
	global tensor_eigenvectors_codes

	#field submenu
	Menu_Cascade $tab $parent $menubutton "Field $field_num: Tensor"

	#Eigenvector Magnitude submenu
	Menu_Cascade $tab $menubutton "Field $field_num: Tensor" "$field_num: Tensor Eigenvectors"
	foreach t $tensor_eigenvectors code $tensor_eigenvectors_codes {
	    set cmd [subst {$menuname configure -text "Field $field_num: Tensor Eigenvectors $t"; \
				set code_var $code; set ${code_var}_field $field_num}]
	    Menu_Radio $tab "Field $field_num: Tensor" "$field_num: Tensor Eigenvectors" "$field_num: $t" $var $val $cmd;
	    incr val;
	}

	return $val
    }

    proc Tensor_Tensor_Field_Menu { tab  parent field_num  menubutton var val menuname code_var } {
	set cmd [subst {$menuname configure -text "Field $field_num: Tensor"; set code_var "4100"; \
			    set ${code_var}_field $field_num}]
	Menu_Radio $tab $parent $menubutton "Field $field_num: Tensor" $var $val $cmd;

	return $val
    }
    
    proc Vector_Vector_Field_Menu { tab parent field_num  menubutton var val menuname code_var } {
	set cmd [subst {$menuname configure -text "Field $field_num: Vector"; set code_var "2010"; \
			    set ${code_var}_field $field_num}]
	Menu_Radio $tab $parent $menubutton "Field $field_num: Vector" $var $val $cmd;

	return $val
    }
    
    proc Vector_Scalar_Field_Menu { tab parent field_num  menubutton var val menuname code_var } {
	global vector_components
	global vector_components_codes


	#Vector Magnitude
	set cmd [subst {$menuname configure -text "Field $field_num: Vector Magnitude"; \
			    set code_var "2001"; set ${code_var}_field $field_num}]
	Menu_Radio $tab $parent $menubutton "Field $field_num: Vector Magnitude" $var $val $cmd;

	#Vector Components submenu
	Menu_Cascade $tab $parent $menubutton "Field $field_num: Vector Components"
	foreach t $vector_components code $vector_components_codes {
	    incr val;
	    set cmd [subst {$menuname configure -text "Field $field_num: Vector Component $t"; \
				set code_var $code; set ${code_var}_field $field_num}]
	    Menu_Radio $tab $menubutton "Field $field_num: Vector Components" "$field_num: $t" $var $val $cmd;
	}

	return $val
    }
    
    proc Scalar_Scalar_Field_Menu { tab parent field_num  menubutton var val menuname code_var } {
	set cmd [subst {$menuname configure -text "Field $field_num: Scalar"; \
			    set code_var "1001"; set ${code_var}_field $field_num}]

	Menu_Radio $tab $parent $menubutton "Field $field_num: Scalar" $var $val  $cmd;
	
	return $val
    }

####################################
# Helper Functions for generating the dynamic cascading menus
####################################

    proc Menu_Setup { tab menubar } {
	global menu2

	frame $tab.$menubar
	pack $tab.$menubar -side top -fill x

	set menu2($tab.menubar) $tab.$menubar
	set menu2($tab.uid) 0
    }

    proc Menubutton { tab label } {
	global menu2

	if [info exists menu2(menu,$label)] {
	    error "Menu $label already defined"
	}

	# Create the menubutton and its menu
	set name $menu2($tab.menubar).mb$menu2($tab.uid)
	set menuName $name.menu
	incr menu2($tab.uid)
	set mb [menubutton $name -text $label -menu $menuName -relief raised]
	pack $mb -side left
	menu $menuName -tearoff 0

	# Remember the name to menu mapping
	set menu2(menu.$tab,$label) $menuName
    }

    # A menu parent is needed to dis-ambiguate menus the have the 
    # same name under diferent parent menus.
    proc MenuGetParent {tab menuName} {
	global menu2

	if [catch {set menu2(menu.$tab,$menuName)} m] {
	    return -code error "MenuGetParent: No such parent menu: $menuName"
	}

	return $m
    }

    proc MenuGet {tab parent menuName} {
	global menu2

	if [catch {set menu2(menu$parent.$tab,$menuName)} m] {
	    return -code error "MenuGet2: No such menu: $menuName"
	}

	return $m
    }

    proc Menu_Cascade { tab parent menuName label } {
	global menu2

	set m [MenuGet $tab $parent  $menuName]

	if [info exists menu2(menu$parent.$tab,$label)] {
	    error "Menu_Cascade: Menu2 $label already defined"
	}

	set sub $m.sub$menu2($tab.uid)
	incr menu2($tab.uid)
	menu $sub -tearoff 0
	$m add cascade -label $label -menu $sub
	set menu2(menu$menuName.$tab,$label) $sub
    }

    proc Menu_Command {tab parent menuName label command } {
	set m [MenuGet $tab $parent $menuName]

	$m add command -label $label -command $command
    }

    proc Menu_Check { tab parent menuName label var { command {} } } {
	set m [MenuGet $tab  $parent $menuName]

	$m add check -label $label -command $command \
	    -variable $var
    }

    proc Menu_Radio { tab parent menuName label var {val {}} {command {}} } {
	set m [MenuGet $tab  $parent $menuName]

	if {[string length $val] == 0} {
	    set val $label
	}

	$m add radio -label $label -command $command \
	    -value $val -variable $var
    }

    proc Menu_Separator {tab parent  menuName } {
	[MenuGet $tab  $parent $menuName] add separator
    }

#For handling when $this-colormap_color is modified, 
# by changing the color on the colormap parameter menubutton.
    method glyphs_color_type_changed {name1 name2 op} {
# 	puts "glyphs_color_type_changed"

	global glyph
        global default_backgroundColor
        global $this-colormap_color
        global paramframe

	set menuname $paramframe.colormap.colormapMenu.mb0

 	if {[set $this-colormap_color] == 0  } {
 	    $menuname   configure -background  $default_backgroundColor
 	} else { 
 	    $menuname   configure -background  "Grey"
 	}

    }

# Handles when there are changes to the state of $this-primary_has_data
# and $this-glyphs_display_type changes. These happen when the primary
# field has data connected/disconnected to it, and when the user changes the 
# type of glyph to display.
    method glyphs_display_type_changed {name1 name2 op} {
# 	puts "glyphs_display_type_changed"

	global glyph
	global $this-glyph_paramters
	global $this-primary_has_data
	global $this-glyph_tab_exists
	global parameters

	#Load up parameters
	$this-c "get_parameters" [set $this-glyphs_display_type]

	set parameters [set $this-glyph_paramters]

	# load of the Glyph image
	set i2 [image create photo -width 200 -height 130 -data [set $this-glyph_image]];
	
	set window .ui[modname]
	if {[winfo exists $window]} {
	    set dof [$window.options.disp.frame_title childsite]	
	    
	    #If Glyph tab exists, remove it, so it can be rebuilt
	    if {[set $this-glyph_tab_exists] } {
		$dof.tabs delete "Glyph"
		set $this-glyph_tab_exists 0
	    }

	    #Create Glyph Tab	    
	    add_glyph_tab $dof

	    # Load glyph image
	    $glyph.glyph.img create image 100 65 -image $i2 ;
	    
	    $dof.tabs view [set $this-active_tab]
	}
    }
    
    method raiseColor {col color colMsg} {
# 	puts "raiseColor"

	global $color
	set window .ui[modname]
	if {[winfo exists $window.color]} {
	    SciRaise $window.color
	    return
	} else {
	    # makeColorPicker now creates the $window.color toplevel.
	    makeColorPicker $window.color $color \
		"$this setColor $col $color $colMsg" \
		"destroy $window.color"
	}
    }

    method setColor {col color colMsg} {
#	puts "setColor"

	global $color
	global $color-r
	global $color-g
	global $color-b
	global paramframe
	global glyph
	global default_backgroundColor
	global $this-colormap_color
	
	set ir [expr int([set $color-r] * 65535)]
	set ig [expr int([set $color-g] * 65535)]
	set ib [expr int([set $color-b] * 65535)]
	
	set default_backgroundColor [format #%04x%04x%04x $ir $ig $ib]
	
	set window .ui[modname]
	$col config -background $default_backgroundColor
	
	$this-c $colMsg
	
	#Get the correct menu button for the colormap parameter, to change it's color
	if {[set $this-colormap_color]} {
	    set thislength [string length $this]
	    set outstring [string replace $paramframe 0 $thislength $this]
	    set outstring [string replace $outstring 0 1 ".ui"]
	    # Change the color on the colormap button if Default color is selected
	    $outstring.colormap.colormapMenu.mb0 configure -background [format #%04x%04x%04x $ir $ig $ib]
	}
    }

    method addColorSelection {frame text color colMsg} {
# 	puts "addColorSelection"

	#add node color picking 
	global default_backgroundColor
	global $this-colormap_color
	global paramframe
	global $color
	global $color-r
	global $color-g
	global $color-b

	set ir [expr int([set $color-r] * 65535)]
	set ig [expr int([set $color-g] * 65535)]
	set ib [expr int([set $color-b] * 65535)]

	set default_backgroundColor [format #%04x%04x%04x $ir $ig $ib]

	if {$default_backgroundColor != "#ffffffffffff"} {
	    #Get the correct menu button
	    if {[set $this-colormap_color]} {
		set thislength [string length $this]
		set outstring [string replace $paramframe 0 $thislength $this]
		if {$outstring != ""} {
		    set outstring [string replace $outstring 0 1 ".ui"]
		    # Change the color on the colormap button if Default color is selected
		    $outstring.colormap.colormapMenu.mb0 configure -background [format #%04x%04x%04x $ir $ig $ib]
		}
	    }
	}

	frame $frame.colorFrame
	frame $frame.colorFrame.col -relief ridge -borderwidth \
	    4 -height 0.8c -width 1.0c \
	    -background $default_backgroundColor
	
	set cmmd "$this raiseColor $frame.colorFrame.col $color $colMsg"
	button $frame.colorFrame.set_color \
	    -text $text -command $cmmd
	
	#pack the node color frame
	pack $frame.colorFrame.set_color $frame.colorFrame.col -side left -padx 2
	pack $frame.colorFrame -side left
    }

    method set_active_tab {act} {
#	puts "set_active_tab"

	global $this-active_tab

	set $this-active_tab $act
    }

    method switch_to_active_tab {name1 name2 op} {
#	puts "switch_to_active_tab"

	set window .ui[modname]
	if {[winfo exists $window]} {
	    set dof [$window.options.disp.frame_title childsite]
	    $dof.tabs view [set $this-active_tab]
	}
    }

    # Text Tab
    method add_text_tab {dof} {
#	puts "add_text_tab"

	set text [$dof.tabs add -label "Text" \
		      -command "$this set_active_tab \"Text\""]
	checkbutton $text.show_text \
	    -text "Show Text" \
	    -command "$this-c toggle_display_text" \
	    -variable $this-text_on

	frame $text.def_col -borderwidth 2

	checkbutton $text.backfacecull \
	    -text "Cull backfacing text if possible" \
	    -command "$this-c rerender_text" \
	    -variable $this-text_backface_cull

	checkbutton $text.alwaysvisible \
	    -text "Text always visible (not hidden by faces)" \
	    -command "$this-c rerender_text" \
	    -variable $this-text_always_visible

	checkbutton $text.locations \
	    -text "Render indices as locations" \
	    -command "$this-c rerender_text" \
	    -variable $this-text_render_locations

	frame $text.show 
	checkbutton $text.show.data \
	    -text "Show data values" \
	    -command "$this-c rerender_text" \
	    -variable $this-text_show_data
	checkbutton $text.show.nodes \
	    -text "Show node indices" \
	    -command "$this-c rerender_text" \
	    -variable $this-text_show_nodes
	checkbutton $text.show.edges \
	    -text "Show edge indices" \
	    -command "$this-c rerender_text" \
	    -variable $this-text_show_edges
	checkbutton $text.show.faces \
	    -text "Show face indices" \
	    -command "$this-c rerender_text" \
	    -variable $this-text_show_faces
	checkbutton $text.show.cells \
	    -text "Show cell indices" \
	    -command "$this-c rerender_text" \
	    -variable $this-text_show_cells

	make_labeled_radio $text.size \
	    "Text Size:" "$this-c rerender_text" left 5 \
	    $this-text_fontsize \
	    {{"XS" 0} {"S" 1} {"M" 2} {"L" 3} {"XL" 4}}

	pack $text.show.data $text.show.nodes $text.show.edges \
	    $text.show.faces $text.show.cells \
	    -side top -fill y -anchor w
	
	global $this-text_color_type
	make_labeled_radio $text.color \
	    "Text Coloring" "$this-c rerender_text" top 3 \
	    $this-text_color_type \
	    { {Text 0} {"Colormap Lookup" 1} {"RGB Conversion" 2} }

	addColorSelection $text.def_col "Text Color" $this-text_color \
	    "text_color_change"

	frame $text.precision
	label $text.precision.label -text "Text Precision  "
	scale $text.precision.scale -orient horizontal \
	    -variable $this-text_precision -from 1 -to 16 \
	    -showvalue true -resolution 1
	bind $text.precision.scale <ButtonRelease> "$this-c rerender_text"
	pack $text.precision.label -side left -anchor s -fill x
	pack $text.precision.scale -side left -anchor n -fill x
	
	pack $text.show_text $text.show $text.backfacecull \
	    $text.alwaysvisible $text.locations \
	    $text.color $text.def_col $text.size \
	    $text.precision -side top -pady 2 -fill y -anchor w
    }

###############
# This procedure decodes the default parameter value for the 
# parameter to load up it's default value.
###############
    proc decode_paramater_default { default_code } {
#	puts "decode_paramater_default"

	global parameterType
	global parameterDefault

	# Determine input field datatype
	set input_field_code [string index $default_code 0]
	set tensor_output_field_code [string index $default_code 1]
	set vector_output_field_code [string index $default_code 2]
	set scalar_output_field_code [string index $default_code 3]

	# Verify Correct Format
	if {[string length $default_code] != 4} {
	    error "Invalid default code length: \"$default_code\""}
	if {![string match {[12456]*} $default_code ] } {	error "Invalid Input Field Type: \"$default_code\""}
	if {![regexp {(^[12456]00.$|^[12456]0.0$|^[12456].00$)} $default_code]} {
	    error "Invalid Output Field Code: \"$default_code\""}

	
	switch $input_field_code {
	    "1" {lappend parameterType "Scalar"; 
		if {$tensor_output_field_code != 0} {
		    error "Tensor Output Error - code \"$default_code\""
		} elseif {$vector_output_field_code != 0} {
		    error "Vector Output Error - code \"$default_code\""
		} elseif {$scalar_output_field_code !=0} {
		    switch $scalar_output_field_code {
			"1" {lappend parameterDefault "Scalar Value"}
			"2" {lappend parameterDefault "1/6 Length"}
			"3" {lappend parameterDefault "1/6 Diameter"}
			default {error "Scalar Output Error - code \"$default_code\""}
		    }
		} else {error "Output Error - code \"$default_code\""}
	    }

	    "2" { #lappend parameterType "Vector"; 
		if {$tensor_output_field_code != 0} {
		    error "Tensor Output Error - code \"$default_code\""
		} elseif {$vector_output_field_code !=0} {
		    switch $vector_output_field_code {
			"1" {lappend parameterType "Vector"; lappend parameterDefault "Vector Orientation"}
			default {error "Vector Error - code \"$default_code\""}
		    }
		} elseif {$scalar_output_field_code !=0} {
		    switch $scalar_output_field_code {
			"1" {lappend parameterType "Scalar"; lappend parameterDefault "Vector Magnitude"}
			"2" {lappend parameterType "Scalar"; lappend parameterDefault "Vector Component Vx"}
			"3" {lappend parameterType "Scalar"; lappend parameterDefault "Vector Component Vy"}
			"4" {lappend parameterType "Scalar"; lappend parameterDefault "Vector Component Vz"}
			default {error "Scalar Output Error - code \"$default_code\""}
		    }
		} else {error "Output Error - code \"$default_code\""}
	    }

	    "4" { #lappend parameterType "Tensor";
		if {$tensor_output_field_code == 1} {
		    lappend parameterType "Tensor"; 
		    lappend parameterDefault "Tensor Matrix"
		} elseif {$vector_output_field_code !=0} {
		    switch $vector_output_field_code {
			"1" {lappend parameterType "Vector"; lappend parameterDefault "Primary Tensor Eigenvector"}
			"2" {lappend parameterType "Vector"; lappend parameterDefault "Secondary Tensor Eigenvector"}
			"3" {lappend parameterType "Vector"; lappend parameterDefault "Tertianary Tensor Eigenvector"}
			default {error "Vector Output Error"}
		    }
		} elseif {$scalar_output_field_code !=0} {
		    switch $scalar_output_field_code {
			"1" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Primary Eigenvalue"}
			"2" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Secondary Eigenvalue"}
			"3" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Tertianary Eigenvalue"}
			default {error "Scalar Output Error - code \"$default_code\""}
		    }
		} else {error "Output Error - code \"$default_code\""}
	    }

	    "5" { #lappend parameterType "Tensor";
		if {$tensor_output_field_code == 1} {
		    lappend parameterType "Tensor"; 
		    lappend parameterDefault "Tensor Matrix"
		} elseif {$vector_output_field_code !=0} {
		    switch $vector_output_field_code {
			"1" {lappend parameterType "Vector"; lappend parameterDefault "Primary Tensor Eigenvector"}
			"2" {lappend parameterType "Vector"; lappend parameterDefault "Secondary Tensor Eigenvector"}
			"3" {lappend parameterType "Vector"; lappend parameterDefault "Tertianary Tensor Eigenvector"}
			default {error "Vector Output Error - code \"$default_code\""}
		    }
		} elseif {$scalar_output_field_code !=0} {
		    switch $scalar_output_field_code {
			"1" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Primary Vx"}
			"2" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Primary Vy"}
			"3" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Primary Vz"}
			"4" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Secondary Vx"}
			"5" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Secondary Vy"}
			"6" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Secondary Vz"}
			"7" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Tertianary Vx"}
			"8" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Tertianary Vy"}
			"9" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Eigenvalue Component Tertianary Vz"}
			default {error "Scalar Output Error - code \"$default_code\""}
		    }
		} else {error "Output Error - code \"$default_code\""}
	    }

	    "6" { #lappend parameterType "Tensor"; 
		if {$tensor_output_field_code == 1} {
		    lappend parameterType "Tensor"; 
		    lappend parameterDefault "Tensor Matrix"
		} elseif {$vector_output_field_code !=0} {
		    switch $vector_output_field_code {
			"2" {lappend parameterType "Vector"; lappend parameterDefault "Primary Tensor Eigenvector"}
			"3" {lappend parameterType "Vector"; lappend parameterDefault "Secondary Tensor Eigenvector"}
			"4" {lappend parameterType "Vector"; lappend parameterDefault "Tertianary Tensor Eigenvector"}
			default {error "Vector Output Error - code \"$default_code\""}
		    }
		} elseif {$scalar_output_field_code !=0} {
		    switch $scalar_output_field_code {
			"1" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m11"}
			"2" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m12"}
			"3" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m13"}
			"4" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m21"}
			"5" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m22"}
			"6" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m23"}
			"7" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m31"}
			"8" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m32"}
			"9" {lappend parameterType "Scalar"; lappend parameterDefault "Tensor Component m33"}
			default {error "Scalar Output Error - code \"$default_code\""}
		    }
		} else {error "Output Error - code \"$default_code\""}
	    }

	    default {error "Error: Undefined code: \"$default_code\""}
	}
    }

##########################################################################
# This is the added code to generate the Glyph tab with all it's
# dynamic cascading menus and sliders.
##########################################################################
    # Glyph Tab
    method add_glyph_tab {dof} {
#	puts "add_glyph_tab"

	# Global Variables
	global glyph
	global $this-glyph_tab_exists
	global $this-primary_has_Scalar_data
	global $this-primary_has_Vector_data
	global $this-primary_has_Tensor_data
	global $this-secondary_has_Scalar_data
	global $this-secondary_has_Vector_data
	global $this-secondary_has_Tensor_data
	global $this-tertiary_has_Scalar_data
	global $this-tertiary_has_Vector_data
	global $this-tertiary_has_Tensor_data
	global $this-scalar_glyphs
	global $this-scalar_glyphs_values
	global $this-scalar_glyphs_images
	global $this-vector_glyphs
	global $this-vector_glyphs_values
	global $this-vector_glyphs_images
	global $this-tensor_glyphs
	global $this-tensor_glyphs_values
	global $this-tensor_glyphs_images
	global $this-glyphs_display_type
global $this-glyphs_resolution
global $this-glyph_scale
global $this-glyph_scaleNV
	global $this-glyph_image
	global glyph_choice
	global parameters
	global parameterType
	global parameterDefault
	global  chosen_glyph_type
	global $this-glyph_parameter0
	global $this-glyph_parameter0_field
	global $this-glyph_parameter1
	global $this-glyph_parameter1_field
	global $this-glyph_parameter2
	global $this-glyph_parameter2_field
	global $this-glyph_parameter3
	global $this-glyph_parameter3_field
	global $this-glyph_parameter4
	global $this-glyph_parameter4_field
	global $this-glyph_parameter5
	global $this-glyph_parameter5_field
	global $this-glyph_parameter6
	global $this-glyph_parameter6_field
	global $this-glyph_parameter7
	global $this-glyph_parameter7_field
	global $this-glyph_alpha
	global $this-glyph_alpha_field
	global glyph_colormap
	global glyph_colormap_field
	global default_backgroundColor
	global paramframe
global $this-glyphs_color_type

#Set intial values for tcl
	set glyph_parameter0 "9999"
	set glyph_parameter0_field 0
	set glyph_parameter1 "9999"
	set glyph_parameter1_field 0
	set glyph_parameter2 "9999"
	set glyph_parameter2_field 0
	set glyph_parameter3 "9999"
	set glyph_parameter3_field 0
	set glyph_parameter4 "9999"
	set glyph_parameter4_field 0
	set glyph_parameter5 "9999"
	set glyph_parameter5_field 0
	set glyph_parameter6 "9999"
	set glyph_parameter6_field 0
	set glyph_parameter7 "9999"
	set glyph_parameter7_field 0
	set glyph_alpha "9999"
	set glyph_alpha_field 0
	set glyphs_color_type "9999"
	set glyphs_color_type_field 0

	#Constants for tab
	set Tensor 3
	set Vector 2
	set Scalar 1

	# Temporary set up variables for debugging
	#set $this-primary_has_Tensor_data 1
	#set $this-primary_has_Vector_data 0
	#set $this-primary_has_Scalar_data 0
	#set $this-secondary_has_Tensor_data 0
	#set $this-secondary_has_Vector_data 1
	#set $this-secondary_has_Scalar_data 0
	#set $this-tertiary_has_Tensor_data 0
	#set $this-tertiary_has_Vector_data 0
	#set $this-tertiary_has_Scalar_data 1


	#Initialize to empty strings
	set parameterList ""
	set parameterType ""
	set parameterDefault ""
	

	# Extract Parameter names and default codes from the parameter list
	foreach {parameter defaultVal} $parameters {
	    lappend parameterList $parameter
	    decode_paramater_default $defaultVal
	}

	#Determine what subsets of Glyphs to give as choices
	set highest_data_type "null"
	if {[set $this-primary_has_Tensor_data] == 1 } {
	    set highest_data_type "tensor"
	} elseif {[set $this-primary_has_Vector_data] == 1  } {
	    set highest_data_type "vector"
	} elseif {[set $this-primary_has_Scalar_data] == 1 } {
	    set highest_data_type "scalar"
	} else {
	    set highest_data_type "null"
	}

	#Create the tab variable glyph
	set glyph [$dof.tabs add -label "Glyph" \
		       -command "$this set_active_tab \"Glyph\""]
	set $this-glyph_tab_exists 1

	# Selector Frame
	frame $glyph.glyph

	#Glyph selector
	frame $glyph.glyph.ted
	pack $glyph.glyph.ted -side left -anchor nw

	frame $glyph.glyph.ted.selector
	pack $glyph.glyph.ted.selector -side top -fill x -expand 1 -padx 15   -pady 12

	label $glyph.glyph.ted.selector.glyphMenu_label -text "Selected Glyph:"
	pack $glyph.glyph.ted.selector.glyphMenu_label -side top -fill y -expand 0

	# get the latest Glyphs that are available to the user, this call the C code.
	$this-c "get_glyphs"

###################
# THis code Generates the Glyph Type menu at the top of the Glyph Tab
###################
	#Create glyph menu
	Menu_Setup $glyph.glyph.ted.selector glyphMenu 
	Menubutton $glyph.glyph.ted.selector  {Choose Glyph}

	#Scalar sub-menu
	if { $highest_data_type == "tensor" | $highest_data_type == "vector" | $highest_data_type == "scalar"} {
	    Menu_Cascade $glyph.glyph.ted.selector ""  "Choose Glyph" Scalar;
	    set i 0
	    set glyph_type "scalar"
	    foreach t [set $this-scalar_glyphs] {
		set var [lindex [set $this-scalar_glyphs_values] $i]
		set cmd [subst { set chosen_glyph_type $glyph_type;  set glyph_choice "$t"; set $glyph.glyph $var ;\
				     $glyph.glyph.ted.selector.glyphMenu.mb0 configure -text "$t"; \
				     $this-c "get_image" [expr ($i + 1)]; set $this-glyphs_display_type [expr $i + 1];}]

		Menu_Radio $glyph.glyph.ted.selector "Choose Glyph" Scalar $t $this-glyph_value $var $cmd;

		incr i
	    }
	}

	#Vector sub-menu
	if { $highest_data_type == "tensor" | $highest_data_type == "vector" } {
	    Menu_Cascade $glyph.glyph.ted.selector ""  "Choose Glyph" Vector
	    set i 0
	    set glyph_type "vector"
	    foreach t [set $this-vector_glyphs] {
		set var [lindex [set $this-vector_glyphs_values] $i]
		set cmd [subst { set chosen_glyph_type $glyph_type;  set glyph_choice "$t"; \
				     $glyph.glyph.ted.selector.glyphMenu.mb0 configure -text "$t"; \
				     $this-c "get_image" [expr ($i + 1)*16]; set $this-glyphs_display_type [expr ($i + 1)*16]; }]

		Menu_Radio $glyph.glyph.ted.selector "Choose Glyph" Vector $t $this-glyph_value $var $cmd

		incr i
	    }
	}

	#Tensor sub-menu
	if { $highest_data_type == "tensor"} {
	    Menu_Cascade $glyph.glyph.ted.selector "" "Choose Glyph" Tensor
	    set i 0
	    set glyph_type "tensor"
	    foreach t [set $this-tensor_glyphs] {
		set var [lindex [set $this-tensor_glyphs_values] $i]
		set cmd [subst { set chosen_glyph_type $glyph_type;  set glyph_choice "$t"; \
				     $glyph.glyph.ted.selector.glyphMenu.mb0 configure -text "$t"; \
				     $this-c "get_image" [expr ($i + 1)*256]; set $this-glyphs_display_type [expr ($i + 1)*256]; }]

		Menu_Radio $glyph.glyph.ted.selector "Choose Glyph" Tensor $t $this-glyph_value $var $cmd

		incr i
	    }
	}

	pack $glyph.glyph.ted.selector.glyphMenu -side left -fill none -expand 0 

	#Set ChooseGlyph Menu Label
	$glyph.glyph.ted.selector.glyphMenu.mb0 configure -text $glyph_choice

#####################
#The Glyph option buttons at the top left of the glyph tab.
#####################
	# Glyph buttons
	frame $glyph.glyph.ted.buttons
	pack $glyph.glyph.ted.buttons -side top -padx 5

	#Show Glyph Button
	checkbutton $glyph.glyph.ted.buttons.use -text "Show Glyph"  -variable $this-glyphs_on 
	grid $glyph.glyph.ted.buttons.use  -row 1 -sticky w

	
	#Glyph Normalize Button
	checkbutton $glyph.glyph.ted.buttons.normalize -text "Normalize data"  -variable $this-glyphs_normalize
	grid $glyph.glyph.ted.buttons.normalize -row 2 -sticky w

	# Vector Glyphs Bidirectional
	if { $chosen_glyph_type == "vector" } {
	    checkbutton $glyph.glyph.ted.buttons.bidirectional -text "Vector Bidirectional"  -variable $this-vectors_bidirectional
	    grid $glyph.glyph.ted.buttons.bidirectional -row 4 -sticky w
	}

	pack $glyph.glyph -side top

##############################
#This is the Resizable frame for holding all of the parameters
##############################

	#Canvas glyph display
	canvas $glyph.glyph.img -relief groove -height 130 -width 200 

	pack $glyph.glyph.img -side right -fill none -expand 0 -padx 2 -pady 2
	
	#parameters Frame
	frame $glyph.param_label
	label $glyph.param_label.pram_label -text "Parameter(Data Type)" -width 25 -bg "yellow"
	label $glyph.param_label.data_label -text "Input Data" -width 25 -bg "yellow"
	pack $glyph.param_label.pram_label -side left
	pack $glyph.param_label.data_label -side left
	pack $glyph.param_label  -side top -fill x -expand 0

	#parameters Frame
	# create the scrolled frame
	iwidgets::scrolledframe $glyph.param -width 640 -height 90 \
	    -vscrollmode dynamic -hscrollmode dynamic \
	    -sbwidth 10 -relief groove 

	pack $glyph.param -side top -pady 2 -fill both -expand 1  -anchor w

	# get the childsite to add stuff to
	set paramframe [$glyph.param childsite]

	# Place the parameter menu buttons in the scrolled frame
          
        #############
	# Loop throught the paramter list getting the parameter names, datatypes, and default values
        #############
	set parameter_number 0
	foreach parameter $parameterList paramType $parameterType defaultValue $parameterDefault {
	    incr parameter_number
	    #Convert parameter name to a lower case
	    set parameterLC    [string tolower $parameter]

	    # Set up frame to hold label and menubutton
	    frame $paramframe.$parameterLC
	    label $paramframe.$parameterLC.label -text "$parameter \($paramType\)"   -width 25  -anchor w

	    Menu_Setup $paramframe.$parameterLC menu 
	    Menubutton $paramframe.$parameterLC $defaultValue
	    
	    # Cycle throught the fields to create the menus for the field
	    # dynamically using the helper function by resolving the $field and $filedType
            # variables to access the right helper function.
	    set i 0
	    foreach field {primary secondary tertiary} {
		foreach fieldType {Scalar Vector Tensor} {
		    global $this-${field}_has_${fieldType}_data
		    if { [set "$this-${field}_has_${fieldType}_data" ]} {
			if { [expr $$fieldType] >= [expr $$paramType] } {
			    ${fieldType}_${paramType}_Field_Menu $paramframe.$parameterLC "" $i \
				$defaultValue $this-glyph_parameter$i $i $paramframe.$parameterLC.menu.mb0 $this-glyph_parameter$i
			}
		    }
		}
		incr i
	    }

	    pack $paramframe.$parameterLC -side top -fill x  -anchor w
	    pack $paramframe.$parameterLC.label -in $paramframe.$parameterLC -side left  -anchor w -padx 5 
	    pack $paramframe.$parameterLC.menu -in $paramframe.$parameterLC -side top  -anchor w  
	}

        ########
	#Colormapping and Alpha Frame
        # Have to generate these menu partway manually, due to the different options thay have.
        ########
	frame $paramframe.colormap
	frame $paramframe.alpha 
	label $paramframe.colormap.pram7 -text "Colormaping (Scalar)" -width 25  -anchor w -padx 5 
	label $paramframe.alpha.pram8 -text "Alpha (Scalar)" -width 25  -anchor w -padx 5 

	#Colormap
	set paramType Scalar
	#set $this-colormap_color 0

	Menu_Setup $paramframe.colormap colormapMenu 
	Menubutton $paramframe.colormap  {Colormap Menu}

	set menuname $paramframe.colormap.colormapMenu.mb0

	set cmd [subst { $menuname configure -text "Default Color"; }]
	Menu_Radio $paramframe.colormap "" "Colormap Menu" "Default Color" $this-colormap_color 0 $cmd

	set cmd [subst { $menuname configure -text "Colormap Port 0"; }]
	Menu_Radio $paramframe.colormap "" "Colormap Menu" "Colormap Port 0" $this-colormap_color 1 $cmd

	set cmd [subst { $menuname configure -text "Colormap Port 1"; }]
	Menu_Radio $paramframe.colormap "" "Colormap Menu" "Colormap Port 1" $this-colormap_color 2 $cmd

	set cmd [subst {$menuname configure -text "Colormap Port 2"; }]
	Menu_Radio $paramframe.colormap "" "Colormap Menu" "Colormap Port 2" $this-colormap_color 3 $cmd

	Menu_Cascade $paramframe.colormap "" "Colormap Menu"  "RGB Conversion"

	# Cycle throught the fields to create the menus for the field
	set i 4
	set field_num 0
	foreach field {primary secondary tertiary} {
	    foreach fieldType {Vector Scalar Tensor} {
		global $this-${field}_has_${fieldType}_data
		if { [set "$this-${field}_has_${fieldType}_data" ] } {
		    if { [expr $$fieldType] >= [expr $$paramType] } {
			set i	[${fieldType}_${paramType}_Field_Menu $paramframe.colormap "Colormap Menu" \
				     $field_num "RGB Conversion" $this-colormap_color $i $menuname "$this-glyphs_color_type"]
		    }
		}
	    }
	    incr field_num 
	}

	#Set colormap default menuname and color
	$menuname   configure -text "Default Color"
	if {[set $this-colormap_color] == 0 } {
 	    $menuname   configure -background  $default_backgroundColor
	} else { 
 	    $menuname   configure -background  "Gray"
 	}

	pack $paramframe.colormap -side top -fill x  -anchor w
	pack $paramframe.colormap.pram7 -in $paramframe.colormap -side left -fill x -anchor w
	pack $paramframe.colormap.colormapMenu -in $paramframe.colormap -side left  -anchor w



	#Alpha
	set alpha_value 0
	set paramType Scalar
	set $this-glyph_alpha 0

	Menu_Setup $paramframe.alpha alphaMenu 
	Menubutton $paramframe.alpha  {Off}

	set cmd [subst {$paramframe.alpha.alphaMenu.mb0 configure -text "Off" }]
	Menu_Radio $paramframe.alpha "" "Off" "Off" $this-glyph_alpha 0 $cmd

	set cmd [subst {$paramframe.alpha.alphaMenu.mb0 configure -text "Color Definition" }]
	Menu_Radio $paramframe.alpha "" "Off" "Color Definition" $this-glyph_alpha 1 $cmd

	Menu_Cascade $paramframe.alpha "" "Off" "Data"
	# Cycle throught the fields to create the menus for the field
	set field_num 0
	set i 2
	foreach field {primary secondary tertiary} {
	    foreach fieldType {Vector Scalar Tensor} {
		global $this-${field}_has_${fieldType}_data
		if { [set "$this-${field}_has_${fieldType}_data" ]} {
		    if { [expr $$fieldType] >= [expr $$paramType] } {
			set i [	${fieldType}_${paramType}_Field_Menu $paramframe.alpha "Off" \
				    $field_num "Data" $this-glyph_alpha $i $paramframe.alpha.alphaMenu.mb0 $this-glyph_alpha]
		    }
		}
	    }
	    incr field_num
	}

	pack $paramframe.alpha -side top  -fill x -anchor w
	pack $paramframe.alpha.pram8 -in $paramframe.alpha  -side left -anchor w
	pack $paramframe.alpha.alphaMenu -in $paramframe.alpha  -side left 



	#Glyph Scale Window
	expscale $paramframe.glyph_scale -label "Glyph Scale" \
	    -orient horizontal \
	    -variable $this-glyphs_scale

	bind $paramframe.glyph_scale.scale <ButtonRelease> "$this-c glyphs_scale; set $this-use_dafault_size 0"
	pack $paramframe.glyph_scale -side bottom -pady 2 -fill both -expand 1

	# Tensor Emphasis
	if { [set $this-glyph_value] == "superquadrics" } {
	    expscale $paramframe.emphasis -label "Superquadric Emphasis" \
		-orient horizontal \
		-variable $this-tensors_emphasis

	    bind $paramframe.emphasis.scale <ButtonRelease> "$this-c tensors_emphasis"
	    pack $paramframe.emphasis -side bottom -pady 2 -fill both -expand 1
	}


	#Glyph Resolution Window
	iwidgets::labeledframe $glyph.resolution \
	    -labelpos nw -labeltext "Glyph Resolution"
	pack $glyph.resolution -side top -fill x -expand 0

	set res [$glyph.resolution childsite]
	scale $res.scale -orient horizontal \
	    -variable $this-glyphs_resolution \
	    -from 3 -to 20 -showvalue true -resolution 1
	bind $res.scale <ButtonRelease> "$this-c glyphs_resolution"
	pack $res.scale -side bottom -pady 2 -fill both -expand 1

    }


    method ui {} { 
#	puts "ui"

	global $this-primary_has_data
	global $this-active_tab
	global $this-interactive_mode
	global default_backgroundColor
	global $this-def_color
	global $this-def_color-r
	global $this-def_color-g
	global $this-def_color-b

	set window .ui[modname]
	if {[winfo exists $window]} {
	    return
	}
	toplevel $window
	wm minsize $window 380 548
	
	#frame for all options to live
	frame $window.options
	
	# node frame holds ui related to vert display (left side)
	frame $window.options.disp -borderwidth 2
	pack $window.options.disp -padx 2 -pady 2 -side left \
	    -fill both -expand 1
	
	# Display Options
	iwidgets::labeledframe $window.options.disp.frame_title \
	    -labelpos nw -labeltext "Display Options"
	set dof [$window.options.disp.frame_title childsite]
	
	iwidgets::tabnotebook  $dof.tabs -height 420 -width 365 \
	    -raiseselect true 

	add_text_tab $dof

	#Create glyph tab if primary source has data
	if {[set $this-primary_has_data] == 1 } {
	    add_glyph_tab $dof
	}

	# view the active tab
	if [catch "$dof.tabs view [set $this-active_tab]"] {
	    catch "$dof.tabs view 0"
	}
	
	$dof.tabs configure -tabpos "n"
	
	pack $dof.tabs -side top -fill x -expand yes -padx 2 -pady 2
	
	#pack notebook frame
	pack $window.options.disp.frame_title -side top -expand yes -fill x
	
	#add bottom frame for execute and dismiss buttons
	frame $window.control -relief groove -borderwidth 2 -width 500
	frame $window.def
	frame $window.def.vals
	frame $window.def.col
	frame $window.def.col.f
	frame $window.def.col.le
	
	pack $window.def.col $window.def.vals -side left -padx 10
	label $window.def.col.le.approxl -text "PWL Approx Div:"
	entry $window.def.col.le.approx -textvar $this-approx-div -width 4
	
	bind $window.def.col.le.approx <KeyRelease> "$this-c approx"
	
	set ir [expr int([set $this-def_color-r] * 65535)]
	set ig [expr int([set $this-def_color-g] * 65535)]
	set ib [expr int([set $this-def_color-b] * 65535)]
	set default_backgroundColor [format #%04x%04x%04x $ir $ig $ib]
	
	addColorSelection $window.def.col.f "Default Color" \
	    $this-def_color "default_color_change"

	button $window.def.vals.calcdefs -text "Calculate Default Size" \
	    -command "$this-c calcdefs; set $this-use_default_size 1"
	checkbutton $window.def.vals.use_defaults \
	    -text "Use Default Size" \
	    -variable $this-use_default_size

	pack $window.def.col.f $window.def.col.le -side top -pady 2 -anchor w
	pack $window.def.col.le.approxl $window.def.col.le.approx -side left
	pack $window.def.vals.use_defaults $window.def.vals.calcdefs \
	    -side top -pady 2

	frame $window.fname -borderwidth 2
	label $window.fname.label -text "Field Name"
	entry $window.fname.entry -textvar $this-field_name
	checkbutton $window.fname.override \
	    -text "Override" \
	    -command "$this-c rerender_all" \
	    -variable $this-field_name_override

	TooltipMultiWidget "$window.fname.entry $window.fname.label" \
	    "Enter (optional) Field Name here.  The name will be displayed\nin the Viewer Window's list of Objects."
	
	pack $window.fname.label $window.fname.entry -side left
	pack $window.fname.override -side left -padx 6
	pack $window.fname -anchor w -padx 6 -pady 6

	# execute policy
	make_labeled_radio $window.control.exc_policy \
	    "Execute Policy" "$this-c execute_policy" top 2 \
	    $this-interactive_mode \
	    {{"Interactively update" Interactive} \
		 {"Execute button only" OnExecute}}


	pack $window.options -padx 2 -pady 2 -side top -fill x -expand 1
	pack $window.def $window.control \
	    -padx 2 -pady 2 -side top

	pack $window.control.exc_policy -side top -fill both

	frame $window.control.excdis -borderwidth 2
	pack $window.control.excdis -padx 4 -pady 4 -side top -fill both

	makeSciButtonPanel $window.control.excdis $window $this
	moveToCursor $window

	pack $window.control -padx 4 -pady 4 -side top -fill both
    }
}
