// SceneViewerView.cpp : implementation of the CSceneViewerView class
//

#include "stdafx.h"
#include "sceneviewer.h"
#include "sceneviewerDoc.h"
#include "sceneviewerView.h"
#include "font.h"
#include ".\sceneviewerview.h"
#include <io/FindFile.h>
#include <io/PathName.h>
#include <gr/dx/DX_Context.h>
#include <gr/sw/SW_Context.h>
#include <lua/test.h>
#include <hgr/Light.h>
#include <hgr/Mesh.h>
#include <hgr/Globals.h>
#include <hgr/GlowPipe.h>
#include <hgr/DefaultPipe.h>
#include <hgr/DefaultResourceManager.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/Profile.h>
#include <lang/System.h>
#include <lang/Exception.h>
#include <math/IntersectionUtil.h>

#define BUILD_DX
//#define BUILD_SW

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

USING_NAMESPACE(io)
USING_NAMESPACE(gr)
USING_NAMESPACE(hgr)
using namespace ode;
USING_NAMESPACE(lang)
USING_NAMESPACE(math)


static void loadShaders( Context* context, const String& path, bool recursesubdirs )
{
	P(Shader) fx;
	for ( FindFile find(PathName(path,"*.fx").toString()) ; find.more() ; find.next() )
	{
		fx = DefaultResourceManager::get(context)->getShader( find.data().path.toString(), 0 );
		fx = DefaultResourceManager::get(context)->getShader( find.data().path.toString(), 1 );
	}
}

static void loadParticles( Context* context, const String& path, bool recursesubdirs )
{
	P(ParticleSystem) prt;
	for ( FindFile find(PathName(path,"*.prs").toString()) ; find.more() ; find.next() )
		prt = DefaultResourceManager::get(context)->getParticleSystem( find.data().path.toString(), find.data().path.parent().toString(), find.data().path.parent().toString() );
}

static String getDefaultDataPath()
{
	// check if KA3D is defined, use that as default data path, otherwise data/
	const char* env = getenv("KA3D");
	if ( env == 0 )
		return "data/";
	else
		return PathName(env,"data/").toString();
}


// CSceneViewerView

IMPLEMENT_DYNCREATE(CSceneViewerView, CView)

BEGIN_MESSAGE_MAP(CSceneViewerView, CView)
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_ACTIVATE()
	ON_WM_ENTERMENULOOP()
	ON_WM_EXITMENULOOP()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CSceneViewerView construction/destruction

CSceneViewerView::CSceneViewerView() :
	m_initdone( false ),
	m_animTime( 0 ),
	m_time( 0 ),
	m_active( true ),
	m_angle( 0,0,0 ),
	m_speed( 100.f ),
	m_paused( false ),
	m_passes( -1 ),
	m_sim( true ),
	m_particleRefreshIndex( 0 ),
	m_debugMode( DEBUGMODE_NONE ),
	m_recordAnim( false ),
	m_mousepos( 0, 0 ),
	m_mouseLB( 2 )
{
	// TODO: add construction code here
}

CSceneViewerView::~CSceneViewerView()
{
	NS(hgr,Globals)::cleanup();
}

BOOL CSceneViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CSceneViewerView drawing

void CSceneViewerView::OnDraw(CDC* pDC)
{
	CSceneViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc || CSceneViewerApp::get()->suspended())
		return;

	try
	{
		OnDrawCanThrow();
	}
	catch ( Throwable& e )
	{
		HandleError( e );
	}

	// request new update
	InvalidateRect( NULL, FALSE );
}

