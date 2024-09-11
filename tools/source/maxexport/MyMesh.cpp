#include "StdAfx.h"
#include "MyMesh.h"
#include "MtlUtil.h"
#include "SkinUtil.h"
#include "ArrayUtil.h"
#include "INodeUtil.h"
#include "Matrix3Util.h"
#include "PhysiqueUtil.h"
#include <io/PropertyParser.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <plugapi.h>
#include <Bipexp.h>
#include <algorithm>
#include <config.h>


USING_NAMESPACE(io)
USING_NAMESPACE(lang)
USING_NAMESPACE(math)


MyMesh::MyMesh( INode* node3ds ) :
	MyNode( node3ds, new hgr::Mesh ),
	m_geom( new MyGeometry(node()->name()) )
{
	node()->setEnabled( 0 != node3ds->Renderable() );

	// make sure collision-only objects don't cast/receive shadows or get rendered
	PropertyParser parser( userprop, name() );
	if ( parser.hasKey("collision") && 
		String(parser.getString("collision")).toLowerCase() == "only" )
	{
		node()->setEnabled( false );
		castShadows = false;
		receiveShadows = false;
	}
}

MyMesh::~MyMesh()
{
}

void MyMesh::extractBones( const Array<MyNode*>& nodes )
{
	// extract bones from Physique modifier
	Modifier* phymod = PhysiqueUtil::findPhysiqueModifier( node3ds() );
	if ( phymod )
	{
		// get (possibly shared) Physique export interface
		IPhysiqueExport* phyexp = (IPhysiqueExport*)phymod->GetInterface( I_PHYINTERFACE );
		assert( phyexp );

		// export from initial pose?
		phyexp->SetInitialPose( false );

		// get (unique) context dependent export inteface
		IPhyContextExport* phycontextexp = (IPhyContextExport*)phyexp->GetContextInterface( node3ds() );
		if( !phycontextexp )
			throwError( Exception( Format("No Physique modifier context export interface") ) );

		// convert to rigid for time independent vertex assignment
		phycontextexp->ConvertToRigid( true );

		// allow blending to export multi-link assignments
		phycontextexp->AllowBlending( true );

		// list bone nodes
		Array<INode*> bones;
		PhysiqueUtil::listBones( phycontextexp, bones );

		// add bone indices and inverse rest transform to the mesh
		Matrix3 toskintm3ds = Inverse( PhysiqueUtil::getInitSkinTM(node3ds(),phyexp) );
		for ( int i = 0 ; i < bones.size() ; ++i )
		{
			INode* bonenode3ds = bones[i];

			// bone -> skin tm in non-deforming pose
			Matrix3 inittm3ds = PhysiqueUtil::getInitBoneTM( bonenode3ds, phyexp );
			Matrix3 resttm3ds = inittm3ds * toskintm3ds;
			float3x4 resttm = Matrix3Util::toLH( resttm3ds );
			
			int bonenodeix = -1;
			for ( int i = 0 ; i < nodes.size() ; ++i )
				if ( nodes[i]->node3ds() == bonenode3ds )
				{
					bonenodeix = i;
					break;
				}
			if ( bonenodeix == -1 )
				throwError( Exception( Format("Bone {0} used by Physique Modifier but not exported", (String)bonenode3ds->GetName()) ) );

			node()->addBone( nodes[bonenodeix]->node(), resttm.inverse() );
		}

		// destroy Physique modifier export
		if ( phycontextexp )
		{
			assert( phyexp );
			phyexp->ReleaseContextInterface( phycontextexp );
			phycontextexp = 0;
		}
		if ( phyexp )
		{
			assert( phymod );
			phymod->ReleaseInterface( I_PHYINTERFACE, phyexp );
			phyexp = 0;
		}
	}
	
	// extract bones from Skin modifier
	Modifier* skinmod = SkinUtil::findSkinModifier( node3ds() );
	if ( skinmod )
	{
		// get (possibly shared) Skin interface
		ISkin* skin = (ISkin*)skinmod->GetInterface( I_SKIN );
		if( !skin )
			throwError( Exception( Format("No Skin modifier ISkin interface") ) );

		// get skin context
		ISkinContextData* skincx = skin->GetContextInterface( node3ds() );
		if( !skincx )
			throwError( Exception( Format("No Skin modifier context interface") ) );

		// list bone nodes
		Array<INode*> bones;
		SkinUtil::listBones( skin, bones );

		// add bone indices and inverse rest transform to the mesh
		Matrix3 toskin = Inverse( SkinUtil::getInitSkinTM(node3ds(),skin) );
		for ( int i = 0 ; i < bones.size() ; ++i )
		{
			INode* bonenode3ds = bones[i];
			Matrix3 inittm3ds = SkinUtil::getInitBoneTM( bonenode3ds, skin );
			Matrix3 resttm3ds = inittm3ds * toskin;
			float3x4 resttm = Matrix3Util::toLH( resttm3ds );

			int bonenodeix = -1;
			for ( int i = 0 ; i < nodes.size() ; ++i )
				if ( nodes[i]->node3ds() == bonenode3ds )
				{
					bonenodeix = i;
					break;
				}
			if ( bonenodeix == -1 )
				throwError( Exception( Format("Bone {0} used by Skin Modifier but not exported", (String)bonenode3ds->GetName()) ) );

			node()->addBone( nodes[bonenodeix]->node(), resttm.inverse() );
		}

		// destroy Skin modifier export
		if ( skin )
		{
			assert( skinmod );
			skinmod->ReleaseInterface( I_SKIN, skin );
			skin = 0;
		}
	}
}

