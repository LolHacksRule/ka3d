#include "StdAfx.h"
#include "version.h"
#include "MtlUtil.h"
#include "ArrayUtil.h"
#include "INodeUtil.h"
#include "ColorUtil.h"
#include "SilentQuit.h"
#include "MySceneExport.h"
#include "ProgressDialog.h"
#include "WorldUnitsRescale.h"
#include "resource.h"
#include <gr/SurfaceFormat.h>
#include <io/PathName.h>
#include <io/FindFile.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <img/ShapeUtil.h>
#include <img/ImageReader.h>
#include <img/ImageWriter.h>
#include <lua/LuaState.h>
#include <hgr/Globals.h>
#include <hgr/UserPropertySet.h>
#include <hgr/SceneInputStream.h>
#include <hgr/SceneOutputStream.h>
#include <hgr/TransformAnimation.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/System.h>
#include <lang/Globals.h>
#include <lang/Exception.h>
#include <lang/algorithm/sort.h>
#include <math/float3.h>
#include <math/RandomUtil.h>
#include <math/quaternion.h>
#include <lightmappacker/LightMapPacker.h>
#include <time.h>
#include <dummy.h>
#include <plugapi.h>
#include <Opcode.h>
#include <config.h>


#ifdef _DEBUG
#error maxsdk does not support debug builds, build Release only
#endif


//#define N3D_DEFAULTS


USING_NAMESPACE(gr)
USING_NAMESPACE(io)
USING_NAMESPACE(lua)
USING_NAMESPACE(img)
USING_NAMESPACE(lang)
USING_NAMESPACE(math)
using namespace lightmappacker;

static const float			LIGHTMAP_FILLER				= -1e10f;
static const TCHAR* const	SUPPORTED_EXT[]				= {"hgr"};

#ifdef N3D_DEFAULTS
ExportPlatform MySceneExport::sm_exportPlatform = EXPORTPLATFORM_N3D;
#else
ExportPlatform MySceneExport::sm_exportPlatform = EXPORTPLATFORM_DX;
#endif
MySceneExport* MySceneExport::sm_this = 0;

MySceneExport::MySceneExport( HINSTANCE instance ) :
	m_instance( instance ),
	m_dialogret( -1 ),
	m_quiet( false ),
	m_maxBonesPerPrimitive( 29 )
{
	sm_this = this;

	sprintf( m_pluginName, "%s version %s", ShortDesc(), MAXEXPORT_VERSION_NUMBER );
}

MySceneExport::~MySceneExport()
{
}

int	MySceneExport::ExtCount()
{
	return sizeof(SUPPORTED_EXT) / sizeof(SUPPORTED_EXT[0]);
}

const TCHAR* MySceneExport::Ext( int i )
{
	if ( i >= 0 && i < ExtCount() )
		return SUPPORTED_EXT[i];
	else
		return "";
}

const TCHAR* MySceneExport::LongDesc()
{
	return _T("hgr scene export plugin");
}

const TCHAR* MySceneExport::ShortDesc()
{
	return _T("hgr export");
}

const TCHAR* MySceneExport::AuthorName()
{
	return _T("Jani Kajala");
}

const TCHAR* MySceneExport::CopyrightMessage()
{
	return _T("Copyright (C) 2005 Jani Kajala");
}

const TCHAR* MySceneExport::OtherMessage1()
{
	return _T("");
}

const TCHAR* MySceneExport::OtherMessage2()
{
	return _T("");
}

unsigned int MySceneExport::Version()
{
	int maj=0,min=0,fix=0;
	sscanf( MAXEXPORT_VERSION_NUMBER, "%d.%d.%d", &maj, &min, &fix );
	return maj*100+min;
}

BOOL MySceneExport::SupportsOptions( int ext, DWORD options )
{
	return (SCENE_EXPORT_SELECTED & options) != 0;
}

void MySceneExport::ShowAbout( HWND hwnd )
{
	char cap[200];
	sprintf( cap, "About %s...", m_pluginName );
	MessageBox( hwnd, CopyrightMessage(), cap, MB_OK );
}

