#include <boost/thread/thread.hpp>

#include "Client.hpp"
#include "Server.hpp"
#include "RingBuffer.hpp"
#include "ConfigParser.hpp"
#include "Common.hpp"
#include "Logger.hpp"

int main(int argc, char* argv[]) {
    Logger::getInstance().init();
    ConfigParser config;

    if (!config.parse(argc, argv)) {
        return config.isHelpRequested() ? 0 : 1;
    }

    RingBuffer<ClientMessage> client_req_buffer(config.getBufferSize());
    std::vector<size_t> memory_sizes;
    const auto& client_configs = config.getClientConfigs();
    for (const auto& client_config : client_configs) {
        memory_sizes.push_back(client_config.addr_space_size);
    }

    RingBuffer<MemMoveReq> move_page_buffer(config.getBufferSize());
    ServerMemoryConfig server_config = config.getServerMemoryConfig();
    PolicyConfig policy_config = config.getPolicyConfig();
    Server server(client_req_buffer, move_page_buffer, memory_sizes, server_config, policy_config);

    std::vector<std::shared_ptr<Client>> clients;
    std::vector<boost::thread> client_threads;
    for (size_t i = 0; i < client_configs.size(); i++) {
        const auto& client_config = client_configs[i];
        auto client = std::make_shared<Client>(
            client_req_buffer,
            i,
            config.getMessageCount(),
            client_config.addr_space_size,
            client_config.pattern
        );
        clients.push_back(client);
        client_threads.emplace_back([client]() { client->run(); });
    }

    boost::thread server_thread(&Server::start, &server);

    for (auto& thread : client_threads) {
        thread.join();
    }
    server_thread.join();

    return 0;
}
