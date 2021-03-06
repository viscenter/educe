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


// PackageDB.cc - Interface to module-finding and loading mechanisms

#ifdef ASSERT
#  undef ASSERT
#endif

#include <Dataflow/Network/PackageDB.h>
#include <Dataflow/Network/ComponentNode.h>
#include <Dataflow/Network/NetworkEditor.h>
#include <Core/XMLUtil/XMLUtil.h>
#include <Core/Containers/StringUtil.h>
#include <Dataflow/GuiInterface/GuiInterface.h>
#include <Core/Util/Environment.h>
#include <Core/Util/soloader.h>
#include <Core/Util/FileUtils.h>
#include <Core/OS/Dir.h> // for LSTAT
#include <StaticHelper/Loader.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string>
#include <vector>

#include <libxml/xmlreader.h>
#include <libxml/catalog.h>

using namespace std;
#include <sys/stat.h>

#ifdef __APPLE__
  static string lib_ext = ".dylib";
#elif defined(_WIN32)
  const string lib_ext = ".dll";
#else
  static string lib_ext = ".so";
#endif

namespace SCIRun {

  PackageDB* packageDB = 0;

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
}

using namespace SCIRun;
  
PackageDB::PackageDB(GuiInterface* gui)
  : delayed_commands_(),
    db_(new Packages), 
    packageList_(0), 
    gui_(gui)
{
}

PackageDB::~PackageDB()
{ 
  delete db_; 
}

typedef void (*pkgInitter)(const string& tclPath);

bool
PackageDB::findMaker(ModuleInfo* moduleInfo)
{
  string cat_bname, pak_bname;
  if(moduleInfo->package_name_ == "SCIRun") {
    cat_bname = "Dataflow_Modules_";
    pak_bname = "Dataflow";
  } else {
    cat_bname = "Packages_" + moduleInfo->package_name_ + "_Dataflow_Modules_";
    pak_bname = "Packages_" + moduleInfo->package_name_ + "_Dataflow";
  }
  string errstr;
  string errorstr;

  string libname = pak_bname+lib_ext;
#ifndef _WIN32
  // don't add the "lib" under windows
  libname = string("lib") + libname;
#endif
  // try the large version of the shared library
  LIBRARY_HANDLE package_so = findLib(libname,errorstr);
  if (!package_so)
    errstr = string(" - ")+errorstr+string("\n");

  // If package is FieldsChoose, FieldsCreate, FieldsHandle Fields Packages,
  // or UnuA-M, UnuN-Z
  string cat_name = moduleInfo->category_name_;
  if((cat_name.find("Field") != string::npos || 
      cat_name.find("Mesh") != string::npos) &&
     moduleInfo->package_name_ == "SCIRun")
  { cat_name = "Fields"; }
  else if (cat_name.substr(0, 7) == "UnuAtoM") { cat_name = "Unu"; }
  else if (cat_name.substr(0, 7) == "UnuNtoZ") { cat_name = "Unu"; }

  // try the small version of the shared library
  libname = cat_bname+cat_name+lib_ext;
#ifndef _WIN32
  // don't add the "lib" under windows
  libname = string("lib") + libname;
#endif
  LIBRARY_HANDLE category_so = findLib(libname,errorstr);
  if (!category_so)
    errstr = string(" - ")+errorstr+string("\n");

  LIBRARY_HANDLE executable = GetLibraryHandle("");

  if (!category_so && !package_so && !executable) {
    printMessage("Unable to load all of package '" + moduleInfo->package_name_ +
		 "' (category '" + moduleInfo->category_name_ + "' failed) :\n" 
		 + errstr);
    cerr << ("Unable to load all of package '" + moduleInfo->package_name_ +
             "' (category '" + moduleInfo->category_name_ + "' failed) :\n"
             + errstr);
    return false;
  }

  string makename = "make_" + moduleInfo->module_name_;

  if (category_so)
    moduleInfo->maker_ = 
      (ModuleMaker)GetHandleSymbolAddress(category_so,makename);
  if (!moduleInfo->maker_ && package_so)
    moduleInfo->maker_ = 
      (ModuleMaker)GetHandleSymbolAddress(package_so,makename);
  if (!moduleInfo->maker_ && executable)
    moduleInfo->maker_ = 
      (ModuleMaker)GetHandleSymbolAddress(executable,makename);
  if (!moduleInfo->maker_ && executable)
    moduleInfo->maker_ = SCIRun_getModule(makename);
  if (!moduleInfo->maker_) {
    // the messages happen elsewere...
    if (!moduleInfo->optional_) {
      printMessage("Unable to load module '" + moduleInfo->module_name_ +
		   "' :\n - can't find symbol '" + makename + "'\n");
    }
    return false;
  }
  return true;
}


