/****************************************************************************
* MeshLab                                                           o o     *
* An extendible mesh processor                                    o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005, 2009                                          \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

// vertex to fragment shader io
varying vec3 n;  //normal
varying vec3 vpos;

// globals
uniform int istransparent;

// entry point
void main()
{
		vec4 diffuse  = vec4(0.0);		
		// the material properties are embedded in the shader (for now)
		vec4 ambientPhong = vec4(0.2, 0.2, 0.2, 1.0);
		vec4 ambientTransparent = vec4(0.1, 0.5, 1.0, 1.0);
		vec4 mat_diffuse = vec4(0.7, 0.7, 0.7, 1.0);
		
		// diffuse term
		for(int i = 0; i < 2; i++){
			vec3 lightDir = normalize(gl_LightSource[i].position.xyz - vpos);
			float NdotL = dot(n, lightDir);
		
			if (NdotL > 0.0){
				diffuse = mat_diffuse * NdotL;
			}
			else{
				discard;
			}
		}
		
		if(istransparent == 1){
			gl_FragColor = ambientTransparent * 0.5 + diffuse * 0.5;
			gl_FragColor.a = 0.35;
		}		 
		else{
			gl_FragColor = ambientPhong + diffuse;
		}
}


























