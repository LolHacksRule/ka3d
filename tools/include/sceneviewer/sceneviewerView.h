// sceneviewerView.h : interface of the CSceneViewerView class
//


#pragma once


#include <gr/Context.h>
#include <hgr/Pipe.h>
#include <hgr/Scene.h>
#include <hgr/Lines.h>
#include <hgr/Camera.h>
#include <hgr/Console.h>
#include <hgr/PipeSetup.h>
#include <ode/ODEWorld.h>
#include <ode/ODEObject.h>
#include <lang/String.h>
#include <lang/Hashtable.h>
#include <lang/Exception.h>
#include <math/float2.h>
#include <time.h>


class CSceneViewerView : public CView
{
protected: // create from serialization only
	CSceneViewerView();
	DECLARE_DYNCREATE(CSceneViewerView)

// Attributes
public:
	CSceneViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CSceneViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );
	afx_msg BOOL OnEraseBkgnd( CDC* );
	afx_msg void OnEnterMenuLoop( BOOL );
	afx_msg void OnExitMenuLoop( BOOL );
protected:
	DECLARE_MESSAGE_MAP()
private:
	enum DebugMode
	{
		DEBUGMODE_NONE,
		DEBUGMODE_RENDERINFO,
		DEBUGMODE_PICKOBJECT,
		DEBUGMODE_BBOXVISUALIZATION,
		DEBUGMODE_TANGENTVISUALIZATION,
		DEBUGMODE_NORMALMAPVISUALIZATION,
		DEBUGMODE_COUNT
	};

	bool				m_initdone;
	P(gr::Context)		m_context;
	P(hgr::Console)		m_console;
	NS(lang,String)		m_scenefilename;
	P(NS(hgr,Scene))		m_scene;
	P(NS(hgr,Camera))		m_camera;
	float				m_animTime;
	float				m_time;
	bool				m_active;
	P(NS(hgr,PipeSetup))	m_pipeSetup;
	P(NS(hgr,Pipe))		m_defaultPipe;
	P(NS(hgr,Pipe))		m_normalOnlyPipe;
	P(NS(hgr,Pipe))		m_glowPipe;
	P(NS(hgr,Pipe))		m_spritePipe;
	lang::Hashtable<NS(lang,String),time_t>	m_particlesModified;
	int										m_particleRefreshIndex;
	// debug
	NS(lang,Array)<NS(math,float3)>	m_staticDebugBeams;
	// user control
	NS(math,float3)		m_angle;
	float				m_speed;
	bool				m_paused;
	int					m_passes;
	bool				m_sim;
	DebugMode			m_debugMode;
	P(hgr::Lines)		m_debugLines;
	bool				m_recordAnim;
	NS(math,float2)		m_mousepos; // relative +-1, origin in center
	int					m_mouseLB; // mouse left button: bit 0: down, bit 1: released

	P(ode::ODEWorld)				m_world;
	NS(lang,Array)<P(ode::ODEObject)>	m_objects;

	void Init();
	void LoadScene( const NS(lang,String)& fname );
	void Simulate( float dt );
	void InitSim();
	void HandleError( lang::Throwable& e );
	void OnDrawCanThrow();
	void RenderDebugInfo();
	void TogglePass( int index );
	void BoundingBoxVisualization();
	void TangentVisualization();
	void PickObjectVisualization();
	void NormalMapWorldAxisVisualization();

	static const char* toString( DebugMode mode );
};

#ifndef _DEBUG  // debug version in SceneViewerView.cpp
inline CSceneViewerDoc* CSceneViewerView::GetDocument() const
   { return reinterpret_cast<CSceneViewerDoc*>(m_pDocument); }
#endif

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
