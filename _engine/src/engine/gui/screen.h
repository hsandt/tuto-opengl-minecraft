#pragma once

#include "engine/gui/panel.h"
#include "engine/gui/bouton.h"
#include "engine/gui/edtbox.h"
#include "engine/gui/lstbox.h"
#include "engine/gui/combox.h"
#include "engine/gui/slope.h"
#include "engine/gui/label.h"
#include "engine/gui/slider.h"
#include "engine/gui/progressbar.h"

class GUIScreen
{
	public :
		std::vector<GUIPanel *> Elements;
		bool _Active;

	public :
		GUIScreen()
		{
			_Active = true;
		}

		~GUIScreen()
		{
			for (GUIPanel* panel : Elements)
			{
				delete panel;
			}
		}

		void activate(bool active)
		{
			_Active = active;
		}
		
		void addElement(GUIPanel * element) { Elements.push_back(element);}

		// added util to quickly add label + param (inline fine since a mere shortcut)
		void addLabeledElement(uint16 x, uint16 &y, std::string text, GUIPanel * panel, int bottomMargin = 10)
		{
			GUILabel * label = new GUILabel();
			label->X = x;
			label->Y = y;
			label->Text = text;
			addElement(label);

			y += label->Height + 1;

			panel->setPos(x, y);
			addElement(panel);

			y += panel->Height + 1;
			y += bottomMargin;  // bonus margin to make things readable
		}

		//retourne TRUE si concerne par le click
		bool mouseCallback(int x, int y, uint32 click, sint32 wheel, uint32 elapsed)
		{
			bool focusAvailable = true;
			//ZORDER 1
			for(unsigned int i = 0;i<Elements.size();i++)
			{
				if(Elements[i]->mouseCallback(x,y,click,wheel,1,focusAvailable,elapsed))
					focusAvailable = false;
			}
			
			//ZORDER 0
			for(unsigned int i = 0;i<Elements.size();i++)
			{
				if(Elements[i]->mouseCallback(x,y,click,wheel,0,focusAvailable,elapsed))
					focusAvailable = false;
			}

			return !focusAvailable;
		}


		sint8 keyCallback(char car,bool * keys, uint32 elapsed)
		{
			sint8 res = 0;

			//Si c'est tab, on bouge le focus
			if(car == '\t')
			{
				for(unsigned int i = 0;i<Elements.size();i++)
				{
					if(Elements[i]->hasFocus())
					{
						Elements[i]->loseFocus();
						Elements[(i+1)%Elements.size()]->setFocus();
						break;
					}
				}
			}
			else
			{
				for(unsigned int i = 0;i<Elements.size();i++)
				{
					if(Elements[i]->keyCallback(car, keys, elapsed))
						res = 1;
				}
			}
			
			return res;
		}

		sint8 specialKeyCallback(char car,bool * keys, uint32 elapsed)
		{
			sint8 res = 0;
			for(unsigned int i = 0;i<Elements.size();i++)
			{
				if(Elements[i]->specialKeyCallback(car, keys, elapsed))
					res = 1;
			}
			return res;
		}


		void render(void)
		{
			//ZORDER 0
			for(unsigned int i = 0;i<Elements.size();i++)
			{
				Elements[i]->render(0);
			}
					

			//ZORDER 1
			for(unsigned int i = 0;i<Elements.size();i++)
			{
				Elements[i]->render(1);
			}

		}

		void update(float elapsed)
		{
			for(unsigned int i = 0;i<Elements.size();i++)
			{
				Elements[i]->update(elapsed);
			}
		}



};