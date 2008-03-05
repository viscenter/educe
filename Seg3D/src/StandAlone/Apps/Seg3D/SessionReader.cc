//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2006 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : SessionReader.cc
//    Author : McKay Davis
//    Date   : Tue Oct 17 15:55:03 2006

#include <StandAlone/Apps/Seg3D/SessionReader.h>
#include <Core/XMLUtil/XMLUtil.h>
#include <Core/Containers/StringUtil.h>
#include <Core/Util/Environment.h>
#include <Core/Util/Assert.h>
#include <Core/Events/LoadVolumeEvent.h>
#include <Core/Events/EventManager.h>
#include <Core/Skinner/XMLIO.h>
#include <Core/Skinner/Variables.h>
#include <libxml/xmlreader.h>
#include <libxml/catalog.h>
#include <libxml/xinclude.h>
#include <libxml/xpathInternals.h>
#include <iostream>



namespace SCIRun {

// Any value over 1<<8 will be invalid.  Just pick one that's easy to debug.
#define INVALID_LABEL_MARKER 31773

#define CURRENT_VERSION 2

SessionReader::SessionReader(Painter *painter) :
  painter_(painter),
  dir_()
{
}

SessionReader::~SessionReader()
{
}


bool
SessionReader::load_session(string filename)
{
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  filename = substituteTilde(filename);
  pair<string, string> dir_file = split_filename(filename);
  dir_ = dir_file.first;

  LIBXML_TEST_VERSION;
  
  /* the parser context */
  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  if (!ctxt) {
    std::cerr << "SessionReader failed xmlNewParserCtx()\n";
    return false;
  }
      
  /* parse the file, activating the DTD validation option */
  xmlDocPtr doc = 
    xmlCtxtReadFile(ctxt, filename.c_str(), 0, XML_PARSE_PEDANTIC);
  
  if (!doc) {
    std::cerr << "Skinner::XMLIO::load failed to parse " 
              << filename << std::endl;
    return false;
  } 

  // parse the doc at network node.
  NrrdVolumes volumes;
  for (xmlNode *cnode=doc->children; cnode!=0; cnode=cnode->next) {
    if (XMLUtil::node_is_element(cnode, "Seg3D")) {
      const char *vstr = XMLUtil::node_att_as_const_char_str(cnode, "version");
      version_ = atoi(vstr);
      if (version_ > CURRENT_VERSION)
      {
        throw "Unable to read session file with new version.";
      }
      eval_seg3d_node(cnode);
    } 
    else if (!XMLUtil::node_is_comment(cnode))
      throw "Unknown node type.";
  }               
  
  xmlFreeDoc(doc);
  xmlFreeParserCtxt(ctxt);  
#ifndef _WIN32
  xmlCleanupParser();
#endif

  return true;
}

void
SessionReader::eval_seg3d_node(const xmlNodePtr node)
{
  volumes_.clear();
  painter_->clear_all_volumes();

  for (xmlNode *cnode=node->children; cnode!=0; cnode=cnode->next) {
    if (XMLUtil::node_is_element(cnode, "volume")) {
      NrrdVolumes empty;
      eval_volume_node(cnode, empty);
    }
    if (XMLUtil::node_is_element(cnode, "appearance")) {
      eval_appearance_node(cnode);
    }
  }

  if (volumes_.size())
  {
    painter_->volumes_ = volumes_;
    painter_->current_volume_ = volumes_.back();
    painter_->extract_all_window_slices();
    painter_->rebuild_layer_buttons();

    if (first_volume_.get_rep())
    {
      painter_->current_vrender_target_ = first_volume_;
      painter_->current_vrender_target_deferred_ = true;
      event_handle_t unused;
      painter_->ShowVolumeRendering2(unused);
      
      first_volume_->get_geom_group();
      painter_->Autoview(unused);
    }

    painter_->redraw_all();
  }
}


void
SessionReader::eval_volume_node(const xmlNodePtr node, NrrdVolumes parents)
{
  Skinner::Variables *vars = new Skinner::Variables("");
  for (xmlNode *cnode=node->children; cnode!=0; cnode=cnode->next) {
    if (XMLUtil::node_is_element(cnode, "var")) {
      Skinner::XMLIO::eval_var_node(cnode, vars);
    } 
  }
  NrrdVolumeHandle volume = 0;

  unsigned int label = 0;
  if (vars->exists("label")) {
    label = vars->get_int("label");
  }

  int label_index = 0;
  if (label > 0xffffff) { label_index = 3; label = label >> 24; }
  else if (label > 0xffff) { label_index = 2; label = label >> 16; }
  else if (label > 0xff) { label_index = 1; label = label >> 8; }

  if (!parents.size()) {
    string filename = vars->get_string("filename");  
    pair<string, string> dir_file = split_filename(filename);
    if (dir_file.first.empty()) { dir_file.first = dir_; }
    if (!label)
    {
      volume = painter_->load_volume<float>(dir_file.first+filename);
      if (!volume.get_rep())
      {
        throw "Error reading auxiliary data volume " + filename + ".";
      }
      if (!first_volume_.get_rep())
      {
        // TODO:  Emulate the bad behavior of Seg3D and volume selection
        // here.  There is not currently a way to select which volume
        // is volume-rendered so we just pick an arbitrary one (the first).
        // Change this to a variable when we can select which are visible.
        first_volume_ = volume;
        first_data_file_ = dir_file.first + filename;
      }
    }
    else if (version_ == 1)
    {
      NrrdVolumeHandle tmp =
        painter_->load_volume<unsigned int>(dir_file.first+filename);
      if (!tmp.get_rep())
      {
        throw "Error reading auxiliary label volume " + filename + ".";
      }
      vector<NrrdDataHandle> nrrds;
      VolumeOps::dice_32_into_8(tmp->nrrd_handle_, nrrds);
      for (size_t i = 0; i < nrrds.size(); i++)
      {
        NrrdVolumeHandle vol =
          new NrrdVolume(painter_, "", nrrds[i], INVALID_LABEL_MARKER);
        parents.push_back(vol);
      }
      volume = parents[label_index];
    }
    else if (version_ == 2)
    {
      bool found = false;
      for (size_t i = 0; i < volumes_.size(); i++)
      {
        if (volumes_[i]->filename_ == filename)
        {
          volume = volumes_[i]->create_child_label_volume(label);
          found = true;
          break;
        }
      }
      if (!found)
      {
        volume = painter_->load_volume<unsigned char>(dir_file.first+filename);
        if (!volume.get_rep())
        {
          throw "Error reading auxiliary label volume " + filename + ".";
        }
      }
    }
      
    volume->filename_ = filename;
  }
  else
  {
    if (parents[label_index]->label_ == INVALID_LABEL_MARKER)
    {
      volume = parents[label_index];
    }
    else
    {
      volume = parents[label_index]->create_child_label_volume(label);
    }
  }

  volume->set_label(label);

  if (label)
  {
    if (vars->exists("label_color"))
    {
      Skinner::Color sc = vars->get_color("label_color");
      Color c(sc.r, sc.g, sc.b);
      volume->set_label_color(c);
    }
    else
    {
      unsigned int colormap_index = 0;
      if (vars->exists("colormap_index"))
      {
        colormap_index = vars->get_int("colormap_index");
      }
      volume->set_label_color_legacy(colormap_index);
    }
  }

  if (vars->exists("name")) {
    volume->name_ = vars->get_string("name");
  }

  if (vars->exists("opacity")) {
    volume->opacity_ = vars->get_double("opacity");
  }

  volumes_.push_back(volume);

  for (xmlNode *cnode=node->children; cnode!=0; cnode=cnode->next) {
    if (XMLUtil::node_is_element(cnode, "volume")) {
      eval_volume_node(cnode, parents);
    } 
  }
}


void
SessionReader::eval_appearance_node(const xmlNodePtr node)
{
  // TODO:  Might have to handle non-variable data differently,
  // for instance slice position?
  Skinner::Variables *vars = painter_->get_vars();
  for (xmlNode *cnode = node->children; cnode != 0; cnode = cnode->next) {
    if (XMLUtil::node_is_element(cnode, "var")) {
      Skinner::XMLIO::eval_var_node(cnode, vars);
    }
  }
}



} // end namespace SCIRun