void CSceneViewerView::OnDrawCanThrow()
{
	CSceneViewerApp* app = CSceneViewerApp::get();
	CSceneViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// do init if not initialized yet
	// (init needs to be deferred in some cases so better do it here)
	if ( !m_initdone )
	{
		m_initdone = true;
		Init();
	}
	Context* context = m_context;
	if ( context == 0 )
		return;

	// init new scene if needed
	if ( pDoc->fname != "" )
	{
		// init console
#ifdef BUILD_DX
		if ( !m_console )
		{
			void* data;
			int w;
			int h;
			getFontImage( &data, &w, &h );

			P(Texture) tex = context->createTexture( w, h, SurfaceFormat::SURFACE_A8R8G8B8, 0, 0 );
			tex->blt( 0, 0, data, w*4, w, h, SurfaceFormat::SURFACE_A8R8G8B8, 0, SurfaceFormat() );
			m_console = new Console( context, tex, 20, 23, -5, "sprite-add" );
		}
#endif

		String fname = pDoc->fname;
		pDoc->fname = "";
		LoadScene( fname );
	}
	if ( m_scene == 0 || m_camera == 0 )
	{
		context->present();
		return;
	}
	Scene* scene = m_scene;

	// reload particles if they have been modified,
	// but check only one file / update to avoid burdening file system too much
	static int s_check = 0;
	if ( --s_check < 0 )
	{
		s_check = 0;

		int particlecount = 0;
		for ( Node* node = m_scene ; node != 0 ; node = node->next(m_scene) )
		{
			ParticleSystem* ps = dynamic_cast<ParticleSystem*>( node );
			if ( ps != 0 )
			{
				if ( particlecount == m_particleRefreshIndex )
				{
					String name = ps->name();
					FindFile find( name );
					if ( find.more() && 
						m_particlesModified[name] != find.data().writeTime )
					{
						String texturepath = PathName(name).parent().toString();
						ps->load( context, name, DefaultResourceManager::get(context), texturepath );
						m_particlesModified[name] = find.data().writeTime;
					}
				}
				++particlecount;
			}
		}
		m_particleRefreshIndex = (m_particleRefreshIndex+1) % (particlecount+1);
	}
	
	// update time
	if ( !m_active )
		return;

	// update animations
	float dt = app->tick();
	if ( m_recordAnim )
		dt = 1.f/30.f;

	if ( !m_paused )
	{
		m_time += dt;
		m_animTime += dt;

		m_scene->applyAnimations( m_animTime, dt );

		Simulate( 1.f/100.f );
	}

	// user control
	if ( GetFocus() == this )
	{
		bool freecam = m_scene->transformAnimations()->get(m_camera->name()) == 0;

		// update movement
		float3 dir(0,0,0);
		if ( GetKeyState('W') < 0 )
			dir += float3(0,0,1);
		if ( GetKeyState('S') < 0 )
			dir += float3(0,0,-1);
		if ( GetKeyState('A') < 0 || (GetKeyState(VK_LEFT) < 0 && GetKeyState(VK_SHIFT) < 0) )
			dir += float3(-1,0,0);
		if ( GetKeyState('D') < 0 || (GetKeyState(VK_RIGHT) < 0 && GetKeyState(VK_SHIFT) < 0) )
			dir += float3(1,0,0);
		if ( GetKeyState(VK_UP) < 0 && GetKeyState(VK_SHIFT) < 0 )
			dir += float3(0,1,0);
		if ( GetKeyState(VK_DOWN) < 0 && GetKeyState(VK_SHIFT) < 0 )
			dir += float3(0,-1,0);
		if ( dir.length() > 0 && freecam )
		{
			float3 delta = normalize0(dir) * (m_speed * dt);
			m_camera->setPosition( m_camera->position() + m_camera->rotation() * delta );
		}

		// update rotation
		float3 rot(0,0,0);
		if ( GetKeyState(VK_RIGHT) < 0 && GetKeyState(VK_SHIFT) >= 0 )
			rot += float3(0,1,0);
		if ( GetKeyState(VK_LEFT) < 0 && GetKeyState(VK_SHIFT) >= 0 )
			rot += float3(0,-1,0);
		if ( GetKeyState(VK_UP) < 0 && GetKeyState(VK_SHIFT) >= 0 )
			rot += float3(1,0,0);
		if ( GetKeyState(VK_DOWN) < 0 && GetKeyState(VK_SHIFT) >= 0 )
			rot += float3(-1,0,0);
		if ( rot.length() > 0 && freecam )
		{
			float anglespeed = Math::PI;
			m_angle += rot * (anglespeed * dt);
			m_camera->setRotation( 
				float3x3(float3(0,1,0),m_angle.y) *
				float3x3(float3(1,0,0),m_angle.x) );
		}
		
		// release camera if input received
		if ( dir.length() > 0.f || rot.length() > 0.f )
		{
			if ( !freecam )
			{
				m_scene->transformAnimations()->remove( m_camera->name() );
			}
		}
	}

	// render frame
	{
		Context::RenderScene rs( context );

		// render bone visualizations
		if ( m_debugMode == DEBUGMODE_BBOXVISUALIZATION )
			BoundingBoxVisualization();
		else if ( m_debugMode == DEBUGMODE_TANGENTVISUALIZATION )
			TangentVisualization();
		else if ( m_debugMode == DEBUGMODE_PICKOBJECT )
			PickObjectVisualization();
		
		// add mark to lights
		/*for ( Node* node = m_camera->root() ; node != 0 ; node = node->next() )
		{
			if ( dynamic_cast<Light*>(node) != 0 )
				m_debugLines->addBox( node->worldTransform(), float3(-1,-1,-1), float3(1,1,1), float4(1,1,0,1) );
		}*/

		// render scene
#ifdef BUILD_DX
		if ( m_debugMode == DEBUGMODE_NORMALMAPVISUALIZATION )
		{
			NormalMapWorldAxisVisualization();
			m_pipeSetup->setup( m_camera );
			m_defaultPipe->render( 0, context, scene, m_camera );
			m_normalOnlyPipe->render( 0, context, scene, m_camera );
		}
		else
		{
			bool defaultenabled = (m_passes&(1<<0)) != 0;
			bool spriteenabled	= (m_passes&(1<<2)) != 0;
			bool glowenabled	= (m_passes&(1<<3)) != 0;
			{PROFILE(pipeSetup);	m_pipeSetup->setup( m_camera );}
			{PROFILE(defaultPipe);	if (defaultenabled)	m_defaultPipe->render( 0, context, scene, m_camera );}
			{PROFILE(spritePipe);	if (spriteenabled)	m_spritePipe->render( 0, context, scene, m_camera );}
			{PROFILE(glowPipe);		if (glowenabled)	m_glowPipe->render( 0, context, scene, m_camera );}
		}
#elif defined(BUILD_SW) || defined(BUILD_N3D)
		m_camera->render( context );
#else
#error rendering not defined for this build
#endif

		// render debug info
		if ( m_console != 0 )
			RenderDebugInfo();

		m_debugLines->removeLines();
	}

	// flip back buffer
	if ( m_recordAnim )
		context->capture( PathName(PathName(m_scenefilename).parent().toString(),"capture%04d.bmp").toString() );
	context->present( 0 );
}

