// SceneViewerDoc.cpp : implementation of the CSceneViewerDoc class
//

#include "stdafx.h"
#include "sceneviewer.h"
#include "sceneviewerDoc.h"
#include <lang/Debug.h>
#include <lang/String.h>

USING_NAMESPACE(lang)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSceneViewerDoc

IMPLEMENT_DYNCREATE(CSceneViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CSceneViewerDoc, CDocument)
END_MESSAGE_MAP()


// CSceneViewerDoc construction/destruction

CSceneViewerDoc::CSceneViewerDoc()
{
	// TODO: add one-time construction code here

}

CSceneViewerDoc::~CSceneViewerDoc()
{
}

BOOL CSceneViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CSceneViewerDoc serialization

void CSceneViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
		CFile* file = ar.GetFile();
		if ( file )
		{
			fname = String( file->GetFilePath() );
			Debug::printf( "Queued file \"%s\" for loading\n", fname.c_str() );
		}
	}
}


// CSceneViewerDoc diagnostics

#ifdef _DEBUG
void CSceneViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSceneViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CSceneViewerDoc commands

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
