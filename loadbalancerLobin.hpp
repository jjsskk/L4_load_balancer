template <typename T>
bool contains(std::set<T> &listOfElements, const T &element)
{
	// Find the iterator if element in list
	auto it = std::find(listOfElements.begin(), listOfElements.end(), element); // # include <algorithm>
	// return if iterator points to end or not. It points to end then it means element
	//  does not exists in list
	return it != listOfElements.end();
}

void check_portlist()
{

	do
	{
		port_incoming_pkt_from_server = rand() % 65535;
		// port_incoming_pkt_from_server = 10000;

	} while (contains(portlist_for_server, port_incoming_pkt_from_server));

	portlist_for_server.insert(port_incoming_pkt_from_server);
	printf("\nport list used :");
	for (auto it = portlist_for_server.begin(); it != portlist_for_server.end(); it++)
		printf("  %d", *it);
	printf("\n");
}

void round_robin(struct sockaddr_in *src, struct sockaddr_in *dst, int *tempport, int tcph_port, char *iph_saddr)
{
	check_portlist();
	struct ip_port_element *element_client = (struct ip_port_element *)malloc(sizeof(struct ip_port_element));
	struct ip_port_element *element_server = (struct ip_port_element *)malloc(sizeof(struct ip_port_element));
	strcpy(element_client->ip, iph_saddr); // based on host order
	element_client->port = tcph_port;	   // based on host order

	// round robin algorithm
	auto selected_server = server_table.at(round_robin_index);
	strcpy(server_ip, selected_server->ip);
	server_port = selected_server->port;
	round_robin_index = round_robin_index + 1;
	round_robin_index = round_robin_index % 3;
	// round robin algorithm

	strcpy(element_server->ip, server_ip); // server ip to send pkt to //based on host order
	element_server->port = server_port;	   // server port to send pkt to // based on host order
	element_server->port_incoming_pkt_from_server = port_incoming_pkt_from_server;
	printf("ippp port :%s %d\n", element_client->ip, element_client->port);

	std::list<struct ip_port_element *> *element = new std::list<struct ip_port_element *>;
	element->push_back(element_client);
	element->push_back(element_server);
	ip_port_table.push_back(element);

	if (inet_pton(AF_INET, server_ip, &(dst->sin_addr)) != 1) // set server ip addr
	{
		perror("Server destination IP and Port configuration failed");
		exit(EXIT_FAILURE);
	}

	*tempport = port_incoming_pkt_from_server;
	src->sin_port = htons(port_incoming_pkt_from_server);
	dst->sin_port = htons(server_port);
}