/*
	Button.cpp - simple button for SimAVR

	Original Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

	Rewritten/converted to c++ in 2020 by VintagePC <https://github.com/vintagepc/>

 	This file is part of MK404.

	MK404 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MK404 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MK404.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Button.h"
#include "IKeyClient.h"
#include "TelemetryHost.h"
#include <algorithm>        // for copy
#include <iostream>  // for printf

Button::Button(const std::string &strName):Scriptable(strName),IKeyClient(),m_strName(strName)
{
	RegisterActionAndMenu("Press", "Simulate pressing the button", Actions::ActPress);
	RegisterActionAndMenu("Release", "Simulate releasing the button", Actions::ActRelease);
	RegisterActionAndMenu("PressAndRelease", "Simulate pressing and then releasing  the button", Actions::ActPressAndRelease);
}

Button::Button(const std::string &strName, const Key& key, const std::string& strDesc):Button(strName)
{
	RegisterKeyHandler(key,strDesc);
}

Scriptable::LineStatus Button::ProcessAction(unsigned int iAction, const std::vector<std::string>&)
{
	switch (iAction)
	{
	case ActPressAndRelease:
		Press();
		break;
	case ActPress:
		RaiseIRQ(BUTTON_OUT, 0);// press
		break;
	case ActRelease:
		RaiseIRQ(BUTTON_OUT,1);
		break;
	}
	return LineStatus::Finished;
}

void Button::SetIsToggle(bool bVal)
{
	m_bIsToggle = bVal;
}

void Button::OnKeyPress(const Key& /*key*/)
{
	std::cout << "Pressed: " << m_strName << '\n';
	Press(500);
}

void Button::Init(avr_t* avr)
{
	// if name was not provided, init uses the defaults
	const char * pName = m_strName.c_str();
	_Init(avr, this, &pName);

	auto &TH = TelemetryHost::GetHost();
	TH.AddTrace(this, BUTTON_OUT,{TC::InputPin, TC::Misc});

}


avr_cycle_count_t Button::AutoRelease(avr_t *, avr_cycle_count_t uiWhen)
{
	RaiseIRQ(BUTTON_OUT,1);
	std::cout << "Button released after " << uiWhen << " uSec" << '\n';
	return 0;
}

void Button::Press(uint32_t uiUsec)
{
	CancelTimer(m_fcnRelease,this);
	// register the auto-release
	if (m_bIsToggle)
	{
		RaiseIRQ(BUTTON_OUT, m_pIrq[BUTTON_OUT].value^1);
	}
	else
	{
		RaiseIRQ(BUTTON_OUT, 0);// press
		RegisterTimerUsec(m_fcnRelease,uiUsec, this);
	}
}
