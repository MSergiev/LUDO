//-----------------------------
//-----------INCLUDES----------
//-----------------------------

//Include SDL modules
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

//Include local modules
#include "Shared.h"
#include "Board.h"
#include "Player.h"
#include "Sound.h"
#include "Dice.h"
#include "Button.h"
#include "TitleScreen.h"
#include "WinScreen.h"
#include "Recovery.h"

//Misc library inclusion
#include <iostream>
using std::cerr;
using std::endl;
#include <vector>
using std::vector;
#include <deque>
using std::deque;
#include <algorithm>
using std::random_shuffle;

//-----------------------------
//----------VARIABLES----------
//-----------------------------

//SDL attributes
SDL_Window* window;
SDL_Renderer* renderer;

//Exit flag
bool quit = 0;

//Game state flags
bool title = 1;
bool loop = 1;
bool win = 1;

//SDL event container
SDL_Event event;

//Game font
TTF_Font* font;

//Game board
Board board;

//Button object
Button button(600, 600, 100, 60);

//Dice object
Dice dice;

//Title screen object
TitleScreen titleScreen;

//Win screen object
WinScreen winScreen;

//Active board layout (top row leftmost square considered 1)
Pawn* boardLayout[BOARD_LENGTH+10] = {NULL};
unsigned pawnsOnSquare[BOARD_LENGTH+10] = {0};

//Board highlighter array
Button boardHighlghters[BOARD_LENGTH+6];

//Turn counter
int turns = 0;

//Ordered player container
deque<Player*> turnOrder;

//Active highlighter vector
vector<int> activeHighlighters;

//-----------------------------
//---------PROTOTYPES----------
//-----------------------------

//SDL initializing function
bool init();

//Event handler
void eventHandler();

//Resource freeing function
void free();

//Get screen coordinates from pawn position
//Args:
//Colors c - pawn color
//int p - pawn position
pair<int, int> getCoords(Colors c, int p);

//Player turn
//Args:
//Player *p - pointer to active player
void turn(Player *p);

//Determine turn order
void determineTurnOrder();

//Roll the dice
//Args:
//Colors c - dice color
int diceRoll(Colors c);

//Move pawn
//Args:
//Pawn* p - pawn pointer
//int from - index on board array to move it from
//int with - amount of spaces to move it with
void movePawn(Pawn* p, int with);

//Collision detection
//Args:
//Pawn* p - pawn pointer
//int to - index on board array to move to
void collision(Pawn* p, int to);

//Pawn highlighter
//Args:
//int index - square index to highlight
//Colors c - color for base selection (not required)
void highlight(int index, Colors c = NONE);

//Highlighted squares event handler
int getHighlightedChoice();

//Delay
//Args:
//Uint32 ms - milliseconds to delay for
void delay(Uint32 ms);

//Render assets
//Args:
//bool renderDice - flag to render dice
void render(bool renderDice = 0);

//Activate pawn
//Args:
//Player p* - pointer to player
void activatePawn(Player* p);

//Get position relative to pawn
//Args:
//Colors c - pawn color
//int pos - absolute position (not required)
inline int getRelative(Colors c, int pos = 0);

//-----------------------------
//------------MAIN-------------
//-----------------------------

