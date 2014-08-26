#ifndef __RfMeshUtils_h__
#define __RfMeshUtils_h__

#include "RfMeshNodeConfig.h"

class RfMeshUtils {
	public:
		static std::string u8tohex(uint8_t u) {
			uint8_t r = u;
			std::string res = "";
			while (r > 0) {
				uint8_t h = r % 16;
				if (h < 10)
					res = ((char)('0' + h)) + res;
				else if (h < 16)
					res = ((char)('A' + h - 10)) + res;
				r = (r - h) / 16;
			}
			while (res.length() < 2)
				res = "0" + res;
			return res;
		}

		static std::string u16tohex(uint16_t u) {
			uint16_t r = u;
			std::string res = "";
			while (r > 0) {
				uint16_t h = r % 16;
				if (h < 10)
					res = ((char)('0' + h)) + res;
				else if (h < 16)
					res = ((char)('A' + h - 10)) + res;
				r = (r - h) / 16;
			}
			while (res.length() < 4)
				res = "0" + res;
			return res;
		}

		static uint8_t hextou8(std::string h) {
			uint8_t res = 0;
			for (uint8_t i = 0; i < h.length(); i++) {
				res *= 16;
				if (h[i] >= '0' && h[i] <= '9')
					res += h[i] - '0';
				else if (h[i] >= 'A' && h[i] <= 'F')
					res += h[i] - 'A' + 10;
				else if (h[i] >= 'a' && h[i] <= 'f')
					res += h[i] - 'a' + 10;
//				else
//					LOG4CXX_ERROR(logger, "ERROR in hextou8 - converting hex string to uint8 : " + h);
			}
			return res;
		}

		static uint16_t hextou16(std::string h) {
			uint16_t res = 0;
			for (uint8_t i = 0; i < h.length(); i++) {
				res *= 16;
				if (h[i] >= '0' && h[i] <= '9')
					res += h[i] - '0';
				else if (h[i] >= 'A' && h[i] <= 'F')
					res += h[i] - 'A' + 10;
				else if (h[i] >= 'a' && h[i] <= 'f')
					res += h[i] - 'a' + 10;
//				else
//					LOG4CXX_ERROR(logger, "ERROR in hextou16 - converting hex string to uint16 : " + h);
			}
			return res;
		}

		static std::string u8tos(uint8_t i) {
			char s[4];
			sprintf(s, "%d", i);
			return (std::string)s;
		}

		static std::string u16tos(uint16_t i) {
			char s[6];
			sprintf(s, "%d", i);
			return (std::string)s;
		}
		
		static uint8_t stou8(std::string d) {
			uint8_t res = 0;
			for (uint8_t i = 0; i < d.length(); i++) {
				res *= 10;
				if (d[i] >= '0' && d[i] <= '9')
					res += d[i] - '0';
//				else
//					LOG4CXX_ERROR(logger, "ERROR in hextou8 - converting hex string to uint8 : " + h);
			}
			return res;
		}

		static uint16_t stou16(std::string d) {
			uint16_t res = 0;
			for (uint8_t i = 0; i < d.length(); i++) {
				res *= 10;
				if (d[i] >= '0' && d[i] <= '9')
					res += d[i] - '0';
//				else
//					LOG4CXX_ERROR(logger, "ERROR in hextou16 - converting hex string to uint16 : " + h);
			}
			return res;
		}
		
		static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
			std::stringstream ss(s);
			std::string item;
			while (std::getline(ss, item, delim))
				elems.push_back(item);
			return elems;
		}

		static std::vector<std::string> split(const std::string &s, char delim) {
			std::vector<std::string> elems;
			split(s, delim, elems);
			return elems;
		}
		
		static std::string MeshMessageToJson(meshMessage mm) {
			Json::Value root;
			root["tc"] = RfMeshUtils::u8tohex(mm.tc);
			root["to"] = RfMeshUtils::u16tohex(mm.to);
			root["from"] = RfMeshUtils::u16tohex(mm.from);
			root["source"] = RfMeshUtils::u16tohex(mm.source);
			root["dest"] = RfMeshUtils::u16tohex(mm.dest);
			root["pid"] = RfMeshUtils::u8tohex(mm.pid);
			root["ptype"] = RfMeshUtils::u8tohex(mm.ptype);
			root["cost"] = RfMeshUtils::u8tohex(mm.cost);
			Json::Value data = Json::Value(Json::arrayValue);
			for (uint8_t i = 0; i < mm.len; i++)
				data.append(RfMeshUtils::u8tohex(mm.data[i]));
			root["data"] = data;
			Json::FastWriter writer;
			std::string json = writer.write(root);
			return json;
		}

		static std::string AppMessageToJson(appMessage am) {
			Json::Value root;
			root["source"] = RfMeshUtils::u16tohex(am.source);
			root["dest"] = RfMeshUtils::u16tohex(am.dest);
			root["pid"] = RfMeshUtils::u8tohex(am.pid);
			root["ptype"] = RfMeshUtils::u8tohex(am.ptype);
			Json::Value data = Json::Value(Json::arrayValue);
			for (uint8_t i = 0; i < am.len; i++)
				data.append(RfMeshUtils::u8tohex(am.data[i]));
			root["data"] = data;
			Json::FastWriter writer;
			std::string json = writer.write(root);
			return json;
		}

		static appMessage JsonToAppMessage(std::string json) {
			appMessage am;
			Json::Value root;
			Json::Reader reader;
			if (!reader.parse(json, root)) {
				printf("error parsing json\n");
				return am;
			}
			am.source = RfMeshUtils::hextou16(root.get("source", "0").asString());
			am.dest = RfMeshUtils::hextou16(root.get("dest", "0").asString());
			am.pid = RfMeshUtils::hextou8(root.get("pid", "0").asString());
			am.ptype = RfMeshUtils::hextou8(root.get("ptype", "0").asString());
			const Json::Value data = root["data"];
			am.len = data.size();
			for (int i = 0; i < data.size(); i++)
				am.data[i] = RfMeshUtils::hextou8(data[i].asString());
			return am;
		}
		
		static std::string now() {
			time_t rawtime;
			struct tm * timeinfo;
			char buffer[80];
			time (&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer,80,"%F %T",timeinfo);
			std::string str(buffer);
			return str;
		}
};

#endif