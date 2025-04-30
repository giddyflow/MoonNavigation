#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/extensions/permessage_deflate/enabled.hpp> // Включаем заголовок расширения сжатия
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <exception> // Для std::exception
#include <atomic>



using json = nlohmann::json;

// Определяем новую конфигурацию клиента, включающую сжатие permessage_deflate
struct compressed_config : public websocketpp::config::asio_client {
    // Подключаем типы из базовой конфигурации
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

    // Определяем список расширений
    struct extentions {
        typedef websocketpp::extensions::permessage_deflate::enabled<compressed_config> permessage_deflate;
    };
};

// Используем новую конфигурацию для клиента
typedef websocketpp::client<compressed_config> client;
// Обновляем тип указателя на сообщение
typedef client::message_ptr message_ptr;

class WebSocketClient {
public:
    WebSocketClient(const std::string& uri)
        : m_uri(uri), m_connected(false), m_authorized(false), m_stop(false), m_modelId(0),
        m_ping_interval_ms(30000), // Интервал пинга в миллисекундах
        m_ping_timeout_ms(m_ping_interval_ms * 2), // Таймаут ответа на пинг
        m_reconnect_delay_sec(5), // Задержка перед переподключением
        m_ping_stop(false)
    {
        m_client.init_asio(); // Инициализация Asio

        // Настройка логирования (можно раскомментировать для отладки)
        m_client.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect | websocketpp::log::alevel::app);
        m_client.set_error_channels(websocketpp::log::elevel::info | websocketpp::log::elevel::warn | websocketpp::log::elevel::rerror | websocketpp::log::elevel::fatal);
        //    m_client.clear_access_channels(websocketpp::log::alevel::all); // Отключить все логи доступа
            //m_client.clear_error_channels(websocketpp::log::elevel::all);  // Отключить все логи ошибок

            // Установка обработчиков событий WebSocket
        m_client.set_open_handler(bind(&WebSocketClient::on_open, this, std::placeholders::_1));
        m_client.set_message_handler(bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
        m_client.set_fail_handler(bind(&WebSocketClient::on_fail, this, std::placeholders::_1));
        m_client.set_close_handler(bind(&WebSocketClient::on_close, this, std::placeholders::_1));
        m_client.set_pong_handler(bind(&WebSocketClient::on_pong, this, std::placeholders::_1, std::placeholders::_2));
        m_client.set_pong_timeout_handler(bind(&WebSocketClient::on_pong_timeout, this, std::placeholders::_1, std::placeholders::_2));

        // Установка таймаута для пинга (библиотека сама будет отправлять пинги)
        m_client.set_pong_timeout(m_ping_timeout_ms);
    }

    ~WebSocketClient() {
        stop();
    }

    // Старт клиента с авторизацией
    void start(const std::string& login, const std::string& password) {
        m_login = login;
        m_password = password;
        m_stop = false;
        m_ping_stop = false; // Убедимся, что пинг разрешен при старте
        std::cout << "Starting WebSocket client..." << std::endl;
        // Запускаем основной цикл Asio и подключение в отдельном потоке
        m_thread = std::thread([this]() {
            connect(); // Начать первое подключение
            try {
                m_client.run(); // Запустить цикл обработки событий Asio
                std::cout << "ASIO run loop finished." << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Exception in ASIO run loop: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unknown exception in ASIO run loop." << std::endl;
            }
            });
        // Запускаем поток для пинга (если используем ручной пинг)
        // start_ping_timer_thread(); // Раскомментировать, если set_pong_timeout не используется или нужен доп. контроль
    }

