/*********************************************************************
Matt Marchant 2014 - 2016
http://trederia.blogspot.com

xygine - Zlib license.

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

#ifndef XY_TMX_OBJECT_HPP_
#define XY_TMX_OBJECT_HPP_

#include <xygine/Config.hpp>
#include <xygine/tilemap/Property.hpp>

#include <SFML/Graphics/Rect.hpp>

#include <string>

namespace pugi
{
    class xml_node;
}

namespace xy
{
    namespace tmx
    {
        /*!
        \brief Objects are stored in ObjectGroup layers.
        Objects may be rectangular, elliptical, polygonal or
        a polyline. Rectangular and elliptical Objects have their
        size determined via the AABB, whereas polygon and polyline
        shapes are defined by a list of points. Objects are 
        rectangular by default.
        */
        class XY_EXPORT_API Object final
        {
        public:
            enum class Shape
            {
                Rectangle,
                Ellipse,
                Polygon,
                Polyline
            };

            Object();
            ~Object() = default;

            /*!
            \brief Attempts to parse the given xml node and
            read the Object properties if it is valid.
            */
            void parse(const pugi::xml_node&);

            /*!
            \brief Returns the unique ID of the Object
            */
            sf::Uint32 getUID() const { return m_UID; }
            /*!
            \brief Returns the name of the Object
            */
            const std::string& getName() const { return m_name; }
            /*!
            \brief Returns the type of the Object, as defined in the editor
            */
            const std::string& getType() const { return m_type; }
            /*!
            \brief Returns the position of the Object in pixels
            */
            const sf::Vector2f& getPosition() const { return m_position; }
            /*!
            \brief Returns the global Axis Align Bounding Box.
            The AABB is positioned via the left and top properties, and
            define the Objects width and height. This can be used to derive
            the shape of the Object if it is rectangular or elliptical.
            */
            const sf::FloatRect& getAABB() const { return m_AABB; }
            /*!
            \brief Returns the rotation of the Object in degrees clockwise
            */
            float getRotation() const { return m_rotation; }
            /*!
            \brief Returns the global tile ID associated with the Object
            if there is one. This is used to draw the Object (and therefore
            the Object must be rectangular)
            */
            sf::Uint32 getTileID() const { return m_tileID; }
            /*!
            \brief Returns whether or not the Object is visible
            */
            bool visible() const { return m_visible; }
            /*!
            \brief Returns the Shape of the Object
            */
            Shape getShape() const { return m_shape; }
            /*!
            \brief Returns a reference to the vector of points which
            make up the Object. If the Object is rectangular or elliptical
            then the vector will be empty. Point coordinates are in pixels,
            relative to the object position.
            */
            const std::vector<sf::Vector2f>& getPoints() const { return m_points; }
            /*!
            \brief Returns a reference to the vector of properties belonging to
            the Object.
            */
            const std::vector<Property>& getProperties() const { return m_properties; }

        private:
            sf::Uint32 m_UID;
            std::string m_name;
            std::string m_type;
            sf::Vector2f m_position;
            sf::FloatRect m_AABB;
            float m_rotation;
            sf::Uint32 m_tileID;
            bool m_visible;

            Shape m_shape;
            std::vector<sf::Vector2f> m_points;
            std::vector<Property> m_properties;

            void parsePoints(const pugi::xml_node&);
        };
    }
}

#endif //XY_TMX_OBJECT_HPP_