Mesh* MyMesh::getMesh( std::auto_ptr<TriObject>* tridel )
{
	// mesh without material is probably error so we treat it as such
	Mtl* mtl = node3ds()->GetMtl();
	if ( !mtl )
		throwError( Exception( Format("Mesh {0} needs to have material set", node()->name()) ) );

	ObjectState objstate = node3ds()->EvalWorldState( 0 );
	if ( !objstate.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID,0)) )
		throwError( Exception( Format("Cannot convert object \"{0}\" to triangle mesh", node()->name()) ) );

	TriObject* tri = static_cast<TriObject*>( objstate.obj->ConvertToType( 0, Class_ID(TRIOBJ_CLASS_ID,0) ) );
	*tridel = std::auto_ptr<TriObject>( tri != objstate.obj ? tri : 0 );

	return &tri->mesh;
}

void MyMesh::extractMtls( Array<Mtl*>& mtls )
{
	// get 3dsmax Mesh and Mtl
	std::auto_ptr<TriObject> tridel;
	Mesh* mesh = getMesh( &tridel );
	Mtl* mtl = node3ds()->GetMtl();
	
	// list unique Mtls used by this mesh
	for ( int fi = 0 ; fi < mesh->getNumFaces() ; ++fi )
	{
		Face& face = mesh->faces[fi];
		if ( !MtlUtil::hasFaceMtl(face,mtl) )
			throwError( Exception( Format("Multi-Sub Material \"{0}\" (used by mesh \"{1}\") has undefined sub-material id {2}", String(mtl->GetName()), name(), face.getMatID()+1) ) );

		Mtl* submtl = MtlUtil::getFaceMtl(face,mtl);
		if ( submtl != 0 )
		{
			//Debug::printf( "Mesh %s face %d uses material %s\n", this->name().c_str(), fi, (char*)submtl->GetName() );
			mtls.add( submtl );
		}
	}
	ArrayUtil::removeDuplicates( mtls );
}

