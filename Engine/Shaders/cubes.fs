$input v_color0, v_texcoord0, v_normal

/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

SAMPLER2D(s_texColor,  0);
uniform vec4 u_lightDir;

void main()
{	
	// Think you need to multiply the normal by the modelviewmatrix before applying lighting
	vec3 flippedNormal = -v_normal.xyz;
	float lightMag = dot(normalize(u_lightDir.xyz), flippedNormal) + 0.6;
	gl_FragColor = toLinear(texture2D(s_texColor, v_texcoord0) ) * lightMag;
}
