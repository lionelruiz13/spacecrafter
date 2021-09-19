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

#include <chrono>
#include "mainModule/CPUInfo.hpp"

CPUInfo::CPUInfo()
{
	this->nbThread = std::thread::hardware_concurrency();
	//~ std::cout << "Le CPU dispose de " << this->nbThread << " threads" << std::endl;
	entrieA.resize(nbThread+1);
	entrieB.resize(nbThread+1);
	result.resize(nbThread+1);
}

void CPUInfo::diffEntrie(const std::vector<CoreData> _entrieA, const std::vector<CoreData> _entrieB)
{
	for (unsigned int i=0; i<this->nbThread+1; i++) {
		for(unsigned int j=0; j< NUM_CPU_STATES; j++) {
			result[i].times[j] = _entrieB[i].times[j] - _entrieA[i].times[j];
		}
	}
}


CPUInfo::~CPUInfo()
{
	entrieA.clear();
	entrieB.clear();
	result.clear();
}

void CPUInfo::getCPUstate()
{
	if (frame%2==1) {
		this->getCPUstate(entrieA);
	} else {
		this->getCPUstate(entrieB);
	}
	frame++;
	//~ std::cout << "frame " << frame << std::endl;
}

void CPUInfo::getCPUstate(std::vector<CoreData> &entrie)
{
	std::ifstream fileStat("/proc/stat");

	std::string line, name;

	const std::string STR_CPU("cpu");
	const std::size_t LEN_STR_CPU = STR_CPU.size();

	int j=0;

	while(std::getline(fileStat, line))
	{
		// cpu stats line found
		if(!line.compare(0, LEN_STR_CPU, STR_CPU))
		{
			std::istringstream ss(line);

			// read unused cpu label
			ss >> name;
			// read times
			for(int i = 0; i < NUM_CPU_STATES; ++i)
				ss >> entrie[j].times[i];
		}
		j++;
	}
}


void CPUInfo::getGPUstate()
{
	char buffer[128];
    FILE* pipe = nullptr;
	pipe = popen("nvidia-smi --format=csv,noheader,nounits --query-gpu=utilization.gpu,utilization.memory,memory.free,pstate,fan.speed,temperature.gpu", "r");
    if (!pipe) return;
	//seule la 1° ligne nous intéresse
	if (fgets(buffer, 128, pipe) != NULL)
        gpuOss << frame-1 <<  "," << std::string(buffer);

    pclose(pipe);
}


void CPUInfo::archivingData()
{
	if (frame%2==1)
		this->diffEntrie(this->entrieA, this->entrieB);
	else
		this->diffEntrie(this->entrieB, this->entrieA);
	diff++;

	for (unsigned int i=0; i<nbThread+1; i++) {

		oss[diff-1] << frame-1  << "," << i << "," 
				<< result[i].times[S_USER] << "," 
				<< result[i].times[S_NICE] << "," 
				<< result[i].times[S_SYSTEM] << "," 
				<< result[i].times[S_IDLE] << "," 
				<< result[i].times[S_IOWAIT] << "," 
				<< result[i].times[S_IRQ] << "," 
				<< result[i].times[S_SOFTIRQ] << "," 
				<< result[i].times[S_STEAL] << "," 
				<< result[i].times[S_GUEST] << "," 
				<< result[i].times[S_GUEST_NICE] << "," 
		<< std::endl;
	}
	if (diff>TAMPON_SIZE-1)
		this->saveToFile();
	//~ CPUfileLog << oss.str();
}

void CPUInfo::saveToFile()
{
	//~ std::cout << "savegarde..." << std::endl;
	for(unsigned int i=0; i<diff; i++) {
		CPUfileLog << oss[i].str();
		oss[i].str("");
	}
	diff=0;

	GPUfileLog << gpuOss.str();
	gpuOss.str("");
}


void CPUInfo::mainFunc()
{
	CPUfileLog << "frame, cpu,user, nice,system, idle, iowait, irq, softirq,steal, guest, guest_nice" << std::endl;
	GPUfileLog << "frame, gpu.usage, memory.usage,memory.free,power.state,fan.speed,temperature" << std::endl;
	this -> getCPUstate();

	while (this->isActived) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		this -> getCPUstate();
		this -> archivingData();

		this -> getGPUstate();
	}
	this->saveToFile();
}

void CPUInfo::start()
{
	t = std::thread(&CPUInfo::mainFunc, this);
}

void CPUInfo::stop()
{
	if (!isActived)
		return;

	isActived = false;
	t.join();
	CPUfileLog.close();
	GPUfileLog.close();
}

void CPUInfo::init(const std::string &logCPU_file, const std::string &logGPU_file )
{
	CPUfileLog.open(logCPU_file);
	GPUfileLog.open(logGPU_file);

	if (!CPUfileLog.is_open() || !GPUfileLog.is_open())
		isActived = false;
	
}
