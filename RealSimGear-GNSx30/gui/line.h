#pragma once
#include "stdafx.h"
#include "tools.h"
#include "widget.h"
#include "color.h"


namespace gui {

	class line : public widget {

	private:
		color _color;
		float _line_width;
		inline static const color default_color = color(115, 122, 137);

	protected:
		virtual void draw(XPLMWindowID window_id, int window_left, int window_top) override {
			int left = window_left + _left;
			int right = left + _width;
			int top = window_top - _top;
			int bottom = top - _height;

			GLfloat col[4];
			_color.to_gl_color(col);
			glColor4fv(col);

			glLineWidth(_line_width);

			glBegin(GL_LINES);
			glVertex2i(left, top);
			glVertex2i(right, bottom);
			glEnd();
		}

	public:
		line(int left, int top, int width, int height, 
			 color color=default_color, float line_width=1)
			: widget(left, top, width, height), _color(color), _line_width(line_width) {}

		void set_color(color color) { _color = color; }
		void set_line_width(float line_width) { _line_width = line_width; }

	};

}