bool MySceneExport::flushWin32MsgQueue()
{
	MSG msg;
	if ( PeekMessage(&msg,0,0,0,PM_NOREMOVE) )
	{
		while ( WM_QUIT != msg.message &&
			PeekMessage(&msg,0,0,0,PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		return WM_QUIT != msg.message;
	}
	return true;
}

BOOL CALLBACK MySceneExport::dlgproc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	MySceneExport* myexp = reinterpret_cast<MySceneExport*>( GetWindowLongPtr(hwnd,GWLP_USERDATA) );

	switch ( msg ) 
	{
	case WM_INITDIALOG:{
		myexp = reinterpret_cast<MySceneExport*>(lp);
		myexp->m_dialogret = -1;
		SetWindowLongPtr( hwnd, GWLP_USERDATA, lp );
		CenterWindow( hwnd, GetParent(hwnd) );

		char captiontext[100];
		sprintf( captiontext, "Options (%s)", myexp->m_pluginName );
		SetWindowText( hwnd, captiontext );

		for ( HashtableIterator< int, DialogItem<bool> > it = myexp->m_dialogBooleans.begin() ; it != myexp->m_dialogBooleans.end() ; ++it )
			CheckDlgButton( hwnd, it.key(), it.value().value ); 
		for ( HashtableIterator< int, DialogItem<float> > it = myexp->m_dialogNumbers.begin() ; it != myexp->m_dialogNumbers.end() ; ++it )
			SetWindowText( GetDlgItem(hwnd,it.key()), Format("{0}",it.value().value).format().c_str() ); 
		for ( HashtableIterator< int, DialogItem<String> > it = myexp->m_dialogStrings.begin() ; it != myexp->m_dialogStrings.end() ; ++it )
			SetWindowText( GetDlgItem(hwnd,it.key()), Format("{0}",it.value().value).format().c_str() ); 

		// setup platform selection list box
		HWND listbox = GetDlgItem( hwnd, IDC_PLATFORMLIST );
		SendMessage( listbox, LB_RESETCONTENT, 0, 0 );
		for ( int i = 0 ; i < EXPORTPLATFORM_COUNT ; ++i )
			SendMessage( listbox, LB_ADDSTRING, 0, (LPARAM)getExportPlatformName((ExportPlatform)i) );
		int platformsel = (int)myexp->sm_exportPlatform;
		SendMessage( listbox, LB_SETCURSEL, platformsel, 0 );

		// set ok as default
		HWND okbutton = GetDlgItem(hwnd,IDOK);
		assert( okbutton );
		SetActiveWindow( okbutton );
		return FALSE;}

	case WM_COMMAND:
		switch ( LOWORD(wp) ) 
		{
		case IDOK:{
			for ( HashtableIterator< int, DialogItem<bool> > it = myexp->m_dialogBooleans.begin() ; it != myexp->m_dialogBooleans.end() ; ++it )
				it.value().value = IsDlgButtonChecked(hwnd,it.key()) != 0;

			for ( HashtableIterator< int, DialogItem<float> > it = myexp->m_dialogNumbers.begin() ; it != myexp->m_dialogNumbers.end() ; ++it )
			{
				TCHAR sz[256];
				GetWindowText( GetDlgItem(hwnd,it.key()), sz, sizeof(sz)/sizeof(sz[0])-1 );

				if ( 1 != sscanf( sz, "%g", &it.value().value ) )
				{
					MessageBox( hwnd, Format("{0} expects number (\"{1}\" is not)", it.value().name, sz).format().c_str(), "Error", MB_OK );
					return FALSE;
				}
			}

			for ( HashtableIterator< int, DialogItem<String> > it = myexp->m_dialogStrings.begin() ; it != myexp->m_dialogStrings.end() ; ++it )
			{
				TCHAR sz[512];
				GetWindowText( GetDlgItem(hwnd,it.key()), sz, sizeof(sz)/sizeof(sz[0])-1 );
				it.value().value = sz;
			}

			Format err;
			if ( !validateDialogItems(myexp->m_dialogBooleans,&err) ||
				!validateDialogItems(myexp->m_dialogNumbers,&err) ||
				!validateDialogItems(myexp->m_dialogStrings,&err) )
			{
				MessageBox( hwnd, err.format().c_str(), "Invalid option", MB_OK );
				return FALSE;
			}

			// get platform selection
			HWND listbox = GetDlgItem( hwnd, IDC_PLATFORMLIST );
			int platformsel = SendMessage( listbox, LB_GETCURSEL, 0, 0 );
			myexp->sm_exportPlatform = (ExportPlatform)platformsel;

			myexp->m_dialogret = 1;
			break;}

		case IDCANCEL:
			myexp->m_dialogret = 0;
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

int	MySceneExport::DoExport( const TCHAR* namesz, ExpInterface* ei, Interface* ip, BOOL quiet, DWORD options )
{
	try
	{
		sm_this = this;
		m_quiet = (quiet != 0);
		m_outputFilename = String(namesz).toLowerCase();
		String configname = m_outputFilename + ".cfg";

		Debug::printf( "--------------------------------------------------------------------------\n" );
		Debug::printf( "%s, exporting %s\n", m_pluginName, namesz );
		Debug::printf( "--------------------------------------------------------------------------\n" );

		// remove old files
		remove( getPostfixedOutputFilename("_lines.txt").c_str() );

		// load dialog options
		setOptionDefaults();
		try
		{
			LuaState lua;
			LuaTable config( &lua );
			FileInputStream configin( configname );
			config.read( &configin );
			getOptions( config );
		}
		catch ( Throwable& e )
		{
			Debug::printf( "Error (%s) while loading configuration, resetting to defaults\n", e.getMessage().format().c_str() );
			setOptionDefaults();
		}

		// ask export options
		if ( !m_quiet ) 
		{
			HWND dlg = CreateDialogParam( m_instance, MAKEINTRESOURCE(IDD_DIALOG1), ip->GetMAXHWnd(), dlgproc, (LPARAM)this );
			assert( dlg );
			ShowWindow( dlg, SW_NORMAL );

			while( flushWin32MsgQueue() && -1 == m_dialogret )
				Sleep( 10 );

			DestroyWindow( dlg );
			if ( !m_dialogret )
				return 1;
		}

		// DEBUG: print dialog selections
		Debug::printf( "Selected export options (exporting to %s):\n", getExportPlatformName(sm_exportPlatform) );
		printDialogItems( "boolean", m_dialogBooleans );
		printDialogItems( "number", m_dialogNumbers );
		printDialogItems( "string", m_dialogStrings );

		// save dialog options
		LuaState lua;
		LuaTable config( &lua );
		for ( HashtableIterator< int, DialogItem<bool> > it = m_dialogBooleans.begin() ; it != m_dialogBooleans.end() ; ++it )
			config.setMember( it.value().name, it.value().value );
		for ( HashtableIterator< int, DialogItem<float> > it = m_dialogNumbers.begin() ; it != m_dialogNumbers.end() ; ++it )
			config.setMember( it.value().name, it.value().value );
		for ( HashtableIterator< int, DialogItem<String> > it = m_dialogStrings.begin() ; it != m_dialogStrings.end() ; ++it )
			config.setMember( it.value().name, it.value().value );
		config.setMember( "ExportPlatform", (float)sm_exportPlatform );
		FileOutputStream configout( configname );
		config.write( &configout );

		// export scene
		Debug::printf( "Extracting scene...\n" );
		int time = System::currentTimeMillis();
		m_progress = new ProgressDialog( m_instance, ip->GetMAXHWnd(), m_pluginName );
        extractScene( ei, ip, options );
		if ( m_nodes.size() == 0 )
			throwError( Exception( Format("Nothing to export!") ) );

		// write scene
		Debug::printf( "Writing scene...\n" );
		writeScene( ei, ip, options );
		m_progress->destroy();
		time = System::currentTimeMillis()-time;

		Debug::printf( "--------------------------------------------------------------------------\n" );
		Debug::printf( "%s exported in %g seconds\n", m_outputFilename.c_str(), 1e-3f*float(time) );
		Debug::printf( "--------------------------------------------------------------------------\n" );
	}
	catch ( SilentQuit& )
	{
		if ( m_progress != 0 )
			m_progress->destroy();

		Debug::printf( "Aborted export process silently\n" );
	}
	catch ( Throwable& err )
	{
		if ( m_progress != 0 )
			m_progress->destroy();

		if ( !m_quiet )
		{
			char cap[100];
			sprintf( cap, "Error - %s", m_pluginName );

			char buff[1024];
			err.getMessage().format( buff, sizeof(buff) );

			MessageBox( ip->GetMAXHWnd(), buff, cap, MB_OK|MB_ICONERROR );
		}
	}
	catch ( ... )
	{
		Debug::printf( "Unknown exception!\n" );
		if ( !m_quiet )
		{
			char cap[100];
			sprintf( cap, "Error - %s", m_pluginName );

			MessageBox( ip->GetMAXHWnd(), "Unknown exception!", "Error", MB_OK|MB_ICONERROR );
		}
	}
	cleanup();
	return 1;
}

void MySceneExport::getOptions( LuaTable& tab )
{
	for ( HashtableIterator< int, DialogItem<bool> > it = m_dialogBooleans.begin() ; it != m_dialogBooleans.end() ; ++it )
		it.value().value = tab.getBoolean( it.value().name );
	for ( HashtableIterator< int, DialogItem<float> > it = m_dialogNumbers.begin() ; it != m_dialogNumbers.end() ; ++it )
		it.value().value = tab.getNumber( it.value().name );
	for ( HashtableIterator< int, DialogItem<String> > it = m_dialogStrings.begin() ; it != m_dialogStrings.end() ; ++it )
		it.value().value = tab.getString( it.value().name );

	sm_exportPlatform = (ExportPlatform)(int)tab.getNumber( "ExportPlatform" );
}

void MySceneExport::setOptionDefaults()
{
	// set default export options
	m_dialogBooleans[IDC_PARSEIDSFROMNAMES] = DialogItem<bool>( "ParseIDsFromNames", false );
	m_dialogBooleans[IDC_FOGENABLED] = DialogItem<bool>( "FogEnabled", false );
	m_dialogBooleans[IDC_ANIMATIONONLY] = DialogItem<bool>( "AnimationOnly", false );
	m_dialogBooleans[IDC_OPTIMIZEANIM] = DialogItem<bool>( "OptimizeAnim", true );
	m_dialogBooleans[IDC_SCALEWORLDUNITSTOMETERS] = DialogItem<bool>( "ScaleWorldUnitsToMeters", false );
	m_dialogBooleans[IDC_COPYTEXTURES] = DialogItem<bool>( "CopyTextures", true );
	m_dialogBooleans[IDC_EXPORTVERTEXCOLORS] = DialogItem<bool>( "ExportVertexColors", false );
	m_dialogBooleans[IDC_COMPUTELIGHTMAPS] = DialogItem<bool>( "ComputeLightMaps", false );
	m_dialogBooleans[IDC_COMPUTEOCCLUSIONMAPS] = DialogItem<bool>( "ComputeOcclusionMaps", false );
	m_dialogNumbers[IDC_ANIMATIONQUALITY] = DialogItem<float>( "AnimationQuality", 5.f, new DialogNumberValidator<float>(0,100) );
	m_dialogNumbers[IDC_ANIMATIONQUALITY2] = DialogItem<float>( "AnimationSampleRate", 30.f, new DialogNumberValidator<float>(1,30) );
	m_dialogNumbers[IDC_FOGSTART] = DialogItem<float>( "FogStart", -20.f, 0 );
	m_dialogNumbers[IDC_FOGEND] = DialogItem<float>( "FogEnd", 150.f, 0 );
	m_dialogNumbers[IDC_LMAX] = DialogItem<float>( "UnoccludedDistance", 1.f, 0 );
	m_dialogNumbers[IDC_TEXELSIZE] = DialogItem<float>( "TexelSize", 0.20f, new DialogNumberValidator<float>(0.001f,100.f) );
	m_dialogNumbers[IDC_RAYSTEXEL] = DialogItem<float>( "TexelRays", 200.f, new DialogNumberValidator<float>(10.f,2000.f) );
	//m_dialogNumbers[IDC_MAXBONESPERPRIMITIVE] = DialogItem<float>( "Max bones per primitive", sm_maxBonesPerPrimitive, new DialogNumberValidator<float>(12,256) );
#ifdef N3D_DEFAULTS
	m_dialogBooleans[IDC_SAVETEXTURESAS] = DialogItem<bool>( "SaveTexturesAs", true );
	m_dialogStrings[IDC_SHADER_NOTEX] = DialogItem<String>( "DefaultPlainShader", "unlit-plain", 0 );
	m_dialogStrings[IDC_SHADER_DIFFTEX] = DialogItem<String>( "DefaultDiffuseTextureShader", "unlit-tex", 0 );
	m_dialogStrings[IDC_SHADER_DIFFTEXALPHA] = DialogItem<String>( "DefaultDiffuseTextureAlphaShader", "unlit-tex", 0 );
	m_dialogStrings[IDC_SHADER_BUMPTEX] = DialogItem<String>( "DefaultBumpTextureShader", "unlit-tex", 0 );
	m_dialogStrings[IDC_SHADER_BUMPREFL] = DialogItem<String>( "DefaultBumpReflShader", "unlit-tex", 0 );
	m_dialogStrings[IDC_TEXTUREEXTENSION] = DialogItem<String>( "TextureExtension", "tga", 0 );
#else
	m_dialogBooleans[IDC_SAVETEXTURESAS] = DialogItem<bool>( "SaveTexturesAs", false );
	m_dialogStrings[IDC_SHADER_NOTEX] = DialogItem<String>( "DefaultPlainShader", "diff-plain", 0 );
	m_dialogStrings[IDC_SHADER_DIFFTEX] = DialogItem<String>( "DefaultDiffuseTextureShader", "diff-tex", 0 );
	m_dialogStrings[IDC_SHADER_DIFFTEXALPHA] = DialogItem<String>( "DefaultDiffuseTextureAlphaShader", "diff-tex-alpha", 0 );
	m_dialogStrings[IDC_SHADER_BUMPTEX] = DialogItem<String>( "DefaultBumpTextureShader", "bump", 0 );
	m_dialogStrings[IDC_SHADER_BUMPREFL] = DialogItem<String>( "DefaultBumpReflShader", "bump-refl", 0 );
	m_dialogStrings[IDC_TEXTUREEXTENSION] = DialogItem<String>( "TextureExtension", "dds", 0 );
#endif
	m_dialogStrings[IDC_TEXTUREPATH] = DialogItem<String>( "TexturePath", "", 0 );
}

bool MySceneExport::getOptionBoolean( const String& name )
{
	for ( HashtableIterator< int, DialogItem<bool> > it = m_dialogBooleans.begin() ; it != m_dialogBooleans.end() ; ++it )
		if ( it.value().name == name )
			return it.value().value;
	throwError( Exception( Format("No such boolean option as {0}", name) ) );
	return false;
}

float MySceneExport::getOptionNumber( const String& name )
{
	for ( HashtableIterator< int, DialogItem<float> > it = m_dialogNumbers.begin() ; it != m_dialogNumbers.end() ; ++it )
		if ( it.value().name == name )
			return it.value().value;
	throwError( Exception( Format("No such number option as {0}", name) ) );
	return 0.f;
}

String MySceneExport::getOptionString( const String& name )
{
	for ( HashtableIterator< int, DialogItem<String> > it = m_dialogStrings.begin() ; it != m_dialogStrings.end() ; ++it )
		if ( it.value().name == name )
			return it.value().value;
	throwError( Exception( Format("No such string option as {0}", name) ) );
	return "";
}

void MySceneExport::cleanup()
{
	Debug::printf( "Cleaning up exporter\n" );

	m_lmp = 0;
	m_userPropertySet = 0;
	m_progress = 0;
	m_outputFilename = String();

	m_dialogret = -1;
	m_dialogBooleans.clear();
	m_dialogNumbers.clear();
	m_dialogStrings.clear();

	m_cameras.clear();
	m_shapes.clear();
	m_dummies.clear();
	m_lights.clear();
	m_meshes.clear();
	m_others.clear();
	m_nodes.clear();
	m_materials.clear();
	m_textures.clear();
	m_primitives.clear();

	NS(hgr,Globals)::cleanup();
	lang_Globals::cleanup();
}

void MySceneExport::extractScene( ExpInterface* ei, Interface* ip, DWORD options )
{
	WorldUnitsRescale worldunitsrescale( ip, UNITS_METERS, m_dialogBooleans[IDC_SCALEWORLDUNITSTOMETERS].value );
	Interval animrange = ip->GetAnimRange();

	// extract nodes
	m_progress->setText( "Extracting nodes..." );
	Array<INode*> nodes3ds;
	Array<Mtl*> materials3ds;
	INodeUtil::getNodesToExport( ip, options, nodes3ds );
	m_nodes.clear();
	for ( int i = 0 ; i < nodes3ds.size() ; ++i )
	{
		m_progress->setText( Format("Extracting node \"{0}\"...", String(nodes3ds[i]->GetName()?nodes3ds[i]->GetName():"")).format().c_str() );
		m_progress->setProgress( float(i)/float(nodes3ds.size()) );

		INode* node3ds = nodes3ds[i];
		ObjectState objstate = node3ds->EvalWorldState( 0 );

		if ( 0 != objstate.obj )
		{
			if ( SHAPE_CLASS_ID == objstate.obj->SuperClassID() )
			{
				m_shapes.add( new MyShape(node3ds) );
				m_nodes.add( m_shapes.last() );
			}
			else if ( INodeUtil::isExportableGeometry(node3ds,objstate) )
			{
				m_meshes.add( new MyMesh(node3ds) );
				m_nodes.add( m_meshes.last() );
			}
			else if ( objstate.obj->SuperClassID() == CAMERA_CLASS_ID )
			{
				m_cameras.add( new MyCamera(node3ds) );
				m_nodes.add( m_cameras.last() );
			}
			else if ( objstate.obj->SuperClassID() == LIGHT_CLASS_ID )
			{
				m_lights.add( new MyLight(node3ds) );
				m_nodes.add( m_lights.last() );
			}
			else if ( objstate.obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
			{
				m_dummies.add( new MyDummy(node3ds) );
				m_nodes.add( m_dummies.last() );
			}
			else
			{
				m_others.add( new MyNode(node3ds,new hgr::Node) );
				m_nodes.add( m_others.last() );
			}
		}
	}

	// warn about duplicate names
	Array<String> names;
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
		names.add( m_nodes[i]->name() );
	LANG_SORT( names.begin(), names.end() );
	Array<String> dupnames;
	for ( int i = 1 ; i < names.size() ; ++i )
	{
		if ( names[i] == names[i-1] && names[i].length() > 0 )
			dupnames.add( names[i] );
	}
	if ( dupnames.size() > 0 )
	{
		dupnames.resize( std::unique( dupnames.begin(), dupnames.end() ) - dupnames.begin() );
		String msg = "Multiple objects have identical names: ";
		for ( int i = 0 ; i < dupnames.size() ; ++i )
		{
			if ( i+1 == dupnames.size() )
				msg = msg + dupnames[i];
			else
				msg = msg + dupnames[i] + ", ";
		}
		int res = MessageBox( ip->GetMAXHWnd(), msg.c_str(), "Warning: Identical object names", MB_OKCANCEL );
		if ( res == IDCANCEL )
			throwError( SilentQuit() );
	}

	// extract user properties
	m_userPropertySet = new hgr::UserPropertySet;
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
	{
		MyNode* node = m_nodes[i];
		if ( node->userprop.length() > 0 )
			m_userPropertySet->put( node->name(), node->userprop );
	}

	// extract materials and copy textures
	if ( !m_dialogBooleans[IDC_ANIMATIONONLY].value )
	{
		// extract material users
		m_progress->setText( "Extracting materials..." );
		m_progress->setProgress( 0.f );
		Array<Mtl*> mtls;
		Array< Array<Mtl*> > meshmtls( m_meshes.size() );
		for ( int i = 0 ; i < m_meshes.size() ; ++i )
		{
			m_meshes[i]->extractMtls( meshmtls[i] );
			for ( int k = 0 ; k < meshmtls[i].size() ; ++k )
				mtls.add( meshmtls[i][k] );

			Debug::printf( "Mesh \"%s\" materials:\n", m_meshes[i]->name().c_str() );
			for ( int k = 0 ; k < meshmtls[i].size() ; ++k )
				Debug::printf( "    \"%s\"\n", (char*)meshmtls[i][k]->GetName() );
		}
		ArrayUtil::removeDuplicates( mtls );
		/*Debug::printf( "Scene uses materials (%d):\n", mtls.size() );
		for ( int i = 0 ; i < mtls.size() ; ++i )
			Debug::printf( "  %s\n", (char*)mtls[i]->GetName() );*/

		// extract materials
		m_materials.clear();
		Array<MyMesh*> meshusers;
		for ( int i = 0 ; i < mtls.size() ; ++i )
		{
			m_progress->setProgress( float(i)/float(mtls.size()) );

			// list user meshes
			meshusers.clear();
			assert( meshmtls.size() == m_meshes.size() );
			for ( int k = 0 ; k < meshmtls.size() ; ++k )
			{
				if ( meshmtls[k].indexOf(mtls[i]) != -1 )
				{
					assert( k >= 0 && k < m_meshes.size() );
					assert( m_meshes[k] != 0 );
					meshusers.add( m_meshes[k] );
				}
			}

			P(MyMaterial) mtl = new MyMaterial(mtls[i],i,ip,this,meshusers);
			m_materials.add( mtl );
		}

		// set material flags
		for ( int i = 0 ; i < m_materials.size() ; ++i )
		{
			MyMaterial* mtl = m_materials[i];

			if ( m_dialogBooleans[IDC_COMPUTELIGHTMAPS].value ||
				m_dialogBooleans[IDC_COMPUTEOCCLUSIONMAPS].value )
			{
				mtl->flags |= MyMaterial::SHADER_LIGHTMAPPING;
			}
		}

		// extract textures
		m_textures.clear();
		for ( int i = 0 ; i < m_materials.size() ; ++i )
		{
			MyMaterial* mat = m_materials[i];
			for ( int i = 0 ; i < mat->textures() ; ++i )
			{
				String texname = mat->getTexture(i).value;
				
				Texture tex;
				tex.filename = texname;
				if ( mat->getTexture(i).type == "REFLMAP" )
					tex.mapping = 1;
				else
					tex.mapping = 0;

				m_textures.add( tex );
			}
		}
		ArrayUtil::removeDuplicates( m_textures );

		// copy textures
		if ( m_dialogBooleans[IDC_COPYTEXTURES].value )
			copyTextures( ip );
	} // if ( !m_dialogBooleans[IDC_ANIMATIONONLY].value )

	// link parents
	m_progress->setText( "Extracting node hierarchy..." );
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(m_nodes.size()) );

		MyNode* node = m_nodes[i];
		INode* node3ds = node->node3ds();
		if ( !node3ds->IsRootNode() )
		{
			INode* parent3ds = node3ds->GetParentNode();

			// re-parent biped spine to root instead of pelvis
			// to break dependency between lower body and upper body
			if ( INodeUtil::isSplitNeeded(node3ds) )
			{
				parent3ds = parent3ds->GetParentNode();
				if ( 0 != parent3ds )
				{
					if ( parent3ds->IsRootNode() )
						parent3ds = 0;
					else
						parent3ds = parent3ds->GetParentNode(); // skip granpa
				}
			}

			MyNode* parent = 0;
			for ( int i = 0 ; i < m_nodes.size() ; ++i )
			{
				if ( m_nodes[i]->node3ds() == parent3ds )
				{
					parent = m_nodes[i];
					node->node()->linkTo( parent->node() );
					break;
				}
			}
		}
	}

	// bone nodes can be added to meshes only after all nodes have been created
	// since mesh node could otherwise require bone node before it's created
	m_progress->setText( "Extracting bones..." );
	for ( int i = 0 ; i < m_meshes.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(m_meshes.size()) );
		m_meshes[i]->extractBones( m_nodes );
	}

	// extract animations
	m_progress->setText( "Extracting animations..." );
	
	int samplerate = int( m_dialogNumbers[IDC_ANIMATIONQUALITY2].value + 0.5f );
	TimeValue sampleinterval = TimeValue( float(TIME_TICKSPERSEC) / samplerate );
	samplerate = int( float(TIME_TICKSPERSEC) / float(sampleinterval) );
	
	int samplekeys = (animrange.End()-animrange.Start()) / sampleinterval;
	if ( samplekeys < 1 )
		samplekeys = 1;
	
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
		m_nodes[i]->tmanim = new hgr::TransformAnimation( hgr::TransformAnimation::BEHAVIOUR_REPEAT, samplekeys, samplekeys, samplekeys, samplerate, samplerate, samplerate );

	TimeValue time = animrange.Start();
	TimeValue ticksperframe = GetTicksPerFrame();
	for ( int keyix = 0 ; keyix < samplekeys ; ++keyix )
	{
		m_progress->setProgress( float(keyix)/float(samplekeys) );
		
		for ( int nodeix = 0 ; nodeix < m_nodes.size() ; ++nodeix )
		{
			MyNode* node = m_nodes[nodeix];
			NS(math,float3x4) tm = INodeUtil::getModelTransformLH( node->node3ds(), time );
			
			node->tmanim->setPositionKey( keyix, tm.translation() );
			
			NS(math,float3x3) rot = tm.rotation();
			node->tmanim->setRotationKey( keyix, math::quaternion(rot.orthonormalize()) );
			
			NS(math,float3) scl;
			for ( int i = 0 ; i < 3 ; ++i )
				scl[i] = dot( rot.getColumn(i), normalize0(rot.getColumn(i)) );
			node->tmanim->setScaleKey( keyix, scl );
		}

		time += sampleinterval;
	}

	// optimize animations
	m_progress->setText( "Optimizing animations..." );
	float animoptimizationquality = m_dialogNumbers[IDC_ANIMATIONQUALITY].value / 100.f;
	bool optimizeanim = m_dialogBooleans[IDC_OPTIMIZEANIM].value;
	Debug::printf( "Resampling animations using quality %g\n", animoptimizationquality );
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(m_nodes.size()) );

		MyNode* node = m_nodes[i];

		int oldposkeys = node->tmanim->positionKeys();
		int oldrotkeys = node->tmanim->rotationKeys();
		int oldsclkeys = node->tmanim->scaleKeys();
		if ( optimizeanim )
		{
			node->tmanim->optimize( animoptimizationquality );
		}
		else
		{
			node->tmanim->removeRedundantKeys();
		}

		Debug::printf( "Optimized animation of %s:\n", node->name().c_str() );
		Debug::printf( "    pos %d -> %d\n", oldposkeys, node->tmanim->positionKeys() );
		Debug::printf( "    rot %d -> %d\n", oldrotkeys, node->tmanim->rotationKeys() );
		Debug::printf( "    scl %d -> %d\n", oldsclkeys, node->tmanim->scaleKeys() );
	}

	if ( !m_dialogBooleans[IDC_ANIMATIONONLY].value )
	{
		// extract geometry
		m_progress->setText( "Extracting geometry..." );
		for ( int i = 0 ; i < m_meshes.size() ; ++i )
		{
			m_progress->setText( Format("Extracting \"{0}\" geometry...", m_meshes[i]->node()->name()).format() );
			m_progress->setProgress( float(i)/float(m_meshes.size()) );

			m_meshes[i]->extractGeometry( m_materials, 
				(m_dialogBooleans[IDC_EXPORTVERTEXCOLORS].value ? MyMesh::EXPORT_VERTEX_COLORS : MyMesh::DONT_EXPORT_VERTEX_COLORS) );
		}

		// prepare lightmaps
		if ( m_dialogBooleans[IDC_COMPUTELIGHTMAPS].value ||
			m_dialogBooleans[IDC_COMPUTEOCCLUSIONMAPS].value )
		{
			packLightmaps();
			assignLightmaps();
			computeLightmapData();
			saveLightmaps();
		}
		
		// extract primitives
		m_progress->setText( "Optimizing geometry..." );
		for ( int i = 0 ; i < m_meshes.size() ; ++i )
		{
			m_progress->setProgress( float(i)/float(m_meshes.size()) );
			
			//m_meshes[i]->extractPrimitives( (int)m_dialogNumbers[IDC_MAXBONESPERPRIMITIVE].value );
			m_meshes[i]->extractPrimitives( m_maxBonesPerPrimitive );

			for ( int k = 0 ; k < m_meshes[i]->primitives() ; ++k )
				m_primitives.add( m_meshes[i]->getPrimitive(k) );
		}
		ArrayUtil::removeDuplicates( m_primitives );
	} // if ( !m_dialogBooleans[IDC_ANIMATIONONLY].value )

	// generate shading textures
	/*for ( int k = 0 ; k < m_generateShadingTextures.size() ; ++k )
	{
		float shininess = m_generateShadingTextures[k];
		String filename = generateShadingTexture( shininess );
		int w = 256;
		int h = 256;
		Array<uint32_t> data( w*h );
		for ( int j = 0 ; j < h ; ++j )
		{
			for ( int i = 0 ; i < w ; ++i )
			{
				float NdotL = float(i)/float(w);
				float NdotH = float(j)/float(h);
				float diffuse = NdotL;
				float specular = Math::pow(NdotH, shininess);
				data[j*w+i] = ColorUtil::rgb32f( diffuse, diffuse, diffuse, specular );
			}
		}

		img::ImageWriter::write( getTextureName(filename), SurfaceFormat::SURFACE_A8R8G8B8, data.begin(), w, h, w*4, SurfaceFormat::SURFACE_A8R8G8B8, 0, SurfaceFormat() );
	}*/
}

