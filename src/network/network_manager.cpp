#include "network_manager.h"
#include <iostream>

NetworkManager::NetworkManager() : host(nullptr), peer(nullptr), isServer(false) 
{
    if (enet_initialize() != 0) {
        assert(0);
    }
}

NetworkManager::~NetworkManager()
{
    Shutdown();
    enet_deinitialize();
}

bool NetworkManager::InitializeServer(int& port) 
{
    Shutdown();
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, 1, 1, 0, 0);
    int counter = 0;
    while (!host && counter < 200) 
    {
        address.port++;
        host = enet_host_create(&address, 1, 1, 0, 0);
        counter++;
    }
    port = address.port;

    if(!host)
    {
        return false;
    }
    
    isServer = true;

    return true;
}

bool NetworkManager::InitializeClient(const char* address, int port) 
{
    Shutdown();
    host = enet_host_create(nullptr, 1, 1, 0, 0);
    if (!host) {
        return false;
    }
    
    ENetAddress serverAddress;
    enet_address_set_host(&serverAddress, address);
    serverAddress.port = port;
    
    peer = enet_host_connect(host, &serverAddress, 1, 0);
    if (!peer) {
        enet_host_destroy(host);
        host = nullptr;
        return false;
    }
    
    isServer = false;

    return true;
}

void NetworkManager::Update()
{
    if (!host) return;

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
        {
            peer = event.peer;
            std::cout << "Connected." << std::endl;
            onConnectionChange(true);
            break;
        }

        case ENET_EVENT_TYPE_RECEIVE: 
        {
                void* receivedData = event.packet->data;
                size_t receivedDataSize = event.packet->dataLength;

                uint8_t type = *(reinterpret_cast<uint8_t*>(receivedData));

                switch (type)
                {
                    case (uint8_t(0)): 
                    {
                        GameStateMsg* msg = reinterpret_cast<GameStateMsg*>(receivedData);
                        if(receivedDataSize != sizeof(GameStateMsg))
                        {
                            std::cout << "\nGameStateMsg receiving error - receivedDataSize != sizeof(GameStateMsg)\n";
                            std::cout << " sizeof(GameStateMsg) = " << sizeof(GameStateMsg) << " receivedDataSize = " << receivedDataSize;
                        }
                        if (onGameStateReceive )
                        {
                            onGameStateReceive(msg);
                        }
                        else
                        {
                            std::cout << "\nGameStateMsg receiving error\n";
                        }
                        break;
                    }
                    case (uint8_t(1)):
                    {
                        StartGameMsg* msg = reinterpret_cast<StartGameMsg*>(receivedData);
                        if (onStartGameReceive && receivedDataSize == sizeof(StartGameMsg))
                        {
                            onStartGameReceive(msg);
                        }
                        else 
                        {
                            std::cout << "\nStartGameMsg receiving error\n";
                        }
                        break;
                    }
                    case (uint8_t(2)):
                    {
                        StopGameMsg* msg = reinterpret_cast<StopGameMsg*>(receivedData);
                        if (onStopGameReceive && receivedDataSize == sizeof(StopGameMsg))
                        {
                            onStopGameReceive(msg);
                        }
                        else
                        {
                            std::cout << "\nStopGameMsg receiving error\n";
                        }
                        break;
                    }
                    case (uint8_t(3)):
                    {

                        SnakeDirChangeMsg* msg = reinterpret_cast<SnakeDirChangeMsg*>(receivedData);
                        if (onSnakeDirChangeReceive && receivedDataSize == sizeof(SnakeDirChangeMsg))
                        {
                            onSnakeDirChangeReceive(msg);
                        }
                        else
                        {
                            std::cerr << "SnakeDirChangeMsg receiving error";
                        }
                        break;
                    }

                    default:
                    {
                        std::cerr << "Unknown message type received " <<static_cast<int>(type) << std::endl;
                        break;
                    }
                }
            enet_packet_destroy(event.packet);
            break;
        }

        case ENET_EVENT_TYPE_DISCONNECT:
        {
            peer = nullptr;
            std::cout << "Disconnected." << std::endl;
            if (onConnectionChange)
            {
                onConnectionChange(false);
            }
            break;
        }

        default:
            break;
        }
    }
}

void NetworkManager::Shutdown() 
{
    std::cout << "\nvoid NetworkManager::Shutdown() \n";
    if (peer) {
        enet_peer_disconnect_now(peer, 0);
        peer = nullptr;
    }
    if (host) {
        enet_host_destroy(host);
        host = nullptr;
    }
}

void NetworkManager::Disconnect()
{
    if (peer) {
        enet_peer_disconnect(peer, 0);
        peer = nullptr;
    }
}

void NetworkManager::sendStartGame(StartGameMsg* msg)
{
    if (!isServer)
    {
        std::cerr << "attempt to sendSnakeAddBody from client";
        return;
    }
    ENetPacket* packet = enet_packet_create(msg, sizeof(StartGameMsg), 0);
    if(!packet)
    {
        std::cout << "couldn't create packet StartGameMsg";
    }
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}

void NetworkManager::sendGameState(GameStateMsg* msg)
{
    if (!isServer)
    {
        std::cerr << "attempt to sendSnakeAddBody from client";
        return;
    }
    ENetPacket* packet = enet_packet_create(msg, sizeof(GameStateMsg), 0);
    if(!packet) 
    {
        std::cout << "error when packing StartGameMsg";
        return;
    }
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}

void NetworkManager::sendStopGame(StopGameMsg* msg)
{
    if (!isServer)
    {
        std::cerr << "attempt to sendStopGame from server";
        return;
    }
    ENetPacket* packet = enet_packet_create(msg, sizeof(StopGameMsg), 0);
    if (!packet)
    {
        std::cout << "error when packing StopGameMsg";
        return;
    }
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}

void NetworkManager::sendSnakeDirChange(SnakeDirChangeMsg* msg)
{
    if (isServer)
    {
        std::cerr << "attempt to sendSnakeDirChange from server";
        return;
    }

    ENetPacket* packet = enet_packet_create(msg, sizeof(SnakeDirChangeMsg), 0);
    if (!packet)
    {
        std::cout << "error when packing SnakeDirChangeMsg";
        return;
    }
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}


