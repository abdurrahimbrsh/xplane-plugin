#pragma once
#include "stdafx.h"


namespace gui {

	class window;

	class widget {
		friend class window;

	protected:
		int _left, _top, _width, _height;

		virtual void draw(XPLMWindowID window_id, int window_left, int window_top) {

		}

		virtual XPLMCursorStatus handle_cursor(XPLMWindowID window_id, int x, int y) {
			return xplm_CursorDefault;
		}

		virtual bool handle_mouse_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse) {
			return false;
		}

		virtual void update_geometry() {

		}

		bool is_inside(int x, int y) {
			return _left <= x && x <= _left + _width && _top <= y && y <= _top + _height;
		}

	public:

		widget(int left, int top, int width, int height) :
			_left(left), _top(top), _width(width), _height(height) {

			update_geometry();
		}

		int left() { return _left; }
		int top() { return _top; }
		void move(int left, int top) {
			_left = left; _top = top;
			update_geometry();
		}
		void resize(int width, int height) {
			_width = width; _height = height;
			update_geometry();
		}

	};

}