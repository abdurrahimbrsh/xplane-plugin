#pragma once
#include "stdafx.h"
#include "button.h"
#include "color.h"

namespace gui {

	class circle_button : public button {

	protected:
		const color _circle_color;

		virtual void draw(XPLMWindowID window_id, int window_left, int window_top) override {
			int left = window_left + _left;
			int right = left + _width;
			int top = window_top - _top;
			int bottom = top - _height;

			int center_x = (left + right) / 2;
			int center_y = (top + bottom) / 2;
			int radius = _width / 2;

			GLfloat col[4];
			_circle_color.to_gl_color(col);
			glColor4fv(col);

			glBegin(GL_TRIANGLE_FAN);
			draw_arc_segment(center_x, center_y, radius, 0, -2. * pi);
			glEnd();
		}

		virtual void update_geometry() override {

		}

	public:
		circle_button(color circle_color,
			          std::function<void(std::shared_ptr<button>)> on_clicked,
			          int left, int top, int diameter)
				: button("", on_clicked, left, top, diameter, diameter), _circle_color(circle_color) {

		}

	};

}