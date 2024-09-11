/*********************************************************************NVMH4****
Path:  
File:  

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

  Todo
  -	Performance improvements, right now for each vertex I am building the list
    of it's neighbors, when we could do a single pass and build the full adjacency
	map in the beginning.  Note: I tried this and didn't see a real perf improvement.

  - I'd like to provide a non c++ standard library interface, probably 
    a simple c interface for all those simple c folks. or the 
	old nvMeshMender interface for all those using it already
  
*/


#include "StdAfx.h"
#include <lang/Float.h>
#include <lang/Debug.h>
#include "NVMeshMenderEx.h"
#include <stdint.h>

USING_NAMESPACE(lang)
USING_NAMESPACE(math)

namespace
{
	const unsigned int NO_GROUP=  0xFFFFFFFF;
};

void MeshMender::Triangle::Reset()
{
	handled = false;
	group = NO_GROUP;
}

class MeshMender::CanSmoothChecker
{
public:
	virtual bool CanSmooth(MeshMender::Triangle* t1, MeshMender::Triangle* t2, const float& minCreaseAngle)=0;
};


class CanSmoothNormalsChecker: public MeshMender::CanSmoothChecker
{
public:
	virtual bool CanSmooth(MeshMender::Triangle* t1, MeshMender::Triangle* t2, const float& minCreaseAngle)
	{

		assert(t1 && t2);
		//for checking the angle, we want these to be normalized,
		//they may not be for whatever reason
		D3DXVECTOR3 tmp1 = t1->normal;
		D3DXVECTOR3 tmp2 = t2->normal;
		D3DXVec3Normalize( &tmp1, &tmp1);
		D3DXVec3Normalize( &tmp2, &tmp2);

		if(D3DXVec3Dot( &tmp1, &tmp2 ) >= minCreaseAngle )
		{
			return true;
		}
		else if( ( tmp1 == D3DXVECTOR3(0,0,0) ) && ( tmp2 == D3DXVECTOR3(0,0,0) ) )
		{
			// check for them both being null, then they are 
			// welcome to smooth no matter what the minCreaseAngle is
			return true;
		}
		return false;
	}
};

class CanSmoothTangentsChecker: public MeshMender::CanSmoothChecker
{
public:
	virtual bool CanSmooth(MeshMender::Triangle* t1, MeshMender::Triangle* t2, const float& minCreaseAngle)
	{

		assert(t1 && t2);
		//for checking the angle, we want these to be normalized,
		//they may not be for whatever reason
		D3DXVECTOR3 tmp1 = t1->tangent;
		D3DXVECTOR3 tmp2 = t2->tangent;
		D3DXVec3Normalize( &tmp1, &tmp1);
		D3DXVec3Normalize( &tmp2, &tmp2);

		if(D3DXVec3Dot( &tmp1, &tmp2 ) >= minCreaseAngle )
		{
			return true;
		}
		else if( ( tmp1 == D3DXVECTOR3(0,0,0) ) && ( tmp2 == D3DXVECTOR3(0,0,0) ) )
		{
			// check for them both being null, then they are 
			// welcome to smooth no matter what the minCreaseAngle is
			return true;
		}
		return false;
	}
};

class CanSmoothBinormalsChecker: public MeshMender::CanSmoothChecker
{
public:
	virtual bool CanSmooth(MeshMender::Triangle* t1, MeshMender::Triangle* t2, const float& minCreaseAngle)
	{

		assert(t1 && t2);
		//for checking the angle, we want these to be normalized,
		//they may not be for whatever reason
		D3DXVECTOR3 tmp1 = t1->binormal;
		D3DXVECTOR3 tmp2 = t2->binormal;
		D3DXVec3Normalize( &tmp1, &tmp1);
		D3DXVec3Normalize( &tmp2, &tmp2);

		if(D3DXVec3Dot( &tmp1, &tmp2 ) >= minCreaseAngle )
			return true;
		else if( ( tmp1 == D3DXVECTOR3(0,0,0) ) && ( tmp2 == D3DXVECTOR3(0,0,0) ) )
		{
			// check for them both being null, then they are 
			// welcome to smooth no matter what the minCreaseAngle is
			return true;
		}
		return false;
	}
};


bool operator<( const D3DXVECTOR3& lhs, const D3DXVECTOR3& rhs )
{
	//needed to have a vertex in a map.
	//must be an absolute sort so that we can reliably find the exact
	//position again, not a fuzzy compare for equality based on an epsilon.
    if ( lhs.x == rhs.x )
    {
        if ( lhs.y == rhs.y )
        {
            if ( lhs.z == rhs.z )
            {
                return false;
            }
            else
            {
                return ( lhs.z < rhs.z );
            }
        }
        else
        {
            return ( lhs.y < rhs.y );
        }
    }
    else
    {
        return ( lhs.x < rhs.x );
    }
}


MeshMender::MeshMender()
{
	MinNormalsCreaseCosAngle =   0.3f; 
	MinTangentsCreaseCosAngle =  0.0f;
	MinBinormalsCreaseCosAngle = 0.0f;	
	WeightNormalsByArea = 0.0f;
	m_RespectExistingSplits = DONT_RESPECT_SPLITS;

}
MeshMender::~MeshMender()
{
}

