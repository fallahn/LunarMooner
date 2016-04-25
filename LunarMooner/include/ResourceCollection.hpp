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

#ifndef LM_RESOURCE_COLLECTION_HPP_
#define LM_RESOURCE_COLLECTION_HPP_

#include <xygine/Resource.hpp>
#include <xygine/ShaderResource.hpp>

struct ResourceCollection
{
    xy::TextureResource textureResource;
    xy::ShaderResource shaderResource;
    xy::FontResource fontResource;
    xy::SoundResource soundResource;

    ResourceCollection() = default;
    ResourceCollection(const ResourceCollection&) = delete;
    ResourceCollection& operator = (const ResourceCollection&) = delete;
};

#endif //LM_RESOURCE_COLLECTION_HPP_