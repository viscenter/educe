#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>

#include <Core/Util/Environment.h>
#include <StandAlone/Apps/Seg3D/config.h>
#include <StandAlone/Apps/Seg3D/plugins.h>

#include <dlfcn.h>

namespace SCIRun {

std::vector<fs::path>
PluginManager::ScanPlugins(std::string directory)
{
	std::vector<fs::path> plugins;
	fs::path full_path = fs::system_complete(fs::path(directory,fs::native));
	
	if(fs::exists(full_path) && fs::is_directory(full_path)) {
		fs::directory_iterator end_iter;
		for ( fs::directory_iterator dir_itr( full_path );
				dir_itr != end_iter;
				++dir_itr )
		{
			if(fs::is_regular(dir_itr->status())) {
				plugins.push_back(dir_itr->path());
			}
		}
	}
	return plugins;
}

void
PluginManager::LoadPlugins(wxMenu * menu, int type, wxFrame * frame)
{
	std::vector<fs::path> scanned_plugins = ScanPlugins( sci_getenv("SCIRUN_OBJDIR") +
			string("data") + string("/plugins") );

	for(unsigned int i = 0; i < scanned_plugins.size(); i++) {
		std::cout << scanned_plugins[i] << '\n';
		int thiswxid = curwxid++;
		void * thisdl = dlopen(scanned_plugins[i].string().c_str(), RTLD_LAZY | RTLD_LOCAL);
		if(!thisdl) {
			cerr << "Cannot load library: " << dlerror() << '\n';
		}
		else {
			dlerror(); // reset errors
			int* plugin_version = (int*) dlsym(thisdl, "plugin_version");
			const char * dlsym_error = dlerror();
			if(dlsym_error) {
				cerr << "Cannot load symbol plugin_version: " << dlsym_error << '\n';
			}
			else if((*plugin_version) != PLUGIN_VERSION) {
				cerr << "Plugin version mismatch, skipping\n";
			}
			else {
				plugins[thiswxid] = Plugin(scanned_plugins[i], thisdl); // add plugin
				
				create_t* plugin_create = (create_t*) dlsym(thisdl, "create");
				const char * dlsym_error = dlerror();
				if(dlsym_error) {
					cerr << "Cannot load symbol create: " << dlsym_error << '\n';
				}
				else {
					plugins[thiswxid].gp = plugin_create();
					plugins[thiswxid].plugin_creator = plugin_create;
					GenericPlugin * thisgp = (GenericPlugin *) plugins[thiswxid].gp;
					printf("FP addr inside PM: %x\n",thisgp);
					if(thisgp->get_type() == type) {
						printf("Menu string: %s\n",thisgp->get_menu_string().c_str());
						menu->Append(thiswxid, _T(thisgp->get_menu_string().c_str()));
						frame->Connect(thiswxid, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seg3DFrame::PluginEvent) );
					}
				}
			}
		}
	}
}

void
PluginManager::HandleEvent(wxCommandEvent& event) {
	((GenericPlugin *)plugins[event.GetId()].gp)->menu_event();
}

std::string
PluginManager::GetPluginTitle(int id) {
	return ((GenericPlugin*)plugins[id].gp)->get_menu_string();
}

void *
PluginManager::GetPluginInstance(int id) {
	return plugins[id].plugin_creator();
}

}