const char* CSceneViewerView::toString( DebugMode mode )
{
	switch ( mode )
	{
	case DEBUGMODE_RENDERINFO:				return "RENDER INFO";
	case DEBUGMODE_PICKOBJECT:				return "PICK OBJECTS WITH MOUSE";
	case DEBUGMODE_BBOXVISUALIZATION:		return "BOUNDING BOXES";
	case DEBUGMODE_TANGENTVISUALIZATION:	return "TANGENT SPACE";
	case DEBUGMODE_NORMALMAPVISUALIZATION:	return "NORMAL MAPS";
	default:								return "";
	}
}

void CSceneViewerView::RenderDebugInfo()
{
	assert( m_context != 0 );
	assert( m_camera != 0 );
	const Camera::Statistics& stat1 = m_camera->statistics;
	const Context::Statistics& stat2 = m_context->statistics;

	if ( m_debugMode != DEBUGMODE_NONE )
		m_console->printf( "%s\n", toString(m_debugMode) );

	if ( m_debugMode == DEBUGMODE_RENDERINFO )
	{
		bool defaultenabled = (m_passes&(1<<0)) != 0;
		bool spriteenabled	= (m_passes&(1<<2)) != 0;
		bool glowenabled	= (m_passes&(1<<3)) != 0;
		m_console->printf( "Time: %g (frame %d)\n", m_animTime, int(m_animTime*30.f) );
		m_console->printf( "Rendering passes (default/sprite/glow): %d/%d/%d\n", defaultenabled?1:0, spriteenabled?1:0, glowenabled?1:0 );
		m_console->printf( "Camera position: %g %g %g\n", m_camera->position().x, m_camera->position().y, m_camera->position().z );
		m_console->printf( "Visuals before/after cull: %5d / %5d\n", stat1.visualsBeforeCull, stat1.visualsAfterCull );
		m_console->printf( "Rendered tri / primitives: %5d / %5d\n", stat2.renderedTriangles, stat2.renderedPrimitives );
	}

	m_console->render( m_context );
	m_console->clear();
	m_camera->statistics.reset();
	m_context->statistics.reset();
}

