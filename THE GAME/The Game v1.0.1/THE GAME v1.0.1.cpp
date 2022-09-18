#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <math.h>
#include <string.h>

#define ESC 27
#define KEY_UP 119
#define KEY_DOWN 115
#define KEY_LEFT 97
#define KEY_RIGHT 100
#define KEY_SELECT 13

#define MAP_SIZE 302 //Add 2 for buffering

#define APPLICATION_VERSION "1.0.1"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//Engine Helpers

enum GameState
{
	MainMenu,
	Loading,
	Game,
	Won,
	PauseMenu,
	Settings,
	Quit,	
};

typedef struct Vector2
{
	int _x;
	int _y;
	Vector2()
	{
		
	}
	Vector2(int x,int y)
	{
		_x = x;
		_y = y;
	}
};

typedef struct SavedFile
{
	char _name[20];
	int _currentLevel;
};

class Window
{
	public: 
		int _width;
		int _height;
		int _halfHeight;
		int _halfWidth;
		Window(int x, int y)
		{
			_height = y;
			_width = x;
			_halfHeight = y/2;
			_halfWidth = x/2;
		}
};

void SetWindowSize(int x,int y, Window *&_window)
{
	_window->_width = x;
	_window->_height = y;
	_window->_halfWidth = x/2;
	_window->_halfHeight = y/2;
}
int WindowSizeChange(Window *_window)
{
	int x,y;
	char ip1[100],ip2[100];
	while(1)
	{
		system("cls");
		printf("New screen size (51>X,Y>3): ");
		scanf("%s %s",&ip1,&ip2);
		x = atoi(ip1);
		y = atoi(ip2);
		if(x<3 || x>51 || y<3 || y>51)
		{
			printf("\nInvalid screen size! Press any key to try again");
			getch();
			continue;
		}	
		SetWindowSize(x,y,_window);
		break;
	}
	
}

int TickRateChange(int &_tickRate)
{
	int x;
	char ip[100];
	while(1)
	{
		system("cls");
		printf("New tick rate (501>X>0): ");
		scanf("%s",&ip);
		x = atoi(ip);
		if(x<1 || x>500)
		{
			printf("\nInvalid tick rate! Press any key to try again");
			getch();
			continue;
		}	
		_tickRate = x;	
		break;
	}

}

