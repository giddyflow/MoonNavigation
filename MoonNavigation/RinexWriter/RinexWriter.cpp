#include "RinexWriter.h"

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

    this->eventBus = bus;
    std::string filename = "output.obs";
    m_file.open(filename, std::ios::out);

    if (!m_file.is_open()) {
        std::cerr << "ERROR: Could not open RINEX file: " << filename << std::endl;
        return; 
    }
    
    createHeader(config);

    eventBus->subscribe("ReceiverMeasurementsReady", [this](std::shared_ptr<Event> eventData) {
        auto measurementsEvent = std::dynamic_pointer_cast<ReceiverMeasurementsReadyEvent>(eventData);
    
        if (measurementsEvent) {
            this->processEpoch(measurementsEvent->epoch_time, measurementsEvent->epoch_data);
        }
    });
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
    std::string start_time_str = config.at("start_time").get<std::string>();
    int year, month, day;
    std::tie(year, month, day, std::ignore, std::ignore, std::ignore) = parseDateTime(start_time_str);
    m_file << std::setw(6) << year << std::setw(6) << month << std::setw(6) << day
           << std::setw(6) << 0 << std::setw(6) << 0 << std::setw(13) << std::setprecision(7) << 0.0
           << "      GPS" << " "
           << "TIME OF FIRST OBS" << std::endl;

    m_file << std::left << std::setw(60) << "" << "END OF HEADER" << std::endl;
}


void RinexWriter::processEpoch(double epoch_time, const RinexEpoch& epoch) {

    if (!m_file.is_open() || epoch.obs.empty()) {
        return;
    }

    time_t time_sec_part = static_cast<time_t>(epoch_time);
    std::tm tm_utc;
    double fractional_seconds = epoch_time - time_sec_part;

    #if defined(_WIN32)
        gmtime_s(&tm_utc, &time_sec_part);
    #elif defined(__linux__)
        gmtime_r(&time_sec_part, &tm_utc);
    #else
        #pragma message("Warning: Using non-thread-safe gmtime() on an unsupported OS.")
        std::tm* tm_ptr = std::gmtime(&time_sec_part);
        if (tm_ptr) {
            tm_utc = *tm_ptr;
        }
    #endif

    m_file << "> " << std::setw(4) << tm_utc.tm_year + 1900
           << std::setw(3) << tm_utc.tm_mon + 1
           << std::setw(3) << tm_utc.tm_mday
           << std::setw(3) << tm_utc.tm_hour
           << std::setw(3) << tm_utc.tm_min
           << std::setw(11) << std::setprecision(7) << std::fixed << (tm_utc.tm_sec + fractional_seconds)
           << "  0"
           << std::setw(3) << epoch.obs.size() << std::endl;

    for (const auto& obs : epoch.obs) {
        m_file << obs.system << std::setw(2) << std::setfill('0') << obs.sat_id << std::setfill(' ');
        
        // C1 (Псевдодальность)
        m_file << std::right << std::setw(14) << std::setprecision(3) << std::fixed << obs.c1 << "  ";
        // L1 (Фаза несущей)
        m_file << std::right << std::setw(14) << std::setprecision(3) << std::fixed << obs.l1 << "  ";
        // D1 (Доплер)
        m_file << std::right << std::setw(14) << std::setprecision(3) << std::fixed << obs.d1 << "  ";
        // S1 (Отношение сигнал/шум)
        m_file << std::right << std::setw(14) << std::setprecision(3) << std::fixed << obs.s1 << "  ";
        
        m_file << std::endl;
    }
}