void MeshMender::UpdateIndices(const size_t oldIndex , const size_t newIndex , TriangleList& curGroup)
{
   //make any triangle that used the oldIndex use the newIndex instead

	for( size_t t = 0; t < curGroup.size(); ++t )
	{
		TriID tID = curGroup[ t ];
		for(size_t indx = 0 ; indx < 3 ; ++indx)
		{
			if(m_Triangles[tID].indices[indx] == oldIndex)
			{
				m_Triangles[tID].indices[indx] = newIndex;
			}
		}
	}

}
void MeshMender::ProcessNormals(TriangleList& possibleNeighbors,
								std::vector< Vertex >&    theVerts,
								std::vector< unsigned int >& mappingNewToOldVert,
								D3DXVECTOR3 workingPosition)
{
		NeighborGroupList neighborGroups;//a fresh group for each pass


		//reset each triangle to prepare for smoothing group building
		for( unsigned int i = 0; i < possibleNeighbors.size(); ++i )
		{
			m_Triangles[ possibleNeighbors[i] ].Reset();
		}

		//now start building groups
		CanSmoothNormalsChecker canSmoothNormalsChecker;
		for( i = 0; i < possibleNeighbors.size(); ++i )
		{
			Triangle* currTri = &(m_Triangles[ possibleNeighbors[i] ]);
			assert(currTri);
			if(!currTri->handled)
			{
				BuildGroups(currTri,possibleNeighbors, 
							neighborGroups, theVerts,
							&canSmoothNormalsChecker ,MinNormalsCreaseCosAngle );
			}
		}

		
		std::vector<D3DXVECTOR3> groupNormalVectors;

		for( i = 0; i < neighborGroups.size(); ++i )
		{
			//for each group, calculate the group normal
			TriangleList& curGroup = neighborGroups[ i ];
			D3DXVECTOR3 gnorm( 0.0f, 0.0f, 0.0f );

			assert(curGroup.size()!=0 && "should not be a zero group here.");
			for( size_t t = 0; t < curGroup.size(); ++t )//for each triangle in the group, 
			{
				TriID tID = curGroup[ t ];
				gnorm +=  m_Triangles[ tID ].normal;
			}
			D3DXVec3Normalize( &gnorm, &gnorm );
			groupNormalVectors.push_back( gnorm );
		}
		

		//next step, ensure that triangles in different groups are not
		//sharing vertices. and give the shared vertex their new group vector
		std::set<size_t> otherGroupsIndices;
		for( i = 0; i < neighborGroups.size(); ++i )
		{
			TriangleList& curGroup = neighborGroups[ i ];
			std::set<size_t> thisGroupIndices;

			for( size_t t = 0; t < curGroup.size(); ++t ) //for each tri
			{
				TriID tID = curGroup[ t ];
				for(size_t indx = 0; indx < 3 ; ++indx)//for each vert in that tri
				{
					//if it is at the positions in question
					if( theVerts[ m_Triangles[tID].indices[indx] ].pos  == workingPosition)
					{
						//see if another group is already using this vert
						if(otherGroupsIndices.find( m_Triangles[tID].indices[indx] ) != otherGroupsIndices.end() )
						{
							//then we need to make a new vertex
							Vertex ov;
							ov = theVerts[ m_Triangles[tID].indices[indx] ];
							ov.normal = groupNormalVectors[i];
							size_t oldIndex = m_Triangles[tID].indices[indx];
							size_t newIndex = theVerts.size();
							theVerts.push_back(ov);
							AppendToMapping( oldIndex , m_originalNumVerts , mappingNewToOldVert);
							UpdateIndices(oldIndex,newIndex,curGroup);
						}
						else
						{
							//otherwise, just update it with the new vector
							theVerts[ m_Triangles[tID].indices[indx] ].normal = groupNormalVectors[i];
						}

						//store that we have used this index, so other groups can check
						thisGroupIndices.insert(m_Triangles[tID].indices[indx]);
					}
				}
				
			}
			
			for(std::set<size_t>::iterator it = thisGroupIndices.begin(); it!= thisGroupIndices.end() ; ++it)
			{
				otherGroupsIndices.insert(*it);
			}

		}
	
}

void MeshMender::ProcessTangents(TriangleList& possibleNeighbors,
								std::vector< Vertex >&    theVerts,
								std::vector< unsigned int >& mappingNewToOldVert,
								D3DXVECTOR3 workingPosition)
{
		NeighborGroupList neighborGroups;//a fresh group for each pass


		//reset each triangle to prepare for smoothing group building
		for(unsigned int i =0; i < possibleNeighbors.size(); ++i)
		{
			m_Triangles[ possibleNeighbors[i] ].Reset();
		}

		//now start building groups
		CanSmoothTangentsChecker canSmoothTangentsChecker;
		for(i =0; i < possibleNeighbors.size(); ++i)
		{
			Triangle* currTri = &(m_Triangles[ possibleNeighbors[i] ]);
			assert(currTri);
			if(!currTri->handled)
			{
				BuildGroups(currTri,possibleNeighbors, 
							neighborGroups, theVerts,
							&canSmoothTangentsChecker,MinTangentsCreaseCosAngle  );
			}
		}


		std::vector<D3DXVECTOR3> groupTangentVectors;
	
		
		for(i=0; i<neighborGroups.size(); ++i)
		{
			D3DXVECTOR3 gtang(0,0,0);
			for(unsigned int t = 0; t < neighborGroups[i].size(); ++t)//for each triangle in the group, 
			{
				TriID tID = neighborGroups[i][t];
				gtang+=  m_Triangles[tID].tangent;
			}
			D3DXVec3Normalize( &gtang, &gtang );
			groupTangentVectors.push_back(gtang);
		}
		
		//next step, ensure that triangles in different groups are not
		//sharing vertices. and give the shared vertex their new group vector
		std::set<size_t> otherGroupsIndices;
		for( i = 0; i < neighborGroups.size(); ++i )
		{
			TriangleList& curGroup = neighborGroups[ i ];
			std::set<size_t> thisGroupIndices;

			for( size_t t = 0; t < curGroup.size(); ++t ) //for each tri
			{
				TriID tID = curGroup[ t ];
				for(size_t indx = 0; indx < 3 ; indx ++)//for each vert in that tri
				{
					//if it is at the positions in question
					if( theVerts[ m_Triangles[tID].indices[indx] ].pos  == workingPosition)
					{
						//see if another group is already using this vert
						if(otherGroupsIndices.find( m_Triangles[tID].indices[indx] ) != otherGroupsIndices.end() )
						{
							//then we need to make a new vertex
							Vertex ov;
							ov = theVerts[ m_Triangles[tID].indices[indx] ];
							ov.tangent = groupTangentVectors[i];
							size_t oldIndex = m_Triangles[tID].indices[indx];
							size_t newIndex = theVerts.size();
							theVerts.push_back(ov);
							AppendToMapping( oldIndex , m_originalNumVerts , mappingNewToOldVert);
							UpdateIndices(oldIndex,newIndex,curGroup);
						}
						else
						{
							//otherwise, just update it with the new vector
							theVerts[ m_Triangles[tID].indices[indx] ].tangent = groupTangentVectors[i];
						}

						//store that we have used this index, so other groups can check
						thisGroupIndices.insert(m_Triangles[tID].indices[indx]);
					}
				}
				
			}
			
			for(std::set<size_t>::iterator it = thisGroupIndices.begin(); it!= thisGroupIndices.end() ; ++it)
			{
				otherGroupsIndices.insert(*it);
			}

		}


}