void MyMesh::extractGeometry( const Array<P(MyMaterial)>& materials, 
	ExportVertexColors exportvertexcolors )
{
	Modifier*			phymod			= 0;
	IPhysiqueExport*	phyexp			= 0;
	IPhyContextExport*	phycontextexp	= 0;
	Modifier*			skinmod			= 0;
	ISkin*				skin			= 0;

	try
	{
		// ensure clockwise polygon vertex order
		int vx[3] = {2,1,0};
		if ( INodeUtil::hasNegativeScaling(node3ds()) )
		{
			int tmp = vx[0];
			vx[0] = vx[2];
			vx[2] = tmp;
		}

		// get 3dsmax Mesh and Mtl
		std::auto_ptr<TriObject> tridel;
		Mesh* mesh = getMesh( &tridel );
		Mtl* mtl = node3ds()->GetMtl();
		const int numfaces = mesh->getNumFaces();

		// extract vertex positions
		Matrix3 vertextm = INodeUtil::getVertexTransform( node3ds() );
		int vertices = mesh->getNumVerts();
		m_geom->vertexlist.resize( vertices );
		for ( int vi = 0 ; vi < vertices ; ++vi )
		{
			Point3 v = vertextm * mesh->verts[vi];

			P(ExportVertex) vert = new ExportVertex();
			vert->pos = float3( v.x, v.y, v.z );
			m_geom->vertexlist[vi] = vert;
		}

		// extract triangles
		for ( int fi = 0 ; fi < numfaces ; ++fi )
		{
			Face& face = mesh->faces[fi];

			// triangle material
			Mtl* facemtl = MtlUtil::getFaceMtl( face, mtl );
			assert( facemtl != 0 );

			int polymatindex = std::find_if( materials.begin(), materials.end(), MyMaterial::MtlEqualsMyMaterial(facemtl) ) - materials.begin();
			assert( polymatindex < materials.size() );
			MyMaterial* mat = materials[polymatindex];
			
			P(ExportTriangle) tri = new ExportTriangle();
			m_geom->trianglelist.add( tri );
			tri->material = mat;

			// triangle indices
			for ( int vxi = 0 ; vxi < 3 ; ++vxi )
			{
				int vi = face.v[ vx[vxi] ];

				assert( vi >= 0 && vi < m_geom->vertexlist.size() );
				tri->verts[vxi] = m_geom->vertexlist[vi];
			}
		}

		// pre-reserve memory for vertex->face connections
		Array<int> connectedcounts( vertices, 0 );
		for ( int fi = 0 ; fi < numfaces ; ++fi )
		{
			Face& face = mesh->faces[fi];
			for ( int vxi = 0 ; vxi < 3 ; ++vxi )
			{
				int vi = face.v[ vx[vxi] ];
				assert( vi >= 0 && vi < connectedcounts.size() );
				connectedcounts[vi] += 1;
			}
		}
		for ( int i = 0 ; i < vertices ; ++i )
		{
			m_geom->vertexlist[i]->connectedFaces.resize( connectedcounts[i] );
			m_geom->vertexlist[i]->connectedFaces.clear();
		}

		// extract vertex weights from Physique modifier
		phymod = PhysiqueUtil::findPhysiqueModifier( node3ds() );
		if ( phymod )
		{
			// get (possibly shared) Physique export interface
			phyexp = (IPhysiqueExport*)phymod->GetInterface( I_PHYINTERFACE );
			if( !phyexp )
				throwError( Exception( Format("No Physique modifier export interface") ) );

			// export from initial pose?
			phyexp->SetInitialPose( false );
			
			// get (unique) context dependent export inteface
			phycontextexp = (IPhyContextExport*)phyexp->GetContextInterface( node3ds() );
			if( !phycontextexp )
				throwError( Exception( Format("No Physique modifier context export interface") ) );

			// convert to rigid for time independent vertex assignment
			phycontextexp->ConvertToRigid( true );

			// allow blending to export multi-link assignments
			phycontextexp->AllowBlending( true );

			// list bones
			Array<INode*> bones;
			PhysiqueUtil::listBones( phycontextexp, bones );

			// add vertex weight maps
			for ( int i = 0 ; i < bones.size() ; ++i )
			{
				INode* bone = bones[i];
				String name = bone->GetName();
				assert( i < node()->bones() );
				assert( name == node()->getBone(i)->name() );

				P(ExportWeightMap) wm = new ExportWeightMap( name );
				m_geom->weightmaplist.add( wm );
				PhysiqueUtil::addWeights( wm, m_geom->vertexlist, bone, phycontextexp );
			}
		}

		// extract vertex weights from Skin modifier
		skinmod = SkinUtil::findSkinModifier( node3ds() );
		if ( skinmod )
		{
			skin = (ISkin*)skinmod->GetInterface(I_SKIN);
			if( !skin )
				throwError( Exception( Format("No Skin modifier ISkin interface") ) );

			ISkinContextData* skincx = skin->GetContextInterface( node3ds() );
			if( !skincx )
				throwError( Exception( Format("No Skin modifier context interface") ) );
			if ( skincx->GetNumPoints() != m_geom->vertices() )
				throwError( Exception( Format("Only some vertices ({0}/{1}) of {2} are skinned", skincx->GetNumPoints(), m_geom->vertices(), node()->name()) ) );

			// list bones
			Array<INode*> bones;
			SkinUtil::listBones( skin, bones );

			// add vertex weight maps
			for ( int i = 0 ; i < bones.size() ; ++i )
			{
				INode* bone = bones[i];
				String name = bone->GetName();
				assert( i < node()->bones() );
				assert( name == node()->getBone(i)->name() );

				P(ExportWeightMap) wm = new ExportWeightMap( name );
				m_geom->weightmaplist.add( wm );
				SkinUtil::addWeights( wm, m_geom->vertexlist, bone, skin, skincx );
			}
		}

		// extract vertex colors
		int chn = 0;
		if ( exportvertexcolors == EXPORT_VERTEX_COLORS && 
			mesh->mapSupport(chn) && mesh->getNumMapVerts(chn) > 0 )
		{
			int tverts = mesh->getNumMapVerts( chn );
			UVVert* tvert = mesh->mapVerts( chn );
			TVFace* tface = mesh->mapFaces( chn );

			for ( int fi = 0 ; fi < mesh->getNumFaces() ; ++fi )
			{
				Face& face = mesh->faces[fi];

				for ( int vxi = 0 ; vxi < 3 ; ++vxi )
				{
					int vi = face.v[ vx[vxi] ];
					Point3 tc = tvert[ tface[fi].t[ vx[vxi] ] % tverts ];
					float rgb[3] = {tc.x, tc.y, tc.z};

					m_geom->trianglelist[fi]->colors[vxi] = float4( rgb[0], rgb[1], rgb[2], 1.f );
				}
			}
		}

		// extract texcoords
		m_geom->uvwlayers = 0;
		for ( int chn = 1 ; chn < MAX_MESHMAPS ; ++chn )
		{
			if ( mesh->mapSupport(chn) && mesh->getNumMapVerts(chn) > 0 )
			{
				int tverts = mesh->getNumMapVerts( chn );
				UVVert* tvert = mesh->mapVerts( chn );
				TVFace* tface = mesh->mapFaces( chn );

				if ( m_geom->uvwlayers >= 1 )
					throwError( Exception( Format("Too many texture coordinate channels used in mesh \"{0}\"", node()->name()) ) );

				//Debug::printf( "Mesh %s UVs:\n", name().c_str() );
				for ( int fi = 0 ; fi < mesh->getNumFaces() ; ++fi )
				{
					Face& face = mesh->faces[fi];
					for ( int vxi = 0 ; vxi < 3 ; ++vxi )
					{
						int vi = face.v[ vx[vxi] ];
						Point3 tc = tvert[ tface[fi].t[ vx[vxi] ] % tverts ];
						//Debug::printf( "  %g %g\n", tc.x, tc.y );
						float3 uvw(tc.x, 1.f-tc.y, tc.z);

						ExportTriangle* tri = m_geom->trianglelist[fi];
						tri->uvw[m_geom->uvwlayers][vxi] = uvw;
					}
				}
				m_geom->uvwlayers += 1;
			}
		}

		// lightmap texcoords
		m_geom->uvwlayers += 1;

		// extract smoothing groups
		assert( numfaces == m_geom->trianglelist.size() );
		for ( int fi = 0 ; fi < numfaces ; ++fi )
		{
			ExportTriangle* tri = m_geom->trianglelist[fi];
			Face& face = mesh->faces[ fi ];
			tri->smoothing = face.smGroup;
		}

		// re-export mesh points in non-deformed pose if Skin modifier present
		// NOTE: 3ds Mesh must not be used after this, because collapsing can invalidate it
		if ( skin )
		{
			// evaluate derived object before Skin modifier
			bool evalnext = false;
			bool evaldone = false;
			ObjectState objstate;
			::Object* obj = node3ds()->GetObjectRef();
			while ( obj->SuperClassID() == GEN_DERIVOB_CLASS_ID && !evaldone )
			{
				IDerivedObject* derivedobj = static_cast<IDerivedObject*>(obj);
				for ( int modix = 0 ; modix < derivedobj->NumModifiers() ; ++modix )
				{
					if ( evalnext )
					{
						objstate = derivedobj->Eval( 0, modix );
						evaldone = true;
						break;
					}

					Modifier* mod = derivedobj->GetModifier(modix);
					if ( mod->ClassID() == SKIN_CLASSID )
						evalnext = true;
				}
				obj = derivedobj->GetObjRef();
			}

			// evaluate possible non-derived object
			if ( evalnext && !evaldone )
			{
				objstate = obj->Eval( 0 );
				evaldone = true;
			}

			// convert to TriObject and get points
			if ( evaldone && objstate.obj->CanConvertToType( Class_ID(TRIOBJ_CLASS_ID,0) ) )
			{
				// get TriObject
				std::auto_ptr<TriObject> triAutoDel(0);
				TriObject* tri = static_cast<TriObject*>( objstate.obj->ConvertToType( 0, Class_ID(TRIOBJ_CLASS_ID,0) ) );
				if ( tri != objstate.obj )
					triAutoDel = std::auto_ptr<TriObject>( tri );

				// get mesh points before Skin is applied
				assert( m_geom->vertices() == tri->mesh.getNumVerts() );
				Mesh* mesh = &tri->mesh;
				int vertices = mesh->getNumVerts();
				for ( int vi = 0 ; vi < vertices ; ++vi )
				{
					Point3 v = vertextm * mesh->verts[vi];

					m_geom->vertexlist[vi]->pos = float3(v.x,v.y,v.z);
				}
			}
		}

		// remove degenerate triangles
		bool reportdegenerate = true;
		for ( int i = 0 ; i < m_geom->trianglelist.size() ; ++i )
		{
			if ( m_geom->trianglelist[i]->isDegenerate() )
			{
				m_geom->trianglelist.remove( i-- );
				if ( reportdegenerate )
				{
					Debug::printf( "Removed degenerate polygons from %s\n", name().c_str() );
					reportdegenerate = false;
				}
			}
		}
		// removed unused vertices
		m_geom->connectVerticesToFaces();
		for ( int i = 0 ; i < m_geom->vertexlist.size() ; ++i )
			if ( m_geom->vertexlist[i]->connectedFaces.size() == 0 )
				m_geom->vertexlist.remove( i-- );

		// destroy export interfaces
		if ( phycontextexp )
		{
			assert( phyexp );
			phyexp->ReleaseContextInterface( phycontextexp );
			phycontextexp = 0;
		}
		if ( phyexp )
		{
			assert( phymod );
			phymod->ReleaseInterface( I_PHYINTERFACE, phyexp );
			phyexp = 0;
		}
		if ( skin )
		{
			skinmod->ReleaseInterface( I_SKIN, skin );
			skin = 0;
			skinmod = 0;
		}
	}
	catch ( ... )
	{
		if ( phycontextexp )
		{
			assert( phyexp );
			phyexp->ReleaseContextInterface( phycontextexp );
			phycontextexp = 0;
		}
		if ( phyexp )
		{
			assert( phymod );
			phymod->ReleaseInterface( I_PHYINTERFACE, phyexp );
			phyexp = 0;
		}
		if ( skin )
		{
			skinmod->ReleaseInterface( I_SKIN, skin );
			skin = 0;
			skinmod = 0;
		}
		throw;
	}

	// check for valid data
	for ( int i = 0 ; i < m_geom->trianglelist.size() ; ++i )
	{
		assert( m_geom->trianglelist[i] != 0 );
		assert( m_geom->trianglelist[i]->verts[0] != 0 );
		assert( m_geom->trianglelist[i]->verts[1] != 0 );
		assert( m_geom->trianglelist[i]->verts[2] != 0 );
	}
	for ( int i = 0 ; i < m_geom->vertexlist.size() ; ++i )
	{
		assert( m_geom->vertexlist[i] != 0 );
		assert( m_geom->vertexlist[i]->connectedFaces.size() > 0 );
	}
}

