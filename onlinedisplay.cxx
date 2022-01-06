#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>

#include "TApplication.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TThread.h"

#define EV_MAGIC   0xffffffff
#define EV_TRAILER 0xaaaaaaaa
#define EV_TRAILER_H 0xaaaa


struct event {
	unsigned int magic;
	unsigned short int data[256];
	unsigned int trailer;

};


int scan_min(std::vector<int> &data)
{
	int min = 1024;
	for (unsigned int i = 0 ; i < data.size() ; i++) {
		int val = data[i] & 0x3ff;
		//bool otr = (data[i] & 0x400 != 0);
		if (val < min) min = val;
	}

	return min;
}

int decode_data(char *buf, std::vector<int> &data)
{
	struct event *event = reinterpret_cast<struct event *>(buf);
	unsigned int magic = ntohl(event->magic);
	unsigned short int *bufs = reinterpret_cast<unsigned short int *>(&(event->data[0]));
	int ndata;
	data.clear();
	data.resize(0);

	if (magic == EV_MAGIC) {
		int i = 0;
		while (true) {
			unsigned short int val = ntohs(bufs[i++]);
			if (val == EV_TRAILER_H) break;
			data.push_back(static_cast<int>(val));
			if (i > 257) {
				std::cerr <<  "Data Error" << std::endl;
				exit(1);
			}
		}
		ndata = i;
	} else {
		ndata = 0;
	}
	return ndata;
}

static TH2F *h2fadc;
static TH1F *h1peak;

int hist_init()
{

	h2fadc = new TH2F("FADC", "Fadc", 256, 0, 256, 1024, 0, 1024);
	h1peak = new TH1F("PEAK", "Peak", 512, 0, 1024);

	return 0;
}

int hist_fill(std::vector<int> &vfadc)
{
	std::cout << "#D size " << vfadc.size() << std::endl;
	for (unsigned int i = 0 ; i < vfadc.size() ; i++) {
		int val;
		if ((vfadc[i] & 0x400) == 0x400) {
			val = 1023;
		} else {
			val = vfadc[i] & 0x3ff;
		}

		h2fadc->Fill(i, val, 1.0);
	}
	h1peak->Fill(scan_min(vfadc), 1.0);

	return 0;
}

static int no_exit = 0;

void reader(void *arg = NULL)
{
	static char cbuf[1024];
	std::vector<int> data;
	int event_number = 0;
	while (true) {
		std::cin.read(cbuf, sizeof(struct event));
		if (std::cin.eof()) break;
		if (decode_data(cbuf, data) > 0) {
			hist_fill(data);
		} else {
			std::cerr << "Invalid Header "
				<< ntohl(static_cast<unsigned int>(cbuf[0])) << std::endl;
			break;
		}
		if ((event_number % 100) == 0) std::cout << "Event : " << event_number << std::endl;
		event_number++;
	}

	if (! no_exit) exit(0);
}


int ta_argc = 1;
char *ta_argv[] = {const_cast<char *>("App")};
static TApplication *app = new TApplication("App", &ta_argc, ta_argv);
static TCanvas *c1 = new TCanvas("c1", "Online Display");

void updater(void *arg = NULL)
{
	while (true) {
		TThread::Sleep(1, 0);
		c1->cd(1)->Modified();
		c1->cd(2)->Modified();
		c1->Update();
	}
}


int main(int argc, char* argv[])
{
	for (int i = 1 ; i < argc ; i++) {
		if (strncmp(argv[i], "--no-exit", 9) == 0) no_exit = 1;
	}

	hist_init();

	c1->Divide(1, 2);
	c1->cd(1);
	h2fadc->Draw("colz");
	c1->cd(2);
	h1peak->Draw();
	c1->Update();
	c1->Flush();

	TThread threader("Reader", &reader, NULL);
	TThread thupdater("Updater", &updater, NULL);
	threader.Run();
	thupdater.Run();

	app->Run();

	return 0;
}