int main(int argc, char* argv[]){
#ifdef DEBUG
	cout << "========= DEBUG MODE =========" << endl;
#endif
	
	//Unused warning elimination
	argc = 0; argv = 0;

	//Initialize SDL
	if(!init()) return 1;

	//Play BGM
	//Sound::music(rock);
	

	//Game loop
	while(!quit){
		//On game title screen
		while(title){
			eventHandler();
			titleScreen.render();
			SDL_RenderPresent(renderer);
		}

#ifdef DEBUG
		cout << "Exited title screen" << endl;
#endif
		
		//On game loop
		while(loop){

#ifdef DEBUG
			cout << "Active: ";
			for(unsigned i = 0; i < BOARD_LENGTH+10; ++i) cout << pawnsOnSquare[i];
			cout << endl;
			cout << "Pawns:  ";
			for(unsigned i = 0; i < BOARD_LENGTH+10; ++i) cout << (!boardLayout[i]?NONE:boardLayout[i]->getEColor());
			cout << endl;
#endif
				
			//Handle events
			eventHandler();
			
			//Render objects
			render();
			
			//Execute player turn
			//for(unsigned i = 0; i < turnOrder.size(); i++)
			//	if(turnOrder[i]->getEColor()==YELLOW){ turn(turnOrder[i]); break; }
			turn(turnOrder.front());

			//Increment turn counter
			turns++;

			//Draw on screen
			SDL_RenderPresent(renderer);
		}
	
		//On game win
		while(win){
			eventHandler();
			winScreen.loadData(turnOrder);
			winScreen.render();
			SDL_RenderPresent(renderer);
		}
	}

#ifdef DEBUG
	cout << "Game loop broken" << endl;
#endif

	//Free resources
	free();

#ifdef DEBUG
	cout << "========= SUCCESSFUL EXIT =========" << endl;
#endif
	//Successful exit
	return 0;
}

//-----------------------------
//----------FUNCTIONS----------
//-----------------------------

//Render all assets
void render(bool renderDice){

#ifdef DEBUG
	//cout << "Render called" << endl;
#endif
	
	//Clear screen
		//SDL_SetRenderDrawColor(renderer, 255,255,255,255);
		//SDL_RenderClear(renderer);

		//Draw game board;
		board.render();
		
		//Draw button
		button.render();
		
		//Render highlighters
		for(unsigned i = 0; i < activeHighlighters.size(); ++i)
			boardHighlghters[activeHighlighters[i]].render();
		
		//Draw player sprites
		for(unsigned i = 0; i < turnOrder.size(); ++i){
			//Get player pawn screen coordinates
			vector<pair<int,int> > pos;
			//Found pawns counter
			int pawnCounter = 0;
			//Traverse current player pawns
			for(unsigned j = 0; j < turnOrder[i]->m_vPawns.size(); ++j){
				//If current pawn is active
				if(turnOrder[i]->m_vPawns[j]->getIPosition()!=-1){
					//Get pawn screen coordinates
					pos.push_back(getCoords(turnOrder[i]->getEColor(), turnOrder[i]->m_vPawns[j]->getIPosition()));
					//Increment counter
					pawnCounter++;
				}
				//If all active pawns are found
				if(pawnCounter==turnOrder[i]->getIActivePawns()) break;
			}
			
			//Idle pawn coordinate holder
			pair<int, int> idleCoords = getCoords(turnOrder[i]->getEColor(), getRelative(turnOrder[i]->getEColor()));
			//Draw idle pawns
			for(int j = 0; j < 5-pawnCounter; ++j){
				//Position pawns correctly
				switch(turnOrder[i]->getEColor()){
					case YELLOW: idleCoords.first-=SQUARE_SIZE; break;
					case RED: idleCoords.second+=SQUARE_SIZE; break;
					case BLUE: idleCoords.first+=SQUARE_SIZE; break;
					case NONE: break;
				}
				pos.push_back(idleCoords);
			}
			//Render pawns
			turnOrder[i]->Render(pos);
		}
		//Render dice
		if(renderDice) dice.render(turnOrder.front()->getEColor());
}


