

#include "MinimalBase.h"

int main(int argc, char* argv[])
{
	MinimalBase::Window canvas;
	canvas.create(1024, 768, "Example");
	bool running = true;
	MinimalBase::Image image;
	image.load("A.png");
	int planeX = 300;
	int planeY = 300;
	MinimalBase::SoundManager sounds;
	sounds.loadMusic("music.wav");
	sounds.playMusic();
	sounds.load("explosion.wav");
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
		if (planeX > (canvas.width - image.width) || planeX < 0 || planeY >(canvas.height - image.height) || planeY < 0)
		{
			sounds.play("explosion.wav");
			planeX = (canvas.width / 2) - (image.width / 2);
			planeY = (canvas.height / 2) - (image.height / 2);
		}
		for (int i = 0; i < (canvas.width * canvas.height); i++)
		{
			canvas.draw(i, 0, 0, 255);
		}
		for (int i = 0; i < image.height; i++)
		{
			for (int n = 0; n < image.width; n++)
			{
				if ((planeX + n) >= 0 && (planeX + n) < canvas.width && (planeY + i) >= 0 && (planeY + i) < canvas.height)
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