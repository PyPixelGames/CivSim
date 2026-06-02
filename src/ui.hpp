#pragma once
#include <SDL3/SDL_rect.h>
#include <iostream>
#include <vector>
#include <functional>
#include "types.hpp"

enum UIType{
	TEXT,
	BUTTON,
	INPUT
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

	std::string name="";
	bool focused=false;

	std::unordered_map<UIColors, SDL_Color> colors;

	UIPiece(){
		colors[UIColors::MAIN]={ 208, 252, 179, 255};
		colors[UIColors::BG]={34, 34, 59, 255};
		colors[UIColors::ACCENT]={186, 50, 79, 255};
		colors[UIColors::BORDER] ={77, 126, 168, 255};
	}
};

struct FloatingUI{
	SDL_FRect r = {10, 100, 500, 500};
	std::unordered_map<UIColors, SDL_Color> colors;

	std::vector<std::unique_ptr<UIPiece>> pieces;

	Pos originPos;

	SDL_FPoint dragOffset;
	bool dragging=false;
	bool draggable=true;

	bool dirty=true;
	SDL_Texture* tex=nullptr;

	bool focused=false;
	bool open=true;

	FloatingUI(){
		colors[UIColors::MAIN]={ 208, 252, 179, 255};
		colors[UIColors::BG]={34, 34, 59, 255};
		colors[UIColors::ACCENT]={186, 50, 79, 255};
		colors[UIColors::BORDER] ={77, 126, 168, 255};
	}

	void unfocus(){
		focused=false;
		for (auto& p: pieces){
			p->focused=false;
		}
	}

	void focusPieces(SDL_FPoint* mousePoint){
		for (auto& piece: pieces){
			SDL_FRect rect = {r.x + static_cast<float>(piece->relativePos.x),
				r.y + static_cast<float>(piece->relativePos.y),
				piece->width, piece->height};

			bool clicked = SDL_PointInRectFloat(mousePoint, &rect);

			if (clicked){
				piece->focused=true;
			}else{
				piece->focused=false;
			}
		}
	}
};

void updateUI(SDL_Renderer* renderer, FloatingUI& ui, TTF_Font *font, Mouse& mouse, Keyboard& keyboard);