    // Остановка клиента
    void stop() {
        m_stop = true;      // Сигнал для остановки переподключений
        m_ping_stop = true; // Сигнал для остановки потока пинга (если используется)

        // Закрываем соединение, если оно активно
        websocketpp::lib::error_code ec;
        if (m_connected) {
            std::cout << "Closing WebSocket connection..." << std::endl;
            m_client.close(m_hdl, websocketpp::close::status::going_away, "Client shutdown", ec);
            if (ec) {
                std::cerr << "Error closing connection: " << ec.message() << std::endl;
            }
        }

        // Остановка ASIO (мягкая)
        std::cout << "Stopping ASIO loop..." << std::endl;
        m_client.stop_perpetual(); // Прекратить ожидание новых событий

        // Ожидание завершения потоков
        if (m_thread.joinable()) {
            std::cout << "Joining ASIO thread..." << std::endl;
            m_thread.join();
            std::cout << "ASIO thread joined." << std::endl;
        }
        // if (m_ping_thread.joinable()) { // Если используется поток пинга
        //     std::cout << "Joining ping thread..." << std::endl;
        //     m_ping_thread.join();
        //     std::cout << "Ping thread joined." << std::endl;
        // }
        std::cout << "WebSocket client stopped." << std::endl;
    }

    // Отправка сообщения (если соединение активно и клиент авторизован)
    void send(const std::string& message) {
        if (!m_connected || !m_authorized) {
            if (!m_connected) std::cerr << "Message not sent: Not connected." << std::endl;
            // Не выводим ошибку, если не авторизованы, но пытаемся отправить логин
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
        std::cout << "Sending message (size: " << message.size() << " bytes)..." << std::endl; // Лог отправки
        m_client.send(m_hdl, message, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cerr << "Send error: " << ec.message() << std::endl;
            // Здесь можно инициировать проверку соединения или переподключение, если ошибка критична
            // Например, если ec == websocketpp::error::bad_connection
        }
        else {
            std::cout << "Message sent successfully." << std::endl;
        }
    }

private:
    // Устанавливаем подключение к серверу
    void connect() {
        if (m_stop || m_connected) return; // Не подключаться, если остановлено или уже подключено

        std::cout << "Attempting to connect to " << m_uri << "..." << std::endl;
        websocketpp::lib::error_code ec;
        client::connection_ptr con = m_client.get_connection(m_uri, ec);
        if (ec) {
            std::cerr << "Connect initialization error: " << ec.message() << std::endl;
            schedule_reconnect(); // Попробовать снова позже
            return;
        }

        // Сохраняем хендл соединения для дальнейшего использования
        m_hdl = con->get_handle();

        // Запускаем попытку подключения
        m_client.connect(con);
    }

    // Планирование переподключения через заданный интервал
    void schedule_reconnect() {
        if (m_stop) return; // Не переподключаться, если клиент остановлен

        m_connected = false; // Сбросить флаги состояния
        m_authorized = false;

        std::cout << "Scheduling reconnect in " << m_reconnect_delay_sec << " seconds..." << std::endl;

        // Используем таймер Asio для отложенного вызова connect()
        auto timer = std::make_shared<websocketpp::lib::asio::steady_timer>(
            m_client.get_io_service(), std::chrono::seconds(m_reconnect_delay_sec)
        );
        timer->async_wait([this, timer](const websocketpp::lib::error_code& ec) {
            if (m_stop) return; // Проверка перед попыткой подключения
            if (!ec) {
                std::cout << "Reconnecting now..." << std::endl;
                connect(); // Попытка переподключения
            }
            else {
                std::cerr << "Reconnect timer error: " << ec.message() << std::endl;
                if (!m_stop) schedule_reconnect(); // Попробовать запланировать снова, если не остановлено
            }
            });
    }

    // Обработчик события: соединение успешно установлено
    void on_open(websocketpp::connection_hdl hdl) {
        m_connected = true;
        m_authorized = false; // Сброс авторизации при новом подключении
        std::cout << "WebSocket connection established." << std::endl;

        // Отправляем запрос на авторизацию
        json login_request = {
            {"action", "login"},
            {"login", m_login},
            {"password", m_password}
        };
        std::cout << "Sending login request..." << std::endl;
        send(login_request.dump());

        // Запускаем пинг-таймер, если используем ручной пинг
        // reset_ping_timer();
        // Библиотека сама будет отправлять пинги из-за set_pong_timeout
    }

    // Обработчик события: не удалось установить соединение
    void on_fail(websocketpp::connection_hdl hdl) {
        client::connection_ptr con = m_client.get_con_from_hdl(hdl);
        std::cerr << "Connection failed. Reason: " << con->get_ec().message() << std::endl;
        m_connected = false;
        m_authorized = false;
        schedule_reconnect(); // Попробовать переподключиться
    }

    // Обработчик события: соединение закрыто
    void on_close(websocketpp::connection_hdl hdl) {
        client::connection_ptr con = m_client.get_con_from_hdl(hdl);
        std::cout << "Connection closed. Code: " << con->get_remote_close_code()
            << ", Reason: " << con->get_remote_close_reason() << std::endl;
        m_connected = false;
        m_authorized = false;
        // stop_ping_timer_thread(); // Остановить поток пинга (если используется)
        schedule_reconnect(); // Попробовать переподключиться
    }


    // Обработка входящих сообщений
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
        std::cout << "Received message: " << msg->get_payload() << std::endl; // Лог полученного сообщения
        try {
            json message = json::parse(msg->get_payload());
            if (!message.contains("action")) {
                std::cerr << "Received message without 'action' field." << std::endl;
                return;
            }

            std::string action = message["action"];
            std::cout << "Action received: " << action << std::endl; // Лог действия

            if (action == "login") {
                handle_login_response(message);
            }
            else if (action == "processData") {
                if (!m_authorized) {
                    std::cerr << "Received 'processData' but client is not authorized. Ignoring." << std::endl;
                    return;
                }
                // Запускаем обработку в отдельном потоке, чтобы не блокировать Asio
                std::cout << "Received 'processData', starting processing thread..." << std::endl;
                std::thread processingThread([this, message_copy = std::move(message)]() {
                    // Лямбда захватывает 'this' и ПЕРЕМЕЩАЕТ 'message' в свою копию 'message_copy'
                    // Это гарантирует, что у потока будет своя валидная копия JSON
                    this->process_data_thread(message_copy); // Вызываем функцию-член с этой копией
                    });
                processingThread.detach(); // Отсоединяем поток, он будет работать независимо

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

    // Обработка ответа на логин
    void handle_login_response(const json& message) {
        if (message.value("status", false)) {
            m_apikey = message.value("apikey", "");
            m_authorized = true;
            std::cout << "Login successful. API Key received." << std::endl;
            // std::cout << "API Key: " << m_apikey << std::endl; // Раскомментировать для вывода ключа
        }
        else {
            m_authorized = false;
            std::cerr << "Login failed: " << message.value("message", "Unknown reason") << std::endl;
            // Возможно, стоит остановить клиент или запланировать переподключение с другими кредами,
            // но пока просто остаемся подключенными, но неавторизованными.
            schedule_reconnect(); // Или закрыть соединение и остановить? Зависит от логики.
        }
    }


    // Отправка промежуточного статуса обработки
    void send_intermediate_status(const std::string& state, const float percentage) {
        if (!m_connected || !m_authorized) return; // Не отправлять статус, если не подключены/авторизованы
        try {
            json status_message = {
                {"action", "intermediateStatus"},
                {"apikey", m_apikey}, // API ключ не нужен, сервер знает кто мы по соединению
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
            // Используем .store() для atomic переменной
            m_modelId.store(current_model_id);

            std::cout << "Processing thread started for modelId: " << current_model_id << std::endl;

            if (!message.contains("data") || !message["data"].is_object()) {
                std::cerr << "Processing error: 'data' is missing or not an object for modelId: " << current_model_id << std::endl;
                send_intermediate_status("error: 'data' is missing or not an object", 0.0f);
                return;
            }
            const json& data = message["data"];

            // --- ИЗМЕНЕНИЯ: Парсим строки JSON ---
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
            // --- КОНЕЦ ИЗМЕНЕНИЙ ---

            json nkaeci = json::array(); // Контейнер для выходных данных

            send_intermediate_status("started", 0.0f);

            std::cout << "Calling external process function for modelId: " << current_model_id << "..." << std::endl;
            // Передаем РАСПАРСЕННЫЕ объекты в process
            // Все они передаются по неконстантной ссылке (json&), как требует новая сигнатура process
            process(nka_parsed, nap_parsed, interference_parsed, controller_parsed, nkaeci,
                [this](const std::string& state, const float percentage) {
                    // Используем .load() для чтения atomic m_modelId
                    send_intermediate_status(state, percentage);
                }
            );
            std::cout << "External process function finished for modelId: " << current_model_id << "." << std::endl;

            send_intermediate_status("done", 1.0f);

            // Формируем и отправляем итоговое сообщение с результатами
            // Отправляем обратно *возможно измененные* объекты
            json result_message = {
                {"action", "result"},
                {"modelId", current_model_id},
                {"nka", nka_parsed},
                {"nkaeci", nkaeci},
                {"nap", nap_parsed},
                {"interference", interference_parsed}
                // controller обычно не отправляем обратно
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
        catch (const json::exception& e) { // Перехватываем общие ошибки nlohmann::json
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
        // Используем .store() для atomic
        m_modelId.store(-1);
    }


    // --- Механизм Ping/Pong (Управляется библиотекой через set_pong_timeout) ---

    // Обработчик получения Pong от сервера
    void on_pong(websocketpp::connection_hdl hdl, std::string payload) {
        // std::cout << "Pong received from server." << std::endl;
        // Сбрасываем таймер ожидания Pong (библиотека делает это автоматически)
        m_last_pong_time = std::chrono::steady_clock::now(); // Можно обновлять время для своей логики, если нужно
    }

    // Обработчик таймаута ожидания Pong
    void on_pong_timeout(websocketpp::connection_hdl hdl, std::string payload) {
        std::cerr << "Pong timeout! Server didn't respond to ping." << std::endl;
        // Соединение считается потерянным. Библиотека обычно сама инициирует закрытие.
        // Можно добавить дополнительную логику здесь, но обычно достаточно on_close/on_fail.
        // Закрываем соединение явно, если библиотека этого не сделала (маловероятно)
        if (m_connected) {
            websocketpp::lib::error_code ec;
            std::cerr << "Closing connection due to pong timeout." << std::endl;
            m_client.close(hdl, websocketpp::close::status::policy_violation, "Pong timeout", ec);
            // on_close будет вызван после этого
        }
        else {
            // Если уже не подключены, просто пытаемся переподключиться
            schedule_reconnect();
        }
    }


    // --- Поля класса ---
    std::string m_uri;        // Адрес WebSocket сервера
    client m_client;          // Экземпляр клиента websocketpp
    websocketpp::connection_hdl m_hdl; // Хендл текущего соединения
    std::thread m_thread;     // Поток для основного цикла Asio

    std::string m_login;      // Логин для авторизации
    std::string m_password;   // Пароль для авторизации
    std::string m_apikey;     // Полученный API ключ
    std::atomic<int> m_modelId;

    std::atomic<bool> m_connected;  // Флаг: активно ли соединение
    std::atomic<bool> m_authorized; // Флаг: авторизован ли клиент
    std::atomic<bool> m_stop;       // Флаг: запрошена ли остановка клиента

    // Параметры Ping/Pong и переподключения
    long m_ping_interval_ms;
    long m_ping_timeout_ms;
    long m_reconnect_delay_sec;

    // Для ручного пинга (если не используется set_pong_timeout)
    std::chrono::steady_clock::time_point m_last_pong_time;
    // std::thread m_ping_thread; // Поток для отправки пингов
    std::atomic<bool> m_ping_stop;  // Флаг для остановки потока пинга
};

// --- Точка входа ---
int main() {
    // --- НАСТРОЙКИ ---
    const std::string uri = "ws://194.58.96.141:9072"; // Используй порт WS (9071)
    const std::string login = "user2"; // Твой логин
    const std::string password = "DwfL~tfY#%1{"; // Твой пароль
    // --- КОНЕЦ НАСТРОЕК ---

    try {
        WebSocketClient ws_client(uri);
        ws_client.start(login, password);

        // Держим программу активной (например, 24 часа)
        std::cout << "Client started. Running for 700 hours (or until interrupted)..." << std::endl;
        std::this_thread::sleep_for(std::chrono::hours(700)); // Измените время по необходимости

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
