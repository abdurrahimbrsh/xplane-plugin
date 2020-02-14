#pragma once
#include "stdafx.h"
#include "widget.h"
#if !defined(XPLM300)
#include "circle_button.h"
#endif 


namespace gui {

	// callback wrappers
	namespace callback {
		static void draw_window(XPLMWindowID window_id, void *refcon);
		static XPLMCursorStatus handle_cursor(XPLMWindowID window_id, int x, int y, void *refcon);
		static int handle_mouse_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse, void *refcon);
		static int handle_right_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse, void *refcon);
		static int handle_mouse_wheel(XPLMWindowID window_id, int x, int y, int wheel, int clicks, void *refcon);
		static void handle_key(XPLMWindowID window_id, char key, XPLMKeyFlags flags, char virtual_key, void *refcon, int losing_focus);
	}

	class window {
		friend void callback::draw_window(XPLMWindowID window_id, void *refcon);
		friend XPLMCursorStatus callback::handle_cursor(XPLMWindowID window_id, int x, int y, void *refcon);
		friend int callback::handle_mouse_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse, void *refcon);
		friend int callback::handle_right_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse, void *refcon);
		friend int callback::handle_mouse_wheel(XPLMWindowID window_id, int x, int y, int wheel, int clicks, void *refcon);
		friend void callback::handle_key(XPLMWindowID window_id, char key, XPLMKeyFlags flags, char virtual_key, void *refcon, int losing_focus);

	private:
		XPLMWindowID _id;
		std::list<std::shared_ptr<widget>> _widgets;
		int _width, _height;

		std::string _title;
		int _title_width, _title_height;
		inline static const XPLMFontID title_font = xplmFont_Proportional;

#if !defined(XPLM300)
		inline static const color bg_color = color(38, 50, 64);
		inline static const color title_color = color(20, 28, 38);
		inline static const color title_text_color = color(113, 120, 128);
		inline static const color close_color = color(255, 96, 89);
		inline static const int corner_radius = 5;
		inline static const int border = 8;
		inline static const int title_bar = 25;
		inline static const int title_bar_space = 5;
		inline static const int close_diameter = 12;
		std::shared_ptr<circle_button> _close_button;

		bool moving = false;
		int move_start_mouse_x, move_start_mouse_y;
		int move_start_window_left, move_start_window_top;
