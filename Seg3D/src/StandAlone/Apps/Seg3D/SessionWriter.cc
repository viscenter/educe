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
//    File   : SessionWriter.cc
//    Author : McKay Davis
//    Date   : Tue Oct 17 21:27:22 2006

#include <StandAlone/Apps/Seg3D/SessionWriter.h>
#include <Core/XMLUtil/XMLUtil.h>
#include <Core/Containers/StringUtil.h>
#include <Core/Util/Environment.h>
#include <Core/Util/Assert.h>
#include <Core/Util/FileUtils.h>
#include <Core/Skinner/XMLIO.h>
#include <Core/Skinner/Variables.h>
#include <libxml/xmlreader.h>
#include <libxml/catalog.h>
#include <libxml/xinclude.h>
#include <libxml/xpathInternals.h>
#include <iostream>



namespace SCIRun {

bool
SessionWriter::write_session(const string &filename, Painter *painter)
{
  // TODO:  We could pack the label volume bitplanes here for smaller
  // output file size.
  // painter->pack_labels();

  pair<string, string> dir_file = split_filename(filename);
  if (!write_volumes(painter->volumes_, dir_file.first)) {
    return false;
  }

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  
  LIBXML_TEST_VERSION;
  
  /* the parser context */
  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  if (!ctxt) {
    std::cerr << "SessionWriter failed xmlNewParserCtx()\n";
    return false;
  }
      
  /* parse the file, activating the DTD validation option */
  xmlDocPtr doc = xmlNewDoc(to_xml_ch_ptr("1.0"));
  xmlNodePtr root = xmlNewNode(0,to_xml_ch_ptr("Seg3D"));
  xmlDocSetRootElement(doc, root);

  xmlNewProp(root, to_xml_ch_ptr("version"), to_xml_ch_ptr("2.0"));

  add_volume_nodes(root, painter->volumes_);
  add_appearance_nodes(root, painter);
  
  xmlSaveFormatFileEnc(filename.c_str(), doc, "UTF-8", 1);

  xmlFreeDoc(doc);

  return true;
}


static string
color_to_string(const Color &c)
{
  char temp[24];
  sprintf(temp, "#%02X%02X%02X%02X",
          (unsigned char)(c.r() * 255.0 + 0.5),
          (unsigned char)(c.g() * 255.0 + 0.5),
          (unsigned char)(c.b() * 255.0 + 0.5),
          255);
  return string(temp);
}


static string
remove_extension(const string &filename)
{
  string result;
  std::vector<string> fileext = split_string(filename, '.');
  for (int i = 0; i < (int)fileext.size()-1; i++)
  {
    if (i > 0) result = result + ".";
    result = result + fileext[i];
  }
  return result;
}


void
SessionWriter::add_appearance_nodes(xmlNodePtr node, Painter *painter)
{
  Skinner::Variables *vars = painter->get_vars();
  xmlNodePtr cnode = xmlNewChild(node, 0, to_xml_ch_ptr("appearance"), 0);
  
  add_var_node(cnode, "Painter::volume_visible", vars);
}


struct ndh_less_than
{
  bool operator()(const NrrdDataHandle &s1, const NrrdDataHandle &s2) const
  {
    return s1.get_rep() < s2.get_rep();
  }
};


bool
SessionWriter::write_volumes(NrrdVolumes &volumes, const string &dir)
{
  NrrdVolumes::iterator iter = volumes.begin();  
  NrrdVolumes::iterator end = volumes.end();
  
  std::map<NrrdDataHandle, string, ndh_less_than> written;

  for (; iter != end; ++iter) {
    static int volume_id = 0;
    NrrdVolumeHandle volume = *iter;
    
    // Painter::volumes_ no longer contains a tree.  Write each nrrd
    // out only once.
    std::map<NrrdDataHandle, string, ndh_less_than>::iterator written_loc =
      written.find(volume->nrrd_handle_);
    if (written_loc == written.end())
    {
      string basename = remove_extension(volume->filename_);
      if (basename.empty()) {
        basename = remove_extension(volume->name_);
      }

      if (basename.empty()) {
        basename = "Volume";
      }
    
      string filename = basename + ".nrrd";
      while (validFile(dir + "/" + filename))
      {
        filename = basename + to_string(volume_id++) + ".nrrd";
      }

      volume->filename_ = filename;
      if (!volume->write(dir + "/" + volume->filename_)) return false;

      written[volume->nrrd_handle_] = filename;
    }
    else
    {
      volume->filename_ = written_loc->second;
    }
  }
  return true;
}
    

void
SessionWriter::add_volume_nodes(xmlNodePtr node, NrrdVolumes &volumes)
{
  NrrdVolumes::iterator iter = volumes.begin();  
  NrrdVolumes::iterator end = volumes.end();

  for (; iter != end; ++iter) {
    NrrdVolumeHandle volume = *iter;
    xmlNodePtr cnode = xmlNewChild(node, 0, to_xml_ch_ptr("volume"),0);
    add_var_node(cnode, "name", volume->name_);
    if (volume->filename_ != "")
    {
      add_var_node(cnode, "filename", volume->filename_);
    }
    add_var_node(cnode, "label", to_string(volume->label_));
    add_var_node(cnode, "label_color",
                 color_to_string(volume->get_label_color()));
    add_var_node(cnode, "visible", to_string(volume->visible_ ? 1 : 0));
    add_var_node(cnode, "opacity", to_string(volume->opacity_));
  }
}


void
SessionWriter::add_var_node(xmlNodePtr node, 
                            const string &name,
                            const string &value)
{
  xmlNodePtr cnode = xmlNewChild(node, 0, to_xml_ch_ptr("var"), 
                                 to_xml_ch_ptr(value.c_str()));
  xmlNewProp(cnode, to_xml_ch_ptr("name"), to_xml_ch_ptr(name.c_str()));
}


void
SessionWriter::add_var_node(xmlNodePtr node, 
                            const string &name,
                            Skinner::Variables *vars)
{
  if (vars->exists(name))
  {
    add_var_node(node, name, vars->get_string(name));
  }
}


} // end namespace SCIRun