void MyMesh::getMinMax( const float4* v, int count, float4* minv, float4* maxv )
{
	*minv = float4(0,0,0,0);
	*maxv = float4(0,0,0,0);

	if ( count > 0 )
		*minv = *maxv = v[0];

	for ( int i = 1 ; i < count ; i++ )
	{
		const float4& val = v[i];

		for ( int k = 0 ; k < 4 ; ++k )
		{
			if ( val[k] < (*minv)[k] )
				(*minv)[k] = val[k];
			else if ( val[k] > (*maxv)[k] )
				(*maxv)[k] = val[k];
		}
	}
}

float4 MyMesh::getScaleBias( const float4* v, int count )
{
	float4 minv, maxv;
	getMinMax( v, count, &minv, &maxv );

	float4 scale4 = maxv - minv;
	float4 bias = minv;
	float scale = scale4[0];

	for ( int k = 0 ; k < 4 ; ++k )
	{
		if ( fabsf(scale4[k]) <= FLT_MIN )
			scale4[k] = 1.f;
		if ( scale4[k] > scale )
			scale = scale4[k];
	}
	
	return float4( scale, bias.x, bias.y, bias.z );
}

float4 MyMesh::calculateScaleBias() const {
	Array<float4> vertexList(m_geom->vertexlist.size());
	for(int i = 0; i < m_geom->vertexlist.size(); ++i) {
		vertexList[i] = float4(m_geom->vertexlist[i]->pos, 0.0f);
	}
	return getScaleBias(vertexList.begin(), vertexList.size());
}

