#include <3ds.h>
#include <citro2d.h>
#include "menus/components/ui_element.h"
#include "menus/components/ui_screen.h"
#include "math_helpers.h"

volatile bool start_level = false;

void action_hi(void* data) { start_level = true; };

UIAction actions[] = {
    {"hibykrmal", action_hi}
};

void level_select_loop() {
	UIScreen screen;
	C3D_RenderTarget* bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	ui_load_screen(&screen, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/level_select.txt");

	bool prev_checked = false;

	while (aptMainLoop()) {
		hidScanInput();
        
		if (start_level) break;

		touchPosition touch;
		hidTouchRead(&touch);
		ui_screen_update(&screen, &touch);

		UIElement *checkbox = get_element_by_tag(&screen, "checky");
		if (checkbox) {
			bool checked = checkbox->checkbox.checked;
			if (checked != prev_checked) {
				printf("%d\n", checkbox->checkbox.checked);
				prev_checked = checked;
			}
		}

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(bot, C2D_Color32(3, 177, 255, 255));
        C2D_SceneBegin(bot);

		ui_screen_draw(&screen);
		
		C3D_FrameEnd(0);
	}
}