String MySceneExport::fixTexturePath( String filename, Interface* ip )
{
	// fix texture paths
	PathName maxpath = String( ip->GetCurFilePath() );
	//for ( int i = 0 ; i < m_textures.size() ; ++i )
	{
		Texture tex;
		tex.filename = filename;
		
		Debug::printf( "Locating texture %s\n", tex.filename.c_str() );
		if ( !FindFile::isFileExist(tex.filename,FindFile::FIND_FILESONLY) )
		{
			if ( tex.filename.length() > 3 && tex.filename.charAt(1) == ':' )
			{
				tex.filename = tex.filename.substring(2);
				tex.filename = PathName( maxpath.parent().toString(), tex.filename ).toString();
			}

			Debug::printf( "- Checking location %s\n", tex.filename.c_str() );
			if ( !FindFile::isFileExist(tex.filename,FindFile::FIND_FILESONLY) )
			{
				throwError( Exception(Format("Cannot locate texture {0}", chopFilename(tex.filename))) );
			}
		}

		return tex.filename;
	}
}

void MySceneExport::copyTextures( Interface* ip )
{
	m_progress->setText( "Copying textures..." );
	m_progress->setProgress( 0.f );

	for ( int i = 0 ; i < m_textures.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(m_textures.size()) );

		String src = fixTexturePath( m_textures[i].filename, ip );
		String dst = getTextureName( getOutputFilename(src) );

		if ( m_dialogBooleans[IDC_SAVETEXTURESAS].value &&
			strcmp(PathName(dst.toLowerCase()).suffix(), PathName(src.toLowerCase()).suffix()) )
		{
			String ext = m_dialogStrings[IDC_TEXTUREEXTENSION].value.toLowerCase();
			String newname = dst;
			int ix = dst.lastIndexOf(".");
			if ( ix != -1 )
				newname = dst.substring(0,ix+1);
			newname = newname + ext;
			Debug::printf( "Saving texture \"%s\" as \"%s\"\n", dst.c_str(), newname.c_str() );

			// convert to <ext>
			try
			{
				Array<uint8_t> buf;
				FileInputStream in( src );
				ImageReader rd( &in, ImageReader::guessFileFormat(src) );
				int pitch = rd.surfaceWidth()*rd.format().bitsPerPixel()/8;
				buf.resize( rd.surfaceHeight()*pitch );
				rd.readSurface( &buf[0], pitch, rd.surfaceWidth(), rd.surfaceHeight(), rd.format(), 0, SurfaceFormat() );
				ImageWriter::write( dst, rd.format(), &buf[0], rd.surfaceWidth(), rd.surfaceHeight(), pitch, rd.format(), 0, SurfaceFormat() );
			}
			catch ( Throwable& e )
			{
				char msg[256];
				e.getMessage().format( msg, sizeof(msg) );
				Debug::printf( "ERROR: %s\n", msg );
				if ( !m_quiet )
				{
					char cap[100];
					sprintf( cap, "Error while copying file - %s", m_pluginName );
					int res = MessageBox( ip->GetMAXHWnd(), msg, cap, MB_OKCANCEL|MB_ICONERROR );
					if ( res == IDCANCEL )
						throwError( SilentQuit() );
				}
			}
		}
		else
		{
			// copy file as is
			if ( !CopyFile(src.c_str(),dst.c_str(),FALSE) )
			{
				char msg[_MAX_PATH*2+200];
				sprintf( msg, "Failed to copy \"%s\" to \"%s\".\nTry to render scene in 3dsmax and export again, please. (3dsmax might have not updated texture paths if for example the scene was created on another computer)", chopFilename(src).c_str(), chopFilename(dst).c_str() );
				Debug::printf( "ERROR: %s\n", msg );

				char cap[100];
				sprintf( cap, "Error while copying file - %s", m_pluginName );
				if ( !m_quiet )
				{
					int res = MessageBox( ip->GetMAXHWnd(), msg, cap, MB_OKCANCEL|MB_ICONERROR );
					if ( res == IDCANCEL )
						throwError( SilentQuit() );
				}
			}
		}
	}
}

