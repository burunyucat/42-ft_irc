#include "../../include/Commands.hpp"


void handleCommand()
{

}

int parsingCommand(std::string line)
{
	Commands command;
	int words;
	int indexstart;

	words = 0;
	indexstart = 0;
	for (size_t i = 0; i < line.size(); i++)
	{
		if(line[i] == ' ' || i == line.size() - 1)
		{
			if(words == 0)
				command.name = line.substr(indexstart, i);
			else if(words > 0)
			{
				if(line[indexstart] == ':')
				{
					command.params.push_back(line.substr(indexstart +1, line.size() -indexstart-1));
					break;
				}
				else
					command.params.push_back(line.substr(indexstart, i - indexstart));
			}
			words ++;
			while(line[i] == ' ' && i < line.size())
				i++;
			if(line[i] != ' ' && i < line.size())
				indexstart = i;
			}
	}
	
	// Test parsing
	std::cout << "name = " << command.name <<std::endl;
	for (size_t i = 0; i < command.params.size(); i++)
		std::cout << "params["<< i <<"] = " << command.params[i] <<std::endl;
	return 0;
}