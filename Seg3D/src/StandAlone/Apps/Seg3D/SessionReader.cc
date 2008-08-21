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
#include <Core/Events/EventManager.h>
#include <Core/Skinner/XMLIO.h>
#include <Core/Skinner/Variables.h>
#include <libxml/xmlreader.h>
#include <libxml/catalog.h>
#include <libxml/xinclude.h>
#include <libxml/xpathInternals.h>
#include <iostream>

#include <wx/wfstream.h>
#include <wx/tarstrm.h>
#include <wx/zstream.h>
#include <StandAlone/Apps/Seg3D/Seg3DwxGuiUtils.h>

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
  bool result = false;

  try {
    result = load_session_tgz(filename);
  }
  catch (...)
  {
    result = load_session_old(filename);
  }

  return result;
}


static void
patchup_nrrd_spacings(Nrrd *nrrd)
{
  for (size_t i = 0; i < nrrd->dim; i++)
  {
    nrrd->axis[i].spacing = nrrd->axis[i].spaceDirection[i];
    nrrd->axis[i].min = nrrd->spaceOrigin[i];
    nrrd->axis[i].max = nrrd->axis[i].min +
      (nrrd->axis[i].size-1) * nrrd->axis[i].spacing;
  }
  nrrd->spaceDim = 0;
}


bool
SessionReader::load_session_tgz(string filename)
{
  // Creating this suppresses error message popups, such as that we
  // are trying to open and uncompressed file.
  // TODO: This turns off all errors.  Really only the compressed file
  // check should be turned off.
  wxLogNull suppress_error_popups;

  filename = substituteTilde(filename);

  // TODO: Temporary, don't need once nrrd readers are in.
  pair<string, string> dir_file = split_filename(filename);
  dir_ = dir_file.first;

  wxTarEntry *entry;

  wxFFileInputStream in(std2wx(filename));
  wxZlibInputStream gzin(in);
  wxTarInputStream tar(gzin);

  char *xmlbuffer = NULL;
  size_t xmlsize = 0;
  while ((entry = tar.GetNextEntry()) != NULL)
  {
    string name = wx2std(entry->GetName());
    size_t size = entry->GetSize();

    if (name == "session.xml")
    {
      xmlsize = size;
      xmlbuffer = new char[xmlsize];
      tar.Read(xmlbuffer, xmlsize);
    }
    else
    {
      const size_t maxhbufsize = 1<<18; // 256k is enough for anyone?
      char *hbuffer = new char[maxhbufsize];

      size_t hsize = 0;
      while (hsize < size && hsize < maxhbufsize-1 && tar.LastRead())
      {
        hbuffer[hsize++] = tar.GetC();

        // Check for both Unix and DOS linebreak formats in the nrrd header.
        if (hsize >= 2 &&
            hbuffer[hsize-1] == '\n' && hbuffer[hsize-2] == '\n' ||
            hsize >= 4 &&
            hbuffer[hsize-2] == '\r' && hbuffer[hsize-1] == '\n' &&
            hbuffer[hsize-4] == '\r' && hbuffer[hsize-3] == '\n')
        {
          break;
        }
      }        
      hbuffer[hsize] = '\0';

      NrrdDataHandle nrrdh = new NrrdData();
      NrrdIoState *nio = nrrdIoStateNew();
      nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);
      if (nrrdStringRead(nrrdh->nrrd_, hbuffer, nio))
      {
        char *err = biffGetDone(NRRD);
        throw err;
      }
      delete [] hbuffer;

      // TODO:  Verify uchar and float formats.
      // TODO:  Get endianness correct.
      // TODO:  Verify that the format was RAW.
      const size_t dsize = size - hsize;
      ASSERT(dsize == VolumeOps::nrrd_data_size(nrrdh));
      char *dbuffer = new char[dsize];
      nrrdh->nrrd_->data = dbuffer;

      // TODO: We're using the nrrd spacings fields poorly.  We move
      // from nice orientation here back to min/max and spacings
      // fields.
      patchup_nrrd_spacings(nrrdh->nrrd_);
      
      // Read in 1G blocks to work around 4G filesystem read size limits.
      size_t totalrsize = dsize;
      char *rbuffer = dbuffer;
      while (totalrsize)
      {
        size_t rsize = 1<<30;
        if (rsize > totalrsize) rsize = totalrsize;
        tar.Read(rbuffer, rsize);
        totalrsize -= rsize;
        rbuffer += rsize;
      }

      //cout << "endianness " << nio->endian << " " << AIR_ENDIAN << "\n";

      nrrdIoStateNix(nio);

      tgz_nrrds_[name] = nrrdh;
    }
  }

  if (xmlbuffer == NULL)
  {
    throw "Unable to find session.xml in this session archive.";
  }

  const bool result = parse_session_xml(xmlbuffer, xmlsize);
  delete xmlbuffer;

  return result;
}


