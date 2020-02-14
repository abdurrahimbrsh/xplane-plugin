#pragma once
#include "stdafx.h"


namespace gui {

	struct color {
		float rgb[3];

		color() {
			rgb[0] = 1.0;
			rgb[1] = 1.0;
			rgb[2] = 1.0;
		}

		color(float r, float g, float b) {
			rgb[0] = r;
			rgb[1] = g;
			rgb[2] = b;
		}

		color(int r, int g, int b) {
			rgb[0] = r / 255.0f;
			rgb[1] = g / 255.0f;
			rgb[2] = b / 255.0f;
		}

		float &r() { return rgb[0]; }
		float &g() { return rgb[1]; }
		float &b() { return rgb[2]; }

		const float &r() const { return rgb[0]; }  
		const float &g() const { return rgb[1]; }  
		const float &b() const { return rgb[2]; } 

		void to_gl_color(GLfloat *c) const {
			c[0] = rgb[0];
			c[1] = rgb[1];
			c[2] = rgb[2];
			c[3] = 1.0f;
		}
	};

}
