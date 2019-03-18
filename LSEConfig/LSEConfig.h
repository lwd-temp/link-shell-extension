/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */


#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CLSEConfigApp:
// See LSEConfig.cpp for the implementation of this class
//

class PropPageGeneral;

class CLSEConfigApp : public CWinApp
{
  public:
	CLSEConfigApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CLSEConfigApp theApp;
