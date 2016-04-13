/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

Lunar Mooner Map Editor - Zlib license.

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

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MapEditor
{
    /// <summary>
    /// Contains functions for drawing and manipulating sprites
    /// </summary>

    public partial class MainWindow : Form
    {
        private class TexName
        {
            public string Name { get; set; }
            public SFML.Graphics.Texture Texture { get; set; }
        }
        private List<SFML.Graphics.Sprite> m_sprites = new List<SFML.Graphics.Sprite>();

        private void buttonAddTexture_Click(object sender, EventArgs e)
        {
            OpenFileDialog od = new OpenFileDialog();
            od.Filter = "Portable Network Graphic|*.png|JPEG files|*.jpg|Bitmap|*.bmp";
            if(od.ShowDialog() == DialogResult.OK)
            {
                TexName tn = new TexName();
                tn.Texture = new SFML.Graphics.Texture(od.FileName);
                tn.Name = Path.GetFileName(od.FileName);
                listBoxTextures.Items.Add(tn);

                SFML.Graphics.Sprite spr = new SFML.Graphics.Sprite(tn.Texture);
                m_sprites.Add(spr);
            }
        }

        public void DrawSprites(SFML.Graphics.RenderWindow window)
        {
            foreach(var spr in m_sprites)
            {
                window.Draw(spr);
            }
        }
    }
}