static int getDigit( int ch )
{
	switch ( ch )
	{
	case '0':	return 0;
	case '1':	return 1;
	case '2':	return 2;
	case '3':	return 3;
	case '4':	return 4;
	case '5':	return 5;
	case '6':	return 6;
	case '7':	return 7;
	case '8':	return 8;
	case '9':	return 9;
	default:	return -1;
	}
}

static bool parseIDFromName( hgr::Node* node )
{
	const String& name = node->name();

	if ( name.length() < 3 )
		return false;

	int d1 = getDigit( name.charAt(0) );
	int d2 = getDigit( name.charAt(1) );
	int d3 = getDigit( name.charAt(2) );
	if ( d1 < 0 || d2 < 0 || d3 < 0 )
		return false;
	int id = d1*100 + d2*10 + d3;

	node->setID( id );
	return true;
}

static int getExporterVersionInt()
{
	int major, minor, build;
	int parsed = sscanf("2.9.4","%d.%d.%d",&major,&minor,&build);
	assert( parsed == 3 );
	return (major<<16) + (minor<<8) + (build);
}

void MySceneExport::writeScene( ExpInterface* ei, Interface* ip, DWORD options )
{
	assert( m_nodes.size() > 0 );

	m_progress->setText( Format("Writing scene to {0}", m_outputFilename).format() );

	// reorder to nodes to {meshes, cameras, lights, dummies, shapes, other nodes}
	assert( m_nodes.size() == m_meshes.size()+m_cameras.size()+m_lights.size()+m_dummies.size()+m_shapes.size()+m_others.size() );
	m_nodes.clear();
	for ( int i = 0 ; i < m_meshes.size() ; ++i )
		m_nodes.add( m_meshes[i] );
	for ( int i = 0 ; i < m_cameras.size() ; ++i )
		m_nodes.add( m_cameras[i] );
	for ( int i = 0 ; i < m_lights.size() ; ++i )
		m_nodes.add( m_lights[i] );
	for ( int i = 0 ; i < m_dummies.size() ; ++i )
		m_nodes.add( m_dummies[i] );
	for ( int i = 0 ; i < m_shapes.size() ; ++i )
		m_nodes.add( m_shapes[i] );
	for ( int i = 0 ; i < m_others.size() ; ++i )
		m_nodes.add( m_others[i] );

	// parse IDs from names (if any)
	bool parseids = m_dialogBooleans[IDC_PARSEIDSFROMNAMES].value;
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
	{
		bool ok = parseIDFromName( m_nodes[i]->node() );
		if ( !ok && parseids )
			throwError( Exception( Format("3 character ID number prefix missing from name of object \"{0}\"", m_nodes[i]->name()) ) );
	}

	// find out what data is written to the file
	int dataflags = -1;
	if ( m_dialogBooleans[IDC_ANIMATIONONLY].value )
		dataflags = hgr::SceneInputStream::DATA_ANIMATIONS;

	P(FileOutputStream) file = new FileOutputStream( m_outputFilename );
	P(hgr::SceneOutputStream) out = new hgr::SceneOutputStream( file, dataflags, (Context::PlatformType)sm_exportPlatform, getExporterVersionInt() );

	writeSceneParams( out, ip );
	
	int checkid = 0x12345600;
	out->writeInt( checkid++ );
	writeMaterials( out );

	out->writeInt( checkid++ );
	writePrimitives( out );

	out->writeInt( checkid++ );
	writeMeshes( out );

	out->writeInt( checkid++ );
	writeCameras( out );

	out->writeInt( checkid++ );
	writeLights( out );

	out->writeInt( checkid++ );
	writeDummies( out );

	out->writeInt( checkid++ );
	writeShapes( out );

	out->writeInt( checkid++ );
	writeOtherNodes( out );

	out->writeInt( checkid++ );
	writeTransformInterpolators( out );

	out->writeInt( checkid++ );
	writeUserProperties( out );
}

