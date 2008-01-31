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


/*
 *  GenerateLinearSegments.h:
 *
 *  Written by:
 *   Allen Sanderson
 *   School of Computering
 *   University of Utah
 *   May 2002
 *
 *  Copyright (C) 2002 SCI Group
 */

#if !defined(GenerateLinearSegments_h)
#define GenerateLinearSegments_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>

//! for Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE GenerateLinearSegmentsAlgo : public DynamicAlgoBase
{
public:
  virtual FieldHandle execute( ProgressReporter *reporter,
			       FieldHandle& field_input_handle,
			       double step_size,
			       int max_steps,
			       int direction,
			       int value) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *fsrc,
					    const TypeDescription *sloc,
					    const string &fdst,
					    int value);

};


template< class IFIELD, class OFIELD >
class GenerateLinearSegmentsAlgoT : public GenerateLinearSegmentsAlgo
{
public:
  //! virtual interface. 
  virtual FieldHandle execute( ProgressReporter *reporter,
			       FieldHandle& field_input_handle,
			       double step_size,
			       int max_steps,
			       int direction,
			       int value);

  void FindNodes(vector<Point> &v,
		 Point seed,
		 Vector vec,
		 double step_size,
		 int max_steps )
  {
    for (int i=1; i < max_steps; i++)
    {
      v.push_back(seed+(double)i*step_size*vec);
    }
  }

  virtual void set_result_value(OFIELD *ofield,
                                typename OFIELD::mesh_type::Node::index_type di,
                                IFIELD *ifield,
                                typename IFIELD::mesh_type::Node::index_type si,
                                double data) = 0;
};


template< class IFIELD, class OFIELD >
FieldHandle
GenerateLinearSegmentsAlgoT<IFIELD, OFIELD>::execute( ProgressReporter *reporter,
						      FieldHandle& field_input_handle,
						      double step_size,
						      int max_steps,
						      int direction,
						      int value)
{
  IFIELD *ifield = (IFIELD *) field_input_handle.get_rep();
  typename IFIELD::mesh_handle_type imesh = ifield->get_typed_mesh();

  typename OFIELD::mesh_handle_type omesh = scinew typename OFIELD::mesh_type();
  OFIELD *ofield = scinew OFIELD(omesh);

  Point seed;
  Vector vec;
  vector<Point> nodes;
  nodes.reserve(direction==1?2*max_steps:max_steps);

  vector<Point>::iterator node_iter;
  typename OFIELD::mesh_type::Node::index_type n1, n2;

  // Try to find the linear segment for each seed point.

  typename IFIELD::mesh_type::Node::iterator iter;
  typename IFIELD::mesh_type::Node::iterator iter_end;

  imesh->begin(iter);
  imesh->end(iter_end);

  typename IFIELD::size_type prsize_tmp = ifield->size_fdata();

  const unsigned int prsize = (unsigned int) prsize_tmp;

  int count = 0;

  while (iter != iter_end)
  {
    imesh->get_point(seed, *iter);
    ifield->value(vec, *iter);

    reporter->update_progress(count, prsize);

    nodes.clear();
    nodes.push_back(seed);

    int cc = 0;

    // Find the negative segments.
    if( direction <= 1 )
    {
      FindNodes(nodes, seed, -vec, step_size, max_steps);

      if ( direction == 1 )
      {
	std::reverse(nodes.begin(), nodes.end());
	cc = nodes.size();
	cc = -(cc - 1);
      }
    }
    // Append the positive segments.
    if( direction >= 1 )
    {
      FindNodes(nodes, seed, vec, step_size, max_steps);
    }

    node_iter = nodes.begin();

    if (node_iter != nodes.end())
    {
      n1 = ofield->get_typed_mesh()->add_node(*node_iter);

      ostringstream str;
      str << "Segment " << count << " Node Index";      
      ofield->set_property( str.str(), n1, false );

      ofield->resize_fdata();

      if (value == 0)
        set_result_value(ofield, n1, ifield, *iter, 0);
      else if( value == 1)
        set_result_value(ofield, n1, ifield, *iter, (double)*iter);
      else if (value == 2)
        set_result_value(ofield, n1, ifield, *iter, (double)abs(cc));

      ++node_iter;

      cc++;

      while (node_iter != nodes.end())
      {
	n2 = ofield->get_typed_mesh()->add_node(*node_iter);
	ofield->resize_fdata();
        
        if (value == 0)
          set_result_value(ofield, n2, ifield, *iter, 0);
        else if( value == 1)
          set_result_value(ofield, n2, ifield, *iter, (double)*iter);
        else if (value == 2)
          set_result_value(ofield, n2, ifield, *iter, (double)abs(cc));

	ofield->get_typed_mesh()->add_edge(n1, n2);

	n1 = n2;
	++node_iter;

	cc++;
      }
    }

    ++iter;
    ++count;
  }

  ofield->set_property( "Segment Count", count, false );

  ofield->freeze();

  return FieldHandle(ofield);
}

template <class IFIELD, class OFIELD>
class GenerateLinearSegmentsAlgoTM : public GenerateLinearSegmentsAlgoT<IFIELD, OFIELD>
{
public:

  virtual void set_result_value(OFIELD *ofield,
                                typename OFIELD::mesh_type::Node::index_type di,
                                IFIELD *ifield,
                                typename IFIELD::mesh_type::Node::index_type si,
                                double data)
  {
    ofield->set_value(data, di);
  }
};


template <class IFIELD, class OFIELD>
class GenerateLinearSegmentsAlgoTF : public GenerateLinearSegmentsAlgoT<IFIELD, OFIELD>
{
public:

  virtual void set_result_value(OFIELD *ofield,
                                typename OFIELD::mesh_type::Node::index_type di,
                                IFIELD *ifield,
                                typename IFIELD::mesh_type::Node::index_type si,
                                double data)
  {
    typename OFIELD::value_type val;
    ifield->value(val, si);
    ofield->set_value(val, di);
  }
};


} // end namespace SCIRun

#endif // GenerateLinearSegments_h
