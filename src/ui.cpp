#include "ui.hpp"
#include "helper.hpp"
#include <SDL3/SDL_render.h>


void updateUI(SDL_Renderer* renderer, FloatingUI& ui, TTF_Font *font, Mouse& mouse){
	if (ui.open){
		//RENDER
		if (ui.dirty){
			if (ui.tex) {
				SDL_DestroyTexture(ui.tex);
				ui.tex = nullptr;
			}

			ui.tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,
					static_cast<int>(ui.r.w),static_cast<int>(ui.r.h));
			SDL_SetTextureBlendMode(ui.tex, SDL_BLENDMODE_BLEND);

			SDL_SetRenderTarget(renderer, ui.tex);
			SDL_SetRenderDrawColor(renderer,bg.r, bg.g, bg.b, bg.a);
			SDL_RenderClear(renderer);


			SDL_FRect fillRect = {0, 0, ui.r.w, ui.r.h};
			if (ui.focused){
				setColor(renderer,ui.colors[UIColors::BORDER]);
				SDL_RenderFillRect(renderer, &fillRect);


				SDL_FRect fillRectSmall = {bWidth, bWidth, ui.r.w-(bWidth*2), ui.r.h-(bWidth*2)};
				setColor(renderer,ui.colors[UIColors::BG]);
				SDL_RenderFillRect(renderer, &fillRectSmall);
			}else{
				setColor(renderer,ui.colors[UIColors::BG]);
				SDL_RenderFillRect(renderer, &fillRect);
			}


			for (auto& piece: ui.pieces){
				SDL_FRect r={
					static_cast<float>(piece->relativePos.x),
					static_cast<float>(piece->relativePos.y),
					piece->width,
					piece->height
				};
				if (piece->type==UIType::TEXT){
					std::string text;
					size_t fontPos=piece->name.find("@");
					if(fontPos!=std::string::npos){
						std::string size=piece->name.substr(0, fontPos);
						text=piece->name.substr(fontPos+1);
						TTF_SetFontSize(font, std::stoi(size));
					}else{
						TTF_SetFontSize(font, 18);
						text=piece->name;
					}

					SDL_Texture* textTex=renderText(renderer,text,font,piece->colors[UIColors::MAIN]);

					float w, h;
					SDL_GetTextureSize(textTex, &w, &h);
					SDL_FRect dst={
						static_cast<float>(piece->relativePos.x),
						static_cast<float>(piece->relativePos.y),
						w, h};
					SDL_RenderTexture(renderer, textTex, nullptr, &dst);
					SDL_DestroyTexture(textTex);
				}

				if (piece->type==UIType::BUTTON){
					setColor(renderer,piece->colors[UIColors::BG]);
					SDL_RenderFillRect(renderer, &r);
				}
			}

			SDL_SetRenderTarget(renderer, nullptr);
			SDL_SetTextureScaleMode(ui.tex, SDL_SCALEMODE_NEAREST);

			ui.dirty=false;
		}
		SDL_RenderTexture(renderer, ui.tex, nullptr, &ui.r);


		//HANDLE EVENTS
		if (ui.focused){
			SDL_FPoint localLeftClick = {
				mouse.left.x - ui.r.x,
				mouse.left.y - ui.r.y
			};
			SDL_FPoint leftClick = {
				static_cast<float>(mouse.left.x),
				static_cast<float>(mouse.left.y)
			};


			//DRAG
			SDL_FRect dragRect={ui.r.x, ui.r.y, ui.r.w, 20};
			if (mouse.left.x != -1 && mouse.left.y!=-1){
				ui.dragOffset=localLeftClick;
			}
			if (SDL_PointInRectFloat(&mouse.pos, &dragRect)){
				if (mouse.holdingLeft && mouse.holding){
					std::cout << "holding" << std::endl;
					ui.r.x=mouse.pos.x-ui.dragOffset.x;
					ui.r.y=mouse.pos.y-ui.dragOffset.y;
				}
			}

			// PIECES UPDATE
			for (auto& piece: ui.pieces){
				SDL_FRect r={
					static_cast<float>(piece->relativePos.x),
					static_cast<float>(piece->relativePos.y),
					piece->width, piece->height};

				bool clicked = SDL_PointInRectFloat(&localLeftClick, &r);

				if (piece->type==UIType::BUTTON && clicked){
					if (piece->function) piece->function();
				}
			}

		}
	}

}
