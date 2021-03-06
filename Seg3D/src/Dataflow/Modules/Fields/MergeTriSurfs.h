/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


//    File   : MergeTriSurfs.h
//    Author : Michael Callahan
//    Date   : Jan 2006

#if !defined(MergeTriSurfs_h)
#define MergeTriSurfs_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Geometry/CompGeom.h>
#include <Core/Basis/Constant.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <set>

//! for Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE MergeTriSurfsAlgo : public DynamicAlgoBase
{
public:
  virtual void execute(ProgressReporter *reporter,
                       FieldHandle tris,
                       vector<unsigned int> &new_nodes,
                       vector<unsigned int> &new_elems) = 0;


  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *ftri);
};


template <class FIELD>
class MergeTriSurfsAlgoT : public MergeTriSurfsAlgo
{
public:

  //! virtual interface. 
  virtual void execute(ProgressReporter *reporter,
                       FieldHandle tris,
                       vector<unsigned int> &new_nodes,
                       vector<unsigned int> &new_elems);
};

#define EPSILON 1.0e-6

template <class FIELD>
void
MergeTriSurfsAlgoT<FIELD>::execute(ProgressReporter *reporter,
                                  FieldHandle tris_h,
                                  vector<unsigned int> &new_nodes,
                                  vector<unsigned int> &new_elems)
{
  FIELD *tfield = dynamic_cast<FIELD *>(tris_h.get_rep());
  typename FIELD::mesh_handle_type tmesh = tfield->get_typed_mesh();

  tmesh->synchronize(Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E
                     | Mesh::FACES_E | Mesh::LOCATE_E);

  typename FIELD::mesh_type::Elem::iterator abi, aei;
  tmesh->begin(abi);
  tmesh->end(aei);

  typename FIELD::mesh_type::Elem::size_type prsizetmp;
  tmesh->size(prsizetmp);
  const Field::size_type prsize = prsizetmp;
  unsigned int prcounter = 0;

  vector<Point> newpoints;
  
  while (abi != aei)
  {
    reporter->update_progress(prcounter++, prsize * 2);

    typename FIELD::mesh_type::Node::array_type anodes;
    tmesh->get_nodes(anodes, *abi);
    Point apoints[3];
    BBox tribox;
    for (int i = 0; i < 3; i++)
    {
      tmesh->get_point(apoints[i], anodes[i]);
      tribox.extend(apoints[i]);
    }
    
    const Point padmin(tribox.min() - Vector(EPSILON, EPSILON, EPSILON));
    const Point padmax(tribox.max() + Vector(EPSILON, EPSILON, EPSILON));
    tribox.extend(padmin);
    tribox.extend(padmax);
    
    typedef std::set<typename FIELD::mesh_type::under_type> eset_type;
    eset_type candidates;
    tmesh->locate_bbox(candidates, tribox);

    typename eset_type::const_iterator bbi = candidates.begin();
    while (bbi != candidates.end())
    {
      if (*abi != *bbi)
      {
        typename FIELD::mesh_type::Node::array_type bnodes;
        tmesh->get_nodes(bnodes,
                         typename FIELD::mesh_type::Elem::index_type(*bbi));
        Point bpoints[3];
        for (int i = 0; i < 3; i++)
        {
          tmesh->get_point(bpoints[i], bnodes[i]);
        }

        TriTriIntersection(apoints[0], apoints[1], apoints[2],
                           bpoints[0], bpoints[1], bpoints[2],
                           newpoints);
      }
      ++bbi;
    }
    ++abi;
  }      

  prcounter = 0;
  typename FIELD::mesh_type::Node::index_type newnode;
  typename FIELD::mesh_type::Elem::array_type newelems;
  for (size_t i = 0; i < newpoints.size(); i++)
  {
    reporter->update_progress(prcounter++, newpoints.size());

    Point closest;
    vector<typename FIELD::mesh_type::Elem::index_type> elem;
    tmesh->find_closest_elems(closest, elem, newpoints[i]);
    for (size_t k = 0; k < elem.size(); k++)
    {
      newelems.clear();
      tmesh->insert_node_in_face(newelems, newnode, elem[k], closest);
      new_nodes.push_back(newnode);
      for (size_t j = newelems.size()-1; j >= 0; j--)
      {
        new_elems.push_back(newelems[j]);
      }
    }
  }

  tmesh->synchronize(Mesh::EDGES_E);

  tfield->resize_fdata();
}


} // end namespace SCIRun

#endif // MergeTriSurfs_h
