#include "include/FactoryWin.h"

//using namespace std::chrono_literals;

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int snCmdShows) {

	FactoryWin factory("MainWin");
	factory.addWin("AddWin");
	HWND hwnd = factory.getMainHWND();
	
	while (true) {
		if (factory.isEvent()) {//must isEvent for check event;
			if (factory.event.state.type == Event::Type::KeyboardDown) {
				if ((factory.event.state.key == Event::Keyboard::F11)) {
					factory.switchFullScreen();
				}
			}
			if (factory.event.state.type == Event::Type::MousePressed) {
				if (factory.event.state.mouse == Event::Mouse::RightButton) {
					MessageBox(factory.event.hwnd, "Right", "Right", MB_TOPMOST);
				}
			}
			if (factory.event.state.type == Event::Type::Drop) {
				MessageBox(factory.event.hwnd, factory.event.state.system.c_str(),
					"Drop", MB_TOPMOST);
			}
			/*if (factory.event.mouse.pos.contains(500,500,500,500)) {
				 MessageBox(factory.event.hwnd, "Contains",
					 "Contains", MB_TOPMOST);
			 }*/

			if (factory.event.state.type == Event::Type::Close) {
				break;
			}
		}
		//std::this_thread::sleep_for(1ms);//payload
	}
	
	Logger_Info("ate and asleep");

	factory.Destroy();

	return 0;
}
