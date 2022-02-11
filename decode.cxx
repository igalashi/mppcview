/*
 *
 */

#ifndef INC_MPPC_DECODER
#define INC_MPPC_DECODER

#include <iostream>
#include <iomanip>
#include <vector>

#include <arpa/inet.h>


struct event_header {
	uint32_t magic;
	uint32_t length;
	uint32_t trigger;
	uint32_t reserve;
	uint16_t data[];
};

#define EV_MAGIC 0xffff5555
#define N_CH 8


int decode_data(char *buf, std::vector<std::vector<int> > &data)
{
	struct event_header *event = reinterpret_cast<struct event_header *>(buf);
	unsigned int magic = ntohl(event->magic);
	unsigned int len = ntohl(event->length);
	int ndata = len / sizeof(uint16_t);
	uint16_t *body = reinterpret_cast<unsigned short int *>(&(event->data[0]));

	data.clear();
	data.resize(N_CH);
	for (int i = 0 ; i < N_CH ; i++) {
		data[i].clear();
		data[i].resize(0);
	}
	
	if (magic == EV_MAGIC) {
		for (int i = 0 ; i < ndata ; i++) {
			unsigned short int val = ntohs(body[i]);
			data[i % N_CH].push_back(static_cast<int>(val));
		}
	} else {
		ndata = 0;
	}

	#if 0
	unsigned int trig = ntohl(event->trigger);
	std::cout << "#D magic: 0x" << std::hex << std::setw(8) << magic
		<< " len: " << std::dec << len
		<< " Trig: " << std::dec << trig
		<< std::endl;
	for (int i = 0 ; i < N_CH ; i++) {
		std::cout << "# " << i << " : ";
		for (int j = 0 ; j < 8 ; j++) std::cout << " " << data[i][j];
		std::cout << std::endl;
	}
	#endif

	return ndata;
}


#ifdef TEST_MAIN
void reader(void *arg = NULL)
{
	static char cbuf[1024 * 2 * 8 + sizeof(event_header) + sizeof(uint32_t)];
	struct event_header *evh;
	evh = reinterpret_cast<struct event_header *>(cbuf);
	char *evb = cbuf + sizeof(struct event_header);
	static uint32_t evt;
	static std::vector<std::vector<int> > data;
	int event_number = 0;

	while (true) {

		std::cin.read(cbuf, sizeof(struct event_header));
		if (std::cin.eof()) break;
		unsigned int len = ntohl(evh->length);
		std::cin.read(evb, len);
		std::cin.read(reinterpret_cast<char *>(&evt), sizeof(uint32_t));

		#if 0
		unsigned int magic = ntohl(evh->magic);
		unsigned int trig = ntohl(evh->trigger);
		std::cout << "#D 0x" << std::hex << std::setw(8) << magic
			<< " len: " << std::dec << len
			<< " Trig: " << std::dec << trig
			<< " Trailer: " << std::hex << evt << std::endl;
		#endif
		
		if (decode_data(cbuf, data) > 0) {
			std::cout << std::dec;
			for (unsigned int i = 0 ; i < data.size() ; i++) {
				std::cout << "# " << i << " : " << data[i].size() << " : " ;
				for (int j = 0 ; j < 8 ; j++) std::cout << " " << data[i][j];
				std::cout << std::endl;
			}
		} else {
			std::cerr << "Invalid Header "
				<< ntohl(static_cast<unsigned int>(cbuf[0])) << std::endl;
			break;
		}

		if ((event_number % 100) == 0)
			std::cout << "Event : " << event_number << std::endl;
		event_number++;
	}

}


int main(int argc, char* argv[])
{
	for (int i = 1 ; i < argc ; i++) {
		std::string sargv(argv[i]);
		if (sargv == "--no-exit") std::cout << "no exit" << std::endl;
	}

	reader();

	return 0;
}
#endif

#endif
