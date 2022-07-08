
#ifndef WIN32
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#endif

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996) 

#include "Events.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <format>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#define SERVER "127.0.0.1"	// or "localhost" - ip address of UDP server
#define BUFLEN 512			// max length of answer
#define PORT 6969			// the port on which to listen for incoming data

using nlohmann::json;

namespace DSXSkyrimEvent
{

	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kDataLoaded) {
			logger::info("Data Load CallBack Trigger!");


			if (DSXSkyrim::EquipStartEventHandler::RegisterEquipStartEvent())
				logger::info("Register Equip Event!");
		}
	}

}


namespace DSXSkyrim
{
	using socket_t = decltype(socket(0, 0, 0));
	socket_t mysocket;
	sockaddr_in server;
	DSXSkyrim::TriggersCollection userTriggers;
	vector<Packet> myPackets;

	json to_json(json& j, const TriggerSetting& p)
	{
		j = {
			{ "Name", p.name },
			{ "Category", p.category },
			{ "Description", p.description },
			{ "TriggerSide", p.triggerSide },
			{ "TriggerType", p.triggerType },
			{ "customTriggerMode", p.customTriggerMode },
			{ "playerLEDNewRev", p.playerLEDNewRev },
			{ "MicLEDMode", p.micLEDMode },
			{ "TriggerThreshold", p.triggerThresh },
			{ "ControllerIndex", p.controllerIndex },
			{ "TriggerParams", p.triggerParams },
			{ "RGBUpdate", p.rgbUpdate },
			{ "PlayerLED", p.playerLED }
		};

		return j;
	}

	void from_json(const json& j, TriggerSetting& p)
	{
		j.at("Name").get_to(p.name);
		j.at("Category").get_to(p.category);
		j.at("Description").get_to(p.description);
		j.at("TriggerSide").get_to(p.triggerSide);
		j.at("TriggerType").get_to(p.triggerType);
		j.at("customTriggerMode").get_to(p.customTriggerMode);
		j.at("playerLEDNewRev").get_to(p.playerLEDNewRev);
		j.at("MicLEDMode").get_to(p.micLEDMode);
		j.at("TriggerThreshold").get_to(p.triggerThresh);
		j.at("ControllerIndex").get_to(p.controllerIndex);
		j.at("TriggerParams").get_to(p.triggerParams);
		j.at("RGBUpdate").get_to(p.rgbUpdate);
		j.at("PlayerLED").get_to(p.playerLED);
	}

	void readFromConfig()
	{

		try {
			json j;
			std::ifstream stream(".\\Data\\SKSE\\Plugins\\DSXSkyrim\\DSXSkyrimConfig.json");
			stream >> j;
			logger::info("JSON File Read from location");

			logger::info("Try assign j");

			DSXSkyrim::TriggerSetting conversion;

			for (int i=0; i < j.size(); i++) 
			{
				conversion = j.at(i).get<DSXSkyrim::TriggerSetting>();
				userTriggers.TriggersList.push_back(conversion);
			}
			

		logger::info("Breakpoint here to check value of userTriggers");

		} catch (exception e) {
			throw e;
			
		}
	}

	//inline void writeToConfig(json j)
	//{
	//	try {

	//		const char* path = ".\\Data\\SKSE\\Plugins\\DSXSkyrim\\TestJson23.json";

	//		std::ofstream file_ofstream(path);
	//		file_ofstream << j.dump(4) << std::endl;

	//	} catch (exception& e) {
	//		logger::debug(FMT_STRING("Exception thrown :  {}"), e.what());
	//	}
	//	logger::info("JSON File Written and Closed Supposedly");
	//}

	

	void StartupUDP()
	{
		
		// initialise winsock
		WSADATA ws;
		logger::info("Initialising Winsock...");
		if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
			logger::error("Failed. Error Code: %d", WSAGetLastError());
					}
		logger::info("Initialised.\n");
		// create socket
		
		if ((mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)	 // <<< UDP socket
		{
			logger::error("socket() failed with error code: %d", WSAGetLastError());
		}

		// setup address structure
		memset((char*)&server, 0, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons(PORT);
		server.sin_addr.S_un.S_addr = inet_addr(SERVER);

	}

	void setInstructionParameters(TriggerSetting& TempTrigger, Instruction& TempInstruction)
	{
		switch (TempInstruction.type) {
		case 1:	 //TriggerUpdate
			switch (TempTrigger.triggerParams.size()) {
			case 0:
				TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide),
					std::to_string(TempTrigger.triggerType) };
				break;

			case 1:
				TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType),
				std::to_string(TempTrigger.triggerParams.at(0))};
				break;