//SDL inititalizing function
bool init(){
	
#ifdef DEBUG
	cout << "Init called" << endl;
#endif
	
	//Success flag
	bool success = 1;

	//Try to initialize SDL_main
	if(SDL_Init(SDL_INIT_VIDEO)<0){
		cerr << "SDL Error: " << SDL_GetError() << endl;
		success = 0;
	} else {
	//Try to initialize SDL_image
	if(!IMG_Init(IMG_INIT_PNG)){
		cerr << "IMG Error: " << IMG_GetError() << endl;
		success = 0;
	} else {
	//Try to initialize SDL_mixer
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)<0){
		cerr << "MIX Error: " << Mix_GetError() << endl;
		success = 0;
	} else {
	//Try to initialize SDL_ttf
	if(TTF_Init()==-1){
		cerr << "TTF Error: " << TTF_GetError() << endl;
		success = 0;
	} else {
	//Try to set linear filtering
		if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")){
			cerr << "- Linear filtering not enabled!" << endl;
		}
		//Create window
		window = SDL_CreateWindow("LUDO", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
		//Window integrity check
		if(window==NULL){
			cerr << "Window error: " << SDL_GetError() << endl;
			success = 0;
		} else {
			//Create renderer
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
			//Renderer integrity check
			if(renderer==NULL){
				cerr << "Renderer error: " << SDL_GetError() << endl;
				success = 0;
			} else {
				//Load game font
				font = TTF_OpenFont(FONT_PATH, 15);
				if(font==NULL){
					cerr << "Font error: " << TTF_GetError() << endl;
				} else {
					//Initialize game objects
					Sound::load();
					titleScreen.setRenderer(renderer);
					titleScreen.setFont(font);
					titleScreen.init();
					winScreen.setRenderer(renderer);
					winScreen.setFont(font);
					winScreen.init();
					dice.init();
					dice.setRenderer(renderer);
					board.setRenderer(renderer);
					button.setRenderer(renderer);
					button.setLabel("CLICK", font);

					for(int i = 0; i < BOARD_LENGTH+6; ++i)
						boardHighlghters[i].setRenderer(renderer);
				
					//Try to recover state from XML	
					turnOrder = Recovery::ReadFromXML();
					if(!turnOrder.size()){
					   cout << "Recovery data not found" << endl;
				   	   determineTurnOrder();
					} else {
						cout << "Recovery data found" << endl;
					
						//Set player data
						for(unsigned i = 0; i < turnOrder.size(); ++i){
							for(unsigned j = 0; j < turnOrder[i]->m_vPawns.size(); ++j){
								if(turnOrder[i]->m_vPawns[j]->getIPosition()>-1){
									pawnsOnSquare[turnOrder[i]->m_vPawns[j]->getIPosition()]++;
									boardLayout[turnOrder[i]->m_vPawns[j]->getIPosition()] = turnOrder[i]->m_vPawns[j];
								}
							}
						}
						cout << "Player data:" << endl;
						Recovery::Print(turnOrder);
					}
					for(unsigned i = 0; i < turnOrder.size(); ++i){
						turnOrder[i]->SetRenderer(renderer);
					}
				}
			}
	}
	}
	}
	}
	}

	//Return success flag
	return success;
}

//Event handler
void eventHandler(){
#ifdef DEBUG
	//cout << "EventHandler called" << endl;
#endif
	while(SDL_PollEvent(&event)!=0){
		//Application quit event
		if(event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE){
			quit = 1;
			title = 0;
			loop = 0;
			win = 0;
			free();
			std::exit(0);
		}
		//If on title screen
		if(title){
			//Get current button state
			int titleState = titleScreen.eventHandler(event);
			//If start button is clicked exit title screen
			if(titleState & 0b100){ title = 0; loop = 1; }
			//If continue button is clicked exit title screen
			if(titleState & 0b010){ title = 0; loop = 1; }
			//If quit button is clicked quit application
			if(titleState & 0b001){ title = 0; loop = 0; win = 0; quit = 1; }
		}
		//If on win screen
	   	else if(win){
			//Get current button state
			int winState = winScreen.eventHandler(event);
			//If restart button is clicked exit win screen
			if(winState & 0b01){ win = 0; title = 0; loop = 1; }
			//If exit button is clicked quit application
			if(winState & 0b10){ win = 0; title = 0; }
		}
		if(button.isClicked(event)) Sound::play(bruh);
	}
}


