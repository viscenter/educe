//#include <Dataflow/Network/PackageDB.h>
//#include <Dataflow/Network/ComponentNode.h>
#include <Core/Util/Dir.h> // for LSTAT
#include <Core/Util/FileUtils.h>
#include <Core/Util/Environment.h>
#include <Core/Util/Assert.h>
#include <Core/XMLUtil/XMLUtil.h>
#include <libxml/xmlreader.h>

#include <iostream>
#include <string>
#include <map>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sstream>
#include <fstream>

using namespace SCIRun;
using namespace std;

typedef struct {
  string name;
  string datatype;
  //  iport_maker maker;
} IPortInfo;

typedef struct {
  string name;
  string datatype;
  //  oport_maker maker;
} OPortInfo;

struct ModuleInfo
{
  string                       package_name_;
  string                       category_name_;
  string                       module_name_;
  bool                         optional_;
  bool                         hide_;
  vector<string>               authors_;
  string                       summary_;
  //  ModuleMaker                  maker_;
  std::vector<IPortInfo*>      iports_;
  std::vector<OPortInfo*>      oports_;
  bool                         last_port_dynamic_;
  bool                         has_gui_node_;
};

typedef struct {
  string name;
  std::map<string,ModuleInfo*> modules;
} category;

typedef struct {
  string name;
  std::map<string,category*> categories;
} package;

typedef std::map<int,char*>::iterator char_iter;
//  typedef std::map<int,inport_node*>::iterator inport_iter;
//  typedef std::map<int,outport_node*>::iterator outport_iter;
typedef std::map<string,int>::iterator string_iter;
typedef std::map<string,category*>::iterator category_iter;
typedef std::map<string,ModuleInfo*>::iterator module_iter;
typedef std::map<int,package*>::iterator package_iter;

////////////////////////////////////////////////////////////////////

template <class PInfo>
void
set_port_info(vector<PInfo*> &ports, xmlNodePtr snode)
{
  xmlNodePtr ipnode = snode->children;
  for (; ipnode != 0; ipnode = ipnode->next) {
    if (string(to_char_ptr(ipnode->name)) == string("port"))
      {
        PInfo *pinfo = new PInfo;
        xmlNodePtr pnode = ipnode->children;
        for (; pnode != 0; pnode = pnode->next) {
          if (string(to_char_ptr(pnode->name)) == string("name")) {
            pinfo->name = string(to_char_ptr(pnode->children->content));
          }
          if (string(to_char_ptr(pnode->name)) == string("datatype"))
            {
              pinfo->datatype = string(to_char_ptr(pnode->children->content));
            }
        }
        ports.push_back(pinfo);
      }
  }
}


bool
set_port_info(ModuleInfo &mi, const xmlNodePtr cnode)
{
  xmlNodePtr node = cnode->children;
  for (; node != 0; node = node->next) {
    if (string(to_char_ptr(node->name)) == string("io")) {
      xmlNodePtr ionode = node->children;
      for (; ionode != 0; ionode = ionode->next) {
        if (ionode && (string(to_char_ptr(ionode->name)) ==
                       string("inputs")))
          {
            xmlAttrPtr lpd_att = get_attribute_by_name(ionode,
                                                       "lastportdynamic");
            if (lpd_att) {
              mi.last_port_dynamic_ = false;
              if (string(to_char_ptr(lpd_att->children->content)) ==
                  string("yes"))
                {
                  mi.last_port_dynamic_ = true;
                }
            } else {
              std::cerr << "Missing attribute lastportdynamic for module: "
                        << mi.module_name_ << std::endl;
              return false;
            }
            // set input port info.
            set_port_info(mi.iports_, ionode);
          }
        else if (ionode && (string(to_char_ptr(ionode->name)) ==
                            string("outputs")))
          {
            // set input port info.
            set_port_info(mi.oports_, ionode);
          }
      }
    }
  }
  return false;
}


