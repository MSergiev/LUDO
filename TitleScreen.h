/*
 * TitleScreen.h
 *
 *  Created on: Jul 17, 2017
 *      Author: Puzz
 */

#ifndef TITLESCREEN_H_
#define TITLESCREEN_H_

#include "UI.h"

class TitleScreen: public UI {
public:
	TitleScreen();
	virtual ~TitleScreen();
	void init();
	virtual int eventHandler(SDL_Event& e);
	virtual void render();
private:
	Button StartButton;
	Button ContinueButton;
	Button QuitButton;

};

#endif /* TITLESCREEN_H_ */