void CSceneViewerView::Init()
{
	try
	{
		// auto update
		ShellExecute( this->m_hWnd,
			"open",
			"AutoUpdator.exe",
			"/silent /author \"Jani Kajala\" /program \"KA3D Engine\"",
			"C:/Program Files/Common Files/Thraex Software/AutoUpdator/",
			SW_SHOW );

		// init context
		RECT cr;
		GetClientRect( &cr );

		//int width = cr.right;
		//int height = cr.bottom;
#ifdef BUILD_DX
		int width = 924;
		int height = 675;
		bool fullscreen = false;
		int bits = 32;
		bool stencil = false;
		int surfaces = DX_Context::SURFACE_TARGET + DX_Context::SURFACE_DEPTH;
		if ( stencil )
			surfaces += DX_Context::SURFACE_STENCIL;
		m_context = new DX_Context( width, height, bits, 
			fullscreen ? DX_Context::WINDOW_FULLSCREEN : DX_Context::WINDOW_DESKTOP,
			DX_Context::RASTERIZER_HW,
			DX_Context::VERTEXP_HW,
			surfaces );
#elif defined(BUILD_SW)
		int width = 240;
		int height = 320;
		int bits = 16;
		m_context = SW_createContext( 0, width, height );
#else
#error Unknown build
#endif

#ifdef BUILD_DX
		// init shaders and particles
		String path = getDefaultDataPath();
		loadShaders( m_context, path+"shaders", true );
		loadParticles( m_context, path+"particles", true );

		// init rendering pipes
		m_pipeSetup = new PipeSetup( m_context );
		m_defaultPipe = new DefaultPipe( m_pipeSetup, "Default", -2 );
		m_normalOnlyPipe = new DefaultPipe( m_pipeSetup, "NormalOnly" );
		m_glowPipe = new GlowPipe( m_pipeSetup );
		m_spritePipe = new DefaultPipe( m_pipeSetup, "Default", -3, -3 );
#endif
	}
	catch ( Throwable& e )
	{
		HandleError( e );
	}
}

