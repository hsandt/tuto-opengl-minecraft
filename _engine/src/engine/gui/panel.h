#pragma once

#include "engine/render/renderer.h"
#include "engine/utils/types_3d.h"
#include "gui.h"

class GUIPanel
{
	public:
		sint32 X;
		sint32 Y;
		uint32 ZOrder;
		sint32 Width;
		sint32 Height;
		NYColor ColorBorder;
		NYColor ColorFond;
		NYColor FocusColor;
		NYColor ValidColor;
		bool Visible;
		bool FondPlein;
		bool MouseOn;
		bool DrawBorder;

	protected:
		bool _HasFocus;
		NYRenderer * _Video;
		NYTextEngine * _TextEngine;
		
	public:
		GUIPanel()
		{
			ZOrder = 0;
			X = 1;
			Y = 1;
			Width = 100;
			Height = 100;
			ColorBorder.R = 1.0f;
			ColorBorder.G = 1.0f;
			ColorBorder.B = 1.0f;
			ColorFond.R = 0.0f;
			ColorFond.G = 0.0f;
			ColorFond.B = 0.0f;
			FocusColor.R = 0.2f;
			FocusColor.G = 0.2f;
			FocusColor.B = 0.2f;
			ValidColor.R = 0.3f;
			ValidColor.G = 0.3f;
			ValidColor.B = 0.3f;
			Visible = true;
			FondPlein = false;
			DrawBorder = true;

			_Video = NYRenderer::getInstance();
			_TextEngine = _Video->_TextEngine;
			_HasFocus = false;
			MouseOn = false;
		}

		virtual void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				if(FondPlein)
					drawSquareFilled(X,Y,Width,Height,ColorFond);

				//Draw border;
				if(DrawBorder)
					drawSquare(X,Y,Width,Height,ColorBorder);	
			}
		}

		virtual sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{
			sint8 mouseForMe = 0;
			MouseOn = false;
			if(zorder == ZOrder)
			{
				if(x > X && x < X+Width && y > Y && y < Y+Height)
				{
					mouseForMe = 1;
					MouseOn = true;
				}
			}
			return 0;
		}

		virtual sint8 keyCallback(char car,bool * keys, uint32 elapsed)
		{
			return 0;
		}

		virtual sint8 specialKeyCallback(char car,bool * keys, uint32 elapsed)
		{
			return 0;
		}

		virtual void loseFocus(void)
		{
			_HasFocus = false;
		}

		virtual void setFocus(void)
		{
			_HasFocus = true;
		}

		virtual bool hasFocus(void)
		{
			return _HasFocus;
		}

		// added for more consistency and being able to work on GUIPanel without
		// knowing subclass
		virtual void setPos(uint16 x, uint16 y)
		{
			X = x;
			Y = y;
		}

		virtual void update(float elapsed)
		{

		}

	protected:
		void inline drawSquare(uint32 x,uint32 y,uint32 width,uint32 height,NYColor color)
		{
			glColor4f(color.R,color.G,color.B,color.A);
			glBegin(GL_LINE_LOOP);
			glVertex3i(x,y,0);
			glVertex3i(x,y+height,0);
			glVertex3i(x+width,y+height,0);
			glVertex3i(x+width,y,0); //je sais pas pquoi il me foire toujours ce coin...
			glVertex3i(x+width,y,0);
			glEnd();
		}

		void inline drawSquareFilled(uint32 x,uint32 y,uint32 width,uint32 height,NYColor color)
		{
			glColor4f(color.R,color.G,color.B,color.A);
			glBegin(GL_QUADS);
			glVertex3i(x,y,0);
			glVertex3i(x,y+height,0);
			glVertex3i(x+width,y+height,0);
			glVertex3i(x+width,y,0);
			glEnd();
		}

};