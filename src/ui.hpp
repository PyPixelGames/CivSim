#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include "types.hpp"

enum UIType{
	TEXT,
	BUTTON
};

enum UIColors{
	MAIN,
	BG,
	ACCENT,
	BORDER
};

struct UIPiece{
	UIType type = UIType::TEXT;

	Pos relativePos={10, 10};
	float width=50;
	float height=50;

	std::function<void()> function;

	std::string name;

	std::unordered_map<UIColors, SDL_Color> colors;

	UIPiece(){
		colors[UIColors::MAIN]={0, 0, 0, 255};
		colors[UIColors::BG]={45, 45, 45, 255};
		colors[UIColors::ACCENT]={0, 125, 125, 255};
		colors[UIColors::BORDER]={255, 0, 0, 255};
	}
};

struct FloatingUI{
	SDL_FRect r = {10, 100, 500, 500};
	std::unordered_map<UIColors, SDL_Color> colors;

	std::vector<std::unique_ptr<UIPiece>> pieces;

	Pos originPos;

	bool dirty=true;
	SDL_Texture* tex=nullptr;

	bool focused=false;
	bool open=true;

	FloatingUI(){
		colors[UIColors::MAIN]={225, 225, 225, 255};
		colors[UIColors::BG]={0, 125, 125, 255};
		colors[UIColors::ACCENT]={125, 0, 125, 255};
		colors[UIColors::BORDER]={0, 0, 0, 255};
	}
};

void updateUI(SDL_Renderer* renderer, FloatingUI& ui, TTF_Font *font, SDL_FPoint& leftclick);