void
PackageDB::loadPackage(bool resolve)
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

  LIBXML_TEST_VERSION;

  string dtd = string(sci_getenv("SCIRUN_SRCDIR")) +
    string("/Dataflow/XML/component.dtd");

  xmlInitializeCatalog();
  xmlCatalogAdd(XMLUtil::char_to_xmlChar("public"),
                XMLUtil::char_to_xmlChar("-//SCIRun/Component DTD"),
                XMLUtil::char_to_xmlChar(dtd.c_str()));

  // the format of PACKAGE_PATH is a colon seperated list of paths to the
  // root(s) of package source trees.
  const char *srcdir = sci_getenv("SCIRUN_SRCDIR");
  ASSERT(srcdir);
  packagePath = srcdir + string("/Packages");

  // if the user specififes it, build the complete package path
  const char *packpath = sci_getenv("SCIRUN_PACKAGE_SRC_PATH");
  if (packpath) packagePath = packagePath + ":" + string(packpath);

  // the format of LOAD_PACKAGE is a comma seperated list of package names.
  // build the complete list of packages to load
  ASSERT(sci_getenv("SCIRUN_LOAD_PACKAGE"));
  loadPackage = string(sci_getenv("SCIRUN_LOAD_PACKAGE"));

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
    string finalPath;

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
      
      finalPath = pathElt+"/"+packageElt+"/Dataflow";
      if (validDir(finalPath)) {
	tmpPath = "found";
	break;
      }
      // external Package - do Package/src/Dataflow
      finalPath = pathElt+"/"+packageElt+"/src/Dataflow";
      if (validDir(finalPath)) {
        tmpPath = "found";
        break;
      }
      // We may be building an external package with SCIRun compatible modules.
      // Assume that SCIRun_PACKAGE_SRC_PATH points to the right place and
      // simply look for /Dataflow
      finalPath = pathElt+"/Dataflow";
      if (validDir(finalPath)) {
        tmpPath = "found";
        break;
      }
    }
    
    if (tmpPath=="") {
      printMessage("Unable to load package " + packageElt +
		   ":\n - Can't find " + packageElt +
		   " directory in package path");
      continue;
    }

    string xmldir;
    
    if(packageElt == "SCIRun") {
      xmldir = string(srcdir) + "/Dataflow/XML";
      gui_exec("lappend auto_path "+string(srcdir)+"/Dataflow/GUI");
    } else {
      xmldir = finalPath+"/XML";
      gui_exec(string("lappend auto_path ")+finalPath+"/GUI");
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

    mod_count += files->size();

    for (char_iter i=files->begin(); i != files->end();  i++) 
    {
      new_module = new ModuleInfo;
      new_module->package_name_ = packageElt;
      new_module->maker_ = 0;
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
  gui_exec("addProgressSteps " + to_string(mod_count));

  int index = 0;
  int numreg;
  
  for (pi = packages.begin();
       pi!=packages.end();
       pi++) {

    numreg = 0;
    
    string pname = (*pi).second->name;

    printMessage("Loading package '" + pname + "'");
    gui_exec("setProgressText {Loading package: " + pname + " }");

    for (ci = (*pi).second->categories.begin();
	 ci!=(*pi).second->categories.end();
	 ci++) {
      for (mi = (*ci).second->modules.begin();
	   mi!=(*ci).second->modules.end();
	   mi++) {
	if(resolve){
	  if(findMaker((*mi).second)){
	    registerModule((*mi).second);
	    numreg++;
	  } else {
	    string mname = (*mi).second->module_name_;
	    if (! ((*mi).second)->optional_) {
	      printMessage("Unable to load module '" + mname +
			   "' :\n - can't find symbol 'make_" + mname + "'");
	    }
	  }
	} else {
	  numreg++;
	  registerModule((*mi).second);
	}
	gui_exec("incrProgress");
      }
    }
    
    if (numreg) {
      gui_exec("createPackageMenu " + to_string(index++));
    } else {
      printMessage("Unable to load package " + pname + ":\n"
		   " - could not find any valid modules.");
    }
  }

  printMessage("\nFinished loading packages.");
}
  
void
PackageDB::registerModule(ModuleInfo* info) 
{
  Package* package;
  if(!db_->lookup(info->package_name_,package))
  {
    db_->insert(info->package_name_,package=new Package);
    packageList_.push_back( info->package_name_ );
  }
  
  string cat_name = info->category_name_;
  if((cat_name.substr(0, 13) == "Conglomerate_"))
  { cat_name.erase(0,13); }

  Category* category;
  if(!package->lookup(cat_name,category))
    package->insert(cat_name,category=new Category);
  
  ModuleInfo* moduleInfo;
  if(!category->lookup(info->module_name_,moduleInfo)) {
    moduleInfo=new ModuleInfo;
    category->insert(info->module_name_,info);
  } else cerr << "WARNING: Overriding multiply registered module "
	      << info->package_name_ << "." << info->category_name_ << "."
	      << info->module_name_ << "\n";  
}
 
Module*
PackageDB::instantiateModule(const string& packageName,
                             const string& categoryName,
                             const string& moduleName,
                             const string& instanceName)
{
  Package* package;
  if(!db_->lookup(packageName,package)) {
    cerr << "ERROR: Instantiating from nonexistant package " << packageName 
	 << "\n";
    return 0;
  }
  
  Category* category;
  if(!package->lookup(categoryName,category)) {
    cerr << "ERROR: Instantiating from nonexistant category " << packageName
	 << "." << categoryName << "\n";
    return 0;
  }
  
  ModuleInfo* moduleInfo;
  if(!category->lookup(moduleName,moduleInfo)) {
    cerr << "ERROR: Instantiating nonexistant module " << packageName 
	 << "." << categoryName << "." << moduleName << "\n";
    return 0;
  }

  if(!moduleInfo->maker_){
    if(!findMaker(moduleInfo)){
      cerr << "ERROR: Cannot find maker for module: " << packageName 
	   << "." << categoryName << "." << moduleName << "\n";
      return 0;
    }
  }

  ASSERT(gui_);
  GuiContext* module_context = gui_->createContext(instanceName);
  Module *module = (moduleInfo->maker_)(module_context);
  if(!module)
    return 0;
  
  // Some modules may already know their package and category.
  // If this module doesn't, then set it's package and category here.
  string unknown("unknown");
  if (unknown == module->package_name_)
    module->package_name_ = packageName;
  if (unknown == module->category_name_)
    module->category_name_ = categoryName;
  
  // copy other fields 
  module->lastportdynamic_ = moduleInfo->last_port_dynamic_;
  
  return module;
}
 
bool
PackageDB::haveModule(const string& packageName,
                      const string& categoryName,
                      const string& moduleName) const
{
  Package* package;
  if(!db_->lookup(packageName,package))
    return false;
  
  Category* category;
  if(!package->lookup(categoryName,category))
    return false;
  
  ModuleInfo* moduleInfo;
  if(!category->lookup(moduleName,moduleInfo))
    return false;

  return true;
}


bool
PackageDB::replaceDeprecatedModule(const string &pname,
                                   const string &cname,
                                   const string &mname,
                                   string &npname,
                                   string &ncname,
                                   string &nmname) const
{
  string full_module_name = pname + "::" + cname + "::" + mname;
  static bool loaded = false;
  static map<string, string, less<string> > replacement_map;
  if (!loaded)
  {
    string rmfname = string(sci_getenv("SCIRUN_SRCDIR"))
      + "/scripts/module-remapping.txt";
    ifstream ifile(rmfname.c_str());
    char buffer[4096];
    while (ifile)
    {
      ifile.getline(buffer, 4096);
      string line(buffer);
      if (line.size() < 10) break;
      string before = line.substr(0, line.find_first_of(" \t\n"));
      string after = line.substr(line.find_last_of(" \t\n", line.size()-1)+1);
      replacement_map[before] = after;
    }
    ifile.close();
  }
  map<string, string, less<string> >::iterator loc =
    replacement_map.find(full_module_name);
  if (loc != replacement_map.end())
  {
    const string &replacement = (*loc).second;
    size_t l0 = replacement.find_first_of(':', 0);
    size_t l1 = replacement.find_first_of(':', l0+2);
    npname = replacement.substr(0, l0);
    ncname = replacement.substr(l0+2, l1-l0-2);
    nmname = replacement.substr(l1+2);

    // cout << "REPLACING MODULE " << full_module_name << "\n";
    // cout << " newpac = " << npname << "\n";
    // cout << " newcat = " << ncname << "\n";
    // cout << " newmod = " << nmname << "\n";
    return true;
  }
  else
  {
    return false;
  }
}


vector<string>
PackageDB::packageNames(void) const
{
  // packageList_ is used to keep a list of the packages 
  // that are in this PSE IN THE ORDER THAT THEY ARE SPECIFIED
  // by the user in the Makefile (for main.cc) or in their
  // environment.
  
  return packageList_;
}

vector<string>
PackageDB::categoryVNames(const string& packageName) const
{
  Package* package;
  if(!db_->lookup(packageName, package))
  {
    cerr << "WARNING: Unknown package " << packageName << endl;
    vector<string> result(0);
    return result;
  }
  vector<string> result;
  PackageIter iter(package);
  
  for(iter.first();iter.ok();++iter)
  {
    // Do not list category if it does not contain any modules
    vector<string> test = moduleVNames(packageName,iter.get_key());
    if (test.size()) result.push_back(iter.get_key());
  }
  return result;
}

vector<string>
PackageDB::categoryNames(const string& packageName) const
{
  Package* package;
  if(!db_->lookup(packageName, package))
  {
    cerr << "WARNING: Unknown package " << packageName << endl;
    vector<string> result(0);
    return result;
  }
  vector<string> result;
  PackageIter iter(package);
  
  for(iter.first();iter.ok();++iter)
  {
    // Do not list category if it does not contain any modules
    result.push_back(iter.get_key());
  }
  return result;
}


string
PackageDB::getCategoryName(const string &packName,
			   const string &catName,
			   const string &modName)
{
  Package *package;
  if (!db_->lookup(packName, package)){
    cerr << "WARNING: Unknown package " << packName << "\n";
    return catName;
  }

  Category *category;
  ModuleInfo* modinfo;
  if (package->lookup(catName, category) &&
      category->lookup(modName, modinfo))
  {
    // Original category was fine, just return that.
    return catName;
  }

  // Look up the package name somewhere else.  Find a remapping.
  PackageIter iter(package);
  for (iter.first(); iter.ok();++iter)
  {
    if (iter.get_data()->lookup(modName, modinfo))
    {
      return iter.get_key();
    }
  }
  return catName;
}
 

vector<string>
PackageDB::moduleNames(const string& packageName,
		       const string& categoryName) const
{
  Package* package;
  if(!db_->lookup(packageName, package)){
    cerr << "WARNING: Unknown package " << packageName << "\n";
    vector<string> result(0);
    return result;
  }

  Category* category;
  if(!package->lookup(categoryName, category)){
    cerr << "WARNING: Unknown category " << packageName << "."
	 << categoryName << "\n";
    vector<string> result(0);
    return result;
  }
  
  vector<string> result;
  CategoryIter iter(category);
  //int i=0;
  
  for(iter.first();iter.ok();++iter) 
  {
    result.push_back(iter.get_key());
  }
  return result;
}

vector<string>
PackageDB::moduleVNames(const string& packageName,
		       const string& categoryName) const
{
  Package* package;
  if(!db_->lookup(packageName, package)){
    cerr << "WARNING: Unknown package " << packageName << "\n";
    vector<string> result(0);
    return result;
  }

  Category* category;
  if(!package->lookup(categoryName, category)){
    cerr << "WARNING: Unknown category " << packageName << "."
	 << categoryName << "\n";
    vector<string> result(0);
    return result;
  }
  
  vector<string> result;
  CategoryIter iter(category);
  //int i=0;
  
  for(iter.first();iter.ok();++iter) 
  {
    if (iter.get_data()->hide_ == false)
    {
      result.push_back(iter.get_key());
    }
  }
  return result;
}

void
PackageDB::setGui(GuiInterface* gui)
{
  gui_ = gui;
  if (gui_) {
    for(vector<string>::iterator iter = delayed_commands_.begin();
	iter != delayed_commands_.end(); ++iter){
      gui_->execute(*iter);
    }
    delayed_commands_.clear();
  }
}

void
PackageDB::gui_exec(const string& command)
{
  if(gui_)
    gui_->execute(command);
  else
    delayed_commands_.push_back(command);
}

ModuleInfo*
PackageDB::GetModuleInfo(const string& name,
			 const string& catname,
			 const string& packname)
{
  Package* package;
  if (!db_->lookup(packname,package))
    return 0;

  Category* category;
  if (!package->lookup(catname,category))
    return 0;

  ModuleInfo* info;
  if (category->lookup(name,info))
    return info;
  return 0;
}


void
PackageDB::printMessage(const string &msg) 
{
  if(gui_){
    gui_->post_message(msg);
    gui_->execute("update idletasks");
  } else {
    cerr << msg << "\n";
  }
}

  
