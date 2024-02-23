#ifndef NYCJARVIS_SERVER_HPP_
#define NYCJARVIS_SERVER_HPP_
#include "nyc_jarvis.hpp"
#include "nyc_jarvis_custom_info_types.hpp"


class CustomServer : public nyc::jarvis::server_interface<CustomInfoTypes>
{
public:
	CustomServer(uint16_t nPort) : nyc::jarvis::server_interface<CustomInfoTypes>(nPort)
	{

	}

	virtual char* getCurrentInputString()
	{
		std::cout<<"getCurrentInputString() data: "<<current_input_string<<std::endl;
		return current_input_string;
	}

	virtual void clearCurrentInputString()
	{
		current_input_string_client = nullptr;
		delete current_input_string;
		current_input_string = "";
	}

	virtual void sendInputString(char* inputString)
	{
		std::string inputString_(inputString);

		nyc::jarvis::info<CustomInfoTypes> info;

		info.header.id = CustomInfoTypes::I_S;

		for(int i=0; i<inputString_.size(); i++)
		{
			info<<inputString_[i];
		}
		std::cout<<"sending input string: "<<inputString<<std::endl;
		OnInfo(current_input_string_client, info);
		clearCurrentInputString();
	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<nyc::jarvis::Connection<CustomInfoTypes>> client)
	{
		nyc::jarvis::info<CustomInfoTypes> info;
		info.header.id = CustomInfoTypes::CONNECTION_ACCEPTED;
		client->Send(info);	
		return true;
	}

	virtual void OnClientDisconnect(std::shared_ptr<nyc::jarvis::Connection<CustomInfoTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}
	virtual void OnInfo(std::shared_ptr<nyc::jarvis::Connection<CustomInfoTypes>> client, nyc::jarvis::info<CustomInfoTypes>& info) 
	{
		std::cout << "On Info\n";
		switch (info.header.id)
		{
		case CustomInfoTypes::GIF:
			std::cout << "[" << client->GetID() << "]: Server GIF Ping\n";

			client->Send(info);
			break;
		case CustomInfoTypes::I_S:
			std::cout<<"server received I_S\n";
			client->Send(info);
			break;
		}
	}




};

#endif