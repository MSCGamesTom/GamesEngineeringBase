/*
MIT License

Copyright (c) 2024 MSc Games Engineering Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "GamesEngineeringBase.h"

int main(int argc, char* argv[])
{
	GamesEngineeringBase::Window canvas;
	canvas.create(1024, 768, "Example");
	bool running = true;
	GamesEngineeringBase::Image image;
	image.load("Resources/A.png");
	int planeX = 300;
	int planeY = 300;
	GamesEngineeringBase::SoundManager sounds;
	sounds.loadMusic("Resources/music.wav");
	sounds.playMusic();
	sounds.load("Resources/explosion.wav");
	while (running)
	{
		canvas.checkInput();
		canvas.clear();
		if (canvas.keyPressed(VK_ESCAPE))
		{
			break;
		}
		if (canvas.keyPressed('W'))
		{
			planeY = planeY - 1;
		}
		if (canvas.keyPressed('S'))
		{
			planeY = planeY + 1;
		}
		if (canvas.keyPressed('A'))
		{
			planeX = planeX - 1;
		}
		if (canvas.keyPressed('D'))
		{
			planeX = planeX + 1;
		}
		if (planeX > (canvas.getWidth() - image.width) || planeX < 0 || planeY >(canvas.getHeight() - image.height) || planeY < 0)
		{
			sounds.play("Resources/explosion.wav");
			planeX = (canvas.getWidth() / 2) - (image.width / 2);
			planeY = (canvas.getHeight() / 2) - (image.height / 2);
		}
		for (int i = 0; i < (canvas.getWidth() * canvas.getHeight()); i++)
		{
			canvas.draw(i, 0, 0, 255);
		}
		for (int i = 0; i < image.height; i++)
		{
			for (int n = 0; n < image.width; n++)
			{
				if ((planeX + n) >= 0 && (planeX + n) < canvas.getWidth() && (planeY + i) >= 0 && (planeY + i) < canvas.getHeight())
				{
					if (image.alphaAt(n, i) > 210)
					{
						canvas.draw(planeX + n, planeY + i, image.at(n, i));
					}
				}
			}
		}
		canvas.present();
	}
}