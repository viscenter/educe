#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>

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
				std::cout << dir_itr->path() << "\n";
				plugins.push_back(dir_itr->path());
			}
		}
	}
	return plugins;
}

void
PluginManager::LoadPlugins(wxMenu * menu)
{
	std::vector<fs::path> scanned_plugins = ScanPlugins( DATAPATH "/plugins" );

	for(unsigned int i = 0; i < scanned_plugins.size(); i++) {
		int thiswxid = curwxid++;
		void * thisdl = dlopen(scanned_plugins[i].string().c_str(), RTLD_LAZY);
		if(!thisdl) {
			cerr << "Cannot load library: " << dlerror() << '\n';
		}
		else {
			dlerror(); // reset errors
			plugins[thiswxid] = Plugin(scanned_plugins[i], thisdl); // add plugin
			
			create_t* plugin_create = (create_t*) dlsym(thisdl, "create");
			const char * dlsym_error = dlerror();
			if(dlsym_error) {
				cerr << "Cannot load symbol create: " << dlsym_error << '\n';
			}
			else {
				plugins[thiswxid].gp = plugin_create();
				menu->Append(thiswxid, _T(plugins[thiswxid].gp->get_menu_string().c_str()));
			}
		}

		// menu->Append(MENU_PLUGIN_START+i, _T(plugins[i].leaf().c_str()));
	}
}

}
