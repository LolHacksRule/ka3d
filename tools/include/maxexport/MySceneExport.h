#ifndef _MYSCENEEXPORT_H
#define _MYSCENEEXPORT_H


#include "MyMesh.h"
#include "MyShape.h"
#include "MyLight.h"
#include "MyDummy.h"
#include "MyCamera.h"
#include "MyMaterial.h"
#include "MyPrimitive.h"
#include "DialogItem.h"
#include "OptionInterface.h"
#include "TextureGeneratorInterface.h"
#include "maxexportconfig.h"
#include <hgr/UserPropertySet.h>
#include <lang/Array.h>
#include <lang/Hashtable.h>


class ProgressDialog;

namespace hgr {
	class SceneOutputStream;}

namespace lua {
	class LuaTable;}

namespace lightmappacker {
	class LightMapPacker;}


class MySceneExport :
	public SceneExport,
	public OptionInterface
{
public:
	explicit MySceneExport( HINSTANCE instance );
	~MySceneExport();

	int				ExtCount();
	const TCHAR*	Ext( int i );
	const TCHAR*	LongDesc();
	const TCHAR*	ShortDesc();
	const TCHAR*	AuthorName();
	const TCHAR*	CopyrightMessage();
	const TCHAR*	OtherMessage1();
	const TCHAR*	OtherMessage2();
	unsigned int	Version();
	void			ShowAbout( HWND hwnd );
	int				DoExport( const TCHAR* namesz, ExpInterface* ei, Interface* ip, BOOL quiet, DWORD options );
	BOOL			SupportsOptions( int ext, DWORD options );

	void			getOptions( lua::LuaTable& tab );
	void			setOptionDefaults();
	bool			getOptionBoolean( const NS(lang,String)& name );
	float			getOptionNumber( const NS(lang,String)& name );
	NS(lang,String)	getOptionString( const NS(lang,String)& name );

	static NS(lang,String)		getOutputFilename( const NS(lang,String)& srcname );

	static NS(lang,String)		getPostfixedOutputFilename( const NS(lang,String)& srcname );

	static bool		flushWin32MsgQueue();

	static ExportPlatform	exportPlatform()	{return sm_exportPlatform;}

private:
	class Texture
	{
	public:
		NS(lang,String)	filename;
		int				mapping;

		Texture()											: mapping(0) {}
		Texture( const NS(lang,String)& fname )				: filename(fname), mapping(0) {}

		bool operator<( const Texture& other ) const		{return filename<other.filename;}
		bool operator==( const Texture& other ) const		{return filename==other.filename;}
	};

	HINSTANCE			m_instance;
	char				m_pluginName[32];
	P(ProgressDialog)	m_progress;

	int													m_dialogret;
	lang::Hashtable< int, DialogItem<bool> >			m_dialogBooleans;
	lang::Hashtable< int, DialogItem<float> >			m_dialogNumbers;
	lang::Hashtable< int, DialogItem<NS(lang,String)> >	m_dialogStrings;

	bool								m_quiet;
	NS(lang,String)						m_outputFilename;
	NS(lang,Array)<P(MyCamera)>			m_cameras;
	NS(lang,Array)<P(MyDummy)>				m_dummies;
	NS(lang,Array)<P(MyLight)>				m_lights;
	NS(lang,Array)<P(MyShape)>				m_shapes;
	NS(lang,Array)<P(MyMesh)>				m_meshes;
	NS(lang,Array)<P(MyNode)>				m_others;
	NS(lang,Array)<MyNode*>				m_nodes; // all together
	NS(lang,Array)<P(MyMaterial)>			m_materials;
	NS(lang,Array)<Texture>				m_textures; // full path name
	NS(lang,Array)<P(MyPrimitive)>			m_primitives;
	P(hgr::UserPropertySet)				m_userPropertySet;
	P(lightmappacker::LightMapPacker)	m_lmp;
	int									m_maxBonesPerPrimitive;

	static ExportPlatform				sm_exportPlatform;
	static MySceneExport*				sm_this;

	void	packLightmaps();
	void	assignLightmaps();
	void	computeLightmapData();
	void	saveLightmaps();
	void	copyTextures( Interface* ip );
	
	MyMaterial*		getMaterialWithFlags( MyMaterial* mtl, int flags );
	NS(lang,String)	getTextureName( const NS(lang,String)& origname ) const;

	void	cleanup();
	void	extractScene( ExpInterface* ei, Interface* ip, DWORD options );
	void	writeScene( ExpInterface* ei, Interface* ip, DWORD options );
	void	writeSceneParams( hgr::SceneOutputStream* out, Interface* ip );
	void	writeMaterials( hgr::SceneOutputStream* out );
	void	writePrimitives( hgr::SceneOutputStream* out );
	void	writeMeshes( hgr::SceneOutputStream* out );
	void	writeCameras( hgr::SceneOutputStream* out );
	void	writeLights( hgr::SceneOutputStream* out );
	void	writeDummies( hgr::SceneOutputStream* out );
	void	writeShapes( hgr::SceneOutputStream* out );
	void	writeOtherNodes( hgr::SceneOutputStream* out );
	void	writeTransformInterpolators( hgr::SceneOutputStream* out );
	void	writeUserProperties( hgr::SceneOutputStream* out );
	void	writeEmptyList( hgr::SceneOutputStream* out );

	static NS(lang,String)	chopFilename( NS(lang,String) str );

	static BOOL CALLBACK	dlgproc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );
	static NS(lang,String)		fixTexturePath( NS(lang,String) filename, Interface* ip );

	MySceneExport( const MySceneExport& );
	MySceneExport& operator=( const MySceneExport& );
};


#endif // _MYSCENEEXPORT_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
