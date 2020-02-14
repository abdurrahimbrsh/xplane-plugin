#pragma once
#include "stdafx.h"
#include "tools.h"
#include "widget.h"
#include "color.h"


namespace gui {

	class label : public widget {

	private:
		std::string _text;
		XPLMFontID _font;
		color _color;
		inline static const color default_color = color();

	protected:

		virtual void draw(XPLMWindowID window_id, int window_left, int window_top) override {
			int lower = _top + _height;		
			XPLMDrawString(
				_color.rgb, window_left + _left, window_top - lower,
				const_cast<char *>(_text.c_str()), nullptr, _font);
		}

		virtual void update_geometry() override {
			auto [width, height] = measure_string(_text, _font);
			_width = width;
			_height = height;
		}


	public:

		label(const std::string &text, int left, int top, 
			  color color=default_color, XPLMFontID font=xplmFont_Proportional)
			: widget(left, top, 0, 0), _text(text), _color(color), _font(font) {
			update_geometry();
		}

		void set_text(const std::string &text) {
			_text = text;
			update_geometry();
		}

		void set_font(XPLMFontID font) {
			_font = font;
			update_geometry();
		}

		void set_color(color color) {
			_color = color;
		}

	};

}