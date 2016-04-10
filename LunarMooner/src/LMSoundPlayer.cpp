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

#include <LMSoundPlayer.hpp>
#include <Game.hpp>

#include <xygine/Resource.hpp>
#include <xygine/util/Math.hpp>

using namespace lm;

namespace
{

}

SoundPlayer::SoundPlayer(xy::MessageBus& mb, xy::SoundResource& sr)
    : xy::Component (mb, this),
    m_soundResource (sr),
    m_volume        (0.f)
{
    for (auto& v : m_channelVolumes) v = 1.f;
}

//public
void SoundPlayer::entityUpdate(xy::Entity&, float)
{
    m_sounds.remove_if([](const sf::Sound& s) {return (s.getStatus() == sf::Sound::Stopped); });
}

void SoundPlayer::preCache(ResourceID id, const std::string& path, sf::Uint8 channel)
{
    m_buffers.insert(std::make_pair(id, m_soundResource.get(path)));
    m_buffers[id].channel = channel;
}

void SoundPlayer::playSound(ResourceID id, float x, float y)
{
    m_sounds.emplace_back(m_buffers[id].buffer);
    auto& sound = m_sounds.back();

    sound.setPosition({ x, y, 0.f });
    sound.setVolume(m_volume * m_channelVolumes[m_buffers[id].channel]);
    sound.setAttenuation(0.f);
    sound.play();
}

void SoundPlayer::setMasterVolume(float vol)
{
    m_volume = std::min(vol * Game::MaxVolume, Game::MaxVolume);
}

void SoundPlayer::setChannelVolume(sf::Uint8 channel, float vol)
{
    m_channelVolumes[channel] = xy::Util::Math::clamp(vol, 0.f, 1.f);
}