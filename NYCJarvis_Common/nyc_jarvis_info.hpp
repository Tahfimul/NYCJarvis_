#ifndef NYC_JARVIS_INFO_HPP_
#define NYC_JARVIS_INFO_HPP_

#include "nyc_jarvis_base.hpp"

namespace nyc
{
    namespace jarvis
    {
        template <typename T>
        struct info_header
        {
            T id{};
            uint32_t size = 0;
        };

        template <typename T>
        class info
        {
            public:
                info_header<T> header{};
                std::vector<uint8_t> body;

                size_t size() const
                {
                    return body.size();
                }

                friend std::ostream& operator << (std::ostream& os, const info<T>& info)
                {
                    os<<"ID:"<<int(info.header.id) << " Size: "<<info.header.size<<std::endl;
                    return os;
                }

                template <typename Infotype>
                friend info<T>& operator<<(info<T>& info, const Infotype& infodata)
                {
                    std::cout<<" infodata: "<<infodata;
                    static_assert(std::is_standard_layout<Infotype>::value, "Data is not simple enough to be pushed into body vector");

                    size_t s = info.body.size();

                    info.body.resize(s+sizeof(Infotype));

                    std::memcpy(info.body.data()+s, &infodata, sizeof(Infotype));

                    

                    info.header.size = info.size();

                    return info;
                }

                template<typename Infotype>
                friend info<T>& operator>>(info<T>& info, Infotype& infodata)
                {
                    static_assert(std::is_standard_layout<Infotype>::value, "Data is not simple enough to be pushed into body vector");

                    size_t s = info.body.size() - sizeof(Infotype);

                    std::memcpy(&infodata, info.body.data()+s, sizeof(Infotype));

                    std::cout<<"infodata after std::memcpy: "<<infodata<<" ";

                    info.body.resize(s);

                    std::cout<<"resizing done for char: "<<infodata<<std::endl;

                    info.header.size = info.size();

                    return info;
                }

        };

        template <typename T>
        class Connection;

        template <typename T>
        class owned_info
        {
            public:
                std::shared_ptr<Connection<T>> remote = nullptr;
                info<T> i;

                friend std::ostream& operator<<(std::ostream& os, const owned_info<T>& info)
                {
                    os<<info.i;
                    return os;
                }
        };
    }
}


#endif