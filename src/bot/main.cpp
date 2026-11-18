#include "Bot.hpp"

int	main(int ac, char **av)
{
	if (ac != 4)
		return (1);
	Bot bot(av[1], av[2], av[3]);
	bot.run();
	return (0);
}
