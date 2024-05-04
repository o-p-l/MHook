﻿#include <Windows.h>
#include "MHKeypad.h"
#include "Settings.h"
#include "MHRepErr.h"

extern HWND		MHhwnd;

int MHKeypad::keypad_position=-1;
// Изначально стрелки
WORD MHKeypad::scancode[17]={0xE048, 0xE04D, 0xE050, 0xE04B, 0x3B, 0x3B};

// Для поиска глюков
#ifdef _DEBUG
#endif
static int button_pressed[15]={0};

// Таблица кодирования 8 направлений четырьмя клавишами
int key8[8][4]=
{
	{1,0,0,0}, // 0-е направление = нулевая кнопка
	{1,1,0,0}, // 1-е направление = кнопки 0 и 1
	{0,1,0,0}, // 2
	{0,1,1,0}, // 3
	{0,0,1,0}, // 4
	{0,0,1,1}, // 5
	{0,0,0,1}, // 6
	{1,0,0,1} // 7
};


void MHKeypad::Init(WORD _scancode0, WORD _scancode1, WORD _scancode2, WORD _scancode3, WORD _scancode4, WORD _scancode5,
	WORD _scancode6, WORD _scancode7, WORD _scancode8, WORD _scancode9, WORD _scancode10,
	WORD _scancode11, WORD _scancode12, WORD _scancode13, WORD _scancode14,
	WORD _scancode15, WORD _scancode16)
{
	if(-1!=keypad_position) Reset(); // Перед переинициализацией отпустить кнопки

	scancode[0]=_scancode0;
	scancode[1]=_scancode1;
	scancode[2]=_scancode2;
	scancode[3]=_scancode3;
	scancode[4]=_scancode4;
	scancode[5]=_scancode5;

	scancode[6]=_scancode6;
	scancode[7]=_scancode7;
	scancode[8]=_scancode8;
	scancode[9]=_scancode9;
	scancode[10]=_scancode10;

	scancode[11]=_scancode11;
	scancode[12]=_scancode12;
	scancode[13]=_scancode13;
	scancode[14]=_scancode14;
	scancode[15]=_scancode15;
	scancode[16]=_scancode16;
}


// Тупо нажимает-отжимает одну из 4 клавиш
bool MHKeypad::Press4(int position, bool down, int shift)
{
#ifdef _DEBUG
	//if((position<0)||(position>3)) 
	if((position<0)||(position>10)) // Теперь бывает и 5, и 10 (на правой кнопке мыши) 
 		MHReportError(L"Неверный аргумент у Press4");

	if(down&&(1==button_pressed[position+shift])) 
		MHReportError(L"Повторное нажатие");

	if(!down&&(0==button_pressed[position+shift])) 
		MHReportError(L"Повторное отпускание");

	button_pressed[position+shift] = down ? 1 : 0;
#endif

	int num_of_keys = 0;
	INPUT inputs[2] = { 0 };
	INPUT input1 = { 0 };
	INPUT input2 = { 0 };
	// Спец-кнопка 0xFFFF игнорируется
	if (0xFFFF != scancode[position + shift]) {
		input1.type = INPUT_KEYBOARD;
		input1.ki.dwFlags = KEYEVENTF_SCANCODE;
		if (false == down)
		{
			input1.ki.dwFlags |= KEYEVENTF_KEYUP;
		}
		if (scancode[position + shift] > 0xFF) // Этот скан-код из двух байтов, где первый - E0
		{
			input1.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
		}
		input1.ki.wScan = scancode[position + shift];
		inputs[num_of_keys] = input1;
		++num_of_keys;
	}

	// поддержка второй клавиши для ЛКМ и ПКМ
	bool only_second_key = false;
	if (position == 5 || position == 10) {
		int second_key_index = position == 5 ? 15 : 16;
		if (0xFFFF != scancode[second_key_index]) {
			input2.type = INPUT_KEYBOARD;
			input2.ki.dwFlags = KEYEVENTF_SCANCODE;
			if (false == down)
			{
				input2.ki.dwFlags |= KEYEVENTF_KEYUP;
			}
			if (scancode[second_key_index] > 0xFF) // Этот скан-код из двух байтов, где первый - E0
			{
				input2.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
			}
			input2.ki.wScan = scancode[second_key_index];
			inputs[num_of_keys] = input2;
			++num_of_keys;
			only_second_key = num_of_keys == 1;
		}
	}

	if (num_of_keys == 0) {
		return false;
	}

	SendInput(num_of_keys, inputs, sizeof(INPUT));

	// if only second key pressed return false to also process the mouse button
	if (only_second_key) {
		return false;
	}
	

#ifdef _DEBUG
	TCHAR debug_buf[4096];
	swprintf_s(debug_buf,_countof(debug_buf),L"position: %d, button_pressed: %d %d %d %d %d %d  %d %d %d %d  %d  %d %d %d %d\r\n",
		position, button_pressed[0],button_pressed[1],button_pressed[2],button_pressed[3],button_pressed[4],
		button_pressed[5],
		button_pressed[6],button_pressed[7],button_pressed[8],button_pressed[9],button_pressed[10],
		button_pressed[11],button_pressed[12],button_pressed[13],button_pressed[14]);
	OutputDebugString(debug_buf);
#endif

	return true;
}