void MeshMender::ProcessBinormals(TriangleList& possibleNeighbors,
								std::vector< Vertex >&    theVerts,
								std::vector< unsigned int >& mappingNewToOldVert,
								D3DXVECTOR3 workingPosition)
{
		NeighborGroupList neighborGroups;//a fresh group for each pass


		//reset each triangle to prepare for smoothing group building
		for(unsigned int i =0; i < possibleNeighbors.size(); ++i )
		{
			m_Triangles[ possibleNeighbors[i] ].Reset();
		}

		//now start building groups
		CanSmoothBinormalsChecker canSmoothBinormalsChecker;
		for(i =0; i < possibleNeighbors.size(); ++i )
		{
			Triangle* currTri = &(m_Triangles[ possibleNeighbors[i] ]);
			assert(currTri);
			if(!currTri->handled)
			{
				BuildGroups(currTri,possibleNeighbors, 
							neighborGroups, theVerts,
							&canSmoothBinormalsChecker ,MinBinormalsCreaseCosAngle );
			}
		}


		std::vector<D3DXVECTOR3> groupBinormalVectors;
	
		
		for(i=0; i<neighborGroups.size(); ++i)
		{
			D3DXVECTOR3 gbinormal(0,0,0);
			for(unsigned int t = 0; t < neighborGroups[i].size(); ++t)//for each triangle in the group, 
			{
				TriID tID = neighborGroups[i][t];
				gbinormal+=  m_Triangles[tID].binormal;
			}
			D3DXVec3Normalize( &gbinormal, &gbinormal );
			groupBinormalVectors.push_back(gbinormal);
		}
		
		//next step, ensure that triangles in different groups are not
		//sharing vertices. and give the shared vertex their new group vector
		std::set<size_t> otherGroupsIndices;
		for( i = 0; i < neighborGroups.size(); ++i )
		{
			TriangleList& curGroup = neighborGroups[ i ];
			std::set<size_t> thisGroupIndices;

			for( size_t t = 0; t < curGroup.size(); ++t ) //for each tri
			{
				TriID tID = curGroup[ t ];
				for(size_t indx = 0; indx < 3 ; ++indx)//for each vert in that tri
				{
					//if it is at the positions in question
					if( theVerts[ m_Triangles[tID].indices[indx] ].pos  == workingPosition)
					{
						//see if another group is already using this vert
						if(otherGroupsIndices.find( m_Triangles[tID].indices[indx] ) != otherGroupsIndices.end() )
						{
							//then we need to make a new vertex
							Vertex ov;
							ov = theVerts[ m_Triangles[tID].indices[indx] ];
							ov.binormal = groupBinormalVectors[i];
							size_t oldIndex = m_Triangles[tID].indices[indx];
							size_t newIndex = theVerts.size();
							theVerts.push_back(ov);
							AppendToMapping( oldIndex , m_originalNumVerts , mappingNewToOldVert);
							UpdateIndices(oldIndex,newIndex,curGroup);
						}
						else
						{
							//otherwise, just update it with the new vector
							theVerts[ m_Triangles[tID].indices[indx] ].binormal = groupBinormalVectors[i];
						}

						//store that we have used this index, so other groups can check
						thisGroupIndices.insert(m_Triangles[tID].indices[indx]);
					}
				}
				
			}
			
			for(std::set<size_t>::iterator it = thisGroupIndices.begin(); it!= thisGroupIndices.end() ; ++it)
			{
				otherGroupsIndices.insert(*it);
			}

		}

}


bool MeshMender::Mend( 
                  std::vector< Vertex >&    theVerts,
				  std::vector< unsigned int >& theIndices,
				  std::vector< unsigned int >& mappingNewToOldVert,
				  const float minNormalsCreaseCosAngle,
				  const float minTangentsCreaseCosAngle,
				  const float minBinormalsCreaseCosAngle,
				  const float weightNormalsByArea,
				  const NormalCalcOption computeNormals,
				  const ExistingSplitOption respectExistingSplits,
				  const CylindricalFixOption fixCylindricalWrapping)
{
	Debug::printf( "MeshMender: Mend( verts=%d, indices=%d, ... )\n", theVerts.size(), theIndices.size() );

	MinNormalsCreaseCosAngle = minNormalsCreaseCosAngle;
	MinTangentsCreaseCosAngle= minTangentsCreaseCosAngle;
	MinBinormalsCreaseCosAngle = minBinormalsCreaseCosAngle;
	WeightNormalsByArea = weightNormalsByArea;
	m_RespectExistingSplits = respectExistingSplits;

	//fix cylindrical should happen before we do any other calculations
	if(fixCylindricalWrapping == FIX_CYLINDRICAL)
	{
		FixCylindricalWrapping( theVerts , theIndices , mappingNewToOldVert );
	}


	SetUpData( theVerts, theIndices, mappingNewToOldVert, computeNormals);

	//for each unique position
	for(VertexChildrenMap::iterator vert = m_VertexChildrenMap.begin();
		vert!= m_VertexChildrenMap.end();
		++vert)
	{
		D3DXVECTOR3 workingPosition = vert->first;

		TriangleList& possibleNeighbors = vert->second;
		if(computeNormals == CALCULATE_NORMALS)
		{
			ProcessNormals(possibleNeighbors, theVerts, mappingNewToOldVert, workingPosition );
		}
		ProcessTangents(possibleNeighbors, theVerts, mappingNewToOldVert, workingPosition );
		ProcessBinormals(possibleNeighbors, theVerts, mappingNewToOldVert, workingPosition );
	}

	UpdateTheIndicesWithFinalIndices(theIndices );
	OrthogonalizeTangentsAndBinormals(theVerts);
		
	Debug::printf( "MeshMender: Mend() done\n" );
	return true;
}

void MeshMender::BuildGroups(	Triangle* tri, //the tri of interest
								TriangleList& possibleNeighbors, //all tris arround a vertex
								NeighborGroupList& neighborGroups, //the neighbor groups to be updated
								std::vector< Vertex >& theVerts,
								CanSmoothChecker* smoothChecker,
								const float& minCreaseAngle)
{
	if( (!tri)  ||  (tri->handled) )
		return;
	
	Triangle* neighbor1 = NULL;
	Triangle* neighbor2 = NULL;

	FindNeighbors(tri, possibleNeighbors, &neighbor1, &neighbor2,theVerts);
	
	//see if I can join my first neighbors group
	if(neighbor1 && (neighbor1->group != NO_GROUP))
	{
		if( smoothChecker->CanSmooth(tri,neighbor1, minCreaseAngle) )
		{
			neighborGroups[neighbor1->group].push_back(tri->myID);
			tri->group = neighbor1->group;
		}
	}

	//see if I can join my second neighbors group
	if(neighbor2 && (neighbor2->group!= NO_GROUP))
	{
		if( smoothChecker->CanSmooth(tri,neighbor2, minCreaseAngle) )
		{
			neighborGroups[neighbor2->group].push_back(tri->myID);
			tri->group = neighbor2->group;
		}
	}
	//I either couldn't join, or they weren't in a group, so I think I'll
	//just go and start my own group...right here we go.
	if(tri->group == NO_GROUP)
	{
		tri->group = neighborGroups.size();
		neighborGroups.push_back( TriangleList() );
		neighborGroups.back().push_back(tri->myID);

	}
	assert((tri->group != NO_GROUP) && "error!: tri should have a group set");
	tri->handled = true;

	//continue growing our group with each neighbor.
	BuildGroups(neighbor1,possibleNeighbors,neighborGroups,theVerts,smoothChecker,minCreaseAngle);
	BuildGroups(neighbor2,possibleNeighbors,neighborGroups,theVerts,smoothChecker,minCreaseAngle);
}