void MySceneExport::writeSceneParams( hgr::SceneOutputStream* out, Interface* ip )
{
	// fog
	out->writeByte( m_dialogBooleans[IDC_FOGENABLED].value ? 1 : 0 );
	out->writeFloat( m_dialogNumbers[IDC_FOGSTART].value );
	out->writeFloat( m_dialogNumbers[IDC_FOGEND].value );

	Interval valid;
	Point3 bg = ip->GetBackGround(0,valid);
	out->writeFloat( bg.x );
	out->writeFloat( bg.y );
	out->writeFloat( bg.z );
}

void MySceneExport::writeMaterials( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_MATERIALS) )
	{
		writeEmptyList( out );
		writeEmptyList( out );
		return;
	}

	const Array<P(MyMaterial)>& array = m_materials;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} materials...", array.size()).format() );

	// write texture list
	ArrayUtil::removeDuplicates( m_textures );
	out->writeInt( m_textures.size() );
	for ( int i = 0 ; i < m_textures.size() ; ++i )
	{
		char src[_MAX_PATH];
		String::cpy( src, sizeof(src), getTextureName(m_textures[i].filename) );
		char texbasename[_MAX_FNAME];
		char texext[_MAX_EXT];
		_splitpath( src, 0, 0, texbasename, texext );
		_makepath( src, 0, 0, texbasename, texext );
		out->writeUTF( src );
		out->writeInt( m_textures[i].mapping );
	}

	// write material list
	Array<String> stdmtls;
	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );
		MyMaterial* mat = array[i];

		if ( !mat->isDxMaterial() )
			stdmtls.add( mat->name() );
		
		out->writeUTF( mat->name() );
		out->writeUTF( PathName(mat->shader()).basename() );
		out->writeInt( mat->flags );

		// write texture list
		out->writeByte( mat->textures() );
		for ( int k = 0 ; k < mat->textures() ; ++k )
		{
			int ix = m_textures.indexOf( mat->getTexture(k).value );
			assert( ix >= 0 );

			out->writeUTF( mat->getTexture(k).type );
			out->writeShort( ix );
		}

		out->writeByte( mat->vectors() );
		for ( int k = 0 ; k < mat->vectors() ; ++k )
		{
			out->writeUTF( mat->getVector(k).type );
			out->writeFloat4( mat->getVector(k).value );
		}

		out->writeByte( mat->floats() );
		for ( int k = 0 ; k < mat->floats() ; ++k )
		{
			out->writeUTF( mat->getFloat(k).type );
			out->writeFloat( mat->getFloat(k).value );
		}
	}

	// report non-DX materials
	#ifdef MAXEXPORT_HASDXMATERIAL
	if ( stdmtls.size() > 0 && !m_quiet )
	{
		String msg = "'DirectX Shader' material type is preferred to 'Standard' material.\n'Standard' material works, but it does not have all the features of 'DirectX Shader' material.\n\nFollowing materials are 'Standard' type materials:\n";
		for ( int i = 0 ; i < 10 && i < stdmtls.size() ; ++i )
			msg = msg + stdmtls[i] + "\n";
		if ( stdmtls.size() > 10 )
			msg = msg + "...";
		MessageBox( 0, msg.c_str(), "Note about scene material usage", MB_OK );
	}
	#endif
}

void MySceneExport::writePrimitives( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_PRIMITIVES) )
	{
		writeEmptyList( out );
		return;
	}

	const Array<P(MyPrimitive)>& array = m_primitives;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} primitives...", array.size()).format() );

	Array<gr::Shader*> shaders( m_materials.size() );
	for ( int i = 0 ; i < m_materials.size() ; ++i )
		shaders[i] = m_materials[i];

	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );

		int bytes = out->bytesWritten();
		out->writePrimitive( array[i], shaders );
	}
}

void MySceneExport::writeMeshes( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_NODES) )
	{
		writeEmptyList( out );
		return;
	}

	const Array<P(MyMesh)>& array = m_meshes;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} meshes...", array.size()).format() );

	Array<NS(gr,Primitive)*> primitives( m_primitives.size() );
	for ( int i = 0 ; i < m_primitives.size() ; ++i )
		primitives[i] = m_primitives[i];
	Array<hgr::Node*> nodes( m_nodes.size() );
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
		nodes[i] = m_nodes[i]->node();

	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );
		out->writeMesh( array[i]->node(), primitives, nodes );
	}
}

void MySceneExport::writeCameras( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_NODES) )
	{
		writeEmptyList( out );
		return;
	}

	const Array<P(MyCamera)>& array = m_cameras;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} cameras...", array.size()).format() );

	Array<hgr::Node*> nodes( m_nodes.size() );
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
		nodes[i] = m_nodes[i]->node();

	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );
		out->writeCamera( array[i]->node(), nodes );
	}
}

void MySceneExport::writeLights( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_NODES) )
	{
		writeEmptyList( out );
		return;
	}

	const Array<P(MyLight)>& array = m_lights;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} lights...", array.size()).format() );

	Array<hgr::Node*> nodes( m_nodes.size() );
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
		nodes[i] = m_nodes[i]->node();

	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );
		out->writeLight( array[i]->node(), nodes );
	}
}

void MySceneExport::writeDummies( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_NODES) )
	{
		writeEmptyList( out );
		return;
	}

	const Array<P(MyDummy)>& array = m_dummies;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} dummies...", array.size()).format() );

	Array<hgr::Node*> nodes( m_nodes.size() );
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
		nodes[i] = m_nodes[i]->node();

	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );
		out->writeDummy( array[i]->node(), nodes );
	}
}

void MySceneExport::writeShapes( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_NODES) )
	{
		writeEmptyList( out );
		return;
	}

	const Array<P(MyShape)>& array = m_shapes;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} shapes...", array.size()).format() );

	Array<hgr::Node*> nodes( m_nodes.size() );
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
		nodes[i] = m_nodes[i]->node();

	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );
		out->writeLines( array[i]->node(), nodes );
	}
}

void MySceneExport::writeOtherNodes( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_NODES) )
	{
		writeEmptyList( out );
		return;
	}

	const Array<P(MyNode)>& array = m_others;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} other nodes...", array.size()).format() );

	Array<hgr::Node*> nodes( m_nodes.size() );
	for ( int i = 0 ; i < m_nodes.size() ; ++i )
		nodes[i] = m_nodes[i]->node();

	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );
		out->writeNode( array[i]->node(), nodes );
	}
}

void MySceneExport::writeTransformInterpolators( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_ANIMATIONS) )
	{
		writeEmptyList( out );
		return;
	}

	const Array<MyNode*>& array = m_nodes;
	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} transform animations...", array.size()).format() );

	out->writeInt( array.size() );
	for ( int i = 0 ; i < array.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(array.size()) );
		out->writeUTF( array[i]->name() );
		out->writeTransformAnimation( array[i]->tmanim );
	}
}

void MySceneExport::writeUserProperties( hgr::SceneOutputStream* out )
{
	if ( !out->hasData(hgr::SceneInputStream::DATA_USERPROPERTIES) )
	{
		writeEmptyList( out );
		return;
	}

	int bytes = out->bytesWritten();

	m_progress->setText( Format("Writing {0} user properties...", m_userPropertySet->size()).format() );

	out->writeUserPropertySet( m_userPropertySet );
}

void MySceneExport::writeEmptyList( hgr::SceneOutputStream* out )
{
	out->writeInt( 0 );
}

String MySceneExport::getOutputFilename( const String& srcname )
{
	char src[_MAX_PATH];
	String::cpy( src, sizeof(src), srcname );
	char texbasename[_MAX_FNAME];
	char texext[_MAX_EXT];
	_splitpath( src, 0, 0, texbasename, texext );

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char basename[_MAX_FNAME];
	char ext[_MAX_EXT];
	char dst[_MAX_PATH];
	String::cpy( dst, sizeof(dst), sm_this->m_outputFilename );
	_splitpath( dst, drive, dir, basename, ext );
	
	_makepath( dst, drive, dir, texbasename, texext );

	return dst;
}