//Resource freeing function
void free(){
#ifdef DEBUG
	cout << "Free called" << endl;
#endif
	for(unsigned i = 0; i<turnOrder.size(); ++i)
		delete turnOrder[i];
	
	//Release font
	TTF_CloseFont(font);
	//Release renderer
	SDL_DestroyRenderer(renderer);
	//Release window
	SDL_DestroyWindow(window);
	//SDL Quit functions
	Mix_Quit();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

//Get world coordinates from array index
pair<int, int> getCoords(Colors c, int p){
#ifdef DEBUG
	//cout << "GetCoords called" << endl;
#endif
	//Coordinate pair object
	pair<int, int> coords = {ZERO_X_POS, ZERO_Y_POS};
	//Find safe zone entry point
	int entry = getRelative(c, BOARD_LENGTH-1);
	//If in base
	if(p<0)	coords = {IDLE_POS[c-1][0], IDLE_POS[c-1][1]};
	//If on active squares	
	else if(p<=entry){
		//Calculate position
		for(int i = 0; i < p; ++i){
			//If on active board
			coords.first+=NEXT_SQUARE[i].first*SQUARE_SIZE;
			coords.second+=NEXT_SQUARE[i].second*SQUARE_SIZE;
		}
	}
	//If on safe squares	
	else if(p<=entry+5){
		//Calculate position
		for(int i = 0; i < entry; ++i){
			//If on active board
			coords.first+=NEXT_SQUARE[i].first*SQUARE_SIZE;
			coords.second+=NEXT_SQUARE[i].second*SQUARE_SIZE;
		}
		for(int i = 0; i < p-(BOARD_LENGTH-1); ++i){
			coords.first+=SAFE_SQUARE[c-1].first*SQUARE_SIZE;
			coords.second+=SAFE_SQUARE[c-1].second*SQUARE_SIZE;
		}
	}
	//If on final squares
   	else {
		//Calculate position
		coords.first=FINAL_SQUARE[c-1].first+(p-BOARD_LENGTH-5)*SQUARE_SIZE/2;
		coords.second=FINAL_SQUARE[c-1].second;
	}
	//Return coordinate pair
	return coords;
}

//Player turn
void turn(Player *p){
#ifdef DEBUG
	cout << "Turn called with " << p->getEColor() << endl;
#endif
	//If player has rolled before recovery
	if(!Recovery::hasRolled){
		//Roll the dice
		p->setIDiceRoll(diceRoll(p->getEColor()));
		/*switch(p->getEColor()){
			case YELLOW: p->setIDiceRoll(1); break;
			case RED: p->setIDiceRoll(4); break;
			default: p->setIDiceRoll(2); break;
		}*/
		//Save recovery data
		Recovery::WriteXML(turnOrder, 1);

#ifdef DEBUG
	cout << "Player " << p->getEColor() << " rolled " << p->getIDiceRoll() << endl;
#endif

		delay(100);
	}
	//If roll is a 6 get another turn
	if(p->getIDiceRoll()==6) turnOrder.push_front(p);
	
	//If player has no active pawns
	if(p->getIActivePawns()==0){
		//If roll is a 6
	   	if(p->getIDiceRoll()==6){
			//Add an active pawn
			activatePawn(p);
		}
	}
	//If player has only one active pawn
	else if(p->getIActivePawns()==1 && p->getIDiceRoll()!=6){
		//Traverse the pawns
		for(unsigned i = 0; i < p->m_vPawns.size(); ++i){
			//If pawn is on the board
			if(p->m_vPawns[i]->getIPosition()!=-1){
				cout << "Selected pawn: " << p->m_vPawns[i]->getIPosition() << endl;
				//Move pawn forward
				movePawn(p->m_vPawns[i], p->getIDiceRoll());
				break;	
			}
		}
	}
	//If player has more than one active pawn
	else {
		//If roll is a 6
	   	if(p->getIDiceRoll()==6){
			//Highlight base
			highlight(-1,p->getEColor());
		}
	
		//Active pawn counter
		int activeCount = 0;	
		//Traverse the board
		for(unsigned i = 0; i < p->m_vPawns.size(); ++i){
			//Find active ones
			if(p->m_vPawns[i]->getIPosition()!=-1){
				//Highlight active pawn
				highlight(p->m_vPawns[i]->getIPosition(), p->getEColor());	
				//Increase active counter
				activeCount++;
			}
			//Check if more active pawns exist
			if(activeCount==p->getIActivePawns()) break;
		}
		
		//Get choice from highlights
		int choice = getHighlightedChoice();
		//If base is selected, activate pawn
		if(choice<0) activatePawn(p);
		//Else move selected pawn
		else movePawn(boardLayout[choice], p->getIDiceRoll());	
	}
	
	//Cycle players
	turnOrder.push_back(turnOrder.front());
	//Remove current player from queue
	turnOrder.pop_front();

	//Save recovery data
	Recovery::WriteXML(turnOrder);	
}

//Pawn movement
void movePawn(Pawn* p, int with){
#ifdef DEBUG
	cout << "MovePawn called with " << p->getIPosition() << " " << with << endl;
#endif
	//If movement is within range
	if(getRelative(p->getEColor(),p->getIPosition()+with)<(BOARD_LENGTH+10)){
		//Calculate new player position
		int to;
		//If on active squares
		if(getRelative(p->getEColor(),p->getIPosition()+with)<BOARD_LENGTH)
		   to = (p->getIPosition()+with)%BOARD_LENGTH;
		//If on safe squares
		else to = p->getIPosition()+with;
		//Check for collisions
		collision(p, to);
		cout << "Setting position at " << to << endl;
		//Place pawn in new location
		boardLayout[to] = boardLayout[p->getIPosition()];
		//Decrease old position pawn counter
		pawnsOnSquare[p->getIPosition()]--;
		//Increase board pawn counter
		pawnsOnSquare[to]++;
		//NULL if no more pawns on old position
		if(!pawnsOnSquare[p->getIPosition()]) boardLayout[p->getIPosition()] = NULL;
		//Move pawn forward
		p->setIPosition(to);
		//Add roll to player step count
		turnOrder.front()->setISteps(turnOrder.front()->getISteps()+with);

#ifdef DEBUG		
	cout << "Moved from " << p->getIPosition() << " with " << pawnsOnSquare[p->getIPosition()] << " pawns" << endl;
	cout << "Moved to " << to << " with " << pawnsOnSquare[to] << " pawns" << endl;
#endif	

		//Play SFX
		Sound::play(hitmarker);
	}
}

//Collision detection
void collision(Pawn* p, int to){
#ifdef DEBUG
	cout << "Collision called with " << p->getEColor() << " " << to << endl;
#endif
	//If space is already occupied
	if(pawnsOnSquare[to]!=0){
		//If occupant is a different player
		if(boardLayout[to]->getEColor()!=p->getEColor()){
			//Return other pawn to base
			boardLayout[to]->setIPosition(-1);
			//Go through players to find occupying pawn owner
			for(unsigned i = 1; i < turnOrder.size(); ++i){
				if(boardLayout[to]->getEColor()==turnOrder[i]->getEColor()){
					//Add to other pawns' owners' lost counter
					turnOrder[i]->setILost(turnOrder[i]->getILost()+1);
					//Decrease other players' active pawn counter
					turnOrder[i]->setIActivePawns(turnOrder[i]->getIActivePawns()-1);
					//Decrease board pawn counter
					pawnsOnSquare[to]--;
					//Play SFX
					Sound::play(suprise);
					break;
				}
			}	
			//Add to current players' taken counter
			turnOrder.front()->setITaken(turnOrder.front()->getITaken()+1);
			cout << turnOrder.front()->getEColor() << " took pawn on " << to << endl;
		}
	}
}

//Dice roll
int diceRoll(Colors c){
#ifdef DEBUG
	cout << "DiceRoll called with " << c << endl;
#endif
	//Timer
	Uint32 timer = SDL_GetTicks();
	//Dice roll variable
	int roll = dice.roll();
	//Wait for player click
	do {
		//Handle events
		eventHandler();
		//Render objects
		render(1);
		if((SDL_GetTicks()-timer)>50){
			//Animate dice
			roll = dice.roll();
			dice.render(c);
			timer = SDL_GetTicks();
		}
		SDL_RenderPresent(renderer);
	} while(!dice.Event(event) && !quit);
	//Play SFX
	Sound::play(ding);
	//Return roll	
	return roll;
}

//Activate pawn
void activatePawn(Player* p){
#ifdef DEBUG
	cout << "ActivatePawn called with " << p->getEColor() << endl;
#endif
	//Traverse player pawns
	for(unsigned i = 0; i < p->m_vPawns.size(); ++i){
		//If current pawn is inactive
		if(p->m_vPawns[i]->getIPosition()==-1){
			//Check for collisions
			collision(p->m_vPawns[i], getRelative(p->getEColor()));
			//Place pawn on start position
			p->m_vPawns[i]->setIPosition(getRelative(p->getEColor()));
			//Place pawn on game board
			boardLayout[getRelative(p->getEColor())] = p->m_vPawns[i];
			//Increase board pawn counter
			pawnsOnSquare[getRelative(p->getEColor())]++;
			//Increment player active counter
			p->setIActivePawns(p->getIActivePawns()+1);
			//Play SFX
			Sound::play(suprise);
			break;
		}
	}
}

//Board square highlighter
void highlight(int index, Colors c){
#ifdef DEBUG
	cout << "Highlight called with " << index << " " << c << endl;
#endif
	//Highlighter color
	SDL_Color color;
	//Set highlighter color
	switch(c){
		case RED: color = {255,0,0,255}; break;
		case BLUE: color = {0,0,255,255}; break;
		case YELLOW: color = {255,255,0,255}; break;
		case NONE: color = {255,255,255,255};
	}
	//Get current square screen coordinates
	pair<int, int> coords;
	if(index==-1){
		coords = getCoords(c, getRelative(c));
		//Position base highlighter
		switch(c){
			case YELLOW: coords.first-=SQUARE_SIZE; break;
			case RED: coords.second+=SQUARE_SIZE; break;
			case BLUE: coords.first+=SQUARE_SIZE; break;
			case NONE: break;
		}
	} else coords = getCoords(boardLayout[index]->getEColor(), index);
	//Set highlighter params
	boardHighlghters[index+1].setSize(SQUARE_SIZE, SQUARE_SIZE);
	boardHighlghters[index+1].setLocation(coords.first, coords.second);
	boardHighlghters[index+1].setColor(color);

#ifdef DEBUG
	cout << "Adding highlighter at " << coords.first << " " << coords.second << endl;
#endif
	
	//Add to active highlighter list
	activeHighlighters.push_back(index+1);
}

//Highlighted squares event handler
int getHighlightedChoice(){
#ifdef DEBUG
	cout << "GetHighlightedChoice called" << endl;
#endif
	//If active highlighters exist
	if(activeHighlighters.size()){
		cout << "Active on ";
		for(unsigned i = 0; i < activeHighlighters.size(); i++) cout << (activeHighlighters[i]-1) << " ";
		cout << endl;
		//Wait for user choice
		while(1){
			//Traverse active highlighters
			for(unsigned i = 0; i < activeHighlighters.size(); ++i){
				//If clicked
				if(boardHighlghters[activeHighlighters[i]].isClicked(event)){
					//Save board index before clearing
					int choice = activeHighlighters[i];
					//Clear active highlighters
					while(activeHighlighters.size()>0) activeHighlighters.pop_back();
					//Play SFX
					Sound::play(camera);
					//Return pressed index
					return choice-1;
				}
			}
			//Event handler
			eventHandler();
			//Render sprites
			render();
			//Draw image on screen
			SDL_RenderPresent(renderer);
		}
	}
	return 0;
}

//Determine turn order
void determineTurnOrder(){
#ifdef DEBUG
	cout << "DetermineTurnOrder called" << endl;
#endif
	//Temporary vector of colors to choose from
	vector<Colors> order = {RED, BLUE, YELLOW};
	//Shuffle vector
	random_shuffle(order.begin(), order.end());
	//Initialize player objects
	for(int i = 0; i < PLAYERS; ++i){
		turnOrder.push_back(new Player(order[i]));
		//Add a starting pawn
		activatePawn(turnOrder.back());
	}
#ifdef DEBUG
	cout << "Player turns: " << turnOrder[0]->getEColor() << " " << turnOrder[1]->getEColor() << " " << turnOrder[2]->getEColor() << endl;
#endif
}

//Get relative position
int getRelative(Colors c, int pos){
	return START_POS[c-1]+pos;
}

//Delay
void delay(Uint32 ms){
#ifdef DEBUG
	cout << "Delay called with " << ms << endl;
#endif
	Uint32 timerDelay = SDL_GetTicks();
	while(SDL_GetTicks()-timerDelay<ms && !quit)
		eventHandler();
}