void MeshMender::FindNeighbors(Triangle* tri, 
				   TriangleList&possibleNeighbors, 
				   Triangle** neighbor1, 
				   Triangle** neighbor2,
				   std::vector< Vertex >& theVerts)
{
	*neighbor1 = NULL;
	*neighbor2 = NULL;

	std::vector<Triangle*> theNeighbors;
	for(unsigned int n = 0; n < possibleNeighbors.size(); ++n)
	{
		TriID tID = possibleNeighbors[n];
		Triangle* possible =&(m_Triangles[ tID]);
		if(possible != tri ) //check for myself
		{
			if( SharesEdge(tri, possible, theVerts)  )
			{
				theNeighbors.push_back(possible);  

			}
		}
	}
	
	if(theNeighbors.size()>0)
		*neighbor1 = theNeighbors[0];
	if(theNeighbors.size()>1)
		*neighbor2 = theNeighbors[1];
}



bool MeshMender::TriHasEdge(const size_t& p0,
							const size_t& p1,
							const size_t& triA,
							const size_t& triB,
							const size_t& triC)
{
	if ( ( ( p0 == triB ) && ( p1 == triA ) ) ||
	     ( ( p0 == triA ) && ( p1 == triB ) ) )
	{
		return true;
	}

	if ( ( ( p0 == triB ) && ( p1 == triC ) ) ||
	     ( ( p0 == triC ) && ( p1 == triB ) ) )
	{
		return true;
	}

	if ( ( ( p0 == triC ) && ( p1 == triA ) ) ||
	     ( ( p0 == triA ) && ( p1 == triC ) ) )
	{
		return true;
	}
	return false;
}

bool MeshMender::TriHasEdge(const D3DXVECTOR3& p0,
							const D3DXVECTOR3& p1,
							const D3DXVECTOR3& triA,
							const D3DXVECTOR3& triB,
							const D3DXVECTOR3& triC)
{
	if ( ( ( p0 == triB ) && ( p1 == triA ) ) ||
	     ( ( p0 == triA ) && ( p1 == triB ) ) )
	{
		return true;
	}

	if ( ( ( p0 == triB ) && ( p1 == triC ) ) ||
	     ( ( p0 == triC ) && ( p1 == triB ) ) )
	{
		return true;
	}

	if ( ( ( p0 == triC ) && ( p1 == triA ) ) ||
	     ( ( p0 == triA ) && ( p1 == triC ) ) )
	{
		return true;
	}
	return false;
}

bool MeshMender::SharesEdgeRespectSplits(	Triangle* triA, 
									Triangle* triB,
				std::vector< Vertex >& theVerts)
{
	assert(triA && triB && "invalid data passed to SharesEdgeNoSplit");
	//here we want to compare based solely on indices.

	size_t a1 = triA->indices[0];
	size_t b1 = triA->indices[1];
	size_t c1 = triA->indices[2];

	size_t a2 = triB->indices[0];
	size_t b2 = triB->indices[1];
	size_t c2 = triB->indices[2];

	//edge B1->A1
	if( TriHasEdge(b1,a1,a2,b2,c2)  )
		return true;

	//edge A1->C1
	if( TriHasEdge(a1,c1,a2,b2,c2)  )
		return true;
	
	//edge C1->B1
	if( TriHasEdge(c1,b1,a2,b2,c2)  )
		return true;

	return false;
}

bool MeshMender::SharesEdge(Triangle* triA, 
				Triangle* triB,
				std::vector< Vertex >& theVerts)
{
	assert(triA && triB && "invalid data passed to SharesEdge");

	//check based on position not on indices, because there may be splits
	//we don't care about. unless the user has told us they care about those
	//splits
	if(m_RespectExistingSplits == RESPECT_SPLITS)
	{
		return SharesEdgeRespectSplits(triA, triB, theVerts);
	}

	D3DXVECTOR3 a1 = theVerts[ triA->indices[0] ].pos;
	D3DXVECTOR3 b1 = theVerts[ triA->indices[1] ].pos;
	D3DXVECTOR3 c1 = theVerts[ triA->indices[2] ].pos;

	D3DXVECTOR3 a2 = theVerts[ triB->indices[0] ].pos;
	D3DXVECTOR3 b2 = theVerts[ triB->indices[1] ].pos;
	D3DXVECTOR3 c2 = theVerts[ triB->indices[2] ].pos;

	//edge B1->A1
	if( TriHasEdge(b1,a1,a2,b2,c2)  )
		return true;

	//edge A1->C1
	if( TriHasEdge(a1,c1,a2,b2,c2)  )
		return true;
	
	//edge C1->B1
	if( TriHasEdge(c1,b1,a2,b2,c2)  )
		return true;
	
	return false;
}

void MeshMender::SetUpData(
				  std::vector< Vertex >&    theVerts,
				  std::vector< unsigned int >& theIndices,
				  std::vector< unsigned int >& mappingNewToOldVert,
				  const NormalCalcOption computeNormals)
{
	assert( ((theIndices.size()%3 )== 0) && "expected the indices to be a multiple of 3");
	unsigned int i;

	//initialize the mapping
	for(i = 0 ; i < theVerts.size() ; ++i)
		mappingNewToOldVert.push_back(i);

	m_originalNumVerts = theVerts.size();

	//set up our triangles
	for(i = 0; i < theIndices.size() ; i += 3)
	{
		Triangle t;

		t.indices[0] = theIndices[i+0];
		t.indices[1] = theIndices[i+1];
		t.indices[2] = theIndices[i+2];

		//set up bin, norm, and tan
		SetUpFaceVectors(t,theVerts, computeNormals);
		
		t.myID = m_Triangles.size();//set id, to my index into m_Triangles
		m_Triangles.push_back(t);
	}

	//build vertex position/traingle pairings.
	//we use the position and not the actual vertex, because there may
	//be multiple coppies of the same vertex for textureing
	//but we don't want that to 
	//effect our decisions about normal smoothing.  
	//note: maybe this should be an option, the position thing.
	for(i= 0; i< m_Triangles.size();++i)
	{

		for(size_t indx = 0 ; indx < 3 ; ++indx )
		{
			D3DXVECTOR3 v = theVerts[m_Triangles[i].indices[indx]].pos;
			VertexChildrenMap::iterator iter = m_VertexChildrenMap.find( v );
			if(iter != m_VertexChildrenMap.end())
			{
				//we found it, so just add ourselves to it.
				iter->second.push_back(TriID(i));
			}
			else
			{
				//we didn't find it so join whatever was there.
				std::vector<TriID> tmp;
				m_VertexChildrenMap[v] = tmp;
				m_VertexChildrenMap[v].push_back( TriID(i) );
			}
		}
	}
}

