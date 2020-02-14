#pragma once
#include "stdafx.h"
#include "widget.h"
#include "color.h"


namespace gui {

	class button : public widget, public std::enable_shared_from_this<button> {

	protected:
		enum class mouse_state { passive, highlight, click };

		inline static const color passive_background = color(34, 115, 191);
		inline static const color highlight_background = color(42, 151, 252);
		inline static const color click_background = color(148, 202, 252);
		inline static const color text_color = color(255, 255, 255);
		inline static const color text_disabled_color = color(130, 130, 130);
		inline static const XPLMFontID font = xplmFont_Proportional;
		inline static const int corner_radius = 5;

		#if defined(WIN32)
		HCURSOR hand_cursor;
		#endif

		std::string _text;
		bool _enabled = true;
		
		int _text_left, _text_bottom;
		mouse_state _state = mouse_state::passive;

		virtual void draw(XPLMWindowID window_id, int window_left, int window_top) override {
			int left = window_left + _left;
			int right = left + _width;
			int top = window_top - _top;
			int bottom = top - _height;

			GLfloat col[4];
			if (_state == mouse_state::passive)
				passive_background.to_gl_color(col);
			if (_state == mouse_state::highlight)
				highlight_background.to_gl_color(col);
			if (_state == mouse_state::click)
				click_background.to_gl_color(col);
			glColor4fv(col);

			glBegin(GL_POLYGON);
			glVertex2i(left + corner_radius, top);
			glVertex2i(right - corner_radius, top);
			draw_arc_segment(right - corner_radius, top - corner_radius, corner_radius, pi / 2., 0);
			glVertex2i(right, top - corner_radius);
			glVertex2i(right, bottom + corner_radius);
			draw_arc_segment(right - corner_radius, bottom + corner_radius, corner_radius, 0, -pi / 2.);
			glVertex2i(right - corner_radius, bottom);
			glVertex2i(left + corner_radius, bottom);
			draw_arc_segment(left + corner_radius, bottom + corner_radius, corner_radius, 3. / 2. * pi, pi);
			glVertex2i(left, bottom + corner_radius);
			glVertex2i(left, top - corner_radius);
			draw_arc_segment(left + corner_radius, top - corner_radius, corner_radius, pi, pi / 4.);
			glEnd();

			auto tc = _enabled ? text_color : text_disabled_color;
			XPLMDrawString(
				const_cast<float *>(tc.rgb), 
				window_left + _text_left, window_top - _text_bottom,
				const_cast<char *>(_text.c_str()), nullptr, font);
		}

		virtual XPLMCursorStatus handle_cursor(XPLMWindowID window_id, int x, int y) override {			
			if (is_inside(x, y) && _enabled) {
				if (_state != mouse_state::click)
					_state = mouse_state::highlight;
				#if defined(WIN32) && defined(XPLM300)
				SetCursor(hand_cursor);
				return xplm_CursorCustom;
				#else
				return xplm_CursorDefault;
				#endif
			} else {
				_state = mouse_state::passive;
				return xplm_CursorDefault;
			}
		}

		virtual bool handle_mouse_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse) override {
			if (is_inside(x, y) && _enabled) {
				if (mouse == xplm_MouseDown) {
					_state = mouse_state::click;
				} else if (mouse == xplm_MouseUp && _state == mouse_state::click) {
					_state = mouse_state::highlight;
					if (on_clicked)
						on_clicked(shared_from_this());
				}
				return true;
			} else {
				_state = mouse_state::passive;
				return false;
			}
		}

		virtual void update_geometry() override {
			auto[text_width, text_height] = measure_string(_text, font);
			_text_left = _left + (_width - text_width) / 2;
			_text_bottom = _top + (_height + text_height) / 2;
		}
		
		void load_cursor() {
			#if defined(WIN32)
			hand_cursor = LoadCursor(NULL, IDC_HAND);
			#endif
		}

	public:

		button(const std::string &text, std::function<void(std::shared_ptr<button>)> on_clicked, 
			   int left, int top, int width=0, int height=0)
			: widget(left, top, 0, 0), _text(text), on_clicked(on_clicked) {

			auto[text_width, text_height] = measure_string(_text, font);
			if (width != 0)
				_width = width;
			else
				_width = text_width + 20;
			if (height != 0)
				_height = height;
			else
				_height = text_height + 15;

			load_cursor();
			update_geometry();
		}

		const std::string &text() const { return _text; }
		void set_text(const std::string &text) {
			_text = text;
			update_geometry();
		}		

		bool enabled() const { return _enabled; };
		void set_enabled(bool e) {
			_enabled = e;
		}

		std::function<void(std::shared_ptr<button>)> on_clicked;


	};

}