bool
SessionReader::parse_session_xml(const char *xmlbuffer, size_t xmlsize)
{
  /*
   * This initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION;

  // The parser context
  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  if (!ctxt) {
    std::cerr << "SessionReader failed xmlNewParserCtx()\n";
    return false;
  }
      
  // Parse the file, activating the DTD validation option.
  xmlDocPtr doc = 
    xmlCtxtReadMemory(ctxt, xmlbuffer, xmlsize, "session.xml",
                      0, XML_PARSE_PEDANTIC);
  
  if (!doc) {
    std::cerr << "Skinner::XMLIO::load failed to parse session.xml" << std::endl;
    return false;
  } 

  // Parse the doc at network node.
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


bool
SessionReader::load_session_old(string filename)
{
  /*
   * This initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  filename = substituteTilde(filename);
  pair<string, string> dir_file = split_filename(filename);
  dir_ = dir_file.first;

  LIBXML_TEST_VERSION;

  // The parser context
  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  if (!ctxt) {
    std::cerr << "SessionReader failed xmlNewParserCtx()\n";
    return false;
  }
      
  // Parse the file, activating the DTD validation option.
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
  }

  if (volumes_.size())
  {
    painter_->volumes_ = volumes_;
    painter_->current_volume_ = volumes_.back();
  }

  for (xmlNode *cnode=node->children; cnode!=0; cnode=cnode->next) {
    if (XMLUtil::node_is_element(cnode, "appearance")) {
      eval_appearance_node(cnode);
    }
  }

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

    tgz_map_type::iterator loc = tgz_nrrds_.find(filename);
    if (loc != tgz_nrrds_.end())
    {
      NrrdDataHandle nrrdh = (*loc).second;
      if (!label)
      {
        volume = new NrrdVolume(painter_, "", nrrdh, 0);
        if (!first_volume_.get_rep())
        {
          // TODO:  Emulate the bad behavior of Seg3D and volume selection
          // here.  There is not currently a way to select which volume
          // is volume-rendered so we just pick an arbitrary one (the first).
          // Change this to a variable when we can select which are visible.
          first_volume_ = volume;
        }
      }
      else
      {
        bool found = false;
        for (size_t i = 0; i < volumes_.size(); i++)
        {
          if (volumes_[i]->nrrd_handle_.get_rep() == nrrdh.get_rep())
          {
            volume = volumes_[i]->create_child_label_volume(label);
            found = true;
            break;
          }
        }
        if (!found)
        {
          volume = new NrrdVolume(painter_, "", nrrdh, label);
        }
      }
    }
    else
    {
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
    }

    if (!volume.get_rep()) { throw "Bad session volume pointer."; }

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
  
  if (vars->exists("visible")) {
    volume->tmp_visible_ = vars->get_bool("visible");
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

  delete vars;
}


void
SessionReader::eval_appearance_node(const xmlNodePtr node)
{
  Skinner::Variables *vars = new Skinner::Variables("");
  for (xmlNode *cnode = node->children; cnode != 0; cnode = cnode->next) {
    if (XMLUtil::node_is_element(cnode, "var")) {
      Skinner::XMLIO::eval_var_node(cnode, vars);
    }
  }
  
  painter_->get_session_appearance_vars(vars);
  delete vars;
}



} // end namespace SCIRun