void CSceneViewerView::PickObjectVisualization()
{
	if ( !m_camera )
		return;

	// pick vector
	const ViewFrustum& frustum = m_camera->frustum();
	float dimx, dimy, viewplanez;
	frustum.getViewDimensions( &dimx, &dimy, &viewplanez );
	dimx *= 0.5f;
	dimy *= 0.5f;
	float viewplanex = m_mousepos.x*dimx;
	float viewplaney = m_mousepos.y*dimy;
	float3x4 worldcamtm = m_camera->worldTransform();
	float3 worldcamx = worldcamtm.getColumn(0);
	float3 worldcamy = worldcamtm.getColumn(1);
	float3 worldcampos = worldcamtm.translation();
	float3 worldcamtarget( worldcamtm.transform( float3(viewplanex,viewplaney,viewplanez) ) );
	float3 worldcamdir = normalize( worldcamtarget - worldcampos );

	// pick object
	float minu = Float::MAX_VALUE;
	Mesh* selobj = 0;
	float3 seltri[3];
	for ( Node* node = m_scene ; node != 0 ; node = node->next(m_scene) )
	{
		Mesh* mesh = dynamic_cast<Mesh*>(node);
		if ( mesh != 0 )
		{
			float3x4 worldtm = mesh->worldTransform();
			float3x4 invworldtm = worldtm.inverse();
			float3 objcampos = invworldtm.transform( worldcampos );
			float3 objcamdir = invworldtm.rotate( worldcamdir );

			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				Primitive* prim = mesh->getPrimitive( i );
				Primitive::Lock lk( prim, Primitive::LOCK_READ );

				for ( int k = 0 ; k < prim->indices() ; k += 3 )
				{
					int ind[3];
					prim->getIndices( k, ind, 3 );
					float3 v[3];
					float4 scalebias = prim->vertexPositionScaleBias();
					for ( int n = 0 ; n < 3 ; ++n )
					{
						float4 vtmp;
						prim->getVertexPositions( ind[n], &vtmp, 1 );
						v[n] = vtmp.xyz();
						v[n] *= scalebias.x;
						v[n] += float3( scalebias.y, scalebias.z, scalebias.w );
					}

					float u;

					if ( IntersectionUtil::findRayTriangleIntersection( objcampos, objcamdir, v[0], v[1], v[2], &u ) )
					{
						if ( u < minu )
						{
							minu = u;
							selobj = mesh;
							for ( int n = 0 ; n < 3 ; ++n )
								seltri[n] = worldtm.transform( v[n] );
						}
					}
				}
			}
		}
	}

	// store CSI beam if requested
	if ( m_mouseLB & 1 )
	{
		m_staticDebugBeams.add( worldcampos );
		m_staticDebugBeams.add( worldcampos + worldcamdir*minu );
		m_mouseLB &= ~1;
	}

	// visualization colors
	float4 cursorcolor( 1,1,1,1 );
	float4 cursorpolycolor( 0,1,0,1 );
	float4 meshpolycolor( 0,1,0,0.3f );
	float4 boxcolor( 0,0,1,1 );
	float4 csibeamcolor( 1,0,0,0.5f );

	// visualize
	float3 pos = m_camera->worldTransform().translation();
	float3 pos1 = pos+worldcamdir*500.f;
	m_debugLines->addLine( pos, pos1, cursorcolor );
	m_debugLines->addLine( pos1-worldcamx*100.f, pos1+worldcamx*100.f, cursorcolor );
	m_debugLines->addLine( pos1-worldcamy*100.f, pos1+worldcamy*100.f, cursorcolor );

	if ( minu != Float::MAX_VALUE )
	{
		m_debugLines->addLine( seltri[0], seltri[1], cursorpolycolor );
		m_debugLines->addLine( seltri[1], seltri[2], cursorpolycolor );
		m_debugLines->addLine( seltri[2], seltri[0], cursorpolycolor );

		m_debugLines->addBox( selobj->worldTransform(), selobj->boundBoxMin(), selobj->boundBoxMax(), boxcolor );

		float3x4 worldtm = selobj->worldTransform();
		for ( int i = 0 ; i < selobj->primitives() ; ++i )
		{
			Primitive* prim = selobj->getPrimitive( i );
			Primitive::Lock lk( prim, Primitive::LOCK_READ );

			for ( int k = 0 ; k < prim->indices() ; k += 3 )
			{
				int ind[3];
				prim->getIndices( k, ind, 3 );
				float3 v[3];
				float4 scalebias = prim->vertexPositionScaleBias();
				for ( int n = 0 ; n < 3 ; ++n )
				{
					float4 vtmp;
					prim->getVertexPositions( ind[n], &vtmp, 1 );
					v[n] = vtmp.xyz();
					v[n] *= scalebias.x;
					v[n] += float3( scalebias.y, scalebias.z, scalebias.w );
					v[n] = worldtm.transform( v[n] );
				}

				m_debugLines->addLine( v[0], v[1], meshpolycolor );
				m_debugLines->addLine( v[1], v[2], meshpolycolor );
				m_debugLines->addLine( v[2], v[0], meshpolycolor );
			}
		}
	}

	for ( int i = 0 ; i < m_staticDebugBeams.size() ; i += 2 )
	{
		float3 v0 = m_staticDebugBeams[i];
		float3 v1 = m_staticDebugBeams[i+1];
		m_debugLines->addLine( v0, v1, csibeamcolor );
	}
}

