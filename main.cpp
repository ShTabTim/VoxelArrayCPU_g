#include <Windows.h>
#include "FundLibs/sh_rend_cpu/sh_win.h"
#include "FundLibs/key/keys.h"
#include "FundLibs/math/vec2.h"
#include "FundLibs/math/vec3.h"

#define sign(a) (((a)>0)-((a)<0))

class MainRenderer : public sh_dwaw_win_cpu {
	const int MAX_RAY_STEPS = 50;
	double t = 0;
	float x_angle = 0;
	float y_angle = 0;
	vec3f pos;

	bool getVoxel(vec3i c) {
		return 8*cos(c.x*0.1f)*cos(c.z*0.1f)*cos(c.y*0.1f) - 8 > c.y*0.2f;
	}

	vec3f mainImage(float x_c, float y_c) {
		vec2f screenPos(x_c * 2 - 1, y_c * 2 - 1);
		vec3f rayDir(screenPos.x, screenPos.y, 0.8f);
		vec3f rayPos = pos;// (0.0f, 2.0f * cos(t), -12.0f);

		{
			float s = sin(y_angle), c = cos(y_angle);
			float r = rayDir.y;
			rayDir.y = r * c + rayDir.z * s;
			rayDir.z = rayDir.z * c - r * s;
			s = sin(x_angle), c = cos(x_angle);
			r = rayDir.x;
			rayDir.x = r * c - rayDir.z * s;
			rayDir.z = rayDir.z * c + r * s;

		}

		vec3i mapPos((int32_t)rayPos.x, (int32_t)rayPos.y, (int32_t)rayPos.z);

		vec3f deltaDist = rayDir.get_norm(1);
		deltaDist.x = abs(1.0f/deltaDist.x);
		deltaDist.y = abs(1.0f/deltaDist.y);
		deltaDist.z = abs(1.0f/deltaDist.z);

		vec3i8 rayStep((int32_t)sign(rayDir.x), (int32_t)sign(rayDir.y), (int32_t)sign(rayDir.z));

		vec3f sideDist = ((vec3f(mapPos) - rayPos + vec3f(0.5f, 0.5f, 0.5f)) * rayStep + vec3f(0.5f, 0.5f, 0.5f)) * deltaDist;

		vec3<bool> mask;

		for (uint16_t i(MAX_RAY_STEPS); i--;) {
			if (getVoxel(mapPos)) {
				vec3f color(1, 1, 1);
				if (mask.x) {
					color *= vec3f(0.5f, 0.5f, 0.5f);
				}
				if (mask.y) {
					color *= vec3f(1, 1, 1);
				}
				if (mask.z) {
					color *= vec3f(0.75f, 0.75f, 0.75f);
				}
				return color;
			}
			if (sideDist.x < sideDist.y) {
				if (sideDist.x < sideDist.z) {
					sideDist.x += deltaDist.x;
					mapPos.x += rayStep.x;
					mask = vec3<bool>(true, false, false);
				} else {
					sideDist.z += deltaDist.z;
					mapPos.z += rayStep.z;
					mask = vec3<bool>(false, false, true);
				}
			} else {
				if (sideDist.y < sideDist.z) {
					sideDist.y += deltaDist.y;
					mapPos.y += rayStep.y;
					mask = vec3<bool>(false, true, false);
				} else {
					sideDist.z += deltaDist.z;
					mapPos.z += rayStep.z;
					mask = vec3<bool>(false, false, true);
				}
			}
		}

		vec3f color(0.4f, 0.6f, 0.8f);
		return color;
	}
    bool sh_init() {
        AppName = L"CPURenderer";
        return 1;
    }
	vec3f vel;
    bool sh_loop(double dt) {
        key_loop(get_hwnd());

        if (get_key(VK_ESCAPE).held)
            return 0;
		
		vel.z = get_key('W').held - get_key('S').held;
		vel.x = get_key('D').held - get_key('A').held;
		vel.y = get_key(VK_SPACE).held - get_key(VK_SHIFT).held;
		vel.norm(dt*10.0f);
		x_angle += (get_key(VK_LEFT).held - get_key(VK_RIGHT).held)*dt*5.0f;
		y_angle += (get_key(VK_UP).held - get_key(VK_DOWN).held)*dt*5.0f;


		pos.x += vel.x * cos(x_angle) - vel.z * sin(x_angle);
		pos.z += vel.z * cos(x_angle) + vel.x * sin(x_angle);
		pos.y += vel.y;
        t += dt;

		for(uint16_t i(get_dr_w()); i--;)
			for (uint16_t j(get_dr_h()); j--;) {
				vec3f col = mainImage(i / (float)get_dr_w(), j / (float)get_dr_h());
				col *= 255;
				draw_pix(i, j, col.x, col.y, col.z);
			}

        return 1;
    }
    bool sh_finit() {
        return 1;
    }
};

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR sCmdLine, int iShow) {
	MainRenderer simulation;
	if (simulation.init(hInst, 128, 128, 128, 128))
		simulation.run();
	return 0;
}