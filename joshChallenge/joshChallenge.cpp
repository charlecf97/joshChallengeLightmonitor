#include <iostream>
#include <map>
#include "httplib.h"
#include "json.hpp"

struct Light {
	std::string name;
    std::string id;
    std::string room;
    bool on;
    int brightness;
};

class Monitor {
private:
    std::map<std::string, Light> lights;
    httplib::Client cli;
	bool firstRun = true;

public:
    Monitor() : cli("http://localhost:8080") {
        // Initialization if needed
    }

    void checkLightDetails(const Light& l, std::map<std::string, Light>& tempHolder) {
        Light lightDetails = getLightState(l.id);
        if (lightDetails.id.empty()) {
            std::cerr << "Failed to get light state for ID: " << l.id << std::endl;
            return;
		}
        if (lights.empty() || lights.count(l.id) == 0) {
            tempHolder[l.id] = lightDetails;
            if (!firstRun) {
				printNewLight(lightDetails);
            }
        }else { //light exist in currrent state, check if it has changed
            checkLightChanges(lightDetails,tempHolder);
        }
	}

    void checkLightChanges(const Light& updatedLight, std::map<std::string, Light>& tempHolder) {
		Light currentLight = lights[updatedLight.id];
        if (currentLight.on != updatedLight.on) {
            printChange(updatedLight.id, "on", updatedLight.on ? "true" : "false");
        }
        if (currentLight.brightness != updatedLight.brightness) {
            printChange(updatedLight.id, "brightness", std::to_string(updatedLight.brightness));
        }
        if (currentLight.room != updatedLight.room) {
            printChange(updatedLight.id, "room", updatedLight.room);
        }
        if (currentLight.name != updatedLight.name) {
            printChange(updatedLight.id, "name", updatedLight.name);
		}

		// Add Updated light to tempHolder, and delete it from the current lights map to track removed lights later
        tempHolder[updatedLight.id] = updatedLight;
        lights.erase(updatedLight.id);
    }

    void printChange(std::string id, std::string property, std::string value) {
        std::cout << "{" << std::endl;
        std::cout << "    \"id\": \"" << id << "\"," << std::endl;
        std::cout << "    \"" << property << "\": \"" << value << "\"" << std::endl;
        std::cout << "}" << std::endl;
    }

    void printNewLight(const Light& l) {
        std::cout << "{" << std::endl;
        std::cout << "    \"name\": \"" << l.name << "\"," << std::endl;
        std::cout << "    \"id\": \"" << l.id << "\"," << std::endl;
        std::cout << "    \"room\": \"" << l.room << "\"," << std::endl;
        std::cout << "    \"on\": \"" << (l.on ? "true" : "false") << "\"," << std::endl;
        std::cout << "    \"brightness\": \"" << l.brightness << "\"" << std::endl;
        std::cout << "}" << std::endl;
    }

    void printRemovedLight(const Light& l) {
		std::cout << l.name << " (" << l.id << ") has been removed." << std::endl;
    }

    int lightBrightnessPercent(int brightness) {
        return static_cast<int>(brightness * 100.0 / 255.0);
	}

    Light getLightState(const std::string& id) {
        auto res = cli.Get("/lights/" + id);
        if (!res) {
            std::cerr << "Request failed or connection error.\n";
            return Light{};
        }
        if (res->status != 200) {
            std::cerr << "Failed to get light state. HTTP status: " << res->status << std::endl;
            return Light{};
        }
        try {
            nlohmann::json j = nlohmann::json::parse(res->body);
            Light l;
            l.id = id;
            l.name = j.value("name", "");
            l.room = j.value("room", "");
            l.on = j.value("on", false);
            l.brightness = lightBrightnessPercent(j.value("brightness", 0));
            return l;
        }
        catch (const std::exception& ex) {
            std::cerr << "JSON parse error: " << ex.what() << std::endl;
            return Light{};
        }
	}

    void start() {
        // Start monitoring lights, e.g., by setting up a timer and loop
		// Sleeps for 5 seconds between polls to avoid overwhelming the server
        while(true) {
            getAllLights();
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Poll every 5 seconds
		}
    }

    void getAllLights() {
        std::map<std::string, Light> tempHolder;
        auto res = cli.Get("/lights");

        if (!res) {
            std::cerr << "Request failed or connection error.\n";
            return;
        }

        if (res->status != 200) {
            std::cerr << "Failed to get lights status. HTTP status: " << res->status << std::endl;
            return;
        }

        try {
            nlohmann::json j = nlohmann::json::parse(res->body);

            // The API returns a top-level array of lights:
            // [
            //   { "id": "...", "name": "...", "room": "..." },
            //   ...
            // ]
            if (!j.is_array()) {
                std::cerr << "Unexpected JSON format for lights: expected array\n";
                return;
            }

            for (auto& item : j) {
                Light l;
                // Set defaults for fields not present in this API shape
                l.on = false;
                l.brightness = 0;

                if (item.contains("id") && item["id"].is_string()) {
                    l.id = item["id"].get<std::string>();
                }
                if (item.contains("name") && item["name"].is_string()) {
                    l.name = item["name"].get<std::string>();
                }
                if (item.contains("room") && item["room"].is_string()) {
                    l.room = item["room"].get<std::string>();
                }

				// Update internal map if we have an id not in the map yet
                if (!l.id.empty()) {
					checkLightDetails(l, tempHolder);
					
                    // If checkLightDetails populated tempHolder[l.id], copy the real state back into the JSON item
                    auto it = tempHolder.find(l.id);
                    if (it != tempHolder.end()) {
                        item["on"] = it->second.on;
                        item["brightness"] = it->second.brightness;
                    }
                }
            }
            if (firstRun) {
				std::cout << j.dump(4) << std::endl;
				std::cout << std::endl;
				firstRun = false;
            }
			// After processing all lights, any remaining lights in the internal map are considered removed
            for (const auto& pair : lights) {
                printRemovedLight(pair.second);
            }
            // Update the internal map to the new state
			lights = tempHolder;
        }
        catch (const std::exception& ex) {
            std::cerr << "JSON parse error: " << ex.what() << std::endl;
        }
    }
};

int main()
{
	Monitor monitor;
    monitor.start();
}

