# Converts primtive info file (*.ifg) into an array initialiser
#
# infogen <input-file>
#
# Read each primitive line
#
%global_properties = ();

# Counted for the summary line at the end; a refused block still goes in the table.
$softprim_skipped = 0;

foreach (@ARGV) {

	/^\s*(.*)(=.*)?\s*/;

	if(/^\s*(.*)=(.*)/) {
		$global_properties{$1} = $2;
	} elsif(/^\s*(.*)/) {
		$global_properties{$1} = "TRUE";
	}
}

while (<STDIN>) {

	# Ignore comments
	#
	next if(/^#/);

	if(/^\s*\[(.*)\]/) {
		$props = $1;
		# Common properties
		#
		# Go through each property
		#
		%common_properties = %global_properties;

		foreach (split(",",$props)) {

			/^\s*(.*)(=.*)?\s*/;

			if(/^\s*(.*)=(.*)/) {
				$common_properties{$1} = $2;
			} elsif(/^\s*(.*)/) {
				$common_properties{$1} = "TRUE";
			}
		}
	} elsif(/^\s*(\w+\:)?\s*(\w+)\s*=\s*\[(.*)\]/) {

#		print "[$1] [$2] [$3] [$4]\n";

		# Remember image and renderer name
		#
		$image = $1;
		$render = $2;
		$props = $3;

		$image =~ s/^\s*(.*)\s*:/\1/;


		# Set default state
		#
		&set_default;

		# Go through each property
		#
		%properties = %common_properties;

		foreach (split(",",$props)) {

			/^\s*(.*)(=.*)?\s*/;

			if(/^\s*(.*)=(.*)/) {
				$properties{$1} = $2;
			} elsif(/^\s*(.*)/) {
				$properties{$1} = "TRUE";
			}
		}
		# This rather nasty way of doing things ensures that the subroutines are called
		# in a defiend order, irrespective of the input order
		#
		&property_mmx($properties{"mmx"}) if $properties{"mmx"};

		&property_point if $properties{"point"};
		&property_line if $properties{"line"};
		&property_triangle if $properties{"triangle"};
		&property_quad if $properties{"quad"};
		&property_sprite if $properties{"sprite"};

		&property_index8 if $properties{"index8"};
		&property_rgb555 if $properties{"rgb555"};
		&property_rgb565 if $properties{"rgb565"};
		&property_rgb888 if $properties{"rgb888"};
		&property_rgbx888 if $properties{"rgbx888"};

		&property_z_buffered if $properties{"z_buffered"};

		&property_interpolated_intensity if $properties{"interpolated_intensity"};
        &property_range_zero if $properties{"range_zero"};

		&property_constant_intensity if $properties{"constant_intensity"};
		&property_interpolated_colour if $properties{"interpolated_colour"};
		&property_constant_colour if $properties{"constant_colour"};
		&property_interpolated_alpha if $properties{"interpolated_alpha"};
		&property_constant_alpha if $properties{"constant_alpha"};
		&property_linear_depth if $properties{"linear_depth"};

		&property_no_depth_write if $properties{"no_depth_write"};

		&property_duplicate if $properties{"duplicate"};

        &property_shade_table if $properties{"shade_table"};

		&property_texture if $properties{"texture"};
		&property_texture8x8 if $properties{"texture8x8"};
		&property_texture16x16 if $properties{"texture16x16"};
		&property_texture32x32 if $properties{"texture32x32"};
		&property_texture64x64 if $properties{"texture64x64"};
		&property_texture128x128 if $properties{"texture128x128"};
		&property_texture256x256 if $properties{"texture256x256"};
		&property_texture512x512 if $properties{"texture512x512"};
		&property_texture1024x1024 if $properties{"texture1024x1024"};

		&property_texture_index8 if $properties{"texture_index8"};
   		&property_texture_index8_palette if $properties{"texture_index8_palette"};
		&property_texture_rgb555 if $properties{"texture_rgb555"};
		&property_texture_rgb565 if $properties{"texture_rgb565"};
		&property_texture_rgbx888 if $properties{"texture_rgbx888"};
   		&property_texture_power2 if $properties{"texture_power2"};
		&property_texture_stride_positive if $properties{"texture_stride_positive"};
		&property_texture_no_skip if $properties{"texture_no_skip"};
		&property_unscaled_texture_coords if $properties{"unscaled_texture_coords"};

		&property_perspective if $properties{"perspective"};
		&property_perspective_subdivide if $properties{"perspective_subdivide"};

		&property_dithered if $properties{"dithered"};

		&property_colour7bit if $properties{"colour7bit"};
	
		&property_dithered_map if $properties{"dithered_map"};
		&property_colour_index if $properties{"colour_index"};
		&property_colour_rgb if $properties{"colour_rgb"};

		&property_rgb_shade if $properties{"rgb_shade"};
		&property_decal if $properties{"decal"};
		&property_bump if $properties{"bump"};
		&property_blend_table if $properties{"blend_table"};

        &property_blendrgb if $properties{"blendrgb"};
        &property_screendoor if $properties{"screendoor"};

        &property_fog if $properties{"fog"};

		&property_float_components if $properties{"float_components"};
		&property_integer_components if $properties{"integer_components"};
		&property_fixed_components if $properties{"fixed_components"};

		&property_float_homogeneous_coords if $properties{"float_homogeneous_coords"};
		&property_float_coords if $properties{"float_coords"};
		&property_float_depth if $properties{"float_depth"};
		&property_float_colour if $properties{"float_colour"};
		&property_float_texture_coords if $properties{"float_texture_coords"};
		&property_float_intensity if $properties{"float_intensity"};
		&property_float_alpha if $properties{"float_alpha"};
		&property_float_linear_depth if $properties{"float_linear_depth"};

		&property_integer_homogeneous_coords if $properties{"integer_homogeneous_coords"};
		&property_integer_coords if $properties{"integer_coords"};
		&property_integer_depth if $properties{"integer_depth"};
		&property_integer_colour if $properties{"integer_colour"};
		&property_integer_texture_coords if $properties{"integer_texture_coords"};
		&property_integer_intensity if $properties{"integer_intensity"};
		&property_integer_alpha if $properties{"integer_alpha"};
		&property_integer_linear_depth if $properties{"integer_linear_depth"};

		&property_fixed_homogeneous_coords if $properties{"fixed_homogeneous_coords"};
		&property_fixed_coords if $properties{"fixed_coords"};
		&property_fixed_depth if $properties{"fixed_depth"};
		&property_fixed_colour if $properties{"fixed_colour"};
		&property_fixed_texture_coords if $properties{"fixed_texture_coords"};
		&property_fixed_intensity if $properties{"fixed_intensity"};
		&property_fixed_alpha if $properties{"fixed_alpha"};
		&property_fixed_linear_depth if $properties{"fixed_linear_depth"};

		&property_generic_setup($properties{"generic_setup"}) if $properties{"generic_setup"};

		&property_pixel_stride($properties{"pixel_stride"}) if $properties{"pixel_stride"};
		&property_parameter_struct($properties{"parameter_struct"}) if $properties{"parameter_struct"};
		&property_area_test($properties{"area_test"}) if $properties{"area_test"};

		# Print out entry
		#
		&print_entry;

		# Print externs ?
		#
		# &print_externs;
	}
}

if($global_properties{"softprim"}) {
	# "emitted" keeps its old meaning - a block with a kernel. A refused block is
	# carried in this list so the matcher can walk it, but it is not implemented,
	# and counting it here would report the whole block set as covered.
	my $emitted = grep { !$_->{refused} } @softprim_blocks;

	print STDERR "softprim: $softprim_skipped blocks outside the implemented axis spec, "
	           . "$emitted blocks emitted\n";
	foreach my $b (@softprim_blocks) {
		my $macro = $b->{refused} ? "SOFTPRIM_REFUSED" : "SOFTPRIM_BLOCK";
		print "$macro($b->{name}, $b->{flags_mask}, $b->{flags_cmp}, $b->{subdivide}, $b->{unscaled}, $b->{offset_y}, ",
		      "$b->{colour7bit}, $b->{cscale}, $b->{mapsize}, $b->{depth_type}, $b->{texture_type}, $b->{shade_type}, ",
		      "$b->{blend_type}, $b->{fog_type}, ", join(", ", @{$b->{tuple}}), ")\n";
	}
	exit 0;
}

sub set_default {
	$identifier = "";
	$type = "BR_PRIMT_UNKNOWN";
    $autoloader = "RenderAutoloadThunk";
	@flags = ();
	@prim_flags = ();
	@constant_components = ();
	@vertex_components = ();
	@convert_mask_f = ();
	@convert_mask_x = ();
	@convert_mask_i = ();
	@use_buffers = ();

	@match_flags_set = ();	
	@match_flags_clear = ();

	@flags_mask = ();	
	@flags_cmp = ();
	@range_flags = ();

	$colour_type = "PMT_NONE";
	$depth_type = "PMT_NONE";
	$texture_type = "PMT_NONE";
	$shade_type = "PMT_NONE";
	$blend_type = "PMT_NONE";
	$bump_type = "PMT_NONE";
	$lighting_type = "PMT_NONE";
	$screendoor_type = "PMT_NONE";
	$fog_type = "PMT_NONE";

	$input_colour_type = "0";

	$colour_row_size = 0;
	$depth_row_size = 0;

	$texture_width = 0;
	$texture_height = 0;
	$decal_flag = "FALSE";

	$colour_base = "BR_SCALAR(0),BR_SCALAR(0),BR_SCALAR(0)";
	$colour_scale = "BR_SCALAR(0),BR_SCALAR(0),BR_SCALAR(0)";

    $alpha_base = "BR_SCALAR(0)";
	$alpha_scale = "BR_SCALAR(0)";

	%integer_component = ();
	%float_component = ();

	$generic_setup = "";
	$pixel_stride = 0;
	$param_size = "0";
	$area_limit = "0.0f";
	$param_struct = "";
}

#
# The softprim axis tuple, projected from the same state the pentprim table is
# built from.  See the axis list in the design note; only the axes that change
# the per-pixel instruction sequence or the live state are represented here,
# everything else (map dimensions, strides, palette, table contents) stays
# runtime and is deliberately absent.
#
#
# Which axis values softprim currently implements, as named on the command line:
#   softprim_implemented=SP_FMT_I8,SP_TOP_TRI,...
# A tuple is emitted only if every one of its values is either named in the spec
# or belongs to an axis the spec does not mention at all.  The axis of a value is
# taken from its prefix, so the spec can name any subset of any axis without
# needing a positional encoding.
#
# The point of the filter is that an unimplemented shape is then absent from the
# table and refused by the matcher, rather than present and silently drawing
# nothing.  Anything that passes the filter must have a kernel, or it will not
# link.
#
sub softprim_implemented {
	my (@tuple) = @_;
	my ($v, $axis);

	&softprim_spec;

	return 1 if(!$global_properties{"softprim_implemented"});

	foreach $v (@tuple) {
		($axis) = $v =~ /^(SP_[A-Z0-9]+)_/;
		return 0 if($softprim_impl_axis{$axis} && !$softprim_impl{$v});
	}

	# The RGB-output shade-table values are in the spec, but naming an axis value
	# admits every shape that carries it, and softprim has a kernel for only some
	# of them. Refuse the rest explicitly rather than let them link against a
	# kernel that is not theirs.
	#
	# The RGB_888 shapes - prim_t24's only occupants of this group - are the whole
	# of the arbitrary-width 888 family, and all of it is implemented: the entry
	# is three bytes, so the reference's reader is in bounds for every index the
	# packing produces, and softprim ports that reader (RgbAwtPixel).
	#
	# The 15/16bpp shapes have kernels for two of their cells. The arbitrary-width
	# z-sorted pair is awtmi.h's LIGHT span, ported as RgbAwtTriangle. The
	# 256x256 power-of-two cells are perspi.h's separate perspective mapper,
	# ported as PizShadeTriangle - the dithered-map cell's perspi.h setup at BPP 2
	# with t15_pip.asm's ScanLinePITIP fragment - and 256 is the only size the
	# tables declare. The z-buffered ones are unreachable (the MMX table is tried
	# first and one of its untextured rows always matches). See softprim's
	# RgbAwtTriangle, PizShadeTriangle and the render dispatch.
	if(grep { $_ eq "SP_SHADE_CONST_I_RGB" || $_ eq "SP_SHADE_INTERP_I_RGB" } @tuple) {
		my %t = map { $_ => 1 } @tuple;

		return 1 if($t{"SP_ADDR_SHIFT"} && $t{"SP_TOP_TRI"} && $t{"SP_DEPTH_NONE"} && ($t{"SP_FMT_555"} || $t{"SP_FMT_565"}));

		return 0 if(!$t{"SP_TOP_TRI"});
		return 0 if(!$t{"SP_ADDR_DIVIDE"});
		return 1 if($t{"SP_FMT_888"});
		return 0 if(!$t{"SP_FMT_555"} && !$t{"SP_FMT_565"});
		return 0 if(!$t{"SP_DEPTH_NONE"});
	}

	# The 555/565-typed colour maps are in the spec for the same reason, and
	# naming the value admits every shape that samples one. Two are implemented:
	# the z-sorted arbitrary-width triangle, which is awtmi.h's span with a
	# two-byte texel decode instead of the RGB_888 one's three (RgbAwtTriangle),
	# and the line and point walk, which copies the map's own word the same way
	# (SoftPrimLine and SoftPrimPoint, with l_pi.c, p_pi.c, l_piz.c and p_piz.c as
	# the arithmetic).
	#
	# The z-buffered triangles are unreachable - the MMX table is tried first for
	# a 555/565 output, its textured rows all require an INDEX_8 map with a
	# palette, and one of its untextured rows always matches, so a 555/565 map is
	# never sampled and the primitive is drawn untextured. The power-of-two ones
	# are the MMX and perfect-scan mappers, which pack their own base texel from a
	# compiled-in width, and the line/point family has no power-of-two form at
	# all. What is left is refused rather than left to fall through to the
	# untextured twin.
	if(grep { $_ eq "SP_TEX_555" || $_ eq "SP_TEX_565" } @tuple) {
		my %t = map { $_ => 1 } @tuple;

		return 0 if(!$t{"SP_ADDR_DIVIDE"});
		return 1 if($t{"SP_TOP_TRI"} && $t{"SP_DEPTH_NONE"});
		return 1 if($t{"SP_TOP_LINE"} || $t{"SP_TOP_POINT"});
		return 0;
	}

	return 1;
}

sub softprim_spec {
	my ($v, $axis);

	return if($softprim_spec_done++);
	return if(!$global_properties{"softprim_implemented"});

	foreach $v (split(/,/, $global_properties{"softprim_implemented"})) {
		($axis) = $v =~ /^(SP_[A-Z0-9]+)_/;
		die "softprim: malformed axis value '$v'\n" unless $axis;
		$softprim_impl{$v}           = 1;
		$softprim_impl_axis{$axis}   = 1;
	}
}

sub softprim_tuple {
	my (@t, %flags, %vc, %cc);

	%flags = map { $_ => 1 } @match_flags_set;
	%vc    = map { $_ => 1 } @vertex_components;
	%cc    = map { $_ => 1 } @constant_components;

	# A1 output format
	my %fmt = ("BR_PMT_INDEX_8" => "SP_FMT_I8",  "BR_PMT_RGB_555" => "SP_FMT_555",
	           "BR_PMT_RGB_565" => "SP_FMT_565", "BR_PMT_RGB_888" => "SP_FMT_888");
	die "softprim: unknown output format '$colour_type'\n" unless $fmt{$colour_type};
	push(@t, $fmt{$colour_type});

	# A2 topology
	my %top = ("TRIANGLE" => "SP_TOP_TRI", "LINE" => "SP_TOP_LINE", "POINT" => "SP_TOP_POINT");
	die "softprim: unknown topology '$type'\n" unless $top{$type};
	push(@t, $top{$type});

	# A3 depth
	push(@t, ($depth_type && $depth_type ne "PMT_NONE") ? "SP_DEPTH_ZW" : "SP_DEPTH_NONE");

	# A4 shading.  The slot set is usually pinned by the output format: INDEX_8
	# carries an intensity, an RGB output carries R,G,B. The exception is the 40
	# RGB-output shade-table blocks - prm_t15/prm_t16/prim_t24's "Interpolated/
	# Constant Intensity, Textured" entries - whose colour slot carries an
	# intensity (awtmi.h's LIGHT path). Deriving only from $vc{"R"}/$cc{"R"}
	# dropped it and collapsed the gouraud and flat entries onto the unlit tuple,
	# which is drawn by the palette-decode kernel that reads no intensity. They
	# now get their own axis values so the tuple carries the declared CM_I; no
	# kernel implements them and SP_SPEC does not name the values, so the shapes
	# are refused rather than drawn by the unlit sibling.
	#
	# The untextured shade-table blocks (property_shade_table) look their fragment
	# up in the bound shade table rather than writing it, which is a different
	# kernel; they carry their own axis values so they cannot collapse onto the
	# plain untextured intensity blocks they otherwise share a tuple with.
	if($colour_type eq "BR_PMT_INDEX_8") {
		my $st = $properties{"shade_table"} ? 1 : 0;
		push(@t, $st && $vc{"I"} ? "SP_SHADE_INTERP_I_TABLE"
		       : $st && $cc{"I"} ? "SP_SHADE_CONST_I_TABLE"
		       : $vc{"I"} ? "SP_SHADE_INTERP_I"
		       : $cc{"I"} ? "SP_SHADE_CONST_I"
		                    : "SP_SHADE_NONE");
	} else {
		push(@t, $vc{"R"} ? "SP_SHADE_INTERP_RGB"
		       : $cc{"R"} ? "SP_SHADE_CONST_RGB"
		       : $vc{"I"} ? "SP_SHADE_INTERP_I_RGB"
		       : $cc{"I"} ? "SP_SHADE_CONST_I_RGB"
		                    : "SP_SHADE_NONE");
	}

	# A5 texture decode
	# The generic `texture` property sets the texture type from the OUTPUT colour
	# type (infogen.pl:623), so an unindexed 24-bit output samples an RGB_888 map.
	my %tex = ("BR_PMT_INDEX_8" => "SP_TEX_I8", "BR_PMT_RGB_555" => "SP_TEX_555",
	           "BR_PMT_RGB_565" => "SP_TEX_565", "BR_PMT_RGBX_888" => "SP_TEX_RGBX888",
	           "BR_PMT_RGB_888" => "SP_TEX_RGB888");
	my $texture = "SP_TEX_NONE";
	if($texture_type && $texture_type ne "PMT_NONE") {
		die "softprim: unknown texture type '$texture_type'\n" unless $tex{$texture_type};
		$texture = $tex{$texture_type};
	}
	push(@t, $texture);

	# A6 texture address mode - only meaningful with a texture bound.
	#
	# The dimension-specific properties (texture8x8 .. texture1024x1024) set an
	# explicit width and height, which is the power-of-two shift/mask family; the
	# bare `texture` property leaves both zero and is the arbitrary-width divide
	# family.  POWER2 is the runtime flag pentprim's matcher derives from the
	# bound map, and a block that requires it (property_texture_power2) is also a
	# shift/mask block, so it triggers the same value.
	push(@t, ($texture eq "SP_TEX_NONE") ? "SP_ADDR_NONE"
	                                      : (($texture_width != 0 || $flags{"POWER2"}) ? "SP_ADDR_SHIFT" : "SP_ADDR_DIVIDE"));

	# A7 perspective.  Note that pentprim also falls back to the affine kernel at
	# runtime for triangles with a narrow w range (pfpsetup.asm SETUP_FLOAT_CHECK_
	# PERSPECTIVE_CHEAT), so a CORRECT entry still needs the affine path reachable.
	push(@t, $flags{"PERSPECTIVE"} ? "SP_PERSP_CORRECT" : "SP_PERSP_AFFINE");

	# A8 blend
	#
	# Two of the variants have no state variable to read. property_screendoor and
	# property_blendrgb only raise the BLEND match flag, and both $screendoor_type
	# and $blend_type are emitted into pentprim's own table (see the block below),
	# so they cannot be repurposed here. Read the properties instead: they are the
	# .ifg's own name for the variant, and the axis value names the kernel, so a
	# variant that only differs from its twin by a match flag has to be projected
	# from the flag's source rather than from the flag.
	push(@t, ($properties{"screendoor"} || ($screendoor_type && $screendoor_type ne "PMT_NONE"))
	                                                     ? "SP_BLEND_SCREENDOOR"
	       : $properties{"blendrgb"}                     ? "SP_BLEND_ALPHA"
	       : ($blend_type eq "BR_PMT_INDEX_8")           ? "SP_BLEND_INDEX"
	       : ($blend_type && $blend_type ne "PMT_NONE")  ? "SP_BLEND_ALPHA"
	       : $flags{"DECAL"}                             ? "SP_BLEND_DECAL"
	                                                     : "SP_BLEND_NONE");

	# A9 fog
	push(@t, ($fog_type && $fog_type ne "PMT_NONE") ? "SP_FOG_INDEX" : "SP_FOG_NONE");

	# A10 dither
	push(@t, $flags{"DITHER_COLOUR"} ? "SP_DITH_COLOUR"
	       : $flags{"DITHER_MAP"}    ? "SP_DITH_MAP"
	                                 : "SP_DITH_NONE");

	return @t;
}

sub print_entry {

	# softprim mode: emit the axis tuple plus the matcher metadata instead of a
	# pentprim table element.  Everything comes from the same variables that
	# produce the normal table below, so a block cannot describe something the
	# table does not say.
	#
	# The metadata is what pentprim's match_block() tests besides the flags:
	# the per-map type requirements and the required map dimensions.  softprim
	# needs them because its matcher walks the table in order and takes the
	# first block whose requirements the bound state meets - a block that fails
	# its requirements falls through to the next, which is how pentprim routes
	# a textured primitive with a non-power-of-two map to the untextured
	# kernel.
	#
	if($global_properties{"softprim"}) {
		my @tuple = &softprim_tuple;
		# NB: a bare `map { s///; $_ }` aliases $_, so it would strip the prefix
		# from @tuple itself and the values would reach the predicate and the
		# generated line without it.
		my $name  = "SoftPrim_" . join("_", map { (my $axis_name = $_) =~ s/^SP_//; $axis_name } @tuple);

		# The flag predicate, exactly as pentprim builds it for the normal
		# table below: every flag the block requires or forbids in the mask,
		# the required ones in the comparison.
		my $flags_mask = (@match_flags_set || @match_flags_clear)
		               ? "PRIMF_" . join("|PRIMF_", (@match_flags_set, @match_flags_clear)) : "0";
		my $flags_cmp  = (@match_flags_set)
		               ? "PRIMF_" . join("|PRIMF_", @match_flags_set) : "0";

		# RF_NEED_SUBDIVIDE changes rasterisation: pentprim forces
		# BR_PRIMF_SUBDIVIDE on a block that carries it, so softrend subdivides a
		# perspective_subdivide primitive (pentprim's match.c). Carry it as its
		# own field so the matcher does not have to guess it back from the axes.
		my $subdivide = (grep { $_ eq "NEED_SUBDIVIDE" } @range_flags) ? 1 : 0;

		# RF_UNSCALED_TEXTURE_COORDS is the other range flag the matcher needs:
		# without it, U and V are texel coordinates scaled by the bound map's
		# width and height (pentprim's updateRanges). The address axis does not
		# imply it - the INDEX_8 arbitrary-width blocks are unscaled but the RGB
		# arbitrary-width ones are not - so carry it explicitly rather than let
		# the matcher guess it from the address mode.
		my $unscaled = (grep { $_ eq "UNSCALED_TEXTURE_COORDS" } @range_flags) ? 1 : 0;

		# RF_OFFSET_Y is the third range flag the matcher needs: pentprim's
		# updateRanges offsets SX by half a pixel the other way for a block that
		# carries it (match.c:276-283), which is a whole pixel of shift in the
		# screen x every vertex is projected to. The MMX blocks are the ones that
		# set it.
		my $offset_y = (grep { $_ eq "OFFSET_Y" } @range_flags) ? 1 : 0;

		# colour7bit halves the interpolated R,G,B scale: the property_*_colour
		# handlers below set 126 instead of 254. The axis tuple cannot express
		# it (both are SP_SHADE_INTERP_RGB on the same format and texture), so
		# carry it as its own field for the matcher.
		my $colour7bit = $properties{"colour7bit"} ? 1 : 0;

		# The R,G,B scale the block projects the vertex colour with. It is a
		# per-block runtime value and not a function of the output format: the
		# dithered and dithered-screendoor rows of the MMX tables use
		# 247/251/247 (or 123/125/123 when the block is colour7bit) rather than
		# 254 or 126, so a kernel that assumes one of the two unnamed scales
		# reads a colour that is out by up to 3%. Carry the three numbers so the
		# matcher can hand the kernel the block's own scale.
		my ($cr, $cg, $cb) = ($colour_scale =~ /BR_SCALAR\((\d+)\),BR_SCALAR\((\d+)\),BR_SCALAR\((\d+)\)/) ? ($1, $2, $3) : (0, 0, 0);

		# The map dimensions are a per-block parameter the axis tuple drops, and
		# pentprim's matcher uses them as a positive requirement: a textureNxN
		# block only matches a map of exactly that size, so a power-of-two map
		# with no block of its size falls through to the arbitrary-width block.
		# Zero when the block carries no dimension requirement.
		my $mapsize = $texture_width;

		# A type of PMT_NONE is "no requirement". softprim spells that SP_PMT_NONE
		# rather than pentprim's 255-as-a-type, so a required type never collides
		# with BR_PMT_INDEX_1, which is zero.
		my $dt = ($depth_type   eq "PMT_NONE") ? "SP_PMT_NONE" : $depth_type;
		my $tt = ($texture_type eq "PMT_NONE") ? "SP_PMT_NONE" : $texture_type;
		my $st = ($shade_type   eq "PMT_NONE") ? "SP_PMT_NONE" : $shade_type;
		my $bt = ($blend_type   eq "PMT_NONE") ? "SP_PMT_NONE" : $blend_type;
		my $ft = ($fog_type     eq "PMT_NONE") ? "SP_PMT_NONE" : $fog_type;

		# A shape outside the implemented axis spec has no kernel, but it still
		# belongs in the walk. pentprim spells a variant as a match flag on a
		# block ordered before its twin (the screendoor, dithered, decal and
		# blend-table families all do), so a variant dropped from the table does
		# not refuse anything: the state it was there to take falls through to
		# the twin and is drawn as though the variant had been applied. Emitting
		# it as SOFTPRIM_REFUSED keeps it in the walk, where the matcher stops on
		# it and refuses the primitive.
		my $refused = &softprim_implemented(@tuple) ? 0 : 1;
		$softprim_skipped++ if $refused;

		push(@softprim_blocks, {name => $name, flags_mask => $flags_mask, flags_cmp => $flags_cmp,
		                      subdivide => $subdivide, unscaled => $unscaled, offset_y => $offset_y, colour7bit => $colour7bit,
		                      cscale => "BR_SCALAR($cr),BR_SCALAR($cg),BR_SCALAR($cb)",
		                      mapsize => $mapsize, depth_type => $dt, texture_type => $tt, shade_type => $st,
		                      blend_type => $bt, fog_type => $ft, refused => $refused, tuple => [@tuple]});

		return;
	}

	# Strip trailing comma from name
	# 		
	$identifier =~ s/, $//;

	# Split components by type
	#
	foreach $component (@constant_components, @vertex_components) {
		if ($integer_component{$component}) {
			push(@convert_mask_i,($component));
		} elsif ($float_component{$component}) {
			push(@convert_mask_f,($component));
		} else {
			push(@convert_mask_x,($component));
		}
    }

	@flags_mask = (@match_flags_set, @match_flags_clear);
	@flags_cmp = (@match_flags_set);

	# Convert flag arrays to strings
	#
	$prim_flags_string = "0";
	$prim_flags_string = "BR_PRIMF_" . join("|BR_PRIMF_",@prim_flags) if(@prim_flags);

	$constant_components_string = "0";
	$constant_components_string = "CM_" . join("|CM_",@constant_components) if(@constant_components);

	$vertex_components_string = "0";
	$vertex_components_string = "CM_" . join("|CM_",@vertex_components) if(@vertex_components);

	$convert_mask_f_string = "0";
	$convert_mask_f_string = "(1<<C_" . join(")|(1<<C_",@convert_mask_f) . ")" if(@convert_mask_f);
	$convert_mask_x_string = "0";
	$convert_mask_x_string = "(1<<C_" . join(")|(1<<C_",@convert_mask_x) . ")" if(@convert_mask_x);
	$convert_mask_i_string = "0";
	$convert_mask_i_string = "(1<<C_" . join(")|(1<<C_",@convert_mask_i) . ")" if(@convert_mask_i);

	$constant_slots_string = "0";
	$constant_slots_string = "(1<<C_" . join(")|(1<<C_",@constant_components) . ")" if(@constant_components);

	$use_buffers_string = "0";
	$use_buffers_string = "BUFFER_" . join("_MASK|BUFFER_",@use_buffers) ."_MASK" if(@use_buffers);

	$flags_mask_string = "0";
	$flags_mask_string = "PRIMF_" . join("|PRIMF_",@flags_mask) if(@flags_mask);
	
	$flags_cmp_string = "0";
	$flags_cmp_string = "PRIMF_" . join("|PRIMF_",@flags_cmp) if(@flags_cmp);

	$range_flags_string = "0";
	$range_flags_string = "RF_" . join("|RF_",@range_flags) if(@range_flags);


	if($image) {
		$image = "\"$properties{image_prefix}$image$properties{image_suffix}\"";
                if($generic_setup){
                        $image_entry = "$render";
                }else{
                        $image_entry = "\"_$render\"";
                }
		$entry = $autoloader;
	} else {
		$image = "NULL";
		$image_entry = "NULL";
		$entry = $render;
	}

	print <<END;
{
	.p = {
		/* Render function
		 */
#if AUTOLOAD
		.render = (brp_render_fn *)$entry,
		.chain  = NULL,
#else
		.render = (brp_render_fn *)$render,
		.chain  = NULL,
#endif

		.identifier = "$identifier",
		._reserved0 = NULL,

		.type  = BRT_$type,
		.flags = $prim_flags_string,

		/* components - constant and per vertex
		 */
		.constant_components = $constant_components_string,
		.vertex_components   = $vertex_components_string,

		/* Component slots as - float, fixed or integer
		 */
		.convert_mask_f = $convert_mask_f_string,
		.convert_mask_x = $convert_mask_x_string,
		.convert_mask_i = $convert_mask_i_string,

		/* Constant slots
	 	 */
		.constant_mask = $constant_slots_string,
	},

	/* Offset and scale for R,G,B,A
	 */
	.colour_offsets = {$colour_base,$alpha_base},
	.colour_scales  = {$colour_scale,$alpha_scale},

	/* range flags
	 */
	.range_flags = $range_flags_string,

	/* Work buffer
	 */
	.work = &work,

	/* Masks
	 */
	.flags_mask = $flags_mask_string,
	.flags_cmp  = $flags_cmp_string,

	/* Texture, depth and shade type
	 */
	.depth_type      = $depth_type,
	.texture_type    = $texture_type,
	.shade_type      = $shade_type,
	.blend_type      = $blend_type,
	.screendoor_type = $screendoor_type,
	.lighting_type   = $lighting_type,
	.bump_type       = $bump_type,
	.fog_type        = $fog_type,

	/* Colour & Depth  row size
	 */
	.colour_row_size = $colour_row_size,
	.depth_row_size  = $depth_row_size,

	/* Texture size
	 */
	.map_width  = $texture_width,
	.map_height = $texture_height,

	/* Input colour type
	 */
	.input_colour_type = $input_colour_type,

	/* Autoload info
	 */
#if AUTOLOAD
	.image_name = $image,
	.entry_info = (void *)$image_entry,
#else
	.image_name = NULL,
	.entry_info = NULL,
#endif
END
	print <<END if($generic_setup);
	/* Generic setup info.
	 */
	.setup = {
		.iarea_limit = $area_limit,
		.param_size = $param_size,
		.stride = $pixel_stride,
		.setup_param = $generic_setup,

#if AUTOLOAD
		.rasterise_rl_l = (void *)\"_$rasterise_rl_l\",
		.rasterise_lr_l = (void *)\"_$rasterise_lr_l\",
		.rasterise_rl_s = (void *)\"_$rasterise_rl_s\",
		.rasterise_lr_s = (void *)\"_$rasterise_lr_s\",
#else
		.rasterise_rl_l = $rasterise_rl_l,
		.rasterise_lr_l = $rasterise_lr_l,
		.rasterise_rl_s = $rasterise_rl_s,
		.rasterise_lr_s = $rasterise_lr_s,
#endif
	}
END

	print "},\n";

}

sub print_externs
{
	# Generate some externs
	#
	if($generic_setup) {
		print "void BR_ASM_CALL $generic_setup(void);\n";
		print "void BR_ASM_CALL $rasterise_lr_l(void);\n";
		print "void BR_ASM_CALL $rasterise_rl_l(void);\n";
		print "void BR_ASM_CALL $rasterise_lr_s(void);\n" if($rasterise_lr_l ne $rasterise_lr_s);
		print "void BR_ASM_CALL $rasterise_rl_s(void);\n" if($rasterise_rl_l ne $rasterise_rl_s);
	} else {
		print "void BR_ASM_CALL $render(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2);\n";
	}
}

# Routines for each property
#

sub property_range_zero
{
        push(@match_flags_set,("RANGE_ZERO"));
}

sub property_index8
{
	$identifier .=	"Indexed, ";
	$colour_type = "BR_PMT_INDEX_8";
}

sub property_rgb555
{
	$identifier .=	"RGB 555, ";
	$colour_type = "BR_PMT_RGB_555";
}

sub property_rgb565
{
	$identifier .=	"RGB 565, ";
	$colour_type = "BR_PMT_RGB_565";
}

sub property_rgb888
{
	$identifier .=	"RGB 888, ";
	$colour_type = "BR_PMT_RGB_888";
}

sub property_rgbx888
{
	$identifier .=	"RGBX 888, ";
	$colour_type = "BR_PMT_RGBX_888";
}

sub property_z_buffered
{
	$identifier .=	"Z Buffered, ";
	$depth_type = "BR_PMT_DEPTH_16";
	push(@vertex_components,("SZ"));
	push(@use_buffers,("DEPTH"));
}

sub property_shade_table
{
	if($properties{"constant_intensity"} || $properties{"interpolated_intensity"}) {
		$identifier .= "Indexed Shading, ";

		push(@use_buffers,("SHADE"));

 		$shade_type = $colour_type;
	}
}


# Texture mapped
#
sub shared_texture
{
	push(@vertex_components,("U","V"));

	push(@use_buffers,("TEXTURE"));

	if($properties{"constant_intensity"} || $properties{"interpolated_intensity"}) {
		push(@use_buffers,("SHADE"));

 		$shade_type = $colour_type;
	}

	if($properties{"constant_intensity"} || $properties{"interpolated_intensity"} ||
	   $properties{"constant_colour"} || $properties{"interpolated_colour"}) {
		push(@match_flags_set,("MODULATE"));
	}

	$texture_type = $colour_type;
}

sub property_texture
{
	$identifier .=	"Textured, ";
	$texture_width = 0;
	$texture_height = 0;
	&shared_texture;
}

sub property_texture8x8
{
	$identifier .=  "Textured 8x8, ";
	$texture_width = 8;
	$texture_height = 8;
	&shared_texture;
}

sub property_texture16x16
{
	$identifier .=  "Textured 16x16, ";
	$texture_width = 16;
	$texture_height = 16;
	&shared_texture;
}

sub property_texture32x32
{
	$identifier .=  "Textured 32x32, ";
	$texture_width = 32;
	$texture_height = 32;
	&shared_texture;
}

sub property_texture64x64
{
	$identifier .=	"Textured 64x64, ";
	$texture_width = 64;
	$texture_height = 64;
	&shared_texture;
}

sub property_texture128x128
{
	$identifier .=	"Textured 128x128, ";
	$texture_width = 128;
	$texture_height = 128;
	&shared_texture;
}

sub property_texture256x256
{
	$identifier .=	"Textured 256x256, ";
	$texture_width = 256;
	$texture_height = 256;
	&shared_texture;
}

sub property_texture512x512
{
	$identifier .=	"Textured 512x512, ";
	$texture_width = 512;
	$texture_height = 512;
	&shared_texture;
}

sub property_texture1024x1024
{
	$identifier .=	"Textured 1024x1024, ";
	$texture_width = 1024;
	$texture_height = 1024;
	&shared_texture;
}

# Extra properties for texture mapping
#
sub property_texture_index8
{
	$texture_type = "BR_PMT_INDEX_8";
}

sub property_texture_index8_palette
{
	$texture_type = "BR_PMT_INDEX_8";
	push(@match_flags_set,("PALETTE"));
}

sub property_texture_rgb555
{
	$texture_type = "BR_PMT_RGB_555";
}

sub property_texture_rgb565
{
	$texture_type = "BR_PMT_RGB_565";
}

sub property_texture_rgbx888
{
	$texture_type = "BR_PMT_RGBX_888";
}

sub property_unscaled_texture_coords
{
	$identifier .=  "Unscaled_texture_coords, ";
	push(@range_flags,("UNSCALED_TEXTURE_COORDS"));
}

sub property_texture_power2
{
	push(@match_flags_set,("POWER2"));
}

sub property_texture_stride_positive
{
        push(@match_flags_set,("STRIDE_POSITIVE"));
}

sub property_texture_no_skip
{
        push(@match_flags_set,("NO_SKIP"));
}

sub property_decal
{
	$identifier .=	"Decal, ";
	push(@match_flags_set,("DECAL"));
	push(@range_flags,("DECAL"));
}

sub property_blend
{
	$identifier .=	"Blended, ";

	if($properties{"constant_alpha"} || $properties{"interpolated_alpha"}) {
		$blend_type = $colour_type;
	}
    push(@prim_flags,("BLENDED"));
}

sub property_blendrgb
{
    $identifier .=  "RGB blended, ";
    push(@match_flags_set,("BLEND"));
    push(@prim_flags,("BLENDED"));
}

sub property_screendoor
{
    $identifier .=  "Screendoor, ";
    push(@match_flags_set,("BLEND"));
    push(@prim_flags,("BLENDED"));
}

sub property_perspective
{
	$identifier .=	"Perspective Correct, ";
	push(@vertex_components,("W"));
	push(@match_flags_set,("PERSPECTIVE"));
}

sub property_perspective_subdivide
{
	$identifier .=	"Perspective Correct, ";
	push(@match_flags_set,("PERSPECTIVE"));
	push(@prim_flags,("SUBDIVIDE"));
	push(@range_flags,("NEED_SUBDIVIDE"));
}

sub property_dithered_map
{
	$identifier .=	"Dithered Map, ";
	push(@match_flags_set,("DITHER_MAP"));
}

sub property_dithered
{
	$identifier .=	"Dithered, ";
	push(@match_flags_set,("DITHER_COLOUR"));
}

sub property_colour7bit
{
}

sub property_interpolated_intensity
{
	$identifier .=	"Interpolated Intensity, ";
	push(@vertex_components,("I"));
	push(@match_flags_set,("SMOOTH"));
}

sub property_constant_intensity
{
	$identifier .=	"Constant Intensity, ";
	push(@constant_components,("I"));
	push(@match_flags_clear,("SMOOTH"));
}

sub property_interpolated_colour
{
	$identifier .=	"Interpolated Colour, ";
	push(@vertex_components,("R","G","B"));
	push(@match_flags_set,("SMOOTH"));
	$colour_base = "BR_SCALAR(1),BR_SCALAR(1),BR_SCALAR(1)";

	$colour_scale = "BR_SCALAR(254),BR_SCALAR(254),BR_SCALAR(254)" if(!$properties{"dithered"} && !$properties{"colour7bit"});
	$colour_scale = "BR_SCALAR(126),BR_SCALAR(126),BR_SCALAR(126)" if(!$properties{"dithered"} &&  $properties{"colour7bit"});

	$colour_scale = "BR_SCALAR(247),BR_SCALAR(251),BR_SCALAR(247)" if( $properties{"dithered"} && !$properties{"colour7bit"});
	$colour_scale = "BR_SCALAR(123),BR_SCALAR(125),BR_SCALAR(123)" if( $properties{"dithered"} &&  $properties{"colour7bit"});
}

sub property_constant_colour
{
	$identifier .=	"Constant Colour, ";
	push(@constant_components,("R","G","B"));
	push(@match_flags_clear,("SMOOTH"));
	$colour_base = "BR_SCALAR(1),BR_SCALAR(1),BR_SCALAR(1)";

	$colour_scale = "BR_SCALAR(254),BR_SCALAR(254),BR_SCALAR(254)" if(!$properties{"dithered"} && !$properties{"colour7bit"});
	$colour_scale = "BR_SCALAR(126),BR_SCALAR(126),BR_SCALAR(126)" if(!$properties{"dithered"} &&  $properties{"colour7bit"});

	$colour_scale = "BR_SCALAR(247),BR_SCALAR(251),BR_SCALAR(247)" if( $properties{"dithered"} && !$properties{"colour7bit"});
	$colour_scale = "BR_SCALAR(123),BR_SCALAR(125),BR_SCALAR(123)" if( $properties{"dithered"} &&  $properties{"colour7bit"});
}

sub property_interpolated_alpha
{
	push(@vertex_components,("A"));

	$alpha_base = "BR_SCALAR(0)";
	$alpha_scale = "BR_SCALAR(1)";
}

sub property_constant_alpha
{
	push(@constant_components,("A"));

	$alpha_base = "BR_SCALAR(0)";
	$alpha_scale = "BR_SCALAR(1)";
}

sub property_linear_depth
{
	push(@vertex_components,("SW"));
}

sub property_no_depth_write
{
#        push(@match_flags_clear,("DEPTH_WRITE"));
}

sub property_duplicate
{
	push(@prim_flags,("CONST_DUPLICATE"));
}

sub property_colour_index
{
	$input_colour_type = "BRT_INDEX";
}

sub property_colour_rgb
{
	$input_colour_type = "BRT_RGB";
}

sub property_rgb_shade
{
	$identifier .=	"RGB Shading, ";
	$shade_type = "BR_PMT_RGBX_888";
	push(@range_flags,("RGB_SHADE"));
}

sub property_bump
{
	$identifier .=	"Bump, ";
 	$bump_type = "BR_PMT_INDEX_8";
 	$lighting_type = "BR_PMT_INDEX_8";
}

sub property_blend_table
{
	$identifier .=	"Blend, ";
 	$blend_type = "BR_PMT_INDEX_8";
    push(@prim_flags,("BLENDED"));
}

sub property_fog
{
	$identifier .=  "Fog, ";
	$fog_type = "BR_PMT_INDEX_8";
}

sub property_point
{
	$type = "POINT";
	push(@vertex_components,("SX","SY"));
	push(@use_buffers,("COLOUR"));
}

sub property_line
{
	$type = "LINE";
	push(@vertex_components,("SX","SY"));
	push(@use_buffers,("COLOUR"));
}

sub property_triangle
{
	$type = "TRIANGLE";
	push(@vertex_components,("SX","SY"));
	push(@use_buffers,("COLOUR"));
}

sub property_quad
{
	$type = "QUAD";
	push(@vertex_components,("SX","SY"));
	push(@use_buffers,("COLOUR"));
}

sub property_sprite
{
	$type = "SPRITE";
	push(@vertex_components,("SX","SY"));
	push(@use_buffers,("COLOUR"));
}

sub property_fixed_components
{
	&property_fixed_homogeneous_coords;
	&property_fixed_coords;
	&property_fixed_depth;
	&property_fixed_colour;
	&property_fixed_texture_coords;
	&property_fixed_intensity;
	&property_fixed_alpha;
	&property_fixed_linear_depth;
}

sub property_integer_components
{
	&property_integer_homogeneous_coords;
	&property_integer_coords;
	&property_integer_depth;
	&property_integer_colour;
	&property_integer_texture_coords;
	&property_integer_intensity;
	&property_integer_alpha;
	&property_integer_linear_depth;
}

sub property_float_components
{
	&property_float_homogeneous_coords;
	&property_float_coords;
	&property_float_depth;
	&property_float_colour;
	&property_float_texture_coords;
	&property_float_intensity;
	&property_float_alpha;
	&property_float_linear_depth;
}

sub property_fixed_homogeneous_coords
{
	$integer_component{"X"} = 0;
	$integer_component{"Y"} = 0;
	$integer_component{"Z"} = 0;
	$integer_component{"W"} = 0;

	$float_component{"X"} = 0;
	$float_component{"Y"} = 0;
	$float_component{"Z"} = 0;
	$float_component{"W"} = 0;
}

sub property_fixed_coords
{
	$integer_component{"SX"} = 0;
	$integer_component{"SY"} = 0;

	$float_component{"SX"} = 0;
	$float_component{"SY"} = 0;
}

sub property_fixed_depth
{
	$integer_component{"SZ"} = 0;

	$float_component{"SZ"} = 0;
}

sub property_fixed_colour
{
	$integer_component{"R"} = 0;
	$integer_component{"G"} = 0;
	$integer_component{"B"} = 0;

	$float_component{"R"} = 0;
	$float_component{"G"} = 0;
	$float_component{"B"} = 0;
}

sub property_fixed_texture_coords
{
	$integer_component{"U"} = 0;
	$integer_component{"V"} = 0;

	$float_component{"U"} = 0;
	$float_component{"V"} = 0;
}

sub property_fixed_intensity
{
	$integer_component{"I"} = 0;

	$float_component{"I"} = 0;
}

sub property_fixed_alpha
{
	$integer_component{"A"} = 0;

	$float_component{"A"} = 0;
}

sub property_fixed_linear_depth
{
	$integer_component{"SW"} = 0;

	$float_component{"SW"} = 0;
}

sub property_integer_homogeneous_coords
{
	$integer_component{"X"} = 1;
	$integer_component{"Y"} = 1;
	$integer_component{"Z"} = 1;
	$integer_component{"W"} = 1;

	$float_component{"X"} = 0;
	$float_component{"Y"} = 0;
	$float_component{"Z"} = 0;
	$float_component{"W"} = 0;
}

sub property_integer_coords
{
	$integer_component{"SX"} = 1;
	$integer_component{"SY"} = 1;

	$float_component{"SX"} = 0;
	$float_component{"SY"} = 0;
}

sub property_integer_depth
{
	$integer_component{"SZ"} = 1;

	$float_component{"SZ"} = 0;
}

sub property_integer_colour
{
	$integer_component{"R"} = 1;
	$integer_component{"G"} = 1;
	$integer_component{"B"} = 1;

	$float_component{"R"} = 0;
	$float_component{"G"} = 0;
	$float_component{"B"} = 0;
}

sub property_integer_texture_coords
{
	$integer_component{"U"} = 1;
	$integer_component{"V"} = 1;

	$float_component{"U"} = 0;
	$float_component{"V"} = 0;
}

sub property_integer_intensity
{
	$integer_component{"I"} = 1;

	$float_component{"I"} = 0;
}

sub property_integer_alpha
{
	$integer_component{"A"} = 1;

	$float_component{"A"} = 0;
}

sub property_integer_linear_depth
{
	$integer_component{"SW"} = 1;

	$float_component{"SW"} = 0;
}

sub property_float_homogeneous_coords
{
	$integer_component{"X"} = 0;
	$integer_component{"Y"} = 0;
	$integer_component{"Z"} = 0;
	$integer_component{"W"} = 0;

	$float_component{"X"} = 1;
	$float_component{"Y"} = 1;
	$float_component{"Z"} = 1;
	$float_component{"W"} = 1;
}

sub property_float_coords
{
	$integer_component{"SX"} = 0;
	$integer_component{"SY"} = 0;

	$float_component{"SX"} = 1;
	$float_component{"SY"} = 1;
}

sub property_float_depth
{
	$integer_component{"SZ"} = 0;

	$float_component{"SZ"} = 1;
}

sub property_float_colour
{
	$integer_component{"R"} = 0;
	$integer_component{"G"} = 0;
	$integer_component{"B"} = 0;

	$float_component{"R"} = 1;
	$float_component{"G"} = 1;
	$float_component{"B"} = 1;
}

sub property_float_texture_coords
{
	$integer_component{"U"} = 0;
	$integer_component{"V"} = 0;

	$float_component{"U"} = 1;
	$float_component{"V"} = 1;
}

sub property_float_intensity
{
	$integer_component{"I"} = 0;

	$float_component{"I"} = 1;
}

sub property_float_alpha
{
	$integer_component{"A"} = 0;

	$float_component{"A"} = 1;
}

sub property_float_linear_depth
{
	$integer_component{"SW"} = 0;

	$float_component{"SW"} = 1;
}

sub property_pixel_stride
{

	($pixel_stride) = @_;
}

sub property_parameter_struct
{
	($s) = @_;

	($param_size) = "sizeof(struct $s)";
}

sub property_generic_setup
{
	($r) = @_;

	$rasterise_lr_l = $render . "_LR";
	$rasterise_rl_l = $render . "_RL";

	$rasterise_lr_s = $render . "_LR";
	$rasterise_rl_s = $render . "_RL";

        if($float_component{"X"}) {
		($generic_setup) = "GenericSetupFloat" . $r;
		$render = "GenericSetupTriangleFloat_A";
        } elsif($integer_component{"X"}) {
		($generic_setup) = "GenericSetup" . $r;
		$render = "GenericSetupTriangle_A";
	} else {
		($generic_setup) = "GenericSetupFixed" . $r;
		$render = "GenericSetupTriangleFixed_A";
	}

    $autoloader = "GenericAutoloadThunk";
}

sub property_area_test
{
	($generic_area_limit) = @_;

	$rasterise_lr_s .= "S";
	$rasterise_rl_s .= "S";
}

sub property_mmx
{
	$identifier .=  "MMX, ";
	push(@range_flags,("OFFSET_Y"));
}