// Нажимает кнопку, но отжимает предыдущую нажатую, обрабатывает до 8 позиций
// меняет keypad_position
// перерисовывает экран
void MHKeypad::Press(int position, bool down, int shift)
{
	// Нажатые кнопки повторно не нажимаем, а отжатые - не отжимаем
	if(down && (position==keypad_position))
	{
#ifdef _DEBUG
		//MHReportError("Нажали уже нажатую кнопку");
		OutputDebugString(L"Нажали уже нажатую кнопку");
#endif
		return;
	}

	if(!down && (position!=keypad_position))
	{
#ifdef _DEBUG
		//MHReportError("Отжали ненажатую кнопку");
		OutputDebugString(L"Отжали ненажатую кнопку");
#endif
		return;
	}

	if(-1==position) 
	{
#ifdef _DEBUG
//		MHReportError("Нажали кнопку -1");
		OutputDebugString(L"Нажали кнопку -1");
#endif
		return;
	}
	
		
	if(4==MHSettings::GetNumPositions())
	{
		if(true==down)
		{
			Reset(shift); // при нажатии отпустить все отсальные. А при отпускании не делать этого.
			keypad_position=position;
			//OutputDebugString("Нажали\n");
		}
		else 
		{
			//OutputDebugString("Отпустили\n");
			keypad_position=-1;
		}
		Press4(position,down,shift);
	}
	else // 8 позиций, отпускаем до двух клавиш, нажимаем до двух клавиш
	{
		if(true==down)
		{			
			
			for(int i=0;i<4;i++)
			{
				if(-1!=keypad_position) // сравниваем нажатое и новое направления по 4 клавишам
				{
					if (key8[keypad_position][i] == key8[position][i]) {
						continue; // Эту кнопку не меняем
					}

					if (1 == key8[keypad_position][i]) {
						Press4(i,false,shift); // отпускаем
					}
					else {
						Press4(i,true,shift); // нажимаем
					}
				}
				else // отжимать нечего, только нажимаем
				{
					if (1 == key8[position][i]) {
						Press4(i, true, shift);
					}
				}
			}
			keypad_position=position;

			//OutputDebugString("Нажали8\n");
		}
		else 
		{
			//OutputDebugString("Отпустили8\n");
			Reset(shift); // при нажатии отпустить все оcтальные. А при отпускании не делать этого.
			keypad_position=-1;
		}
	} // 8 направлений

	// Перемещаем квадратик на экране
	InvalidateRect(MHhwnd,NULL,TRUE);
	
}

//====================================================================================
// Сбросить все нажатые кнопки
//====================================================================================
void MHKeypad::Reset(int shift)
{
	if(4==MHSettings::GetNumPositions())
	{
		// Отжимаем одну кнопку
		if(-1!=keypad_position) Press4(keypad_position, false, shift);
	}
	else // когда 8 направлений движения
	{
		switch(keypad_position)
		{
			// Отжимаем одну кнопку
		case 0:
		case 2:
		case 4:
		case 6:

			Press4(keypad_position/2, false, shift);
			break;

			// Отжимаем две кнопки
		case 1: // нулевую и первую
			Press4(0, false, shift);
			Press4(1, false, shift);
			break;

		case 3: // первую и вторую
			Press4(1, false, shift);
			Press4(2, false, shift);
			break;
	
		case 5: // вторую и третью
			Press4(2, false, shift);
			Press4(3, false, shift);
			break;

		case 7: // нулевую и третью
			Press4(0, false, shift);
			Press4(3, false, shift);
			break;
		}
		
	}
	keypad_position=-1;
}

//=========================================================================
// Для 8 разных клавиш (8 умений)
//=========================================================================
void MHKeypad::Press8(int position, bool down)
{
	if(true==down)
	{
		// сначала отпустим старую
		if(keypad_position!=-1)
		{
			if(keypad_position>3) Press4(keypad_position-4,false,6);
			else Press4(keypad_position,false,0);
		}
		keypad_position=position;
		//OutputDebugString("Нажали\n");
	}
	else 
	{
		//OutputDebugString("Отпустили\n");
		keypad_position=-1;
	}
	if(position>3) 	Press4(position-4,down,6);
	else Press4(position,down,0);

	// Перемещаем квадратик на экране
	InvalidateRect(MHhwnd,NULL,TRUE);
}