typedef struct Player
{
	Vector2 _position;
	char _icon;
	Player(int x,int y,char i)
	{
		_position._x=x;
		_position._y=y;
		_icon = i;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//Block deffining
enum BlockType
{
	Block_Wall,
	Block_Move,
	Block_Switch,
	Block_Teleport,
	Block_Win,
	Block_Ground,
	Block_Activate,
	Block_Player,
	//If you want to add a new block type, you must add to the Load Game
};

typedef struct Block
{
	BlockType _blockType;
	Vector2 _position;
	char _icon;
};

void SwitchBlock(Block &a,Block &b)
{
	Block t = a;
	a = b;
	b = t;
}

bool Block_MoveChain(Vector2 *_position,Player *_player,char _code,Block _block[][MAP_SIZE])
{
	
	if(_block[_position->_y][_position->_x]._blockType == Block_Move)
	{
		switch(_code)
		{
			case 'w':
			{
				return Block_MoveChain(new Vector2(_position->_x,(_position->_y)+1),_player,'w',_block);
			}
			case 'a':
			{
				return Block_MoveChain(new Vector2((_position->_x)-1,_position->_y),_player,'a',_block);
			}
			case 's':
			{
				return Block_MoveChain(new Vector2(_position->_x,(_position->_y)-1),_player,'s',_block);
			}
			case 'd':
			{
				return Block_MoveChain(new Vector2((_position->_x)+1,_position->_y),_player,'d',_block);
			}
		}
	}
	if(_block[_position->_y][_position->_x]._blockType == Block_Ground)
	{
		switch(_code)
		{
			case 'w':
			{
				SwitchBlock(_block[(_player->_position)._y+1][(_player->_position)._x],_block[_position->_y][_position->_x]);
				break;
			}
			case 'a':
			{
				SwitchBlock(_block[(_player->_position)._y][(_player->_position)._x-1],_block[_position->_y][_position->_x]);
				break;
			}
			case 's':
			{
				SwitchBlock(_block[(_player->_position)._y-1][(_player->_position)._x],_block[_position->_y][_position->_x]);
				break;			}
			case 'd':
			{
				SwitchBlock(_block[(_player->_position)._y][(_player->_position)._x+1],_block[_position->_y][_position->_x]);
				break;
			}
		}
		return true;
	}
	if(_block[_position->_y][_position->_x]._blockType == Block_Activate && _block[_position->_y][_position->_x]._icon == '+')
	{
		switch(_code)
		{
			case 'w':
			{
				_block[(_player->_position)._y+1][(_player->_position)._x]._icon = ' ';
				_block[(_player->_position)._y+1][(_player->_position)._x]._blockType = Block_Ground;
				_block[_position->_y][_position->_x]._icon = 'A';
				break;
			}
			case 'a':
			{
				_block[(_player->_position)._y][(_player->_position)._x-1]._icon = ' ';
				_block[(_player->_position)._y][(_player->_position)._x-1]._blockType = Block_Ground;
				_block[_position->_y][_position->_x]._icon = 'A';
				break;
			}
			case 's':
			{
				_block[(_player->_position)._y-1][(_player->_position)._x]._icon = ' ';
				_block[(_player->_position)._y-1][(_player->_position)._x]._blockType = Block_Ground;
				_block[_position->_y][_position->_x]._icon = 'A';
				break;			}
			case 'd':
			{
				_block[(_player->_position)._y][(_player->_position)._x+1]._icon = ' ';
				_block[(_player->_position)._y][(_player->_position)._x+1]._blockType = Block_Ground;
				_block[_position->_y][_position->_x]._icon = 'A';
				break;
			}
		}
		return true;
	}
	return false;
}

typedef struct TeleportHash
{
	Vector2 *_pos1;
	Vector2 *_pos2;
};

void NewTPH(Vector2 *_p1,Vector2 *_p2,TeleportHash _tph[],int _index)
{
	(_tph[_index])._pos1= _p1;
	(_tph[_index])._pos2= _p2;
}

Vector2 CheckTP(Vector2 *pos,TeleportHash _tph[],int _tpCount)
{
	for(int i=1;i<=_tpCount;i++)
	{
	TeleportHash _temp =  _tph[i];
		if((_temp._pos1)->_x == pos->_x && (_temp._pos1)->_y == pos->_y)
			return 	*(_temp._pos2);
		if((_temp._pos2)->_x == pos->_x && (_temp._pos2)->_y == pos->_y)
			return 	*(_temp._pos1);
	}
	return *(new Vector2(0,0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//Input Handler
void Input(bool &A_Quit, GameState &_gameState,Player *&_player,Block _block[][MAP_SIZE],TeleportHash _tph[],int _tpCount, int &currentLevel,char _fileName[],int _atvCount,Vector2 _atvPos[])
{
	if(kbhit())
	{
		switch(getch())
		{
			//y+1
			case KEY_UP:
				{
					switch(_block[(_player->_position)._y+1][(_player->_position)._x]._blockType)
					{
						case Block_Ground:
						{
							(_player->_position)._y++;
							break;
						}
						case Block_Switch:
						{
							if(_block[(_player->_position)._y][(_player->_position)._x]._blockType != Block_Teleport)
							{
								SwitchBlock(_block[(_player->_position)._y][(_player->_position)._x],_block[(_player->_position)._y+1][(_player->_position)._x]);
								(_player->_position)._y++;
							}
							break;
						}
						case Block_Move:
						{
							if(Block_MoveChain(new Vector2((_player->_position)._x,(_player->_position)._y+2),_player,'w',_block))
							{
								(_player->_position)._y++;
							}
							break;
						}
						case Block_Wall:
						{
							break;
						}
						case Block_Activate:
						{
							break;
						}
						case Block_Teleport:
						{
							_player->_position = CheckTP(new Vector2((_player->_position)._x,(_player->_position)._y+1),_tph,_tpCount);
							break;
						}
						case Block_Win:
						{
							bool _winAble = true;
							for(int i=1;i<=_atvCount;i++)
							{
								if(_block[_atvPos[i]._y][_atvPos[i]._x]._icon == '+')
								{
									_winAble = false;
									break;
								}
							}
							if(_winAble)
							{
								currentLevel++;
								sprintf(_fileName,"Level File/Level%d.txt",currentLevel);
								if(fopen(_fileName,"r")== NULL)
									_gameState = Won;
								else
									_gameState = Loading;
							}
							break;
						}
					}
					break;
				}
			//y-1
			case KEY_DOWN:
				{
					switch(_block[(_player->_position)._y-1][(_player->_position)._x]._blockType)
					{
						case Block_Ground:
						{
							(_player->_position)._y--;
							break;
						}
						case Block_Switch:
						{
							if(_block[(_player->_position)._y][(_player->_position)._x]._blockType != Block_Teleport)
							{
								SwitchBlock(_block[(_player->_position)._y][(_player->_position)._x],_block[(_player->_position)._y-1][(_player->_position)._x]);
								(_player->_position)._y--;
							}
							break;
						}
						case Block_Move:
						{
							if(Block_MoveChain(new Vector2((_player->_position)._x,(_player->_position)._y-2),_player,'s',_block))
							{
								(_player->_position)._y--;
							}
							break;
						}
						case Block_Wall:
						{
							break;
						}
						case Block_Activate:
						{
							break;
						}
						case Block_Teleport:
						{
							_player->_position = CheckTP(new Vector2((_player->_position)._x,(_player->_position)._y-1),_tph,_tpCount);
							break;
						}
						case Block_Win:
						{
							bool _winAble = true;
							for(int i=1;i<=_atvCount;i++)
							{
								if(_block[_atvPos[i]._y][_atvPos[i]._x]._icon == '+')
								{
									_winAble = false;
									break;
								}
							}
							if(_winAble)
							{
								currentLevel++;
								sprintf(_fileName,"Level File/Level%d.txt",currentLevel);
								if(fopen(_fileName,"r")== NULL)
									_gameState = Won;
								else
									_gameState = Loading;
							}
							break;
						}
					}
					break;
				}
			//x-1
			case KEY_LEFT:
				{
					switch(_block[(_player->_position)._y][(_player->_position)._x-1]._blockType)
					{
						case Block_Ground:
						{
							(_player->_position)._x--;
							break;
						}
						case Block_Switch:
						{
							if(_block[(_player->_position)._y][(_player->_position)._x]._blockType != Block_Teleport)
							{
								SwitchBlock(_block[(_player->_position)._y][(_player->_position)._x],_block[(_player->_position)._y][(_player->_position)._x-1]);
								(_player->_position)._x--;
							}
							break;
						}
						case Block_Move:
						{
							if(Block_MoveChain(new Vector2((_player->_position)._x-2,(_player->_position)._y),_player,'a',_block))
							{
								(_player->_position)._x--;
							}
							break;
						}
						case Block_Wall:
						{
							break;
						}
						case Block_Activate:
						{
							break;
						}
						case Block_Teleport:
						{
							_player->_position = CheckTP(new Vector2((_player->_position)._x-1,(_player->_position)._y),_tph,_tpCount);
							break;
						}
						case Block_Win:
						{
							bool _winAble = true;
							for(int i=1;i<=_atvCount;i++)
							{
								if(_block[_atvPos[i]._y][_atvPos[i]._x]._icon == '+')
								{
									_winAble = false;
									break;
								}
							}
							if(_winAble)
							{
								currentLevel++;
								sprintf(_fileName,"Level File/Level%d.txt",currentLevel);
								if(fopen(_fileName,"r")== NULL)
									_gameState = Won;
								else
									_gameState = Loading;
							}
							break;
						}
					}
					break;
				}
			//x+1
			case KEY_RIGHT:
				{
					switch(_block[(_player->_position)._y][(_player->_position)._x+1]._blockType)
					{
						case Block_Ground:
						{
							(_player->_position)._x++;
							break;
						}
						case Block_Switch:
						{
							if(_block[(_player->_position)._y][(_player->_position)._x]._blockType != Block_Teleport)
							{
								SwitchBlock(_block[(_player->_position)._y][(_player->_position)._x],_block[(_player->_position)._y][(_player->_position)._x+1]);
								(_player->_position)._x++;
							}
								
							break;
						}
						case Block_Move:
						{
							if(Block_MoveChain(new Vector2((_player->_position)._x+2,(_player->_position)._y),_player,'d',_block))
							{
								(_player->_position)._x++;
							}
							break;
						}
						case Block_Wall:
						{
							break;
						}
						case Block_Activate:
						{
							break;
						}
						case Block_Teleport:
						{
							_player->_position = CheckTP(new Vector2((_player->_position)._x+1,(_player->_position)._y),_tph,_tpCount);
							break;
						}
						case Block_Win:
						{
							bool _winAble = true;
							for(int i=1;i<=_atvCount;i++)
							{
								if(_block[_atvPos[i]._y][_atvPos[i]._x]._icon == '+')
								{
									_winAble = false;
									break;
								}
							}
							if(_winAble)
							{
								currentLevel++;
								sprintf(_fileName,"Level File/Level%d.txt",currentLevel);
								if(fopen(_fileName,"r")== NULL)
									_gameState = Won;
								else
									_gameState = Loading;
							}
							break;
						}
					}
					break;
				}
			case ESC:
				{
					_gameState = PauseMenu;
					break;
				}
		}
	}
}
void HideCursor()
{
	//Hide Cursor
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CursoInfo;
	CursoInfo.dwSize = 1;         /* The size of caret */
	CursoInfo.bVisible = false;   /* Caret is visible? */
	SetConsoleCursorInfo(hConsole, &CursoInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//Data Handling
void LoadGame(char _fileName[50],FILE *_fptr,Player *&_player,Block _blockArr[][MAP_SIZE],TeleportHash _tph[],int &_tpCount ,int &currentLevel,bool &_gameSaved,char _saveName[],int &_atvCount, Vector2 _atvPos[])
{
	//Reset map properties
	_atvCount = 0;
	_tpCount = 0;
	_fptr = fopen(_fileName,"r+");
	
	if(strstr(_fileName,"Level")==NULL)
	{
		fgets(_saveName,30,_fptr);
	}
	fscanf(_fptr,"%d",&currentLevel);
	getc(_fptr);
	//Read map
	for(int i = MAP_SIZE - 2;i>=0;i--)
	{
		for(int j = 0;j<=MAP_SIZE - 2;j++)
		{
			_blockArr[i][j]._icon = getc(_fptr);
			switch(_blockArr[i][j]._icon)
			{
				case ' ':
				{
					_blockArr[i][j]._blockType =  Block_Ground;
					break;
				}
				case 'W':
				{
					_blockArr[i][j]._blockType =  Block_Wall;
					break;				
				}
				case 'S':
				{
					_blockArr[i][j]._blockType =  Block_Switch;
					break;				
				}
				case 'N':
				{
					_blockArr[i][j]._blockType =  Block_Move;
					break;				
				}
				case 'T':
				{
					_blockArr[i][j]._blockType =  Block_Teleport;
					break;				
				}
				case '+':
				{
					_blockArr[i][j]._blockType =  Block_Activate;
					break;
				}
				case 'A':
				{
					_blockArr[i][j]._blockType =  Block_Activate;
					break;
				}
				case '*':
				{
					_blockArr[i][j]._blockType =  Block_Win;
					break;				
				}
				default:
				{
					_blockArr[i][j]._icon = ' ';
					_blockArr[i][j]._blockType =  Block_Ground;
					break;
				}
			}
		}
		getc(_fptr);
	}
	//Read Player pos
	int p_x,p_y;
	fscanf(_fptr,"%d %d",&p_x,&p_y);
	_player = new Player(p_x,p_y,'P');
	//Read Teleport points
	int _x1,_y1,_x2,_y2;
	fscanf(_fptr,"%d",&_tpCount);
	for(int i=1;i<=_tpCount;i++)
	{
		fscanf(_fptr,"%d %d %d %d",&_x1,&_y1,&_x2,&_y2);
		NewTPH(new Vector2(_x1,_y1),new Vector2(_x2,_y2), _tph, i);
	}
	//Read activation points
	fscanf(_fptr,"%d",&_atvCount);
	for(int i=1;i<=_atvCount;i++)
	{
		fscanf(_fptr,"%d %d",&_atvPos[i]._x,&_atvPos[i]._y);
	}
	_gameSaved = false;
	fclose(_fptr);
}

void LoadSettings(int &_tickRate,Window *_window)
{
	int _x,_y;
	FILE *_fptr;
	_fptr = fopen("Settings.txt","r");
	//(tick rate,window width,window height)
	fscanf(_fptr,"%d %d %d",&_tickRate,&_x,&_y);
	SetWindowSize(_x,_y,_window);
	fclose(_fptr);
}

void SaveGame(Block _blockArr[][MAP_SIZE],FILE *_fptr,char _fileName[50],int _currentLevel,int _tpCount, TeleportHash _tph[],Player *_player,bool &_gameSaved,char _saveName[],int _atvCount,Vector2 _atvPos[])
{
	_fptr = fopen(_fileName,"w");
	//Save name and level
	fprintf(_fptr,"%s\n",_saveName);
	fprintf(_fptr,"%d\n",_currentLevel);
	//Print map
	for(int i = MAP_SIZE - 2;i>=0;i--)
	{
		for(int j = 0;j<=MAP_SIZE - 2;j++)
		{
			fprintf(_fptr,"%c",_blockArr[i][j]._icon);
		}
		fprintf(_fptr,"\n");
	}
	//Player position
	fprintf(_fptr,"%d %d\n",(_player->_position)._x,(_player->_position)._y);
	//Teleport save
	fprintf(_fptr,"%d\n",_tpCount);
	for(int i = 1;i<=_tpCount;i++)
	{
		fprintf(_fptr,"%d %d %d %d\n",(_tph[i]._pos1)->_x,(_tph[i]._pos1)->_y,(_tph[i]._pos2)->_x,(_tph[i]._pos2)->_y);
	}
	//Activation point save
	fprintf(_fptr,"%d\n",_atvCount);
	for(int i = 1;i<=_atvCount;i++)
	{
		fprintf(_fptr,"%d %d\n",_atvPos[i]._x,_atvPos[i]._y);
	}
	
	_gameSaved = true;
	fclose(_fptr);
}

void SaveSettings(int _tickRate,Window *_window)
{
	int _x,_y;
	FILE *_f;
	_f = fopen("Settings.txt","w");
	//(tick rate,window width,window height)
	fprintf(_f,"%d %d %d",_tickRate,_window->_width,_window->_height);
	fclose(_f);
}

void SaveProgress(int _index,Block _blockArr[][MAP_SIZE],FILE *_fptr,char _fileName[50],int _currentLevel,int _tpCount, TeleportHash _tph[],Player *_player,bool &_gameSaved,char _saveName[],SavedFile _fileProperties[],int currentLevel,char _strArr [][50],int _atvCount,Vector2 _atvPos[])
{
	if(!strcmpi(_saveName,"-None-"))
	{
		char _temp[30];
		while(1)
		{
			system("cls");
			printf("Enter the new save name (s<=20,""cancel"" to exit): ");
			fflush(stdin);
			gets(_temp);
			if(strlen(_temp)>20)
			{
				printf("\nSave name is to long!Press any key to retry!");
				getch();
				continue;
			}
			if(!strcmpi(_temp,"cancel"))
				break;
			strcpy(_saveName,_temp);
			strcpy(_fileProperties[_index]._name,_saveName);
			_fileProperties[_index]._currentLevel = currentLevel;
			sprintf(_fileName,"Save File/Save%d.txt",_index+1);
			SaveGame(_blockArr,_fptr,_fileName,currentLevel,_tpCount,_tph,_player,_gameSaved,_saveName,_atvCount,_atvPos);
			break;
		}
	}
	else
	{
		strcpy(_fileProperties[_index]._name,_saveName);
		_fileProperties[_index]._currentLevel = currentLevel;
		sprintf(_fileName,"Save File/Save%d.txt",_index+1);
		SaveGame(_blockArr,_fptr,_fileName,currentLevel,_tpCount,_tph,_player,_gameSaved,_saveName,_atvCount,_atvPos);
	}
	//Display the changed save
	strcpy(_strArr[_index],"");
	//Name of the save file
	strcpy(_strArr[_index],_fileProperties[_index]._name);
	int _temp = strlen(_strArr[_index]);
	//Spacing so the total length of the name is 33
	for(int j=_temp ;j<33;j++)
	{
		strcat(_strArr[_index]," ");
	}
	//Add current level
	sprintf(_strArr[_index],"%s%d",_strArr[_index],_fileProperties[_index]._currentLevel);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//Game State UI
void MainMenuUI(GameState &_gameState,bool &A_quit,char _fileName[50],SavedFile _fileProperties[],char _saveName[])
{
	int maxNum = 2;
	int currentNum = 0;
	char _nameArr[maxNum+2][50];
	strcpy(_nameArr[0],"New Game");
	strcpy(_nameArr[1],"Load Game");
	strcpy(_nameArr[2],"Quit");
	do
	{
		system("cls");
		printf("\n\n");
		printf("	==========	||      ||	========	==========	     ^     	||\\      /||	========\n");
		printf("	    ||     	||      ||	||      	||        	   // \\\\  	|| \\    / ||	||      \n");
		printf("	    ||     	||      ||	||      	||        	 //     \\\\ 	||  \\  /  ||	||      \n");
		printf("	    ||     	||======||	||======	||  ======	||       ||	||   \\/   ||	||======\n");
		printf("	    ||     	||      ||	||      	||      ||	||=======||	||        ||	||      \n");
		printf("	    ||     	||      ||	||      	||      ||	||       ||	||        ||	||      \n");
		printf("	    ||     	||      ||	========	==========	||       ||	||        ||	========\n\n");
		printf("                                                    v%s                                         \n",APPLICATION_VERSION);
		printf("\n\n");
		for(int i = 0;i<=maxNum;i++)
		{
			if(i==currentNum)
				printf("						--> ");
			else
				printf("						    ");
			printf("%s\n",_nameArr[i]);
		}
		switch(getch())
		{
			case KEY_UP:
				{
					if(currentNum==0)
						currentNum=maxNum;
					else
						currentNum--;
					break;
				}
			case KEY_DOWN:
				{
					if(currentNum==maxNum)
						currentNum=0;
					else
						currentNum++;
					break;
				}
			case KEY_SELECT:
				{
					switch(currentNum)
					{
						case 0:
							{
								strcpy(_saveName,"-None-");
								_gameState = Loading;
								strcpy(_fileName,"Level File/Level1.txt");
								break;
							}
						case 1:
							{
								// Load Game screen
								bool _localExit = false;
								int currentIndex = 0;
								char _strArr [7][50];
								for(int i=0;i<=4;i++)
								{
									//Name of the save file
									strcpy(_strArr[i],_fileProperties[i]._name);
									int _temp = strlen(_strArr[i]);
									//Spacing so the total length of the name is 30
									for(int j=_temp ;j<30;j++)
									{
										strcat(_strArr[i]," ");
									}
									//Add current level
									sprintf(_strArr[i],"%s%d",_strArr[i],_fileProperties[i]._currentLevel);
								}
								strcpy(_strArr[5],"Back");
								
								do
								{
									system("cls");
									printf("      ================= LOAD GAME =================\n");
									printf("      ||       Name         ||   Current Level   ||\n");
									for(int i=0;i<=5;i++)
										{
											if(i==5)
												printf("\n");
											if(currentIndex==i)
												printf("\n   -->");
											else
												printf("\n      ");
											
											printf("%s",_strArr[i]);
											
										}
										switch(char c=getch())
										{
											case KEY_UP:
												{
													currentIndex--;
													if(currentIndex<0)
														currentIndex=5;
													break;
												}
											case KEY_DOWN:
												{
													currentIndex++;
													if(currentIndex>5)
														currentIndex=0;
													break;
												}
											case KEY_SELECT:
												{
													switch(currentIndex)
													{
														case 0:
															{
																if(_fileProperties[0]._currentLevel==0)
																	break;
																strcpy(_fileName,"Save File/Save1.txt");
																_gameState = Loading;
																_localExit = true;
																break;
															}
														case 1:
															{
																if(_fileProperties[1]._currentLevel==0)
																	break;
																strcpy(_fileName,"Save File/Save2.txt");
																_gameState = Loading;
																_localExit = true;
																break;
															}
														case 2:
															{
																if(_fileProperties[2]._currentLevel==0)
																	break;
																strcpy(_fileName,"Save File/Save3.txt");
																_gameState = Loading;
																_localExit = true;
																break;
															}
														case 3:
															{
																if(_fileProperties[3]._currentLevel==0)
																	break;
																strcpy(_fileName,"Save File/Save4.txt");
																_gameState = Loading;
																_localExit = true;
																break;
															}
														case 4:
															{
																if(_fileProperties[4]._currentLevel==0)
																	break;
																strcpy(_fileName,"Save File/Save5.txt");
																_gameState = Loading;
																_localExit = true;
																break;
															}
														case 5:
															{
																_localExit = true;
																break;
															}
													}
													break;
												}
										}
									
									Sleep(5);
								}while(!_localExit);
								break;
							}
						case 2:
							{
								A_quit = true;
								_gameState = Quit;
								break;
							}
					}
					break;
				}
		}
		Sleep(5);
	}while(_gameState==MainMenu);
}

void PauseMenuUI(GameState &_gameState,bool &A_Quit,Block _blockArr[][MAP_SIZE],FILE *_fptr,char _fileName[], int &currentLevel,int &_tpCount,TeleportHash _tph[],Player *_player,bool _gameSaved,SavedFile _fileProperties[],char _saveName[],int _atvCount,Vector2 _atvPos[])
{
	int maxNum = 4;
	int currentNum = 0;
	char _nameArr[maxNum+2][50];
	strcpy(_nameArr[0],"Resume");
	strcpy(_nameArr[1],"Restart Level");
	strcpy(_nameArr[2],"Save Game");
	strcpy(_nameArr[3],"Settings");
	strcpy(_nameArr[4],"Main Menu");

	do
	{
		system("cls");
		printf("    PAUSE MENU\n\n");
		for(int i = 0;i<=maxNum;i++)
		{
			if(i==currentNum)
				printf(" --> ");
			else
				printf("    	 ");
			printf("%s\n",_nameArr[i]);
		}
		switch(getch())
		{
			case KEY_UP:
				{
					if(currentNum==0)
						currentNum=maxNum;
					else
						currentNum--;
					break;
				}
			case KEY_DOWN:
				{
					if(currentNum==maxNum)
						currentNum=0;
					else
						currentNum++;
					break;
				}
			case KEY_SELECT:
				{
					switch(currentNum)
					{
						case 0:
							{
								//Resume Game
								_gameState = Game;
								break;
							}
						case 1:
							{
								//Restart Level
								sprintf(_fileName,"Level File/Level%d.txt",currentLevel);
								_gameState = Loading;
								break;
							}
						case 2:
							{
								//Save Game screen
								bool _localExit = false;
								int currentIndex = 0;
								char _strArr [7][50];
								for(int i=0;i<=4;i++)
								{
									//Name of the save file
									strcpy(_strArr[i],_fileProperties[i]._name);
									int _temp = strlen(_strArr[i]);
									//Spacing so the total length of the name is 30
									for(int j=_temp ;j<33;j++)
									{
										strcat(_strArr[i]," ");
									}
									//Add current level
									sprintf(_strArr[i],"%s%d",_strArr[i],_fileProperties[i]._currentLevel);
								}
								strcpy(_strArr[5],"Back");
								
								do
								{
									system("cls");
									printf("      ================= SAVE GAME =================\n");
									printf("      ||       Name         ||   Current Level   ||\n");
									for(int i=0;i<=5;i++)
										{
											if(i==5)
												printf("\n");
											if(currentIndex==i)
												printf("\n   -->");
											else
												printf("\n      ");
											
											printf("%s",_strArr[i]);
											
										}
										switch(char c=getch())
										{
											case KEY_UP:
												{
													currentIndex--;
													if(currentIndex<0)
														currentIndex=5;
													break;
												}
											case KEY_DOWN:
												{
													currentIndex++;
													if(currentIndex>5)
														currentIndex=0;
													break;
												}
											case KEY_SELECT:
												{
													switch(currentIndex)
													{
														case 0:
															{
																SaveProgress(0,_blockArr,_fptr,_fileName,currentLevel,_tpCount,_tph,_player,_gameSaved,_saveName,_fileProperties,currentLevel,_strArr,_atvCount,_atvPos);
																break;
															}
														case 1:
															{
																SaveProgress(1,_blockArr,_fptr,_fileName,currentLevel,_tpCount,_tph,_player,_gameSaved,_saveName,_fileProperties,currentLevel,_strArr,_atvCount,_atvPos);
																break;
															}
														case 2:
															{
																SaveProgress(2,_blockArr,_fptr,_fileName,currentLevel,_tpCount,_tph,_player,_gameSaved,_saveName,_fileProperties,currentLevel,_strArr,_atvCount,_atvPos);
																break;
															}
														case 3:
															{
																SaveProgress(3,_blockArr,_fptr,_fileName,currentLevel,_tpCount,_tph,_player,_gameSaved,_saveName,_fileProperties,currentLevel,_strArr,_atvCount,_atvPos);
																break;
															}
														case 4:
															{
																SaveProgress(4,_blockArr,_fptr,_fileName,currentLevel,_tpCount,_tph,_player,_gameSaved,_saveName,_fileProperties,currentLevel,_strArr,_atvCount,_atvPos);
																break;
															}
														case 5:
															{
																_localExit = true;
																break;
															}
													}
													break;
												}
										}
									
									Sleep(5);
								}while(!_localExit);
								break;
							}
						case 3:
							{
								//Settings
								_gameState = Settings;
								break;
							}
						case 4:
							{
								//Check for player confirmaion if they haven't saved
								if(_gameSaved)
									_gameState = MainMenu;
								else 
								{
									bool _exit = false;
									int currentIndex = 0;
									char _words [3][10];
									strcpy(_words[0],"Yes");
									strcpy(_words[1],"No");
									do
									{
										
										system("cls");
										printf("   You haven't saved yet, are you sure you want to go back to the main menu?");
										printf("\n");
										for(int i=0;i<=1;i++)
										{
											if(currentIndex==i)
												printf("\n   -->");
											else
												printf("\n      ");
											printf("%s",_words[i]);
										}
										switch(char c=getch())
										{
											case KEY_UP:
												{
													currentIndex--;
													if(currentIndex<0)
														currentIndex=1;
													break;
												}
											case KEY_DOWN:
												{
													currentIndex++;
													if(currentIndex>1)
														currentIndex=0;
													break;
												}
											case KEY_SELECT:
												{
													switch(currentIndex)
													{
														case 0:
															{
																_gameState = MainMenu;
																break;
															}
														case 1:
															{
																//Don't do anything here
																break;
															}
													}
													_exit = true;
													break;
												}
										}
										Sleep(5);
									}while(!_exit);
								}
								break;
							}
					}
					break;
				}
		}
		Sleep(5);
	}while(_gameState==PauseMenu);
}

void SettingsUI(GameState &_gameState,int &_tickRate,Window *_window)
{
	int maxNum = 4;
	int currentNum = 0;
	char _nameArr[maxNum+2][50];
	strcpy(_nameArr[0],"Window Size");
	strcpy(_nameArr[1],"Tick Rate");
	strcpy(_nameArr[2],"Audio Level");
	strcpy(_nameArr[3],"Quality");
	strcpy(_nameArr[4],"Back");

	do
	{
		system("cls");
		printf("	    SETTINGS MENU\n\n");
		for(int i = 0;i<=maxNum;i++)
		{
			if(i==currentNum)
				printf("	 --> ");
			else
				printf("	    	 ");
			printf("%s\n",_nameArr[i]);
		}
		switch(getch())
		{
			case KEY_UP:
				{
					if(currentNum==0)
						currentNum=maxNum;
					else
						currentNum--;
					break;
				}
			case KEY_DOWN:
				{
					if(currentNum==maxNum)
						currentNum=0;
					else
						currentNum++;
					break;
				}
			case KEY_SELECT:
				{
					switch(currentNum)
					{
						case 0:
							{
								//Change Window size
								WindowSizeChange(_window);
								break;
							}
						case 1:
							{
								//Change the refresh rate
								TickRateChange(_tickRate);
								break;
							}
						case 2:
							{
								//Audio
								break;
							}
						case 3:
							{
								//Quality for a shitty 2D game
								break;
							}
						case 4:
							{
								//Back to the Pause Menu
								_gameState = PauseMenu;
								break;
							}
					}
					break;
				}
		}
		Sleep(5);
	}while(_gameState==Settings);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//This is where we draw our game

void DrawGame(Window *_window,Block _block[][MAP_SIZE],Player *_player,int currentLevel,/*Debug*/TeleportHash _tph[],int _tpCount,Vector2 _atvPos[],int _atvCount)
{
	system("cls");
	//Game Window
	for(int j=(_window->_height)+1;j>=0;j--)
	{
		if(j==0 || j==(_window->_height)+1)
		{
			//Upper border
			for(int index =0;index<=(_window->_width)+1;index++)
			{
				printf("=");
			}
			printf("\n");
		}
		else
		{
			//Side border
			for(int i=0;i<=(_window->_width)+1;i++)
			{
				if(i==0 || i==(_window->_width)+1)
				{
					printf("|");
					if(i==(_window->_width)+1)
						printf("\n");
				}
				else
				{
					//Draw Game Here
					if((j-(_window->_halfHeight)==1)&&(i-(_window->_halfWidth) ==1))
					{
						printf("P");
						continue;
					}
					printf("%c",_block[j+((_player->_position)._y)-(_window->_halfHeight)-1][i+((_player->_position)._x)-(_window->_halfWidth)-1]._icon);
				}
			}
		}
	}
	//Debug
	printf("\n Position: X %d, Y %d",(_player->_position)._x,(_player->_position)._y);
	printf("\n Current level: %d", currentLevel);
}
void Logic()
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//Main function where we excute everything

int main()
{
	HideCursor();
	//Game engine
	bool A_Quit = false;
	FILE* _fptr;
	char _fileName[50];
	char _saveName[30] = "-None-";
	int _tickRate = 5;
	GameState _gameState = MainMenu;
	static Window *_window = new Window(20,7);
	Player *_player;

	//Game Data
	Block _blockArr[MAP_SIZE][MAP_SIZE];
	TeleportHash _tph[100];
	Vector2 _atvPos[100];
	int _atvCount = 0;
	int _tpCount = 0;
	int currentLevel = 0;
	
	//Save Game properties for displaying saved files
	bool _gameSaved = false;
	SavedFile _fileProperties[7];
	for(int i = 0;i<=4;i++)
	{
		sprintf(_fileName,"Save File/Save%d.txt",i+1);
		_fptr = fopen(_fileName,"r");
		if(_fptr == NULL)
		{
			strcpy(_fileProperties[i]._name,"-None-");
			_fileProperties[i]._currentLevel = 0;
		}
		else
		{
			//Get the save name
			fgets(_fileProperties[i]._name,30,_fptr);
			//Delete the \n in the string
			_fileProperties[i]._name[strlen(_fileProperties[i]._name)-1] ='\0';
			//Get current Level
			fscanf(_fptr,"%d",&_fileProperties[i]._currentLevel);
		}
		fclose(_fptr);
	}
	
	//Game Loop
	do
	{
		switch(_gameState)
		{
			case MainMenu:
			{
				MainMenuUI(_gameState,A_Quit,_fileName,_fileProperties,_saveName);
				break;
			}
			case Loading:
			{
				system("cls");
				printf("	LOADING GAME\n");
				LoadSettings(_tickRate,_window);
				printf("/////////////");
				Sleep(500);
				LoadGame(_fileName,_fptr,_player,_blockArr,_tph,_tpCount, currentLevel,_gameSaved,_saveName,_atvCount,_atvPos);
				printf("/////////////");
				Sleep(500);
				_gameState = Game;
				break;	
			}
			case Game:
			{
				do
				{
					Input(A_Quit,_gameState,_player,_blockArr,_tph,_tpCount,currentLevel,_fileName,_atvCount,_atvPos);
					Logic();
					DrawGame(_window,_blockArr,_player,currentLevel,_tph,_tpCount,_atvPos,_atvCount);
					//UpdateDelay
					Sleep(_tickRate);
				}while(_gameState==Game);
				break;
			}
			case Won:
			{
				system("cls");
				printf("YOU WIN!\n\n");
				Sleep(1000);
				printf("PRESS ANY KEY TO RETURN TO MENU");
				getch();
				_gameState = MainMenu;
				break;
			}
			case Settings:
			{
				SettingsUI(_gameState,_tickRate,_window);
				SaveSettings(_tickRate,_window);
				break;
			}
			case PauseMenu:
			{
				PauseMenuUI(_gameState,A_Quit,_blockArr,_fptr,_fileName,currentLevel,_tpCount,_tph,_player,_gameSaved,_fileProperties,_saveName,_atvCount,_atvPos);
				break;
			}
		}
	}while(A_Quit == false);

	return 0;
}
