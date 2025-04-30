#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/extensions/permessage_deflate/enabled.hpp> // �������� ��������� ���������� ������
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <exception> // ��� std::exception
#include <atomic>



using json = nlohmann::json;

// ���������� ����� ������������ �������, ���������� ������ permessage_deflate
struct compressed_config : public websocketpp::config::asio_client {
    // ���������� ���� �� ������� ������������
    typedef websocketpp::config::asio_client base;

    typedef base::concurrency_type concurrency_type;
    typedef base::request_type request_type;
    typedef base::response_type response_type;
    typedef base::message_type message_type;
    typedef base::con_msg_manager_type con_msg_manager_type;
    typedef base::endpoint_msg_manager_type endpoint_msg_manager_type;
    typedef base::alog_type alog_type;
    typedef base::elog_type elog_type;
    typedef base::rng_type rng_type;
    typedef base::transport_type transport_type;
    typedef base::endpoint_base endpoint_base;

    // ���������� ������ ����������
    struct extentions {
        typedef websocketpp::extensions::permessage_deflate::enabled<compressed_config> permessage_deflate;
    };
};

// ���������� ����� ������������ ��� �������
typedef websocketpp::client<compressed_config> client;
// ��������� ��� ��������� �� ���������
typedef client::message_ptr message_ptr;

class WebSocketClient {
public:
    WebSocketClient(const std::string& uri)
        : m_uri(uri), m_connected(false), m_authorized(false), m_stop(false), m_modelId(0),
        m_ping_interval_ms(30000), // �������� ����� � �������������
        m_ping_timeout_ms(m_ping_interval_ms * 2), // ������� ������ �� ����
        m_reconnect_delay_sec(5), // �������� ����� ����������������
        m_ping_stop(false)
    {
        m_client.init_asio(); // ������������� Asio

        // ��������� ����������� (����� ����������������� ��� �������)
        m_client.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect | websocketpp::log::alevel::app);
        m_client.set_error_channels(websocketpp::log::elevel::info | websocketpp::log::elevel::warn | websocketpp::log::elevel::rerror | websocketpp::log::elevel::fatal);
        //    m_client.clear_access_channels(websocketpp::log::alevel::all); // ��������� ��� ���� �������
            //m_client.clear_error_channels(websocketpp::log::elevel::all);  // ��������� ��� ���� ������

            // ��������� ������������ ������� WebSocket
        m_client.set_open_handler(bind(&WebSocketClient::on_open, this, std::placeholders::_1));
        m_client.set_message_handler(bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
        m_client.set_fail_handler(bind(&WebSocketClient::on_fail, this, std::placeholders::_1));
        m_client.set_close_handler(bind(&WebSocketClient::on_close, this, std::placeholders::_1));
        m_client.set_pong_handler(bind(&WebSocketClient::on_pong, this, std::placeholders::_1, std::placeholders::_2));
        m_client.set_pong_timeout_handler(bind(&WebSocketClient::on_pong_timeout, this, std::placeholders::_1, std::placeholders::_2));

        // ��������� �������� ��� ����� (���������� ���� ����� ���������� �����)
        m_client.set_pong_timeout(m_ping_timeout_ms);
    }

    ~WebSocketClient() {
        stop();
    }

    // ����� ������� � ������������
    void start(const std::string& login, const std::string& password) {
        m_login = login;
        m_password = password;
        m_stop = false;
        m_ping_stop = false; // ��������, ��� ���� �������� ��� ������
        std::cout << "Starting WebSocket client..." << std::endl;
        // ��������� �������� ���� Asio � ����������� � ��������� ������
        m_thread = std::thread([this]() {
            connect(); // ������ ������ �����������
            try {
                m_client.run(); // ��������� ���� ��������� ������� Asio
                std::cout << "ASIO run loop finished." << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Exception in ASIO run loop: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unknown exception in ASIO run loop." << std::endl;
            }
            });
        // ��������� ����� ��� ����� (���� ���������� ������ ����)
        // start_ping_timer_thread(); // �����������������, ���� set_pong_timeout �� ������������ ��� ����� ���. ��������
    }

