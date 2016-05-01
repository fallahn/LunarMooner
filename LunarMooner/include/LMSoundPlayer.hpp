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

#ifndef LM_SOUNDPLAYER_HPP_
#define LM_SOUNDPLAYER_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>

#include <map>
#include <list>
#include <array>

namespace xy
{
    class SoundResource;
}

namespace lm
{
    class SoundPlayer final : public xy::Component
    {
    public:
        SoundPlayer(xy::MessageBus&, xy::SoundResource&);
        ~SoundPlayer() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        using ResourceID = sf::Int32;
        void preCache(ResourceID, const std::string&, sf::Uint8 = 0);
        void playSound(ResourceID, float, float, float = 1.f);
        void setMasterVolume(float);
        void setChannelVolume(sf::Uint8, float);
    private:
        xy::SoundResource& m_soundResource;
        struct Buffer final
        {
            Buffer() = default;
            Buffer(sf::SoundBuffer& b) : buffer(b) {}
            sf::SoundBuffer buffer;
            sf::Uint8 channel = 0;
        };
        std::map<ResourceID, Buffer> m_buffers;
        std::list<sf::Sound> m_sounds;
        float m_volume;

        std::array<float, 4u> m_channelVolumes;
    };
}

#endif //LM_SOUNDPLATER_HPP_