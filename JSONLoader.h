#pragma once
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace JSONLoader {
	static const std::string jsonPath = "JSON/";				// JSON config files folder

	json Load(std::string filename);

	class JSONLoader {

	public:
		json _jObj;

		std::ifstream _jFile;
		//std::ofstream _jFile;						// output file stream - not needed yet

		//bool LoadCameras(&_cam)
	};

}