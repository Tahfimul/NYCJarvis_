#ifndef NYC_JARVIS_SERVER_HPP_
#define NYC_JARVIS_SERVER_HPP_

#include "nyc_jarvis_base.hpp"
#include "nyc_jarvis_threadsafeQueue.hpp"
#include "nyc_jarvis_info.hpp"
#include "nyc_jarvis_connection.hpp"
#include "nyc_jarvis_custom_info_types.hpp"

namespace nyc
{
    namespace jarvis
    {


        template<typename T>
        class server_interface
        {
            public:
                server_interface(uint16_t port)
                    : m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port)), current_input_string("")
                {
                }

                virtual ~server_interface()
                {
                    Stop();
                }

                bool Start()
                {
                    try
                    {
                        WaitForClientConnection();
                        m_threadContext = std::thread([this]() {m_asioContext.run(); });
                    }
                    catch (std::exception& e)
                    {
                        std::cerr << "[SERVER] Exception: " << e.what() << std::endl;
                        return false;
                    }

                    std::cout << "[SERVER] Started\n";
                    return true;

                }

                void Stop()
                {
                    m_asioContext.stop();

                    if (m_threadContext.joinable()) m_threadContext.join();

                    std::cout << "[SERVER] Stopped" << std::endl;
                }

                void WaitForClientConnection()
                {
                    m_asioAcceptor.async_accept(
                        [this](std::error_code ec, asio::ip::tcp::socket socket)
                        {
                            if (!ec)
                            {
                                std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << std::endl;

                                std::shared_ptr<Connection<T>> newConnection =
                                    std::make_shared<Connection<T>>(Connection<T>::owner::server,
                                        m_asioContext, std::move(socket), m_qInfosIn);

                                if (OnClientConnect(newConnection))
                                {
                                    m_deqConnections.push_back(std::move(newConnection));

                                    m_deqConnections.back()->ConnectToClient(this, nIDCounter++);

                                    std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection Approved\n";
                                }
                                else
                                {
                                    std::cout << "[-----] Connection Denied\n";
                                }
                            }
                            else
                            {
                                std::cout << "[SERVER] New Connection Error: " << ec.message() << std::endl;
                            }

                            WaitForClientConnection();
                        });
                }

                void SendInfoToClient(std::shared_ptr<Connection<T>> client, const info<T>& info)
                {
                    if (client && client->IsConnected())
                    {
                        client->Send(info);
                    }
                    else
                    {
                        OnClientDisconnect(client);
                        client.reset();
                        m_deqConnections.erase(
                            std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end()
                        );
                    }
                }

                void SendInfoToAllClients(const info<T>& info, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr)
                {
                    bool bInvalidClientExists = false;
                    for (auto& client : m_deqConnections)
                    {
                        if (client && client->IsConnected())
                        {
                            if (client != pIgnoreClient)
                                client->Send(info);
                        }
                        else
                        {
                            OnClientDisconnect(client);
                            client.reset();
                            bInvalidClientExists = true;
                        }
                    }

                    if (bInvalidClientExists)
                        m_deqConnections.erase(
                            std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end()
                        );
                }

                void Update(size_t nMaxInfos = -1, bool bWait=false)
                {
                    // std::cout<<"update called\n";
                    if (bWait) m_qInfosIn.wait();
                    size_t nInfoCount = 0;

                    while (nInfoCount < nMaxInfos && !m_qInfosIn.empty())
                    {
                        // std::cout<<"m_qInfosIn is not empty\n";
                        auto info = m_qInfosIn.front();
                        if(info.i.header.id == CustomInfoTypes::I_S)
                        {
                            
                            if(current_input_string=="")
                            {
                                
                                int original_info_size = info.i.size();
                                current_input_string = new char[original_info_size];
                                for(int i=0; i<original_info_size; i++)
                                {
                                    info.i >> current_input_string[i];
                                }

                                current_input_string_client = info.remote;
                                m_qInfosIn.pop_front();
                                std::cout<<"current_input_string: "<<current_input_string<<std::endl;

                            }
                        }
                        else
                        {
                            m_qInfosIn.pop_front();
                            std::cout << "info.remote " << info.remote << " info.i " << info.i << std::endl;
                            OnInfo(info.remote, info.i);
                        }
                           
                       
                        nInfoCount++;
                    }
                }

                protected:
                    virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)
                    {
                        return false;
                    }

                    virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client)
                    {

                    }

                    virtual void OnInfo(std::shared_ptr<nyc::jarvis::Connection<T>> client, nyc::jarvis::info<T>& info)
                    {
                        std::cout << "net_server onInfo called\n";
                        
                    }

                    virtual void sendInputString(char* inputString)
                    {}

                    virtual void clearCurrentInputString(){}

                    virtual char* getCurrentInputString(){}
                public:

                    virtual void OnClientValidated(std::shared_ptr<Connection<T>> client)
                    {

                    }

                
                protected:
                    threadsafeQueue<owned_info<T>> m_qInfosIn;
                    std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;

                    asio::io_context m_asioContext;
                    std::thread m_threadContext;

                    asio::ip::tcp::acceptor m_asioAcceptor;
                    uint32_t nIDCounter = 10000;

                    char* current_input_string;
                    std::shared_ptr<Connection<T>> current_input_string_client;

        };
    }
}


#endif