			case 2:
				TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType),
				std::to_string(TempTrigger.triggerParams.at(0)), std::to_string(TempTrigger.triggerParams.at(1)) };
				break;

			case 3:
				TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType),
				std::to_string(TempTrigger.triggerParams.at(0)), std::to_string(TempTrigger.triggerParams.at(1)),
				std::to_string(TempTrigger.triggerParams.at(2)) };
				break;

			case 4:
				TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType),
					std::to_string(TempTrigger.triggerParams.at(0)), std::to_string(TempTrigger.triggerParams.at(1)),
					std::to_string(TempTrigger.triggerParams.at(2)), std::to_string(TempTrigger.triggerParams.at(3)) };
				break;

			case 5:
				TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType),
					std::to_string(TempTrigger.triggerParams.at(0)), std::to_string(TempTrigger.triggerParams.at(1)),
					std::to_string(TempTrigger.triggerParams.at(2)), std::to_string(TempTrigger.triggerParams.at(3)),
				std::to_string(TempTrigger.triggerParams.at(4)) };
				break;

			case 6:
				TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType),
					std::to_string(TempTrigger.triggerParams.at(0)), std::to_string(TempTrigger.triggerParams.at(1)),
					std::to_string(TempTrigger.triggerParams.at(2)), std::to_string(TempTrigger.triggerParams.at(3)),
					std::to_string(TempTrigger.triggerParams.at(4)), std::to_string(TempTrigger.triggerParams.at(5)) };
				break;

			case 7:
				if (TempTrigger.triggerType == 12) {
					TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType),
						std::to_string(TempTrigger.customTriggerMode), std::to_string(TempTrigger.triggerParams.at(0)), std::to_string(TempTrigger.triggerParams.at(1)),
						std::to_string(TempTrigger.triggerParams.at(2)), std::to_string(TempTrigger.triggerParams.at(3)),
						std::to_string(TempTrigger.triggerParams.at(4)), std::to_string(TempTrigger.triggerParams.at(5)),
						std::to_string(TempTrigger.triggerParams.at(6)) };
					break;
				} else {
					TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType),
						std::to_string(TempTrigger.triggerParams.at(0)), std::to_string(TempTrigger.triggerParams.at(1)),
						std::to_string(TempTrigger.triggerParams.at(2)), std::to_string(TempTrigger.triggerParams.at(3)),
						std::to_string(TempTrigger.triggerParams.at(4)), std::to_string(TempTrigger.triggerParams.at(5)),
						std::to_string(TempTrigger.triggerParams.at(6)) };
					break;
				}

			default:
				TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerType) };
				break;
			}
			break;

		case 2:	 //RGBUpdate
			TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.rgbUpdate.at(0)),
			std::to_string(TempTrigger.rgbUpdate.at(1)), std::to_string(TempTrigger.rgbUpdate.at(2)) };
			break;

		case 3:	 //PlayerLED    --- parameters is set to vector<int> so the bool is not coming across. need to fix
			TempInstruction.parameters = {
				std::to_string(TempTrigger.controllerIndex), "false", "false", "false", "false", "false"};
			break;

		case 4:	 //TriggerThreshold
			TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide), std::to_string(TempTrigger.triggerThresh) };
			break;

		case 5:	 //InstructionType.MicLED
			TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.micLEDMode) };
			break;

		case 6:	 //InstructionType.PlayerLEDNewRevision
			TempInstruction.parameters = { std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.playerLEDNewRev) };
			break;
		
		}
	}

	void generatePacketInfo(TriggersCollection& userTriggers, vector<Packet>& myPackets)
	{
		
		for (int i = 0; i < userTriggers.TriggersList.size(); i++) 
		{
			Packet TempPacket;
			myPackets.push_back(TempPacket);

			myPackets.at(i).instructions[0].type = 1;	 //InstructionType.TriggerUpdate
			setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[0]);

			myPackets.at(i).instructions[2].type = 2;	 //InstructionType.RGBUpdate
			setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[2]);

			myPackets.at(i).instructions[3].type = 3;	 //InstructionType.PlayerLED - fk this contains bools and mixed int
			setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[3]);

			myPackets.at(i).instructions[1].type = 4;  //InstructionType.TriggerThreshold
			setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[1]);

			myPackets.at(i).instructions[5].type = 5;  //InstructionType.MicLED
			setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[5]);

			myPackets.at(i).instructions[4].type = 6;	 //InstructionType.PlayerLEDNewRevision
			setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[4]);

		}
	}


}



//
//
//// receive a reply and print it
//// clear the answer by filling null, it might have previously received data
//char answer[BUFLEN] = {};
//
//// try to receive some data, this is a blocking call
//int slen = sizeof(sockaddr_in);
//int answer_length;
//if (answer_length = recvfrom(client_socket, answer, BUFLEN, 0, (sockaddr*)&server, &slen) == SOCKET_ERROR) {
//	logger::error("recvfrom() failed with error code: %d", WSAGetLastError());
//	exit(0);
//}


//
//closesocket(client_socket);
//WSACleanup();
