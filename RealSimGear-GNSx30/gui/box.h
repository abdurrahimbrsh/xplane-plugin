#pragma once
#include "stdafx.h"
#include "tools.h"
#include "widget.h"
#include "color.h"


namespace gui {

	class box : public widget {

	private:
		color _color;
		inline static const color default_color = color(0.0f, 0.0f, 0.0f);

	protected:
		virtual void draw(XPLMWindowID window_id, int window_left, int window_top) override {
			int left = window_left + _left;
			int right = left + _width;
			int top = window_top - _top;
			int bottom = top - _height;

			GLfloat col[4];
			_color.to_gl_color(col);
			glColor4fv(col);

			glBegin(GL_POLYGON);
			glVertex2i(left, top);
			glVertex2i(right, top);
			glVertex2i(right, bottom);
			glVertex2i(left, bottom);
			glVertex2i(left, top);
			glEnd();
		}

	public:
		box(int left, int top, int width, int height, color color=default_color)
			: widget(left, top, width, height), _color(color) {}

		void set_color(color color) { _color = color; }

	};

}