    // ��������� �������
    void stop() {
        m_stop = true;      // ������ ��� ��������� ���������������
        m_ping_stop = true; // ������ ��� ��������� ������ ����� (���� ������������)

        // ��������� ����������, ���� ��� �������
        websocketpp::lib::error_code ec;
        if (m_connected) {
            std::cout << "Closing WebSocket connection..." << std::endl;
            m_client.close(m_hdl, websocketpp::close::status::going_away, "Client shutdown", ec);
            if (ec) {
                std::cerr << "Error closing connection: " << ec.message() << std::endl;
            }
        }

        // ��������� ASIO (������)
        std::cout << "Stopping ASIO loop..." << std::endl;
        m_client.stop_perpetual(); // ���������� �������� ����� �������

        // �������� ���������� �������
        if (m_thread.joinable()) {
            std::cout << "Joining ASIO thread..." << std::endl;
            m_thread.join();
            std::cout << "ASIO thread joined." << std::endl;
        }
        // if (m_ping_thread.joinable()) { // ���� ������������ ����� �����
        //     std::cout << "Joining ping thread..." << std::endl;
        //     m_ping_thread.join();
        //     std::cout << "Ping thread joined." << std::endl;
        // }
        std::cout << "WebSocket client stopped." << std::endl;
    }

    // �������� ��������� (���� ���������� ������� � ������ �����������)
    void send(const std::string& message) {
        if (!m_connected || !m_authorized) {
            if (!m_connected) std::cerr << "Message not sent: Not connected." << std::endl;
            // �� ������� ������, ���� �� ������������, �� �������� ��������� �����
            else if (!m_authorized) {
                try {
                    json j = json::parse(message);
                    if (j.value("action", "") != "login") {
                        std::cerr << "Message not sent: Not authorized." << std::endl;
                    }
                }
                catch (...) {/* ignore parse error */ }
            }
            if (!m_connected || (m_connected && !m_authorized && json::parse(message).value("action", "") != "login")) return;
        }

        websocketpp::lib::error_code ec;
        std::cout << "Sending message (size: " << message.size() << " bytes)..." << std::endl; // ��� ��������
        m_client.send(m_hdl, message, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cerr << "Send error: " << ec.message() << std::endl;
            // ����� ����� ������������ �������� ���������� ��� ���������������, ���� ������ ��������
            // ��������, ���� ec == websocketpp::error::bad_connection
        }
        else {
            std::cout << "Message sent successfully." << std::endl;
        }
    }

private:
    // ������������� ����������� � �������
    void connect() {
        if (m_stop || m_connected) return; // �� ������������, ���� ����������� ��� ��� ����������

        std::cout << "Attempting to connect to " << m_uri << "..." << std::endl;
        websocketpp::lib::error_code ec;
        client::connection_ptr con = m_client.get_connection(m_uri, ec);
        if (ec) {
            std::cerr << "Connect initialization error: " << ec.message() << std::endl;
            schedule_reconnect(); // ����������� ����� �����
            return;
        }

        // ��������� ����� ���������� ��� ����������� �������������
        m_hdl = con->get_handle();

        // ��������� ������� �����������
        m_client.connect(con);
    }

    // ������������ ��������������� ����� �������� ��������
    void schedule_reconnect() {
        if (m_stop) return; // �� ����������������, ���� ������ ����������

        m_connected = false; // �������� ����� ���������
        m_authorized = false;

        std::cout << "Scheduling reconnect in " << m_reconnect_delay_sec << " seconds..." << std::endl;

        // ���������� ������ Asio ��� ����������� ������ connect()
        auto timer = std::make_shared<websocketpp::lib::asio::steady_timer>(
            m_client.get_io_service(), std::chrono::seconds(m_reconnect_delay_sec)
        );
        timer->async_wait([this, timer](const websocketpp::lib::error_code& ec) {
            if (m_stop) return; // �������� ����� �������� �����������
            if (!ec) {
                std::cout << "Reconnecting now..." << std::endl;
                connect(); // ������� ���������������
            }
            else {
                std::cerr << "Reconnect timer error: " << ec.message() << std::endl;
                if (!m_stop) schedule_reconnect(); // ����������� ������������� �����, ���� �� �����������
            }
            });
    }

