#include "GameWindow.h"
#include "GameObject.h"

#include <vector>

int screenWidth = 800;
int screenHeight = 600;

GameWindow* bis;
GameObject* bg;
GameObject* wall;
GameObject* gameOver;

GameObject* mc;

GameSurface* coinSurface;

SDL_Rect Clip[8];

int gravity;
int speed;
bool jump;
int mX, mY;
bool kspace, Kleft, Kright;

bool end;

int Map[6][8] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 1, 1, 1, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 1, 1,
	1, 1, 1, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};

struct position
{
	int x, y;
	position(int _x, int _y):x(_x), y(_y){}
};

vector<position*> tile;
vector<GameObject*> coin;

void GenMap()
{
	int i, a;

	for(i = 0; i < 6; i++)
	{
		for(a = 0; a < 8; a++)
		{
			if(Map[i][a] == 1)
			{
				tile.push_back(new position(a*wall->width, i*wall->height));
			}
		}
	}
}

void AddCoin(int x, int y)
{
	GameObject* ob = new GameObject(bis->GetRenderer());
	ob->CreateGameTextureFromGameSurface(coinSurface);
	
	//ob->LoadClip("source/Coin.animate");
	ob->GenerateClip(10, 1);

	ob->width = 44;
	ob->height = 40;
	ob->x = (x*wall->width)+(ob->width/2);
	ob->y = (y*wall->height)+(wall->height-ob->height);
	ob->SetTimeScale(0.8);
	ob->SetAnimation(0, 9);
	ob->Play();

	coin.push_back(ob);
}

void Start()
{
	end = false;

	mX = 0;
	mY = 0;

	gravity = 5;
	speed = 5;
	jump = false;
	kspace = false;
	Kleft = false;
	Kright = false;

	bg = new GameObject(bis->GetRenderer());
	bg->Load("source/BG.jpg");
	bg->width = 800;
	bg->height = 600;

	wall = new GameObject(bis->GetRenderer());
	wall->Load("source/Wall.jpg");
	wall->width = 100;
	wall->height = 100;

	GenMap();

	coinSurface = new GameSurface();
	coinSurface->Load("source/Coin.png");

	AddCoin(2, 1);
	AddCoin(3, 1);
	AddCoin(4, 1);
	AddCoin(1, 3);
	AddCoin(2, 3);
	AddCoin(3, 4);
	AddCoin(4, 4);
	AddCoin(5, 3);

	mc = new GameObject(bis->GetRenderer());
	mc->Load("source/SPplayer.png");

	//mc->LoadClip("source/SPplayer.animate");
	mc->GenerateClip(8, 1);

	mc->width = 50;
	mc->height = 80;
	mc->x = wall->width*6;
	mc->y = wall->height*2;

	mc->SetFlip(FLIP_HORIZONTAL);
	mc->SetTimeScale(0.8);
	mc->SetAnimation(0, 0);
	mc->Play();

	gameOver = new GameObject(bis->GetRenderer());
	gameOver->Load("source/GameOver2.png");
	gameOver->x = 150;
	gameOver->y = 100;
	gameOver->width = 500;
	gameOver->height = 400;
}

bool CheckCollision(GameObject* ob, bool chk = false)
{
	int topLeft = Map[ob->y/wall->height][ob->x/wall->width];
	int topRight = Map[ob->y/wall->height][(ob->x+ob->width-1)/wall->width];
	int bottomLeft = Map[(ob->y+ob->height-1)/wall->height][ob->x/wall->width];
	int bottomRight = Map[(ob->y+ob->height-1)/wall->height][(ob->x+ob->width-1)/wall->width];

	if(topLeft == 1 || topRight == 1 || bottomLeft == 1 || bottomRight == 1)
	{
		if(chk)
		{
			if(topLeft == 1 || topRight == 1)
			{
				ob->y = ((ob->y/wall->height)*wall->height)+wall->height;
				mY = 0;
			}
			
			if(bottomLeft == 1 || bottomRight == 1)
			{
				mY = 0;
				ob->y = (((ob->y+ob->height-1)/wall->height)*wall->height)-ob->height;
				jump = false;
			}
		}
		return true;
	}

	return false;
}