bool
set_description(ModuleInfo &mi, const xmlNodePtr cnode)
{
  xmlNodePtr onode = cnode->children;
  for (; onode != 0; onode = onode->next) {
    if (string(to_char_ptr(onode->name)) == string("overview")) {

      xmlNodePtr node = onode->children;
      for (; node != 0; node = node->next) {
        if (string(to_char_ptr(node->name)) == string("summary"))
          {
            mi.summary_ = get_serialized_children(node);
          }
        else if (string(to_char_ptr(node->name)) == string("authors"))
          {
            xmlNodePtr anode = node->children;
            for (; anode != 0; anode = anode->next) {
              if (string(to_char_ptr(anode->name)) == string("author"))
                {
                  string s = string(to_char_ptr(anode->children->content));
                  mi.authors_.push_back(s);
                }
            }
          }
      }
    }
  }
  return false;
}


bool
set_gui_info(ModuleInfo &mi, const xmlNodePtr cnode)
{
  mi.has_gui_node_ = false;
  xmlNodePtr onode = cnode->children;
  for (; onode != 0; onode = onode->next) {
    if (string(to_char_ptr(onode->name)) == string("gui")) {
      mi.has_gui_node_ = true;
      return true;
    }
  }
  return false;
}

bool
read_component_file(ModuleInfo &mi, const char* filename)
{
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION;

  xmlParserCtxtPtr ctxt; /* the parser context */
  xmlDocPtr doc; /* the resulting document tree */

  /* create a parser context */
  ctxt = xmlNewParserCtxt();
  if (ctxt == 0) {
    std::cerr << "ComponentNode.cc: Failed to allocate parser context\n";
    return false;
  }
  /* parse the file, activating the DTD validation option */
  doc = xmlCtxtReadFile(ctxt, filename, 0, (XML_PARSE_DTDATTR |
                                            XML_PARSE_DTDVALID |
                                            XML_PARSE_NOERROR));
  /* check if parsing suceeded */
  if (doc == 0 || ctxt->valid == 0) {

    xmlError* error = xmlCtxtGetLastError(ctxt);
    //    GuiInterface *gui = GuiInterface::getSingleton();
    string mtype = "Parse ";
    if (doc) {
      mtype = "Validation ";
    }
    ostringstream msg;
    msg << "createSciDialog -error -title \"Component XML "
        << mtype << "Error\" -message \""
        << endl << mtype << "Failure for: " << endl << filename << endl
        << endl << "Error Message: " << endl << error->message << endl << "\"";
    //     gui->eval(msg.str());
    return false;
  }

  xmlNode* node = doc->children;
  for (; node != 0; node = node->next) {
    // skip all but the component node.
    if (node->type == XML_ELEMENT_NODE &&
        string(to_char_ptr(node->name)) == string("component"))
      {
        //! set all the ModuleInfo
        xmlAttrPtr name_att = get_attribute_by_name(node, "name");
        xmlAttrPtr cat_att = get_attribute_by_name(node, "category");

        // set the component attributes.
        if (name_att == 0 || cat_att == 0) {
          std::cerr << "Attibutes missing from component node in : "
                    << filename << std::endl;
          return false;
        }
        mi.module_name_ = string(to_char_ptr(name_att->children->content));
        mi.category_name_ = string(to_char_ptr(cat_att->children->content));
        xmlAttrPtr opt_att = get_attribute_by_name(node, "optional");
        mi.optional_ = false;
        if (opt_att != 0)
          {
            if (string(to_char_ptr(opt_att->children->content)) == string("true"))
              {
                mi.optional_ = true;
              }
          }

        if (!sci_getenv_p("SCIRUN_SHOW_HIDDEN_MODULES"))
          {
            opt_att = get_attribute_by_name(node, "hide");
            mi.hide_ = false;
            if (opt_att != 0)
              {
                if (string(to_char_ptr(opt_att->children->content)) ==
                    string("true"))
                  {
                    mi.hide_ = true;
                  }
              }
          }

        //set the description string.
        set_description(mi, node);
        set_port_info(mi, node);
        set_gui_info(mi, node);
      }
  }

  xmlFreeDoc(doc);
  /* free up the parser context */
  xmlFreeParserCtxt(ctxt);
#ifndef _WIN32
  // there is a problem on windows when using Uintah
  // which is either caused or exploited by this
  xmlCleanupParser();
#endif
  return true;
}