    // ���������� �������: ���������� ������� �����������
    void on_open(websocketpp::connection_hdl hdl) {
        m_connected = true;
        m_authorized = false; // ����� ����������� ��� ����� �����������
        std::cout << "WebSocket connection established." << std::endl;

        // ���������� ������ �� �����������
        json login_request = {
            {"action", "login"},
            {"login", m_login},
            {"password", m_password}
        };
        std::cout << "Sending login request..." << std::endl;
        send(login_request.dump());

        // ��������� ����-������, ���� ���������� ������ ����
        // reset_ping_timer();
        // ���������� ���� ����� ���������� ����� ��-�� set_pong_timeout
    }

    // ���������� �������: �� ������� ���������� ����������
    void on_fail(websocketpp::connection_hdl hdl) {
        client::connection_ptr con = m_client.get_con_from_hdl(hdl);
        std::cerr << "Connection failed. Reason: " << con->get_ec().message() << std::endl;
        m_connected = false;
        m_authorized = false;
        schedule_reconnect(); // ����������� ����������������
    }

    // ���������� �������: ���������� �������
    void on_close(websocketpp::connection_hdl hdl) {
        client::connection_ptr con = m_client.get_con_from_hdl(hdl);
        std::cout << "Connection closed. Code: " << con->get_remote_close_code()
            << ", Reason: " << con->get_remote_close_reason() << std::endl;
        m_connected = false;
        m_authorized = false;
        // stop_ping_timer_thread(); // ���������� ����� ����� (���� ������������)
        schedule_reconnect(); // ����������� ����������������
    }


