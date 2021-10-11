#include "../include/FactoryWin.h"

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

std::deque<myMSG> FactoryWin::queue_msg;/*recieve msg for window*/
static std::string GetLastErrorStr() {
	DWORD error = GetLastError();
	if (error)
	{
		LPVOID lp_msg_buf;
		DWORD buf_len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lp_msg_buf,
			0, NULL);
		if (buf_len)
		{
			LPCSTR lp_msg_str = (LPCSTR)lp_msg_buf;
			std::string result(lp_msg_str, lp_msg_str + buf_len);
			LocalFree(lp_msg_buf);
			return result;
		}
	}
	return "";
}
/*-------------Public-----------*/
/*--------------Init------------*/
/*Fisrt-Main Step Init FactorWin*/
FactoryWin::FactoryWin(std::string title, std::pair<int, int>Size,
					   std::pair<int, int>Pos, bool fls ,
					   cmdParam other, StyleWin style):
	Title(title),sizeWin(Size), posWin(Pos), fullScreen(fls),
	Other(other),winStyle(style)  {
	
	InitLogger();
	if (InitWinClass()) {//if init windows class -> TODO	
		if (!addWin(Title))
			Logger_Error("Error");
	}
}

/*Pre-Second Step*/
void FactoryWin::InitLogger() {
	CppLogger::registerTarget(
		new FileLoggerTarget("./FactoryWin.log", LOG_LEVEL_DEBUG));
	Logger_Debug("StartLog");
}
/*Second step Init default WinClass*/
bool FactoryWin::InitWinClass() {
	if (winStyle == StyleWin::SimpleWin) {
		if (FindWindowA(WinClass.c_str(), NULL) == NULL) {
			ZeroMemory(&wc, sizeof(WNDCLASSEX));
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.style = CS_HREDRAW | CS_VREDRAW | DS_CENTER | CS_CLASSDC;
			wc.lpfnWndProc = WndProc;
			wc.hInstance = Other.hInstance;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.lpszClassName = WinClass.c_str();
			if (!RegisterClassEx(&wc)) {
				Logger_Critical("Register Class "+ WinClass+" is failed!");
				return false;
			}
			//wnd.hIcon = ::LoadIconW(hInstance, (LPCWSTR)IDI_ICON1);
			//wnd.hIconSm = wnd.hIcon;
			Logger_Debug("InitWinClass OK");
			return true;
		}
	}
	Logger_Warn("Wrong StyleWin");
	return false;
}
/*pre-third*/
bool FactoryWin::addWin(std::string title, std::pair<int, int>Size) {
	if (!title.empty()) {
		if (map.size() == 0) {
			if (!InitWin(std::stringstream(title))) {
				Logger_Critical("InitWin"+title+" is failed!");
				return false;
			}
			Logger_Debug("Init Win - " + title + " is OK");
			return true;
		}
		else {//think
			sizeWin = Size;
			if (!InitWin(std::stringstream(title))) {
				Logger_Critical("InitWin" + title + " is failed!");
				return false;
			}
			Logger_Debug("Init Win - " + title + " is OK");
			return true;
		}
	}
	Logger_Warn("Title is empty");
	return false;
}
bool FactoryWin::isEvent() {
	if (event.state.type != Event::Type::Close) {
		writeAccess = true;
		HandleMsg();
	}
	return true;
}
/*third*/
bool FactoryWin::InitWin(std::stringstream title) {//
	if (!title.str().empty()) {//Need Title for Window
		RECT wr = {0,0,posWin.first,posWin.second};
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
		HWND hwnd = CreateWindowEx(0,
			WinClass.c_str(),
			title.str().c_str(),
			WS_OVERLAPPEDWINDOW,
			posWin.first,//x
			posWin.second,//y
			sizeWin.first,//width
			sizeWin.second,//height
			nullptr,
			nullptr,
			Other.hInstance,//nullptr or
			nullptr);
		DragAcceptFiles(hwnd, true);
		map.push_back({title.str(),hwnd});//add newWindows
		SetMiddle(hwnd);//set middle display
		ShowWindow(hwnd, Other.cmdShows);//2=10->SHOW
		UpdateWindow(hwnd);//send WM_PAINT
		return true;
	}
	return false;
}
/*Destroy*/
void FactoryWin::Destroy() {
	if (!map.empty()) {
		for (auto& del : map)
		{
			if (del.second != nullptr) {
				ShowWindow(del.second, SW_HIDE);
				DestroyWindow(del.second);
				del.second = nullptr;
			}
		}
		map.clear();
	}
	UnregisterClassA(WinClass.c_str(), Other.hInstance);
	event = {};
	wc = {};
	for (auto& del: queue_msg)
	{
		del.hwnd = nullptr;
	}
	queue_msg.clear();
}
/*TODO*/
void FactoryWin::setCmdParam(cmdParam other) {
	Other=other;
}
/*------------------------------*/
/*--------------Get-------------*/
size_t FactoryWin::cntWindow() {
	return map.size();
}
HWND FactoryWin::getHWND(std::string title) {
	auto it = std::find_if(map.begin(), map.end(), [title](std::pair<std::string, HWND> p) {
		return title == p.first;
		});
	if (it != map.end())
		return it->second;
	Logger_Warn("Can't getHWND - may be wrong title - "+title);
	return nullptr;
}
HWND FactoryWin::getMainHWND() {
	return (map.size() > 0) ? map.begin()->second : nullptr;
}
std::string FactoryWin::getTitle(HWND hwnd) {
	for (const auto& it : map) {
		if (it.second == hwnd) {
			return it.first;
		}
	}
	Logger_Debug("Not found tilte for HWND = " + hwndToString(hwnd));
	return std::string();
}
std::pair<int, int> FactoryWin::getMousePos() {
	return { event.mouse.pos.x,event.mouse.pos.y };
}
std::pair<int, int> FactoryWin::getMouseWheel() {
	return{ event.mouse.wheel.key,event.mouse.wheel.delta };;
}
std::string FactoryWin::getPathDropFiles() {
	if (!pathDrop.empty())
		return pathDrop;
	return {};
}
std::pair<size_t, std::string> FactoryWin::dumpQueue() {
	std::string dump;
	for (auto it = queue_msg.begin(); it != queue_msg.end(); ++it) {
		dump += std::to_string(it->msg) + " " +
			std::to_string(it->wp) + " " +
			std::to_string(it->lp) + "\n";
	}
	return { queue_msg.size(),dump };
}
/*------------------------------*/
/*--------------Set-------------*/
bool FactoryWin::setTitle(std::string currentTitle, std::string newTitle) {
	auto it = std::find_if(map.begin(), map.end(), [currentTitle](std::pair<std::string, HWND> p) {
		return currentTitle == p.first;
		});
	if (it != map.end()) {
		if (SetWindowTextA(it->second, newTitle.c_str())) {
			it->first = newTitle;
			Logger_Debug("ok setTitle " + newTitle);
			return true;
		}else {
			Logger_Warn("No found win"	+ currentTitle );
			return false;
		}
	}
	Logger_Warn("Can't setTitle not found Win with Title = "+currentTitle);
	return false;
}
bool FactoryWin::setTitle(HWND hwnd, std::string newTitle) {
	return setTitle(getTitle(hwnd), newTitle);
}
bool FactoryWin::setSize(HWND hwnd,int w, int h) {
	std::string tmp(hwndToString(hwnd));
	if (SetWindowPos(hwnd, HWND_TOP, posWin.first,
		posWin.second, w, h, SWP_NOMOVE)) {
		sizeWin = { w,h };
		Logger_Debug("ok setSize for HWND " + tmp);
		return true;
	}
	Logger_Warn("Fault set_New size for HWND " + tmp);
	return false;
}
bool FactoryWin::switchFullScreen() {
	fullScreen = !fullScreen;
	return (fullScreen) ? enterFullscreen(): exitFullscreen();
}
/*----------TODO----------------*/
void FactoryWin::ChangeStyleWindow(StyleWin style) {}
/*------------------------------*/
/*------------Private------------------------------------------*/
/*--------------Get-------------*/
std::string FactoryWin::hwndToString(const HWND hwnd) {
	std::string tmp;
	int len = GetWindowTextLength(hwnd);
	if (len > 0) {
		tmp.resize(len + 1);
		len = GetWindowText(hwnd, &tmp[0], tmp.size());
		tmp.resize(len);
	}
	return tmp;
}
/*------------------------------*/
/*--------------Set-------------*/
/*InitWin call this func*/
void FactoryWin::SetMiddle(HWND hwnd) {
	HDC dc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	INT px = GetDeviceCaps(dc, HORZRES);
	INT py = GetDeviceCaps(dc, VERTRES);
	DeleteDC(dc);//GetWindowRect(hwnd, &rec);
	RECT rec{0,0,sizeWin.first,sizeWin.second};
	INT dx = rec.right - rec.left;
	INT dy = rec.bottom - rec.top;
	MoveWindow(hwnd, (px - dx) / 2, (py - dy) / 2, rec.right, rec.bottom, TRUE);
}
/*swtichFullscreen call these func*/
bool FactoryWin::enterFullscreen() {
	HDC hdc= GetDC(event.hwnd);
	DEVMODE fullscreenSettings;
	bool isChangeSuccessful;
	int fullscreenWidth = GetDeviceCaps(hdc, HORZRES);
	int fullscreenHeight = GetDeviceCaps(hdc, VERTRES);
	int colourBits = GetDeviceCaps(hdc, BITSPIXEL);
	int refreshRate = GetDeviceCaps(hdc, VREFRESH);
	HWND hwnd = event.hwnd;
	EnumDisplaySettings(NULL, 0, &fullscreenSettings);
	fullscreenSettings.dmPelsWidth = fullscreenWidth;
	fullscreenSettings.dmPelsHeight = fullscreenHeight;
	fullscreenSettings.dmBitsPerPel = colourBits;
	fullscreenSettings.dmDisplayFrequency = refreshRate;
	fullscreenSettings.dmFields = DM_PELSWIDTH |
		DM_PELSHEIGHT |
		DM_BITSPERPEL |
		DM_DISPLAYFREQUENCY;
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, fullscreenWidth, fullscreenHeight, SWP_SHOWWINDOW);
	isChangeSuccessful = ChangeDisplaySettings(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
	ShowWindow(hwnd, SW_MAXIMIZE);
	return isChangeSuccessful;
}
bool FactoryWin::exitFullscreen() {
	HWND hwnd = event.hwnd;
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_LEFT);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	bool ChangeSuccessful = ChangeDisplaySettings(NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
	//SetWindowPos(hwnd, HWND_TOP, 0, 0, 800, 600, SW_HIDE);//SWP_NOMOVE
	SetMiddle(hwnd);
	ShowWindow(hwnd, SW_SHOW);//SW_RESTORE
	return ChangeSuccessful;
}
/*------------------------------*/
/*------Kernel------------------*/
void FactoryWin::HandleMsg() {//
	if (!IsQuit()) {
		for (const auto& it : map) {
			MSG msg;
			while ((::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) != NULL) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);	
				if (IsQuit()) break;
				if (writeAccess) {
					event = {};
					isKey();
					MouseEvent();//down up or wheel
					DragAndDrop();
					writeAccess = false;
				}
				MouseMove();
				ReSize();
			}
		}
	} 
}
/*------------------------------*/
/*--------------Queue-----------*/
bool FactoryWin::IsQuit() {
	if (!queue_msg.empty()) {
		auto it = std::find_if(queue_msg.cbegin(), queue_msg.cend(), [](const myMSG& msg) {
			return (msg.msg == WM_CLOSE || msg.msg == WM_DESTROY || msg.msg == WM_QUIT);
			});
			if (it != queue_msg.end()) {
				if (it->hwnd == getMainHWND()) {
					event = {};
					event.state.type = Event::Type::Close;
					Logger_Debug("Close MainWin");
					if (!delWin(it->hwnd))//might crash
						Logger_Error("Can't delete hwnd from map");
					queue_msg.erase(it);
					return true;
				}
				else {
					if (!delWin(it->hwnd))
						Logger_Error("Can't delete hwnd from map");
					queue_msg.erase(it);
					return false;
				}
			}
	}
	return false;
}
bool FactoryWin::delWin(HWND hwnd) {
	auto it = std::find_if(map.cbegin(), map.cend(), [=](std::pair<std::string, HWND> it) {
		return (it.second == hwnd);
		});
	if (it != map.end()) {
		ShowWindow(it->second, SW_HIDE);
		DestroyWindow(it->second);
		map.erase(it);
		return true;
	}
	return false;
}
void FactoryWin::MouseMove() {
	if (!queue_msg.empty()) {
		auto it = std::find_if(queue_msg.cbegin(), queue_msg.cend(), [](const myMSG& msg) {
			return (msg.msg == WM_MOUSEMOVE);
			});
		if (it != queue_msg.end()) {
			event.mouse.pos = { GET_X_LPARAM(it->lp),GET_Y_LPARAM(it->lp)};
			queue_msg.erase(it);
		}
	}
}
void FactoryWin::MouseEvent() {
	if (!queue_msg.empty()) {
		auto it = std::find_if(queue_msg.cbegin(), queue_msg.cend(), [](const myMSG& msg) {
			return (msg.msg == WM_RBUTTONDOWN || msg.msg == WM_LBUTTONDOWN||
					msg.msg == WM_RBUTTONUP || msg.msg == WM_LBUTTONUP||
					msg.msg == WM_MOUSEWHEEL);
		});
		if (it != queue_msg.end()) {		
			if (it->msg == WM_RBUTTONDOWN || it->msg == WM_LBUTTONDOWN) {
				event.state.type = Event::Type::MousePressed;//down
				(it->msg == WM_RBUTTONDOWN) ?
					event.state.mouse = Event::Mouse::RightButton:
					event.state.mouse = Event::Mouse::LeftButton;
			}
			else if (it->msg == WM_RBUTTONUP || it->msg == WM_LBUTTONUP) {
				event.state.type = Event::Type::MouseReleased;
				(it->msg == WM_RBUTTONUP) ?
					event.state.mouse = Event::Mouse::RightButton :
					event.state.mouse = Event::Mouse::LeftButton;
			}
			if (it->msg == WM_MOUSEWHEEL) {
				event.state.type = Event::Type::MouseWheel;
				WPARAM fwKeys = GET_KEYSTATE_WPARAM(it->wp);
				WPARAM zDelta = GET_WHEEL_DELTA_WPARAM(it->wp);
				event.mouse.wheel = {static_cast<int>(fwKeys) ,
									 static_cast<int>(zDelta) };
			}
			queue_msg.erase(it);
		}
	}
}
void FactoryWin::isKey() {
	if (!queue_msg.empty()) {
		auto it = std::find_if(queue_msg.cbegin(), queue_msg.cend(), [](const myMSG& msg) {
			return (msg.msg == WM_KEYDOWN || msg.msg == WM_KEYUP);//vk a 0x41;
			});
		if (it != queue_msg.end()) {
			event.hwnd = it->hwnd;
			if (it->msg == WM_KEYDOWN)
				event.state.type = Event::Type::KeyboardDown;//down
			else if (it->msg == WM_KEYUP)
				event.state.type = Event::Type::KeyboardUp;//up
			event.state.key = it->wp;
			queue_msg.erase(it);
		}
	}
}
/*DragAndDrop call this func*/
bool FactoryWin::tryDrop (WPARAM wp) {
	HDROP drop = (HDROP)wp;
	UINT filePathLength = DragQueryFileW(drop, 0xFFFFFFFF, 0, 512);
	wchar_t* fileName = nullptr;
	UINT longestFileNameLength = 0;
	pathDrop.clear();
	for (UINT i = 0; i < filePathLength; ++i)
	{
		UINT fileNameLength = DragQueryFileW(drop, i, 0, 512) + 1;
		if (fileNameLength > longestFileNameLength)
		{
			longestFileNameLength = fileNameLength;
			void* tmp = realloc(fileName, longestFileNameLength * sizeof(*fileName));
			if(tmp!=nullptr)//realloc(fileName, longestFileNameLength * sizeof(*fileName))
				fileName = reinterpret_cast<wchar_t*>(tmp);
		}
		DragQueryFileW(drop, i, fileName, fileNameLength);
		if (fileName != nullptr) {
			std::wstring tmp = fileName;
			pathDrop += std::string(tmp.begin(), tmp.end());
			pathDrop += "\n";
			//pathDrop += std::string(fileName, fileName + fileNameLength)+"\n";
		}
	}
	free(fileName);
	DragFinish(drop);
	return (pathDrop.size()) ? true : false;
}
void FactoryWin::DragAndDrop() {
	if (!queue_msg.empty()) {
		auto it = std::find_if(queue_msg.cbegin(), queue_msg.cend(), [](const myMSG& msg) {
			return (msg.msg == WM_DROPFILES);
			});
		if (it != queue_msg.end()) {
			if (tryDrop(it->wp)) {
				event.hwnd = it->hwnd;
				event.state.type = Event::Type::Drop;
				event.state.system = pathDrop;
			}
			queue_msg.erase(it);
		}
	}
}
void FactoryWin::ReSize() {
	if (!queue_msg.empty()) {
		auto it = std::find_if(queue_msg.cbegin(), queue_msg.cend(), [](const myMSG& msg) {
			return (msg.msg == WM_SIZE);
			});
		if (it != queue_msg.end()) {
			sizeWin = { LOWORD(it->lp),HIWORD(it->lp) };
			queue_msg.erase(it);		
		}
	}
}
void FactoryWin::RePaint() {
	if (!queue_msg.empty()) {
		auto it = std::find_if(queue_msg.cbegin(), queue_msg.cend(), [](const myMSG& msg) {//
			return (msg.msg == WM_PAINT);
			});
		if (it != queue_msg.end()) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(it->hwnd, &ps);
			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
			EndPaint(it->hwnd, &ps);
			std::string err = GetLastErrorStr();
			if (!err.empty())
				Logger_Error("Error mpaint() = " + err);
			queue_msg.erase(it);
		}
	}
}
/*TODO*/
void FactoryWin::SysCommand() {
	if (!queue_msg.empty()) {
		auto it = std::find_if(queue_msg.cbegin(), queue_msg.cend(), [](const myMSG& msg) {//
			return (msg.msg == WM_SYSCOMMAND);
			});
		if (it != queue_msg.end()) {
			queue_msg.erase(it);
		}
	}
}
/*------------------------------*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
/*------------------------------*/
LRESULT FactoryWin::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){	
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam)) {
		return true;
	}
	myMSG _msg{ hwnd,0,wParam,lParam};
	switch (message) {
	case WM_CREATE:
	{
		bool isRun = true;
		break;
	}
	case WM_DESTROY: {
		_msg << WM_DESTROY;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_QUIT: {
		_msg << WM_QUIT;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_CLOSE: {
		//exit(0); crash
		_msg << WM_CLOSE;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_SIZE: {
		_msg << WM_SIZE;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_PAINT: {
		_msg << WM_PAINT;
		queue_msg.push_back(_msg);
		RePaint();/*paint*/
		break;
	}
	case WM_DROPFILES: {
		_msg << WM_DROPFILES;
		queue_msg.push_back(_msg);
		break;
	}//TODO---this
	case WM_SYSCOMMAND: {
		_msg << WM_SYSCOMMAND;
		//queue_msg.push_back(_msg);
		break;
	}
	case WM_KEYDOWN: {
		_msg << WM_KEYDOWN;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_KEYUP: {
		_msg << WM_KEYUP;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_MOUSEMOVE: {
		_msg << WM_MOUSEMOVE;
		queue_msg.push_back(_msg);//if monitors>1 will be may not work
		break;
	}
	case WM_LBUTTONDOWN: {
		_msg << WM_LBUTTONDOWN;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_RBUTTONDOWN: {
		_msg << WM_RBUTTONDOWN;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_LBUTTONUP: {
		_msg << WM_LBUTTONUP;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_RBUTTONUP: {
		_msg << WM_RBUTTONUP;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_MBUTTONDOWN: {
		_msg << WM_MBUTTONDOWN;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_MBUTTONUP: {//midlle-wheel
		_msg << WM_MBUTTONUP;
		queue_msg.push_back(_msg);
		break;
	}
	case WM_MOUSEWHEEL: {
		_msg << WM_MOUSEWHEEL;
		queue_msg.push_back(_msg);
		break;
	}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*------------------------------*/
/*WndProc-add-cases*/
/*case WM_SYSKEYUP: {
		_msg << WM_SYSKEYUP;
		queue.push_back(_msg);
		break;
	}
	case WM_SYSKEYDOWN: {//
		_msg << WM_SYSKEYDOWN;
		queue.push_back(_msg);
		break;
	}
	case WM_XBUTTONDOWN: {
		_msg << WM_XBUTTONDOWN;
		queue.push_back(_msg);
		break;
	}
	case WM_XBUTTONUP: {
		_msg << WM_XBUTTONUP;
		queue.push_back(_msg);
		break;
	}
	case WM_MOUSEHOVER: {//return mouse hold on win
		_msg << WM_MOUSEHOVER;
		queue.push_back(_msg);
		break;
	}
	case WM_INPUT: {//?? need??
		_msg << WM_INPUT;
		queue.push_back(_msg);
		break;
	}
	*/