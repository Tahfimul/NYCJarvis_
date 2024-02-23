#ifndef NYC_JARVIS_CONNECTION_HPP_
#define NYC_JARVIS_CONNECTION_HPP_

#include "nyc_jarvis_base.hpp"
#include "nyc_jarvis_threadsafeQueue.hpp"
#include "nyc_jarvis_info.hpp"


namespace nyc
{
    namespace jarvis
    {
        template<typename T>
        class server_interface;

        template<typename T>
        class Connection : public std::enable_shared_from_this<Connection<T>>
        {
            public:
                enum class owner
                {
                    server,
                    client
                };

                Connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, threadsafeQueue<owned_info<T>>& qIn)
                :m_asioContext(asioContext), m_socket(std::move(socket)), m_qInfosIn(qIn)
                {
                    m_nOwnerType = parent;

                    if(m_nOwnerType == owner::server)
                    {
                        m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

                        m_nHandshakeCheck = scramble(m_nHandshakeOut);
                    }
                    else if(m_nOwnerType==owner::client)
                    {
                        m_nHandshakeCheck = 0;
                        m_nHandshakeOut = 0;
                    }
                }

                virtual ~Connection()
                {}

                void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
                {
                    if(m_nOwnerType==owner::client)
                    {
                        asio::async_connect(m_socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
                        {
                            if(!ec)
                            {
                                std::cout<<"Connected to server\n";
                                ReadValidation();
                            }
                        });
                    }
                }

                void Disconnect()
                {
                    if(IsConnected())
                        asio::post(m_asioContext, [this](){m_socket.close();});
                }

                bool IsConnected()
                {
                    return m_socket.is_open();
                }

                void Send(const info<T>& info)
                {
                    asio::post(m_asioContext,
                        [this, info]()
                        {
                            bool bWritingMessage = !m_qInfosOut.empty();

                            m_qInfosOut.push_back(info);

                            if(!bWritingMessage)
                                WriteHeader();
                        }
                    );
                }

                uint32_t GetID() const
                {
                    return id;
                }

                void ConnectToClient(nyc::jarvis::server_interface<T>* server, uint32_t uid=0)
                {
                    if(m_nOwnerType==owner::server)
                    {
                        if(m_socket.is_open())
                        {
                            id = uid;
                            std::cout << "socket cnnection now open with client from server\n";

                            WriteValidation();

                            ReadValidation(server);
                        }
                    }
                }
            private:
                void ReadHeader()
                {
                    asio::async_read(m_socket, asio::buffer(&m_infoTemporaryIn.header, sizeof(info_header<T>)),
                        [this](std::error_code ec, std::size_t length)
                        {
                            if(!ec)
                            {
                                if(m_infoTemporaryIn.header.size>0)
                                {
                                    std::cout<< "m_infoTemporaryIn.header.size > 0\n";
                                    m_infoTemporaryIn.body.resize(m_infoTemporaryIn.header.size);
                                    ReadBody();
                                }
                                else
                                {
                                    AddToIncomingInfoQueue();
                                }
                            }
                            else
                            {
                                std::cout << "[" << id << "] Read Header Fail.\n";
                                m_socket.close();
                            }
                        }
                    );
                }

                void ReadBody()
                {
                    	asio::async_read(m_socket, asio::buffer(m_infoTemporaryIn.body.data(), m_infoTemporaryIn.body.size()),
                        [this](std::error_code ec, std::size_t length)
                        {
                            if (!ec)
                            {
                                std::cout << "Read body\n";
                                AddToIncomingInfoQueue();
                            }
                            else
                            {
                                std::cout << "[" << id << "] Read Body Fail.\n";
                                m_socket.close();
                            }
                        });
                }

                void WriteHeader()
                {
                    asio::async_write(m_socket, asio::buffer(&m_qInfosOut.front().header, sizeof(info_header<T>)),
                        [this](std::error_code ec, std::size_t length)
                        {
                            if (!ec)
                            {
                                if (m_qInfosOut.front().body.size()>0)
                                {
                                    WriteBody();
                                }
                                else{
                                    m_qInfosOut.pop_front();

                                    if (!m_qInfosOut.empty())
                                        WriteHeader();
                                }
                            }
                            else
                            {
                                std::cout << "[" << id << "] Write Header Fail.\n";
                                m_socket.close();
                            }
                    });
                }

                void WriteBody()
                {
                    asio::async_write(m_socket, asio::buffer(m_qInfosOut.front().body.data(), m_qInfosOut.front().body.size()),
                        [this](std::error_code ec, std::size_t length)
                        {
                            if (!ec)
                            {
                                m_qInfosOut.pop_front();

                                if (!m_qInfosOut.empty())
                                    WriteHeader();
                            }
                            else
                            {
                                std::cout << "[" << id << "] Write Body Fail.\n";
                                m_socket.close();
                            }
                        }
                        );
                }

                void AddToIncomingInfoQueue()
                {
                    std::cout << "added to incoming info queue "<<(int)m_nOwnerType << std::endl;
                    if (m_nOwnerType == owner::server)
                        m_qInfosIn.push_back({ this->shared_from_this(), m_infoTemporaryIn });

                    else if (m_nOwnerType == owner::client)
                    {
                        std::cout << "Added incoming info to queue\n";
                        m_qInfosIn.push_back({ nullptr, m_infoTemporaryIn });
                    }

                    ReadHeader();

                }

                uint64_t scramble(uint64_t nInput)
                {
                    uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
                    out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;

                    return out ^ 0xC0DEFACE12345678;
                }

                void WriteValidation()
                {
                    std::cout << "sending validation code\n";
                    asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
                        [this](std::error_code ec, std::size_t length)
                        {
                            if (!ec)
                            {

                                if (m_nOwnerType == owner::client)
                                    ReadHeader();
                            }
                            else
                            {
                                m_socket.close();
                            }
                        }
                        );
                }

                void ReadValidation(nyc::jarvis::server_interface<T>* server = nullptr)
                {
                        asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
                            [this, server](std::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {
                                    if (m_nOwnerType == owner::server)
                                    {
                                        if (m_nHandshakeIn == m_nHandshakeCheck)
                                        {
                                            std::cout << "Client Validated Successfully\n";
                                            server->OnClientValidated(this->shared_from_this());

                                            ReadHeader();
                                        }
                                        else
                                        {
                                            std::cout << "Client Disconnected (Fail Validation)\n";
                                            m_socket.close();
                                        }
                                    }

                                    else if (m_nOwnerType == owner::client)
                                    {
                                        
                                        m_nHandshakeOut = scramble(m_nHandshakeIn);
                                        WriteValidation();
                                    }
                                }
                                else
                                {
                                    std::cout << "Client Disconnected (Read Validation)\n";
                                    m_socket.close();
                                }
                            }
                            );
                }

                protected:
                    asio::ip::tcp::socket m_socket;

                    asio::io_context& m_asioContext;

                    threadsafeQueue<info<T>> m_qInfosOut;

                    threadsafeQueue<owned_info<T>>& m_qInfosIn;
                    info<T> m_infoTemporaryIn;
                    owner m_nOwnerType = owner::server;

                    uint32_t id = 0;

                    uint64_t m_nHandshakeOut = 0;
                    uint64_t m_nHandshakeIn = 0;
                    uint64_t m_nHandshakeCheck = 0;
        };
    }
}

#endif