void MyMesh::extractPrimitives( int maxbonesperprimitive )
{

	float4 scalebias = calculateScaleBias();

	// collect used materials
	Array<MyMaterial*> materials;
	for ( int i = 0 ; i < m_geom->trianglelist.size() ; ++i )
		materials.add( m_geom->trianglelist[i]->material );
	ArrayUtil::removeDuplicates( materials );

	// create prim / mtl
	node()->removePrimitives();
	for ( int i = 0 ; i < materials.size() ; ++i )
	{
		P(MyPrimitive) prim = new MyPrimitive( materials[i], m_geom, scalebias );
		node()->addPrimitive( prim );
	}

	// make sure primitives dont use too many bones / primitive
	if ( node()->bones() > maxbonesperprimitive )
	{
		// sort bone nodes by depth in hierarchy
		Array<hgr::Node*> bonesbydepth;
		for ( int i = 0 ; i < node()->bones() ; ++i )
			bonesbydepth.add( node()->getBone(i) );
		std::sort( bonesbydepth.begin(), bonesbydepth.end(), NodeDepthLess() );

		// split primitives using too many bones
		for ( int i = 0 ; i < primitives() ; ++i )
		{
			P(MyPrimitive) prim = getPrimitive(i);
			if ( prim->usedBones() > maxbonesperprimitive )
			{
				removePrimitive( i-- );

				P(MyPrimitive) out1=0, out2=0;
				prim->splitPrimitive( maxbonesperprimitive, &out1, &out2 );

				if ( out1 != 0 )
					node()->addPrimitive( out1 );
				if ( out2 != 0 )
					node()->addPrimitive( out2 );
			}
		}
	}
}

bool MyMesh::isSkinned() const
{
	INode* node3ds = this->node3ds();
	if ( node3ds != 0 )
	{
		Modifier* phymod = PhysiqueUtil::findPhysiqueModifier( node3ds );
		Modifier* skinmod = SkinUtil::findSkinModifier( node3ds );
		return phymod != 0 || skinmod != 0;
	}
	return false;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