void CSceneViewerView::TangentVisualization()
{
	// setup bone bounding box visualization
	for ( Node* node = m_scene ; node ; node = node->next(m_scene) )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( 0 != mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				Primitive* prim = mesh->getPrimitive(i);
				Primitive::Lock lk( prim, Primitive::LOCK_READ );

				if ( !prim->vertexFormat().hasData(VertexFormat::DT_TANGENT) )
					continue;

				for ( int k = 0 ; k < prim->vertices() ; ++k )
				{
					float4 t,p,n;
					prim->getVertexTangents( k, &t, 1 );
					prim->getVertexPositions( k, &p, 1 );
					prim->getVertexNormals( k, &n, 1 );

					float3x4 tm = mesh->worldTransform();
					float3 pos = tm.transform( p.xyz() );
					float3 normal = tm.rotate( n.xyz() );
					float3 tangent = tm.rotate( t.xyz() );
					//pos += normal*.5f;

					m_debugLines->addLine( pos, pos+tangent, float4(1,0,0,1) );
					m_debugLines->addLine( pos, pos+cross( normal, tangent ), float4(0,1,0,1) );
					m_debugLines->addLine( pos, pos+normal, float4(0,0,1,1) );
				}
			}
		}
	}

	// pos axis
	float r = 100.f;
	float z = 0.f;
	m_debugLines->addLine( float3(0,0,0), float3(1,0,0)*r, float4(1,z,z,1) );
	m_debugLines->addLine( float3(0,0,0), float3(0,1,0)*r, float4(z,1,z,1) );
	m_debugLines->addLine( float3(0,0,0), float3(0,0,1)*r, float4(z,z,1,1) );
}

void CSceneViewerView::NormalMapWorldAxisVisualization()
{
	float r = 100.f;
	float z = 0.5f;
	// pos axis
	m_debugLines->addLine( float3(0,0,0), float3(1,0,0)*r, float4(1,z,z,1) );
	m_debugLines->addLine( float3(0,0,0), float3(0,1,0)*r, float4(z,1,z,1) );
	m_debugLines->addLine( float3(0,0,0), float3(0,0,1)*r, float4(z,z,1,1) );
	// neg axis
	m_debugLines->addLine( float3(0,0,0), float3(-1,0,0)*r, float4(0,z,z,1) );
	m_debugLines->addLine( float3(0,0,0), float3(0,-1,0)*r, float4(z,0,z,1) );
	m_debugLines->addLine( float3(0,0,0), float3(0,0,-1)*r, float4(z,z,0,1) );
}

void CSceneViewerView::BoundingBoxVisualization()
{
	const char* bones[] = 
	{
		"Bip01 Head",
		"Bip01 L Calf",
		"Bip01 L Clavicle",
		"Bip01 L Foot",
		"Bip01 L Forearm",
		"Bip01 L Hand",
		"Bip01 L Thigh",
		"Bip01 L Toe0",
		"Bip01 L UpperArm",
		"Bip01 Neck",
		"Bip01 Pelvis",
		"Bip01 R Calf",
		"Bip01 R Clavicle",
		"Bip01 R Foot",
		"Bip01 R Forearm",
		"Bip01 R Hand",
		"Bip01 R Thigh",
		"Bip01 R Toe0",
		"Bip01 R UpperArm",
		"Bip01 Spine",
		"Bip01 Spine1",
		"Bip01 Spine2",
		"Bip01 Spine3",
		0
	};

	// setup bone bounding box visualization
	float3 scenemin, scenemax;
	m_scene->getBoundBox( &scenemin, &scenemax );
	m_debugLines->addBox( float3x4(1.f), scenemin, scenemax, float4(1,1,1,0.5) );

	for ( Node* node = m_scene ; node ; node = node->next(m_scene) )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( 0 != mesh )
		{
			if ( 0 == mesh->bones() )
			{
				m_debugLines->addBox( mesh->worldTransform(), mesh->boundBoxMin(), mesh->boundBoxMax(), float4(1,1,1,1) );
			}
			else
			{
				for ( int i = 0 ; i < mesh->primitives() ; ++i )
					mesh->getPrimitive(i)->lock( Primitive::LOCK_READ );

				for ( int i = 0 ; i < mesh->bones() ; ++i )
				{
					Node* bonenode = mesh->getBone(i);
					float3x4 boneworldtm = bonenode->worldTransform();
					float3x4 boneinverseresttm = mesh->getBoneInverseRestTransform(i);

					bool found = false;
					for ( int n = 0 ; bones[n] ; ++n )
						if ( found = (bonenode->name() == bones[n]) )
							break;
					if ( !found )
						continue;

					float3 min( Float::MAX_VALUE, Float::MAX_VALUE, Float::MAX_VALUE );
					float3 max( -Float::MAX_VALUE, -Float::MAX_VALUE, -Float::MAX_VALUE );

					for ( int k = 0 ; k < mesh->primitives() ; ++k )
					{
						Primitive* prim = mesh->getPrimitive( k );
						for ( int j = 0 ; j < prim->vertices() ; ++j )
						{
							for ( int l = 0 ; l < prim->usedBones() ; ++l )
							{
								if ( prim->usedBoneArray()[l] == i )
								{
									float4 bi;
									prim->getVertexBoneIndices( j, &bi, 1 );
									assert( (bi.x == (float)i) == (int(bi.x+0.5f) == i) );
									float4 bw;
									prim->getVertexBoneWeights( j, &bw, 1 );
									assert( bw.x >= bw.y && bw.y >= bw.z );

									if ( bi.x == (float)l && bw.x > 0.f )
									{
										float4 v;
										prim->getVertexPositions( j, &v, 1 );
										v.w = 1.f;
										v = boneinverseresttm.transform( v );
									
										//float3 x = boneworldtm.transform( v.xyz() );
										//m_debugLines->addLine( x, x-float3(0,0,0.2f), float4(1,1,1,1) );

										for ( int n = 0 ; n < 3 ; ++n )
										{
											min[n] = Math::min( min[n], v[n] );
											max[n] = Math::max( max[n], v[n] );
										}
									}
								}
							}
						}

						m_debugLines->addLine( boneworldtm.translation(), bonenode->parent()->worldTransform().translation(), float4(0,1,0,1) );
					}

					m_debugLines->addBox( bonenode->worldTransform(), min, max, float4(1,1,1,1) );
				}

				for ( int i = 0 ; i < mesh->primitives() ; ++i )
					mesh->getPrimitive(i)->unlock();
			}
		}
	}
}