//sets up the normal, binormal, and tangent for a triangle
//assumes the triangle indices are set to match whats in the verts
void MeshMender::SetUpFaceVectors(Triangle& t, 
								  const std::vector< Vertex >&verts, 
								  const NormalCalcOption computeNormals)
{

	if(computeNormals == CALCULATE_NORMALS)
	{ 
		D3DXVECTOR3 edge0 = verts[t.indices[1]].pos - verts[t.indices[0]].pos;
		D3DXVECTOR3 edge1 = verts[t.indices[2]].pos - verts[t.indices[0]].pos;

		D3DXVec3Cross( &t.normal, &edge0, &edge1);

		if( WeightNormalsByArea < 1.0f )
		{
			
			D3DXVECTOR3 normalizedNorm;
			D3DXVec3Normalize(&normalizedNorm,&t.normal);
			D3DXVECTOR3 finalNorm = (normalizedNorm * (1.0f - WeightNormalsByArea))
							+ (t.normal * WeightNormalsByArea);
			t.normal = finalNorm;
		}

	}
	//need to set up tangents, and binormals here
	GetGradients(  verts[ t.indices[0] ],
                   verts[ t.indices[1] ],
                   verts [t.indices[2] ],
                   t.tangent,
                   t.binormal);
}

void MeshMender::OrthogonalizeTangentsAndBinormals( 
						std::vector< Vertex >&   theVerts )
{
	//put our tangents and binormals through the final orthogonalization
	//with the final processed normals
	size_t len = theVerts.size();
	for(size_t i = 0 ; i < len ; ++ i )
	{

		assert( D3DXVec3Length(&(theVerts[i].normal)) > 0.00001f && 
			"found zero length normal when calculating tangent basis!,\
			if you are not using mesh mender to compute normals, you\
			must still pass in valid normals to be used when calculating\
			tangents and binormals.");


		//now with T and B and N we can get from tangent space to object space
		//but we want to go the other way, so we need the inverse
		//of the T, B,N matrix
		//we can use the Gram-Schmidt algorithm to find the newTangent and the newBinormal
		//newT = T - (N dot T)N
		//newB = B - (N dot B)N - (newT dot B)newT

		//NOTE: this should maybe happen with the final smoothed N, T, and B
		//will try it here and see what the results look like

		D3DXVECTOR3 tmpTan = theVerts[i].tangent;
		D3DXVECTOR3 tmpNorm = theVerts[i].normal;
		D3DXVECTOR3 tmpBin = theVerts[i].binormal;


		D3DXVECTOR3 newT = tmpTan -  (D3DXVec3Dot(&tmpNorm , &tmpTan)  * tmpNorm );
		D3DXVECTOR3 newB = tmpBin - (D3DXVec3Dot(&tmpNorm , &tmpBin) * tmpNorm)
							- (D3DXVec3Dot(&newT,&tmpBin)*newT);

		D3DXVec3Normalize(&(theVerts[i].tangent), &newT);
		D3DXVec3Normalize(&(theVerts[i].binormal), &newB);		

		//this is where we can do a final check for zero length vectors
		//and set them to something appropriate
		float lenTan = D3DXVec3Length(&(theVerts[i].tangent));
		float lenBin = D3DXVec3Length(&(theVerts[i].binormal));

		if( (lenTan <= 0.001f) || (lenBin <= 0.001f)  ) //should be approx 1.0f
		{	
			//the tangent space is ill defined at this vertex
			//so we can generate a valid one based on the normal vector,
			//which I'm assuming is valid!

			if(lenTan > 0.5f)
			{
				//the tangent is valid, so we can just use that
				//to calculate the binormal
				D3DXVec3Cross(&(theVerts[i].binormal), &(theVerts[i].normal), &(theVerts[i].tangent) );

			}
			else if(lenBin > 0.5)
			{
				//the binormal is good and we can use it to calculate
				//the tangent
				D3DXVec3Cross(&(theVerts[i].tangent), &(theVerts[i].binormal), &(theVerts[i].normal) );
			}
			else
			{
				//both vectors are invalid, so we should create something
				//that is at least valid if not correct
				D3DXVECTOR3 xAxis( 1.0f , 0.0f , 0.0f);
				D3DXVECTOR3 yAxis( 0.0f , 1.0f , 0.0f);
				//I'm checking two possible axis, because the normal could be one of them,
				//and we want to chose a different one to start making our valid basis.
				//I can find out which is further away from it by checking the dot product
				D3DXVECTOR3 startAxis;

				if( D3DXVec3Dot(&xAxis, &(theVerts[i].normal) )  <  D3DXVec3Dot(&yAxis, &(theVerts[i].normal) ) )
				{
					//the xAxis is more different than the yAxis when compared to the normal
					startAxis = xAxis;
				}
				else
				{
					//the yAxis is more different than the xAxis when compared to the normal
					startAxis = yAxis;
				}

				D3DXVec3Cross(&(theVerts[i].tangent), &(theVerts[i].normal), &startAxis );
				D3DXVec3Cross(&(theVerts[i].binormal), &(theVerts[i].normal), &(theVerts[i].tangent) );

			}
		}
		else
		{
			//one final sanity check, make sure that they tangent and binormal are different enough
			if( D3DXVec3Dot(&(theVerts[i].binormal), &(theVerts[i].tangent) )  > 0.999f )
			{
				//then they are too similar lets make them more different
				D3DXVec3Cross(&(theVerts[i].binormal), &(theVerts[i].normal), &(theVerts[i].tangent) );
			}

		}

	}
	
}

void MeshMender::GetGradients( const MeshMender::Vertex& v0,
                               const MeshMender::Vertex& v1,
                               const MeshMender::Vertex& v2,
                               D3DXVECTOR3& tangent,
                               D3DXVECTOR3& binormal) const
{
	//using Eric Lengyel's approach with a few modifications
	//from Mathematics for 3D Game Programmming and Computer Graphics
	// want to be able to trasform a vector in Object Space to Tangent Space
	// such that the x-axis cooresponds to the 's' direction and the
	// y-axis corresponds to the 't' direction, and the z-axis corresponds
	// to <0,0,1>, straight up out of the texture map

	//let P = v1 - v0
	D3DXVECTOR3 P = v1.pos - v0.pos;
	//let Q = v2 - v0
	D3DXVECTOR3 Q = v2.pos - v0.pos;
	float s1 = v1.s - v0.s;
	float t1 = v1.t - v0.t;
	float s2 = v2.s - v0.s;
	float t2 = v2.t - v0.t;


	//we need to solve the equation
	// P = s1*T + t1*B
	// Q = s2*T + t2*B
	// for T and B


	//this is a linear system with six unknowns and six equatinos, for TxTyTz BxByBz
	//[px,py,pz] = [s1,t1] * [Tx,Ty,Tz]
	// qx,qy,qz     s2,t2     Bx,By,Bz

	//multiplying both sides by the inverse of the s,t matrix gives
	//[Tx,Ty,Tz] = 1/(s1t2-s2t1) *  [t2,-t1] * [px,py,pz]
	// Bx,By,Bz                      -s2,s1	    qx,qy,qz  

	//solve this for the unormalized T and B to get from tangent to object space

	
	float tmp = 0.0f;
	if(fabsf(s1*t2 - s2*t1) <= 0.0001f)
	{
		tmp = 1.0f;
	}
	else
	{
		tmp = 1.0f/(s1*t2 - s2*t1 );
	}
	
	tangent.x = (t2*P.x - t1*Q.x);
	tangent.y = (t2*P.y - t1*Q.y);
	tangent.z = (t2*P.z - t1*Q.z);
	
	tangent = tmp * tangent;

	binormal.x = (s1*Q.x - s2*P.x);
	binormal.y = (s1*Q.y - s2*P.y);
	binormal.z = (s1*Q.z - s2*P.z);

	binormal = tmp * binormal;



	//after these vectors are smoothed together,
	//they must be again orthogonalized with the final normals
	// see OrthogonalizeTangentsAndBinormals

}