String MySceneExport::getPostfixedOutputFilename( const String& srcname )
{
	char basename2[_MAX_FNAME];
	char ext2[_MAX_EXT];
	_splitpath( srcname.c_str(), 0, 0, basename2, ext2 );

	char basename[_MAX_FNAME];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];
	char dst[_MAX_PATH];
	String::cpy( dst, sizeof(dst), sm_this->m_outputFilename );
	_splitpath( dst, drive, dir, basename, ext );
	
	strcat( basename, basename2 );

	_makepath( dst, drive, dir, basename, ext2 );
	return dst;
}

void MySceneExport::packLightmaps()
{
	m_progress->setText( "Preparing lightmap coordinates..." );

	const float		texelsize = m_dialogNumbers[IDC_TEXELSIZE].value;
	const int		mapwidth = 1024;
	const int		mapheight = 1024;

	m_lmp = new LightMapPacker( 1.f/texelsize, mapwidth, mapheight );

	for ( int i = 0 ; i < m_meshes.size() ; ++i )
	{
		MyMesh* mesh = m_meshes[i];
		assert( mesh != 0 );

		if ( !mesh->receiveShadows )
		{
			MyGeometry* geom = mesh->geometry();
			for ( int k = 0 ; k < geom->trianglelist.size() ; ++k )
			{
				ExportTriangle* tri = geom->trianglelist[k];
				tri->material = getMaterialWithFlags( tri->material, 0 );
			}
			continue;
		}

		MyGeometry* geom = mesh->geometry();
		assert( geom != 0 );
		assert( geom->uvwlayers == 2 );

		const float3x4 tm = mesh->node()->worldTransform();
		float3 v[3];

		for ( int k = 0 ; k < geom->trianglelist.size() ; ++k )
		{
			ExportTriangle* tri = geom->trianglelist[k];
			assert( tri != 0 );
			assert( tri->material != 0 );

			if ( tri->material->unlit() )
				continue;

			const int mtlx = m_materials.indexOf( tri->material );
			assert( mtlx != -1 );

			for ( int n = 0 ; n < 3 ; ++n )
			{
				assert( tri->verts[n] != 0 );
				v[n] = tm.transform( tri->verts[n]->pos );
			}

			Debug::printf( "lmp.addTriangle( %d, %d, %d, float3(%ff,%ff,%ff), float3(%ff,%ff,%ff), float3(%ff,%ff,%ff) );\n", i, k, mtlx, v[0].x, v[0].y, v[0].z, v[1].x, v[1].y, v[1].z, v[2].x, v[2].y, v[2].z );
			m_lmp->addTriangle( i, k, mtlx, v[0], v[1], v[2] );
		}
	}
	m_lmp->build( m_progress );

	// check for over-sized triangles
	if ( m_lmp->clippedTriangles().size() > 0 )
	{
		const Triangle& tri = m_lmp->clippedTriangles()[0].triangle;
		MyMesh* mesh = m_meshes[tri.mesh];
		throwError( Exception(Format("Too large triangle for lightmapping in mesh {0}", mesh->name())) );
	}
}

void MySceneExport::assignLightmaps()
{
	assert( m_lmp != 0 );
	m_progress->setText( "Assigning lightmaps..." );

	// assign lightmaps to mesh geometry triangles materials and
	// combine materials with lightmaps (and split materials if needed)
	int duplicatedmaterialcount = 0;
	const Array<Triangle>& lmptriangles = m_lmp->triangles();
	for ( int i = 0 ; i < lmptriangles.size() ; ++i )
	{
		if ( (i&0xFF) == 0 )
			m_progress->setProgress( float(i)/float(lmptriangles.size()) );

		const Triangle& lmptri = lmptriangles[i];
		MyMesh* mesh = m_meshes[ lmptri.mesh ];
		ExportTriangle* tri = mesh->geometry()->trianglelist[ lmptri.triangle ];

		for ( int k = 0 ; k < 3 ; ++k )
		{
			tri->uvw[ExportVertex::UVW_LIGHTMAP][k] = float3( lmptri.uv[k], 0.f );
			tri->uvw[ExportVertex::UVW_LIGHTMAP][k].x *= 1.f/float(lmptri.lightmap->width());
			tri->uvw[ExportVertex::UVW_LIGHTMAP][k].y *= 1.f/float(lmptri.lightmap->height());
		}

		char namebuf[1024];
		const int lightmapindex = lmptri.lightmap->index();
		String lightmapname[3];
		for ( int cs = 0 ; cs < 3 ; ++cs )
		{
			sprintf( namebuf, "L%04d_%d.dds", lightmapindex, cs );
			lightmapname[cs] = getOutputFilename( namebuf );
			lmptri.lightmap->name[cs] = lightmapname[cs];
		}

		// 1) no previous lightmap 2) same lightmap 3) different lightmap
		if ( !tri->material->hasTexture("LIGHTMAP0") )
		{
			tri->material->setTexture( "LIGHTMAP0", lightmapname[0] );
			tri->material->setTexture( "LIGHTMAP1", lightmapname[1] );
			tri->material->setTexture( "LIGHTMAP2", lightmapname[2] );
		}
		else
		{
			if ( tri->material->getTextureName("LIGHTMAP0") != lightmapname[0] )
			{
				Debug::printf( "Material \"%s\" has lightmap \"%s\", but \"%s\" is needed\n", tri->material->name().c_str(), tri->material->getTextureName("LIGHTMAP0").c_str(), lightmapname[0].c_str() );
				
				// duplicate material
				MyMaterial* oldmat = tri->material;
				P(MyMaterial) newmat = new MyMaterial( *oldmat );
				newmat->setTexture( "LIGHTMAP0", lightmapname[0] );
				newmat->setTexture( "LIGHTMAP1", lightmapname[1] );
				newmat->setTexture( "LIGHTMAP2", lightmapname[2] );
				m_materials.add( newmat );
				++duplicatedmaterialcount;

				// change all triangles, which need to use the new combination
				for ( int k = i ; k < lmptriangles.size() ; ++k )
				{
					const Triangle& lmptri2 = lmptriangles[k];
					MyMesh* mesh2 = m_meshes[ lmptri2.mesh ];
					ExportTriangle* tri2 = mesh2->geometry()->trianglelist[ lmptri2.triangle ];

					if ( tri2->material == oldmat && 
						lmptri2.lightmap->index() == lightmapindex )
					{
						tri2->material = newmat;
					}
				}
			}
		}
	}

	Debug::printf( "Duplicated materials (because of lightmaps): %d\n", duplicatedmaterialcount );
}