void CSceneViewerView::LoadScene( const String& fname )
{
	Debug::printf( "Loading file \"%s\"\n", fname.c_str() );

	try
	{
		m_scenefilename = fname;
		m_scene = new Scene( m_context, fname );

		// DEBUG: set all rotations identity, used for testing Y-sorting
		//for ( Node* node = m_scene ; node != 0 ; node = node->next(m_scene) )
		//	node->setRotation( float3x3(1.f) );

		m_camera = m_scene->camera();
		m_animTime = 0.f;
		m_time = 0.f;

		// add debug lines
		m_debugLines = new Lines( m_context );
		m_debugLines->linkTo( m_scene );

		// check for valid user property types
		//const char* keylist[] = {"physics", "mass", "density", "particle", "time", "collision"};
		//m_scene->userProperties()->check( keylist, sizeof(keylist)/sizeof(keylist[0]) );

		// find if there is a light, add default light to camera if not
		P(Light) lt = 0;
		for ( Node* n = m_scene ; n != 0 ; n = n->next(m_scene) )
		{
			lt = dynamic_cast<Light*>( n );
			if ( lt != 0 )
				break;
		}
		if ( lt == 0 )
		{
			lt = new Light;
			lt->linkTo( m_camera );
		}

		InitSim();

		CSceneViewerApp::get()->tick();
	}
	catch ( Throwable& e )
	{
		HandleError( e );
	}
}

void CSceneViewerView::Simulate( float dt )
{
	PROFILE(simulation);

	// update simulation at fixed interval
	if ( m_sim )
	{
		if ( m_time > 1.f )
			m_time = 1.f;
		for ( ; m_time > dt ; m_time -= dt )
			m_world->step( dt, 0 );
	}

	// get visual object positions from the rigid bodies
	for ( int i = 0 ; i < m_objects.size() ; ++i )
		m_objects[i]->updateVisualTransform();
}