////////////////////////////////////////////////////////////////////
void printMessage(const string& str) {
  cout << str;
}

void gui_exec(const string& str) {
  cout << "gui_exec: "<<str<<"\n";
}

bool
findMaker(ModuleInfo* moduleInfo)
{
  return false;
}

void
registerModule(ModuleInfo* info)
{

}

void
makeModuleExtern(ostream& output, const string& function_name)
{
  output << "extern \"C\" Module* "<<function_name<<"(GuiContext* ctx);\n";
}

void
makeFindReturn(ostream& output, const string& name_string,
               const string& function_name)
{
  output << "if (("<<name_string<<") == \""<<function_name<<"\") return "<<function_name<<";\n";
}

void
makeIPortExtern(ostream& output, const string& function_name)
{
  output << "extern \"C\" IPort* "<<function_name<<"(Module* module, const string& name);\n";
}

void
makeOPortExtern(ostream& output, const string& function_name)
{
  output << "extern \"C\" OPort* "<<function_name<<"(Module* module, const string& name);\n";
}

void
makeFindPorts(ostream& output, const string& name_string,
              const string& function_name)
{
  output << "if (("<<name_string<<") == \""<<function_name<<"\") return reinterpret_cast<void*>("<<function_name<<");\n";
}

void
loadPackage(ostream& output)
{
  string loadPackage;
  string result;
  std::map<int,package*> packages;
  package* new_package = 0;
  category* new_category = 0;
  ModuleInfo* new_module = 0;
  module_iter mi;
  category_iter ci;
  package_iter pi;
  string packageElt;
  int mod_count = 0;
  string notset("notset");
  string packagePath;

  printMessage("Loading packages, please wait...");

  // the format of PACKAGE_PATH is a colon seperated list of paths to the
  // root(s) of package source trees.
  const char *srcdir = sci_getenv("SCIRUN_SRCDIR");
  ASSERT(srcdir);
  packagePath = srcdir + string("/Packages");

  // if the user specififes it, build the complete package path
  const char *packpath = sci_getenv("SCIRUN_PACKAGE_SRC_PATH");
  if (packpath) packagePath = string(packpath) + ":" + packagePath;

  // the format of LOAD_PACKAGE is a comma seperated list of package names.
  // build the complete list of packages to load
  ASSERT(sci_getenv("SCIRUN_LOAD_PACKAGE"));
  loadPackage = string(sci_getenv("SCIRUN_LOAD_PACKAGE"));
  cout << "loadPackage = ["<<loadPackage<<"]\n";

  while(loadPackage!="") {
    // Strip off the first element, leave the rest for the next
    // iteration.
    const unsigned int firstComma = loadPackage.find(',');
    if(firstComma < loadPackage.size()) {
      packageElt=loadPackage.substr(0,firstComma);
      loadPackage=loadPackage.substr(firstComma+1);
    } else {
      packageElt=loadPackage;
      loadPackage="";
    }

    string tmpPath = packagePath;
    string pathElt;

    for (;tmpPath!="";) {
      if (packageElt=="SCIRun") {
        tmpPath = "found";
        break;
      }
#ifdef _WIN32
      // don't find the drive letter name's ':'...
      const unsigned int firstColon = tmpPath.find(':',2);
#else
      const unsigned int firstColon = tmpPath.find(':');
#endif
      if(firstColon < tmpPath.size()) {
        pathElt=tmpPath.substr(0,firstColon);
        tmpPath=tmpPath.substr(firstColon+1);
      } else {
        pathElt=tmpPath;
        tmpPath="";
      }

      struct stat buf;
      LSTAT((pathElt+"/"+packageElt).c_str(),&buf);
      if (S_ISDIR(buf.st_mode)) {
        tmpPath = "found";
        break;
      }
    }

    cout << "tmpPath = "<<tmpPath<<"\n";
    cout << "packageElt = "<<packageElt<<"\n";
    if (tmpPath=="") {
      printMessage("Unable to load package " + packageElt +
                   ":\n - Can't find " + packageElt +
                   " directory in package path");
      continue;
    }

    cout << "Finding XML files\n";
    gui_exec("lappend auto_path "+pathElt+"/"+packageElt+"/Dataflow/GUI");

    string xmldir;

    if(packageElt == "SCIRun") {
      xmldir = string(srcdir) + "/Dataflow/XML";
      gui_exec("lappend auto_path "+string(srcdir)+"/Dataflow/GUI");
    } else {
      xmldir = pathElt+"/"+packageElt+"/Dataflow/XML";
      gui_exec(string("lappend auto_path ")+pathElt+"/"+packageElt+
               "/Dataflow/GUI");
    }
    std::map<int,char*>* files;
    files = GetFilenamesEndingWith((char*)xmldir.c_str(),".xml");

    // WARNING... looks like the 'files' variable is memory leaked...
    // both in the failure case directly below and in the success case.

    if (!files || files->size() == 0) {
      printMessage("Unable to load package " + packageElt +
                   ":\n - Couldn't find *.xml in " + xmldir );
      continue;
    }

    new_package = new package;
    new_package->name = packageElt;
    packages.insert(std::pair<int, package*>(packages.size(), new_package));

    cout << "Changing number of modules from "<<mod_count;
    mod_count += files->size();
    cout << " to "<<mod_count<<"\n";

    for (char_iter i=files->begin(); i != files->end();  i++)
      {
        //cout << "Reading "<<(*i).second<<"\n";
        new_module = new ModuleInfo;
        new_module->package_name_ = packageElt;
        //        new_module->maker_ = 0;
        if (! read_component_file(*new_module,
                                  (xmldir+"/"+(*i).second).c_str()))
          {
            printMessage("Unable to read or validate " +
                         xmldir+"/"+(*i).second + "\n  Module not loaded.\n");
            continue;
          }

        string cat_name = new_module->category_name_;
        if((cat_name.substr(0, 13) == "Conglomerate_"))
          { cat_name.erase(0,13); }

        ci = new_package->categories.find(cat_name);
        if (ci==new_package->categories.end())
          {
            new_category = new category;
            new_category->name = cat_name;
            new_package->categories.insert(std::pair<string,
                                           category*>(cat_name,
                                                      new_category));
            ci = new_package->categories.find(string(new_category->name));
          }

        mi = (*ci).second->modules.find(new_module->module_name_);
        if (mi==(*ci).second->modules.end())
          {
            (*ci).second->modules.insert(std::pair<string,
                                         ModuleInfo*>(new_module->module_name_,
                                                      new_module));
          }
      }
  } // end while(loadPackage!="")

  // Update the progress bar to know how many (more) operations will
  // be taking place so it can accurately display the loading
  // progress.
  ostringstream mod_count_str;
  mod_count_str << mod_count;
  gui_exec("addProgressSteps " + mod_count_str.str());

  //  int index = 0;
  int numreg;

  // Key is the port name (i.e. make_BundleIPort), value is the package name.
  typedef map<string, string> MakerMap;
  typedef MakerMap::iterator MakerMapIter;
  MakerMap module_names;
  MakerMap input_port_names;
  MakerMap output_port_names;

  for (pi = packages.begin();
       pi!=packages.end();
       pi++) {

    numreg = 0;

    string pname = (*pi).second->name;
    cout << "pname = "<<pname<<"\n";

    printMessage("Loading package '" + pname + "'");
    gui_exec("setProgressText {Loading package: " + pname + " }");

    for (ci = (*pi).second->categories.begin();
         ci!=(*pi).second->categories.end();
         ci++) {
      for (mi = (*ci).second->modules.begin();
           mi!=(*ci).second->modules.end();
           mi++) {
        ModuleInfo* moduleInfo = (*mi).second;

        string makename = "make_" + moduleInfo->module_name_;
//         cout << "makename = "<<makename<<"\n";
        // See if the maker is already in the map
        MakerMapIter module_maker = module_names.find(makename);
        if (module_maker == module_names.end()) {
          module_names[makename] = pname;
        } else {
          // Already in there, report an error
          cerr << "module("<<pname<<"::"<<makename<<") conflicts with already found module("<<(*module_maker).second<<"::"<<makename<<")\n";
        }
        {
          vector<string> packageNames; // "make_" + datatype + "OPort";
          vector<string> portNames; // "make_" + datatype + "OPort";

          vector<IPortInfo*>::iterator i2 = moduleInfo->iports_.begin();

          while (i2 < moduleInfo->iports_.end())
            {
              IPortInfo* ip = *i2++;
              unsigned long strlength = ip->datatype.length();
              char* package = new char[strlength+1];
              char* datatype = new char[strlength+1];
              sscanf(ip->datatype.c_str(),"%[^:]::%s",package,datatype);
              if (package[0]=='*') {
                packageNames.push_back(string(&package[1]));
              } else {
                packageNames.push_back(string(package));
              }
              portNames.push_back(string("make_") + datatype + "IPort");
            } // end while

//           for(size_t i = 0; i < portNames.size(); ++i) {
//             cout << "\tport["<<i<<"] = "<<packageNames[i]<<": "<<portNames[i]<<"\n";
//           }
          for(size_t i = 0; i < portNames.size(); ++i) {
            string port_name = portNames[i];
            MakerMapIter port_iter = input_port_names.find(port_name);
            if (port_iter == input_port_names.end()) {
              // Insert it
              input_port_names[port_name] = packageNames[i];
            } else {
              // Check to see if it's the same package
              if ((*port_iter).second != packageNames[i]) {
                // Conflict.  same port name, but different package
                cerr << "port("<<packageNames[i]<<"::"<<port_name<<") conflicts with already found port("<<(*port_iter).second<<"::"<<port_name<<")\n";
              }
            }
          }
        } // end input port scope
        {
          vector<string> packageNames; // "make_" + datatype + "OPort";
          vector<string> portNames; // "make_" + datatype + "OPort";

          vector<OPortInfo*>::iterator i2 = moduleInfo->oports_.begin();

          while (i2 < moduleInfo->oports_.end())
            {
              OPortInfo* op = *i2++;
              unsigned long strlength = op->datatype.length();
              char* package = new char[strlength+1];
              char* datatype = new char[strlength+1];
              sscanf(op->datatype.c_str(),"%[^:]::%s",package,datatype);
              if (package[0]=='*') {
                packageNames.push_back(string(&package[1]));
              } else {
                packageNames.push_back(string(package));
              }
              portNames.push_back(string("make_") + datatype + "OPort");
            } // end while

//           for(size_t i = 0; i < portNames.size(); ++i) {
//             cout << "\tport["<<i<<"] = "<<packageNames[i]<<": "<<portNames[i]<<"\n";
//           }
          for(size_t i = 0; i < portNames.size(); ++i) {
            string port_name = portNames[i];
            MakerMapIter port_iter = output_port_names.find(port_name);
            if (port_iter == output_port_names.end()) {
              // Insert it
              output_port_names[port_name] = packageNames[i];
            } else {
              // Check to see if it's the same package
              if ((*port_iter).second != packageNames[i]) {
                // Conflict.  same port name, but different package
                cerr << "port("<<packageNames[i]<<"::"<<port_name<<") conflicts with already found port("<<(*port_iter).second<<"::"<<port_name<<")\n";
              }
            }
          }
        } // end output port scope
      }
    }

    // Done with this package

  } // package iterator

  ostringstream externs;
  ostringstream modules;
  ostringstream ports;

  bool first = true;
  for(MakerMapIter iter = module_names.begin();
      iter != module_names.end();
      iter++) {
    makeModuleExtern(externs, (*iter).first);
    modules << "  ";
    makeFindReturn(modules, "name", (*iter).first);
  }
  first = true;
  for(MakerMapIter iter = input_port_names.begin();
      iter != input_port_names.end();
      iter++) {
    makeIPortExtern(externs, (*iter).first);
    ports << "  ";
    makeFindPorts(ports, "name", (*iter).first);
  }
  for(MakerMapIter iter = output_port_names.begin();
      iter != output_port_names.end();
      iter++) {
    makeOPortExtern(externs, (*iter).first);
    ports << "  ";
    makeFindPorts(ports, "name", (*iter).first);
  }
  printMessage("\nStart of file\n");

  output << "/*\n";
  output << "   For more information, please see: http://software.sci.utah.edu\n";
  output << "\n";
  output << "   The MIT License\n";
  output << "\n";
  output << "   Copyright (c) 2007 Scientific Computing and Imaging Institute,\n";
  output << "   University of Utah.\n";
  output << "\n";
  output << "\n";
  output << "   Permission is hereby granted, free of charge, to any person obtaining a\n";
  output << "   copy of this software and associated documentation files (the \"Software\"),\n";
  output << "   to deal in the Software without restriction, including without limitation\n";
  output << "   the rights to use, copy, modify, merge, publish, distribute, sublicense,\n";
  output << "   and/or sell copies of the Software, and to permit persons to whom the\n";
  output << "   Software is furnished to do so, subject to the following conditions:\n";
  output << "\n";
  output << "   The above copyright notice and this permission notice shall be included\n";
  output << "   in all copies or substantial portions of the Software.\n";
  output << "\n";
  output << "   THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n";
  output << "   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n";
  output << "   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL\n";
  output << "   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n";
  output << "   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n";
  output << "   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n";
  output << "   DEALINGS IN THE SOFTWARE.\n";
  output << "*/\n";

  output << "#include <StaticHelper/Loader.h>\n";
  output << "#include <string>\n";

  output << "using namespace SCIRun;\n";
  output << "using std::string;\n";

  output << "// Start of externs\n";
  output << externs.str() << "\n\n\n";

  output << "// Start of module loader\n";
  output << "ModuleMaker SCIRun_getModule(const std::string& name) {\n";
  output << modules.str() << "\n";
  output << "  return 0;\n";
  output << "}\n\n";

  output << "// Start of port loader\n";
  output << "void* SCIRun_getPort(const std::string& name) {\n";
  output << ports.str() << "\n";
  output << "  return 0;\n";
  output << "}\n";

  printMessage("\nFinished loading packages.");
}

int main(int argc, char* argv[], char** environment)
{
  create_sci_environment(environment, 0);

  string filename("-");
  for(int i = 1; i < argc; ++i) {
    string arg(argv[i]);
    if (arg == "-o") {
      filename = argv[++i];
    }
    else {
      cerr << "Unknown argument "<<arg<<"\n";
      cerr << "Usage:\n";
      cerr << argv[0] << " [-o filename]\n";
      cerr << "\n";
      cerr << "filename defaults to '-' or stdout\n";
      return 1;
    }
  }

  if (filename == "-") {
    loadPackage(cout);
    return 0;
  }

  // Else do file I/O
  ofstream output(filename.c_str());
  if (output.fail()) {
    cerr << "Error opening "<<filename<<" for writing\n";
    return 2;
  }
  loadPackage(output);
  output.close();

  return 0;
}