void Update()
{
	int i;

	mY += gravity;

	if(!end)
	{
		if(Kleft)
		{
			mX = -speed;
			mc->SetFlip(FLIP_HORIZONTAL);
		}
		else if(Kright)
		{
			mX = speed;
			mc->SetFlip(FLIP_NONE);
		}
		else if(!Kleft && !Kright)
		{
			mX = 0;
		}

		if(!jump)
		{
			if(!Kleft && !Kright)
			{
				mc->SetAnimation(0, 0);
			}else{
				if(mc->GetStart() != 1 && mc->GetEnd() != 7)
					mc->SetAnimation(1, 7);
			}
		}else{
			mc->SetAnimation(2, 2);
		}

		mc->y += mY;
		if(mc->y <= 0 || mc->y >= screenHeight-mc->height || CheckCollision(mc, true))
		{
			mc->y -= mY;
		}

		mc->x += mX;
		if(mc->x <= 0 || mc->x >= screenWidth-mc->width || CheckCollision(mc))
		{
			mc->x -= mX;
		}
	}

	bg->Render();

	for(i = 0; i < tile.size(); i++)
	{
		wall->x = tile[i]->x;
		wall->y = tile[i]->y;
		wall->Render();
	}

	for(i = 0; i < coin.size(); i++)
	{
		if(coin[i]->HitTest(mc))
		{
			delete coin[i];
			coin[i] = NULL;
			coin.erase(coin.begin()+i);
		}else{
			coin[i]->Render();
		}
	}

	mc->Render();

	if(coin.size() == 0)
	{
		gameOver->Render();
		end = true;
		mc->Stop();
	}
}

void Event()
{
	switch(bis->GetEvent().type)
	{
	case SDL_KEYDOWN:
		if(bis->GetEvent().key.keysym.sym == SDLK_SPACE)
		{
			if(!kspace && !jump)
			{
				mY = -50;
				jump = true;
				kspace = true;
			}
		}
		else if(bis->GetEvent().key.keysym.sym == SDLK_LEFT)
		{
			Kleft = true;
		}
		else if(bis->GetEvent().key.keysym.sym == SDLK_RIGHT)
		{
			Kright = true;
		}
		else if(bis->GetEvent().key.keysym.sym == SDLK_RETURN)
		{
			if(end)
				bis->Quit();
		}
		break;
	case SDL_KEYUP:
		if(bis->GetEvent().key.keysym.sym == SDLK_SPACE)
		{
			kspace = false;
		}
		else if(bis->GetEvent().key.keysym.sym == SDLK_LEFT)
		{
			Kleft = false;
		}
		else if(bis->GetEvent().key.keysym.sym == SDLK_RIGHT)
		{
			Kright = false;
		}
		break;
	}
}

void Close()
{
	int i;

	delete bg;
	bg = NULL;

	for(i = 0; i < tile.size(); i++)
	{
		delete tile[i];
		tile[i] = NULL;
	}

	tile.clear();

	for(i = 0; i < coin.size(); i++)
	{
		delete coin[i];
		coin[i] = NULL;
	}

	coin.clear();

	delete coinSurface;
	coinSurface = NULL;

	delete mc;
	mc = NULL;

	delete gameOver;
	gameOver = NULL;

	delete bis;
	bis = NULL;
}

int main(int argc, char* argv[])
{
	bis = new GameWindow();

	if(!bis->CreateWindow("Sample Game Mario", screenWidth, screenHeight))
	{
		cerr << "Can't Create GameWindow" << endl;
	}else{
		bis->Run((void*)&Start, (void*)&Update, (void*)&Event);
	}

	Close();

	return 0;
}