#endif

		void draw_window(XPLMWindowID window_id) {
			int window_left, window_top, window_right, window_bottom;
			XPLMGetWindowGeometry(_id, &window_left, &window_top, &window_right, &window_bottom);

			#if !defined(XPLM300)
			GLfloat col[4];
			window_top -= title_bar + title_bar_space;

			// window contents
			bg_color.to_gl_color(col);
			glColor4fv(col);		
			glBegin(GL_POLYGON);
			glVertex2i(window_left - border, window_top + title_bar_space);
			glVertex2i(window_right + border, window_top + title_bar_space);
			glVertex2i(window_right + border, window_bottom - border + corner_radius);
			draw_arc_segment(window_right + border - corner_radius, window_bottom - border + corner_radius, corner_radius, 0, -pi / 2.);
			glVertex2i(window_right + border - corner_radius, window_bottom - border);
			glVertex2i(window_left - border + corner_radius, window_bottom - border);
			draw_arc_segment(window_left - border + corner_radius, window_bottom - border + corner_radius, corner_radius, 3. / 2. * pi, pi);
			glVertex2i(window_left - border, window_bottom - border + corner_radius);
			glVertex2i(window_left - border, window_top + title_bar_space);
			glEnd();

			// title bar
			title_color.to_gl_color(col);
			glColor4fv(col);
			glBegin(GL_POLYGON);
			glVertex2i(window_left - border + corner_radius, window_top + title_bar_space + title_bar);
			glVertex2i(window_right + border - corner_radius, window_top + title_bar_space + title_bar);
			draw_arc_segment(window_right + border - corner_radius, window_top + title_bar_space + title_bar - corner_radius, corner_radius, pi / 2., 0);
			glVertex2i(window_right + border, window_top + title_bar_space + title_bar - corner_radius);
			glVertex2i(window_right + border, window_top + title_bar_space);
			glVertex2i(window_left - border, window_top + title_bar_space);
			glVertex2i(window_left - border, window_top + title_bar_space + title_bar - corner_radius);
			draw_arc_segment(window_left - border + corner_radius, window_top + title_bar_space + title_bar - corner_radius, corner_radius, pi, pi / 4.);
			glEnd();

			// title text
			int title_left = window_left + (window_right - window_left - _title_width) / 2;
			int title_bottom = window_top + title_bar_space + (title_bar - _title_height) / 2 + 2;
			XPLMDrawString(
				const_cast<float *>(title_text_color.rgb),
				title_left, title_bottom,
				const_cast<char *>(_title.c_str()), nullptr, title_font);
			#endif

			for (auto &w : _widgets) {
				w->draw(_id, window_left, window_top);
			}
		}

		XPLMCursorStatus handle_cursor(XPLMWindowID window_id, int x, int y) {
			int window_left, window_top, window_right, window_bottom;
			XPLMGetWindowGeometry(_id, &window_left, &window_top, &window_right, &window_bottom);

			#if !defined(XPLM300)
			// adjust for title bar
			window_top -= title_bar + title_bar_space;

			// handle dragging window
			if (moving)
				return xplm_CursorDefault;
			#endif

			XPLMCursorStatus status = xplm_CursorDefault;
			for (auto w = _widgets.rbegin(); w != _widgets.rend(); w++) {
				auto widget_status = (*w)->handle_cursor(window_id, x - window_left, window_top - y);
				if (status == xplm_CursorDefault)
					status = widget_status;
			}

			return status;
		}

		int handle_mouse_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse) {
			int window_left, window_top, window_right, window_bottom;
			XPLMGetWindowGeometry(_id, &window_left, &window_top, &window_right, &window_bottom);

			#if !defined(XPLM300)
			// adjust for title bar
			window_top -= title_bar + title_bar_space;
			#endif

			XPLMCursorStatus status = xplm_CursorDefault;
			bool handled = false;
			for (auto w = _widgets.rbegin(); w != _widgets.rend(); w++) {
				handled = (*w)->handle_mouse_click(window_id, x - window_left, window_top - y, mouse);
				if (handled)
					break;
			}

			#if !defined(XPLM300)
			// handle dragging window
			if (mouse == xplm_MouseDown && window_top - y < 0 && !moving && !handled) {
				logMessage("Window move start");
				moving = true;
				move_start_mouse_x = x;
				move_start_mouse_y = y;
				move_start_window_left = window_left;
				move_start_window_top = window_top;
				return 1;
			}
			if (mouse == xplm_MouseDrag && moving) {
				int left = move_start_window_left + (x - move_start_mouse_x);
				int top = move_start_window_top + (y - move_start_mouse_y);
				move(left, top);
				return 1;
			}
			if (mouse == xplm_MouseUp && moving) {
				logMessage("Window move end");
				moving = false;
				return 1;
			}
			#endif

			return 1;
		}

		int handle_right_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse) {
			return 1;
		}

		int handle_mouse_wheel(XPLMWindowID window_id, int x, int y, int wheel, int clicks) {
			return 1;
		}

		void handle_key(XPLMWindowID window_id, char key, XPLMKeyFlags flags, 
			            char virtual_key, int losing_focus) {

		}
		

	public:
		window(const std::string &title, int left, int top, int width, int height) 
				: _width(width), _height(height) {

			int screen_left, screen_top, screen_right, screen_bottom;
			#if defined(XPLM300)			
			XPLMGetScreenBoundsGlobal(&screen_left, &screen_top, &screen_right, &screen_bottom);
			#else
			screen_left = 0;
			screen_bottom = 0;
			XPLMGetScreenSize(&screen_right, &screen_top);
			#endif

			XPLMCreateWindow_t cw;
			cw.structSize = sizeof(cw);
			cw.left = screen_left + left;
			cw.top = screen_top - top;
			cw.right = cw.left + width;
			cw.bottom = cw.top - height;
			#if defined(XPLM300)			
			cw.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
			cw.layer = xplm_WindowLayerFloatingWindows;
			#else
			cw.top += title_bar + title_bar_space;
			#endif
			cw.visible = 0;
			cw.drawWindowFunc = &callback::draw_window;
			cw.handleMouseClickFunc = &callback::handle_mouse_click;
			#if defined(XPLM300)			
			cw.handleRightClickFunc = &callback::handle_right_click;
			#endif			
			cw.handleKeyFunc = &callback::handle_key;
			cw.handleCursorFunc = &callback::handle_cursor;
			cw.handleMouseWheelFunc = &callback::handle_mouse_wheel;
			cw.refcon = this;
			_id = XPLMCreateWindowEx(&cw);

			#if defined(XPLM300)			
			XPLMSetWindowPositioningMode(_id, xplm_WindowPositionFree, -1);
			XPLMSetWindowGravity(_id, 0, 1, 0, 1);
			XPLMSetWindowResizingLimits(_id, width, height, width, height);
			#else
			_close_button = std::make_shared<circle_button>(
				close_color,
				[this](auto evt) {this->set_visible(false); },
				3, -title_bar + 2, close_diameter);
			add_widget(std::static_pointer_cast<gui::widget>(_close_button));
			#endif			

			set_title(title);
		}	

		~window() {
			XPLMDestroyWindow(_id);
		}

		bool visible() const { return XPLMGetWindowIsVisible(_id); }
		void set_visible(bool v) {
			XPLMSetWindowIsVisible(_id, v);
			if (v) {
				XPLMBringWindowToFront(_id);
				#if defined(XPLM300)					
				XPLMSetWindowPositioningMode(_id, xplm_WindowPositionFree, -1);
				XPLMSetWindowGravity(_id, 0, 1, 0, 1);
				#endif				
			}
		}

		void set_title(const std::string &title) { 
			_title = title;

			auto[text_width, text_height] = measure_string(_title, title_font);
			_title_width = text_width;
			_title_height = text_height;

			#if defined(XPLM300)					
			XPLMSetWindowTitle(_id, _title.c_str()); 
			#endif			
		}

		bool popped_out() const { 
			#if defined(XPLM300)					
			return XPLMWindowIsPoppedOut(_id); 
			#else
			return false;
			#endif			
		}

		void add_widget(std::shared_ptr<widget> w) {
			_widgets.push_back(w);
		}

		void remove_widget(std::shared_ptr<widget> w) {
			_widgets.remove(w);
		}

		void move(int left, int top) {
			#if defined(XPLM300)			
			XPLMSetWindowGeometry(_id, left, top, left + _width, top - _height);
			#else
			XPLMSetWindowGeometry(_id, left, top + (title_bar + title_bar_space), left + _width, top - _height);
			#endif
		}

	};



	namespace callback {
		static void draw_window(XPLMWindowID window_id, void *refcon) {
			static_cast<window *>(refcon)->draw_window(window_id);
		}

		static int handle_mouse_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse, void *refcon) {
			return static_cast<window *>(refcon)->handle_mouse_click(window_id, x, y, mouse);
		}

		static int handle_right_click(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse, void *refcon) {
			return static_cast<window *>(refcon)->handle_right_click(window_id, x, y, mouse);
		}

		static void handle_key(XPLMWindowID window_id, char key, XPLMKeyFlags flags, char virtual_key, void *refcon, int losing_focus) {
			static_cast<window *>(refcon)->handle_key(window_id, key, flags, virtual_key, losing_focus);
		}

		static XPLMCursorStatus handle_cursor(XPLMWindowID window_id, int x, int y, void *refcon) {
			return static_cast<window *>(refcon)->handle_cursor(window_id, x, y);
		}

		static int handle_mouse_wheel(XPLMWindowID window_id, int x, int y, int wheel, int clicks, void *refcon) {
			return static_cast<window *>(refcon)->handle_mouse_wheel(window_id, x, y, wheel, clicks);
		}
	}

}

