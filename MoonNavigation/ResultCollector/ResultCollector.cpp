#include "ResultCollector.h"
#include <fstream>
#include <string>

ResultCollector::ResultCollector(std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir) {

    m_output_path = output_dir / "output.json";
    
    m_results["timesteps"] = json::object();

    bus->subscribe("MedSatData", [this](std::shared_ptr<Event> eventData) {
        if (auto satEvent = std::dynamic_pointer_cast<SatelliteEvent>(eventData)) {
            handleSatelliteEvent(satEvent);
        }
    });

    // 2. Подписка на данные от приемников
    bus->subscribe("ReceiverData", [this](std::shared_ptr<Event> eventData) {
        if (auto recEvent = std::dynamic_pointer_cast<ReceiverEvent>(eventData)) {
            handleReceiverEvent(recEvent);
        }
    });

    // 3. Подписка на данные от помех
    bus->subscribe("JamData", [this](std::shared_ptr<Event> eventData) {
        if (auto jamEvent = std::dynamic_pointer_cast<JamEvent>(eventData)) {
            handleJamEvent(jamEvent);
        }
    });

    // 4. Подписка на команду сохранения в конце моделирования
    bus->subscribe("SaveAllResults", [this](std::shared_ptr<Event> eventData) {
        this->saveResults();
    });
}

void ResultCollector::handleSatelliteEvent(const std::shared_ptr<SatelliteEvent>& event) {
    // Преобразуем время в строковый ключ
    std::string time_key = std::to_string(static_cast<int>(event->satState.current_time));

    json nka_entry;
    nka_entry["id"] = event->satState.id;
    nka_entry["power"] = event->satState.Ptx;
    
    nka_entry["model_coords_ecef"] = {
        {"x", event->satState.ecef.x},
        {"y", event->satState.ecef.y},
        {"z", event->satState.ecef.z}
    };
    nka_entry["estimated_coords_ecef"] = {
        {"x", event->satState.eph.xyz.x},
        {"y", event->satState.eph.xyz.y},
        {"z", event->satState.eph.xyz.z}
    };
    nka_entry["coords_difference"] = {
        {"dx", event->satState.residual.x},
        {"dy", event->satState.residual.y},
        {"dz", event->satState.residual.z}
    };
    // Добавьте сюда любые другие поля для nka
    m_results["timesteps"][time_key]["nka"].push_back(nka_entry);

    // --- Запись в nkaeci (только модельные координаты ECI) ---
    json nkaeci_entry;
    nkaeci_entry["id"] = event->satState.id;
    nkaeci_entry["model_coords_eci"] = {
        {"x", event->satState.eci.x},
        {"y", event->satState.eci.y},
        {"z", event->satState.eci.z}
    };
    m_results["timesteps"][time_key]["nkaeci"].push_back(nkaeci_entry);
}

void ResultCollector::handleReceiverEvent(const std::shared_ptr<ReceiverEvent>& event) {
    std::string time_key = std::to_string(static_cast<int>(event->recState.current_time));
    
    json nap_entry;
    nap_entry["id"] = event->recState.id;
    
    nap_entry["model_coords_blh"] = {
        {"lat", event->recState.blh.lat},
        {"lon", event->recState.blh.lon},
        {"h", event->recState.blh.h}
    };
    //
    BLH est_coords = ECEF2BLH(event->recState.est_ecef);
    nap_entry["estimated_coords_blh"] = {
        {"lat", est_coords.lat},
        {"lon", est_coords.lon},
        {"h", est_coords.h}
    };
    //
    
    nap_entry["coords_difference"] = {
        {"dx", event->recState.residual.x},
        {"dy", event->recState.residual.y},
        {"dz", event->recState.residual.z}
    };
    nap_entry["dop"] = {
        {"pdop", event->recState.dop.PDOP},
        {"gdop", event->recState.dop.HDOP} // Пример, добавьте нужные
    };

    m_results["timesteps"][time_key]["nap"].push_back(nap_entry);
}

void ResultCollector::handleJamEvent(const std::shared_ptr<JamEvent>& event) {
    std::string time_key = std::to_string(static_cast<int>(event->jamState.current_time));
    
    json jam_entry;
    jam_entry["id"] = event->jamState.id;
    jam_entry["power_dbwt"] = event->jamState.power_tx;
    jam_entry["coords_blh"] = {
        {"lat", event->jamState.blh.lat},
        {"lon", event->jamState.blh.lon},
        {"h", event->jamState.blh.h}
    };

    m_results["timesteps"][time_key]["jam"].push_back(jam_entry);
}

void ResultCollector::saveResults() {
    std::cout << "Saving simulation log to " << m_output_path << "..." << std::endl;
    std::ofstream outFile(m_output_path);
    if (outFile.is_open()) {
        outFile << m_results.dump(4); // dump(4) для красивого вывода с отступами
    } else {
        std::cerr << "Error: could not open file for writing: " << m_output_path << std::endl;
    }
}