// Copyright 2020-2021 David Colson. All rights reserved.

$input v_color0, v_texcoord0, v_normal

#include "common.sh"

uniform vec4 u_lightDir;
uniform vec4 u_baseColor;

void main()
{	
	// Think you need to multiply the normal by the modelviewmatrix before applying lighting
	vec3 flippedNormal = -v_normal.xyz;
	float lightMag = dot(normalize(u_lightDir.xyz), flippedNormal) + 0.6;
	gl_FragColor = u_baseColor * lightMag;
}