void CSceneViewerView::InitSim()
{
	m_objects.clear();
	m_world = new ODEWorld;

	UserPropertySet* ups = m_scene->userProperties();
	if ( ups )
	{
		// create simulation objects from meshes
		for ( Node* node = m_scene ; node != 0 ; node = node->next(m_scene) )
		{
			Mesh* mesh = dynamic_cast<Mesh*>( node );
			if ( mesh )
			{
				// parse properties (from 'User Defined Properties' text field)
				String props = ups->get( mesh->name() );
				ODEObject::GeomType geomtype;
				ODEObject::MassType masstype;
				float mass;
				ODEObject::parseProperties( mesh, props, &geomtype, &masstype, &mass );

				// remove from animation set
				m_scene->transformAnimations()->remove( mesh->name() );

				// create rigid body with these properties
				P(ODEObject) obj = new ODEObject( m_world->world(), m_world->space(), mesh, geomtype, masstype, mass );
				m_objects.add( obj );
			}
		}
	}
}

void CSceneViewerView::HandleError( Throwable& e )
{
	if ( GetDocument() != 0 )
	{
		GetDocument()->OnNewDocument();
		GetDocument()->fname = "";
	}

	m_scene = 0;
	m_camera = 0;
	m_console = 0;

	char buf[256];
	e.getMessage().format( buf, sizeof(buf) );
	MessageBox( buf );
}

// CSceneViewerView diagnostics

#ifdef _DEBUG
void CSceneViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CSceneViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSceneViewerDoc* CSceneViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSceneViewerDoc)));
	return (CSceneViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CSceneViewerView message handlers
afx_msg void CSceneViewerView::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	CSceneViewerDoc* pDoc = GetDocument();
	switch ( nChar )
	{
	case '1':		m_speed = 1.f; break;
	case '2':		m_speed = 5.f; break;
	case '3':		m_speed = 20.f; break; 
	case '4':		m_speed = 50.f; break;
	case '5':		m_speed = 100.f; break;
	case '6':		m_speed = 200.f; break;
	case '7':		m_speed = 400.f; break;
	case '8':		m_speed = 800.f; break;
	case '9':		m_speed = 1600.f; break;
	case VK_F5:		pDoc->fname = m_scenefilename; break;
	case VK_F6:		m_sim = !m_sim; Debug::printf("Simulator is now %d\n", m_sim?1:0); break;
	case VK_F8:		m_debugMode = DebugMode( (m_debugMode+1) % DEBUGMODE_COUNT ); break;
	case VK_F9:		m_recordAnim = !m_recordAnim; break;
	case VK_F11:	if (GetKeyState(VK_SHIFT) < 0) TogglePass(2); else TogglePass(0); break;
	case VK_F12:	if (GetKeyState(VK_SHIFT) < 0) TogglePass(3); else TogglePass(1); break;
	case VK_PAUSE:	m_paused = !m_paused; break;
	}
}

void CSceneViewerView::TogglePass( int index )
{
	int f = 1<<index;
	if ( m_passes & f ) 
		m_passes &= ~f; 
	else 
		m_passes |= f;
}

afx_msg void CSceneViewerView::OnLButtonDown( UINT nFlags, CPoint point )
{
	OnMouseMove( nFlags, point ); // store pos

	if ( m_mouseLB & 2 )
		m_mouseLB |= 1;
}

afx_msg void CSceneViewerView::OnLButtonUp( UINT nFlags, CPoint point )
{
	m_mouseLB |= 2;
}

afx_msg void CSceneViewerView::OnMouseMove( UINT nFlags, CPoint point )
{
	RECT cr;
	GetClientRect( &cr );
	if ( cr.right < 2 || cr.bottom < 2 )
		return;

	m_mousepos.x = (float)(point.x - cr.right/2) / (float)(cr.right/2);
	m_mousepos.y = (float)(cr.bottom/2 - point.y) / (float)(cr.bottom/2);
}

afx_msg void CSceneViewerView::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized )
{
	m_active = ( nState != WA_INACTIVE );
}

void CSceneViewerView::OnEnterMenuLoop( BOOL )
{
	CSceneViewerApp::get()->suspend();
}

void CSceneViewerView::OnExitMenuLoop( BOOL )
{
	CSceneViewerApp::get()->resume();
}

BOOL CSceneViewerView::OnEraseBkgnd( CDC* cdc )
{
	if ( m_scene == 0 )
	{
		RECT cr;
		GetClientRect( &cr );
		CBrush brush;
		brush.CreateSolidBrush( COLORREF(0) );
		cdc->FillRect( &cr, &brush );
	}
	return TRUE;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
