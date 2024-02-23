#include "nyc_jarvis.hpp"

std::mutex m;
bool bQuit = false;


class CustomClient : public nyc::jarvis::client_interface<CustomInfoTypes>
{
public:
	bool getVideo(const char* title)
	{
		/*nyc::jarvis::info<Video> info;
		info.header.id = Video::GIF;
		info << title;
		Send(info);*/
		return true;
	}

	void PingServer()
	{
		nyc::jarvis::info<CustomInfoTypes> info;
		info.header.id = CustomInfoTypes::GIF;

		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		
		info << timeNow;
		std::cout << "pining server\n";
		Send(info);
	}

};

CustomClient c;
void runASIOClient()
{
    m.lock();
    c.Connect("127.0.0.1", 60000);
    m.unlock();
    while (!bQuit)
	{
        m.lock();
        bool IsConnected = c.IsConnected();
        m.unlock();
		if (IsConnected)
		{
            m.lock();
            bool isIncomingEmpty = c.Incoming().empty();
            m.unlock();
			if (!isIncomingEmpty)
			{
				std::cout << "received incoming packet\n";
                m.lock();
				auto info = c.Incoming().pop_front().i;
                m.unlock();

				std::cout << "incoming packet header id: " << (int)info.header.id << std::endl;
				std::cout<<"typeid(info.header.id).name(): "<<typeid(info.header.id).name()<<std::endl;

				switch (info.header.id)
				{
				case CustomInfoTypes::I_S:
				{
					std::cout<<"input_string in client: ";
					int original_info_size_ = info.size();
					std::string m_current_input_string_(original_info_size_, 'x');
					for(int l=0; l<original_info_size_; l++)
					{
						info>>m_current_input_string_[l];
					}
					std::cout<<m_current_input_string_<<std::endl;
				}
					break;
				case CustomInfoTypes::GIF:
					{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					info >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << std::endl;
				
					}				
					break;
				case CustomInfoTypes::CONNECTION_ACCEPTED:
					std::cout << "Server Accepted Connection\n";
					//c.PingServer();

						// char* s = "This is a test!";
						// std::string k(s);
						// std::cout<<"k: "<<k<<std::endl;
						// nyc::jarvis::info<CustomInfoTypes> i;
						// i.header.id = CustomInfoTypes::I_S;
						// std::cout<<"c: ";
						// for(char c:k)
						// {
						// 	std::cout<<c;
						// 	i<<c;
						// }

						// std::cout<<std::endl;

						// c.Send(i);
					break;
				

				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
            m.lock();
			bQuit = true;
            m.unlock();
		}
	}
}

void runGTKClient()
{
    c.StartGTK();
    char input;
    while(!bQuit)
    {

	

        std::cin>>input;

        if(input=='A')
        {
            m.lock();
            bQuit=true;
            m.unlock();

			
        }

	
    }
}

int main()
{

	std::thread t1(runASIOClient);
    std::thread t2(runGTKClient);

    t1.join();
    t2.join();

    std::cout<<"Done\n";


	return 0;


}