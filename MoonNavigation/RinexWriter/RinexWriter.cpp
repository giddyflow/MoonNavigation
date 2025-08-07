#include "RinexWriter.h"

// Вспомогательная функция для получения текущей даты в формате RINEX
std::string getCurrentDateForHeader() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_utc;

    #if defined(_WIN32)
        gmtime_s(&tm_utc, &now_time);
    #elif defined(__linux__)
        gmtime_r(&now_time, &tm_utc);
    #else
        #pragma message("Warning: Using non-thread-safe gmtime() on an unsupported OS.")
        std::tm* tm_ptr = std::gmtime(&now_time);
        if (tm_ptr) {
            tm_utc = *tm_ptr;
        }
    #endif

    std::stringstream ss;
    ss << std::put_time(&tm_utc, "%Y%m%d %H%M%S") << " UTC";
    return ss.str();
}

RinexWriter::RinexWriter(std::shared_ptr<Bus> bus, json config) {

}



RinexWriter::~RinexWriter() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

void RinexWriter::createHeader(json config) {
    if (!m_file.is_open()) return;

    m_file << std::fixed;

    m_file << std::setw(9) << "3.05" << std::setw(11) << "" << "OBSERVATION DATA"
           << std::setw(20) << "M (MIXED)" << std::setw(19) << "" << "RINEX VERSION / TYPE" << std::endl;

    m_file << std::left << std::setw(20) << "MoonNavigation"
           << std::setw(20) << "Grigoriy"
           << std::setw(20) << getCurrentDateForHeader()
           << "PGM / RUN BY / DATE" << std::endl;

    m_file << std::left << std::setw(60) << "STN1" << "MARKER NAME" << std::endl;

    m_file << std::left << std::setw(20) << "Grigoriy" << std::setw(40) << "MyUniversity"
           << "OBSERVER / AGENCY" << std::endl;

    m_file << std::left << std::setw(20) << "SIM01" << std::setw(20) << "MoonNavReceiver"
           << std::setw(20) << "1.0" << "REC # / TYPE / VERS" << std::endl;

    m_file << std::left << std::setw(20) << "SIM_ANT" << std::setw(40) << "SIM_ANTENNA"
           << "ANT # / TYPE" << std::endl;

    double approx_x = config.value("receiver_pos_x", 0.0);
    double approx_y = config.value("receiver_pos_y", 0.0);
    double approx_z = config.value("receiver_pos_z", 0.0);
    m_file << std::right << std::setprecision(4)
           << std::setw(14) << approx_x
           << std::setw(14) << approx_y
           << std::setw(14) << approx_z
           << std::setw(18) << "" << "APPROX POSITION XYZ" << std::endl;

    m_file << "R    4 C1C L1C D1C S1C" << std::setw(23) << "" << "SYS / # / OBS TYPES" << std::endl;
    
    double interval = config.value("period", 1.0);
    m_file << std::setprecision(3) << std::setw(10) << interval << std::setw(50) << ""
           << "INTERVAL" << std::endl;

    int year = config["time"].value("year", 2024);
    int month = config["time"].value("month", 1);
    int day = config["time"].value("day", 1);
    m_file << std::setw(6) << year << std::setw(6) << month << std::setw(6) << day
           << std::setw(6) << 0 << std::setw(6) << 0 << std::setw(13) << std::setprecision(7) << 0.0
           << "      GPS" << " " // Система времени, обычно GPS
           << "TIME OF FIRST OBS" << std::endl;

    m_file << std::left << std::setw(60) << "" << "END OF HEADER" << std::endl;
}


void RinexWriter::processEpoch(double epoch_time, const RinexEpoch& epoch) {

}