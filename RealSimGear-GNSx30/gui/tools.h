#pragma once
#include "stdafx.h"

namespace gui {

	const inline double pi = std::acos(-1);

	inline std::tuple<int, int> measure_string(const std::string &text, XPLMFontID font) {
		int height;
		XPLMGetFontDimensions(font, nullptr, &height, nullptr);
		int width =
			static_cast<int>(XPLMMeasureString(font, text.c_str(), static_cast<int>(text.length())));
		return { width, height };
	}


	inline static const double arc_phi_incr = 2.0 * pi / 100.0;
	inline void draw_arc_segment(int center_x, int center_y, double radius, double start_phi, double end_phi) {
		if (end_phi >= start_phi) {
			for (double phi = start_phi; phi < end_phi; phi += arc_phi_incr) {
				double x = (double)center_x + radius * std::cos(phi);
				double y = (double)center_y + radius * std::sin(phi);
				glVertex2d(x, y);
			}
		} else {
			for (double phi = start_phi; phi > end_phi; phi -= arc_phi_incr) {
				double x = (double)center_x + radius * std::cos(phi);
				double y = (double)center_y + radius * std::sin(phi);
				glVertex2d(x, y);
			}
		}
	}
}