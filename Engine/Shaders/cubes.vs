// Copyright 2020-2021 David Colson. All rights reserved.

$input a_position, a_texcoord0, a_normal, a_color0
$output v_texcoord0, v_color0, v_normal

/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_texcoord0 = a_texcoord0;
	v_normal = a_normal;
}