void MySceneExport::computeLightmapData()
{
	assert( m_lmp != 0 );
	m_progress->setText( "Computing lightmap data..." );

	const float				lmax = m_dialogNumbers[IDC_LMAX].value;
	const Array<Triangle>&	lmptriangles = m_lmp->triangles();
	Array<float>			edge1;
	Array<float>			edge2;
	const int				EDGE_PROP_POS = 0;
	const int				EDGE_PROP_NORMAL = 1;
	const int				EDGE_PROP_TANGENT = 2;
	const int				EDGE_PROPS = 3;
	float3					props[EDGE_PROPS];
	float3					dprops[EDGE_PROPS];
	Array<float3>			edge1props[EDGE_PROPS];
	Array<float3>			edge2props[EDGE_PROPS];

	// total lightmap area to fill in pixels
	int pixels = 0;
	int totalpixels = 0;
	for ( int i = 0 ; i < lmptriangles.size() ; ++i )
	{
		const Triangle& tri = lmptriangles[i];
		float uvarea = tri.areaUV();
		for ( int n = 0, k = 2 ; n < 3 ; k = n++ )
			uvarea += (tri.pos[n]-tri.pos[k]).length();
		totalpixels += (int)( uvarea + .5f );
	}
	const int pixelprogress = totalpixels < 500 ? 500 : totalpixels / 500;
	Debug::printf( "There is total of %dM lightmap texels to compute\n", totalpixels/1000000 );

	// world bounding box for the scene
	float3 worldmin( 1e9f, 1e9f, 1e9f );
	float3 worldmax( -1e9f, -1e9f, -1e9f );
	for ( int i = 0 ; i < lmptriangles.size() ; ++i )
	{
		for ( int m = 0 ; m < 3 ; ++m )
			for ( int k = 0 ; k < 3 ; ++k )
			{
				worldmin[k] = Math::min( lmptriangles[i].pos[m][k], worldmin[k] );
				worldmax[k] = Math::max( lmptriangles[i].pos[m][k], worldmax[k] );
			}
	}
	float3 worldsize = worldmax - worldmin;
	Debug::printf( "World bounding box is (%g %g %g) - (%g %g %g)\n", 
		worldmin[0], worldmin[1], worldmin[2],
		worldmax[0], worldmax[1], worldmax[2] );

	// prepare occlusion test
	Array<Opcode::IndexedTriangle> optriangles;
	Array<Opcode::Point> opvertices;
	for ( int i = 0 ; i < m_meshes.size() ; ++i )
	{
		MyMesh* mesh = m_meshes[i];
		if ( !mesh->castShadows )
			continue;

		MyGeometry* geom = mesh->geometry();
		float3x4 worldtm = mesh->node()->worldTransform();
		for ( int k = 0 ; k < geom->trianglelist.size() ; ++k )
		{
			optriangles.add( Opcode::IndexedTriangle() );
			for ( int n = 0 ; n < 3 ; ++n )
			{
				optriangles.last().mVRef[n] = opvertices.size();
				float3 pos = worldtm.transform( geom->trianglelist[k]->verts[n]->pos );
				opvertices.add( Opcode::Point(pos.x, pos.y, pos.z) );
			}
		}
	}
	
	Opcode::MeshInterface mesh;
	mesh.SetNbTriangles( optriangles.size() );
	mesh.SetNbVertices( opvertices.size() );
	mesh.SetPointers( &optriangles[0], &opvertices[0] );
	mesh.Single = true;
	
	Opcode::BuildSettings settings;
	settings.mRules = Opcode::SPLIT_BEST_AXIS;

	Opcode::OPCODECREATE treebuilder;
	treebuilder.mIMesh = &mesh;
	treebuilder.mSettings = settings;
	treebuilder.mNoLeaf = true;
	treebuilder.mQuantized = false;
	treebuilder.mKeepOriginal = false;
	treebuilder.mCanRemap = false;
	Opcode::Model model;
	model.Build( treebuilder );

	// occlusion ray collider
	Opcode::RayCollider raycollider;
	Opcode::CollisionFaces collisionfacebuffer;
	raycollider.SetClosestHit( false );
	raycollider.SetCulling( false );
	raycollider.SetDestination( &collisionfacebuffer );
	raycollider.SetMaxDist( lmax );
	assert( !raycollider.ValidateSettings() );

	// light ray collider
	Opcode::RayCollider raycollider2;
	raycollider2.SetClosestHit( false );
	raycollider2.SetCulling( false );
	raycollider2.SetDestination( &collisionfacebuffer );
	raycollider2.SetMaxDist( 1e6f );
	assert( !raycollider2.ValidateSettings() );

	// lightmap computation params
	const bool enablelighting = m_dialogBooleans[IDC_COMPUTELIGHTMAPS].value;
	const bool enableocclusion = m_dialogBooleans[IDC_COMPUTEOCCLUSIONMAPS].value;
	const int rayspertexel = enableocclusion ? (int)m_dialogNumbers[IDC_RAYSTEXEL].value : 0;

	// DEBUG: fill combined polygon rectangles
	/*const Array<P(LightMap)>& lightmaps = m_lmp->lightmaps();
	for ( int i = 0 ; i < lightmaps.size() ; ++i )
	{
		LightMap* lmap = lightmaps[i];
		lmap->data.resize( lmap->area(), float3(LIGHTMAP_FILLER,0,0) );

		const float3 colors[] = {float3(1,0,0),float3(1,1,0),float3(1,0,1),float3(1,1,1),float3(0,0,1),float3(0,1,0)};
		const int COLORS = sizeof(colors)/sizeof(colors[0]);
		for ( int k = 0 ; k < lmap->combined.size() ; ++k )
		{
			lightmappacker::Rect rc = lmap->combined[k]->rectUV();
			for ( int y = rc.y0 ; y < rc.y1 ; ++y )
				for ( int x = rc.x0 ; x < rc.x1 ; ++x )
					if ( x >= 0 && x < lmap->width() && y >= 0 && y < lmap->height() )
						lmap->data[x+y*lmap->width()] = colors[k%COLORS];
		}
	}*/

	// tangent space light sampling directions
	const float3x3 LIGHTING_BASIS = float3x3(
		0.816497f, 0.f, 0.577351f,
		-0.408248f, 0.707109f, 0.577351f,
		-0.408248f, -0.707109f, 0.577351f );

	// compute lighting for each triangle
	clock_t clock0 = clock();
	clock_t lastrefresh = clock0;
	Array<MyLight*> lights;
	for ( int i = 0 ; i < lmptriangles.size() ; ++i )
	{
		const Triangle& lmptri = lmptriangles[i];
		MyMesh* mesh = m_meshes[ lmptri.mesh ];
		ExportTriangle* tri = mesh->geometry()->trianglelist[ lmptri.triangle ];
		const float3x4 worldtm = mesh->node()->transform();

		// world space vertex normals & tangents
		float3 vtangents[3];
		float3 vnormals[3];
		for ( int k = 0 ; k < 3 ; ++k )
		{
			vnormals[k] = worldtm.rotate( tri->getVertexNormal(k) );
			vtangents[k] = worldtm.rotate( tri->getVertexTangent(k) );
		}

		// lightmap dim
		const int mapwidth = lmptri.lightmap->width();
		const int mapheight = lmptri.lightmap->height();
		edge1.clear();
		edge2.clear();
		edge1.resize( mapheight, 1e9f );
		edge2.resize( mapheight, -1e9f );
		for ( int e = 0 ; e < EDGE_PROPS ; ++e )
		{
			edge1props[e].resize( mapheight );
			edge2props[e].resize( mapheight );
		}

		// calc edge uvs & positions
		int miny = mapheight;
		int maxy = -mapheight;
		int n = 2;
		for ( int k = 0 ; k < 3 ; n = k++ )
		{
			int e1 = n;
			int e2 = k;
			if ( lmptri.uv[e1].y > lmptri.uv[e2].y )
			{
				e1 = k;
				e2 = n;
			}

			float2 uv0 = lmptri.uv[e1];
			float2 uv1 = lmptri.uv[e2];
			const int count = int( uv1.y - uv0.y ) + 1;
			const float invcount = 1.f / ( uv1.y - uv0.y );
			float2 duv = (uv1 - uv0) * invcount;

			props[EDGE_PROP_POS] = lmptri.pos[e1];
			dprops[EDGE_PROP_POS] = (lmptri.pos[e2] - lmptri.pos[e1]) * invcount;

			props[EDGE_PROP_NORMAL] = vnormals[e1];
			dprops[EDGE_PROP_NORMAL] = (vnormals[e2] - vnormals[e1]) * invcount;

			props[EDGE_PROP_TANGENT] = vtangents[e1];
			dprops[EDGE_PROP_TANGENT] = (vtangents[e2] - vtangents[e1]) * invcount;

			for ( int i = 0 ; i < count ; ++i )
			{
				int y = (int)( uv0.y );

				if ( unsigned(y) < unsigned(mapheight) )
				{
					if ( y < miny )
						miny = y;
					if ( y > maxy )
						maxy = y;

					float x = uv0.x;
					if ( x > edge2[y] )
					{
						edge2[y] = x;
						for ( int e = 0 ; e < EDGE_PROPS ; ++e )
							edge2props[e][y] = props[e];
					}
					if ( x < edge1[y] )
					{
						edge1[y] = x;
						for ( int e = 0 ; e < EDGE_PROPS ; ++e )
							edge1props[e][y] = props[e];
					}
				}

				uv0 += duv;
				for ( int e = 0 ; e < EDGE_PROPS ; ++e )
					props[e] += dprops[e];
			}
		}

		// find out lights that affect this triangle
		if ( enablelighting )
		{
			lights.clear();
			for ( int k = 0 ; k < m_lights.size() ; ++k )
			{
				MyLight* lt = m_lights[k];

				/*for ( int n = 0 ; n < 3 ; ++n )
				{
					float3 d;
					lt->illuminate( lmptri.pos[n], vnormals[n], &d );
					if ( d.x+d.y+d.z > 1.f/256.f )
					{
						lights.add( lt );
						break;
					}
				}*/
				lights.add( lt );
			}
		}

		// calculate lighting
		Array<float3x3>& data = lmptri.lightmap->data;
		data.resize( mapwidth*mapheight, float3x3(LIGHTMAP_FILLER) );

		for ( int v = miny ; v <= maxy ; ++v )
		{
			int u = (int)( edge1[v] );
			const int u1 = (int)( edge2[v] );
			const int count = u1 - u + 1;
			const float invcount = 1.f/float(count);
			assert( u >= 0 && u1 < mapwidth );

			for ( int e = 0 ; e < EDGE_PROPS ; ++e )
			{
				props[e] = edge1props[e][v];
				dprops[e] = (edge2props[e][v] - edge1props[e][v]) * invcount;
			}
			const float3& pos = props[EDGE_PROP_POS];
			const float3& unormal = props[EDGE_PROP_NORMAL];
			const float3& utangent = props[EDGE_PROP_TANGENT];

			float3x3* datap = &data[v*mapwidth];

			for ( int i = 0 ; i < count ; ++i )
			{	
				// update progress bar
				if ( (pixels++ % pixelprogress) == 0 )
				{
					clock_t now = clock();
					if ( now-lastrefresh > CLOCKS_PER_SEC )
					{
						lastrefresh = now;
						double elapsed = double(now - clock0)/double(CLOCKS_PER_SEC);
						double done = double(pixels)/double(totalpixels);
						double total = double(elapsed) / done;
						int elapsedsec = (int)elapsed;
						int sec = (int)( total - elapsed );
						if ( sec < 0 )
							sec = 0;
						char msg[128];
						sprintf( msg, "Computing lightmap data... (elapsed %dh %dmin %ds, ETA %dh %dmin %ds)", 
							(elapsedsec/3600)%60, (elapsedsec/60)%60, elapsedsec%60,
							(sec/3600)%60, (sec/60)%60, sec%60 );
						m_progress->setText( msg );
						m_progress->setProgress( (float)done );
					}
				}

				// pos: 3D-point in world space
				// unormal: Unnormalized point normal (in world space)
				// utangent: Unnormalized point tangent (in world space)
				// (u,v) = (x,y) position in lightmap
				// datap[u] = float3x3 lightmap color [0,1], 1 column for each lighting space axis
				if ( u >= 0 && u < mapwidth )
				{
					// texel->world space transform
					const float3 normal = normalize( unormal );
					const float3 tangent = normalize( utangent );
					const float3 binormal = cross( normal, tangent );
					const float3 opos = pos + normal * 0.01f;
					float3x3 tangenttoworld;
					tangenttoworld.setColumn( 0, binormal );
					tangenttoworld.setColumn( 1, tangent );
					tangenttoworld.setColumn( 2, normal );

					// compute occlusion value [0,1]
					float visibility = 0.f;
					float totalweight = 0.f;
					for ( int n = 0 ; n < rayspertexel ; ++n )
					{	
						// random positive direction (out of surface)
						float3 dir = RandomUtil::getPointInSphere(1.f,1.f);
						if ( dir.z < 0.f )
							dir.z = -dir.z;
						dir = tangenttoworld.rotate( dir );

						float weight = dot(dir,normal);
						if ( weight <= 0.f )
						{
							--n;
							continue;
						}

						// ray-model test
						Opcode::Ray ray;
						ray.mOrig.x = opos.x;
						ray.mOrig.y = opos.y;
						ray.mOrig.z = opos.z;
						ray.mDir.x = dir.x;
						ray.mDir.y = dir.y;
						ray.mDir.z = dir.z;
						int collisions = 0;
						const Opcode::CollisionFace* collisionlist = 0;
						if ( !raycollider.Collide(ray, model,0) )
						{
							Debug::printf( "ERROR: RayCollider::Collide failed\n" );
							continue;
						}
						collisions = collisionfacebuffer.GetNbFaces();
						collisionlist = collisionfacebuffer.GetFaces();

						// update visiblity value
						totalweight += weight;
						if ( collisions == 0 )
							visibility += weight;
					}
					if ( totalweight > 1e-9f )
						visibility /= totalweight;
					else
						visibility = 1.f;

					// compute diffuse component of lighting
					float3x3 diffuse(0);
					if ( enablelighting )
					{
						for ( int n = 0 ; n < lights.size() ; ++n )
						{
							// raytrace light
							MyLight* lt = lights[n];
							int collisions = 0;
							if ( lt->lightstate.shadow )
							{
								float3 lightv = lt->cachedWorldTransform().translation() - opos;
								float lightdist = lightv.length();
								float3 dir = lightv * (1.f/lightdist);
								Opcode::Ray ray;
								ray.mOrig.x = opos.x;
								ray.mOrig.y = opos.y;
								ray.mOrig.z = opos.z;
								ray.mDir.x = dir.x;
								ray.mDir.y = dir.y;
								ray.mDir.z = dir.z;
								const Opcode::CollisionFace* collisionlist = 0;
								raycollider2.SetMaxDist( lightdist );
								if ( !raycollider2.Collide(ray, model,0) )
								{
									Debug::printf( "ERROR: RayCollider::Collide failed\n" );
									continue;
								}
								collisions = collisionfacebuffer.GetNbFaces();
								collisionlist = collisionfacebuffer.GetFaces();
							}

							if ( 0 == collisions )
							{
								for ( int cs = 0 ; cs < 3 ; ++cs )
								{
									float3 v = tangenttoworld.rotate( LIGHTING_BASIS.getRow(cs) );

									float3 d;
									lt->illuminate( pos, v, &d );
									//Debug::printf( "Lit vector (%g,%g,%g) -> (%g,%g,%g), normal was (%g,%g,%g)\n", v.x, v.y, v.z, d.x, d.y, d.z, normal.x, normal.y, normal.z );

									diffuse(cs,0) += d[0];
									diffuse(cs,1) += d[1];
									diffuse(cs,2) += d[2];
								}
							}
						}
					}
					else
					{
						for ( int cs = 0 ; cs < 3 ; ++cs )
							diffuse.setRow( cs, float3(1,1,1) );
					}

					// store diffuse * ambient occlusion
					datap[u] = diffuse * visibility;
				}

				++u;
				for ( int e = 0 ; e < EDGE_PROPS ; ++e )
					props[e] += dprops[e];
			}
		}
	}

	// report elapsed time
	int elapsedsec = int( double(clock() - clock0)/double(CLOCKS_PER_SEC) );
	Debug::printf( "Lightmap data computation took %dh %dmin %ds\n", (elapsedsec/3600)%60, (elapsedsec/60)%60, elapsedsec%60 );

	// pad lightmaps
	m_progress->setText( "Padding lightmaps..." );
	for ( int i = 0 ; i < m_lmp->lightmaps().size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(m_lmp->lightmaps().size()) );

		LightMap* lm = m_lmp->lightmaps()[i];
		const int w = lm->width();
		const int h = lm->height();
		Array<float3x3> data = lm->data;

		for ( int y = 0 ; y < h ; ++y )
		{
			for ( int x = 0 ; x < w ; ++x )
			{
				for ( int cs = 0 ; cs < 3 ; ++cs )
				{
					float3 c = lm->data[x+y*w].getRow( cs );
					if ( c[cs] == LIGHTMAP_FILLER )
					{
						c = float3(0,0,0);
						int n = 0;
						int xc[] = {-1,0,1,  -1,1, -1,0,1};
						int yc[] = {-1,-1,-1, 0,0, 1,1,1};
						for ( int k = 0 ; k < int(sizeof(xc)/sizeof(xc[0])) ; ++k )
						{
							int x1 = x + xc[k];
							int y1 = y + yc[k];
							if ( x1 >= 0 && x1 < w && y1 >= 0 && y1 < h )
							{
								const float3 c1 = lm->data[x1+y1*w].getRow( cs );
								if ( c1.x != LIGHTMAP_FILLER )
								{
									c += c1;
									++n;
								}
							}
						}
						if ( n > 0 )
							data[x+y*w].setRow( cs, c * (1.f/float(n)) );
					}
				}
			}
		}

		// replace rest filler texels with average intensity
		float3x3 avg(0);
		int n = 0;
		for ( int y = 0 ; y < h ; ++y )
		{
			for ( int x = 0 ; x < w ; ++x )
			{
				for ( int cs = 0 ; cs < 3 ; ++cs )
				{
					float3 c = lm->data[x+y*w].getRow(cs);
					if ( c[cs] != LIGHTMAP_FILLER )
					{
						avg.setRow( cs, avg.getRow(cs)+c );
						++n;
					}
				}
			}
		}
		if ( n > 0 )
		{
			avg *= 1.f/float(n);
			for ( int y = 0 ; y < h ; ++y )
			{
				for ( int x = 0 ; x < w ; ++x )
				{
					for ( int cs = 0 ; cs < 3 ; ++cs )
					{
						float3 c = data[x+y*w].getRow(cs);
						if ( c[cs] == LIGHTMAP_FILLER )
							data[x+y*w].setRow( cs, avg.getRow(cs) );
					}
				}
			}
		}

		lm->data = data;
	}
}

