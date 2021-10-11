/* 0_0 */
#include <Windows.h>//

#include <sstream>//stringstream
#include <string>

#include <deque>//deque<msg>
#include <utility>//pair
#include <vector>//vector<>
#include <algorithm>//find_if labmda

/* Brief:*/
#include "Event.h"//namespace Event->Struct//Event->Mouse,IntRect||Enum Keyboard
#include "Struct.h"//Struct myMSG,cmdParam and enum StyleWin
#include "../logger/logger.h"

#include "../imgui/backends/imgui_impl_win32.h"

class FactoryWin {
public:
    /*-----Main func-----------*/
	/*Create MainWin*/
	FactoryWin(std::string Title,std::pair<int, int>Size = {800,600},
		      std::pair<int, int>Pos = {0,0}, bool fls = false,
		      cmdParam other = {}, StyleWin style=StyleWin::SimpleWin);
	/*your Title for default win*/
	bool addWin(std::string, std::pair<int, int>Size = { 800,600 });
	/*access*/
	bool isEvent();
	/*------------GET-----------------*/
	/*get getSizeMap->count hwnd->count win*/
	size_t cntWindow();
	/*get HWND by name of title*/
	HWND getHWND(std::string);
	/*REBUILD so return first*/
	HWND getMainHWND();
	/*Find title by HWND*/
	std::string getTitle(HWND);
	/*get path to drop file*/
	std::string getPathDropFiles();
	/*get mouse position*/
	std::pair<int, int> getMousePos();
	/*get mouse wheel state*/
	std::pair<int, int> getMouseWheel();
	/*size of queue and get dump of queue*/
	std::pair<size_t,std::string> dumpQueue();
    /*--------------------------------*/
    /*----SET-------------------------*/
	/*write current window title and new Title*/
	bool setTitle(std::string ,std::string);
	/*write hwnd of window and new Title*/
	bool setTitle(HWND, std::string);
	/*set size of window.Example:800,600*/
	bool setSize(HWND hwnd,int width, int height);
	/*On-enterFullscreen Off-exitFullscreen*/
	bool switchFullScreen();
    /*--------------------------------*/
	/*Event*/
	Event::Event event;
	/*--------------------------------*/
    /*------------TODO-------------*/
    void ChangeStyleWindow(StyleWin style = StyleWin::SimpleWin);
	void setCmdParam(cmdParam other);
	/*-----------------------------*/
	/*Destruction*/
	void Destroy();
private:
	/*Private*/
	/*INIT*/
	std::string Title;//tmp
	void InitLogger();
	bool InitWinClass();
	bool InitWin(std::stringstream);
	std::pair<int, int> posWin;//x,y of win
	std::pair<int, int> sizeWin;//width height of win
	StyleWin winStyle;//SimpleWin = 0)
	/*Class for default_win style and name*/
	WNDCLASSEX wc;
	std::string WinClass = "Window_Class_Default";
	cmdParam Other;//4 param
	/*----------------------------------------------------------*/
	/*Map of window*/
	std::vector<std::pair<std::string, HWND>> map;
	/*----Set--------------------*/
	void SetMiddle(HWND);//set middle display
	/*--------------------------------*/
	/*----Get-------------------------*/
	/*get String From HWND*/
	std::string hwndToString(const HWND);
	/*--------------------------------*/
	/*-----(Kernel)-------------------*/
	void HandleMsg();//Event::Event event=event
	/*main function wndProc-static*/
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
		WPARAM wParam, LPARAM lParam);
	static std::deque<myMSG> queue_msg;/*recieve msg for window*/
	/*--------------Queue-------------*/
	bool IsQuit();
	void MouseMove();
	void MouseEvent();
	void isKey();
	void DragAndDrop();
	static void RePaint();
	void ReSize();
	/*TODO*/
	void SysCommand();
	bool delWin(HWND);//return pos win in map if -1 not found may be error
	/*--------------------------------*/
	/*try parseDrop and save path to string*/
	bool tryDrop(WPARAM);
	/*For drop res*/
	std::string pathDrop;
	/*FullScreen*/
	bool enterFullscreen();
	bool exitFullscreen();
	bool fullScreen = false;
	bool writeAccess;
};