void MeshMender::UpdateTheIndicesWithFinalIndices(std::vector< unsigned int >& theIndices )
{
	//theIndices is assumed to be filled with a copy of the in Indices.

	assert(((theIndices.size()/3) == m_Triangles.size()) && "invalid number of tris, or indices.");
	//Note that we do not change the number or the order of indices at all,
	//so we just need to copy the triangles indices to the output.
	size_t oIndex = 0;

	for( size_t i = 0; i < m_Triangles.size();++i )
	{
		theIndices[oIndex+0] = m_Triangles[i].indices[0];
		theIndices[oIndex+1] = m_Triangles[i].indices[1];
		theIndices[oIndex+2] = m_Triangles[i].indices[2];
		oIndex+=3;
	}
}


void MeshMender::FixCylindricalWrapping(	std::vector< Vertex >& theVerts , 
								std::vector< unsigned int >& theIndices,
								std::vector< unsigned int >& mappingNewToOldVert)
{
	//when using cylindrical texture coordinate generation,
	//you can end up with triangles that have <s,t> coordinates like
	// <0,0.9> -------------> <0,0>
	// and
	// <0,0.9> -------------> <0,0.1>
	//this will cause the texture to be mapped from 0.9 back to 0.0 or 0.1 when
	//what you really want it to do is 
	//wrap arround to 1.0, then start from 0.0 again.
	//to fix this, we can duplicate a vertex and add 1.0 to the wrapped texture coordinate
	//we need to do this for both the S and the T directions.

	size_t index;
	for(index = 0 ;  index < theIndices.size() ; index += 3)
	{
		//for each triangle
		std::set<unsigned int> alreadyDuped;

		
		for( unsigned int begin = 0 ; begin < 3 ; ++begin)
		{

			unsigned int end = begin +1;
			if(begin == 2)
				end = 0;
			//for each   begin -> end   edge

			float sBegin = theVerts[ theIndices[index + begin] ].s;
			float sEnd = theVerts[ theIndices[index + end] ].s;

			if( sBegin <= 1.0f  && sEnd <= 1.0f  && sBegin >= 0.0f  && sEnd >= 0.0f  )
			{
				//we only handle coordinates between 0 and 1 for the cylindrical wrappign fix
				if( fabsf(sBegin - sEnd) > 0.5f )
				{
					unsigned int theOneToDupe = begin;
					//we have some wrapping going on.
					if(sBegin > sEnd)
						theOneToDupe = end;

					if(alreadyDuped.find(theOneToDupe) == alreadyDuped.end())
					{
						size_t oldIndex = theIndices[index + theOneToDupe];
						Vertex theDupe = theVerts[ oldIndex ];
						alreadyDuped.insert(theOneToDupe);
						theDupe.s += 1.0f;
						theIndices[index + theOneToDupe] = theVerts.size();
						theVerts.push_back(theDupe);
						AppendToMapping( oldIndex , m_originalNumVerts , mappingNewToOldVert);
					}
					else
					{
						theVerts[ theIndices[index + theOneToDupe] ].s += 1.0f;
					}
				}
			}



			float tBegin = theVerts[ theIndices[index + begin] ].t;
			float tEnd = theVerts[ theIndices[index + end] ].t;

			if( tBegin <= 1.0f  && tEnd <= 1.0f  && tBegin >= 0.0f  && tEnd >= 0.0f  )
			{
				//we only handle coordinates between 0 and 1 for the cylindrical wrappign fix
				if( fabsf(tBegin - tEnd) > 0.5f )
				{
					unsigned int theOneToDupe = begin;
					//we have some wrapping going on.
					if(tBegin > tEnd)
						theOneToDupe = end;

					if(alreadyDuped.find(theOneToDupe) == alreadyDuped.end())
					{
						size_t oldIndex = theIndices[index + theOneToDupe];
						Vertex theDupe = theVerts[ oldIndex ];
						alreadyDuped.insert(theOneToDupe);
						theDupe.t += 1.0f;
						theIndices[index + theOneToDupe] = theVerts.size();
						theVerts.push_back(theDupe);
						AppendToMapping( oldIndex , m_originalNumVerts , mappingNewToOldVert);
					}
					else
					{
						theVerts[ theIndices[index + theOneToDupe] ].t += 1.0f;
					}
				}
			}


		}
	}

}


void MeshMender::AppendToMapping(	const size_t oldIndex,
						const size_t originalNumVerts,
						std::vector< unsigned int >& mappingNewToOldVert)
{
	if( oldIndex >= originalNumVerts )
	{
		//then this is a newer vertex we are mapping to another vertex we created in meshmender.
		//we need to find the original old vertex index to map to.
		// so we can just use the mapping
	
		//that is to say, just keep the same mapping for this new one.
		unsigned int originalVertIndex = mappingNewToOldVert[oldIndex];
		assert( originalVertIndex < originalNumVerts );
		
		mappingNewToOldVert.push_back( originalVertIndex ); 
	}
	else
	{
		//this is mapping to an original vertex
		mappingNewToOldVert.push_back( oldIndex );
	}
}


/**
 * Takes a vertex buffer with vertices of type NVTangentVertex.
 *
 * This finds vertices within epsilon in position and averages their tangent
 * bases to make a smooth tangent space across the model.  This is useful for
 * lathed objects or authored models which duplicate vertices along material
 * boundaries.
 * Tangent Basis must have already been computed for this to work! =)
 *
 * @param smoothnormals If true then the vertex normals are averaged too
 */