void MySceneExport::saveLightmaps()
{
	assert( m_lmp != 0 );
	
	m_progress->setText( "Storing lightmaps..." );

	SurfaceFormat				imgformat = SurfaceFormat::SURFACE_X8R8G8B8;
	Array<uint32_t>				img;
	const Array<P(LightMap)>&	lightmaps = m_lmp->lightmaps();

	for ( int i = 0 ; i < lightmaps.size() ; ++i )
	{
		m_progress->setProgress( float(i)/float(lightmaps.size()) );

		LightMap* lmap = lightmaps[i];
		const int mapwidth = lmap->width();
		const int mapheight = lmap->height();
		img.resize( mapwidth*mapheight );
		const Array<float3x3>& src = lmap->data;

		for ( int cs = 0 ; cs < 3 ; ++cs )
		{
			int p = 0;

			for ( int y = 0 ; y < mapheight ; ++y )
			{
				for ( int x = 0 ; x < mapwidth ; ++x )
				{
					float3 cf = clamp( src[x+p].getRow(cs)*128.f, float3(0,0,0), float3(255,255,255) );
					//Debug::printf( "cf = %g %g %g\n", cf.x, cf.y, cf.z );
					uint32_t c = 0xFF000000 + (int(cf.x)<<16) + (int(cf.y)<<8) + int(cf.z);
					//Debug::printf( "c = %08x\n", c );
					img[p+x] = c;
				}
				p += mapwidth;
			}

			/*Debug::printf( "DEBUG: Lightmap %s triangles:\n", lmap->name[cs].c_str() );
			for ( int k = 0 ; k < lmap->combined.size() ; ++k )
				for ( int n = 0 ; n < lmap->combined[k]->triangles.size() ; ++n )
					Debug::printf( "  [%d:%d] = (%g %g) (%g %g) (%g %g)\n", k, n, 
						lmap->combined[k]->triangles[n]->uv[0].x,
						lmap->combined[k]->triangles[n]->uv[0].y,
						lmap->combined[k]->triangles[n]->uv[1].x,
						lmap->combined[k]->triangles[n]->uv[1].y,
						lmap->combined[k]->triangles[n]->uv[2].x,
						lmap->combined[k]->triangles[n]->uv[2].y );*/

			m_textures.add( lmap->name[cs] );
			ImageWriter::write( getTextureName(lmap->name[cs]), SurfaceFormat::SURFACE_DXT1,
				&img[0], mapwidth, mapheight, mapwidth*4, imgformat, 0, SurfaceFormat() );
		}
	}
}

MyMaterial* MySceneExport::getMaterialWithFlags( MyMaterial* mtl, int flags )
{
	if ( mtl->flags == flags )
		return mtl;

	for ( int i = 0 ; i < m_materials.size() ; ++i )
	{
		MyMaterial* m2 = m_materials[i];
		if ( m2->mtl3ds() == mtl->mtl3ds() && flags == m2->flags )
			return m2;
	}

	P(MyMaterial) m2 = new MyMaterial( *mtl );
	m2->flags = flags;
	m_materials.add( m2 );
	return m2;
}

String MySceneExport::getTextureName( const NS(lang,String)& origname ) const
{
	String newname = origname;
	if ( m_dialogBooleans[IDC_SAVETEXTURESAS].value )
	{
		int ix = origname.lastIndexOf(".");
		if ( ix != -1 )
			newname = origname.substring( 0, ix+1 );

		String ext = m_dialogStrings[IDC_TEXTUREEXTENSION].value.toLowerCase();
		newname = newname + ext;
	}
	return newname;
}

String MySceneExport::chopFilename( String str )
{
	if ( str.length() > 64 )
	{
		int offset = str.length() - 64;
		return String("... ") + str.substring( offset );
	}
	return str;
}

ExportPlatform getExportPlatform()
{
	return MySceneExport::exportPlatform();
}

const char* getExportPlatformName( ExportPlatform platformid )
{
	return NS(gr,Context)::getDescription( (NS(gr,Context)::PlatformType)platformid );
}
