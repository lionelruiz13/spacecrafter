/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 of Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _CPUINFO_H_
#define _CPUINFO_H_

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

const int NUM_CPU_STATES = 10;
const int TAMPON_SIZE = 10;

/**
 * \class CPUInfo
 * \brief Creation of CPU cores usage logs
 * \author Olivier NIVOIX
 * \date 16 juin 2018
 * 
 * The purpose of this class is to regularly record the activities of the processor cores in a file
 * 
 * @section DESCRIPTION
 * 
 * The class uses the reading of the system file /proc/stat that it parses 
 * 
 * start() creates a thread that periodically records information from /proc/stat
 * 
 * stop() stops the thread and closes the records.
 * 
 * @section part thread
 * 
 * When starting() is launched, the function creates a thread that loops in this way:
 * 
 * std::this_thread::sleep_for(std::chrono::seconds(1));
 * this -> getCPUstate();
 * this -> archivingData();
 * 
 * archivingData() empties the information table thanks to the variable nbDiff which varies from 0 to BUFFER_SIZE
 * 
 * @section OPERATION
 * 
 * The class has few methods.
 * 
 * CPUInfo cpuInfo;
 * cpuInfo.init("destination_file");
 * cpuInfo.start();
 * 
 * ... various instructions ...
 * 
 * cpuInfo.stop();
 * 
 */
 
class CPUInfo
{
public:
	CPUInfo();
	~CPUInfo();

	//! starts the analysis of CPU logs
	void start();

	//! ends the analysis of the CPU logs
	void stop();

	//! retrieves the parameters needed by the class
	//! \param CPUlogFile : name of the file storing the CPU data
	//! \param GPUlogFile : name of the file storing GPU data
	void init(const std::string &logCPU_file, const std::string &logGPU_file );
	//~ void display(std::vector<CoreData> entrie);

private:
	// description of /proc/stat entries
	enum CPUStates {
		S_USER = 0,
		S_NICE,
		S_SYSTEM,
		S_IDLE,
		S_IOWAIT,
		S_IRQ,
		S_SOFTIRQ,
		S_STEAL,
		S_GUEST,
		S_GUEST_NICE
	};

	//contains info from one core
	typedef struct CoreData {
		size_t times[NUM_CPU_STATES];
	} CoreData;

	// main function for the thread
	void mainFunc();
	// allows to capture data in entrieA or entrieB
	void getCPUstate();
	//function to read a value from the GPU
	void getGPUstate();
	// function to store data in an array
	void archivingData();
	// allows you to capture data in a specified entry
	void getCPUstate(std::vector<CoreData> &entrie);
	// performs the subtraction between two entries
	void diffEntrie(const std::vector<CoreData> entrieA, const std::vector<CoreData> entrieB);
	// function that allows to physically save the information on the DD
	void saveToFile();
	// variables used for data capture
	std::vector<CoreData> entrieA, entrieB, result;
	std::stringstream oss[TAMPON_SIZE];	// output buffer for CPUfileLog
	std::stringstream gpuOss;	 		// cGPU output character string
	unsigned int nbThread=0;	// Variable containing the number of cores available on the machine
	uint64_t frame=0;	// Number of the analyzed frame
	unsigned char diff = 0;		// Difference counter before saving in CPUfileLog
	bool isActived = true;		// Flag to close the thread
	std::thread t;				// thread
	std::ofstream CPUfileLog;	// file for CPU information
	std::ofstream GPUfileLog;	// file for GPU information
};


#endif // _CPUINFO_H_