static void findAndFixDegenerateVertexBasis( NVTangentVertex* vertexdata, int vertices, int* /*indexdata*/, int /*indices*/, bool smoothnormals )
{
	assert( vertexdata != 0 );

	float epsilon = 1.0e-9f;
	float x,y,z,dist;

	////////////////////////////////////////////////////////////////

	int i,j;

	////////////////////////////////////////////////////////////////
	// Sloppy, but allocate a pointer and char for each vertex
	// As overlapping vertices are found, increment their duplicate_count
	//   and allocate an array of MAX_OVERLAP vertex indices to reference
	//   which vertices overlap.

	const int MAX_OVERLAP = 50;

	Array<uint8_t> duplicate_count( vertices );
		// duplicate_index is array of pointers to bins.  Each bin is
		// allocated when a match is found.
	Array<int*> duplicate_index( vertices );

	memset( duplicate_count.begin(), 0, vertices * sizeof( uint8_t  ) );
	memset( duplicate_index.begin(), 0, vertices * sizeof( int* ) );


	// Need to search the mesh for vertices with the same spatial coordinate
	// These are vertices duplicated for lathed/wrapped objects to make a
	//   2nd set of texture coordinates at the point in order to avoid bad
	//   texture wrapping
	// In short:  For each vertex, find any other vertices that share it's 
	//   position.  "Average" the tangent space basis calculated above for
	//   these duplicate vertices.  This is not rigorous, but works well 
	//   to fix up authored models.  ** Models should not have t juntions! **

	// Check each vert with every other.  There's no reason to check
	//   j with i after doing i with j, so start j from i ( so we test
	//   1 with 2 but never 2 with 1 again).
	// This is a model pre-processing step and only done once.  For large
	//   models, compute this off-line if you have to and store the resultant
	//   data.
	// The whole thing could be made much more efficient (linked list, etc)

	bool once = true;
	for( i=0; i < vertices; i++ )
	{
		for(j=i+1; j < vertices; j++ )
		{
			x = vertexdata[i].pos.x - vertexdata[j].pos.x;
			y = vertexdata[i].pos.y - vertexdata[j].pos.y;
			z = vertexdata[i].pos.z - vertexdata[j].pos.z;

			dist = x*x + y*y + z*z;
			
			if( dist < epsilon )
			{
				// if i matches j and k, just record into i.  j will be 
				//  half full as it will only match k, but this is
				//  taken care of when i is processed.
				if( duplicate_count[i] == 0 )
				{
					// allocate bin
					duplicate_index[i] = new int[MAX_OVERLAP];
				}
				if( duplicate_count[i] < MAX_OVERLAP )
				{
					assert( j <= 32767 );
					duplicate_index[i][duplicate_count[i]] = (int) j;
					duplicate_count[i] ++;
				}
				else
				{
					//FDebug("Ran out of bin storage!!\n");
					if ( once ) 
					{
						Debug::printf( "findAndFixDegenerateVertexBasis: vertex data:\n" );
						for ( int n = 0 ; n < vertices ; ++n )
							Debug::printf( "v%d = (%g %g %g)\n", n, vertexdata[n].pos.x, vertexdata[n].pos.y, vertexdata[n].pos.z );

						char buf[MAX_OVERLAP*10];
						buf[0] = 0;
						for ( int k = 0 ; k < MAX_OVERLAP ; ++k )
							sprintf( buf+strlen(buf), "%d,", int(duplicate_index[i][k]) );
						Debug::printf( "ERROR: findAndFixDegenerateVertexBasis ran out of bin storage! (%s)\n", buf );
						once = false;
					}
					//assert( false );
				}
			}
		}
	}

	// Now average the tangent spaces & write the new result to
	//  each duplicated vertex


	float3	S_temp, T_temp, SxT_temp, N_temp;


	for( i = 0; i < vertices; i++ )
	{
		// do < 10 check to not average the basis at poles of sphere or
		//  other ridiculous geometry with too many degenerate vertices

		if( duplicate_count[i] > 0 && duplicate_count[i] < 10 )
		{
			//	FDebug("Averaging vert prop at %d for %d vertices\n", i, duplicate_count[i]);

			// If there are more than 5 vertices sharing this point then
			//  the point is probably some kind of lathe axis node.  No need to
			//  process it here

			// Set accumulator to value for vertex in question

			S_temp		= vertexdata[i].s;
			T_temp		= vertexdata[i].t;
			SxT_temp	= vertexdata[i].sxt;
			N_temp		= vertexdata[i].normal;

			// add in basis vectors for all other vertices which
			//  have the same positon (found above)

			for( j=0; j < duplicate_count[i]; j++ )
			{
				S_temp		= S_temp   + vertexdata[duplicate_index[i][j]].s;
				T_temp		= T_temp   + vertexdata[duplicate_index[i][j]].t;
				SxT_temp	= SxT_temp + vertexdata[duplicate_index[i][j]].sxt;

				N_temp		= N_temp   + vertexdata[duplicate_index[i][j]].normal;
			}

			// Normalize the basis vectors
			// Note that sxt might not be perpendicular to s and t
			//  anymore.  Not absolutely necessary to re-do the 
			//  cross product.

			S_temp = normalize0(S_temp);
			T_temp = normalize0(T_temp);
			SxT_temp = normalize0(SxT_temp);
			N_temp = normalize0(N_temp);

			// Write the average basis to the first vertex for which
			//   the duplicates were found

			vertexdata[i].s = S_temp;
			vertexdata[i].t = T_temp;
			vertexdata[i].sxt = SxT_temp;

			if( smoothnormals )
				vertexdata[i].normal = N_temp;

			// Now write to all later vertices with the same position
				
			for(j=0; j < duplicate_count[i]; j++ )
			{
				// Set the vertices in the same position to
				//  the average basis.

				vertexdata[duplicate_index[i][j]].s = S_temp;
				vertexdata[duplicate_index[i][j]].t = T_temp;
				vertexdata[duplicate_index[i][j]].sxt = SxT_temp;

				if( smoothnormals )
					vertexdata[duplicate_index[i][j]].normal = N_temp;


				// Kill the duplicate index lists of all vertices of
				//  higher index which overlap this one.  This is so
				//  higher index vertices do not average a smaller 
				//  subset of bases.
				// Arrays are de-allocated later

				duplicate_count[ duplicate_index[i][j] ] = 0;

			}

		}

		if( duplicate_index[i] != 0 )
		{
			delete [] duplicate_index[i];
			duplicate_index[i] = 0;
			duplicate_count[i] = 0;
		}
	}
}

/**
 * Creates basis vectors, based on a vertex and index list.
 *
 * NOTE: Assumes an indexed triangle list, with vertices of type NVTangentVertex
 * Does not check for degenerate vertex positions - ie vertices with same 
 * position but different texture coords or normals.  Another function 
 * can do this to average the basis vectors & "smooth" the tangent space
 * for those duplicated vertices.
 */
