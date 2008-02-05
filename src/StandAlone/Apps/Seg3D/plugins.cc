#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>

#include <StandAlone/Apps/Seg3D/config.h>
#include <StandAlone/Apps/Seg3D/plugins.h>

namespace SCIRun {

std::vector<fs::path>
ScanPlugins(void)
{
	std::vector<fs::path> plugins;
	fs::path full_path = fs::system_complete(fs::path(DATAPATH "/plugins" ,fs::native));
	
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
LoadPlugins(wxMenu * menu)
{
	std::vector<fs::path> plugins = ScanPlugins();

	for(unsigned int i = 0; i < plugins.size(); i++) {
		menu->Append(MENU_PLUGIN_START+i, _T(plugins[i].leaf()));
	}
}

}