    // ��������� �������� ���������
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
        std::cout << "Received message: " << msg->get_payload() << std::endl; // ��� ����������� ���������
        try {
            json message = json::parse(msg->get_payload());
            if (!message.contains("action")) {
                std::cerr << "Received message without 'action' field." << std::endl;
                return;
            }

            std::string action = message["action"];
            std::cout << "Action received: " << action << std::endl; // ��� ��������

            if (action == "login") {
                handle_login_response(message);
            }
            else if (action == "processData") {
                if (!m_authorized) {
                    std::cerr << "Received 'processData' but client is not authorized. Ignoring." << std::endl;
                    return;
                }
                // ��������� ��������� � ��������� ������, ����� �� ����������� Asio
                std::cout << "Received 'processData', starting processing thread..." << std::endl;
                std::thread processingThread([this, message_copy = std::move(message)]() {
                    // ������ ����������� 'this' � ���������� 'message' � ���� ����� 'message_copy'
                    // ��� �����������, ��� � ������ ����� ���� �������� ����� JSON
                    this->process_data_thread(message_copy); // �������� �������-���� � ���� ������
                    });
                processingThread.detach(); // ����������� �����, �� ����� �������� ����������

            }
            else if (action == "info") {
                std::cout << "[Server Info]: " << message.value("message", "") << std::endl;
            }
            else {
                std::cerr << "Unknown action received: " << action << std::endl;
            }
        }
        catch (const json::parse_error& e) {
            std::cerr << "Error parsing received JSON: " << e.what() << std::endl;
            std::cerr << "Received payload: " << msg->get_payload() << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error processing message: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown error processing message." << std::endl;
        }
    }

    // ��������� ������ �� �����
    void handle_login_response(const json& message) {
        if (message.value("status", false)) {
            m_apikey = message.value("apikey", "");
            m_authorized = true;
            std::cout << "Login successful. API Key received." << std::endl;
            // std::cout << "API Key: " << m_apikey << std::endl; // ����������������� ��� ������ �����
        }
        else {
            m_authorized = false;
            std::cerr << "Login failed: " << message.value("message", "Unknown reason") << std::endl;
            // ��������, ����� ���������� ������ ��� ������������� ��������������� � ������� �������,
            // �� ���� ������ �������� �������������, �� �����������������.
            schedule_reconnect(); // ��� ������� ���������� � ����������? ������� �� ������.
        }
    }


    // �������� �������������� ������� ���������
    void send_intermediate_status(const std::string& state, const float percentage) {
        if (!m_connected || !m_authorized) return; // �� ���������� ������, ���� �� ����������/������������
        try {
            json status_message = {
                {"action", "intermediateStatus"},
                {"apikey", m_apikey}, // API ���� �� �����, ������ ����� ��� �� �� ����������
                {"modelId", m_modelId.load()},
                {"state", state},
                {"simulation_progress", percentage}
            };
            std::cout << "Sending intermediate status: " << status_message.dump() << std::endl;
            send(status_message.dump());
        }
        catch (const std::exception& e) {
            std::cerr << "Error creating intermediate status JSON: " << e.what() << std::endl;
        }
    }

    void process_data_thread(const json& message) {
        int current_model_id = -1;
        try {
            if (!message.contains("modelId")) {
                std::cerr << "Processing error: 'modelId' is missing in 'processData' message." << std::endl;
                return;
            }
            current_model_id = message["modelId"];
            // ���������� .store() ��� atomic ����������
            m_modelId.store(current_model_id);

            std::cout << "Processing thread started for modelId: " << current_model_id << std::endl;

            if (!message.contains("data") || !message["data"].is_object()) {
                std::cerr << "Processing error: 'data' is missing or not an object for modelId: " << current_model_id << std::endl;
                send_intermediate_status("error: 'data' is missing or not an object", 0.0f);
                return;
            }
            const json& data = message["data"];

            // --- ���������: ������ ������ JSON ---
            json nka_parsed, nap_parsed, interference_parsed, controller_parsed;

            try {
                if (!data.contains("nka") || !data["nka"].is_string()) { send_intermediate_status("error: 'data.nka' is missing or not a string", 0.0f); return; }
                nka_parsed = json::parse(data["nka"].get<std::string>());

                if (!data.contains("nap") || !data["nap"].is_string()) { send_intermediate_status("error: 'data.nap' is missing or not a string", 0.0f); return; }
                nap_parsed = json::parse(data["nap"].get<std::string>());

                if (!data.contains("interference") || !data["interference"].is_string()) { send_intermediate_status("error: 'data.interference' is missing or not a string", 0.0f); return; }
                interference_parsed = json::parse(data["interference"].get<std::string>());

                if (!data.contains("controller") || !data["controller"].is_string()) { send_intermediate_status("error: 'data.controller' is missing or not a string", 0.0f); return; }
                controller_parsed = json::parse(data["controller"].get<std::string>());

            }
            catch (const json::parse_error& pe) {
                std::cerr << "JSON parsing error for inner data string for modelId " << current_model_id << ": " << pe.what() << std::endl;
                send_intermediate_status("error: Failed to parse inner JSON data - " + std::string(pe.what()), 0.0f);
                return;
            }
            // --- ����� ��������� ---

            json nkaeci = json::array(); // ��������� ��� �������� ������

            send_intermediate_status("started", 0.0f);

            std::cout << "Calling external process function for modelId: " << current_model_id << "..." << std::endl;
            // �������� ������������ ������� � process
            // ��� ��� ���������� �� ������������� ������ (json&), ��� ������� ����� ��������� process
            process(nka_parsed, nap_parsed, interference_parsed, controller_parsed, nkaeci,
                [this](const std::string& state, const float percentage) {
                    // ���������� .load() ��� ������ atomic m_modelId
                    send_intermediate_status(state, percentage);
                }
            );
            std::cout << "External process function finished for modelId: " << current_model_id << "." << std::endl;

            send_intermediate_status("done", 1.0f);

            // ��������� � ���������� �������� ��������� � ������������
            // ���������� ������� *�������� ����������* �������
            json result_message = {
                {"action", "result"},
                {"modelId", current_model_id},
                {"nka", nka_parsed},
                {"nkaeci", nkaeci},
                {"nap", nap_parsed},
                {"interference", interference_parsed}
                // controller ������ �� ���������� �������
            };
            // Serialize the result message ONCE
            std::string result_payload = result_message.dump();

            // --- ADDED LOGGING ---
            std::cout << "Sending final result message (size: " << result_payload.size() << " bytes) for modelId: " << current_model_id << "..." << std::endl;
            // --- END ADDED LOGGING ---

            // Send the final result payload
            send(result_payload);
            std::cout << "Final result sent for modelId: " << current_model_id << "." << std::endl;
        }
        catch (const json::exception& e) { // ������������� ����� ������ nlohmann::json
            std::cerr << "JSON error during processing data for modelId " << current_model_id << ": " << e.what() << std::endl;
            if (current_model_id != -1) {
                send_intermediate_status("error: JSON processing failed - " + std::string(e.what()), 0.0f);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Standard exception during processing data for modelId " << current_model_id << ": " << e.what() << std::endl;
            if (current_model_id != -1) {
                send_intermediate_status("error: Processing failed - " + std::string(e.what()), 0.0f);
            }
        }
        catch (...) {
            std::cerr << "Unknown exception during processing data for modelId " << current_model_id << "." << std::endl;
            if (current_model_id != -1) {
                send_intermediate_status("error: Unknown processing failure", 0.0f);
            }
        }
        std::cout << "Processing thread finished for modelId: " << current_model_id << "." << std::endl;
        // ���������� .store() ��� atomic
        m_modelId.store(-1);
    }


    // --- �������� Ping/Pong (����������� ����������� ����� set_pong_timeout) ---

    // ���������� ��������� Pong �� �������
    void on_pong(websocketpp::connection_hdl hdl, std::string payload) {
        // std::cout << "Pong received from server." << std::endl;
        // ���������� ������ �������� Pong (���������� ������ ��� �������������)
        m_last_pong_time = std::chrono::steady_clock::now(); // ����� ��������� ����� ��� ����� ������, ���� �����
    }

    // ���������� �������� �������� Pong
    void on_pong_timeout(websocketpp::connection_hdl hdl, std::string payload) {
        std::cerr << "Pong timeout! Server didn't respond to ping." << std::endl;
        // ���������� ��������� ����������. ���������� ������ ���� ���������� ��������.
        // ����� �������� �������������� ������ �����, �� ������ ���������� on_close/on_fail.
        // ��������� ���������� ����, ���� ���������� ����� �� ������� (������������)
        if (m_connected) {
            websocketpp::lib::error_code ec;
            std::cerr << "Closing connection due to pong timeout." << std::endl;
            m_client.close(hdl, websocketpp::close::status::policy_violation, "Pong timeout", ec);
            // on_close ����� ������ ����� �����
        }
        else {
            // ���� ��� �� ����������, ������ �������� ����������������
            schedule_reconnect();
        }
    }


    // --- ���� ������ ---
    std::string m_uri;        // ����� WebSocket �������
    client m_client;          // ��������� ������� websocketpp
    websocketpp::connection_hdl m_hdl; // ����� �������� ����������
    std::thread m_thread;     // ����� ��� ��������� ����� Asio

    std::string m_login;      // ����� ��� �����������
    std::string m_password;   // ������ ��� �����������
    std::string m_apikey;     // ���������� API ����
    std::atomic<int> m_modelId;

    std::atomic<bool> m_connected;  // ����: ������� �� ����������
    std::atomic<bool> m_authorized; // ����: ����������� �� ������
    std::atomic<bool> m_stop;       // ����: ��������� �� ��������� �������

    // ��������� Ping/Pong � ���������������
    long m_ping_interval_ms;
    long m_ping_timeout_ms;
    long m_reconnect_delay_sec;

    // ��� ������� ����� (���� �� ������������ set_pong_timeout)
    std::chrono::steady_clock::time_point m_last_pong_time;
    // std::thread m_ping_thread; // ����� ��� �������� ������
    std::atomic<bool> m_ping_stop;  // ���� ��� ��������� ������ �����
};

// --- ����� ����� ---
int main() {
    // --- ��������� ---
    const std::string uri = "ws://194.58.96.141:9072"; // ��������� ���� WS (9071)
    const std::string login = "user2"; // ���� �����
    const std::string password = "DwfL~tfY#%1{"; // ���� ������
    // --- ����� �������� ---

    try {
        WebSocketClient ws_client(uri);
        ws_client.start(login, password);

        // ������ ��������� �������� (��������, 24 ����)
        std::cout << "Client started. Running for 700 hours (or until interrupted)..." << std::endl;
        std::this_thread::sleep_for(std::chrono::hours(700)); // �������� ����� �� �������������

        std::cout << "Stopping client after timeout..." << std::endl;
        ws_client.stop();
        std::cout << "Client finished." << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception in main: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unhandled unknown exception in main." << std::endl;
        return 1;
    }

    return 0;
}