void createBasisVectors( NVTangentVertex* vertexdata, int* indexdata, int vertices, int indices, bool smoothnormals )
{
	int i;

	// Clear the basis vectors
	for (i = 0; i < vertices; i++)
	{
		vertexdata[i].s = float3(0.0f, 0.0f, 0.0f);
		vertexdata[i].t = float3(0.0f, 0.0f, 0.0f);
	}

	// Walk through the triangle list and calculate gradiants for each triangle.
	// Sum the results into the s and t components.
    for( i = 0; i < indices; i += 3 )
    {       
		int TriIndex[3];
		float3 du, dv;
		float3 edge01;
		float3 edge02;
		float3 cp;
		
		TriIndex[0] = indexdata[i];
		TriIndex[1] = indexdata[i+1];
		TriIndex[2] = indexdata[i+2];

		assert((TriIndex[0] < vertices) && (TriIndex[1] < vertices) && (TriIndex[2] < vertices));

		NVTangentVertex& v0 = vertexdata[TriIndex[0]];
		NVTangentVertex& v1 = vertexdata[TriIndex[1]];
		NVTangentVertex& v2 = vertexdata[TriIndex[2]];

		// x, s, t
		edge01 = float3( v1.pos.x - v0.pos.x, v1.uv.x - v0.uv.x, v1.uv.y - v0.uv.y );
		edge02 = float3( v2.pos.x - v0.pos.x, v2.uv.x - v0.uv.x, v2.uv.y - v0.uv.y );

		cp = cross( edge01, edge02 );
		if ( fabs(cp.x) > Float::MIN_VALUE )
		{
			v0.s.x += -cp.y / cp.x;
			v0.t.x += -cp.z / cp.x;

			v1.s.x += -cp.y / cp.x;
			v1.t.x += -cp.z / cp.x;
			
			v2.s.x += -cp.y / cp.x;
			v2.t.x += -cp.z / cp.x;
		}

		// y, s, t
		edge01 = float3( v1.pos.y - v0.pos.y, v1.uv.x - v0.uv.x, v1.uv.y - v0.uv.y );
		edge02 = float3( v2.pos.y - v0.pos.y, v2.uv.x - v0.uv.x, v2.uv.y - v0.uv.y );

		cp = cross( edge01, edge02 );
		if ( fabs(cp.x) > Float::MIN_VALUE )
		{
			v0.s.y += -cp.y / cp.x;
			v0.t.y += -cp.z / cp.x;

			v1.s.y += -cp.y / cp.x;
			v1.t.y += -cp.z / cp.x;
			
			v2.s.y += -cp.y / cp.x;
			v2.t.y += -cp.z / cp.x;
		}

		// z, s, t
		edge01 = float3( v1.pos.z - v0.pos.z, v1.uv.x - v0.uv.x, v1.uv.y - v0.uv.y );
		edge02 = float3( v2.pos.z - v0.pos.z, v2.uv.x - v0.uv.x, v2.uv.y - v0.uv.y );

		cp = cross( edge01, edge02 );
		if ( fabs(cp.x) > Float::MIN_VALUE )
		{
			v0.s.z += -cp.y / cp.x;
			v0.t.z += -cp.z / cp.x;

			v1.s.z += -cp.y / cp.x;
			v1.t.z += -cp.z / cp.x;
			
			v2.s.z += -cp.y / cp.x;
			v2.t.z += -cp.z / cp.x;
		}
    }

	findAndFixDegenerateVertexBasis( vertexdata, vertices, indexdata, indices, smoothnormals );

    // Calculate the sxt vector
  	for(i = 0; i < vertices; i++)
  	{		
  		// Normalize the s, t vectors
		vertexdata[i].s = normalize0(vertexdata[i].s);
		vertexdata[i].t = normalize0(vertexdata[i].t);
  
  		// Get the cross of the s and t vectors
		vertexdata[i].sxt = normalize0( cross( vertexdata[i].s, vertexdata[i].t ) );
  
  		// Need a normalized normal
		vertexdata[i].normal = normalize0( vertexdata[i].normal );
   		
  		// Get the direction of the sxt vector
		if ( dot(vertexdata[i].sxt, vertexdata[i].normal) < 0.f )
  			vertexdata[i].sxt = -vertexdata[i].sxt;
  	}
}

// MyPrimitive.cpp using MeshMender class:
	/*
	void remap( Array<float4>& vdata, const std::vector<unsigned>& mapping )
	{
		if ( vdata.size() > 0 )
		{
			Array<float4> newvdata;
			for ( unsigned i = 0 ; i < mapping.size() ; ++i )
			{
				assert( mapping[i] < (unsigned)vdata.size() && "NVMeshMender failed" );
				float4 v = vdata[mapping[i]];
				newvdata.add( v );
			}
			vdata = newvdata;
		}
	}
	
	...

	// compute vertex tangents if bump map channel used in material (using MeshMender)
	if ( mat->hasTexture(MyMaterial::PARAM_NORMMAP) )
	{
		m_vf.addTangent( VertexFormat::DF_V3_32 );

		int timebefore = System::currentTimeMillis();

		//fill up the vectors with your mesh's data
		std::vector<MeshMender::Vertex> theVerts;
		for( int i = 0; i < vpos.size() ; ++i )
		{
			MeshMender::Vertex v;
			v.pos = D3DXVECTOR3( vpos[i].x, vpos[i].y, vpos[i].z );
			v.s = vtexcoords[0][i].x;
			v.t = vtexcoords[0][i].y;
			v.normal = D3DXVECTOR3( vnormals[i].x, vnormals[i].y, vnormals[i].z );
			//meshmender will computer normals, tangents, and binormals, no need to fill those in.
			//however, if you do not have meshmender compute the normals, you _must_ pass in valid
			//normals to meshmender
			theVerts.push_back(v);
		}

		std::vector<unsigned> theIndices;
		for( int i = 0 ; i < indices.size() ; ++i )
			theIndices.push_back( indices[i] );

		// pass it in to the mender to do it's stuff
		std::vector<unsigned> mappingNewToOldVert;
		MeshMender mender;
		mender.Mend( theVerts,  theIndices, mappingNewToOldVert,
			0.f,
			0.f,
			0.f,
			1.f,
			MeshMender::DONT_CALCULATE_NORMALS,
			MeshMender::RESPECT_SPLITS,
			MeshMender::DONT_FIX_CYLINDRICAL );

		// DEBUG: info about mesh complexity increased by MeshMender
		int newindices = theIndices.size() - indices.size();
		int newvertices = theVerts.size() - vpos.size();

		//then update your mesh with the data provided in the Vertex vector
		//NOTE that MeshMender may add vertices to your mesh if needs to split
		//because of smoothing groups.  So you can't just reuse your old vertex
		//and index data. you have to update it.
		//to help associate any special per vertex data you had in your original mesh
		//you can use the mappingNewToOld ( see notes in comments below for Mend )
		indices.clear();
		for ( unsigned i = 0 ; i < theIndices.size() ; ++i )
		{
			assert( theIndices[i] >= 0 && theIndices[i] < theVerts.size() );
			indices.add( theIndices[i] );
		}
		vnormals.clear();
		vtans.clear();
		for ( unsigned i = 0 ; i < theVerts.size() ; ++i )
		{
			vtans.add( float4(theVerts[i].tangent.x, theVerts[i].tangent.y, theVerts[i].tangent.z, 0.f) );
			vnormals.add( float4(theVerts[i].normal.x, theVerts[i].normal.y, theVerts[i].normal.z, 0.f) );
		}
		remap( vpos, mappingNewToOldVert );
		remap( vcolors, mappingNewToOldVert );
		for ( int k = 0 ; k < vtexcoords.size() ; ++k )
			remap( vtexcoords[k], mappingNewToOldVert );
		remap( vweights, mappingNewToOldVert );
		remap( vbones, mappingNewToOldVert );
	}*/

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
