// sceneviewerDoc.h : interface of the CSceneViewerDoc class
//

#include <lang/String.h>


#pragma once

class CSceneViewerDoc : public CDocument
{
protected: // create from serialization only
	CSceneViewerDoc();
	DECLARE_DYNCREATE(CSceneViewerDoc)

// Attributes
public:
	NS(lang,String)	fname;

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CSceneViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};



// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
