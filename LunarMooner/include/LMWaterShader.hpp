/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

LunarMooner - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#ifndef LM_WATER_SHADER_HPP_
#define LM_WATER_SHADER_HPP_

#include <LMShaderIds.hpp>

namespace lm
{
    static const std::string WaterFrag =
        "#version 120\n"

        "uniform float u_time = 0.0;\n"
        "uniform sampler2D u_texture;\n"
        "uniform float u_texOffset = 0.5;\n"

        "const float waveCount = 65.0;\n"
        "const float amplitude = 0.009;\n"
        "const float reflectIntensity = 0.9;\n"

        "void main()\n"
        "{\n"
        "    vec2 coord = gl_TexCoord[0].xy;\n"

        "    float surface = ((sin(coord.x * 60.0 + (u_time * 100.0)) + 1.1) / 2.0) * 0.012;\n"
        "    surface -= ((cos((1.0 - coord.x) * 80.0 + (u_time * 120.0)) + 1.1) / 2.0) * 0.011;\n"
        "    if(coord.y < u_texOffset + surface)\n"
        "    {\n"
        "        discard;\n"
        "    }\n"

        "    coord.x += sin((coord.y + u_time) * waveCount) * amplitude * (coord.y - u_texOffset);\n"
        "    coord.y += cos((coord.x + u_time) * waveCount) * amplitude * (coord.y - u_texOffset);\n"

        "    vec4 refractColour = texture2D(u_texture, coord);\n"
        "    refractColour.rgb *= gl_Color.rgb;\n"

        "    float mixAmount = coord.y * reflectIntensity;\n"
        "    coord.y -= (coord.y - u_texOffset) * 2.0;\n"
        "    vec4 reflectColour = texture2D(u_texture, coord);\n"

        "    gl_FragColor.rgb = mix(reflectColour.rgb, refractColour.rgb, mixAmount);// + vec3(dither * 5.0);\n"
        "    gl_FragColor.a = max(refractColour.a, gl_Color.a);\n"
        "}\n";
}

#endif //LM_WATER_SHADER_HPP_